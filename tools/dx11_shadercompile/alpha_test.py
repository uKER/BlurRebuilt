"""Preserve the PC fixed-function alpha test at native pixel-shader exits.

The game binds the current enable/comparison/reference in PS constant buffer
b13 immediately before drawing, after either FX11 or built-in shader binding.
This buffer is independent of the shaders' authored constant layouts.
"""

import re


ALPHA_TEST_HLSL = """
cbuffer BlurPCAlphaTestState : register(b13) {
    uint BlurPCAlphaEnabled;
    uint BlurPCAlphaFunction;
    float BlurPCAlphaReference;
    uint BlurPCAlphaFormat;
};
// D3D9 converts both the output alpha and the reference to the precision of
// render target 0 before comparing (measured on the D3D9 HAL; analysis.md).
// BlurPCAlphaFormat: 0 float32, 1 float16, otherwise UNORM bit count k.
float BlurPCAlphaConvert(float value) {
    if (BlurPCAlphaFormat == 1) return asfloat(asuint(value) & 0xffffe000u);
    if (BlurPCAlphaFormat < 8) return value;
    // u = floor(v * 2^(k+4)), then (u * (2^k - 1) + 2^(k+3) - 1) >> (k+4).
    uint shift = BlurPCAlphaFormat + 4;
    uint scale = (1u << BlurPCAlphaFormat) - 1u;
    uint fixedPoint = (uint)min(floor(saturate(value) * (float)(1u << shift)),
                                (float)((1u << shift) - 1u));
    uint bias = (1u << (shift - 1u)) - 1u;
    if (shift <= 16u) return (float)((fixedPoint * scale + bias) >> shift);
    // 16-bit targets: the 36-bit product is summed in 16-bit halves.
    uint low = (fixedPoint & 0xffffu) * scale;
    uint high = (fixedPoint >> 16) * scale + (low >> 16) + (bias >> 16) +
                (((low & 0xffffu) + (bias & 0xffffu)) >> 16);
    return (float)(high >> (shift - 16u));
}
void BlurPCAlphaTest(float alpha) {
    if (!BlurPCAlphaEnabled) return;
    alpha = BlurPCAlphaConvert(alpha);
    float reference = BlurPCAlphaConvert(BlurPCAlphaReference);
    // D3DCMPFUNC values, as used by the original PC render state.
    bool passed = false;
    switch (BlurPCAlphaFunction) {
        case 1: passed = false; break;
        case 2: passed = alpha <  reference; break;
        case 3: passed = alpha == reference; break;
        case 4: passed = alpha <= reference; break;
        case 5: passed = alpha >  reference; break;
        case 6: passed = alpha != reference; break;
        case 7: passed = alpha >= reference; break;
        case 8: passed = true; break;
    }
    if (!passed) discard;
}
"""


def inject_alpha_test(source: str, entries: set[str]) -> str:
    """Wrap each PS entry, testing output colour zero before any MRT writes."""
    if not entries:
        return source
    source = re.sub(r"^\s*#line[^\n]*", "", source, flags=re.MULTILINE)
    if re.search(r"register\s*\(\s*b13\s*\)", source):
        raise ValueError("pixel shader uses reserved alpha-test buffer b13")
    for entry in sorted(entries):
        signature = re.search(
            r"\b(\w+)\s+(" + re.escape(entry) +
            r")\s*\(([^)]*)\)\s*(?::\s*(\w+))?\s*\{", source)
        if not signature:
            raise ValueError(f"cannot find pixel entry {entry}")
        result_type, _, parameters, semantic = signature.groups()
        colour = "result"
        if semantic:
            if not re.fullmatch(r"(?:COLOR|SV_Target)0?", semantic, re.I):
                raise ValueError(f"unexpected pixel result semantic {entry}: {semantic}")
            if result_type not in ("float4", "half4"):
                raise ValueError(f"unexpected pixel result type {entry}: {result_type}")
        else:
            struct = re.search(r"\bstruct\s+" + re.escape(result_type) +
                               r"\s*\{([^}]*)\}", source)
            member = re.search(r"\b(?:float4|half4)\s+(\w+)\s*:\s*"
                               r"(?:COLOR|SV_Target)0?\s*;", struct[1], re.I) if struct else None
            if not member:
                raise ValueError(f"cannot find colour zero in {entry}: {result_type}")
            colour += "." + member[1]
        arguments = []
        for parameter in parameters.split(","):
            if not parameter.strip():
                continue
            declaration = parameter.split(":")[0].strip()
            if re.search(r"\b(?:out|inout)\b", declaration):
                raise ValueError(f"output parameter in pixel entry {entry}")
            argument = re.search(r"\b(\w+)\s*$", declaration)
            if not argument:
                raise ValueError(f"unsupported pixel parameter {entry}: {parameter}")
            arguments.append(argument[1])
        depth, end = 1, signature.end()
        while depth and end < len(source):
            depth += (source[end] == "{") - (source[end] == "}")
            end += 1
        if depth:
            raise ValueError(f"unterminated pixel entry {entry}")
        wrapper = (f"\n{result_type} {entry}({parameters})" +
                   (f" : {semantic}" if semantic else "") + " {\n" +
                   f"    {result_type} result = {entry}_BlurOriginal({', '.join(arguments)});\n" +
                   f"    BlurPCAlphaTest({colour}.a);\n    return result;\n}}\n")
        source = (source[:signature.start(2)] + entry + "_BlurOriginal" +
                  source[signature.end(2):end] + wrapper + source[end:])
    return ALPHA_TEST_HLSL + source


def inject_effect_alpha_test(source: str) -> str:
    entries = set(re.findall(r"CompileShader\(\s*ps_5_0\s*,\s*(\w+)\s*\(", source))
    return inject_alpha_test(source, entries)
