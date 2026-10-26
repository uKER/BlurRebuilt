#!/bin/sh
# [Blur2PC] Regenerate from the jamfiles and build Release Amax for the phase
# whose worktree this is run from (every phase branch inherits this script;
# the commands are AGENTS.md "Building"). Judge the result by errsum's error
# count, then copy _work/build/vs/Release/Amax.exe and .pdb to _work/run
# (AGENTS.md protocol rule 9) once the running exe, if any, has exited.
set -e
cd "$(dirname "$0")/.."
J=tools/jam2cmake
CM="/c/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
mkdir -p _work/build
python $J/jamextract.py --top . --root code/amax/game/jamfile.jam --target WIN32 --platform PC --build Speed --out _work/build/targets_pc_speed.json | tail -1
python $J/cpgen.py $(find code/mercury/library code/amax/game -iname "*.cp" -o -iname "*.cv" -o -iname "*.sp") 2>/dev/null | tail -1
python $J/shadergen.py --json _work/build/targets_pc_speed.json --out _work/build/gen/shaders | tail -1
python $J/luabundle.py --json _work/build/targets_pc_speed.json --out _work/build/gen/blobs/amax_lua_bundle.cpp | tail -1
python $J/gen_cmake.py --json _work/build/targets_pc_speed.json --out cmake/targets.cmake | tail -1
"$CM" -S . -B _work/build/vs -G "Visual Studio 18 2026" -A Win32 | tail -1
"$CM" --build _work/build/vs --config Release --target Amax -- -m -v:m -nologo > _work/build/amax.log 2>&1 || true
python $J/errsum.py _work/build/amax.log | head -6
