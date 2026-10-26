@echo off

set COMP="%XEDK%\bin\win32\fxc" /nologo /XZi
set COPY="%XEDK%\bin\win32\xbcp" /f /y
set ODIR=%BIZARRE_ROOT%\shitstorm\working\360\data\Image\Shaders\World
set CDIR=xe:\shooter_xenon\Image\Shaders\World
rem set CDIR=xe:\genesis\minimal_xenon\Data\Shaders\World

set PLATFORM_NAME=360

set DEFINES=
set DEFINES=%DEFINES% /D PLATFORM_XENON
set DEFINES=%DEFINES% /D XB2
set DEFINES=%DEFINES% /D FX_DEBUG
set DEFINES=%DEFINES% /D USE_PCFSHADOWMAP
