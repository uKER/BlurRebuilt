# HDR display support

T238, implemented on child branch `uKER/blur1pcdx11-hdr`, based on `blur1pcdx11` at `e46ee524`. The user requested a separate branch/worktree and native HDR, with support for arbitrary display luminance. The original DX11 branch is unchanged. Worktree: `C:/Users/uKER/orca/workspaces/Blur2PC/blur1pcdx11-hdr`.

## Operation and calibration

Enable HDR in Windows, then launch this worktree's `_work/run/Amax.exe` through its normal launcher. Automatic output selects the monitor containing the game window, checks Windows HDR state and uses the monitor's reported peak luminance and Windows SDR-content brightness. Detection repeats by elapsed time once per second and immediately on monitor movement. The renderer uses a floating-point scRGB flip swap chain; Windows performs the display encoding. Unavailable HDR capabilities or invalid/missing peak information select SDR output.

Live settings use the existing debug-variable interface and persistence:

- `Graphics.Render.HDR.Display.Enable`: enable HDR grading/output, default true. Disabling it keeps the scRGB container but presents SDR at Windows SDR white.
- `Graphics.Render.HDR.Display.PeakNits`: positive display peak in cd/m²; zero uses display metadata.
- `Graphics.Render.HDR.Display.PaperWhiteNits`: positive scene/UI reference white in cd/m²; zero uses Windows SDR-content brightness.

Equivalent launch overrides: `BLUR_DX11_HDR_PEAK_NITS` and `BLUR_DX11_HDR_PAPER_WHITE_NITS`. Live positive calibration takes precedence over launch overrides. Paper white cannot exceed the selected peak. There is no fixed 1,000/4,000/10,000-nit cap. Invalid, nonpositive and nonfinite values return to automatic detection. `BLUR_DX11_HDR=off` (or `0`) preserves the previous BGRA8 SDR path, including exclusive-fullscreen blt presentation. `BLUR_DX11_HDR_TEST=1` is solely a probe diagnostic override, allowing synthetic HDR tests without an HDR-enabled display.

