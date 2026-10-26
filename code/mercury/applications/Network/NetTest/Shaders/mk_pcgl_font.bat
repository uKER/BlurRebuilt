set COMP="%CG_BIN_PATH%\cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\render"
set ODIR=".\pcgl"
set CDIR="..\Data\Shaders"

cls

%COMP% -profile vp30 -I%IDIR% -o %ODIR%/vsFontRender.pcglv %SDIR%\vsFontRender.vsh

%COPY% %ODIR%\vsFontRender.pcglv %CDIR%

%COMP% -profile fp30 -I%IDIR% -o %ODIR%/psFontRender.pcglp %SDIR%\psFontRender.psh

%COPY% %ODIR%\psFontRender.pcglp %CDIR%

pause