set COMP="sce-cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\render"
set ODIR=".\ps3"
set CDIR="..\Data\Shaders"

cls

%COMP% -quiet -profile sce_vp_rsx -I %IDIR% -o %ODIR%/vsFontRender.p3v %SDIR%\vsFontRender.vsh

%COPY% %ODIR%\vsFontRender.p3v %CDIR%

%COMP% -quiet -profile sce_fp_rsx -I %IDIR% -o %ODIR%/psFontRender.p3p %SDIR%\psFontRender.psh

%COPY% %ODIR%\psFontRender.p3p %CDIR%

pause