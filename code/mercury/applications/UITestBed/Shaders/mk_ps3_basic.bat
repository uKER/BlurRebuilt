set COMP="sce-cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\render"
set ODIR=".\ps3"
set CDIR="..\Data\Shaders"

cls

%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsBasicRenderColoured2D.p3v %SDIR%\vsBasicRenderColoured2D.vsh
%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsBasicRenderColoured3D.p3v %SDIR%\vsBasicRenderColoured3D.vsh
%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsBasicRenderTextured2D.p3v %SDIR%\vsBasicRenderTextured2D.vsh
%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsBasicRenderTextured3D.p3v %SDIR%\vsBasicRenderTextured3D.vsh

%COPY% %ODIR%\vsBasicRenderColoured2D.p3v %CDIR%
%COPY% %ODIR%\vsBasicRenderColoured3D.p3v %CDIR%
%COPY% %ODIR%\vsBasicRenderTextured2D.p3v %CDIR%
%COPY% %ODIR%\vsBasicRenderTextured3D.p3v %CDIR%

%COMP% -p sce_fp_rsx -I %IDIR% -o %ODIR%/psBasicRenderColoured.p3p %SDIR%\psBasicRenderColoured.psh
%COMP% -p sce_fp_rsx -I %IDIR% -o %ODIR%/psBasicRenderTextured.p3p %SDIR%\psBasicRenderTextured.psh

%COPY% %ODIR%\psBasicRenderColoured.p3p %CDIR%
%COPY% %ODIR%\psBasicRenderTextured.p3p %CDIR%

pause