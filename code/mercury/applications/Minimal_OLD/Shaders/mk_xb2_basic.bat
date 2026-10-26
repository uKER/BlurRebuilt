set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\render"
set ODIR=".\xb2"
set CDIR="xe:\mercury\minimal_xenon\Data\Shaders"

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsBasicRenderColoured2D.xev %SDIR%\vsBasicRenderColoured2D.vsh
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsBasicRenderColoured3D.xev %SDIR%\vsBasicRenderColoured3D.vsh
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsBasicRenderTextured2D.xev %SDIR%\vsBasicRenderTextured2D.vsh
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsBasicRenderTextured3D.xev %SDIR%\vsBasicRenderTextured3D.vsh

%COPY% %ODIR%\vsBasicRenderColoured2D.xev %CDIR%
%COPY% %ODIR%\vsBasicRenderColoured3D.xev %CDIR%
%COPY% %ODIR%\vsBasicRenderTextured2D.xev %CDIR%
%COPY% %ODIR%\vsBasicRenderTextured3D.xev %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psBasicRenderColoured.xep %SDIR%\psBasicRenderColoured.psh
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psBasicRenderTextured.xep %SDIR%\psBasicRenderTextured.psh

%COPY% %ODIR%\psBasicRenderColoured.xep %CDIR%
%COPY% %ODIR%\psBasicRenderTextured.xep %CDIR%

pause