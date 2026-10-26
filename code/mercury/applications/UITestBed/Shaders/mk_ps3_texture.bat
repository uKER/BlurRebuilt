set COMP="sce-cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\render"
set ODIR=".\ps3"
set CDIR="..\Data\Shaders"

cls

%COMP% -p sce_fp_rsx -I %IDIR% -o %ODIR%/psTextureRenderRGB.p3p %SDIR%\psTextureRender.psh -DRGB
%COMP% -p sce_fp_rsx -I %IDIR% -o %ODIR%/psTextureRenderA.p3p %SDIR%\psTextureRender.psh -DA

%COPY% %ODIR%\psTextureRenderRGB.p3p %CDIR%
%COPY% %ODIR%\psTextureRenderA.p3p %CDIR%

pause