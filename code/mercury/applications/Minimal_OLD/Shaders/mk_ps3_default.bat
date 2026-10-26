set COMP="sce-cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\render"
set ODIR=".\ps3"
set CDIR="..\Data\Shaders"

cls

%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsDefault_XY.p3v %SDIR%\vsDefault_XY.vsh

%COPY% %ODIR%\vsDefault_XY.p3v %CDIR%

%COMP% -p sce_fp_rsx -I %IDIR% -o %ODIR%/psDefault_UV.p3p %SDIR%\psDefault_UV.psh

%COPY% %ODIR%\psDefault_UV.p3p %CDIR%

pause