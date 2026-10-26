set COMP="%DXSDK_DIR%Utilities\Bin\x86\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\render\include\render\shaders"
set SDIR="..\..\..\library\render\include\render\shaders\sky"
set ODIR=".\pcd3d9"
set CDIR="..\Data\Shaders"

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%\vsSkyDome.pcv %SDIR%\vsSkyDome.vsh

%COPY% %ODIR%\vsSkyDome.pcv %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%\psSkyDome.pcp %SDIR%\psSkyDome.psh

%COPY% %ODIR%\psSkyDome.pcp %CDIR%

pause