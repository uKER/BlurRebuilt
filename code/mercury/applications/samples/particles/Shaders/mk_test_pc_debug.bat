set COMP="%DXSDK_DIR%Utilities\Bin\x86\fxc"
set COPY=copy
set IDIR=%SRC_ROOT%\code\mercury\library\display\include\display\shaders
set SDIR=%SRC_ROOT%\code\mercury\library\render\include\render\shaders\debug
set ODIR=.\pc
set CDIR=..\Data\Shaders

cls

%COMP% /T vs_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsDebug2D.pcv %SDIR%\vsDebug2D.vsh
%COMP% /T vs_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsDebug3D.pcv %SDIR%\vsDebug3D.vsh


%COPY% %ODIR%\vsDebug2D.pcv %CDIR%
%COPY% %ODIR%\vsDebug3D.pcv %CDIR%

%COMP% /T ps_2_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psDebug.pcp %SDIR%\psDebug.psh

%COPY% %ODIR%\psDebug.pcp %CDIR%

pause