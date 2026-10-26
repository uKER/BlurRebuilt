set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\render\include\render\shaders"
set SDIR="..\..\..\library\render\include\render\shaders\debug"
set ODIR=".\xb2"
set CDIR="xe:\mercury\uitestbed_xenon\Data\Shaders"

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsDebug2D.xev %SDIR%\vsDebug2D.vsh
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsDebug3D.xev %SDIR%\vsDebug3D.vsh


%COPY% %ODIR%\vsDebug2D.xev %CDIR%
%COPY% %ODIR%\vsDebug3D.xev %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psDebug.xep %SDIR%\psDebug.psh

%COPY% %ODIR%\psDebug.xep %CDIR%

pause