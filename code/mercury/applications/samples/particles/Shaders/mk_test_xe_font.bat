set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR=%SRC_ROOT%\code\mercury\library\display\include\display\shaders
set SDIR=%SRC_ROOT%\code\mercury\library\display\include\display\shaders\render
set ODIR=.\xe
set CDIR=xe:\particles_xe\Data\Shaders

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsFontRender.xev %SDIR%\vsFontRender.vsh

%COPY% %ODIR%\vsFontRender.xev %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psFontRender.xep %SDIR%\psFontRender.psh

%COPY% %ODIR%\psFontRender.xep %CDIR%

pause