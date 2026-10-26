set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR=%SRC_ROOT%\code\mercury\library\display\include\display\shaders
set SDIR=%SRC_ROOT%\code\mercury\library\render\include\render\shaders\sky
set ODIR=.\xe
set CDIR=xe:\particles_xe\Data\Shaders

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsSkyDome.xev %SDIR%\vsSkyDome.vsh

%COPY% %ODIR%\vsSkyDome.xev %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psSkyDome.xep %SDIR%\psSkyDome.psh

%COPY% %ODIR%\psSkyDome.xep %CDIR%

pause