## Building

The build is CMake generated from the original jamfiles (decision D08). Each phase builds from its own worktree with the same commands (`tools/build.sh` runs them all), in order:

```
python tools/jam2cmake/jamextract.py --top . --root code/amax/game/jamfile.jam --target WIN32 --platform PC --build Speed --out _work/build/targets_pc_speed.json
python tools/jam2cmake/cpgen.py $(find code/mercury/library code/amax/game -iname "*.cp" -o -iname "*.cv" -o -iname "*.sp")
python tools/jam2cmake/shadergen.py --json _work/build/targets_pc_speed.json --out _work/build/gen/shaders
python tools/jam2cmake/luabundle.py --json _work/build/targets_pc_speed.json --out _work/build/gen/blobs/amax_lua_bundle.cpp
python tools/jam2cmake/gen_cmake.py --json _work/build/targets_pc_speed.json --out cmake/targets.cmake
cmake -S . -B _work/build/vs -G "Visual Studio 18 2026" -A Win32
cmake --build _work/build/vs --config Release --target Amax -- -m -v:m -nologo > _work/build/amax.log 2>&1
python tools/jam2cmake/errsum.py _work/build/amax.log
```

Run `gen_cmake.py` again after adding or removing sources, including after a merge that does, and commit the regenerated `cmake/targets.cmake`. `cpgen.py`'s headers land beside the `.cp` files and are gitignored, so it runs in every worktree.

- `cmake` lives at `C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe`; MSBuild prints its summary in the system locale (Spanish), so judge builds by `errsum.py`'s error count.
- Configurations map to the engine's build configs: Debug → `BUILD_CONFIG_DEBUG`, Release → `BUILD_CONFIG_SPEED`, RelWithDebInfo → `BUILD_CONFIG_RELEASE`, MinSizeRel → `BUILD_CONFIG_MASTER`.
- Jam's `[ Batch … : StdAfx.h ]` groups were **unity translation units**; the generator reproduces them as `_work/build/gen/unity/<target>_batchN.cpp`. A file that fails alone but built originally is almost always relying on an earlier file in its group. Run `gen_cmake.py` again after adding or removing sources.
- `cmake/targets.cmake` is generated; never edit it. Per-file or per-target tweaks go in `cmake/overrides.cmake` (on `blur1pc`, so every phase inherits it) with a comment saying why; the shared flags, vendor SDKs and shims are in `cmake/blur_common.cmake`. Source edits on `blur1pc` get a `// [Blur2PC]` comment.
- Third-party inputs live in `Blur1PC/_work/vendor` (D3DX 9.29 from the `Microsoft.DXSDK.D3DX` NuGet, OpenAL Soft 1.24.3 binaries, IJG libjpeg 6b); `blur_common.cmake` has cache variables for each and the other phases resolve them there. Shims for pieces we do not have: `code/vendor/dxerr` (DXGetErrorString) and `code/vendor/efx-util` (Creative's reverb-preset conversion).
- Built-in shaders (`.psh`/`.vsh`, 647 variants) are compiled by `shadergen.py` with the Windows SDK `fxc` into `_work/build/gen/shaders/<target>_shaders.cpp` in the blob layout `C_D3D9Device::CreatePixelShader` reads. Re-run it after editing any of them.

What the tree shipped with, for reference:

- `code/amax/game/amax.sln` is a VS2008 (vcproj 9.00) **makefile project**. Its build command is `z:\tools\common_bin\jam -sTARGET=WIN32 $(JAMOPTS) -sBUILD=<Debug|Release|Speed|Profile|Master>`, output `bin\Win32\<Build>\Amax.exe`. Targets: WIN32.
- Jam sources are in `code/tools/jam` and `code/tools/jamplus`, but the custom rules the jamfiles call (`SupportedTargets`, `IncludePath`, `Define`, `CheckXDKVersion`, `LINKLIBS`, `PCLIBS`) are **not in the tree**; they lived in Bizarre's Jambase.
- Required externals that are **missing on this machine**: `$(TOP)/libs/vendor` (Bink 1.9r, OpenAL 1.1, Massive ad client), `$(DXSDK_PATH)` (DirectX SDK: d3dx9, dinput8, xinput, dxerr), VS2008. Demonware libs for win32-vs8 are present under `code/vendor/demonware`.
- Source lists and defines per target are authoritative in the jamfiles: `code/amax/game/jamfile.jam` (plus `graphics.jam`, `RaceLogic.jam`, `air_amax.jam`, `demonware.jam`) and `code/mercury/library/jamfile.jam`. `tools/jam2cmake/jamextract.py` evaluates them (see `docs/analysis.md` §1 for the command) and writes `_work/build/targets_pc_<build>.json`: 70 targets, 2,449 sources. Generate build files from that JSON; do not hand-maintain source lists.
- The build must define exactly one of `BUILD_CONFIG_DEBUG | RELEASE | PROFILE | SPEED | MASTER`; `core/Core.h` derives the `CFG_*` macros and `PLATFORM_PC` from it and `_WIN32`.
- PC preprocessor identity: `PLATFORM_PC` (the vcproj adds `_MSC_VER=1400`). Graphics is forced to D3D9 + HLSL in `code/mercury/library/graphics/display/include/display/device/DeviceDefines.h`.
- Tests: cxxtest in `code/vendor/cxxtest`; engine test sections in `code/mercury/library/gamesys/testsections` (`TestSectionFs.cpp` shows how pak, memory and overlay mounts combine). No test runner is wired yet.
