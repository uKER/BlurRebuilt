set COMP="%CG_BIN_PATH%\cgc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\library\display\include\display\shaders\test"
set ODIR=".\pcgl"
set CDIR="..\Data\Shaders"

cls

%COMP% -profile vp30 -I%IDIR% -o %ODIR%/vsTest.pcglv %SDIR%\vsTest.vsh

%COPY% %ODIR%\vsTest.pcglv %CDIR%

%COMP% -profile fp30 -I%IDIR% -o %ODIR%/psTest.pcglp %SDIR%\psTest.psh

%COPY% %ODIR%\psTest.pcglp %CDIR%

pause