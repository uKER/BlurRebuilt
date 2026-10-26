# Ledger (Blur1PCDX11)

Append-only log of sessions and agent runs, newest at the bottom. Every session ends with an entry. Keep facts in `analysis.md` and decisions in `decisions.md`; the ledger says what was done, how it was checked, and what comes next.

Entry template:

```
## YYYY-MM-DD HH:MM — <session or agent name> — <task IDs>
Did: …
Verified: … (how; command or file diffed)
Blocked: … (or none)
Next: … (exact next step for whoever picks this up)
Files touched: …
```

---

## 2026-09-26 19:00 — session Codex DX11 — T127

Did: created `blur1pcdx11` at `Blur1PCDX11/` from `blur1pc` HEAD `547ccfb1`. Added an isolated native D3D11 device module and offscreen `dx11_probe` target, plus conditional SM5 variants of two existing display shaders. The device creates a feature level 11 hardware or WARP context, compiles the repository shaders with nested includes, draws, and reads back. Added branch README and recorded the display/shader boundary in analysis.md. The Amax backend selection and game rendering remain D3D9.

Verified: the probe returned center RGBA `(125,67,63,255)` and corner `(5,10,20,255)` on hardware and WARP. D3D11 debug validation found and helped fix a vertex/pixel interpolator register mismatch. `shadergen.py` produced 693 built-in SM3 variants with zero failures; standard Release `Amax` build had zero errors; `git diff --check` passed. No game was launched during the native probe or build.

Blocked: a playable D3D11 exe needs the Display and Display2 resources/draw-state port, DXGI presentation, and a shader path for the retail `PCFX` effects. No DX11 game exe has been delivered to `Blur1PCDX11/_work/run`. Changes are uncommitted under protocol rule 6.

Next: implement a D3D11 display manager/window and resource bindings in the new branch, then replace the D3D9 backend selection only after the game's shaders and render targets are supported. Keep verifying against the native probe and compile the existing Amax target; run the game through the queued minimized harness when the backend supports it.

Files touched: Blur1PCDX11 `CMakeLists.txt`, `cmake/dx11.cmake`, display `device/d3d11/{D3D11Device.h,D3D11Device.cpp}`, two built-in display shaders, `tools/dx11_probe/main.cpp`, `README.Blur1PCDX11.md`; Blur1PC docs/{BOARD,LEDGER,analysis}.md. No commit.

## 2026-09-26 19:43 — session Codex DX11 continuation — T127

Did: continued the native `blur1pcdx11` branch. Added DXGI window swap-chain creation, present and resize; PC texture and vertex/index buffer allocation; Shader Model 5 variants of the repository's 2D textured and 3D coloured shaders; shader reflection and named constant-buffer binding for the engine's clip vector, world matrix and view-projection matrix. Added a native 3D indexed draw path. `BLUR_USE_DX11` now links the new device library when selected, but the `Amax` game cannot compile with that option because its Display and Display2 native backend types are not implemented. No Direct3D 9 API translation layer was added. No DX11 game exe was delivered.

Verified: the D3D11 probe passed on hardware and WARP with the debug layer, drawing 2D colour, 2D texture and translated 3D colour, plus DXGI present/resize; no validation messages. The default Direct3D 9 `Amax` Release build completed with zero errors after 693 SM3 built-ins compiled successfully. `git diff --check` passed. A direct `fx_5_0` compile of an existing game `.fx` file reached a D3D9 effect `shared` declaration error after include paths and profiles were adjusted, confirming that retail effect source needs an SM5 port. No game was launched. No commit.

Blocked: native Display and Display2 resource/device implementations, then the retail `PCFX` effect shader path. The game is still D3D9 in this branch; T127 remains in progress.

Next: implement the native Display/Display2 backend in `Blur1PCDX11/`, port the effect source and shader cache to SM5, select and build `Amax` under `BLUR_USE_DX11=ON`, then deliver and validate the DX11 exe under the game-run protocol.

Files touched: Blur1PCDX11 `CMakeLists.txt`, `cmake/dx11.cmake`, `README.Blur1PCDX11.md`, display `device/d3d11/*`, shader variants, `tools/dx11_probe/main.cpp`; Blur1PC docs/{BOARD,analysis,LEDGER}.md. No commit.

## 2026-09-26 21:20 — session Codex DX11 continuation — T127

Did: continued the native `blur1pcdx11` branch without a D3D9 API translation layer. Vendored Microsoft FX11 source; extracted the original Jam effect variant list; compiled all authored PC effects to FX5 with native pass blend/depth/raster states, alpha discard and sampler filter/sRGB metadata. Added a name-indexed FX11 effect library, a reusable indexed material draw with cached input layouts, native 2D/3D/cube texture uploads, compressed BC1/2/3 and sRGB sampling, and a Jam-selected SM5 compiler for every built-in shader. Added one SM5 `SV_Position` correction for `psPolygonPaint`.

Verified: `dx11_fx_all` compiled 1,254/1,254 effects and `dx11_shaders_all` compiled 693/693 built-in variants. `dx11_probe` with the D3D11 debug layer passed on hardware and WARP: preloaded all 1,254 FX11 effects, drew an authored ScreenFacingSprite material through the reusable device path, checked MeshEffect blend/depth/cull state and an alpha-discard instruction, loaded compiled built-in shader binaries, sampled BGRA/sRGB and BC1/2/3 textures, created volume/cube/multisample resources, and presented/resized a DXGI swap chain. Python tool syntax checks and `git diff --check` passed. No game was launched. No commit.

Blocked: `BLUR_USE_DX11=ON` Amax still does not compile because its Display and Display2 native backend types, resource wrappers, context methods and FX::Manager integration remain absent. No native DX11 game exe can be delivered yet.

Next: implement the Display2 D3D11 context/resource/device API and Display C_Device/manager wrappers in this branch, replace the game FX::Manager PCFX/D3D9 shader path with the compiled FX5 index, then build Amax and deliver/test its standard `_work/run/Amax.exe` through the queued minimized game protocol.

