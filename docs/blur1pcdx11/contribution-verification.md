# Contribution scope and verification

Updated 7 November 2026. Base: upstream `master`
`a3f14caf6abc20f832dd0114561381908fee8f3f`. Official v1.6 is
`29c94b0f3e4503495f784f1926ed27d7864db9a3`; these revisions differ only in README.

**Review status:** this branch contains a selective source contribution. The
tested local player included additional development-interface cleanup and local
compiler compatibility flags that are deliberately excluded here. This exact
filtered tree has not yet completed its own full build/runtime/CI validation.
The package installed on the test PC must not be represented as a binary built
from this exact branch. No public binary release is proposed at this stage.

## Included changes

| Change | Problem and implementation | Evidence and boundary |
|---|---|---|
| Particle trail allocation | An 8-byte element stride is too small for the native x64 pointer/salt record. Allocate `sizeof(C_Particle_E)` and include its defining header. Serialized particle formats are unchanged. | Actual-source layout control: old x64 records overlap; repaired x64 records do not. Native x86 remains 8 bytes and passes. A preserved game dump identified the overlapping trail record. |
| Texture staging-lock lifetime | The global map retains records after a texture's final COM destruction. New resources reusing the address can encounter dead locks. Resource-private ownership now enqueues retirement; registry operations collect it under the registry mutex. | Untouched upstream extraction leaves six dead cube-face locks on hardware and WARP. Corrected source passes aliases, unrelated owners, final destruction, callback-under-lock and 128 reuse cycles. Buffers retain their existing ownership rules. |
| Shader stage interfaces | Two pixel-input layouts do not match the registers emitted by their paired vertex programs. Preserve unused native inputs to maintain those layouts without changing shader formulas. | Both original pairs fail D3D11 debug linkage (343) on NVIDIA, AMD and WARP. Both corrected pairs pass on all three. Original four bytecode programs are present byte-for-byte in the official v1.6 EXE. |
| DPI awareness | Upstream's manifest permits DPI virtualization, enlarging the physical fullscreen window under display scaling. Embed PerMonitorV2 with the legacy true/pm declaration. | Same native window code/original versus corrected manifests at 300%: requested 1920x1080 becomes 5760x3240 versus 1920x1080. Earlier laptop 125% game cropping and corrected user acceptance are retained. This is not a new general mixed-monitor DPI implementation. |
| Refresh selection | Keeping the first mode for each resolution can choose 24 Hz on a 60 Hz TV. Prefer the current output's rate when supported, otherwise request driver default. Round fractional enumeration rates. | Six permutations of 24/30/60: upstream chooses wrongly four times, corrected zero. 59.94 and unavailable/unknown current-rate cases pass. Real TV 1080p/4K at 60 Hz exercised. Multi-output coverage is incomplete. |
| Adapter selection | Explicit high-performance hardware preference on hybrid systems, with `BLUR_DX11_GPU=system` fallback and file-only device/presentation evidence. | RTX 4050 selected in the accepted installed test without per-path GPU registry overrides. This is policy and observability, not a proven AMD/NVIDIA driver repair. |
| Optional controller backend | Integrate unmodified InputFusion 0.4.1 through explicit sibling-DLL loading and existing state/rumble paths. Preserve Windows XInput fallback. | USB and Bluetooth controls/rumble tested, with DS4Windows closed. The fresh installed copy's Bluetooth controls, vibration and Options pause were explicitly accepted. Backend source/protocol implementation belongs to InputFusion and its dependencies. |
| Saved Maximum FPS | Expose the existing render limiter through normal graphics options; presets 0/30/60/90/120/144 and a retained valid custom value. Add caption support and a separate versioned XML element. | Save/restart, malformed preferences, Cancel/No/timeout rollback and measured 30/120 caps tested. Existing positional attributes/binary profile unchanged; no simulation-cadence change. New captions are English. |
| Build path quoting | Invoke the existing companion script using `cmd /c call` so checkout paths containing spaces work. | Local companion build succeeded from a path containing spaces. |
| Offline installer | One Setup EXE contains Rebuilt, effects, InputFusion/notices and the signed x64 C++ prerequisite. Locate/validate the game folder, refuse busy files, back up managed files, preserve the baseline on updates, restore on uninstall. Keep the ZIP as an optional manual path. | All 4,997 selected files installed; game/backend hashes match. Invalid/busy targets, update and uninstall/restoration passed in an isolated deep path with spaces. All 4,985 effects-index references exist. A real fresh-game-folder installation was then accepted by the user. |

