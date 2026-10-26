set COMP="sce-cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\render\include\render\shaders"
set SDIR="..\..\..\library\render\include\render\shaders\debug"
set ODIR=".\ps3"
set CDIR="..\Data\Shaders"

cls

%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsDebug2D.p3v %SDIR%\vsDebug2D.vsh
%COMP% -p sce_vp_rsx -I %IDIR% -o %ODIR%/vsDebug3D.p3v %SDIR%\vsDebug3D.vsh

%COPY% %ODIR%\vsDebug2D.p3v %CDIR%
%COPY% %ODIR%\vsDebug3D.p3v %CDIR%

%COMP% -p sce_fp_rsx -I %IDIR% -o %ODIR%/psDebug.p3p %SDIR%\psDebug.psh

%COPY% %ODIR%\psDebug.p3p %CDIR%

pause