**80 nits is scRGB's unit, not a brightness limit or the display target.** A 1,000-nit output is approximately 12.5 scRGB units; calibration and highlight mapping follow the actual selected display peak. The Windows SDR-white query uses its documented 80-nit fallback only when the query is unavailable; paper white can always be overridden. See Microsoft's [HDR rendering guidance](https://learn.microsoft.com/en-us/windows/win32/direct3darticles/high-dynamic-range), [output metadata](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/ns-dxgi1_6-dxgi_output_desc1), and [SDR white-level units](https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-displayconfig_sdr_white_level).

## Scene range and output

The October 3 assessment incorrectly inferred an 8-bit scene target from `HDRU32` shader names. The current PC post-process setup selects `E_HighDynamicRangeType_Shader`, creates RGBA_F10 scene views, and maps them to FP16 under native DX11. Runtime readback confirms format 10 (`R16G16B16A16_FLOAT`). Retail PC `GetExposure` sqrt-encodes exposed RGB without clipping it; the native port retains those recovered material shaders.

The losses relevant to output were the bounded final colour-grading lookup/matrix and 8-bit per-window/back-buffer targets. HDR now keeps the final window and composition targets FP16. The final tone-map shader normalizes values above white for the authored colour-grading lookup, then restores their brightness extension after grading/film grain. Values at or below reference white retain the authored SDR processing. HDR colour-matrix results retain positive extended values. SDR follows the original bounded branch. This extension is a new HDR feature, not a recovered retail tone curve.

The user subsequently reported that emissive lights and the sky did not visibly look HDR. Container and peak readback alone do not establish that result. The follow-up removes two additional storage ceilings: the reduced particle/lens-flare manager and postprocess bloom/downsample targets use FP16 when the native output container is floating point. The racification effect's explicit SDR RGB saturation becomes a positive-only clamp while HDR is active; its SDR branch is unchanged. The FX5 compiler preserves explicit native b-register bindings, and the draw path binds HDR b12 after FX11's buffer updates. These are HDR feature additions, not claims of recovered retail behavior. No guessed sky or material brightness multiplier is introduced.

The shared emissive-lighting/exposure and current textured-sky shaders already allow extended RGB. The native HDR grade and presenter now preserve that range. An emissive or sky source can exceed paper white when its authored/exposed intensity does; ordinary diffuse surfaces and UI do not receive an indiscriminate brightness boost. Actual physical appearance of all retail lights remains a separate verification requirement.

The final presenter decodes the authored gamma-domain composite with the sRGB transfer function, maps values above paper white with a continuous, hue-preserving exponential shoulder to the selected display peak, and writes linear scRGB. Black stays zero. UI authored white maps to paper white. The original assets remain Rec.709; no additional asset colour gamut is invented. Peak metadata does not promise full-screen sustained brightness; the panel controls its own ABL.

The presenter restores every changed context binding, including FX11 shader class instances, RTV/DSV, viewport, raster/blend/depth state, shaders, layout/topology, PS texture 0 and PS constant buffer 12. HDR constants use b12, outside legacy b0-b7 and alpha state b13. Device reset releases swap-chain views before resizing and recreates the FP16 proxy/output views. Exclusive fullscreen starts with a windowed flip chain, enters fullscreen, and resizes before taking buffer references.

## Verification and capture

The standard Release build regenerates retail/native built-in shaders and shader headers. `dx11_hdr_reference` uses the production output helper and shader, checking GPU readback against CPU transfer/shoulder references on hardware and WARP at peaks 80, 200, 400, 600, 1,000, 2,000, 4,000 and 10,000 nits, plus fractional live calibration at 1,733.33 nits. Checks include black, SDR/reference white, extended highlights, hue ratios, peak/monotonicity, live disable, target/viewport restoration and resize. WARP additionally verifies a synthetic dynamically linked pixel shader's class-instance restoration. That synthetic fixture caused a hardware access violation on this machine; its cause is unresolved, so that fixture runs only on WARP. Hardware presenter/readback and actual game FX11 usage are checked separately.

The probe also accepts generated shader roots and the compiled racification LOD effect:

```powershell
_work/build/vs/Release/dx11_hdr_reference.exe --hardware _work/build/gen/dx11_builtins _work/build/gen/fx11/effects/Amax/Game/Shaders/Racification/Racification__MaterialLOD.fxo
# Repeat with --warp for software-GPU verification.
```

It exercises the actual sky shader through FP16 scene storage, actual final grading with an identity LUT and production HDR presentation. At 200-nit reference white / 1,000-nit peak, sky radiance 0/1/4/16 produces 0/200/646.875/990 nits. The FX5 check reflects b12 and applies the real racification pass before external HDR binding: RGB (4,2,1) remains extended in HDR and clamps to (1,1,1) in SDR. It detects the earlier compiler binding loss. The test vertex signatures match each production pixel shader's registers; initial fixture signature/texture-slot mismatches were corrected before accepting the results.

Ordinary `play.bat` launches write display activation, detected peak and selected paper white to `_work/run/hdr_output.log`. This makes output-state verification possible without an automation script. The log is rewritten on each launch and updates when capabilities/calibration change.

Automation stills produce a normal SDR TGA preview and a `.tga.pfm` companion containing unmodified floating-point scRGB RGB. Multiply PFM values by 80 to obtain encoded channel cd/m². The log reports maximum channel nits and pixels above paper white. The SDR preview clips highlights and cannot demonstrate how the physical HDR panel looks.

Evidence is under `_work/t238/`: standard build logs, hardware/WARP probe logs, and queued minimized driving runs. Final results and limitations are recorded in the owning phase's analysis and ledger. Physical appearance on other panels, Windows HDR toggles during a race, monitor migration and sustained full-screen panel luminance require additional hardware checks.

## Lamp artifact repair

The native exposure shader matches retail FXB rsq/rcp semantics with sqrt(abs(exposedRGB)). This prevents negative finite lamp-edge lighting from producing NaNs that spread into black squares through depth-of-field. Positive HDR radiance stays unbounded and MSAA remains enabled. Startup at60/unlocked and Tokyo Shutoko moving runs were verified with HDR and8xMSAA; the user confirmed the artifacts and their other manifestation are gone before requesting commit. See analysis.md for detailed evidence and remaining verification limits.
