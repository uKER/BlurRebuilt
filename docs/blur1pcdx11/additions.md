# Controller, display and FPS settings

## Automatic installation

The recommended release is BlurRebuilt-<version>-Setup.exe. Confirm the installed
Blur folder and setup places the complete payload, InputFusion and notices
together. It includes the signed Microsoft x64 C++ redistributable and installs
it only when needed, without requiring a download. It preserves retail Blur.exe
and saves, refuses files in use, and backs up replaced files for restoration on
uninstall. The ZIP remains an optional manual installation path.

Build setup using tools/installer/build.ps1 against the release staging folder.
The builder prepares a verified portable Inno compiler and Microsoft runtime;
these are build/distribution inputs, not tools players must install themselves.
The normal release workflow produces and publishes both Setup EXE and ZIP.

## Controller backend

InputFusion 0.4.1 is an optional, unmodified bundled DLL. The game explicitly
calls its input/rumble API while retaining system XInput as a selectable fallback.
Run `powershell -File tools/inputfusion/setup.ps1` before configuring the default
x64 build. The setup verifies the official archive and DLL SHA256 values.
An offline archive can be passed with `-ArchivePath`. Notices accompany packages.
Configure `-DBLUR_WITH_INPUTFUSION=OFF` to omit the dependency entirely.
`BLUR_INPUT_BACKEND=system` selects system XInput in a bundled build.
Use only one translation path: close DS4Windows for direct DualSense operation.
This adds standard controls/rumble, not adaptive triggers or new button artwork.

## Portable dependencies

The existing Windows 10/11 x64, DirectX 11 and current x64 Visual C++ runtime
requirements still apply. The runtime must be at least as recent as the tools
used to build the shipped binaries. Both the game and InputFusion import it;
backend fallback cannot replace a missing runtime needed to start the game.
Copy BlurInputFusion.dll and InputFusion-Licenses beside the executable and
retain the complete fx11 folder. SDL and Detours are compiled into InputFusion;
no separate SDL installation, DS4Windows, ViGEm, Steam or NVIDIA App is required.
Connect the controller by USB or pair it in Windows; it must be visible to games.
Missing/invalid/unloadable backends use system XInput, losing direct DualSense
support. BLUR_INPUT_BACKEND=system deliberately chooses that fallback.

For building, the new archive setup uses PowerShell and tar.exe with 7z support.
It accepts a pinned offline archive or the optional dependency can be disabled.
The existing build still needs Python, CMake, MSVC x86/x64 tools, Windows SDK
and its shader compiler; GPU preference requires SDK dxgi1_6.h. Windows without
the runtime factory interface uses system adapter selection. PerMonitorV2 is a
manifest declaration, with the legacy true/pm fallback, not a new DLL install.
Players do not need the build tools or Microsoft Graphics Tools. No new shader
compiler installation is needed to play the packaged release.

PE import checks find identical DLL-name sets in the official/current game;
InputFusion's additional imports are Windows system libraries and the same C++
runtime family. The existing x86 AmaxClientHost uses a static C++ runtime and
imports only KERNEL32/WS2_32, adding no .NET or x86 C++ runtime prerequisite.
The production backend passed present, missing, missing-transitive-dependency,
bad-image and explicit-system loader controls. A clean Windows VM/second-PC
package acceptance test remains pending; these checks do not establish it.

## Display and frame limit

The executable declares PerMonitorV2 DPI awareness. The renderer requests a
high-performance hardware adapter; `BLUR_DX11_GPU=system` retains OS selection.
Explicit WARP requests retain precedence. Adapter/output/Present evidence goes
to renderer-startup.log; this contribution adds no on-screen statistics or menus.
Fullscreen preserves the current output refresh when the selected resolution
supports it and otherwise lets the driver select its default.

Graphics > Maximum FPS offers Unlimited, 30, 60, 90, 120 and 144. A valid saved
custom value is retained as an extra option. Accept/confirm saves the setting;
Cancel, No and timeout restore it. The existing eight graphical attributes and
binary profile format remain unchanged: max_fps is stored in a separate named
rebuilt_graphics_settings XML element, version 1. An existing developer
BLUR2PC_FRAMELIMIT startup override is retained. No simulation cadence changes.

## Renderer repairs

Trail allocation now uses the native pointer/salt record size. Texture staging
locks retire with final texture lifetime; deferred collection avoids a
driver-context/registry lock inversion and preserves aliases. Two built-in pixel
shader inputs preserve the register layout of their paired vertex outputs.

## Verification and limits

These changes were exercised in a local development build. Actual-source
regressions cover x64/x86 particle records and hardware/WARP texture ownership,
aliases and repeated destruction. The four original shader programs were found
byte-for-byte inside the official v1.6 executable. Both local configurations
built; a player build passed repeated normal-menu race loads with and without
the SDK validation layer, using high detail, 8x MSAA and V-Sync.

Physical testing was on one hybrid-GPU laptop and one TV. Hardware-dependent
triggers are not universal reproduction claims. Cross-monitor mixed-DPI movement,
all output topologies, every event/results flow and the upstream CI environment
are not comprehensively verified. No AMD/NVIDIA driver fix or measured input
latency improvement is claimed. Proper borderless, render scaling, a refresh
selector, new AA algorithms and HDR menu controls are separate future work.

## Contribution status

This selective branch is a review preview, not a validated binary release.
See [scope and verification](contribution-verification.md) for exact-tree build,
clean-Windows and hardware coverage limits and accepted local-build tests.
