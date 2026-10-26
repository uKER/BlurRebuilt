set COMP="sce-cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\render\include\render\shaders"
set SDIR="..\..\..\library\render\include\render\shaders\sky"
set ODIR=".\ps3"
set CDIR="..\Data\Shaders"

cls

%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsSkyDome.p3v %SDIR%\vsSkyDome.vsh

%COPY% %ODIR%\vsSkyDome.p3v %CDIR%

%COMP% -p sce_fp_rsx -I %IDIR% -o %ODIR%/psSkyDome.p3p %SDIR%\psSkyDome.psh

%COPY% %ODIR%\psSkyDome.p3p %CDIR%

pause