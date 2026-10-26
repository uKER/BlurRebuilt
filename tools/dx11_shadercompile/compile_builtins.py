"""Compile the Jam-selected built-in PC shaders as native Shader Model 5 programs."""

from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
from pathlib import Path
import re
import subprocess

from alpha_test import inject_alpha_test


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--top", required=True, type=Path)
    parser.add_argument("--fxc", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--jobs", type=int, default=4)
    args = parser.parse_args()

    top = args.top.resolve()
    output_root = args.out.resolve()
    targets = json.loads(args.manifest.read_text(encoding="utf-8"))["targets"]
    work = []
    for target in targets:
        includes = [top / entry["path"] for entry in target["includes"]
                    if entry["exists"]]
        for shader in target["shaders"]:
            source = top / shader["file"]
            stage = "ps" if source.name.lower().startswith("ps") else "vs"
            output = output_root / target["name"] / f"{shader['symbol']}.cso"
            work.append((target["name"], shader, source, stage, output,
                         [source.parent, source.parent.parent, *includes]))

    def compile_one(item):
        target, shader, source, stage, output, includes = item
        output.parent.mkdir(parents=True, exist_ok=True)
        preprocessed = output.with_suffix(".preprocessed.hlsl")
        transformed = output.with_suffix(".dx11.hlsl")
        command = [str(args.fxc), "/nologo", "/P", str(preprocessed),
                   "/DBLUR2PC=1", "/DBLUR_DX11=1"]
        for definition in shader["defines"]:
            command.append(f"/D{definition}=1")
        for include in includes:
            command.extend(("/I", str(include)))
        command.append(str(source))
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, encoding="utf-8", errors="replace")
        if not result.returncode:
            text = preprocessed.read_text(encoding="utf-8-sig").rstrip("\0")
            # Preserve the engine's D3D9 constant-register offsets in b0.
            # The native device writes those offsets directly into its SM5
            # constant buffer, so automatic packing puts matrices and clip
            # vectors at the wrong positions.
            constant = re.compile(
                r"^[ \t]*(?P<declaration>(?:float|half|int|bool)\w*\s+\w+"
                r"(?:\s*\[\s*\d+\s*\])?)\s*:\s*register\s*"
                r"\(\s*c(?P<register>\d+)\s*\)\s*;[ \t]*$",
                re.IGNORECASE | re.MULTILINE,
            )
            declarations = []
            def move_constant(match):
                declaration = match.group("declaration")
                register = int(match.group("register"))
                matrix = re.match(r"(?:float|half|int|bool)([1-4])x([1-4])", declaration)
                array = re.search(r"\[\s*(\d+)\s*\]", declaration)
                span = (int(matrix.group(1)) if matrix else 1) * (
                    int(array.group(1)) if array else 1)
                declarations.append((declaration, register, span))
                return ""
            text = constant.sub(move_constant, text)
            if declarations:
                groups = []
                for declaration, register, span in declarations:
                    occupied = set(range(register, register + span))
                    for group in groups:
                        if not occupied.intersection(group[0]):
                            group[0].update(occupied)
                            group[1].append((declaration, register))
                            break
                    else:
                        groups.append((occupied, [(declaration, register)]))
                buffers = []
                for index, (_, members) in enumerate(groups):
                    buffers.append(
                        f"cbuffer LegacyConstants{index} : register(b{index}) {{\n" +
                        "\n".join(f"    {declaration} : packoffset(c{register});"
                                  for declaration, register in members) + "\n};\n"
                    )
                text = "".join(buffers) + text
            if stage == "ps":
                text = inject_alpha_test(text, {"main"})
            transformed.write_text(text, encoding="utf-8")
            result = subprocess.run(
                [str(args.fxc), "/nologo", "/T", f"{stage}_5_0", "/E", "main",
                 "/O3", "/Zpr", "/Gec", "/Fo", str(output), str(transformed)],
                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True,
                encoding="utf-8", errors="replace",
            )
        if result.returncode:
            output.with_suffix(".log").write_text(result.stdout, encoding="utf-8")
        return target, shader["symbol"], source, output, result

    failures = []
    succeeded = 0
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for future in as_completed([pool.submit(compile_one, item) for item in work]):
            target, symbol, source, output, result = future.result()
            if result.returncode:
                failures.append({"target": target, "symbol": symbol,
                                 "source": str(source.relative_to(top)),
                                 "log": str(output.with_suffix(".log").relative_to(output_root))})
            else:
                succeeded += 1
    (output_root / "report.json").write_text(json.dumps({
        "succeeded": succeeded, "total": len(work), "failures": sorted(
            failures, key=lambda item: (item["target"], item["symbol"]))
    }, indent=2), encoding="utf-8")
    print(f"DX11 built-in shaders: {succeeded}/{len(work)} compiled, "
          f"{len(failures)} failed")
    for item in failures[:20]:
        log = output_root / item["log"]
        first = next((line.strip() for line in log.read_text(encoding="utf-8").splitlines()
                      if "error X" in line), "compiler failed without an error line")
        print(f"{item['symbol']}: {first}")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
