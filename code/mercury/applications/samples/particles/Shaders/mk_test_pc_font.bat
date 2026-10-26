set COMP="%DXSDK_DIR%Utilities\Bin\x86\fxc"
set COPY=copy
set IDIR="%SRC_ROOT%\code\mercury\library\display\include\display\shaders"
set SDIR="%SRC_ROOT%\code\mercury\library\display\include\display\shaders\render"
set ODIR=".\pc"
set CDIR="%SRC_ROOT%\code\mercury\applications\samples\particles\Data\Shaders"

%COMP% /T vs_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\vsFontRender.pcv %SDIR%\vsFontRender.vsh

%COPY% %ODIR%\vsFontRender.pcv %CDIR%

%COMP% /T ps_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\psFontRender.pcp %SDIR%\psFontRender.psh

%COPY% %ODIR%\psFontRender.pcp %CDIR%

pause