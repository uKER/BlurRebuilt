# ghidra_retail

PyGhidra scripts used to read retail Blur PC (Blur.exe 1.2) through the analysed Ghidra project at `C:\tools\ghidra_proj` (set up by the Brul project; see `C:\AIProjects\Brul\docs\tooling.md` section 2).

Run from PowerShell, inputs through environment variables named at the top of each script:

```
$env:JAVA_HOME = "C:\Program Files\Microsoft\jdk-21.0.12.101-hotspot"
$env:BLUR2PC_ADDRS = "00415a30"; $env:BLUR2PC_OUT = "C:\AIProjects\Blur2PC\_work\ghidra\out.c"
& "C:\tools\ghidra_12.1.2_PUBLIC\support\pyghidraRun.bat" -H C:/tools/ghidra_proj Blur -process Blur.exe -noanalysis -readOnly -scriptPath C:/AIProjects/Blur2PC/Blur1PC/tools/ghidra_retail -postScript DecompileAt.py
```

Findings made with them are in `docs/analysis.md` ("Controls on PC").

Added for T008 (graphics options): `FindDataRefs.py` (who touches a global), `DisasmRange.py` (linear disassembly with strings, works on the DRM-relocated `.8thSin` code), `DevVarCalls.py` (calls on debug-variable globals, resolved to their registered names), `FindVCallSites.py` (calls through a vtable slot). Retail UI Lua: extracted from the exe and decompiled with unluac (`C:\tools\unluac\unluac.jar`) into `_work/retail_lua`; see docs/analysis.md "Graphics options (PC)". A private copy of the Ghidra project at `_work/ghidra_proj` avoids the lock when another session is using `C:\tools\ghidra_proj`.
