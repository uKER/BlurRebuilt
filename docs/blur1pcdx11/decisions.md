# DX11 decisions

## T187 — stream effect constants and memoize legacy samplers (2026-10-07)

Found in / Belongs to Blur1PCDX11. Ordinary FX11 constant buffers use dynamic storage and WRITE_DISCARD, copying the complete effect backing store whenever it is dirty. Texture buffers retain DEFAULT/UpdateSubresource; externally supplied non-updatable buffers retain their existing behavior. This changes upload mechanics while preserving authored values, pass ordering and simulation cadence. GPU readback verifies partial parameter writes, independent cloned effects and 128 queued alternating draws on hardware and WARP.

Legacy sampler slots memoize their last descriptor and native object. An actual FX Apply advances the binding epoch, requiring the next legacy setter to rebind even if the descriptor is unchanged. Descriptor changes and device changes also force lookup/binding. Keep the existing global state-object cache and multithread protection.

Retain the repaired fullscreen blt presentation path. Temporary GPU timestamp/Present instrumentation is removed from the delivered build. These optimizations improve measured performance; they do not establish DX9 parity or complete graphics parity. Measurements and verification limits are in analysis.md.

## T187 — resolve FX bindings once and retain unaffected native bindings (2026-10-07)

Found in / Belongs to Blur1PCDX11. The user authorized this follow-up after the performance assessment. Cache immutable per-pass binding metadata and parameter interfaces, while reading current shared/material/object data on every update. Effect destruction releases the records through the existing NativeFX11ReleaseCache path; rebuilding a function's binding array invalidates its records.

The engine's PS b13 alpha-state bind invalidates only that FX constant-buffer slot. Preserve FX/device buffer restoration and the existing alpha-test values and comparisons. Keep full-stage invalidation for callers that change whole categories of state.

Skip unchanged native vertex/index buffer and input-layout submissions, following DX9's existing binding checks. Compare raw buffers as well as stride/offset/format. Input layouts retain the existing immutable declaration/shader pair key. Device state invalidation and the raw postprocess diagnostic draw invalidate these memos; failed layout creation remains retryable. Preserve multithread protection, constant upload policy and the accepted fullscreen presentation path.

## T238 — native HDR on a separate DX11 child branch (2026-10-07)

Status: accepted scope, explicitly requested by the user. Found in / Belongs to the DX11 HDR child branch. Create `uKER/blur1pcdx11-hdr` from `blur1pcdx11`, leaving the parent and all lower phases unchanged. Implement scene-derived HDR with automatic display detection and arbitrary positive peak/paper-white calibration. The user explicitly rejected treating 80 nits as a display output limit.

Use FP16/scRGB presentation and retain existing retail exposed scene RGB. Add an explicitly new HDR extension to the final bounded grade, plus a continuous hue-preserving shoulder above paper white. SDR values retain authored grade processing. Keep the existing BGRA8/fullscreen blt path for launch-time HDR off and SDR fallback when native output capabilities are unavailable. HDR exclusive fullscreen uses windowed creation followed by transition/resize before acquiring views, satisfying the flip-model contract without changing the parent's accepted SDR presentation decision. Details and calibration controls are in hdr.md. No simulation or exposure-adaptation timing changes.

T238 follow-up scope (user confirmed): all emissive categories and the sky must be able to exceed paper white. Remove intermediate RGB/storage ceilings under HDR and preserve authored lighting/exposure; do not use a global scene gain or guessed material/sky multipliers to stand in for recovered values. Extend racification RGB only with active HDR; keep its retail SDR saturation. Preserve explicit native FX constant-buffer slots and bind device HDR state after FX application. Validate actual source/grade shaders in addition to the output container. Physical visual approval remains outstanding following the user's report; clipped previews cannot provide that approval. Lamppost square artifacts are explicitly deferred.


