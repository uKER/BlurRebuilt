"""Compare shipped PC effect shaders with the same passes compiled from source.

[Blur2PC][Blur1PCDX11] The leaked shader source is older than the shipped
FXBs in places (vehicle techniques, the prepass alpha clip). This finds every
such pass. Inputs:

  * retail pass microcode dumped by the game with BLUR_DX11_DUMP_FX=* into
    <dump>/<fxb basename>_<technique>_<pass>_<ps|vs>.bin;
  * the Jam effect manifest (variant defines) and the shader source tree.

Each pass is preprocessed with the PC exporter's defines (Fx_export.cpp:
platform, Jam variant and technique Define annotations), compiled with the
D3D9-era compiler by legacy_compile.exe, and both sides are disassembled by
the same fxc. Output: <work>/report.txt and per-pass .asm files.
"""

import argparse
import collections
import json
import re
import subprocess
import sys
from concurrent.futures import ThreadPoolExecutor
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "dx11_fxcompile"))
from compile_fx import annotation_defines, technique_blocks  # noqa: E402


def instructions(assembly: str) -> list[str]:
    result = []
    for line in assembly.splitlines():
        line = line.strip()
        if not line or line.startswith("//"):
            continue
        result.append(re.sub(r"\s+", " ", line))
    return result


