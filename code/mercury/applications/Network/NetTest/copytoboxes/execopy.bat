@echo copy nettest app to box. 
@echo param #1 : target
@echo param #2 : build (debug / release)
@color 37

set _LOCALISATION="%BIZARRE_ROOT%\Nightwing\working\360\Image\Xt\Loc\Multiplayer\Multiplayer-en.loc"
set _SEND="%BIZARRE_ROOT%\tools\common_bin\xbrobocopy\xbrobocopy.exe" /E /X360
set _CACHE=".\cache"
set _SOURCE="..\bin\Xbox_360\%2\*.xex"
set _DEST="DEVKIT:\mercury"

@echo SEND command : %_SEND%
@echo TARGET       : %1%
@echo BUILD        : %2%
@echo LOCALISATION : %_LOCALISATION%
@echo SOURCE       : %_SOURCE%
@echo CACHE        : %_CACHE%
@echo DEST         : %_DEST%

xcopy /Y /D %_SOURCE% %_CACHE%
xcopy /Y /D %_LOCALISATION% ".\cache\image\xt"
%_SEND% %1 %_CACHE% %_DEST%
pause
exit