## Test environment

- One Windows 11 x64 physical laptop with NVIDIA GeForce RTX 4050 Laptop GPU
  and AMD Radeon 780M integrated graphics. No second physical PC validation.
- Connected TCL 55-inch 4K television, active 3840x2160 at 60 Hz; the laptop's
  scaled display was used for earlier DPI/fullscreen tests.
- NVIDIA and AMD hardware D3D11 devices plus Microsoft's WARP software device
  were used for shader controls. Hardware/WARP texture controls used real D3D
  textures and the SDK validation layer.
- Local compilation used MSVC 2022 Build Tools 14.44 and Windows SDK 10.0.26100.
  Local Release/Speed and Master builds passed before filtering the submission.
  Those builds used compatibility flags excluded from this branch; a clean
  build with the upstream-supported environment remains required.
- InputFusion 0.4.1 DLL SHA256:
  `242e16a3a5904eb9068bd442c6a466fd983dbbe7851022704985dce3fb6ec408`.
  Inno Setup 7.1.1 is prepared only as a build tool. The local setup included
  Microsoft's signed x64 C++ runtime 14.51.36247.0.

## Completed checks

### Component and packaging controls

- Source identities and published shader bytecode establish the original
  particle/lock/shader mechanisms predate our recompilation.
- Shader controls: 12 expected outcomes across two original/corrected pairs
  and three devices. Corrected pairs produce no linkage errors.
- Actual-source texture and particle controls; manifest-controlled window-size
  checks; verbatim refresh-enumeration-loop checks.
- InputFusion ON/OFF configuration, disabled module compilation, pinned
  archive/DLL hashes, explicit system fallback and packaged notices checked.
- Unchanged production backend loader controls pass with the real DLL present,
  DLL missing, a fixture transitive DLL missing, invalid image and explicit
  system selection. System DLLs were not removed to perform these tests.
- Official/current local game DLL-name import sets are identical. Both the game
  and backend need the shared x64 VC runtime. Import-name equality does not
  prove equal minimum runtime versions. The existing x86 companion uses `/MT`
  and imports only KERNEL32/WS2_32; it adds no .NET requirement.
- The native setup bootstrap imports no VC runtime of its own. It can start
  before the prerequisite it installs. The compiler download is hash-pinned
  and signature-checked; the bundled redistributable's Microsoft signature and
  x64 identity/version are checked. Package provenance records exact hashes.
- Installer install/update/uninstall tests restore pre-existing managed files
  and preserve the retail executable, an unmanaged root XInput DLL and custom
  settings. Busy/invalid targets fail before replacing game files. No process
  is automatically terminated. Shared C++ runtime is retained on uninstall.
- Selected patch applies to pristine affected base files; whitespace and
  regenerated CMake source-list checks pass. Shared files were filtered by
  hunk, not taken wholesale.

### Gameplay and physical acceptance

The corrected local player passed eight race loads/six ordinary event-menu
reloads across validation and normal runs at high detail/8x MSAA/V-Sync, without
logged lock/SDK failures. Settings and private saves isolated automated runs.

An untouched official v1.6 comparison reached its first race and stalled returning
to the menu. Its 120-second scripted condition timed out (exit 3), while the
modified local build completed four loads/three reloads (exit 0). This is **not**
an isolated attribution: official Master versus local Release/Speed, local-only
cleanup and DPI-dependent physical dimensions differ. Do not present it as a
performance benchmark or assign that stall to an individual patch.

Finally, roughly 7.434 GiB of game data was copied to a separate physical game
folder. Existing ASI/Discord add-ons were omitted. Actual Setup installed the
whole payload, preserved retail Blur.exe, and created a direct Start-menu launch.
The existing machine already satisfied the VC prerequisite. Two automated race
loads and a normal menu reload passed (exit 0/144 s); a true-fullscreen follow-up
loaded a third race. The first harness run intentionally forced a windowed
3840x2097 client; that was distinguished from the later fullscreen test, not
treated as a renderer defect.

The real selected GPU was RTX 4050, TV output 3840x2160 at 60 Hz, V-Sync interval
1, Present S_OK, with no per-launch GPU/HDR/input override. GPU counters showed
actual RTX rendering. The existing NVIDIA overlay visibly reported 60 FPS;
PresentMon ETW access was unavailable and no sustained frame-time trace was
obtained. No measured end-to-end latency or FPS percentile claim is made.

