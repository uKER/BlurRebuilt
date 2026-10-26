set COMP="sce-cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\occlusion"
set ODIR=".\ps3"
set CDIR="..\Data\Shaders"

cls

%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsOcclusion.p3v %SDIR%\vsOcclusion.vsh

%COPY% %ODIR%\vsOcclusion.p3v %CDIR%

%COMP% -p sce_fp_rsx -I %IDIR% -o %ODIR%/psOcclusion.p3p %SDIR%\psOcclusion.psh

%COPY% %ODIR%\psOcclusion.p3p %CDIR%

pause