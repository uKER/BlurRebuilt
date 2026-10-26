"""Compile a Blur PC effect source into native Direct3D 11 FX5 bytecode.

The original effect files use FX9's ``shared`` globals and SM3 compile
profiles. FX5 requires those globals in ordinary constant buffers and SM5
profiles. This tool transforms only the generated preprocessed source; it
never edits the authored effect files or retail data.
"""

from __future__ import annotations

import argparse
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "dx11_shadercompile"))
from alpha_test import inject_effect_alpha_test


def technique_blocks(source: str) -> list[tuple[str, str, int, int]]:
    """Return technique names, annotations and balanced source spans."""
    result = []
    for match in re.finditer(r"\btechnique\s+(\w+)\s*(<[^>]*>)?\s*\{", source):
        depth, end = 1, match.end()
        while depth and end < len(source):
            depth += (source[end] == "{") - (source[end] == "}")
            end += 1
        if depth:
            raise ValueError(f"unterminated technique: {match[1]}")
        result.append((match[1], match[2] or "", match.start(), end))
    return result


def annotation_defines(annotations: str) -> list[str]:
    result = []
    for match in re.finditer(r'\bstring\s+Define\s*=\s*"([^"]*)"', annotations,
                             re.IGNORECASE):
        result.extend(value.strip() for value in match[1].split(";") if value.strip())
    for value in result:
        if not re.fullmatch(r"[A-Za-z_]\w*", value):
            raise ValueError(f"unsupported technique define: {value}")
    return result


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--fxc", required=True, type=Path)
    parser.add_argument("--source", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--include", action="append", default=[], type=Path)
    parser.add_argument("--define", action="append", default=[])
    parser.add_argument("--technique", help="Compile one technique with its annotation defines")
    args = parser.parse_args()

    output = args.out.resolve()
    output.parent.mkdir(parents=True, exist_ok=True)
    preprocessed = output.with_suffix(".preprocessed.fx")
    transformed = output.with_suffix(".fx5.fx")
    command = [str(args.fxc), "/nologo", "/P", str(preprocessed)]
    for directory in args.include:
        command.extend(("/I", str(directory.resolve())))
    for definition in args.define:
        if not re.fullmatch(r"[A-Za-z_]\w*", definition):
            parser.error(f"invalid effect define: {definition}")
        command.append(f"/D{definition}=1")
    command.extend(("/DPLATFORM_PC=1", "/DBLUR_DX11=1", "/DBUILD_CONFIG_SPEED=1",
                    "/Dvs_3_0=vs_5_0", "/Dps_3_0=ps_5_0",
                    str(args.source.resolve())))
    if subprocess.run(command).returncode != 0:
        return 1

    source = preprocessed.read_text(encoding="utf-8-sig").rstrip("\0")
    techniques = technique_blocks(source)
    if args.technique:
        selected = next((block for block in techniques if block[0] == args.technique), None)
        if not selected:
            raise ValueError(f"technique {args.technique} disappeared under its defines")
        # Other techniques can use functions/structures disabled by these defines.
        for name, _, start, end in reversed(techniques):
            if name != args.technique:
                source = source[:start] + source[end:]
    else:
        # [Blur1PCDX11] Fx_export.cpp:WriteEffect compiles each technique with
        # GenerateAnnotationCmdLineParams. FX11 does not interpret Define annotations.
        # Keep separate effects so differing HLSL types and uniform layouts cannot collide.
        permutations = []
        for name, annotations, _, _ in techniques:
            defines = annotation_defines(annotations)
            if not defines:
                continue
            variant = output.with_name(output.stem + "__" + name + output.suffix)
            child = [sys.executable, str(Path(__file__).resolve()),
                     "--fxc", str(args.fxc), "--source", str(args.source),
                     "--out", str(variant), "--technique", name]
            for directory in args.include:
                child.extend(("--include", str(directory)))
            for definition in dict.fromkeys(args.define + defines):
                child.extend(("--define", definition))
            if subprocess.run(child).returncode != 0:
                return 1
            permutations.append(f"{name}\t{variant.name}\n")
        output.with_suffix(".techniques").write_text("".join(permutations), encoding="utf-8")
    source = re.sub(r"\bshared\s+", "", source)
    # FX9 binds independent vertex/pixel c-register files. FX5 packs global
    # values into constant buffers, so the old explicit register assignments
    # collide. The native renderer addresses parameters by reflected name.
    # [Blur2PC][HDR] Native buffers (HDR b12, alpha test b13) must keep
    # their device binding; only legacy FX9 c/s register files are repacked.
    source = re.sub(r":\s*register\s*\(([^)]*)\)",
                    lambda match: match[0] if re.fullmatch(r"b\d+", match[1].strip()) else "", source)
    # FX5 ignores the original FX9 min/mag/mip sampler properties. The game
    # uses either point for all three or linear for all three in every sampler.
    # The old sRGB flag belongs on the DX11 texture view, so keep a sidecar
    # recording it for the native texture binding path.
    sampler_records: list[str] = []
    sampler_block = re.compile(
        r"\bsampler(?:2D|3D|CUBE)?\s+([A-Za-z_]\w*)\s*=\s*sampler_state\s*\{([^{}]*)\}",
        re.DOTALL,
    )

    def convert_sampler(match: re.Match[str]) -> str:
        body = match[2]
        values = {key.lower(): value.strip() for key, value in re.findall(
            r"(?m)^\s*([A-Za-z_]\w*)\s*=\s*([^;]+);", body)}
        filters = {values.get(key, "LINEAR").upper()
                   for key in ("minfilter", "magfilter", "mipfilter")}
        if len(filters) != 1 or next(iter(filters)) not in ("POINT", "LINEAR"):
            raise ValueError(f"sampler filter needs native port: {match[1]} in {args.source}")
        filter_name = "MIN_MAG_MIP_" + next(iter(filters))
        texture = values.get("texture", "").strip("< >")
        sampler_records.append(
            f"{match[1]}\t{texture}\t{int(values.get('srgbtexture', 'FALSE').upper() == 'TRUE')}"
            f"\t{filter_name}\n"
        )
        body = re.sub(r"(?m)^\s*(?:MinFilter|MagFilter|MipFilter|SRGBTEXTURE)\s*=\s*[^;]+;", "", body)
        # FX9 defaults to wrapping texture coordinates. FX11's legacy sampler
        # syntax defaults to CLAMP, so preserve each omitted PC address mode.
        for axis in "UVW":
            if "address" + axis.lower() not in values:
                body += f"\nAddress{axis} = WRAP;"
        sampler_type = match[0].split()[0]
        return (f"{sampler_type} {match[1]} = sampler_state {{{body}\n"
                f"Filter = {filter_name};\n}}")

    source = sampler_block.sub(convert_sampler, source)
    output.with_suffix(".samplers").write_text("".join(sampler_records), encoding="utf-8")
    source = re.sub(r"\btechnique\b", "technique11", source)
    shader_assignment = re.compile(
        r"\b(Vertex|Pixel)Shader\s*=\s*compile\s+(vs_5_0|ps_5_0)\s+"
        r"([A-Za-z_]\w*)\s*\(\s*\)\s*;"
    )
    source = shader_assignment.sub(
        lambda match: f"Set{match[1]}Shader(CompileShader({match[2]}, {match[3]}()));",
        source,
    )
    # [Blur1PCDX11] PCFX imports pass state into the engine's FXB state manager.
    # Context::SetRenderState honours render flags; the render stage owns depth,
    # stencil and write masks. FX11 must only bind shaders/resources here.
    # Baking incomplete state objects resets reverse depth and stencil (Puddle,
    # MeshEffect), and defeats the engine's blend/cull overrides. Keep authored
    # assignments as diagnostics, not a second source of GPU render state.
    # Alpha test is implemented by injecting a pixel shader discard below.
    pass_block = re.compile(r"\bpass\s+([A-Za-z_]\w*)\s*\{([^{}]*)\}", re.DOTALL)
    state_assignment = re.compile(r"(?m)^\s*([A-Za-z_]\w*)\s*=\s*([^;]+);\s*$")
    technique_names = [(m.start(), m[1]) for m in re.finditer(
        r"\btechnique11\s+([A-Za-z_]\w*)", source)]
    state_records: list[str] = []

    def convert_pass(match: re.Match[str]) -> str:
        original = match[2]
        states = {key: value.strip() for key, value in state_assignment.findall(original)}
        technique = next((name for pos, name in reversed(technique_names)
                          if pos < match.start()), "")
        for key, value in states.items():
            state_records.append(f"{technique}\t{match[1]}\t{key}\t{value}\n")
        return f"pass {match[1]} {{{state_assignment.sub('', original)}\n}}"

    source = pass_block.sub(convert_pass, source)
    source = inject_effect_alpha_test(source)
    output.with_suffix(".states").write_text("".join(state_records), encoding="utf-8")
    transformed.write_text(source, encoding="utf-8")
    return subprocess.run(
        [str(args.fxc), "/nologo", "/Gec", "/Zpr", "/T", "fx_5_0", "/Fo", str(output),
         str(transformed)],
    ).returncode


if __name__ == "__main__":
    raise SystemExit(main())
