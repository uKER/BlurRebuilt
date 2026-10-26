call BatchSetVars

set SHADER_NAME=WorldRender World
title Building %PLATFORM_NAME% %SHADER_NAME% Shaders

cls
mkdir %ODIR%

echo.
echo Building %PLATFORM_NAME% %SHADER_NAME% Shaders
echo.

set IDIR=%SRC_ROOT%\code\genesis\library\include\render\shaders\world
set SDIR=%SRC_ROOT%\code\genesis\library\include\render\shaders\world

set VS_OPTIONS=/Xbe:1- /T vs_3_0 /I %IDIR% 
set PS_OPTIONS=/Xbe:1- /T ps_3_0 /I %IDIR%

%COMP% %VS_OPTIONS% /Fo %ODIR%/$(VERTEX_FILENAME).xev  %SDIR%/vsWorld.fx %DEFINES% /D HLSL $(DEFINES)

%COMP% %PS_OPTIONS% /Fo %ODIR%/$(PIXEL_FILENAME).xep  %SDIR%/psWorld.fx %DEFINES% /D HLSL $(DEFINES)

pause

echo.
echo Copying %SHADER_NAME% Shaders to %PLATFORM_NAME%
echo.

%COPY% %ODIR%\$(VERTEX_FILENAME).xev %CDIR%

%COPY% %ODIR%\$(PIXEL_FILENAME).xep %CDIR%

%COPY% ShaderRegistry.xeb %ODIR%
%COPY% ShaderRegistry.xeb %CDIR%

pause
