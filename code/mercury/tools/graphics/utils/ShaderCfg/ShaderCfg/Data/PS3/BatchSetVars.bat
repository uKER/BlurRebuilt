@echo off

set COMP="C:\usr\local\cell\host-win32\Cg\bin\sce-cgc"
set COPY=xcopy /d /f /y /k /r /c 
set ODIR=%BIZARRE_ROOT%\shitstorm\working\PS3\data\Image\Shaders\World
set CDIR=%BIZARRE_ROOT%\shitstorm\game\PS3\Data\Image\Shaders\World
rem set CDIR=%SRC_ROOT%\code\genesis\applications\Minimal\Data\Shaders\World

set PLATFORM_NAME=PS3

set DEFINES=
set DEFINES=%DEFINES% -DPLATFORM_PS3
set DEFINES=%DEFINES% -DPS3
rem set DEFINES=%DEFINES% -DFX_DEBUG
