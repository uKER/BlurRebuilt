"""Recover native vehicle FX from dumped retail SM3 instructions, without guessing.

FXC supplies readable instructions and original CTAB parameter names. Generated
register-level HLSL retains those names for the engine's reflected FX bindings.
Unsupported instructions, missing uniforms or sampler metadata fail closed.
This is a build-time recovery tool, not a runtime D3D9 compatibility renderer.
"""
from __future__ import annotations

import argparse
import hashlib
from pathlib import Path
import re
import subprocess


def disassemble(fxc: Path, binary: Path) -> str:
    result = subprocess.run([str(fxc), "/nologo", "/dumpbin", str(binary)],
                            capture_output=True, text=True, check=True)
    return result.stdout


def registers(assembly: str) -> dict[str, tuple[str, int, int]]:
    return {name: (kind, int(index), int(count)) for name, kind, index, count in
            re.findall(r"^//\s+(\w+)\s+([cs])(\d+)\s+(\d+)\s*$", assembly, re.M)}


def sampler_metadata(source: str) -> dict[str, tuple[str, str, str]]:
    result = {}
    for match in re.finditer(r"\bsampler(?:2D|3D|CUBE)?\s+(\w+)\s*"
                            r"(?::\s*register\s*\(\s*s\d+\s*\))?\s*=\s*"
                            r"sampler_state\s*\{([^{}]*)\}", source):
        values = {k.lower(): v.strip() for k, v in
                  re.findall(r"(\w+)\s*=\s*([^;]+);", match[2])}
        texture = values["texture"].strip("< >")
        filters = {values.get(k, "LINEAR").upper() for k in
                   ("minfilter", "magfilter", "mipfilter")}
        if len(filters) != 1 or next(iter(filters)) not in ("POINT", "LINEAR"):
            raise ValueError(f"unsupported sampler filter: {match[1]}")
        filter_name = "MIN_MAG_MIP_" + next(iter(filters))
        states = " ".join(f"Address{axis} = {values.get('address' + axis.lower(), 'WRAP')};"
                          for axis in "UVW")
        result[match[1]] = (texture, values.get("srgbtexture", "FALSE"),
                            f"Filter = {filter_name}; {states}")
    return result


