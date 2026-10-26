set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\render\include\render\shaders"
set SDIR="..\..\..\library\render\include\render\shaders\sky"
set ODIR=".\xb2"
set CDIR="xe:\mercury\minimal_xenon\Data\Shaders"

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsSkyDome.xev %SDIR%\vsSkyDome.vsh

%COPY% %ODIR%\vsSkyDome.xev %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psSkyDome.xep %SDIR%\psSkyDome.psh

%COPY% %ODIR%\psSkyDome.xep %CDIR%

pause