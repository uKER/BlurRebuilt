set COMP="%DXSDK_DIR%Utilities\Bin\x86\fxc"
set COPY=copy
set IDIR="%SRC_ROOT%\code\mercury\library\display\include\display\shaders"
set SDIR="%SRC_ROOT%\code\mercury\library\render\include\render\shaders\sky"
set ODIR=".\pc"
set CDIR="%SRC_ROOT%\code\mercury\applications\samples\particles\Data\Shaders"

%COMP% /T vs_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\vsSkyDome.pcv %SDIR%\vsSkyDome.vsh

%COPY% %ODIR%\vsSkyDome.pcv %CDIR%

%COMP% /T ps_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\psSkyDome.pcp %SDIR%\psSkyDome.psh

%COPY% %ODIR%\psSkyDome.pcp %CDIR%

pause