T238 lamp-square scope (2026-10-08, user explicitly requested): the earlier deferral ends for the lamp-head black blocks. Match the actual retail FXB exposure rsq/rcp magnitude semantics in native shaders, keeping positive HDR radiance unbounded and MSAA enabled. Do not use intensity guesses, disable lighting effects, or alter simulation/adaptation timing. Final trace-free build is delivered; broader HDR physical review remains pending. This adds the requested defect repair without reversing the accepted HDR direction.


T238 completion (2026-10-08): user confirms the additional manifestation is gone and authorizes committing the HDR implementation and lamp-square repair together on the HDR child. Retain the separate child branch; no merge into the original DX11 branch or push was requested. Recorded verification limitations remain unchanged.

T238 remaining range fixes (2026-10-08): accepted, user explicitly requests fixing all limits identified by the source audit. Extend HDR storage through dynamic-light accumulation, planar-reflection blur and bloom feedback. Extend vehicle SH lighting and interior-glass reflection above their retail upper bounds only in the active HDR branch, retaining their lower bound and authored light/reflection calculation. Replace the finite RGBM vehicle-light encoding with floating storage for the HDR container, retaining the existing RGBM decoder and SDR compatibility. No guessed intensity gain, simulation/adaptation cadence change, lower-phase edit or commit is authorized by this follow-up.

T238 range-fix commit/base synchronization (2026-10-08): user explicitly authorizes committing the repairs and incorporating later Blur1PC fixes. The committed base is already current at 4ec6226e; retain the HDR child and record the ancestry check. No push requested.

T244 x64 child (2026-10-18, user request): create `uKER/blur1pcdx11-x64` from the HDR child and convert it to 64-bit. The user then said "Feel free to remove any DX9 code" and "Remove Xbox code as well": on this branch Direct3D 11 is the only renderer (D3D9 device, D3D9 probe and D3DX9 removed) and Xbox 360 code is removed; PS3/Wii code is kept. DemonWare is built from its bundled sources and OpenSSL 0.9.8i from the official tarball (`no-asm`, `/MD`). Fixed-width data formats keep their layout: pointer-carrying 32-bit FX sampler words hold `Core::PointerToSlot32` slots on x64; runtime-only structures widen to pointer size.

Proposed (needs confirmation; tools are authored in Blur1PC): let `tools/build.sh` take the CMake architecture from the phase (for example a `BLUR_ARCH` variable or a phase file) so the x64 branch can use the standard build script. Until then the x64 build is `tools/build.sh` with `-A x64` (`_work/t244/build64.sh`).

T248 reflection blur range (2026-10-18, accepted: user confirmed the result and asked to commit): narrows the T238 remaining-range fix for planar reflections. The blurred reflection keeps retail A8R8G8B8 under HDR output; the sharp reflection, scene, bloom, racification and light accumulation stay FP16. Measured: without light flares the blurred reflection never exceeded 1.0, so the cap removes only flare overdrive. An 8-bit bloom chain and a racification saturate were tried and withdrawn.

T263 popup coverage (2026-10-23, accepted by explicit user instructions): make the confirmation-dimming layer aspect-agnostic through 4:3, exactly viewport-sized without excess coverage, and implement only in BlurRebuilt. The destination overrides the generic lowest-phase ownership/propagation rule for this task. Preserve the existing popup layout and fade behavior; size the authored centered filter during initialization. User approved the delivered result and requested commit on 2026-10-23; keep the commit in BlurRebuilt only.


T264 menu vignette coverage (2026-10-23, continued explicit user scope): extend the same exact-viewport, aspect-agnostic coverage requirement to event selection and car selection in BlurRebuilt only. Resize the existing centered vignettes during PostInit using SCREEN_WIDTH and height 480; preserve their visual properties and activation rules. No lower-phase propagation. The prior T263 commit request does not authorize a new T264 commit.


T264 commit and propagation (2026-10-23, accepted by explicit user instruction): user approved the delivered result and said "Commit and propagate." This supersedes the earlier BlurRebuilt-only propagation restriction for these overlay fixes. Reuse the exact source already committed and merged under T265 (f1f2ba55); commit T264 verification/approval notes only in BlurRebuilt, whose phase docs have no higher descendant. No push requested.
