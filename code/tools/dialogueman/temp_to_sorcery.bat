@echo off

set src=%APPDATA%\DialogueMan\sorcery3\main
set dst=%1

copy /y "%src%\Packs\*.xtd" "%dst%\Packs\"

echo Done