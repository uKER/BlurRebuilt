set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\\library\display\include\display\shaders\render"
set ODIR=".\xb2"
set CDIR="xe:\mercury\minimal_xenon\Data\Shaders"

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsFontRender.xev %SDIR%\vsFontRender.vsh

%COPY% %ODIR%\vsFontRender.xev %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psFontRender.xep %SDIR%\psFontRender.psh

%COPY% %ODIR%\psFontRender.xep %CDIR%

pause