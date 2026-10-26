set COMP="%DXSDK_DIR%Utilities\Bin\x86\fxc"
set COPY=copy
set IDIR="%SRC_ROOT%\code\mercury\library\display\include\display\shaders"
set SDIR="%SRC_ROOT%\code\mercury\library\display\include\display\shaders\render"
set ODIR=".\pc"
set CDIR="%SRC_ROOT%\code\mercury\applications\samples\particles\Data\Shaders"

%COMP% /T vs_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\vsBasicRenderColoured2D.pcv %SDIR%\vsBasicRenderColoured2D.vsh
%COMP% /T vs_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\vsBasicRenderColoured3D.pcv %SDIR%\vsBasicRenderColoured3D.vsh
%COMP% /T vs_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\vsBasicRenderTextured2D.pcv %SDIR%\vsBasicRenderTextured2D.vsh
%COMP% /T vs_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\vsBasicRenderTextured3D.pcv %SDIR%\vsBasicRenderTextured3D.vsh

%COPY% %ODIR%\vsBasicRenderColoured2D.pcv %CDIR%
%COPY% %ODIR%\vsBasicRenderColoured3D.pcv %CDIR%
%COPY% %ODIR%\vsBasicRenderTextured2D.pcv %CDIR%
%COPY% %ODIR%\vsBasicRenderTextured3D.pcv %CDIR%

%COMP% /T ps_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\psBasicRenderColoured.pcp %SDIR%\psBasicRenderColoured.psh
%COMP% /T ps_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%\psBasicRenderTextured.pcp %SDIR%\psBasicRenderTextured.psh

%COPY% %ODIR%\psBasicRenderColoured.pcp %CDIR%
%COPY% %ODIR%\psBasicRenderTextured.pcp %CDIR%

pause