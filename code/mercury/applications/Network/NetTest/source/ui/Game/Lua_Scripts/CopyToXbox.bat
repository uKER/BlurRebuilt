set PATH=%XEDK%\bin\win32;%PATH%;
set INCLUDE=%XEDK%\include\win32;%XEDK%\include\xbox;%XEDK%\include\xbox\sys;%INCLUDE%
set LIB=%XEDK%\lib\win32;%XEDK%\lib\xbox;%LIB%
set _NT_DEBUG_1394_CHANNEL=1
set _NT_DEBUG_BUS=1394
set _NT_SYMBOL_PATH=SRV*%XEDK%\bin\xbox\symsrv;%_NT_SYMBOL_PATH%

xbcp /R /S /T /Y /D *.lua DEVKIT:\nettest\User\UI\Scripts\

rem pause
