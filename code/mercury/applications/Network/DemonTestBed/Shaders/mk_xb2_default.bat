set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\\library\display\include\display\shaders\render"
set ODIR=".\xb2"
set CDIR="xe:\mercury\uitestbed_xenon\Data\Shaders"

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsDefault_XY.xev %SDIR%\vsDefault_XY.vsh

%COPY% %ODIR%\vsDefault_XY.xev %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psDefault_UV.xep %SDIR%\psDefault_UV.psh

%COPY% %ODIR%\psDefault_UV.xep %CDIR%

pause