def pass_entries(technique_source: str) -> list[dict[str, tuple[str, str]]]:
    passes = []
    for match in re.finditer(r"\bpass\s*\w*\s*(<[^>]*>)?\s*\{", technique_source):
        depth, end = 1, match.end()
        while depth and end < len(technique_source):
            depth += (technique_source[end] == "{") - (technique_source[end] == "}")
            end += 1
        body = technique_source[match.end():end]
        entries = {}
        for kind, profile, entry in re.findall(
                r"\b(VertexShader|PixelShader)\s*=\s*compile\s+(\w+)\s+(\w+)\s*\(\s*\)", body):
            entries["vs" if kind == "VertexShader" else "ps"] = (profile, entry)
        passes.append(entries)
    return passes


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--top", required=True, type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--dump", required=True, type=Path)
    parser.add_argument("--work", required=True, type=Path)
    parser.add_argument("--fxc", required=True, type=Path)
    parser.add_argument("--legacy", required=True, type=Path, help="legacy_compile.exe")
    parser.add_argument("--exclude-prefix", action="append", default=["veh_"])
    parser.add_argument("--define", action="append", default=[],
                        help="extra define, e.g. BLUR_DX11 to audit the native source")
    args = parser.parse_args()

    top = args.top.resolve()
    work = args.work.resolve()
    for name in ("pre", "src", "asm"):
        (work / name).mkdir(parents=True, exist_ok=True)
    include_dirs = [top / "code/amax/game/Shaders",
                    top / "code/mercury/library/graphics/shader/include",
                    top / "code/mercury/library/graphics/shader/include/shader",
                    top / "code/mercury/library/graphics/vehicle/include"]

    variants: dict[str, tuple[Path, list[str]]] = {}
    for entry in json.loads(args.manifest.read_text(encoding="utf-8"))["effects"]:
        source = top / entry["fx"]
        for variant in entry["variants"]:
            suffix, flags = re.fullmatch(r"<([^>]*)>(.*)", variant).groups()
            variants[(Path(entry["fx"]).stem + suffix).lower()] = (
                source, re.findall(r"-D\s+([A-Za-z_]\w*)", flags))

    retail = {}
    unmatched = []
    for binary in sorted(args.dump.glob("*.bin")):
        if any(binary.name.startswith(prefix) for prefix in args.exclude_prefix):
            continue
        stem = binary.stem
        match = re.fullmatch(r"(.+)_(\d+)_(ps|vs)", stem)
        head, pass_index, kind = match[1], int(match[2]), match[3]
        variant = next((head[:i] for i in range(len(head), 0, -1)
                        if head[i:i + 1] == "_" and head[:i].lower() in variants), None)
        if not variant:
            unmatched.append(binary.name)
            continue
        retail[(variant.lower(), head[len(variant) + 1:], pass_index, kind)] = binary

    def run_preprocessor(source: Path, output: Path, defines: list[str]) -> subprocess.CompletedProcess:
        command = [str(args.fxc), "/nologo", "/P", str(output)]
        for directory in include_dirs:
            command += ["/I", str(directory)]
        for definition in dict.fromkeys(defines + args.define):
            command.append(f"/D{definition}=1")
        command += ["/DPLATFORM_PC=1", "/DBUILD_CONFIG_SPEED=1", str(source)]
        return subprocess.run(command, capture_output=True, text=True)

    def strip_line_directives(text: str) -> str:
        # fxc /P can leave #line between a technique's annotations and its body.
        return re.sub(r"(?m)^\s*#line.*$", "", text)

    def preprocess(key: tuple[str, str]) -> tuple[tuple[str, str], Path | None, str]:
        variant, technique = key
        source, defines = variants[variant]
        output = work / "pre" / f"{variant}__{technique}.fx"
        # Techniques can come from an included effect (MeshEffectUVDistort.fx
        # includes MeshEffect.fx), so read annotations after preprocessing,
        # as compile_fx.py does.
        base = work / "pre" / f"{variant}__{technique}.base.fx"
        result = run_preprocessor(source, base, defines)
        if result.returncode:
            return key, None, result.stdout + result.stderr
        text = strip_line_directives(base.read_text(encoding="utf-8-sig", errors="replace"))
        blocks = {name: annotations for name, annotations, _, _ in technique_blocks(text)}
        result = run_preprocessor(source, output, defines + annotation_defines(blocks.get(technique, "")))
        return key, (output if result.returncode == 0 else None), result.stdout + result.stderr

    keys = sorted({(variant, technique) for variant, technique, _, _ in retail})
    with ThreadPoolExecutor(8) as pool:
        preprocessed = {key: (path, log) for key, path, log in pool.map(preprocess, keys)}

    jobs, status = [], {}
    for key, binary in sorted(retail.items()):
        variant, technique, pass_index, kind = key
        path, log = preprocessed[(variant, technique)]
        if not path:
            status[key] = "preprocess failed: " + log.strip().splitlines()[-1][:160]
            continue
        text = strip_line_directives(path.read_text(encoding="utf-8-sig", errors="replace"))
        block = next((b for b in technique_blocks(text) if b[0] == technique), None)
        if not block:
            status[key] = "technique absent from source"
            continue
        passes = pass_entries(text[block[2]:block[3]])
        if pass_index >= len(passes) or kind not in passes[pass_index]:
            status[key] = "pass/shader absent from source"
            continue
        profile, entry = passes[pass_index][kind]
        output = work / "src" / binary.name
        jobs.append(f"{path}\t{entry}\t{profile}\t{output}\n")
        status[key] = (profile, entry, output)
    (work / "jobs.txt").write_text("".join(jobs), encoding="utf-8")
    subprocess.run([str(args.legacy), str(work / "jobs.txt")])

    def disassemble(binary: Path) -> list[str]:
        target = work / "asm" / (binary.parent.name + "_" + binary.name + ".asm")
        if not target.exists() or target.stat().st_mtime < binary.stat().st_mtime:
            result = subprocess.run([str(args.fxc), "/nologo", "/dumpbin", str(binary)],
                                    capture_output=True, text=True)
            target.write_text(result.stdout, encoding="utf-8")
        return instructions(target.read_text(encoding="utf-8"))

    def compare(item):
        key, binary = item
        state = status[key]
        if isinstance(state, str):
            return key, state, None
        output = state[2]
        if not output.exists():
            return key, "source compile failed", None
        shipped, rebuilt = disassemble(binary), disassemble(output)
        if shipped == rebuilt:
            return key, "identical", None
        a = collections.Counter(line.split(" ")[0] for line in shipped)
        b = collections.Counter(line.split(" ")[0] for line in rebuilt)
        detail = {"retail": len(shipped), "source": len(rebuilt),
                  "retail_only": dict(a - b), "source_only": dict(b - a)}
        return key, ("same opcodes" if a == b else "different"), detail

    with ThreadPoolExecutor(8) as pool:
        results = list(pool.map(compare, sorted(retail.items())))

    counts = collections.Counter(state for _, state, _ in results)
    lines = [f"retail passes: {len(retail)}; unmatched dump files: {len(unmatched)}\n"]
    lines += [f"{count:6} {state}\n" for state, count in counts.most_common()]
    for key, state, detail in results:
        if state not in ("identical", "same opcodes"):
            lines.append(f"{state}\t{key[0]}\t{key[1]}\t{key[2]}\t{key[3]}\t{json.dumps(detail) if detail else ''}\n")
    for name in unmatched:
        lines.append(f"unmatched\t{name}\n")
    (work / "report.txt").write_text("".join(lines), encoding="utf-8")
    print("".join(lines[:1 + len(counts)]), end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
