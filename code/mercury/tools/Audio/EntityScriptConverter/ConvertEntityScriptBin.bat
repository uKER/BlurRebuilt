echo off
set BIN_FILENAME=data.bin
echo converting bin file 'export\%BIN_FILENAME%' ...
if exist %BIN_FILENAME% del %BIN_FILENAME%
EntityScriptConverter export\%BIN_FILENAME%
if errorlevel 1 goto convertfailed
echo created bin file '%BIN_FILENAME%' ok
echo copying bin to kit...
"C:\Program Files\Microsoft Xbox 360 SDK\bin\win32\xbcp" /T /Y %BIN_FILENAME% DEVKIT:\mercurygame\data\%BIN_FILENAME%	
pause
exit
:convertfailed
echo bin file conversion failed
pause
exit