Files touched: Blur1PCDX11 CMakeLists.txt, cmake/dx11.cmake, README.Blur1PCDX11.md, display device/d3d11/*, built-in shader variants, ui psPolygonPaint, code/tools/{dx11_fxcompile,dx11_shadercompile,dx11_probe}, code/vendor/fx11; Blur1PC docs/{BOARD,analysis,LEDGER}.md. No commit.


## 2026-09-29 — session Claude DX11 — T127

Did: made the native D3D11 backend run `Amax` and hunted the black screen. Fixes: `ID3D11Multithread::SetMultithreadProtected` on the immediate context (the loading thread and the main thread both used it; the debug layer reported corruption and hardware removed the device); the vertex/pixel stage signature convention (vertex shaders emit interpolants first with `SV_Position` last, pixel shaders declare the same order and no position — applied to 24 built-in shaders under `#if defined(BLUR_DX11)`); `BindRenderTargets` no longer binds a depth view of a different size than the colour target; `C_PCD3D11GameDevice::RestoreWindow` follows the retail branch (quad onto the back buffer, copy onto any other surface) instead of always drawing the quad; the D3D11 info queue filters warnings and a vectored exception handler logs its messages before the crash handler; draws with no bound vertex shader are skipped instead of raising, and name the effect that failed. Found that the eight vehicle effects cannot be provided: the retail `.fxb`s ship ten techniques each (`MaterialPrepass`, the `NoDamage` and `LOD` variants) while the leaked `vehicle_common.h` declares five, so every vehicle pass fails with `ERROR_NOT_FOUND` and 64 draws a frame are dropped at the start screen. `System::OnEffectLoad` now dumps the retail microcode of those passes to `_work/build/fxdump/`.

Verified: `Amax` builds with no errors and runs the start screen on retail PC data; the D3D11 debug layer over a full start-screen run reports zero linkage errors, zero thread-corruption messages and no information-queue exceptions (previously 150k linkage messages, a corruption message and a removed device). `Quad::DrawFullScreenColour` into the back buffer and into the window texture reads back magenta and cyan, proving the quad, target and readback paths. A D3D9 reference run (`Blur1PC/_work/run/Amax.exe`, same automation script) shows the start screen at 20 s for comparison. Screen is still black under DX11; only the block-compressed texture-upload alignment message remains.

Blocked: the start screen renders nothing — the frame never reaches the back buffer, and the composite that should put it there has not been identified. Opening a debug race crashes in `UI::C_UiManager::LoadingTextureClone`. The vehicle effects need the shipped shader revision (not in the source tree) reconstructed from the dumped microcode.

Next: dump and disassemble `_work/build/fxdump/veh_*_MaterialPrepass*` with D3DX (`D3DXDisassembleShader`) and reconstruct the missing vehicle techniques as HLSL under `BLUR_DX11`; separately, trace which pass is supposed to present the frame on the back buffer by instrumenting `Blur1PC`'s D3D9 device for one run and comparing its target bindings with the DX11 ones.

Files touched: Blur1PCDX11 shader sources (24 files), display device `d3d11/{D3D11GameDeviceNative.cpp,PCD3D11GameDevice.cpp}`, `display/source/shader/{VertexShaderUtils.cpp,PixelShaderUtils.cpp}`, `fx/{NativeFX11.cpp,System.cpp}`, `fx/include/fx/NativeFX11.h`, `_work/run/{run_script.ps1,logcfg.xml}`, docs/blur1pcdx11/{analysis.md,LEDGER.md,BOARD.md}. No commit (user instruction).

## 2026-09-29 (later) — session Claude DX11 — T127

Did: got a debug race to run and isolated the black screen to one step. `Display2::CloneTexture2D` crashed on D3D11 (`memcpy` between two null CPU addresses) and the race's loading screen called it; it now copies on the GPU under `DEVICE_D3D11`. `C_PCD3D11GameDevice::RestoreWindow` follows the retail branch (quad onto the back buffer, copy onto any other surface). Counters added to the composition path: per race frame `RenderSplitscreenWithoutAA` binds the back buffer and `RestoreWindow` still takes the copy path, because a layer rebinds a post-process target through `BeginRenderSplitscreenWithoutAA` between the bind and the restore. The window render texture holds a complete frame (road, trees, HUD, tyre marks); the back buffer is black.

Verified: a debug race (Right Ctrl on the start screen, Enter) reaches `Racing` and runs 20 s with zero debug-layer messages, zero rejected draws except the vehicle prepass, and a clean exit; `<shot>.window.tga` mean 104 against `<shot>.tga` (back buffer) mean 0. Build has no errors.

Blocked: the back buffer prime. The D3D9 device has to be instrumented the same way and compared before changing the port, since retail takes the same branch and still shows the frame.

Next: build the `BLUR_USE_DX11=OFF` configuration from this worktree into a second build directory with the bind trace in `display/source/device/d3d9/PCD3D9Device.cpp`, run the same race script, and compare the surface sequence with the DX11 one.

Files touched: `display2/source/utils/Utils.cpp`, display device `d3d11/{D3D11GameDeviceNative.cpp,PCD3D11GameDevice.cpp}`, `ApplicationBase.cpp` (trace only), docs/blur1pcdx11/{analysis.md,LEDGER.md}. No commit (user instruction).

## 2026-09-29 (3) — session Claude DX11 — T127

Did: traced the presentation chain and found the fault's home. Retail presents in `I_PostProcess::DisplayFinal` (`E_Type_Final`): bind the back buffer, draw the last chain texture full screen, clear the manager's active flag; `C_Manager::End`'s own final quad is therefore dead when post-processing runs, which is why the earlier counters looked wrong. Disabling post-processing (`Graphics.Render.PostProcess.Enable=false` in the game's `debug_values.xml`, restored afterwards) makes the same race present: back buffer mean 44.8 against 0.0, window texture unchanged at 104. Removed the investigation-only traces afterwards (`postprocess/Manager.cpp` back to its original bytes, `ApplicationBase.cpp` phase log, the `SetColourSurface`/chain-source probes in the D3D11 device).

Verified: race runs 20 s with zero debug-layer messages and a clean exit after the cleanup; `race2.tga.window.tga` mean 104.6 (the full frame) with `race2.tga` mean 0.0 (post-process on) and 44.8 (post-process off).

Blocked: the post-process chain loses the frame between `Begin` and `DisplayFinal`. The vehicle `MaterialPrepass` hole is the white quad over the car.

Next: capture the chain texture at each `I_PostProcess::DisplayIntermediate` to find the link that goes black; the chain's shaders are the post-process pair set already fixed for stage signatures, so check sampler/texture binding and the resolve/ping-pong of the chain render texture next.

Files touched: `display2/source/utils/Utils.cpp`, display device `d3d11/{D3D11GameDeviceNative.cpp,PCD3D11GameDevice.cpp}`, docs/blur1pcdx11/{analysis.md,LEDGER.md,BOARD.md}; `postprocess/Manager.cpp` and `ApplicationBase.cpp` reverted to their previous state. No commit (user instruction).

## 2026-09-29 (4) — session Claude DX11 — T127

Did: committed the branch's uncommitted state and pushed `blur1pcdx11`, at the user's instruction. No source changes this session. T150's `CloneTexture2D` GPU copy went in on its own commit; everything else, including the T127 and T150 records in this file and on the board, went in on a T127 commit.

Verified: `git status` before staging listed 97 modified tracked files and 109 untracked files, all inside the DX11 scope (display and display2 D3D11 backends, 24 built-in shader sources, `fx/NativeFX11`, `movie/w32/bink`, `tools/dx11_*`, `code/vendor/fx11`, `docs/blur1pcdx11`). Largest new file 199 KB, no binary among them, and the only secret-shaped string is `secrets.GITHUB_TOKEN` in Microsoft's own fx11 CI workflow. The tree still carries the T127/T149 investigation traces (`ApplicationBase.cpp`, `deferred/Manager.cpp`, `GameWorld.cpp`, `Lighting/Prepass.cpp`, `ModelResource.cpp`), all behind `#if defined(BLUR_USE_DX11)`.

Blocked: unchanged — the post-process chain loses the frame, and the vehicle effects need the shipped techniques (T148 and T149 open).

Next: as in the entry above — capture the chain texture at each `I_PostProcess::DisplayIntermediate`; `deferred/Manager.cpp` `GetColourTexture` is already instrumented for it.

Files touched: docs/blur1pcdx11/LEDGER.md.

## 2026-09-29 (4) — session Claude DX11 — T127, T149

Did: found and fixed the four faults that kept the DX11 build from presenting (analysis.md, "four faults between the renderer and the screen"): built-in shaders got their textures in the wrong registers (sampler-to-texture register map from reflection in `D3D11GameDeviceNative.cpp`); `C_D3D11GameDevice::Unbind` cleared the whole pipeline (now a port of the D3D9 function); the FX11 bridge read FX texture values as `Display::C_Texture*` when they are `ID3D11Resource*`, and could not resolve texture parameter names (`NativeFX11.cpp`, `D3D11EffectLibrary`); `Display::Quad` applied D3D9's half-pixel shift (now skipped under `BLUR_USE_DX11`, as the engine does on PS3). Added a one-frame draw trace (`BLUR_DX11_TRACE_AT`, `BLUR_DX11_TRACE_PX`, per-draw debug messages with `BLUR_DX11_DEBUG=1`) and `_work/run/run_slot.ps1` (runs an automation script through `tools/game_slot.ps1`). Delivered `_work/run/Amax.exe`.

Verified: builds with no errors. Start screen (`scripts/start.txt`): back buffer mean RGB 28/47/73; showroom street, lights and all UI present, matching the D3D9 reference except the car body. Debug race (`scripts/race.txt`): reaches Racing, runs 18 s, clean exit; HUD, mirror, minimap, buildings and far road present. Before this session both back buffers were mean 0.

Blocked: none for presenting. Open rendering faults: the car body (T148), the near road rendering white in the race (`Road / MaterialLights` output; input not identified), no sRGB decode on texture reads.

Next: trace `Road / MaterialLights` at (0.70, 0.83) of a race frame with each of its textures replaced in turn by a black view (or disassemble the FX11 pass and dump its constant buffer) to find the input that whitens it; then sRGB (TYPELESS texture creation plus `_SRGB` views where `SetTextureGamma` or `effects.samplers` asks for them).

Files touched: display device `d3d11/{D3D11GameDeviceNative.cpp,D3D11GameDevice.cpp,PCD3D11GameDevice.cpp,D3D11EffectLibrary.cpp}`, `display/include/display/device/d3d11/D3D11EffectLibrary.h`, `display/source/render/Render.cpp`, `fx/source/NativeFX11.cpp`, `_work/run/{run_slot.ps1,scripts/race.txt,scripts/start.txt}`, docs/blur1pcdx11/{analysis.md,LEDGER.md,BOARD.md}. No commit (user instruction).

## 2026-09-29 — session Codex DX11 — continuation status inspection

Did: inspected the current phase board, ledger, analysis, presentation fixes and Git state after the provider handover. The previous session's commit-and-push request is complete at `517ebfec`; the wider T127 port remains in progress. No rendering work was started during this status inspection.

Verified: initial worktree was clean; HEAD and origin tracking reference were `517ebfec0820aff3fc364975fdbd9df069576a4c`, and `git ls-remote origin refs/heads/blur1pcdx11` confirmed the live remote has that commit. The delivered `_work/run/Amax.exe` and standard Release build exe have identical SHA256 `354DDFEB66F58FF4F5F8014D16663C0419CBCFCFDBAA97DF27B59E10E1A81E27`. Current source and phase docs contain the four presentation fixes. No build or game run performed. As confirmed by the user, blur1pcdx11 is the top branch and requires no upward merge.

Blocked: none for the completed commit-and-push request. The board still records missing vehicle techniques (T148), white near road and missing sRGB reads.

Next: await the user's next instruction; the recorded rendering investigation begins with the `Road / MaterialLights` inputs, followed by sRGB views and vehicle shader reconstruction.

Files touched: docs/blur1pcdx11/LEDGER.md (this inspection entry only; uncommitted).

## 2026-09-29 18:54 — session Codex DX11 — T127

Did: resumed the port with a fresh visual assessment at the user's direction. Captured matched debug-menu races from the delivered DX11 exe and the D3D9 phase: DX11's scene is substantially worse than the previous summary suggested. Corrected ignored technique Define annotations by compiling and selecting separate FX11 effects; shared-group changes now update native effect parameters by name; legacy constant-buffer writes leave active FX11 buffers bound; raw render-context texture setters now use the device's reflected register map. That last change visibly restored the sky's clouds. Fixed the native probe's stale input-signature call, made Amax depend on its FX compilation, and updated the obsolete branch README and phase analysis. Removed all new investigation-only parameter dumps and road texture suppression after testing. Delivered Amax.exe/Amax.pdb and the new native effect binaries and metadata to this phase's standard run directory. No commit or push.

Verified: all 1,254 source effect variants compiled with zero failures, producing 5,196 indexed entries; 693 built-in shaders compiled; standard Release Amax build had zero errors by errsum.py. Hardware and WARP probes with the debug layer each preload the complete effect set, verify the road lighting technique's dynamic-light resource, draw the material, test other resources/shaders, present/resize and exit zero. Queued minimized race runs show the sky improvement, but still show washed-out ground, broken buildings, duplicated mirror strips and missing car body. Suppressing the road's planar reflection for one diagnostic run did not remove the white foreground; the window frame is already broken before final post-processing. The final cleanup build's first verification run ended before its scripted capture (no crash.txt; cause unknown), so its stale screenshot was not counted as evidence. A fresh retry (`visual_delivery.txt`) wrote a new still at 28.29 s, completed its scripted quit and exited zero in 30 s; visually inspected `visual_delivery.png`, confirming the sky improvement and remaining scene faults. No debug-layer error, binding-failure or view-failure lines appeared in that final log. Exe and PDB hashes match their standard Release outputs; git diff --check passed.

Blocked: none for this continuation's compiler/binding corrections and visual assessment. T127 is still incomplete: major scene sampling/lighting faults, sRGB reads and vehicle body techniques (T148) remain. Clean probes and logs do not establish visual parity.

Next: use the fresh D3D9/ DX11 visual baseline, isolate Road / MaterialLights albedo and diffuse inputs in the window render texture, and validate actual texture contents and GPU resource/sampler/constant-buffer bindings at the draw. Implement the existing sRGB sampler contract, then tackle the missing shipped vehicle techniques. The reflection-only isolation has already been tried and did not solve the washout.

Files touched: README.Blur1PCDX11.md, cmake/dx11.cmake, display device/d3d11/{D3D11EffectLibrary.h/.cpp,D3D11GameDevice.h,D3D11GameDeviceNative.cpp,D3D11Device.cpp}, display/source/context/Context_D3D11.cpp, fx/source/{NativeFX11.cpp,FxContext.cpp}, tools/dx11_fxcompile/{compile_fx.py,compile_manifest.py}, tools/dx11_probe/main.cpp, docs/blur1pcdx11/{BOARD.md,analysis.md,LEDGER.md}; ignored build/run scripts, effects and visual artifacts. All tracked changes remain uncommitted.

## 2026-09-29 18:35 - Codex context-efficiency - T176
Did: applied the user-authorized context-efficiency patch to this phase worktree and the other active phase worktrees (blur1pc, blur1multi, blurx, blur1pcdx11). Reduced always-loaded AGENTS.md from about 22 KB to 6,054 bytes; preserved every original substantive section in docs/agent-reference/{layout,build,architecture,workflow}.md and added context.md. Added tools/agent_context.py for bounded task/ownership/ledger reads, saved-log excerpts, and recorded Codex token metrics. Global Codex limits: tool_output_token_limit=4000, model_auto_compact_token_limit=140000.
Verified: all phase instruction links resolve; all original sections preserved; helper copies identical; all four context commands expose the requested task, ledger, and ownership within 8,000 characters. Exercised log neighbors/tail, missing-task rejection, malformed usage-event handling, token metrics, and small/default output caps. TOML parsed and installed Codex rendered a fresh prompt successfully. Only this patch's paths were edited; concurrent code work was not changed by this task. No game build/run required for documentation and read-only helper changes.
Blocked: none for applying the patch. Worktree changes remain uncommitted under the project's commit-on-request rule; no branch merges or pushes performed. The untouched master/source snapshot was not modified.
Next: reload instructions/configuration in new sessions, then compare recorded per-request tokens and same-session cumulative usage deltas for comparable completed tasks. Full logs remain on disk for any truncated excerpt. Commit these paths only when requested, preserving the existing branch ancestry and other work.
Files touched: AGENTS.md; docs/agent-reference/; tools/agent_context.py; docs/blur1pcdx11/{BOARD,LEDGER}.md. Global config: C:/Users/uKER/.codex/config.toml. Backups: C:/Users/uKER/.codex/tmp/context-efficiency-20260929-182911.

## 2026-09-29 19:10 — session Codex DX11 — T127

Did: switched the primary visual iteration testbed to startup Showroom at the user's request. Added ignored single-shot DX11/D3D9 automation scripts, captured both delivered phase executables, and updated the task and analysis. No renderer changes, rebuild, commit or push in this continuation.

Verified: queued minimized DX11 runs exited zero in eight seconds; the D3D9 reference exited zero in thirteen seconds. Inspected both stills: DX11 has wheels but no car body and broken dark background surfaces; D3D9 has a full car and properly textured scene. Saved DX11 log confirms dropped `veh_paint / MaterialPrepass` draws without a vertex shader. Capture uses one scripted still per run, not PNG bursts.

Blocked: none for changing the testbed. The port remains incomplete; missing shipped vehicle techniques and scene rendering faults remain open.

Next: use `showroom_dx11.txt` for fast visual checks; recover the missing vehicle passes from the existing retail microcode dumps, and investigate background texture/sampler/constant bindings in this smaller scene. Use races only for later coverage.

Files touched: docs/blur1pcdx11/{BOARD.md,analysis.md,LEDGER.md}; ignored `_work/run/scripts/showroom_{dx11,dx9}.txt`, captures and DX11 log. Existing renderer changes remain uncommitted.

## 2026-09-29 20:03 — session Codex DX11 — T148, T127

Did: recovered all ten vehicle material techniques for 102 exact loaded retail permutations as native HLSL. Found and fixed selection by nonunique internal FXB names; the bridge now uses the loaded filename, and opt-in dumps preserve distinct variants. Discarded the first unkeyed generated sources and rebuilt them from correctly keyed captures. Fixed the DX11 shared-immediate cache contract and typed sampler metadata capture. Added fail-closed recovery tooling, six unit checks, GPU-bound buffer/view diagnostics, complete recovered-pass checks and an alloy MRT normal/depth probe. Effect compilation now has freshness-checked reuse of unchanged authored shaders. Delivered the standard exe/PDB plus effect binaries/index/metadata. No commit or push.

Verified: final standard Release build has zero errors; hardware and WARP debug probes preload 4,913 indexed entries and verify all 1,020 recovered techniques. Both alloy prepasses return the expected packed normal and -0.2 depth. Recovery tests pass 6/6. Successive minimized queued Showroom captures demonstrate the progression from no body, through black silhouette, to full body and shaded wheels. The final run with tracing disabled (`showroom_delivery.txt`) exits zero in nine seconds and writes a fresh still, visually inspected. Delivered exe/PDB hashes match their Release outputs. No missing-effect, parameter/texture-binding or input-layout failure appears in that final log. Some other draws still report no vertex shader with no associated FX error; the scene remains too dark and incorrectly textured. git diff --check passes.

Blocked: none for the recovered-pass task; T148 is done for the 102 captured permutations. Full vehicle/scene parity, uncaptured material variants, native alpha testing and texture sRGB remain T127, which is active again. Do not treat the probe or restored body as proof the port is finished.

Next: inspect a broken Showroom building draw's actual GPU inputs, shader/declaration formats, constants and texture samples; compare its serialized vertex contract with the compiled shader and implement the existing sRGB contract. Keep Showroom as the fast testbed; use races only for broader validation later. Exact recovery evidence is `_work/build/fxdump_variants`, not the historical overwrite-prone `fxdump`.

Files touched: fx/{Effect.h,NativeFX11.h,Manager.cpp,System.cpp,NativeFX11.cpp,FxContext.cpp}; display/device/d3d11/D3D11GameDeviceNative.cpp; vehicle/include/vehicle/shaders/retail_dx11/{102 .fx files and sampler sidecars}; tools/dx11_fxcompile/{compile_fx.py,compile_manifest.py,recover_vehicle.py,test_recover_vehicle.py}; tools/dx11_probe/main.cpp; cmake/dx11.cmake; README.Blur1PCDX11.md; docs/blur1pcdx11/{BOARD.md,analysis.md,LEDGER.md}; ignored build/dump/run artifacts. Renderer work remains uncommitted.

## 2026-09-29 23:54 — session Codex visual diagnosis — T127

Did: delivered a visual repair batch grounded in the pristine PC mechanisms and actual GPU traces. Fixed omitted sampler addressing defaults, alignment/upload of streamed mip tails, native device default sentinels, five families of shader input interfaces, mutable per-draw alpha testing across authored/recovered/built-in shaders, and D3D9 colour-factor translation into valid D3D11 alpha blend factors. Traced the large white race rectangle to a rejected drop-shadow multiply state and corrected the translation. Restored visible road/building textures, foliage cutouts and the mirror; removed the white rectangle. Retained opt-in diagnostics and improved debug-message deduplication. Updated the board, analysis and README. Found in / Belongs to Blur1PCDX11. No commit or push; no lower-phase changes.

Verified: standard Release build reports zero errors; 693 built-ins and 1,254 base effect outputs compile with 4,913 indexed entries. Hardware and WARP probes each pass 45 GPU alpha comparisons, five GPU RGBA blend-factor checks, 42 compiled shader-interface pairs, actual sampler-default checks, all 1,020 recovered vehicle techniques, alloy MRT readback and existing presentation checks. Python compiler modules compile. Final minimized, queued race and Showroom runs exit zero in 27 and 13 seconds respectively; both logs have no D3D11 severity 0/1/2 messages. Visually inspected one still from each: `_work/build/race_shadow_fixed.png` and `showroom_shadow_fixed.png`. Delivered `_work/run/Amax.exe` and `Amax.pdb` hashes match standard Release outputs; effects are delivered in `_work/run/fx11`. Whitespace checks pass. Streaming-copy correctness was checked against pristine source and game scenes, not a standalone streaming GPU test.

Blocked: none for this repair batch. T127 remains in progress: vehicle/scene lighting and sRGB sampling lack parity, some unidentified draws have no vertex shader, and uncaptured retail variants remain outside recovered coverage. Clean debug logs do not establish full visual parity.

Next: implement the original per-sampler sRGB contract with compatible texture/view formats, verify actual GPU sample values, then compare the same Showroom against the saved D3D9 reference. Separately identify the remaining no-VS draw callers and recover any missing variants from correctly keyed retail captures.

Files touched: display/source/{context/Context_D3D11.cpp,device/d3d11/{D3D11GameDeviceNative.cpp,D3D11States.cpp,PCD3D11GameDevice.cpp},texture/d3d11/D3D11TextureLoad.cpp}; display/include/display/device/d3d11/D3D11States.h; fx/source/{NativeFX11.cpp,System.cpp}; render/deferred/UI/drop-shadow pixel shader interfaces; tools/dx11_shadercompile/{alpha_test.py,compile_builtins.py}; tools/dx11_fxcompile/{compile_fx.py,compile_manifest.py}; tools/dx11_probe/main.cpp; cmake/dx11.cmake; README.Blur1PCDX11.md; docs/blur1pcdx11/{BOARD.md,analysis.md,LEDGER.md}; ignored diagnostic/build/run artifacts. Existing unrelated work preserved.

## 2026-09-30 01:30 — session Codex visual diagnosis — T127

Did: committed the requested visual repair batch as `24d23d41` (`Fix DX11 texture streaming, alpha testing and shadow blending`), then continued the native-renderer work. Implemented compatible typeless colour resources with linear/sRGB views and live FX sampler-state/force-gamma overrides; restored omitted PC occlusion/particle shader initialization on DX11; corrected one/two-sided stencil state, winding and PC CCW mask/reference semantics. Delivered these follow-up changes plus bounded actual-state and selected-material diagnostics. Refreshed the D3D9 Showroom reference and reclassified its background blur as expected. Follow-up changes remain uncommitted. No push or lower-phase source changes. Found in / Belongs to Blur1PCDX11.

Verified: standard Release builds report zero errors, including final `_work/build/visual_alloy_trace_build.txt`. Hardware and WARP probes each pass 24 actual gamma transitions plus HDR preservation, 24 stencil-byte checks, 45 alpha comparisons, five blend checks, 147 shader-interface pairs, all 1,020 recovered vehicle techniques, alloy MRT readback and presentation checks. Final rendering-fix Showroom and race runs (`showroom_stencil_final` and `race_stencil_final`) exit zero with debug enabled and no missing shaders, binding failures, rejected draws or severity 0/1/2 messages; both stills were inspected. Two subsequent diagnostic-only Showroom runs also exit zero. Current delivered and standard Release exe hashes both equal `88E3249147736871D0B0D7AF2308EB9DD69FF2EAA86E0D24A07D775169A1175D`; PDB hashes both equal `EC73CAD7EC7A120ED062661990CD81927BA90867499084C38BD2B87041449A96`. git diff --check passes. Probe triangle cases do not cover line/point stencil behavior; streaming correctness remains source/game verified rather than independently GPU tested.

Blocked: no build/delivery blocker. T127 remains open: vehicle/reflection shading differs from the PC reference; legacy luminance channel expansion is unimplemented; uncaptured retail variants remain outside recovered coverage. One loading attempt crashed through a null damage-volume LockBox3D pointer; five later race runs succeeded, but no crash fix or causal explanation is established. The actual alloy draws have valid depth targets and expected reverse-depth/cull state; their matrices and sampled resources are populated. This does not establish visual parity or prove duplicated geometry.

Next: isolate the main wheel material output before later draws, then compare cube samples and lighting constants with D3D9 using `_work/build/showroom_alloy_inputs.log` and the refreshed `_work/build/dx9_compare/showroom_current_dx9.png`. For the intermittent volume crash, instrument null-resource, duplicate-lock and staging-allocation failure paths in native LockResource before another reproduction; do not mask it with a vehicle-side null check. Separately GPU-verify the legacy L8/A8L8 RGBA sample contract before choosing the native expansion path. Task ownership remains this session; the board and phase analysis record this checkpoint.

Files touched after the commit: display device/d3d11 format/state/effect-library/game-device code and headers; display texture/d3d11 loader and occlusion initialization; display2 native device/context code; dynamic particle initialization; fx native binding/context/header; dx11_probe; README.Blur1PCDX11.md; docs/blur1pcdx11/{BOARD.md,analysis.md,LEDGER.md}; ignored build/probe/run/capture artifacts. Existing work preserved; no broad staging.

## 2026-09-30 - Codex Business Bay - T186 propagation claim
Owner: session Codex Business Bay. Inherited base commit 66a7afac by merge. Expected phase files: board, analysis and ledger only; no phase-specific engine changes. Standard regeneration/build underway; next verify zero errors and deliver the own standard EXE/PDB after any running delivered process exits.

## 2026-09-30 - Codex Business Bay - T186 propagation complete
Did: inherited 66a7afac via merge 6bd5e080; no phase-specific engine edit. Verified: standard tools/build.sh, errsum0, own standard EXE/PDB delivered and hashes match build sources (_work/t186/delivery.txt). No separate phase runtime claim. Blockers: none. Exact next step: none for this propagation task. No push.

## 2026-09-30 02:47 — session Codex visual diagnosis — T127

Did: implemented the PC L8/A8L8/L16 sampling contract after measuring the D3D9 HAL GPU, including pre-filter sRGB decoding, original PC format metadata, streamed uploads, clones and PC-layout staging locks. Added independent D3D9 sampling and native GPU probes. Added bounded native lock-failure diagnostics and selected-draw captures; fixed screenshot readback of typeless colour targets. Isolated wheel bands in the main alloy material output before later body/composition draws. Actual D3D9 car capture is valid; the exact retail alloy pixel shader, material/light constants, SH node, view-projection matrix and sampler address/filter/gamma settings match the native trace. Small wheel-world differences between runs remain animation-dependent and are not a diagnosed fault. Found in / Belongs to Blur1PCDX11. Follow-up work uncommitted; no push or lower-phase source changes. Preserved concurrent T186 propagation.

Verified: standard Release build `_work/build/visual_filtered_final_build.txt` has zero errors. Hardware and WARP probes each pass 58 luminance samples, 48 byte-exact PC-layout readbacks, 24 gamma transitions, 24 stencil checks, 45 alpha comparisons, five blend checks, 147 shader-interface pairs, all 1,020 recovered vehicle techniques, MRT and presentation checks. Independent D3D9 reference passes 12 checks. Minimized queued Showroom and DebugRb race runs with the luminance fix exit zero, with no missing VS, lock/map/view failure or severity 0/1/2 messages; stills inspected. Two selected wheel-draw captures also exit zero. Final delivered standard exe hash is `0E39306D0CAFF49B64469F1615F1F997F2C8E053D1753381B5F484E4C8545C43`; PDB is `F1FCC4C78BC9CC0D02D934AF2AA155AF7F364561FBE1C979163AA179B441B356`, both match Release outputs. Whitespace checks pass. Readback probes test production conversion/upload, not the game's complete staging lock lifecycle.

Blocked: no build/delivery blocker. Vehicle/reflection visual parity remains open; luminance expansion uses more native memory while serializer sizes retain their PC meaning. The intermittent damage-volume lock crash has not recurred or been causally explained. Uncaptured retail variants remain outside recovered coverage.

Next: inspect the queued D3D9 replay state at alloy draw 2207381 (`_work/build/dx9_compare/alloy_state.json`) and compare actual bound texture contents and vertex declaration/data against DX11. Matching constants/samplers eliminate those particular settings as evidence for the wheel discrepancy; do not tune shader arithmetic from screenshots. Separately verify the suspected Display2 live sampler LOD-bias translation against pristine source before changing it. Task remains owned and active.

Files touched: native display formats/device/streaming, display2 resource/context and new D3D11TextureFormats.h, Screenshot.cpp, selected-draw diagnostics, dx11_probe luminance/reference tests, cmake/dx11.cmake, README and phase board/analysis/ledger; ignored build/probe/reference/capture files. Prior session changes preserved.

## 2026-09-30 03:08 — session Codex visual diagnosis — T127

Did: corrected FX11 pass state ownership. D3D9 replay proves the alloy bands are expected; selected native draws locate the actual wheel corruption in the later puddle pass. Removed guessed complete FX11 blend/depth/stencil/raster objects so the imported PC FXB and render stages retain their live state. Added GPU state-preservation checks across ordinary and repeated/immediate effect applies. Found in / Belongs to Blur1PCDX11. No lower-phase source changes or push; follow-up work remains uncommitted.

Verified: standard Release errsum0 (`visual_fx_state_build.txt`). Hardware and WARP regressions pass along with prior probes. Queued minimized Showroom and DebugRb race exit zero, clean debug/missing-shader logs, both stills inspected. Native puddle draw now matches D3D9 GREATER/no-depth-write/stencil-replace state; wheel pixel remains unchanged and the reflection overlay is gone. Delivered standard exe `23C8A1BA1224B0E8DD72D7166C3A292CDCD46F5EBC94D512E5026A624F02A577`, PDB `D606F19B18E50D96217DA23A06A15CDE67479BC3C67F901EC545E589AD894AFF` and regenerated FX5 effects. Exe/PDB match Release outputs.

Blocked: none for build/delivery. Broader visual parity, the unreproduced damage-volume loading crash and uncaptured variants remain open.

Next: recover and verify the live sampler filter/address/border/LOD contract from pristine PC source. FX11 currently binds live gamma but retains compiled sampler defaults for the other properties. Distinguish Display2's encoded LOD bias from the old Display device API; preserve the actual PC anisotropy behavior. Add GPU sampling checks before claiming a fix. Task remains active.

Files touched: dx11_fxcompile/compile_fx.py, fx/FxContext.cpp, dx11_probe/main.cpp, README and phase docs; ignored reference captures/build/runtime logs. Other sessions' pending changes preserved.

## 2026-09-30 03:52 — session Codex visual diagnosis — T127

Did: committed FX pass state preservation as `96daf4ae`, as requested. Implemented live sampler address/filter/border/LOD binding and PC /16 bias encoding, with NONE disabling mip selection. Preserved PC maximum-anisotropy behavior recovered from pristine source. Found in / Belongs to Blur1PCDX11. No lower source changes or push.

Verified: standard Release errsum0 (`visual_live_sampler_build.txt`); independent D3D9 HAL passes 23 sampling checks; hardware and WARP pass eleven new live-binding samples and all existing regressions. Queued minimized Showroom/race exits zero, clean debug logs, stills inspected. Delivered standard exe SHA256 `F52D0D0EE81ACFA3A470EF30D2B467A7362593E3119BF4723450FDCE6794398F` and PDB `486F919881CD5D8323A5E6036CF93A1634E3C7FA33D18F69C834D9443870EB60`, both matching Release outputs, plus regenerated effects. Fresh D3D9 race reference and both native/D3D9 draw traces saved. Packed-font GPU output before repair proves NaN positions and zero alpha from valid SHORT2/UBYTE4 inputs (`packed_font_before.log`, expected probe failure exit10).

Blocked: no build/delivery blocker. Broader lighting parity, missing large race-position numeral, intermittent damage-volume crash and uncaptured variants remain open.

Next: finish the standard build of the faithful packed-font numeric conversion, rerun hardware/WARP output checks, deliver and verify the large numeral in a minimized DebugRb race. Then compare headlight/body lighting at actual D3D9/native draws. Task active; font fix not yet delivered at this checkpoint.

Files touched: native FX bindings/effect library, native sampler state/device, dx11_probe sampling/font checks, UI polygon-font shader, README and phase docs; ignored builds/runtime/reference traces.

## 2026-09-30 03:56 — session Codex visual diagnosis — T127

Did: repaired packed polygon-font inputs using native int2/uint4 followed by the numeric conversion performed by D3D9. Pristine UI declarations/storage establish the contract. GPU capture before repair proves NaN positions and zero alpha; after repair both hardware and WARP return all expected values. Found in / Belongs to Blur1PCDX11. No lower source changes or push.

Verified: standard Release errsum0 (`visual_packed_font_build.txt`), full hardware/WARP probes pass. Queued minimized DebugRb race exits zero with clean debug/missing-shader logs. Inspected `race_packed_font_final.png` confirms the large white position numeral is restored. Standard exe/PDB delivered, hashes match Release outputs: exe `7FF617DA6DF045A2D100B5E5DFB52C6166B8CF7B53EC8FFF1D9D9150C66A8580`, PDB `6BC17EC01B8E482280D6EF8145F43FB9862F389BF9954922CAFBC6BFE7682F7D`.

Blocked: none for this repair. Broader lighting parity, remaining packed UI paths, intermittent damage-volume crash and uncaptured variants remain open.

Next: inspect packed stroke/trail declarations and shaders for the same conversion fault; compare actual native/D3D9 road and vehicle lighting draws from retained race traces. Task active.

Files touched: UI vsPolygonFont.vsh, dx11_probe main.cpp/new polygon_font_checks.h, phase analysis/ledger; ignored build/probe/runtime capture files.


## 2026-09-30 04:18 — session Codex visual diagnosis — T127

Did: committed packed font fix as `afce7ba0`. Continued headlight isolation using actual native and D3D9 draw bindings; found corrupt native road depth and NaNs, with valid negative retail vehicle depth. Added opt-in bound built-in shader tracing and temporary scene-prepass diagnostic probe. No lighting arithmetic or depth-source engine change yet.

Verified: diagnostic standard Release errsum0, standard exe/PDB delivered, queued minimized headlight run and D3D9 replay exit zero. Compiled Default prepass signatures agree; headlight constants/states agree. Depth numerical plot and reference framebuffer inspected.

Blocked: none. Broader parity remains active.

Next: inspect all actual prepass writers for interface/arithmetic differences, isolate the draw creating the negative road wedge/NaNs, then repair the faithful native mechanism and verify GPU/runtime output. Packed stroke/trail paths remain open.

Files touched: native device diagnostics, dx11_probe main/new prepass_checks.h, phase board/analysis/ledger; ignored probe/build/capture artifacts.

## 2026-09-30 04:42 — session Codex visual diagnosis — T127

Did: recovered the retail road prepass depth interpolant from actual D3D9 draw 7047254 and repaired the native NO_WORLDNORMAL path. Added 128 GPU depth checks across 64 road permutations. Found in / Belongs to Blur1PCDX11; no lower-phase source changed.

Verified: standard Release and probe builds have zero errors; full hardware/WARP probes pass. Standard exe/PDB and regenerated effects delivered to _work/run with matching hashes. Minimized queued race exits zero, no severe D3D11 diagnostics. Per-draw depth and inspected headlight/final race captures confirm removal of the corrupt road wedge and restoration of headlight wash. Logs/captures: _work/build/road_depth_final and road_depth_{hardware,warp}.log.

Blocked: none. T127 remains in progress; remaining NaN depth pixels, packed UI and broader parity are unverified.

Next: commit this verified repair, isolate the first writer of a remaining NaN depth pixel, then verify packed stroke/trail inputs against the PC source.

Checkpoint: road prepass repair committed as f8344dfd; continuing remaining NaN depth and packed UI diagnosis.

## 2026-09-30 05:07 — session Codex visual diagnosis — T127

Did: preserved the shipped billboard zero-depth result in native shaders after recovering the loaded D3D9 instructions and verifying zero-multiply arithmetic on the GPU. Found in / Belongs to Blur1PCDX11. Road depth repair is committed as f8344dfd.

Verified: sequential standard/probe rebuilds succeed; hardware and WARP pass eight billboard depth checks plus previous checks. D3D9 reference passes25. Rebuilt standard exe/effects delivered; final queued minimized race exits0 with clean diagnostics and no nonfinite depth pixels. Logs and inspected captures: _work/build/foliage_depth and billboard_depth_final. The initial concurrent generator failure is superseded by the sequential rebuild.

Blocked: none. Full parity remains T127. The new stroke GPU capture reproduces NaN positions/UVs before conversion; UI repair verification follows.

Next: commit the billboard repair; finish hardware/WARP verification and deliver the packed stroke/trail/gradient/multiply conversions, then compare the resulting race UI with PC.

## 2026-09-30 05:15 — session Codex visual diagnosis — T127

Did: restored numeric SHORT2 input conversion for stroke/trail/gradient/multiply while preserving each pristine PC shader's coordinate and UV math. Billboard zero-depth repair committed as4a847184. Found in / Belongs to Blur1PCDX11.

Verified: captured actual stroke failure before repair (NaN/incorrect UV), then 88 output components pass hardware/WARP across all four paths. Full probes, standard Release and probe build pass; standard exe/PDB and regenerated shaders/effects delivered, hashes match. Queued minimized race exits0 with clean debug/missing-shader logs. Inspected final race and PC reference; empty purple power-up boxes match PC, while contact shadow needs a targeted check.

Blocked: none. Broader visual parity remains in progress; screenshot comparison alone does not establish a shadow fault mechanism.

Next: commit packed UI repair, trace native screen-blob shadow draw2809 resources/constants/output and recover its corresponding loaded D3D9 shader/state. Preserve accepted rendering decisions.

## 2026-09-30 05:35 — session Codex visual diagnosis — T127

Did: committed packed UI repair16f389a2; recovered actual PC screen-shadow draw7068883 and isolated native inputs. Fixed pixel parameter/array uploads to use the complete reflected span, faithfully following pristine D3D9. Found in / Belongs to Blur1PCDX11.

Verified: before/after actual GPU matrix binding, matching shadow/stencil texture contents, restored contact shadow in inspected final capture. Standard Release exits0/errsum0; standard exe/PDB/effects delivered, hashes match; queued minimized race exits0 and severe/missing-shader diagnostics are clean. Logs/captures: blob_shadow_before, blob_shadow_trace and blob_shadow_final.

Blocked: none. Full vehicle/reflection and rendering parity remains T127.

Next: commit the reflected-span repair, then compare the corrected race against PC for the next reproducible visual discrepancy. Keep diagnostic captures separate from delivered output.


## 2026-09-30 06:20 — session Codex visual diagnosis — T127

Did: committed reflected pixel-span repair1df3e064. Compared race paint constants, packed inputs and texture contents against actual PC draw7057314. Repaired the retail damage-volume name alias in native reflection/metadata and restored PC's existing vertex-default volume substitution. Found in / Belongs to Blur1PCDX11.

Verified: all98 recovered volume-using effects pass alias/reflection checks; three actual paint VS volume-alpha samples pass hardware/WARP. Standard Release exits0/errsum0; probe builds/full runs pass. Standard exe/PDB/effects delivered, hashes match; minimized queued race exits0 with clean severe/missing-VS diagnostics and actual GPU volume binding now R32G32B32A32_FLOAT / Texture3D. Evidence: paint_lighting_trace and vehicle_volume_final. Paint texture exact; cube/dent/scratch mip0 comparisons within one byte/channel; lighting constants match closely.

Blocked: none. Restored binding/default does not resolve or establish full vehicle paint parity.

Next: commit the volume binding/default repair, then compare environment cube-map mip levels and actual material outputs. Keep broader T127 active.

## 2026-09-30 06:50 — session Codex visual diagnosis — T127

Did: committed retail vehicle volume alias/default repair1839e1ca. Compared all vehicle reflection/damage mips and both actual packed paint vertex paths. Added opt-in real GPU vertex-output capture; continuing final rear-body brightness isolation. Found in / Belongs to Blur1PCDX11.

Verified: texture mip comparisons within BC1 byte rounding, exact PC/native vertex-buffer contents, shipped-instruction CPU/GPU comparisons for normals/UV/world/SH, finite SO values and matching zero control points. Standard diagnostic Release builds exit0/errsum0 and are delivered. Queued minimized native runs and headless PC reference replays exit0. Material-stage selected stills are visually close; full paint/frame parity is unverified.

Blocked: none.

Next: complete queued `post_paint_trace` native/PC input capture, compare scene and bloom textures before final colour composition, then recover and repair the first differing mechanism. T127 remains active.

Files touched: native game-device opt-in diagnostics, phase analysis/board/ledger; scratch evidence under _work/build.


## 2026-09-30 07:20 — session Codex visual diagnosis — T127

Did: completed final-composition input comparison; narrowed the bright rear panel to exact body draw1277 / PC7057320. Isolated a reproducible pixel constant-buffer getter observer effect. No production getter workaround added. Found in / Belongs to Blur1PCDX11.

Verified: four standard diagnostic builds exit0/errsum0 and delivered; queued captures and PC replay exit0. Exact diagnostic-enabled panel RGB matches within one byte. Texture/cube inputs match within BC1 rounding. Slot0 PS getter changes render output; texture/VS reads and PS slot13 getter do not. WARP baseline remains bright.

Blocked: none; earlier diagnostic-enabled material similarity cannot establish normal-launch parity.

Next: consume hardware no-debug baseline, isolate Default-scene versus vehicle getter timing, then compare actual bindings before/after the responsible transition and recover the binding fault. T127 remains active.

Files touched: native game-device opt-in selector/filter diagnostics, phase analysis/board/ledger, scratch evidence.


## 2026-09-30 08:14 — session Codex visual diagnosis — T127

Did: corrected the previous pixel-getter causality claim after isolation disproved it. Removed temporary input-layout cache auditing after zero collisions. Added opt-in post-draw shader/vertex capture and index-count filtering. Found in / Belongs to Blur1PCDX11.

Verified: latest standard Release exit0/errsum0 and delivered. All queued isolation/capture runs exit0. Ordinary current hardware launch has dark body; no production repair explains the change. Actual packed vertices match PC; finite GPU outputs and shipped SH arithmetic agree within 1.124e-8.

Blocked: none; stable normal-launch paint parity remains unverified.

Next: trace final colour matrix source/settings and binding, then recover the first verified differing mechanism. Keep paint brightness instability open. T127 remains active.

Files touched: native game-device opt-in diagnostics, phase analysis/board/ledger; scratch evidence under _work/build.


## 2026-09-30 08:27 — session Codex visual diagnosis — T127

Did: committed the verified native occlusion/particle initialization repair: enable the existing pristine PC setup for DEVICE_PCD3D11 and extend shader linkage checks to the affected stages. Found in / Belongs to Blur1PCDX11.

Verified: hardware and WARP pass all 147 built-in shader interface pairs. Standard Release visual_colour_matrix_build exit0/errsum0, delivered executable/PDB/effects. Queued minimized race colour_matrix_trace exits0 without missing vertex shaders or rejected draws. The colour matrix reflects register2/count4 and current CPU/GPU values agree; the earlier identity result remains unexplained.

Blocked: none; intermittent volume-lock crash and stable paint brightness parity remain open.

Next: consume the latest ordinary baseline, compare the current frame with the PC reference, and recover the first stable differing mechanism. T127 remains active.

Files touched: OcclusionQuery.cpp, particle Manager.cpp, dx11_probe/main.cpp selected linkage additions, phase analysis/board/ledger.


## 2026-09-30 08:38 — session Codex visual diagnosis — T127

Did: committed the verified PC stencil contract: retain CW/CCW banks and the two-sided enable flag, map bank orientation using the rasterizer, apply one-sided state to both faces and retain PC CCW reference/mask no-ops. Found in / Belongs to Blur1PCDX11.

Verified: pristine D3D9Device.inl comparison; hardware/WARP stencil_probe logs each pass 24 GPU stencil-byte cases. Standard Release visual_lut_capture_build exit0/errsum0 and delivered. Queued colour_lut_trace exits0. Current GPU LUT matches the PC's entire 131072-byte upload; current CPU/GPU colour matrix is identity again, so its variability precedes GPU upload and remains unresolved.

Blocked: none; colour/paint stability and intermittent volume-lock crash remain open.

Next: inspect live colour-correction setting in the queued native capture; recover the settings mechanism before proposing a colour fix. T127 remains active.

Files touched: native D3D11States helper and selected game-device stencil paths, dx11_probe selected GPU stencil check, phase analysis/board/ledger.

## 2026-09-30 08:55 — session Codex visual diagnosis — T127

Did: commit the verified PC luminance, per-sampler gamma and live sampler contract, with shared/immediate FX updates and prevention of legacy constants overwriting FX11 buffers. Preserve serialized/locked PC bytes while using expanded native luminance storage. Correct the probe's explicit fstream dependency, committed Get interface calls and effect-count threshold. Found in / Belongs to Blur1PCDX11.

Verified: indexed-source MSVC compilation passes 14 translation units; indexed probe links and queued hardware/WARP runs exit0, including 58 luminance samples, 48 exact round trips, 24 gamma transitions and eleven live samplers each. D3D9 reference passes23. Current standard Release visual_colour_setting_build exit0/errsum0 and delivered; queued race exits0. Both GPU lookup volume readbacks match all131072 PC upload bytes. CPU/GPU colour rows agree; live colour correction enabled; nonidentity correction directions agree with PC at different strengths.

Blocked: none. Matching the active timed post effect remains unverified; paint stability and intermittent volume-lock crash remain open.

Next: compare paint luminance/exposure inputs at the bright and dark draws before final composition; recover the first differing upstream mechanism. T127 remains active.

Files touched: native display/device/context/texture format and sampler paths, Display2 luminance resource/lock transport, FX shared/immediate binding, owned probe sources/headers and CMake reference target, phase docs.

## 2026-09-30 09:31 — session Codex visual diagnosis — T127

Did: isolated the entire bright/dark paint difference to live RearLightColour, using identical-input GPU replay and opt-in CPU upload evidence. Supersedes earlier paint/getter instability claims; no production workaround or shader tuning. Found in / Belongs to Blur1PCDX11 for native diagnostics; upstream reverse-light launch state remains unclassified.

Verified: private actual-mesh/material replay reproduces both RGBA outputs by changing only row7; hardware/WARP agree within one RGB byte. CPU shared upload and GPU agree at zero and .1 in two fresh combined scene captures. Queued runs exit0. Standard Release visual_rear_light_cpu_all_build exit0/errsum0; standard executable/PDB/effects delivered.

Blocked: none. Active timed post effect and neutral-launch reverse-light state are not matched; intermittent volume-lock crash remains open.

Next: compare protected final-composition textures and reflection/road output against PC at matching rear-light state. T127 remains active.

Files touched: NativeFX11.cpp opt-in CPU diagnostic; phase analysis/board/ledger. Private replay and capture harness stay under ignored _work/.


## 2026-10-02 23:08 — session Codex visual diagnosis — T127

Did: verify same-input final composition against D3D9 and capture road material/reflection inputs after matched-resolution PC/native runs. No production tuning. Found in / Belongs to Blur1PCDX11.

Verified: standard Release parity_oct02_build exit0/errsum0 and delivered exe/PDB/effects. Queued PC60, native60 and native unlocked races exit0. Actual final-composition hardware replay differs in five channels by one byte; WARP maximum one byte. Road static material textures within one decoder byte; live road sampler/constant contract agrees with PC.

Blocked: none. Native/PC lightmap residency differs; old road trace resolution and elapsed state differ. Intermittent damage-volume loading crash remains open.

Next: compare road lightmap at matching resident dimensions, then road vertex inputs and reflection producer. T127 remains in progress.

Files touched: phase analysis, board and ledger. Private replay/capture tools remain under ignored _work/.


## 2026-10-02 23:52 — session Codex visual diagnosis — T127

Did: restore retail saturation in the road prepass normal blend and remove the extra RoadTwoSpecN normal-axis swap for DX11. Found in / Belongs to Blur1PCDX11. The foreground noisy paving now renders smoothly.

Verified: actual retail SM3 bytecode versus captured native mesh/texture/constants. Road hardware comparison is byte-exact over 211,463 covered pixels; RoadTwoSpecN controlled-camera comparison is byte-exact over 291,035 pixels. WARP residuals remain explicitly bounded in analysis. Standard road_normal_clamp_build and road_twospec_axis_build exit0/errsum0; 1254 effects/693 built-ins regenerated. Queued native60/unlocked and updated native60 races exit0. Standard exe/PDB/effects delivered; executable hash agrees with build Release. Protected final road lighting inputs in road_paving_lights_oct02.

Blocked: none. Full reflection/lighting and uncaptured retail variant parity remain unverified; intermittent damage-volume loading crash remains open.

Next: compare foreground Road_LN_ND_NG / MaterialLights draw 7059193 against actual retail bytecode using the protected identical inputs. T127 remains in progress.

Files touched: RoadTwoSpecN.fx, Special/RoadSurface.h, phase analysis/board/ledger. Private GPU replay/capture artifacts remain under ignored _work/.


## 2026-10-03 00:16 — session Codex visual diagnosis — T127

Did: verify foreground road final lighting and the road reflection producer against actual retail shader bytecode; protect the next general scenery reflection draw. No production adjustment. Found in / Belongs to Blur1PCDX11.

Verified: road final-lighting hardware replay byte-exact over212,024 covered pixels; road reflection hardware replay byte-exact over56,827 covered pixels. WARP final-lighting max5/reflection max1. Both queued probes exit0. Native road reflection and general scenery reflection capture races exit0. Existing standard Release road repairs remain delivered; this checkpoint adds evidence only.

Blocked: none. Other material/reflection permutations and damage-volume crash remain open.

Next: consume queued default_lod_replay hardware/WARP results and compare the captured general reflection material against retail draw7048564. T127 remains in progress.

Files touched: phase analysis, board and ledger. Private tools and replay artifacts remain under ignored _work/.


## 2026-10-03 00:25 — session Codex visual diagnosis — T127

Did: verify the selected general scenery material through reflection, normal/depth prepass and final lighting using actual retail shaders and identical protected inputs. No production tuning. Found in / Belongs to Blur1PCDX11.

Verified: reflection hardware exact on captured and controlled cameras; prepass hardware normal colour and R32F depth exact; final lighting differs by one byte in one channel. WARP bounds and one reflection coverage residual recorded in analysis. All queued captures/private replays exit0. Existing standard road fixes remain delivered.

Blocked: none. Broader variants and intermittent damage-volume crash remain open.

Next: consume live driving_parity_oct03 batch (PC60/native60/native unlocked), inspect compressed videos and final stills for moving rendering differences, and classify the next reproduced fault. T127 remains in progress.

Files touched: phase analysis, board and ledger. Private artifacts remain under ignored _work/.


## 2026-10-03 00:41 — session Codex visual diagnosis — T127

Did: repair DX11 splash PS and ordinary-particle signature registers, preserving the retail stage selection and shader math. Extend the signature regression to166 actual stage pairs. Add the FX pass name to existing bounded debug-layer diagnostics. Found in / Belongs to Blur1PCDX11.

Verified: standard moving_shader_links_build and moving_binding_diagnostic_build errsum0;693 native built-ins regenerated/embedded; standard exe/PDB/effects delivered. Signature check passes166 pairs. Queued corrected moving native60/unlocked races exit0; both prior linkage errors are gone. PC driving reference reaches133mph/60Hz. Native moving performance remains about6fps; cube/2D binding error persists and is identified as ScrollFade_LN_NL / Material. Native video requests are unsupported; these checks use one final still.

Blocked: none. Performance, ScrollFade binding, broader visual variants and intermittent damage-volume crash remain open.

Next: consume scrollfade_moving_oct03 selected-draw capture and recover the cube sampler's actual resource/default mechanism against retail before changing binding. T127 remains in progress.

Files touched: SplashCamera psSplashCamera.psh, dynamic Particle.h, native PCD3D11GameDevice diagnostics, owned dx11_probe linkage check, phase analysis/board/ledger. Private artifacts remain under ignored _work/.


## 2026-10-03 00:54 — session Codex visual diagnosis — T127

Did: recover PC cube-sampler reads with bound2D textures and repair the native FX dimension mismatch. Add direct PC/native GPU regressions, preserving valid cubes. Found in / Belongs to Blur1PCDX11.

Verified: PC67 GPU checks; native54 checks on hardware and WARP without debug errors; standard scrollfade_cube_binding_build errsum0 and exe/PDB/effects delivered. Queued moving native60/unlocked both exit0/60s/wait0; prior linkage and cube-dimension errors gone. Moving performance remains poor. Owned-process sampling identifies repeated diagnostic getenv calls as a substantial CPU cost.

Blocked: none. Broader visual variants, slowdown and intermittent damage-volume crash remain open.

Next: cache process diagnostic options, build/deliver, repeat moving60/unlocked, and profile remaining hot paths. T127 remains in progress.

Files touched: native D3D11Effect texture binding, owned cube_binding_checks and D3D9 sampling reference, owned probe include/check hunks, phase analysis/board/ledger. Private artifacts remain under ignored _work/.


## 2026-10-03 01:20 — session Codex visual diagnosis — T127

Did: cache hot launch-time native diagnostic options and retain this session's bounded GPU evidence helpers. Found in / Belongs to Blur1PCDX11.

Verified: standard diagnostic_cache_build errsum0 and delivered; indexed2/2 translation units compile. Queued moving60/unlocked exit0; further debug-enabled60 exits0 without validation errors. Environment scanning drops from the leading sampled costs. Car reaches132mph. Dynamic vertex staging maps now dominate the unlocked profile (665/1275 caller stacks).

Blocked: none. Native dynamic locks ignore the pristine caller's discard/no-overwrite flags; visual parity and damage-volume crash remain open.

Next: finish the native dynamic-lock repair and queued GPU retention regression, then consume moving60/unlocked verification and profile remaining cost. T127 remains in progress.

Files touched: owned NativeFX11 cache hunks, owned D3D11GameDeviceNative diagnostics (separate alpha-test edits excluded), phase analysis/board/ledger. Private artifacts remain under ignored _work/.


## 2026-10-03 01:39 — session Codex visual diagnosis — T127

Did: forward retail discard/no-overwrite promises to native dynamic vertex/index maps, removing staging readbacks. Add the standard GPU regression. Repair the reported private delivery/console wrapper. Found in / Belongs to Blur1PCDX11.

Verified: standard builds errsum0; indexed2/2 translation units compile;64 queued GPU draws pass on hardware and WARP without debug errors. Standard EXE/PDB/effects delivery verified. Moving60/unlocked and corrected-wrapper debug60/unlocked all exit0, no previous validation/lock failures. Locked-file failure prevents launch; helper output is private. Final still reaches131mph. Remaining texture-view creation cost profiled.

Blocked: none. Broader visual parity and intermittent damage-volume crash remain open.

Next: build/test the current-view reuse candidate, compare fresh/reused hardware/WARP sampling across resource/gamma/null/reload transitions, then verify moving60/unlocked and profile. T127 remains in progress.

Files touched: owned D3D11Context lock hunks, owned cmake diagnostic target, dynamic_buffer_checks/reference, phase analysis/board/ledger. Private harness remains under ignored _work/.


## 2026-10-03 01:53 — session Codex visual diagnosis — T127

Did: reuse the FX variable's retained native texture view when resource/device/active descriptor match. Preserve gamma and resource changes, null binding and reload. Found in / Belongs to Blur1PCDX11.

Verified: standard build and diagnostic target errsum0/exit0; indexed4/4 source units compile;84 fresh/reused GPU comparisons pass on hardware and WARP without debug errors. Standard EXE/PDB delivery verified. Moving debug60/unlocked exit0/46s/wait0, no previous validation/lock/view failures. Final still reaches134mph. Unlocked engine reports roughly25–32fps; parity remains incomplete.

Blocked: none. Broader variants and intermittent damage-volume crash remain open.

Next: cache repeated reflected variable lookups with effect-reload invalidation; verify uniform/resource/reload behavior on hardware/WARP, then moving60/unlocked and remaining CPU costs. T127 remains in progress.

Files touched: owned D3D11Effect/Formats headers and implementations, owned NativeFX11 binding hunk and cmake diagnostic target, texture_view_reference, phase analysis/board/ledger. Private artifacts remain under ignored _work/.


## 2026-10-03 02:09 — session Codex visual diagnosis — T127

Did: index stable native FX reflection at load; preserve alias and absent-name behavior, clear on reload, keep render-time reads immutable. Found in / Belongs to Blur1PCDX11. The user's reported private copy/visible-console fault was repaired earlier in this continuation.

Verified: standard build/diagnostic target errsum0/exit0; indexed2/2 units compile; hardware/WARP84 paired view comparisons plus12 changed-uniform GPU draws pass, including renamed/missing variables on reload and vehicle alias. EXE/PDB delivered and hashes match. Moving debug60/unlocked exit0/46s/47s/wait0, no validation/lock/view failures; coherent131mph still. GetVariableByName disappears from the owned main-thread sample. Rendering remains25.1–29.0fps in the profiled moving fixture.

Blocked: none. Performance parity, broader variants and intermittent damage-volume crash remain open.

Next: instrument remaining SRV creation reasons in a bounded opt-in fixture, distinguishing live resource/gamma changes from cube/2D defaults; use recovered binding behavior to remove proven redundant work. Then hardware/WARP and moving60/unlocked verification. T127 remains in progress.

Files touched: owned D3D11Effect header/implementation, texture_view_reference, phase analysis/board/ledger. Private artifacts remain under ignored _work/.


## 2026-10-03 03:09 — session Codex visual diagnosis — T127

Did: finish bounded SRV cause statistics and actual-input crash composition replay; focus on clarified intermittent geometry beyond the right barrier. Native reproduction saved; no guessed visual fix.

Verified: clean standard build errsum0 and delivery hashes match; selected PC/native crash replay max1 byte on hardware/WARP, including unlocked capture. PC compressed reference exit0. Three scrape traces exit0; sampled pixels miss the fault. Opt-in colour trigger builds errsum0; refining its first crash-fringe false positive to target horizontal strips.

Blocked: none. Affecting draw/root cause unconfirmed; parity remains incomplete.

Next: catch an affected strip, identify its GPU producer and compare pristine PC mechanism; repair and verify60/unlocked, remove temporary diagnostics, deliver and commit own changes.

Files touched: own temporary native Present diagnostic and phase analysis/board/ledger; private captures/replays/logs under ignored _work/.


## 2026-10-03 — session Codex visual diagnosis — T127 barrier scrape repair

Did: identify stale GPU particle vertices behind the saturated barrier strips; recover retail's true pass-flush default and restore it under the actual native DX11 build guard. Remove temporary diagnostics and the failed shared-range publication experiment. Found in / Belongs to Blur1PCDX11; PC own-executable paired reference remains clean.

Verified: explicit native false/true isolation reproduces/clears malformed geometry; retail constructor/render/flush functions recovered read-only. Corrected standard Release errsum0; compiled native object confirms true and the flush call. Standard EXE/PDB delivered with matching hashes. Four final queued early/late scrape runs, 60-cap/unlocked, exit0/199s/wait0; four inspected stills clean with no debug configuration override or DX11 diagnostic environment. Actual render rate below60; performance gain from this glitch is not established.

Blocked: none for this repair. The initial wrong build macro excluded the attempted default change; its runs were not validation of the final fix. Broader T127 parity, uncaptured variants and intermittent damage-volume crash remain open.

Next: assess existing material SRV creation statistics and recover safe ownership/invalidation before further performance changes; verify any resulting change on hardware/WARP and moving60/unlocked.

Files touched: own dynamic/source/View.cpp gate/default and phase analysis/board/ledger. Private captures, retail recovery and discarded experiment artifacts remain under ignored _work/ only. Commit own repair/docs only, as previously authorized; no push.


## 2026-10-03 — session Codex visual diagnosis — T127 texture ownership and reported recurrence

Did: retain equivalent linear/sRGB native views for the real engine texture lifetime, including serialized assets and streaming replacements. Investigate the user's renewed right-railing glitch; add and then fully remove temporary locked-buffer and compressed-video diagnostics. Preserve the recovered retail pass-flush default; no speculative visual fix added.

Verified: hardware/WARP each pass228 paired GPU comparisons,12 uniform checks,12 destruction markers. Standard asset build and clean diagnostic-removal build errsum0. Moving cap60/unlocked pair exit0/103s; completed locked-buffer diagnostic pair exit0/113s, no still-locked draw comparison triggered. Two compressed videos,720 frames each, ffmpeg/game exit0/143s. Inspected strongest colour candidates do not identify the original malformed strips. Clean standard EXE/PDB delivered through queue and hashes verified. Runtime rates remain variable; no controlled overall speedup established.

Blocked: the returning visual fault is not yet isolated. The earlier clean stills are insufficient to claim it fully repaired. Diagnostic attempts also encountered a loading resource wait timeout and a vehicle-reset AI null crash; the first full-size recorder attempt failed, then was bounded and verified. All diagnostic source changes removed. Broader parity remains open.

Next: obtain an affected frame behind the right railing, capture its producer and compare actual GPU vertex/state data with the PC mechanism. Commit only the verified view ownership change and these owned docs, as already authorized; no push.

Files touched: D3D11Formats.h/cpp, PCD3D11Device.cpp, D3D11TextureLoad.cpp, dx11_probe/texture_view_reference.cpp, two-line native helper dependency in cmake/dx11.cmake, owned phase docs. Temporary diagnostics removed from DynamicVertexBuffer.cpp and PCD3D11GameDevice.cpp; scratch evidence under ignored _work/.


## 2026-10-03 — session Codex visual diagnosis — T127 railing-focused follow-up

Verified: clean delivered executable, queued scrape_recurrence_barrier_oct03 exit0/51s/wait0; existing trace samples(.95,.49) at40s, no saturated producer at that pixel; final still lacks the earlier malformed strips. Texture ownership committed e03536b0. No source changes or diagnostic instrumentation added. Board remains in progress.

Next: await confirmation whether the displayed unlocked-video pink band is the returning fault, then trace its producer with an affected frame. Recurrence remains open; a clean selected pixel is not a complete repair. Evidence under ignored _work/build/scrape_recurrence_barrier_oct03.


## 2026-10-03 — session Codex visual diagnosis — T127 racing lookup and PC scrape reference

Did: measure actual racing CPU samples, hash reflected parameter names at load using FX11-owned string lifetime and exact equality; preserve reload, aliases and missing-name semantics. Capture a fresh lower-PC wall-scrape compressed reference without lower-phase edits. Correct older7-frame reference coverage.

Verified: isolated lookup medians approximately4x faster with identical hit counts; racing FindVariable sample counts reduced at60-cap/unlocked. Standard Release exit0/errsum0;693 builtin shaders compile. Queued hardware/WARP broad probe and owner-view GPU checks pass. Moving cap60/unlocked exit0/101s; standard EXE/PDB delivered with matching hashes. Lower-PC reference exit0/49s,600frames spanning9.98seconds; inspected still has normal right-wall sparks. No whole-game FPS claim.

Blocked: none for verified optimization. Returning right-railing geometry remains unisolated; clean stills/reference do not prove it fixed. Broad parity remains incomplete.

Next: isolate the GPU producer in a confirmed affected right-railing frame; independently audit remaining EffectKey normalization cost. Commit own verified optimization and notes as authorized, no push.

Files touched: D3D11Effect.h/cpp and own phase board/analysis/ledger additions. Private profiles, lookup comparison, GPU logs and PC compressed reference under ignored _work/build/racing_profile*_oct03 and racing_reference_pc_oct03; no retained engine diagnostics.


## 2026-10-03 — session Codex visual diagnosis — T127 early-race railing sample

Verified: lookup repair committed7121dd08; queued unlocked early-race scrape trace32s, pixel(.95,.49), one still, exit0/41s/wait0. No original saturated strip in this selected sample. No source changes or diagnostic instrumentation added. T127 board remains in progress with recurrence unresolved.

Next: obtain confirmed affected-frame evidence and isolate the GPU producer behind the right railing; selected clean samples are insufficient to claim a repair. Private evidence under ignored _work/build/scrape_recurrence_early_railing_oct03.


## 2026-10-03 — session Codex visual diagnosis — T127 extended right-railing checks

Did: reject and remove the EffectKey normalization experiment after complete API benchmarks show no benefit. Confirm the reported fault is behind the right railing only. Capture longer compressed Promenade scrapes with a temporary native recorder, then remove that recorder exactly. Explicitly select verified retail routes and compare Amboy against the lower PC executable.

Verified: normalization reference bytes identical but controlled complete benchmarks regress; pre-edit source bytes restored. Promenade60/unlocked recordings each1800frames, game/ffmpeg/queue exit0,160s/wait0; bounded colour candidates and early scrape frames do not reproduce the original strip. Amboy_1 PC60/native60/native unlocked exit0,81s/wait128 with AI routes27; coherent inspected stills. Final clean standard Release exit0/errsum0; normal EXE/PDB delivery hashes match. Final clean moving60/unlocked Promenade pair exit0,82s/wait0; early stills lack the strip.

Blocked: recurring right-railing producer remains unisolated; clean captures do not establish a fix. Initial debug route0 selections lack AI data and crash on vehicle reset; corrected fixtures explicitly use route1. No AI or installed-data change and no retained diagnostic/fix attempt.

Next: obtain a confirmed affected right-railing frame and compare its actual GPU producer with the PC mechanism; continue valid-route vehicle/reflection coverage independently. Keep parity goal/T127 in progress. Commit own verification notes as authorized; no push.

Files touched: own phase board/analysis/ledger additions only after exact source cleanup. Ignored evidence: effect_key_oct03, railing_followup_oct03, railing_video_valid_oct03, amboy_valid_reference_oct03, railing_clean_delivery_oct03.


## 2026-10-03 — session Codex visual diagnosis — T127 packed skin indices and included-effect dependencies

Found in / Belongs to Blur1PCDX11. User correction retained: the returning coloured geometry is behind the right railing, never above the mirror. That recurrence is still open. Independently isolated a native-only Haruna skinned fragment to Default_Skin draw1003. Retail SM3 replay and actual captured UBYTE4 input establish integer/float shader input mismatch; corrected native bone-index declarations in both Common/Parameters.h copies and MeshEffect.fx, preserving DX9 input and recovered palette arithmetic. No diagnostic engine hook remains.

Verified: broken baseline183 coverage differences; corrected retail Default_Skin hardware byte-exact and WARP2 coverage-edge differences/shared colour error<=1. Separate MeshEffect vertex geometry reference hardware byte-exact/WARP2 edges/shared error0. Both probes pass after complete regeneration. Corrected Haruna60/unlocked fragment ROI0 dark pixels versus pre-fix75/73 and PC0. Initial four normal races exit0/325s, final moving Promenade60/unlocked queue exit0/80s/wait0, selected railing stills clean without proving the intermittent report repaired.

Also fixed included .fx timestamp invalidation in native compile_manifest.py after a variant audit found60 stale MeshEffect wrappers. Real-FXC regression: editing only included Shared.fx changes binary hash, unchanged third run reuses output hash/mtime; independently staged HEAD-based script passes too. Complete standard build exit0/errsum0;1254 native FX11 variants and693 native builtins compiled, zero failures; all760 generated skinned declarations uint4. Verified normal EXE/PDB delivery and all776 generated skin-named FXO data files. EXE SHA2567ed96945f2e7368ffff71ed35ccf2ffa45b1986e4ee7dacf08030ca7889eec2f; PDB cc033d2bab1011f5397bf8b6b0dd99799c3841b9321c07595bbfcf7afcc582db. Evidence under _work/build/{skin_index_replay_oct03,mesh_skin_replay_oct03,fx_include_dependency_oct03,skin_index_complete_delivery_oct03}.

Blockers: none for these verified repairs. Original right-railing recurrence lacks a confirmed affected frame in current captures; no attribution to skinning is asserted. Next: capture an affected railing frame and identify its GPU producer, then compare recovered PC/retail mechanism. T127 and parity goal remain in progress. Commit only these source/documentation hunks as authorized; no push.


## 2026-10-03 — session Codex visual diagnosis — T127 original scrape conditions rechecked

Verified repairs committed8f01a6a4. User reports no visible artifact in current runs. Original30-second scrape, original debug settings, valid UK_Brighton_1: queued60-cap/unlocked normal executable races both exit0, queue108seconds/wait0/no timeout; inspected full-size right-railing stills clean and saved logs contain no native validation/linkage/lock failures. Evidence: _work/build/railing_original_conditions_oct03. No engine changes or instrumentation added.

Next: continue remaining native/DX9 variant parity; capture exact GPU producer if the intermittent right-railing strip recurs. T127/parity goal remain in progress; current clean observations do not establish that the separate skin repair caused the original strip. Standard delivered executable/shader data remain the verified8f01a6a4 repair. Commit this bounded follow-up as authorized; no push.

## 2026-10-03 — session Codex visual diagnosis — T127 city coverage and alpha audit

Six Observatory/Gracia PC60/native60/unlocked normal races exit0, queued206s/wait0, valid AI data, selected stills compared. User observes no artifact; intermittent right-railing report remains unproven. Separate shrub lead: captured Billboard actual after-draw alpha enable0 matches PC schedule, no visual causality asserted.

Private D3D9/native hardware/WARP matrix proves65280/524288 fractional-alpha coverage differences on UNORM. Actual quantized-byte comparison matches D3D9 exactly; GPU float sampling byte-exact and native hardware UNORM output alpha bytes match PC. Float-target comparison instead exactly matches float32(reference)*float32(1/255), all524288 samples; direct division misses504. Last probe exit0/1s/wait0; last selected shrub capture exit0/33s/wait0. Evidence _work/build/{parity_route_inventory_oct03,parity_city_reference_oct03,observatory_shrub_trace_oct03,observatory_shrub_inputs_oct03,alpha_fractional_reference_oct03}.

No production edits or guessed alpha quantizer; no new engine diagnostics. Standard delivery remains8f01a6a4. Blocker for alpha repair: format-dependent conversion not fully recovered; other parity work remains possible. Next: extend references across float16/other UNORM targets, recover exact comparison, implement and validate60/unlocked; independently replay shrub producer before modifying foliage. T127/full parity goal remain active. Commit only own notes/board as authorized; no push.

## 2026-10-03 — session Claude DX11 continuation — T127 alpha-test format contract

Did: repaired double-encoded UTF-8 in the phase docs and committed the pending tree as `ccb9a5f3` at the user's request (T148 recovered vehicle techniques, per-draw `BindAlphaTest` placement, Sept 29-30 notes). Recovered the D3D9 alpha-test comparison for every measured target format (analysis.md) and implemented it natively: `alpha_test.py` `BlurPCAlphaConvert`/`BlurPCAlphaFormat`, `AlphaTestFormat` in `BindRenderTargets`, reference `r * (1/255)`.

Verified: D3D9 matrix fit 0/524,288 mismatches on nine formats; native hardware and WARP replay 0 mismatches on all nine; control without conversion fails as expected. Standard build errsum 0, all 4,913 indexed effects regenerated; EXE/PDB/effects delivered and hash-verified. Queued Observatory native60/unlocked exit 0, no unmeasured target format. Evidence: `_work/build/{alpha_format_matrix_oct03,alpha_native_matrix_oct03,alpha_format_race_oct03}`.

Blocked: none. Other-vendor D3D9 alpha behaviour remains unmeasured.

Next: the Observatory billboard shrubs draw a darker full-quad rectangle on native (PC is clean). Compare the Billboard draw sequence and its non-alpha-tested writes against the D3D9 replay.

Files touched: tools/dx11_shadercompile/alpha_test.py, display device d3d11/D3D11GameDeviceNative.cpp, docs/blur1pcdx11/{analysis,LEDGER,BOARD}.md.

## 2026-10-03 — session Claude DX11 continuation — T127 shipped prepass clip

Did: committed the alpha-test format contract as `fc50e064`. Let the opt-in `BLUR_DX11_DUMP_FX` dump accept `*`, dumped all 3,336 retail shaders from an Observatory race, and found that every shipped non-opaque `PrepassPS` kills `alpha*lodfade < 0.5`, which the leaked source lacks. Added that clip to `Common/Pixel.h` under `BLUR_DX11`.

Verified: retail/native discard comparison matches for all 115 indexed prepass passes; standard build errsum 0, 1254/1254 effects; EXE/PDB/effects delivered with hash checks; queued Observatory native60/unlocked exit 0 with no billboard shrub rectangles. Evidence `_work/build/{retail_fx_dump_oct03,alpha_format_race_oct03}`.

Blocked: none.

Next: audit the retail pass dump against source compiled for ps_3_0/vs_3_0 to find further shipped shader revisions.

Files touched: code/amax/game/Shaders/Common/Pixel.h, fx/source/System.cpp, docs/blur1pcdx11/{analysis,LEDGER,BOARD}.md.

## 2026-10-03 — session Claude DX11 continuation — T127 shader revision audit

Did: committed the prepass alpha clip as `b5d31dcd`. Built `tools/dx11_shader_audit/`, which compiles source passes with the retail-era D3DCompiler_42 and compares them with the retail dump. Recovered under `BLUR_DX11`: the depth-sample `abs`, the water `*NoDepth` techniques, the flare radius saturate, white SyntheticBillboard vertex colour, MeshEffect skinned bind-normal fresnel, and the road `_NW` zero normal with D3D9 `nrm(0)` semantics.

Verified: native audit 1,197/1,288 identical, with the remainder explained interface differences. D3D9/D3D11 zero-normal probes match on hardware. Standard build errsum 0, 1254/1254 effects; EXE/PDB/effects delivered. Brighton detail 1 (PC/native60/unlocked) and Observatory native60/unlocked exit 0. Evidence `_work/build/{shader_audit_oct03,shader_audit_dx11_oct03,road_nw_zero_normal_oct03,shader_revisions_oct03}`.

Blocked: the Blur1PC D3D9 capture at graphical detail 1 is black (Blur1PC fault, not fixed here).

Next: dump retail shaders on other tracks (water, night, perks) and rerun the audit; trace the 8 "VS object has no native shader" draws (caller 0x01499128).

Files touched: Shaders/{Road.fx,Road.h,RoadTwoSpec.fx,Water.fx,WaterCubeMap.fx,WaterFlow.fx,SyntheticBillboard.fx,Effects/LightFlare.fx,Perks/MeshEffect.fx}, shader/misc/Resolve.h, tools/dx11_shader_audit/, docs/blur1pcdx11/{analysis,LEDGER,BOARD}.md.

## 2026-10-03 — session Claude DX11 continuation — T127 native built-in renderers

Did: committed the shader revision audit as `72c52590`/`8b30804f`. Traced the 8 per-run "VS object has no native shader" draws to `C_BasicRender::BindColoured3D` through a module-relative caller in the diagnostic. Enabled embedded built-in shaders for BasicRender, FontRender and TextureRender under `DEVICE_PCD3D11`.

Verified: standard build errsum 0; queued Observatory native60 exit 0 with every renderer shader created and no missing-shader/open-file errors. Evidence `_work/build/vs_rva_oct03`.

Next: final regression at native60/unlocked; extend the retail shader dump/audit to other tracks.

Files touched: display/source/render/{BasicRender,FontRender,TextureRender}.cpp, device/d3d11/D3D11GameDeviceNative.cpp (diagnostic), docs/blur1pcdx11/{analysis,LEDGER}.md.

## 2026-10-03 — session Claude DX11 continuation — T127 render-texture init and route regression

Did: fixed the native `C_RenderTexture2D` constructor, which left its slot index uninitialized; enabling BasicRender exposed this as a boot-movie crash on Downtown. Confirmed the retail effect dump is complete (all effects preload).

Verified: standard build errsum 0; 14 queued races (11 routes native60, 3 unlocked) exit 0 with clean logs and coherent stills. Evidence `_work/build/{retail_fx_dump_routes_oct03,route_regression_oct03}`.

Next: per-pass GPU parity for audited shaders is now limited to the explained interface differences; continue with remaining visual/state parity (Blur1PC detail-1 black frame belongs to Blur1PC).

Files touched: display2/source/context/D3D11Context.cpp, docs/blur1pcdx11/{analysis,LEDGER}.md.

## 2026-10-03 — session Claude DX11 continuation — T127 resolution, MSAA and device reset

Did: user reported a low internal resolution. Native `C_DeviceParams::Setup` overwrote the configured 1920x1080 and MSAA with 1280x720 and no MSAA; it is now a PC no-op, as in D3D9. Implemented native `CreateDeviceVolatile`/`DestroyDeviceVolatile` (swap chain resize and fullscreen state, back buffer and window targets). Fixed the MSAA resolve, which used a typeless format.

Verified: standard build errsum 0; 1920x1080 MSAA 3/0 targets correct; MSAA edges resolved; runtime switch to 1600x900 works; MSAA 3 unlocked exits 0. Fullscreen not automated (unverified). Evidence `_work/build/lowres_title_oct03`.

Next: crowd-flag built-in VS/PS linkage errors; user check of fullscreen start.

Files touched: device/d3d11/{D3D11GameDevice.cpp,PCD3D11GameDevice.cpp}, docs/blur1pcdx11/{analysis,LEDGER}.md.

## 2026-10-03 — session Claude DX11 continuation — T127 MSAA sample count

Did: native surfaces created 1 sample for any multisample option other than 2x/4x, and the user's option 3 is 8x. They now use the option's sample count. The target diagnostic logs sample counts.

Verified: standard build errsum 0; 8x targets created; matched no-AA/8x race captures show anti-aliased edges and equal brightness; 8x/4x debug-layer runs exit 0. The previous ledger claim that MSAA 3 edges were smoothed was wrong (1 sample).

Files touched: display2/source/device/PCD3D11Device.cpp, device/d3d11/D3D11GameDeviceNative.cpp (diagnostic), docs/blur1pcdx11/{analysis,LEDGER}.md.

## 2026-10-03 — session Claude DX11 continuation — T127 user confirmation

User confirmed the delivered build (`b27ce4f6`) works in their own fullscreen 1920x1080 8x MSAA setup: full resolution and anti-aliasing applied. This covers the fullscreen start that automated runs could not exercise.

## 2026-10-03 — session Claude DX11 — T127 shunt refraction rings black (investigation, stopped mid-task)

User report: shunt refraction rings render solid black (fullscreen 3840x2160, 8x MSAA, detail 4).
Measured in windowed 2564x1421 8x private runs (temporary env-gated traces and a player shunt gift, all reverted): rings/arches are `MeshEffectRefract / Material` (558 and 226 indices; arches use negative g_vColour); colour buffer slot = window resolve texture, resolved in RenderWindowMask before the refract pass, non-null on all 200 draws, mean ~95/255; constants sane; shader byte-identical to retail per the oct03 audit; before/after RT captures show the arches refracting correctly. Some stills still showed a dark red-veined arch, not yet attributed to a draw.
Not reproduced windowed. Remaining lead: fullscreen swap-chain reset recreates window targets (ResizeWindows); check whether the FX11 effect's cached g_tColourBuffer view (C_D3D11Effect::CreateTextureView reuses `previous`) can stay bound to a destroyed window texture. Checked: Display::D3D11::CreateTextureView reuses a view only when the view's own (referenced) resource is the bound one, so a recreated window texture cannot inherit a stale view; lead closed. The volatile reset (ResizeWindows) also runs in windowed runs. User later reported the black rings no longer occur; closed without an engine change. If it recurs, the open loose end is the dark red-veined arch seen in two stills before the refract pass.
No engine change; standard delivery unchanged.

## 2026-10-03 — session Claude DX11 perf — T187 DX11 frame rate (stopped mid-task, usage limit)

Bench: _work/t187/bench (1920x1080 windowed, vsync 0, BLUR2PC_FRAMELIMIT=0, debug-menu race start, 30 s stationary; queued via game_slot). Sampler with full stacks: _work/t187/prof.
Baseline: Blur1PC D3D9 400-525 fps; DX11 (b27ce4f6) 45-78 fps. Profile: FX binding (string-keyed lookups + a full FX11 pass Apply per immediate parameter set) ~55%; staging locks with an immediate GPU wait (HDR luminance readback ring, car light cube maps, crowd flag VB) ~16%.
Change 1 (uncommitted): NativeFX11 resolves each parameter name once per technique (interned name pointers) and defers the pass Apply to the device's next bind/draw (Display::D3D11::FlushNativeFX, called before every device shader/view/sampler/constant bind and draw; FX passes carry no render states). DX11 105-149 fps. Car body brightness in the start still now matches D3D9 (mean 101.2 vs 101.5; old DX11 119); cause not isolated.
Change 2 (uncommitted): persistent staging mirror per locked subresource of resources the GPU cannot render into (D3D9 MANAGED/SYSTEMMEM behaviour); GPU write sites call Display2::D3D11ResourceWritten to refresh mirrors. DX11 110-159 fps (one dip to 78); profiled run 122-173.
Both build errsum 0; delivered _work/run/Amax.exe a6a61655 (both changes). Not committed; no hardware/WARP probe or moving60/unlocked regression run yet.
Next: apply scratch patch batch2.py (lazy blend/depth/raster at draw with state-object caches, sampler cache, constant setters stop flushing FX early, cached context/state pointers, default vertex inputs bound once) and rebuild/bench. Then: RTV/DSV and non-owner SRV view caching (owner registry), FX11 redundant-bind filtering. Remaining profile: FX11 ApplyShaderBlock ~13%, legacy state churn ~15%.

## 2026-10-04 — session Claude DX11 perf — T187 continued (in progress)

Root cause of the user's ~50 fps: CPU cost in the native FX/lock paths (changes 1-2 above). After those, every DX11 run sat at exactly 144 fps (display refresh) at both 1280x720 and 1920x1080. Cause: PC D3D9 C_D3D9Device::SetPresentInterval is compiled out (#ifndef PLATFORM_PC) and PCD3D9Device presents with m_DeviceParams.m_PresentInterval (settings vsync); the native device stored the per-frame Graphics.Render.PresentInterval debug value (default 1) and always presented with sync interval 1. Native SetPresentInterval is now a no-op and Present uses the device parameter. Swap chain moved to FLIP_DISCARD (2 buffers, ALLOW_TEARING when supported, tearing flag only while not exclusive fullscreen; blt-model fallback if creation fails).
Further uncommitted changes: lazy blend/depth/raster objects applied at draw with descriptor caches, sampler state cache, constant setters no longer flush FX early, cached context/state pointers, default vertex inputs bound once; owned-texture SRV fast path; RTV/DSV cached per owned resource (surfaces now registered owners); FX11 SetRawValue skips identical writes and SetSampler skips an unchanged user sampler; dynamic buffers locked without DISCARD/NOOVERWRITE keep their contents on the CPU and upload with WRITE_DISCARD (particles); D3D9-style nested buffer locks (a particle "already locked" null lock crashed one run before this); per-parameter sampler memo and per-variable owned-view memo in NativeFX11; device alpha-test CB (b13) rebound only after an FX apply; 20 s FX apply trace gated by BLUR_DX11_TRACE_APPLY.
Same-batch 1920x1080 unlocked bench (debug race start, _work/t187/bench): DX9 639-667 fps, DX11 326-371 fps (was 45-78). Stills match DX9 as closely as before (mean abs diff ~12). All builds errsum 0; current build delivered to _work/run. Not committed; no hardware/WARP probe or moving60/unlocked regression run yet.
Remaining DX11-only cost (ms/frame vs DX9 profile): FX BeginPass +0.43, draws +0.42 (FX11 ApplyShaderBlock full-CB UpdateSubresource and rebinding), SetMaterial/SetPrimitiveStream +0.4.

Update (stopped, usage limit): further changes, all benchmarked and delivered except the last: legacy texture/sampler/pixel-shader binds skipped when the pass has a native FX pass (FxContext m_bNativePass, NativeFX11HasPass); FX11 ApplyShaderBlock skips identical VS/PS shader/CB/sampler/SRV binds (D3DX11InvalidateBoundState called at every device bind of those stages and at OMSetRenderTargets); topology cached; VSGetShader guard only for legacy draws; per-draw Now() trace reordered; alpha-test state dirty only on change. Same-batch bench: DX9 602-660 fps, DX11 362-412 fps (~62%); still diff vs DX9 mean 9.7. Delivered build = the draw-path-fixes build.
Last change (compiles, errsum 0, NOT benchmarked or delivered): direct memo pointers in S_Param and a per-thread direct-mapped owned-view cache in CreateTextureView. Next: bench it; check the cache's generation check against loader-thread owner releases; then partial constant-buffer updates in FX11 and SetVertexDeclaration/input-layout caching. Then regression runs (dx11_probe hardware/WARP, moving60/unlocked) before commit.

## 2026-10-07 — session Codex continuation — T187 graphics regression repair

Did: resumed from current workspace and bounded historical transcript. User reported that performance changes broke fullscreen graphics; prioritized that repair. Reproduced initial fullscreen flip-chain creation at desktop 3840x2160 while engine/depth stayed at configured 1920x1080, with Present 887a0001 errors and automation timeout. Restored committed blt presentation for initial exclusive fullscreen; retained flip for initial windowed creation and kept the chosen model across resets. Also fixed the pending direct-mapped texture-view cache race by holding the owner mutex during generation validation and AddRef. Preserved other work and prior delivered EXE/PDB under _work/t187/visual_regression/prior_delivery. No commit.

Verified: standard tools/build.sh Release errsum 0; EXE/PDB delivered after previous process exit and hashes match standard outputs (EXE 03855B9926DC278F8A2EA7C8833BAA5E1B090C771DA6D8E0B5442EC774809D0D; PDB A1215ACF2367F72614D699264BAF30407D26AED2D382996A0A5FD6C8061BD989). Queued minimized private-settings Observatory moving races at 1920x1080, MSAA option3/8x, detail4: fullscreen60 exit0/36s, fullscreen unlocked and windowed unlocked exit0/70s combined. Complete menu/race captures inspected; no four-part image or severe banding visible, no Present errors or severity0/1/2 validation diagnostics. Unlocked race reports around96-113 render fps with simulation around60Hz; these 8x-AA moving runs are not comparable to prior no-AA stationary benchmarks. dx11_texture_view_reference rebuilt with errsum0; hardware/WARP each pass all six texture kinds, alternating retained/fresh GPU views, uniform changes and resource destruction (exit0/1s combined). git diff --check exit0. Evidence: _work/t187/visual_regression/.

Blocked: none for this repair. The original performance-parity task remains incomplete; these checks do not establish full graphics parity or benchmark the newest shortcuts against D3D9. Full dx11_probe suite was not run in this continuation.

Next: user can test the repaired standard delivery with unchanged player settings. For further T187 optimization, first benchmark identical fullscreen/windowed resolution and MSAA on D3D9 and DX11, retain this fullscreen regression fixture, then profile remaining cost and run full hardware/WARP probes before considering completion. Do not describe parity as achieved.

Files touched in this continuation: phase BOARD/LEDGER/analysis.md; display/device/d3d11/PCD3D11GameDevice.cpp; display/device/d3d11/D3D11Formats.cpp; ignored _work/t187/visual_regression. Other inherited uncommitted changes were preserved.

## 2026-10-07 — session Codex continuation — T187 countdown letterboxing repair

Did: reproduced the user's countdown mask fault, compared the lower-phase D3D9 rendering and actual D3D9 upload, and traced the native mask. Found in / Belongs to Blur1PCDX11. The16x16 DXT5 mask's256 packed bytes were written into a128-byte-pitch staging map instead of the PC64-byte-pitch layout, leaving its lower half transparent. Added packed compressed-texture lock storage and row copies in the native Display2 context. Preserved all inherited optimization work. Updated BOARD/analysis; no commit.

Verified: standard tools/build.sh Release exit0/errsum0. Delivered Amax.exe and Amax.pdb from standard Release to `_work/run/` after confirming no delivered process was active; both hashes match build outputs. Queued minimized countdown runs60Hz and unlocked exit0, inspected vertical and horizontal openings now correct. Selected-draw GPU capture exactly matches all256 bytes of the D3D9 mask; prior capture had88 differing bytes. Evidence `_work/t187/countdown/`, including baseline/reference/fixed captures, original D3D9 trace, build.log, fixed_trace/mask_compare.txt and prior_delivery backup. git diff --check passes.

Limits/blockers: no blocker for this targeted repair. Two pre-existing shader-linkage errors (id343) occur before the countdown in both baseline and repaired runs; selected-frame trace also retains input/output alias unbinding warnings. These are not evidence against the mask-byte repair, but broader correctness is not established. Full hardware/WARP suite and DXT1/DXT3-specific tests not rerun.

Next: user can test countdown with the standard delivery. Continue T187 performance work only with matched settings and full regression checks, retaining this countdown fixture because DebugRb normally skips it. Performance parity remains incomplete. Changes this request: native display2/context/D3D11Context.cpp packed DXT lock handling, phase BOARD/LEDGER/analysis.md, ignored `_work/t187/countdown/` evidence.

## 2026-10-07 � session Codex continuation � user-requested full workspace commit

Scope: the user explicitly requested committing everything, including the inherited T187 optimizations, fullscreen and texture-view lifetime repairs, countdown packed-DXT lock repair, phase documentation, and the existing HDR assessment. The HDR document remains an assessment, not approval to implement HDR. This is a checkpoint; T187 performance parity remains incomplete.

Verification: retained the completed standard Release build (errsum0), delivered EXE/PDB hash verification, 60Hz/unlocked visual checks, texture-view hardware/WARP checks and byte-exact countdown mask comparison recorded above. No engine edits after that build. git diff --check passes. Existing shader-linkage diagnostics and full-suite coverage limits remain as recorded. No push requested.

Next: benchmark matched D3D9/DX11 settings and run full hardware/WARP regression coverage before further optimization or claiming parity.


## 2026-10-07 — session Codex T187 assessment — remaining optimization candidates

Did: reviewed the current native renderer and T187 profiling history in response to the reported remaining 30% FPS gap. Recorded source-backed candidates in phase analysis: FX parameter traversal/full dirty-buffer uploads, redundant input-layout/stream binding, legacy feedback queries/copies and mode-dependent presentation. No engine edits or commit.

Verification: source inspection only; historical profile numbers explicitly separated from current evidence. No build/runtime checks needed for documentation. Current delivered performance has not been measured in this session.

Blockers: none for the assessment; current CPU/GPU/Present attribution remains unknown. Next: capture matched D3D9/DX11 current-delivery frame times and CPU/GPU/Present costs in the user's scenario before implementing the highest-cost candidate. T187 parity remains incomplete.


## 2026-10-07 - session Codex T187 optimization - upload/sampler pass delivered

Did: profiled current D3D9/DX11 render threads; implemented full WRITE_DISCARD uploads for ordinary FX11 constant buffers and epoch-invalidated per-slot legacy sampler memoization. Removed temporary GPU/Present instrumentation. Corrected GPU fixtures' ClearState/FX-cache invalidation and added exact readback of 128 queued alternating original/clone draws with partial uniform updates. Owning decisions and evidence: analysis.md/decisions.md; scratch _work/t187/oct07_perf/. No lower-phase edits, commit or push.

Verification: standard Release errsum0 and verified EXE/PDB delivery, matching hashes recorded in analysis. Fullscreen 1080p/8x-AA original/final stationary pairs average365.03/374.05 FPS (~2.5%, with run variation); D3D9 about488. Early windowed gains were larger but overlapping later controls prevent a stable large-gain claim. Final moving races pass at60 FPS and unlocked, stills inspected. Hardware/WARP texture/partial-uniform/clone/queued-constant and dynamic VB/IB checks pass; probe/reference builds errsum0; git diff --check passes. Existing crowd shader id343 errors remain. Broad probe fails the same billboard case with original and optimized uploads after correcting fixture resets; later suite coverage remains pending.

Blockers/limits: PresentMon ETW access denied; timestamp spans do not prove GPU busy time; one-car DebugRb is not the full Career grid. This optimization pass is complete and delivered; T187 parity is not complete. Exact next step: matched final/original exclusive-fullscreen8x-AA Career-grid profiling, attributing FX traversal/submission and GPU busy time; separately diagnose baseline billboard prepass failure to unblock broad shader coverage. Task ownership released for continuation.


## 2026-10-07 - session Codex T187 commit - user-requested optimization checkpoint

User requested committing the delivered upload/sampler optimization pass, GPU fixture corrections, queued original/clone constant readback, and owning-phase performance/decision records. Retained completed standard Release errsum0, verified EXE/PDB delivery, 60 FPS/unlocked driving and hardware/WARP checks. No engine edits since that verification; git diff --check passes. Fullscreen benchmark average improves about2.5%; parity remains open. Existing crowd linkage errors and baseline billboard full-suite failure remain documented. No push requested.

Next: matched final/original fullscreen8x-AA Career-grid profiling, separating FX submission/traversal and GPU busy time; diagnose the baseline billboard prepass failure before claiming broad shader coverage. Ownership released for continuation.


## 2026-10-07 - session Codex T187 assessment - remaining roughly30% FPS gap

Did: assessed current committed renderer against DX9 and ranked remaining causes in analysis.md. Found in / Belongs to Blur1PCDX11. Identified alpha-state b13 rebinding invalidating all PS constant-buffer slots, repeated FX binding/dependency traversal, native VB/IB setters missing DX9 equality checks, remaining legacy constant uploads, and unmeasured GPU alpha/resolve/composition costs. No engine, shader, settings, build or delivery changes; no commit or push.

Verified: two queued minimized fullscreen1080p/8x-AA, vsync-off, unlocked races on delivered DX11/DX9 executables exit0. Approximately ten-second main-thread profiles after initial diagnostics contain606/611 samples: BeginPass11.55/3.76%, Present0.50/25.20%; overlapping sample shares, not expected speedups. Native confirms hardware/debug0. Recomputed existing throughput excluding30s readbacks; gap persists. Source comparison and Microsoft API contracts checked. Scratch scripts/logs/profiles and metric summaries: _work/t187/oct07_assessment/. git diff --check passes.

Limits: one-car DebugRb is not full Career grid; sampling perturbs execution and ends at game exit before requested20s. No new throughput claim or GPU-busy attribution; ETW access unavailable. Existing broad billboard probe/crowd linkage limits remain. This requested assessment is complete; T187 performance parity remains open.

Next: quantify changed/redundant PS CB bindings around the b13 handoff and FX binding traversal, plus VB/IB/layout calls; validate targeted changes with alternating matched Career-grid runs and diagnostics disabled. Isolate GPU contribution with resolution/AA scaling and per-pass timings. Task ownership released.

Files touched: docs/blur1pcdx11/{analysis,BOARD,LEDGER}.md only.

## 2026-10-07 - session Codex T187 bindings - implementation delivered

Did: implemented pre-resolved FX function/shared sampler bindings, PS b13-only FX invalidation, and redundant native VB/IB/input-layout elision with reset/diagnostic invalidation. Extended queued GPU constants coverage. Recorded implementation, decisions and complete benchmark evidence in the owning phase. Released ownership; T187 parity remains open. No commit or push.
Verified: standard Release and probe builds (zero errors); hardware/WARP texture/queued-constants and dynamic-buffer probes (all exit 0); ten alternating fullscreen stationary runs plus 60 FPS/unlocked driving (all exit 0), reviewed stills, and hash-matched standard EXE/PDB delivery. Existing two id343 crowd-linkage errors persist in both driving checks. Median FPS 375.81 -> 387.52, but a retained unexplained 243.30 FPS new-build outlier makes the all-run mean worse; overall FPS gain remains inconclusive. Evidence: _work/t187/oct07_bindings/ and phase analysis.
Blocked: no implementation/delivery blocker; full Career-grid attribution and the intermittent slow launch remain unverified. Existing broader billboard/linkage limitations remain.
Next: reproduce a slow launch with CPU/driver and GPU telemetry; count attempted/changed FX and geometry bindings; run per-change A/B on the user's full Career grid with recurring diagnostics disabled before claiming a reliable gain or DX9 parity.
Files touched: NativeFX11.cpp; D3D11GameDevice.h/.cpp/Native.cpp; render/postprocess/Interface.cpp diagnostic boundary; vendor/fx11/EffectRuntime.cpp; dx11_probe/texture_view_reference.cpp; phase BOARD/LEDGER/analysis/decisions; ignored _work/t187/oct07_bindings and standard build/run outputs.

## 2026-10-07 - session Codex T187 commit - binding/input checkpoint

User requested committing the delivered binding/input optimization pass and owning-phase assessment, decisions and validation records. No engine edits since verification. Rechecked standard Release errsum0, hardware/WARP probe exit0 records, completed fullscreen comparisons and 60 FPS/unlocked driving, and matching standard build/delivered EXE/PDB SHA256 hashes. Staged diff whitespace check required before commit. No push requested.

The measured FPS gain remains inconclusive because the unexplained slow launch is retained; DX9 parity, full Career-grid attribution and existing crowd/billboard limitations remain open. Ownership released. Next: reproduce the slow launch with CPU/driver and GPU telemetry, then count bindings and run per-change A/B on the user's full Career grid.

## 2026-10-07 — session Codex HDR — T238

Did: created Orca branch/worktree `uKER/blur1pcdx11-hdr` from `blur1pcdx11` e46ee524. Implemented native display-aware FP16/scRGB HDR, preserved extended scene values through final grading and FP16 windows/composition, added arbitrary peak/paper-white calibration with persistence and SDR fallback, restored all presenter state including shader class instances, and added float screenshot evidence and a production GPU reference probe. Corrected the old HDR assessment and recorded owning-phase facts/decision. No commit or push. Original parent/lower-phase sources unchanged.

Verified: standard Release `tools/build.sh` errsum0; shader/header and native blob regeneration completed (retail693/693, no shader failures). Standard-tree `dx11_hdr_reference` hardware/WARP exit0, eight peak calibrations80..10000 plus fractional live calibration/disable/resize/state checks; synthetic dynamic linkage verified on WARP only. Four queued minimized debug-menu Observatory runs (HDR60 windowed, HDR unlocked fullscreen, SDR-off unlocked fullscreen, HDR unlocked1600x900/noAA reset) exit0. Final standard EXE/PDB delivered to this child's `_work/run` with native FX assets and SHA256 matches; final delivered-build60-cap repeat exit0, peak470 nits and10104 pixels above240-nit paper white. PFM floats finite with minimum0. Reviewed stills, final verification preview `_work/t238/hdr60/race.png`; actual float evidence `_work/t238/hdr60/race.tga.pfm`. `git diff --check` passes. Evidence/logs in `_work/t238` and analysis.md. The queued final repeat completed before an attempted cancellation; no process was stopped.

Blocked: no HDR implementation/build/delivery blocker. Existing two id343 crowd linkage errors remain in HDR and SDR, broader billboard/marker artifacts remain outside this task. Hardware access violation in the optional synthetic class-linkage probe fixture is unresolved; WARP coverage is explicit. Other-panel visual appearance, monitor migration/live Windows HDR toggles, sustained luminance and performance impact are unverified.

Next: launch this child worktree's play.bat with Windows HDR enabled and review the physical panel. If metadata disagrees with measured display output, set Graphics.Render.HDR.Display.PeakNits and PaperWhiteNits through existing debug variables or documented launch overrides. Optional next validation: migrate the game between HDR/SDR displays and toggle Windows HDR during a race. Commit only if requested.

Files touched: native PC DX11 device/header and base native constant-binding hook; new D3D11HDROutput/D3D11HDRShaders headers; psToneMapCustom; Screenshot header/source; dx11_probe/hdr_output_reference; cmake/dx11.cmake (source list regenerated, unchanged); phase BOARD/LEDGER/analysis/decisions/hdr.md; ignored _work build/run/t238 assets and logs.

## 2026-10-07 — session Codex HDR — T238 emissive/sky follow-up

Did: reopened after the user's correct-launcher report of no visible HDR. Promoted reduced particle/lens-flare and postprocess downsample/bloom targets to FP16 for the floating output container. Removed racification's RGB upper clamp only with active HDR, preserved native FX b-registers through preprocessing/compilation, and bound HDR b12 after FX buffer flush. Added executable-local hdr_output.log for ordinary launches. Extended the GPU probe to actual sky/grade/presenter shaders and the actual racification FX5 pass, including binding reflection and rejection of shader fixture debug errors. No guessed source intensity boost. Lamppost square artifacts deferred as requested. Switched approval scenes to Tokyo Shutoko and LA Downtown as requested.

Verified: latest standard Release build errsum0, shader/header/blob regeneration; generated24 racification variants retain b12. Production GPU references hardware/WARP exit0 at peaks80..10000 and fractional calibration; actual sky radiance0/1/4/16 produces0/200/646.875/990 nits at white200/peak1000; actual FX5 RGB4/2/1 stays extended in HDR and clamps1/1/1 in SDR. Older delivered FX5 fails b12 regression. Initial fixture resource-slot/signature mismatches corrected before accepting results. Tokyo60, Tokyo unlocked fullscreen, Tokyo SDR off, Tokyo resize to1600x900/noAA, Downtown unlocked HDR and start-screen showroom all exit0, no queued DX11 validation messages. HDR data finite/nonnegative. Tokyo lamp441.46, lit windows310.20, neon425.50 and pickup447.49 photometric nits against white240; body median45.27. Source-specific measurements/caveats in analysis.md; full metrics `_work/t238/followup_results.json`.

Delivered: standard Amax.exe/PDB and native FX assets to this child's `_work/run`, matching EXE/PDB hashes in `delivery_lights_final.log`. Same child play.bat. Current verification preview `_work/t238/tokyo60/race.png` and float race.tga.pfm; brighter-sky scene `_work/t238/downtown_hdr/race.png`. Previews are clipped SDR and do not prove physical HDR appearance. `git diff --check` passes. No commit/push; original worktree clean.

Outstanding: physical HDR visual review after the user's report remains required; task is implemented/delivered awaiting that input. Sky range is preserved but sampled retail clouds remain below paper white; no claim every asset/lighting state produces HDR highlights. Synthetic dynamic class-linkage fixture remains WARP-only as previously documented. Cross-display/live Windows-HDR-toggle behavior, sustained panel brightness and performance impact remain unverified.

Exact next step: relaunch `C:/Users/uKER/orca/workspaces/Blur2PC/blur1pcdx11-hdr/play.bat` with Windows HDR enabled and review Tokyo Shutoko windows/neon/pickups/car lights and LA Downtown sky on the panel. If expected sources still look flat, collect this build's hdr_output.log plus the exact route/light and trace that source through scene/grade output; do not infer success from container maxima. Commit only on request.


## 2026-10-08 — session Codex HDR — T238 lamp-square fix

Did: investigated the user's startup lamp-head blocks and gameplay report. Whole-target FP16 scanning corrected an initial blob-shadow hypothesis to Default/MaterialLights exposure gamma. Matched actual retail FXB rsq/rcp semantics in native shader/common/Hdr.h using sqrt(abs(value)); preserved extended positive radiance, MSAA and existing exposure timing. Regenerated built-ins/native FX assets. Removed all temporary device/FX tracing and diagnostic shader replacements before final delivery.

Verified: parent startup and true HDR-off startup clean; HDR+8x reproduces. Actual pass textures/constants finite, staged shader diagnostic isolates exposure. Standard final build errsum0,693/693 built-ins. Queued HDR+8x startup60/unlocked finite scans:389/388 draws, no scene/DOF/bloom invalid RGB; separately recorded small luminance-reduction nonfinite values remain unverified. Tokyo Shutoko unlocked moving scan:944 checked draws, all RGB finite. Final production startup/unlocked Tokyo and SDR-off60 startup each exit0, no queued DX11 validation messages, reviewed stills clean. Final presented HDR PFM finite/nonnegative. Details and numeric limits in analysis.md and `_work/t238/square_fix_results.json`.

Delivered: standard EXE/PDB and generated native effects to this child's `_work/run`. Matching executable hash F9BCC4896FE62F4EA7CC91DC3DFF8F4A73F40635E57C1FDDD76D0DA622DFA244; matching PDB hashes in `_work/t238/delivery_square_fix_final.log`. Verification screenshots: `_work/t238/squares_final_startup/startup.png`, `_work/t238/squares_final_tokyo/race.png`. Same child play.bat. No commit/push or parent changes.

Outstanding: broader physical HDR review and the user's as-yet undescribed second issue. Startup luminance intermediate nonfinite values are recorded unknowns; do not infer they caused these lamp squares or introduce guessed clamps.

Exact next step: user relaunches `C:/Users/uKER/orca/workspaces/Blur2PC/blur1pcdx11-hdr/play.bat` to review the fixed lamp heads, then describes the second issue. If blocks recur, capture the exact scene/settings and locate the first invalid actual pass with the preserved diagnostic backups under this child's _work/t238; do not compare draw ordinals between separate runs. Commit only if requested.


## 2026-10-08 — session Codex HDR — T238 user-approved commit

Did: user confirms the second manifestation is gone and requests commit. Reviewed and staged only the HDR implementation, probes, regenerated CMake source list and owning phase docs on uKER/blur1pcdx11-hdr. Marked T238 complete for the delivered implementation and confirmed artifact repair.

Verified: previous final standard build errsum0 and delivered EXE/PDB remain current; no source changes since the successful startup60/unlocked, Tokyo moving and SDR fallback checks. User confirms artifacts no longer occur. Staged whitespace check passes; temporary tracing is absent. Verification stills: _work/t238/squares_final_startup/startup.png and _work/t238/squares_final_tokyo/race.png. Existing numeric/hardware limits and startup luminance intermediate unknowns remain recorded in analysis.md.

Outstanding: none for this requested commit. Cross-display and sustained-panel behavior remain documented verification limits. No parent/lower-phase edits, merge or push.

Exact next step: report the resulting HDR-child commit; continue only when the user requests further changes.


## 2026-10-08 - session Codex HDR range audit - T238 remaining over-white limits

Did: reviewed the HDR child at 2fbde954 at the user's request. Traced active scene/effects/reflection/postprocess/presentation storage and relevant shader clamps. Found three reachable raw-RGB A8 intermediates: point/spot-light prepass accumulation, both planar-reflection blur targets, and bloom feedback history. Also documented vehicle SH saturation, the conditional interior-glass reflection ceiling and the RGBM vehicle-light cubemap's maximum of eight. Recorded exact source paths, consumers, scope and exclusions in analysis.md. Found in / Belongs to DX11 HDR child. Phase docs only; no implementation, generated assets, runtime settings or delivery changed.

Verified: source allocation-to-consumer tracing and native UNORM mapping; recovered retail vehicle dp4_sat operations and compiler preference; active DOF/motion-blur inheritance of FP16. git diff --check passes. No new build or runtime test was needed for this audit. Actual above-limit input frequency and visual severity remain unmeasured; no new verification screenshot exists, and prior HDR screenshots are not evidence for these pass-specific findings. Startup luminance nonfinite-value unknown remains separate.

Outstanding: requested audit complete; identified range limits remain unfixed. No commit, push or parent/lower-phase changes. No blocker to reporting the source findings.

Exact next step: report the three storage ceilings and separate material limits. If implementation is requested, capture ranges for the three actual passes, preserve HDR range through their creation/reset paths, and verify at 60 Hz and unlocked rendering with SDR comparison; evaluate retail vehicle material bounds separately before changing their semantics.


## 2026-10-08 - session Codex HDR range fixes - T238 complete

- Owner: session Codex HDR range fixes. Found in / Belongs to DX11 HDR child. User requested fixing all six source-audited limits; no lower-phase fault correction or commit/push requested.
- Changed dynamic-light accumulation, planar reflection blur and bloom history to FP16 for the float native container. Extended vehicle light RGBM using RGBA16F and actual worker row pitch; original decoder retained. Shared vehicle SH/interior-glass helpers remove upper bounds only with active HDR; all 102 recovered vehicle FX sources receive the targeted SH extension at generation. Native HDR state binds VS/PS b12 and presentation restores both independently.
- Verification: standard Release and HDR probe builds exit0/errsum0; 693 built-ins regenerate without failures; hardware/WARP production-shader range references pass, old vehicle negative control fails as intended. Tokyo moving gameplay and HDR startup at60/unlocked, HDR-off startup60 and resolution/MSAA recreation all exit0 with no DX11 validation messages; final HDR readbacks finite/nonnegative. Source/SDR-helper checks retain retail bounds. Details and limits in analysis.md.
- Delivered Amax.exe/PDB and 4985 generated FXOs to _work/run; SHA256 matches in _work/t238/range_fixes/delivery.log. Preview: _work/t238/range_fixes/hdr_tokyo60/race.png. No commit/push.
- Blockers: none for these fixes/delivery. Physical HDR visual approval, WININT runtime reachability and the earlier luminance-reduction unknown are unverified. Exact next step: optional user physical-display review of the delivered build; no required implementation action remains.


## 2026-10-08 - session Codex HDR commit - T238 range repairs and base sync

Did: user authorizes committing the range repairs and incorporating Blur1PC changes since branching. Reviewed only this session's HDR engine/shader/compiler/probe changes and phase records for staging. Current Blur1PC tip 4ec6226e and DX11 tip e46ee524 are already ancestors of the HDR child; there are no incoming committed changes. No new merge or lower-phase changes are needed.

Verified: ancestry check exit0, HEAD..blur1pc and HEAD..blur1pcdx11 empty. Existing standard Release errsum0, hardware/WARP range checks, 60Hz/unlocked HDR startup/Tokyo gameplay, SDR fallback and reset results remain current; no functional source change since verification/delivery. EXE/PDB and generated FXOs delivered and hash-verified. Screenshot: _work/t238/range_fixes/hdr_tokyo60/race.png. Recorded WININT/physical HDR/luminance intermediate limitations remain unchanged.

Outstanding: none for the requested commit/base synchronization. No push requested.

Exact next step: report the resulting range-fix commit and that all current committed Blur1PC fixes are already incorporated.

## 2026-10-18 — session Claude x64 — T244, T245

Did: created worktree/branch `uKER/blur1pcdx11-x64` from `uKER/blur1pcdx11-hdr` 3223bbc7 and converted the DX11/HDR game to x64: arch-aware CMake (x64 D3DX/OpenAL selection, `/we4311 /we4302`), DemonWare and curl built from source (`cmake/demonware.cmake`, `tools/demonware_cmake/`), x64 OpenSSL 0.9.8i, `Core::PointerToSlot32`, and ~250 pointer-width fixes across core, system, scheduler, FX, renderingdata, UI, audio, particles, Gravity, portal and game code (three read-only audits for silent 32-bit accesses). Fixed the x64 UI sprite clear (DCBZ128 vs 136-byte sprites). Per user, removed all D3D9 and Xbox 360 code (720 files reduced, 238 deleted, jamfiles/CMake updated, d3d9/d3dx9 unlinked). Found T245 (Blur1PCDX11 constant-register guard) and applied it uncommitted in Blur1PCDX11 plus an identical stopgap here. Docs: `docs/blur1pcdx11/x64.md`, analysis, decisions (proposed build.sh architecture option).
Verified: x64 Release build errsum 0 (`_work/t244/build11.out`); exe PE32+ importing d3d11/dxgi/d3dcompiler only; queued minimized runs exit 0: startup/menus match the x86 HDR reference (`_work/t244/startup2/ac.png`), Observatory debug race at 60 Hz and unlocked with the D3D11 debug layer and zero validation messages before and after the removals (`_work/t244/race_*`; grid `_work/t244/race_clean_grid.png`). OpenSSL unit tests and TLS handshakes pass. Exe/PDB/fx11/OpenAL x64 delivered to `_work/run`.
Blocked: T245 needs a user-authorized commit in Blur1PCDX11 and merges through the HDR child; standard `tools/build.sh` cannot select x64 (proposed decision). Nothing committed.
Next: user review; on approval commit T244 on this branch, commit T245 in Blur1PCDX11 and merge it up (HDR, then x64) so the stopgap line resolves to the lower-phase fix; decide the build.sh architecture option in Blur1PC.
Files touched: cmake/{blur_common,dx11,demonware,targets}.cmake, CMakeLists.txt, tools/demonware_cmake/*, engine/game sources and jamfiles (see git status), docs/blur1pcdx11/{x64,analysis,decisions,LEDGER,BOARD}.md; Blur1PCDX11: D3D11GameDeviceNative.cpp, docs/blur1pcdx11/BOARD.md.

Commit (2026-10-18, user authorized): T245 committed on blur1pcdx11 as 0cb00815 (code only; the T245 board row in Blur1PCDX11 stays uncommitted beside another session's T233 edit), merged into uKER/blur1pcdx11-hdr as 4a467ae2 (that worktree's uncommitted T239 docs untouched), T244 committed on uKER/blur1pcdx11-x64 and the HDR branch merged into it. No push.

## 2026-10-18 — session Claude x64 — T244 follow-up

Did: moved the worktree to C:/AIProjects/Blur2PC/Blur1PCDX11HDRx64 (the HDR child to C:/AIProjects/Blur2PC/Blur1PCDX11HDR) on user request. Fixed the x64 model-loading crash the user hit in a copied install (RenderingData resource-allocator areas 32/96/192 on x64; BlockAllocator rejects sizes above its largest area). Per user, the player exe on this branch is the Master configuration (no developer menus/HUD/debug_values.xml); automated tests use the Speed build from _work/run_dev. Merged the Blur1PC retail-icon commit (T246) through the HDR child; kept this branch's Master flags beside it in overrides.cmake. Corrected x64.md: audio is DirectSound and no OpenAL DLL is needed; fx11/ is the only runtime file beside the exe.
Verified: Speed build errsum 0; 100 s main-menu idle and Observatory races at 60 Hz/unlocked exit 0 with zero DX11 validation messages; Master build errsum 0, its run exits 0, shows retail's start screen without developer text and leaves debug_values.xml untouched.
Blocked: none.
Next: none required; the icon rebuild of run (Master) and run_dev (Speed) follows this commit.
Files touched: cmake/{dx11,overrides}.cmake, BlockAllocator.cpp, renderingdata target/archive.cpp, docs/blur1pcdx11/{x64,LEDGER}.md.

## 2026-10-18 - session Claude x64 - T247 occlusion-query wait

Did: found the cause of the x64 showroom headlight/lamp flare flicker the user reported (flares gone for one frame, frames 295-297 of their capture). DynamicEffect::Light::C_Manager reads each light's occlusion query two passes after issuing it and treats an unavailable result as visibility 0. Counted reads per second with temporary instrumentation (reverted) at an unlocked rate: Blur1PC D3D9 0 not-ready, Blur1PCDX11 x86 0, HDR x86 0-23/s, x64 35-95% of reads. D3D11 C_OcclusionQuery::Visible used GetData with DONOTFLUSH and no wait; it now flushes and waits once End has been called since Begin, matching what D3D9 delivered. Also committed the T245 board row left uncommitted with 0cb00815.
Verified: x64 Speed and Master builds errsum0; unlocked start-screen transition and unlocked Observatory race exit0 with 0 DX11 validation messages; user confirmed the headlight flicker is gone on the x64 Master build. Frame-rate cost of the wait not measured. Why x64 runs the GPU a frame deeper than HDR x86 at the same fps is unknown.
Blocked: none.
Next: none for T247. The coloured orbs the user also reported are a separate HDR issue.
Files touched: display occlusion/d3d11/D3D11OcclusionQuery.{h,cpp}; docs/blur1pcdx11/{BOARD,analysis,LEDGER}.md.

## 2026-10-18 - session Claude x64 - T248 reflection flare orbs

Did: traced the orbs with real-speed asynchronous FP16 readback (temporary, removed) to scenery LightFlare sprites in the planar reflection, spread into discs by the T238 FP16 reflection blur. Restored retail A8R8G8B8 for the blurred reflection in `GameWorld/View/Reflection.cpp` (identical working-copy edit in x64). Withdrew the earlier 8-bit bloom and racification attempts; the original HDR racification effects are back in both run folders. Not committed.
Verified: x64 diagnostic runs at real speed: baseline 13-15 spikes per transition; blurred reflection at 8-bit with original racification 0, 0, 0. HDR and x64 rebuilt and delivered (see the board row).
Blocked: none. User confirmed the orbs are gone and authorized the commit.
Next: none for T248. Night-race reflection range (content above 1.0 other than flares) was not measured.
Files touched: GameWorld/View/Reflection.cpp; docs/blur1pcdx11/{BOARD,analysis,decisions,LEDGER}.md.

## 2026-10-18 19:45 — session Claude x64 — T249
Did: traced the car shadow that vanished about 0.1-0.5 s after the first Enter on the start screen in the x64 Master build (user report; HDR x86 unaffected). Bisected with real-speed runs and amplified-shadow stills (temporary, removed): x64 Speed kept the shadow, x64 Master lost it, so the fault is configuration dependent, not x64. Blob shadow inputs, casters, parameters and the screen-pass state matched between builds; GPU readbacks showed the blob texture passes drawing with blending off in Master because `SetBlendMode(AlphaPreMultiply)` found that mode cached after the UI changed blending through Display2. The D3D11 `BeginScene`/`EndScene` lacked retail's `Reset()`; restored it (`D3D11GameDevice.cpp`). Applied the identical working-copy edit in HDR and x64 for delivery.
Verified: Blur1PCDX11 Release, HDR Release, x64 Release and x64 Master errsum 0. x64 Master diagnostic run with the fix: every blob pass draws with ONE/INV_SRC_ALPHA and the blob texture stays at 11.2 after Enter (was 1.6). HDR old/new start-screen and menu stills match. x64 Speed Observatory races unlocked and at 60 Hz exit 0 with the debug layer on and 0 DX11 validation messages. Delivered EXE/PDB to Blur1PCDX11, HDR and x64 `_work/run` (x64 Master) and x64 `run_dev` (Speed); hashes match the build outputs. User confirmed the shadow on the x64 Master build.
Blocked: none.
Next: none for T249. Committed on Blur1PCDX11 at the user's request and merged through HDR and x64.
Files touched: Blur1PCDX11 display `source/device/d3d11/D3D11GameDevice.cpp`; docs/blur1pcdx11/{BOARD,analysis,LEDGER}.md.

## 2026-10-18 - session Claude x64 - T251 crowd flag NaN flashes

Did: traced the flashes near the start/finish arch (user report, HDR/x64) with an AI-driven Observatory lap (debug car page, car 1 controller AI) and a real-speed FP16 scene readback (temporary, removed): NaN texels on the crowd's waving flags. Pass drops and an effect-skip switch (temporary) isolated the built-in `psCrowdFlag.psh`; retail's bytecode for it uses `rsq`+`rcp` (abs input). Fixed with `sqrt(abs())` under `BLUR_DX11`. Applied the identical working-copy edit in HDR and x64 for delivery. A racification clamp was tested first and made no difference (reverted).
Verified: x64 Speed lap: NaN frames 268 before, 0 after. Before/after still: x64 `_work/t251/flag_before_after.png` (flag with black fringe and red haze before, clean after). Builds and delivery: see board row.
Blocked: none.
Next: none for T251. User confirmed the flashes are gone; committed on Blur1PCDX11 at the user's request and merged through HDR and x64.
Files touched: Blur1PCDX11 `gamesys/crowd_system/Shaders/psCrowdFlag.psh`; docs/blur1pcdx11/{BOARD,analysis,LEDGER}.md.

## 2026-10-20 — session Codex tool relocation — T252

Did: moved all 9 project-added tool directories in this worktree from code/tools to root tools, including existing edits, untracked files and local caches. Original leaked tool directories stayed in place. Updated build/CMake dependencies, Python imports/project-root depths, relative HDR probe includes where applicable, commands, metadata provenance and documentation references. Shared AGENTS/workflow policy and accepted D18 now reserve code/tools for the original source tools. Historical documentation paths were updated mechanically; no gameplay or rendering changes. No staging, commits or pushes.
Verified: Documented x64 variant of tools/build.sh regenerated CMake/headers/shaders and eight DemonWare libraries. Full Amax build blocked by C2082 in code/mercury/library/common/system/source/network/SMTPClient.cpp:28: local hostname redeclares the Connect parameter. This file was already modified at startup and was not edited by T252. Relocated dx11_probe builds with errsum 0. Existing delivered Master EXE/PDB preserved; x64 delivery pending this unrelated source fix. Tool payload hash/existence checks, Python 3 syntax checks and case-insensitive old-path scan are recorded in Blur1PCDX11HDRx64/_work/t252/verification.json; original master worktree remains clean.
Blocked: Full x64 build/delivery only: pre-existing SMTPClient.cpp hostname C2082, outside T252.
Next: Owner of the existing SMTP edit resolves its duplicate hostname; rerun the documented x64 Release build and deliver Amax.exe/PDB. No relocation work remains.
Files touched: tools/ additions relocated from code/tools; build/CMake/script/source-comment/document path references; AGENTS.md, docs/agent-reference, docs/decisions.md and phase BOARD/analysis/LEDGER. Verification logs under this phase's ignored _work/t252; consolidated inventory/backups/build/test/delivery evidence in Blur1PCDX11HDRx64/_work/t252.

## 2026-10-21 - session Codex tool relocation - T252 commit

User authorized committing T252. Commit the tracked tool relocations, reference updates and placement policy; retain the earlier verification results above. Propagate the shared change with merges through the phase branches. No push requested.

T252 propagation (2026-10-21): merged blur1pc into this branch with git merge; retained this phase's tool set and regenerated the jam-derived CMake source list. Prior T252 build/test verification is recorded above. No push.

T252 propagation (2026-10-21): merged blur1pcdx11 into this branch with git merge; retained this phase's tool set and regenerated the jam-derived CMake source list. Prior T252 build/test verification is recorded above. No push.

T252 propagation (2026-10-21): merged uKER/blur1pcdx11-hdr into this branch with git merge; retained this phase's tool set and regenerated the jam-derived CMake source list. Prior T252 build/test verification is recorded above. No push.


## 2026-10-21 - session Claude x64 - T253 internal infrastructure references

Did: surveyed the whole tree for named internal infrastructure (UNC shares, hosts, mapped tools drive, internal URLs and e-mail domains, source-control bindings, mapped-drive build scripts) with a Python scan, then applied one rule set: delete bindings and server-only CI/publish scripts; replace the Z: share with the `BIZARRE_ROOT`/`SRC_ROOT` variables the original scripts already used and `jam` from the path; blank or localise every server, mailbox and credential default in code and configs; drop wiki links and personal addresses. Engine code changed: `ToolsApplication.cpp` honours the dump path it is given (was hard-coded to `\\bizsymbol`), `EMailSubscriber` has no default sender, `SMTPClient` greets with this machine's name, and the DX11 opt-in diagnostics write under `dx11_trace\` in the working folder (`TraceDir`/`TracePath`, `FxDumpDir`) where they named one developer's `Blur1PCDX11\_work`. Full list and the items left for a decision: docs/blur1pcdx11/x64.md, "Internal infrastructure references removed (T253)". The scan and edit scripts are in this session's scratchpad only.
Verified: `cmake/targets.cmake` regenerates identically from the edited jamfiles. x64 Release errsum 0 and x64 Master errsum 0 (`_work/t253` reuses `_work/t252/build.log`; the first Release pass failed on the SMTP edit's `hostname` C2082 that the T252 session also hit, fixed by renaming the local, then rebuilt clean). Speed smoke run (`_work/t253/smoke.txt`, queued, minimised) boots to the loaded start screen and exits 0 in 17 s: `_work/t253/start.png`. Delivered: Master exe/pdb to `_work/run`, Speed to `_work/run_dev`; hashes match the build outputs. fx11 untouched (no shader change). No run of the stripped standalone tools (nothing builds them here).
Blocked: none.
Next: user decides on the infrastructure-only tool directories (xFarm, bizarresaver, BuildFarmSystem, BuildFarm2, executer, the_monkey, DataLogging, alienbrain_wrapper, bizbrain), the Xlast folder, and whether the DX11 diagnostic-path change goes to Blur1PCDX11/HDR. Nothing committed; `package.bat` at the root is the user's untracked file.
Files touched: 164 deleted (117 .vspscc/.vssscc, 23 .wsf plus their helper .js/.sln/.vcproj, 12 UpdateData.bat, publish/SVN/build batch files, aftkut/tmp.txt, xFarm www/index.htm); ~580 edited under code/ (sln/vcproj/csproj, jamfiles, .bat, tool sources and configs, xFarm www, five engine headers, ToolsApplication.cpp, E-MailSubscriber.cpp, SMTPClient.cpp, D3D11GameDeviceNative.cpp, PCD3D11GameDevice.cpp, fx System.cpp, postprocess Interface.cpp); docs/blur1pcdx11/{x64,BOARD,LEDGER}.md.

## 2026-10-22 - session Claude x64 - T253 second round (tools and literals)

Did: per user, deleted the nine infrastructure-only tool directories (xFarm with farm_library and www, bizarresaver, BuildFarmSystem, BuildFarm2, executer, the_monkey, DataLogging, alienbrain_wrapper, bizbrain; 594 files) and removed every remaining `Z:` literal: test and sample inputs keep their path without the drive, tool logs land beside the tool, the lux texture preprocessor's network fallback and the engine's Alienbrain path converters (StringUtil.cpp/BtlStringUtil.cpp, compiled) take the workspace drive from `BIZARRE_ROOT` and pass paths through when it is unset, `ShimDefinitions.h` drops the unused `ALIEN_BRAIN_DRIVE`. Details in x64.md (T253 section, second round).
Verified: no `Z:`/host/e-mail-domain hit left under code/ by the T253 scan regexes. targets.cmake regenerates identically. x64 Release errsum 0 and x64 Master errsum 0 (`_work/t253/build.log`). Speed smoke run (`_work/t253/smoke.txt`, queued, minimised) boots to the loaded start screen, exit 0 in 17 s: `_work/t253/start.png`. Delivered Master exe/pdb to `_work/run`, Speed to `_work/run_dev`; hashes match. The deleted tools and the edited standalone tools are not built by anything here.
Blocked: none.
Next: user decides whether lux_app/lux_client/vis_cmd/clusterserv_lib (whose jamfiles and clusterserv's sources still name xFarm's farm_library) follow the farm out, and the Xlast folder. Nothing committed.
Files touched: code/tools/{xFarm,bizarresaver,BuildFarmSystem,BuildFarm2,DataLogging,alienbrain_wrapper,bizbrain}, code/tools/c_sharp/applications/{executer,the_monkey} deleted; aftk/attk/aftkut/aftkrender test sources, nexus_cmd.cpp, cmd_line_tool Application.cpp, particle_converter Application.cpp, lux TexPreprocess.cpp, actor_studio editor_visualisaton.xml, system misc StringUtil.cpp/BtlStringUtil.cpp, tools_shared ShimDefinitions.h; docs/blur1pcdx11/{x64,BOARD,LEDGER}.md.

Commit (2026-10-22, user authorized): T253 committed on uKER/blur1pcdx11-x64 as 7df8a585 (1293 files: 758 deleted, the rest edited; only this task's hunk of PCD3D11GameDevice.cpp staged, the other session's uncommitted recorder change in that file left in the working tree; package.bat untracked). Top branch, no merges. No push.

## 2026-10-22 - session Claude ultrawide - T254 DX11 device aspect
Did: merged blur1pc (T254: the PC device aspect is a float that follows the back buffer, retail C_DeviceBase +0x8; see docs/analysis.md Device aspect). The native D3D11 device is this phase's own code, so C_PCD3D11GameDevice::CreateDevice and CreateDeviceVolatile now call SetAspectRatio(back buffer width / height), as C_PCD3D9Device does.
Verified: Blur1PCDX11 Release errsum 0 (_work/t254_build.txt). The behaviour was verified on Blur1PC at 2560x720 (Blur1PC _work/t254/uw/race.png); the user confirmed retail parity.
Blocked: none.
Next: none; merged into uKER/blur1pcdx11-hdr and uKER/blur1pcdx11-x64.
Files touched: code/mercury/library/graphics/display/source/device/d3d11/PCD3D11GameDevice.cpp; docs/blur1pcdx11/{BOARD,LEDGER}.md.

## 2026-10-23 - session Claude ultrawide - T255 DX11 full screen display mode
Did: full screen at 3840x1080 on a 16:9 3840x2160 output was stretched to 16:9. DXGI resolves the mode to its STRETCHED variant and keeps the desktop mode; D3D9 sets 3840x1080 with the driver's aspect-preserving scaling (probes and numbers in analysis.md T255). The D3D11 device now sets the full screen mode with ChangeDisplaySettingsEx(CDS_FULLSCREEN) and presents through a windowed swap chain covering the monitor; focus loss restores the desktop mode and minimises, reactivation re-applies.
Verified: x64 Release errsum 0 and Master (MinSizeRel) errsum 0. Queued minimised Speed run with BLUR2PC_FULLSCREEN=1 at 3840x1080 exit 0: display 3840x1080 with scaling 4 (aspect-ratio centred, same as D3D9) during the run, 3840x2160 restored after; start-screen frame undistorted (`_work/uw_probe/start_view.jpg`). Delivered Master Amax.exe/pdb to _work/run; hashes match.
Blocked: none.
Next: user checks full screen 3840x1080 on the TV, including alt-tab (not exercised by automation). Not committed.
Files touched: code/mercury/library/graphics/display/source/device/d3d11/PCD3D11GameDevice.cpp; docs/blur1pcdx11/{analysis,BOARD,LEDGER}.md.

## 2026-10-23 - session Codex popup-coverage - T263 source prepared, delivery pending
Did: traced the popup filter against pristine source, recovered retail Lua and runtime geometry. Added a PC-only viewport-size assignment in BlurRebuilt TheCustomPopup.lua PopupPostInit. Honored the user's corrected BlurRebuilt-only scope; withdrew the initial base board claim and made no base source/build changes. No commit or push.
Verified: existing BlurRebuilt reproduces the 32:9 gap; diagnostic runtime resizing matches exact viewport dimensions at 3840x1080 and 1440x1080, exits 0, and covers the display. The diagnostic's delayed resize was intentional and noticed by the user; the implementation runs in PostInit before rendering. The candidate load-hook attempt did not install the source change and is not counted as verification.
Blocked: final build/delivery awaits the user's pending build-content choice because current delivery includes another session's uncommitted AMAX work while AGENTS.md says to exclude other sessions' edits.
Next: on that choice, use the documented x64 variant of tools/build.sh, verify errsum 0, deliver canonical EXE/PDB without removing authorized existing functionality, then capture the opening transition and verify exact coverage at 4:3, 16:9 and 32:9. No final approval screenshot exists yet; diagnostic evidence under _work/t263.
Files touched: code/amax/library/Ui/game/lua_scripts/popups/TheCustomPopup.lua; docs/blur1pcdx11/{BOARD,analysis,decisions,LEDGER}.md.


## 2026-10-23 - session Codex popup-coverage - T263 delivered and verified
Did: built the prepared popup initialization fix into BlurRebuilt and delivered standard Release Amax.exe/PDB to _work/run, the desktop shortcut target. Preserved the existing AMAX functionality. This supersedes the preceding pending build-content question; no lower-phase source/build changes, commit or push.
Verified: documented x64 tools/build.sh variant, regenerated Lua bundle, final Amax link, errsum 0, unchanged generated targets.cmake, matching source/delivery EXE and PDB hashes in _work/t263/delivery.json. At the user's suggestion, used real Esc input at the first title screen. Three queued/minimized delivered-executable runs exit 0 and pass exact viewport bounds at 3840x1080, 1920x1080 and 1440x1080; no runtime resize injection. Inspected all three stills and the 32:9 opening video frames: full coverage from first visible fade, no delayed expansion. Approval: _work/t263/verify32/approval.png; transition: verify32/opening-contact.png and opening.mp4; other ratios: verify16/approval.png and verify4/approval.png.
Blocked: none.
Next: user can retest the desktop BlurRebuilt icon; implementation and delivery complete, changes remain uncommitted.
Files touched: code/amax/library/Ui/game/lua_scripts/popups/TheCustomPopup.lua; docs/blur1pcdx11/{BOARD,analysis,decisions,LEDGER}.md.


## 2026-10-23 - session Codex popup-coverage - T263 approved commit
Did: user approved the delivered result and requested commit. Committed only the viewport-sized popup initialization and T263 phase documentation on BlurRebuilt. No propagation or push, per the explicit phase boundary.
Verified: source unchanged since the successful Release build/delivery and real title-screen Esc runs at 4:3, 16:9 and 32:9. Reviewed staged scope and diff whitespace; approval screenshot _work/t263/verify32/approval.png and opening-transition evidence _work/t263/verify32/opening-contact.png.
Blocked: none.
Next: none for T263; delivered fix accepted by the user.
Files touched: code/amax/library/Ui/game/lua_scripts/popups/TheCustomPopup.lua; docs/blur1pcdx11/{BOARD,analysis,decisions,LEDGER}.md.


## 2026-10-23 - session Codex menu-coverage - T264 delivered and verified
Did: traced the independent event/car vignettes against pristine source, recovered retail Lua and runtime geometry. Added PC-only exact viewport sizing in SpEventSelect.lua and Garage.lua PostInit, before first rendering. Kept all changes in BlurRebuilt. Built and delivered the standard Release EXE/PDB to the desktop shortcut's _work/run target. No commit or push.
Verified: errsum 0, regenerated Lua bundle, unchanged targets.cmake, matching Release/delivery hashes (_work/t264/delivery.json). A copied local save reproduced both 32:9 cutoffs before delivery. Three queued/minimized final runs exit 0 with six active-vignette exact-bounds passes at 4:3, 16:9 and 32:9. Inspected all six stills plus both 32:9 opening videos/contact sheets: full viewport coverage from first appearance. Verification screenshots: _work/t264/verify32/event-approval.png and garage-approval.png; other ratios under verify16 and verify4. Initial harness retries are superseded by these successful runs. No live save edits or frame-timed behavior changes.
Blocked: none.
Next: user review from the desktop BlurRebuilt shortcut; implementation and delivery complete, changes remain uncommitted.
Files touched: code/amax/library/Ui/game/lua_scripts/screens/SinglePlayer/SpEventSelect.lua; code/amax/library/Ui/game/lua_scripts/screens/Shared/Garage.lua; docs/blur1pcdx11/{BOARD,analysis,decisions,LEDGER}.md.


## 2026-10-23 - session Codex menu-coverage - T264 approved commit and propagation audit
Did: user approved the delivered screenshots and requested commit and propagation. Audited the current history: the exact event/garage changes already reside in f1f2ba55, merged under T265 into all five active phase/feature branches. Committing only the remaining T264 board, mechanism, verification and approval notes in BlurRebuilt; no duplicate source change or merge. No push.
Verified: all five branches contain f1f2ba55 and identical event/garage viewport-sizing PostInit blocks (_work/t264/propagation-audit.json). BlurRebuilt source matches the earlier zero-error Release and six runtime bounds passes at 4:3/16:9/32:9; screenshot approval is _work/t264/verify32/event-approval.png and garage-approval.png. No source changed, so no repeat build/run is needed. Reviewed staged scope and whitespace.
Blocked: none for source commit/propagation. Separate T265 records the pre-existing BlurX KanzenTracks.cpp syntax error and pending BlurX/Dubai executable delivery; no success claimed there.
Next: none for T264; user-approved source is committed and propagated. T265 owns remaining upper-phase binary delivery.
Files touched: docs/blur1pcdx11/{BOARD,analysis,decisions,LEDGER}.md only.


## 2026-10-23 - session Codex pause-coverage - T266 BlurRebuilt delivery
Did: user requested BlurRebuilt too. Confirmed the approved source f8fc65f1 was already merged; rebuilt with the documented x64 tools/build.sh variant and delivered Release Amax.exe/PDB to the canonical launcher target, preserving existing AMAX functionality. Committed only this phase's T266 handover; no duplicate source patch or push.
Verified: errsum 0; generated Lua bundle includes the inherited pause fix, generated targets.cmake unchanged from pre-build. Matching canonical EXE/PDB hashes in _work/t266/delivery.json. Queued/minimized actual-race debug-menu/Esc run at 3840x1080 passes exact viewport, center, scale and alpha assertions and exits 0 without timeout. Inspected pause still and compressed opening transition, both full-width. Approval screenshot _work/t266/verify32/approval.jpg; opening-contact.jpg and opening.mp4 beside it. Native effects source unchanged; no broad effect-file hash verification claimed.
Blocked: none for BlurRebuilt delivery.
Next: none for T266 here; desktop BlurRebuilt launcher now contains the fix.

## 2026-10-24 - session Codex race-intro coverage - T268 delivered
Did: followed user-authorized base ownership, merged c63bfbf5 via 178ae1a9, regenerated source lists, rebuilt standard x64 Release and delivered Amax.exe/PDB to _work/run. Source propagated to every active upper branch; no push.
Verified: errsum 0 and matching canonical hashes in _work/t268/delivery.json. Queued/minimized natural career run at 32:9 exits 0, passes intro coverage and all three aperture checks, then reaches HUD. Inspected _work/t268/career-verified32/approval.jpg and intro.mp4. Base independently verifies 4:3, 16:9 and 32:9. Early career harness context/loading-screen waits were corrected before the passing run. No simulation timing changes.
Blocked: none for BlurRebuilt delivery; no BlurX/Dubai binary-delivery claim (T265).
Next: user review from the normal BlurRebuilt launcher. Verification screenshot: _work/t268/career-verified32/approval.jpg.
Files touched in this phase: docs/blur1pcdx11/{BOARD,analysis,LEDGER}.md; source inherited from base.

## 2026-10-24 - session Codex race-intro coverage - T268 initial plaque delivered
Did: inherited base correction e9e556f0, rebuilt standard x64 Release, delivered canonical Amax.exe/PDB, and verified the initial loading-to-vista zoom. Source propagated through all active upper branches; no push.
Verified: errsum 0; matching canonical delivery hashes; loading-verified32 run exits 0 with both loading bounds checks and all cinematic/countdown checks passing. Inspected _work/t268/loading-verified32/approval.jpg and intro.mp4: full-width plaque, preserved preview cutout and zoom.
Blocked: none for BlurRebuilt delivery.
Next: user review from the normal launcher. Approval screenshot: _work/t268/loading-verified32/approval.jpg.
