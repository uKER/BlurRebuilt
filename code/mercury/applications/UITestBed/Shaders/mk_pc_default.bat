set COMP="%DXSDK_DIR%Utilities\Bin\x86\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\display\include\display\shaders"
set SDIR="..\..\..\\library\display\include\display\shaders\render"
set ODIR=".\pcd3d9"
set CDIR="..\Data\Shaders"

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%\vsDefault_XY.pcv %SDIR%\vsDefault_XY.vsh

%COPY% %ODIR%\vsDefault_XY.pcv %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%\psDefault_UV.pcp %SDIR%\psDefault_UV.psh

%COPY% %ODIR%\psDefault_UV.pcp %CDIR%

pause