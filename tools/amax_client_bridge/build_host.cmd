@echo off
rem [Blur2PC] Build the x86 companion from the same VS toolchain as the x64 game.
rem Arguments: Visual Studio installation, repository root, output directory.
call "%~1\VC\Auxiliary\Build\vcvars32.bat" >nul
if errorlevel 1 exit /b %errorlevel%
if not exist "%~3" mkdir "%~3"
pushd "%~3"
cl /nologo /W4 /WX /O2 /Zi /MT /EHsc "%~2\code\amax\library\Network\AMAX\ClientHost.cpp" /Fe:AmaxClientHost.exe /Fd:AmaxClientHost.pdb /link /BASE:0x400000 /FIXED /DYNAMICBASE:NO /INCREMENTAL:NO /DEBUG
set bridge_build_result=%errorlevel%
popd
exit /b %bridge_build_result%
