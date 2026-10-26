#!/bin/sh
# [Blur2PC] Rebuild Release Amax in the existing _work/build/vs tree without
# compiling any shaders: no jam/shader generators run, and the tree is
# configured with BLUR_BUILD_SHADERS=OFF so the build reuses the DX11 effects
# and built-in blobs already under _work/build/gen. The tree is put back to
# BLUR_BUILD_SHADERS=ON afterwards, so the next full or IDE build compiles any
# shader whose inputs changed. Needs one earlier full build (tools/build.sh).
# Source list changes still need the full build (gen_cmake.py).
# Judge the result by errsum's error count, as for tools/build.sh.
set -e
cd "$(dirname "$0")/.."
J=tools/jam2cmake
CM="/c/Program Files/Microsoft Visual Studio/18/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe"
test -f _work/build/vs/CMakeCache.txt || { echo "no configured _work/build/vs; run tools/build.sh first"; exit 1; }
# No -A: reconfiguring keeps the tree's own platform (Win32 or x64).
"$CM" -S . -B _work/build/vs -DBLUR_BUILD_SHADERS=OFF | tail -1
"$CM" --build _work/build/vs --config Release --target Amax -- -m -v:m -nologo > _work/build/amax.log 2>&1 || true
"$CM" -S . -B _work/build/vs -DBLUR_BUILD_SHADERS=ON | tail -1
python $J/errsum.py _work/build/amax.log | head -6