After connecting DualSense by Bluetooth, the installed backend detected slot 0.
The user explicitly confirmed steering/acceleration, vibration, colours/screen
fit and the normal blue Options pause all work, with DS4Windows closed. This
acceptance covers the installed local build; it does not certify the unbuilt
filtered source tree in this branch.

## Exclusions and issues encountered

- No developer HUD/statistics cleanup, red debug-menu/pause-input gating,
  PlayerDiagnostics helper or changed developer-mode callback. Those were
  local test-delivery corrections, not proposed upstream improvements. No
  split-screen/shared-menu redesign or additional testing UI is introduced.
- The original timing-report function and developer-mode callback are preserved
  in shared source files; only FPS functionality is selected there.
- No personal launchers, executable renaming, GPU/DPI registry preferences,
  private saves/logs/dumps, local compiler stopgaps or machine-specific paths.
- No driver update/fix, 16x MSAA unlock, FXAA/TAA, true borderless/render scaling,
  refresh selector or HDR calibration menu is claimed.
- An intermediate direct texture-destruction cleanup introduced a lock-order
  deadlock. It was replaced with deferred retirement before final testing;
  destruction while the registry lock is held is now covered by the control.
- Installer tests found manifest encoding, deeply nested backup-path and
  uninstall timestamp-argument defects. They were corrected before the final
  accepted setup. Stable numbered backup records avoid nested backup paths.
- The package initially included the backend while the README's inherited copy
  list omitted it. Documentation was corrected; Setup now handles layout.
- An NVIDIA worker-thread crash observed when reusing a startup-only race helper
  has no established root cause. Ordinary event-menu transitions were tested
  separately. Do not label it a repaired graphics-driver defect.

## Remaining before a binary release

1. Build and run this **exact filtered tree** with the supported upstream
   toolchain. Regenerate all `.cp/.cv/.sp` products and legacy/native built-in
   shaders; do not reuse old shader artifacts after interface changes.
2. Validate its player configuration, FPS save/rollback, USB/Bluetooth rumble,
   display transitions and ordinary race/results flows. Current local acceptance
   does not waive this step.
3. Exercise the complete resulting Setup on clean Windows with only documented
   prerequisites, including the missing/old C++ runtime installation path.
   Existing-PC runtime detection and component fallback checks are insufficient.
4. Run upstream/fork CI and test another physical PC. Cover mixed-DPI monitor
   movement/hot-plug, multi-output adapter selection and more event/results/online
   flows as appropriate. These remain unverified.
5. The local preview Setup is unsigned. Public release signing, localization and
   maintainer acceptance of adapter-default policy remain release decisions.

Source publication as a draft is intended to make these changes reviewable;
it is not an announcement of a production-ready release.

## Credit and dependency references

Rebuilt and its existing renderer/game behavior remain upstream work. InputFusion
is integrated unmodified and dynamically replaceable, not authored by this
contribution. Its pinned source and LGPL/GPL/SDL/Detours notices accompany the
package; see [the dependency notice](../../code/vendor/inputfusion/notices/NOTICE.txt).

- [Pinned InputFusion source](https://github.com/xan105/InputFusion/tree/e3e6e7f97f8e7b3927a09a4649c2ec3bef9ef956)
- [Microsoft C++ runtime requirements](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist)
- [D3D11 optional debug layer](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-devices-layers)
- [DXGI adapter preference requirements](https://learn.microsoft.com/en-us/windows/win32/api/dxgi1_6/nf-dxgi1_6-idxgifactory6-enumadapterbygpupreference)

## Graphics, display, and setup follow-up

The graphics menu keeps all eight rows visible. The settings file persists the FPS cap and replaces the previous file after a complete write. Display mode enumeration follows the monitor that owns the game window. Fullscreen records the prior desktop mode and leaves the window size in place if Windows rejects a mode. Save creation writes data on the first attempt, and photo enumeration respects its existing capacity. Setup checks backup mappings before replacing files and supports read-only retail executables.

The local x64 Release build reported zero errors. It passed three Graphics menu entries and returns, restored a saved 90 FPS cap after relaunch, reloaded a first save after recreating the save directory, and returned 200 entries for 201 photo files. The installer fixture passed install, update, uninstall, and invalid backup mapping checks. This filtered contribution still needs its own full build and runtime checks, including clean Windows setup and mixed-monitor changes. Intermittent blank Options and the separate race colour corruption remain open.
