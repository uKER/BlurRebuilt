set COMP="%CG_BIN_PATH%\cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\render"
set ODIR=".\pcgl"
set CDIR="..\Data\Shaders"

cls

%COMP% -profile vp30 -I%IDIR% -o %ODIR%/vsBasicRenderColoured2D.pcglv %SDIR%\vsBasicRenderColoured2D.vsh
%COMP% -profile vp30 -I%IDIR% -o %ODIR%/vsBasicRenderColoured3D.pcglv %SDIR%\vsBasicRenderColoured3D.vsh
%COMP% -profile vp30 -I%IDIR% -o %ODIR%/vsBasicRenderTextured2D.pcglv %SDIR%\vsBasicRenderTextured2D.vsh
%COMP% -profile vp30 -I%IDIR% -o %ODIR%/vsBasicRenderTextured3D.pcglv %SDIR%\vsBasicRenderTextured3D.vsh

%COPY% %ODIR%\vsBasicRenderColoured2D.pcglv %CDIR%
%COPY% %ODIR%\vsBasicRenderColoured3D.pcglv %CDIR%
%COPY% %ODIR%\vsBasicRenderTextured2D.pcglv %CDIR%
%COPY% %ODIR%\vsBasicRenderTextured3D.pcglv %CDIR%

%COMP% -profile fp30 -I%IDIR% -o %ODIR%/psBasicRenderColoured.pcglp %SDIR%\psBasicRenderColoured.psh
%COMP% -profile fp30 -I%IDIR% -o %ODIR%/psBasicRenderTextured.pcglp %SDIR%\psBasicRenderTextured.psh

%COPY% %ODIR%\psBasicRenderColoured.pcglp %CDIR%
%COPY% %ODIR%\psBasicRenderTextured.pcglp %CDIR%

pause