def recover_shader(assembly: str, name: str,
                   metadata: dict[str, tuple[str, str, str]]) -> tuple[str, dict]:
    table = registers(assembly)
    constants = {i: (uniform, i - start) for uniform, (kind, start, count) in table.items()
                 if kind == "c" for i in range(start, start + count)}
    inputs, outputs, textures, instructions, literals = {}, {}, {}, [], {}
    stage = "VS" if "vs_3_0" in assembly else "PS"
    for line in assembly.splitlines():
        if not line.startswith("    "):
            continue
        opcode, _, arguments = line.strip().partition(" ")
        if opcode in ("vs_3_0", "ps_3_0"):
            continue
        args = [arg.strip() for arg in arguments.split(",")]
        if opcode == "def":
            literals[args[0]] = args[1:]
        elif opcode.startswith("dcl_"):
            semantic = opcode[4:]
            reg = args[0].split(".")[0]
            if reg.startswith("s"):
                uniform = next(n for n, (k, i, _) in table.items()
                               if k == "s" and i == int(reg[1:]))
                authored = uniform.removesuffix("3d")
                if uniform in metadata:
                    authored = uniform
                if authored not in metadata:
                    raise ValueError(f"missing authored sampler: {uniform}")
                textures[reg] = (uniform, semantic, metadata[authored])
            else:
                semantic = semantic.upper()
                if stage == "VS" and reg.startswith("o") and semantic == "POSITION":
                    semantic = "SV_Position"
                (inputs if reg.startswith("v") else outputs)[reg] = semantic
        else:
            instructions.append((opcode, args))
            for arg in args:
                if arg.startswith("oC"):
                    reg = arg.split(".")[0]
                    outputs[reg] = "SV_Target" + reg[2:]

    def source(arg: str) -> str:
        negative = arg.startswith("-")
        if negative:
            arg = arg[1:]
        absolute = "_abs" in arg
        arg = arg.replace("_abs", "")
        match = re.fullmatch(r"([rcvoa]\w*)(?:\[a0\.([xyzw])\])?(?:\.([xyzw]+))?", arg)
        if not match:
            raise ValueError(f"unsupported operand {arg}")
        reg, indirect, swizzle = match.groups()
        if reg.startswith("c"):
            if reg in literals:
                expression = "k" + reg[1:]
            else:
                if int(reg[1:]) not in constants:
                    raise ValueError(f"unmapped constant {reg}")
                uniform, offset = constants[int(reg[1:])]
                expression = f"{uniform}[{offset}" + (f" + a0.{indirect}" if indirect else "") + "]"
        else:
            expression = reg
        if swizzle:
            # All captured operands use scalar replication or four components.
            # Reject a new shorthand rather than guess its unused components.
            if len(swizzle) not in (1, 4):
                raise ValueError(f"unsupported shortened source swizzle: {arg}")
            expression += "." + (swizzle * 4 if len(swizzle) == 1 else swizzle)
        if absolute:
            expression = f"abs({expression})"
        return f"(-{expression})" if negative else expression

    # Use one complete interface in both stages, preserving SM3 TEXCOORD
    # register locations rather than repacking each shader's used subset.
    varying_names = {"SV_Position": "position", **{f"TEXCOORD{i}": f"uv{i}" for i in range(6)}, "TEXCOORD": "uv0"}
    if stage == "VS":
        lines = [f"struct {name}Input {{"]
        # Authored vehicle_common.h marks controlPointInfo as U8_4 (not
        # normalized). D3D9 converted those bytes to floats; DX11 needs an
        # integer IA signature followed by an explicit numerical conversion.
        lines += [f"    {'uint4' if s == 'TEXCOORD4' else 'float4'} {r} : {s};"
                  for r, s in inputs.items()]
        lines += ["};", f"RecoveredVertexOutput {name}({name}Input input) {{"]
        lines += [f"    float4 {r} = input.{r};" for r in inputs]
    else:
        lines = [f"struct {name}Output {{"]
        lines += [f"    float4 {r} : {s};" for r, s in outputs.items()]
        lines += ["};", f"{name}Output {name}(RecoveredVertexOutput input) {{"]
        lines += [f"    float4 {r} = input.{varying_names[s]};" for r, s in inputs.items()]
    lines += [f"    float4 {r} = 0;" for r in outputs]
    temp_regs = sorted(set(re.findall(r"\br\d+\b", "\n".join(
        " ".join(args) for _, args in instructions))))
    lines += [f"    float4 {r} = 0;" for r in temp_regs]
    if "a0" in assembly:
        lines.append("    int4 a0 = 0;")
    lines += [f"    const float4 k{r[1:]} = float4({', '.join(values)});"
              for r, values in literals.items()]
    for opcode, args in instructions:
        original_opcode = opcode
        saturate = "_sat" in opcode
        opcode = opcode.replace("_sat", "").replace("_pp", "")
        dst, *operands = args
        values = [source(arg) for arg in operands if not arg.startswith("s")]
        a, b, c = (values + [""] * 3)[:3]
        if opcode == "mov": expr = a
        elif opcode == "mova": expr = f"int4(round({a}))"
        elif opcode == "add": expr = f"({a} + {b})"
        elif opcode == "mul": expr = f"({a} * {b})"
        elif opcode == "mad": expr = f"({a} * {b} + {c})"
        elif opcode == "max": expr = f"max({a}, {b})"
        elif opcode == "lrp": expr = f"lerp({c}, {b}, {a})"
        elif opcode in ("dp3", "dp4"):
            mask = "xyz" if opcode == "dp3" else "xyzw"
            expr = f"(dot(({a}).{mask}, ({b}).{mask})).xxxx"
        elif opcode == "dp2add": expr = f"(dot(({a}).xy, ({b}).xy) + ({c}).x).xxxx"
        elif opcode == "nrm":
            if "." not in dst or "w" in dst.split(".")[1]:
                raise ValueError("SM3 nrm only defines xyz")
            expr = f"float4(normalize(({a}).xyz), 0)"
        elif opcode == "rcp": expr = f"(1.0 / ({a}).x).xxxx"
        elif opcode == "rsq": expr = f"(rsqrt(abs(({a}).x))).xxxx"
        elif opcode == "exp": expr = f"(exp2(({a}).x)).xxxx"
        elif opcode == "log": expr = f"(log2(abs(({a}).x))).xxxx"
        elif opcode == "frc": expr = f"frac({a})"
        elif opcode in ("texld", "texldl", "texldb"):
            sampler, dimension, meta = textures[operands[1]]
            mask = "xy" if dimension == "2d" else "xyz"
            method = {"texld": "Sample", "texldl": "SampleLevel", "texldb": "SampleBias"}[opcode]
            extra = f", ({a}).w" if opcode != "texld" else ""
            expr = f"{meta[0]}.{method}({sampler}, ({a}).{mask}{extra})"
        else:
            raise ValueError(f"unsupported instruction {opcode}")
        if saturate:
            expr = f"saturate({expr})"
        mask = dst.split(".")[1] if "." in dst else "xyzw"
        lines.append(f"    // {original_opcode} {', '.join(args)}")
        lines.append(f"    {dst} = ({expr}).{mask};")
    output_type = "RecoveredVertexOutput" if stage == "VS" else name + "Output"
    lines += [f"    {output_type} output = ({output_type})0;",
              *[f"    output.{varying_names[s] if stage == 'VS' else r} = {r};" for r, s in outputs.items()],
              "    return output;", "}"]
    return "\n".join(lines), textures


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--fxc", type=Path, required=True)
    parser.add_argument("--dumps", type=Path, required=True)
    parser.add_argument("--authored", type=Path, required=True,
                        help="directory containing preprocessed vehicle source effects")
    parser.add_argument("--out", type=Path, required=True)
    args = parser.parse_args()
    args.out.mkdir(parents=True, exist_ok=True)
    effects = sorted({re.match(r"(.+)_Material", p.name)[1]
                      for p in args.dumps.glob("*_Material*_0_vs.bin")})
    for effect in effects:
        metadata = sampler_metadata((args.authored / (effect + ".preprocessed.fx")).read_text())
        uniforms, samplers, bodies, passes, hashes = {}, {}, [], {}, {}
        evidence = []
        for binary in sorted(args.dumps.glob(effect + "_Material*_0_*.bin")):
            match = re.fullmatch(re.escape(effect) + r"_(Material\w*)_0_(vs|ps)\.bin", binary.name)
            technique, stage = match.groups()
            assembly = disassemble(args.fxc, binary)
            for uniform, (kind, _, count) in registers(assembly).items():
                if kind == "c": uniforms[uniform] = max(uniforms.get(uniform, 0), count)
            digest = hashlib.sha256(binary.read_bytes()).hexdigest()
            evidence.append(f"// {binary.name} SHA256 {digest}")
            if digest not in hashes:
                name = "Recovered" + technique + stage.upper()
                body, texture_info = recover_shader(assembly, name, metadata)
                bodies.append(body)
                hashes[digest] = name
                for sampler, dimension, meta in texture_info.values():
                    record = (dimension, meta)
                    if sampler in samplers and samplers[sampler] != record:
                        raise ValueError(f"conflicting sampler: {sampler}")
                    samplers[sampler] = record
            passes.setdefault(technique, {})[stage] = hashes[digest]
        lines = ["// [Blur1PCDX11][retail-recovered] Generated by recover_vehicle.py.",
                 "// CTAB names preserved; arithmetic follows the shipped SM3 instructions.",
                 "// Pass states remain the engine's retail FXB states; no approximated pass aliases.",
                 *evidence]
        lines += [f"float4 {name}[{count}];" for name, count in sorted(uniforms.items())]
        lines += ["struct RecoveredVertexOutput { float4 position : SV_Position; " +
                  " ".join(f"float4 uv{i} : TEXCOORD{i};" for i in range(6)) + " };"]
        resources = {}
        sidecar = []
        for sampler, (dimension, (texture, srgb, states)) in sorted(samplers.items()):
            typ = {"2d": "Texture2D", "volume": "Texture3D", "cube": "TextureCube"}[dimension]
            if texture in resources and resources[texture] != typ:
                raise ValueError(f"conflicting resource dimension: {texture}")
            resources[texture] = typ
            lines.append(f"SamplerState {sampler} {{ {states} }};")
            filter_name = re.search(r"Filter = (\w+)", states)[1]
            sidecar.append(f"{sampler}\t{texture}\t{int(srgb.upper() == 'TRUE')}\t{filter_name}\n")
        lines += [f"{typ}<float4> {texture};" for texture, typ in sorted(resources.items())]
        lines += bodies
        for technique, shaders in passes.items():
            if set(shaders) != {"vs", "ps"}:
                raise ValueError(f"incomplete pass: {effect}/{technique}")
            lines.append(f"technique11 {technique} {{ pass p0 {{ "
                         f"SetVertexShader(CompileShader(vs_5_0, {shaders['vs']}())); "
                         f"SetPixelShader(CompileShader(ps_5_0, {shaders['ps']}())); }} }}")
        (args.out / (effect + ".fx")).write_text("\n\n".join(lines) + "\n")
        (args.out / (effect + ".samplers")).write_text("".join(sidecar))
        print(f"{effect}: {len(passes)} techniques, {len(bodies)} distinct shaders")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
