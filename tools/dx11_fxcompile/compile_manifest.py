"""Compile the original Jam effect permutations into native FX5 bytecode."""

from __future__ import annotations

import argparse
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
from pathlib import Path
import re
import subprocess
import sys

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "dx11_shadercompile"))
from alpha_test import inject_effect_alpha_test


def extend_vehicle_hdr(source: str) -> str:
    # [Blur2PC][HDR] Only the recovered SH dp4_sat operations lose radiance.
    # Preserve the recovery files and all other retail saturated instructions.
    source, count = re.subn(r"saturate(?=\(\(dot\(\(g_LocalSpaceSHNode\[)",
                            "BlurHDRLighting", source)
    if count:
        source = ('// [Blur2PC][HDR] SH range extension; SDR retains retail dp4_sat.\n'
                  '#define BLUR_DX11 1\n#include <Shader/Common/HDRRange.h>\n' + source)
    return source


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--top", required=True, type=Path)
    parser.add_argument("--manifest", required=True, type=Path)
    parser.add_argument("--fxc", required=True, type=Path)
    parser.add_argument("--out", required=True, type=Path)
    parser.add_argument("--jobs", type=int, default=4)
    parser.add_argument("--skip-existing", action="store_true")
    args = parser.parse_args()

    top = args.top.resolve()
    output_root = args.out.resolve()
    entries = json.loads(args.manifest.read_text(encoding="utf-8"))["effects"]
    compiler = top / "tools/dx11_fxcompile/compile_fx.py"
    include_dirs = [
        top / "code/amax/game/Shaders",
        top / "code/mercury/library/graphics/shader/include",
        top / "code/mercury/library/graphics/shader/include/shader",
        top / "code/mercury/library/graphics/vehicle/include",
    ]
    # [Blur2PC] Effects also include other .fx files (for example MeshEffect.fx).
    # A wrapper's own timestamp cannot detect changes in those shared inputs.
    included_effect_mtime = max((effect.stat().st_mtime_ns for directory in include_dirs
                                 for effect in directory.rglob("*.fx")), default=0)
    # Incremental authored compilation must account for includes and technique
    # compiler changes. The index/metadata are still regenerated on every run.
    authored_inputs = [compiler, args.manifest,
                       top / "tools/dx11_shadercompile/alpha_test.py"] + [header for directory in include_dirs
                                                  for header in directory.rglob("*.h")]
    authored_mtime = max(path.stat().st_mtime_ns for path in authored_inputs)
    jobs: list[tuple[Path, Path, list[str]]] = []
    for entry in entries:
        source_relative = Path(entry["fx"])
        source = top / source_relative
        relative_output = source_relative.relative_to("Code")
        for variant in entry["variants"]:
            match = re.fullmatch(r"<([^>]*)>(.*)", variant)
            if not match:
                raise ValueError(f"unexpected Jam effect variant: {variant}")
            suffix, flags = match.groups()
            defines = re.findall(r"-D\s+([A-Za-z_]\w*)", flags)
            name = relative_output.stem + suffix + ".fxo"
            output = output_root / relative_output.parent / name
            jobs.append((source, output, defines))

    def compile_one(job: tuple[Path, Path, list[str]]) -> tuple[Path, int, str]:
        source, output, defines = job
        # [Blur1PCDX11] Exact retail-recovered shaders for the dumped variants
        # only. Do not apply a different material permutation as a fallback.
        recovered = (top / "code/mercury/library/graphics/vehicle/include/vehicle/shaders/retail_dx11"
                     / (output.stem + ".fx"))
        if recovered.is_file():
            output.parent.mkdir(parents=True, exist_ok=True)
            transformed = output.with_suffix(".retail.fx5.fx")
            transformed.write_text(inject_effect_alpha_test(extend_vehicle_hdr(
                recovered.read_text(encoding="utf-8"))), encoding="utf-8")
            command = [str(args.fxc), "/nologo", "/T", "fx_5_0", "/I",
                       str(top / "code/mercury/library/graphics/shader/include"),
                       "/Fo", str(output), str(transformed)]
            result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                    text=True, encoding="utf-8", errors="replace")
            if result.returncode == 0:
                output.with_suffix(".retail").write_text(str(recovered), encoding="utf-8")
                output.with_suffix(".techniques").write_text("", encoding="utf-8")
                output.with_suffix(".states").write_text("", encoding="utf-8")
                output.with_suffix(".samplers").write_text(
                    recovered.with_suffix(".samplers").read_text(), encoding="utf-8")
            else:
                output.with_suffix(".log").write_text(result.stdout, encoding="utf-8")
            return output, result.returncode, result.stdout
        sidecars = [output.with_suffix(suffix) for suffix in (".techniques", ".samplers", ".states")]
        if (args.skip_existing and not output.with_suffix(".retail").exists()
                and output.is_file() and all(path.is_file() for path in sidecars)
                and output.stat().st_mtime_ns >= max(authored_mtime, included_effect_mtime, source.stat().st_mtime_ns)):
            return output, 0, ""
        command = [sys.executable, str(compiler), "--fxc", str(args.fxc),
                   "--source", str(source), "--out", str(output)]
        for directory in include_dirs:
            command.extend(("--include", str(directory)))
        for definition in defines:
            command.extend(("--define", definition))
        result = subprocess.run(command, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                text=True, encoding="utf-8", errors="replace")
        if result.returncode:
            output.parent.mkdir(parents=True, exist_ok=True)
            output.with_suffix(".log").write_text(result.stdout, encoding="utf-8")
        elif output.with_suffix(".retail").exists():
            output.with_suffix(".retail").unlink()
        return output, result.returncode, result.stdout

    succeeded = 0
    failures: list[tuple[Path, str]] = []
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        futures = [pool.submit(compile_one, job) for job in jobs]
        for future in as_completed(futures):
            output, code, log = future.result()
            if code == 0:
                succeeded += 1
            else:
                line = next((line.strip() for line in log.splitlines()
                             if "error X" in line or "Unrecognized state" in line or
                             "failed to open" in line), "compiler exited without a diagnostic")
                failures.append((output, line))
    print(f"FX11 effects: {succeeded}/{len(jobs)} compiled, {len(failures)} failed")
    for output, line in sorted(failures)[:40]:
        print(f"{output.relative_to(output_root)}: {line}")
    if not failures:
        # The game asks for a compiled effect by its .fx filename. Keep the
        # original Jam output names and resolve them to their authored source
        # tree without depending on the directory order at runtime.
        index: dict[str, Path] = {}
        for _, output, _ in jobs:
            key = output.stem.lower()
            if key in index:
                raise ValueError(f"ambiguous game effect variant: {output.stem}")
            index[key] = output.relative_to(output_root)
            for line in output.with_suffix(".techniques").read_text(encoding="utf-8").splitlines():
                technique, filename = line.split("\t")
                index[key + "@" + technique.lower()] = (output.parent / filename).relative_to(output_root)
        (output_root / "effects.index").write_text(
            "".join(f"{key}\t{path.as_posix()}\n" for key, path in sorted(index.items())),
            encoding="utf-8",
        )
        recovered_keys = [output.stem.lower() for _, output, _ in jobs
                          if output.with_suffix(".retail").is_file()]
        (output_root / "effects.recovered").write_text("\n".join(sorted(recovered_keys)) + "\n",
                                                        encoding="utf-8")
        samplers: list[str] = []
        states: list[str] = []
        for key, relative in index.items():
            output = output_root / relative
            for line in output.with_suffix(".samplers").read_text(encoding="utf-8").splitlines():
                samplers.append(f"{key}\t{line}\n")
            for line in output.with_suffix(".states").read_text(encoding="utf-8").splitlines():
                states.append(f"{key}\t{line}\n")
        (output_root / "effects.samplers").write_text("".join(samplers), encoding="utf-8")
        (output_root / "effects.states").write_text("".join(states), encoding="utf-8")
    return 0 if not failures else 1


if __name__ == "__main__":
    raise SystemExit(main())
