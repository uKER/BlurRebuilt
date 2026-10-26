set PATH=%XEDK%\bin\win32;%PATH%;
set INCLUDE=%XEDK%\include\win32;%XEDK%\include\xbox;%XEDK%\include\xbox\sys;%INCLUDE%
set LIB=%XEDK%\lib\win32;%XEDK%\lib\xbox;%LIB%
set _NT_DEBUG_1394_CHANNEL=1
set _NT_DEBUG_BUS=1394
set _NT_SYMBOL_PATH=SRV*%XEDK%\bin\xbox\symsrv;%_NT_SYMBOL_PATH%

rem xcopy /s /y *.lua %BIZARRE_ROOT%\Amax\image\PS3\dontship\scripts
rem xcopy /s /y *.lua %BIZARRE_ROOT%\Amax\image\PC\dontship\scripts

robocopy . %BIZARRE_ROOT%\mercurygame\image\PS3\dontship\ui\scripts *.lua /S /MIR
robocopy . %BIZARRE_ROOT%\mercurygame\image\PC\dontship\ui\scripts *.lua /S /MIR
cls
xbcp /R /S /T /Y /D *.lua DEVKIT:\mercurygame\dontship\UI\Scripts\


pause
