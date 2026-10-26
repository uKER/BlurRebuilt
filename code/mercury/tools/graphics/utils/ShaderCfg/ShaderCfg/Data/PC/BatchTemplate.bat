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

set VS_OPTIONS=/T vs_3_0 /I%IDIR%
set PS_OPTIONS=/T ps_3_0 /I%IDIR%

%COMP% %VS_OPTIONS% /Fo %ODIR%/$(VERTEX_FILENAME).pcv  %SDIR%/vsWorld.fx %DEFINES% /D HLSL /D USE_POINTLIGHTS $(DEFINES)

%COMP% %PS_OPTIONS% /Fo %ODIR%/$(PIXEL_FILENAME).pcp  %SDIR%/psWorld.fx %DEFINES% /D HLSL $(DEFINES)

pause

echo.
echo Copying %SHADER_NAME% Shaders to %PLATFORM_NAME%
echo.

%COPY% %ODIR%\$(VERTEX_FILENAME).pcv %CDIR%

%COPY% %ODIR%\$(PIXEL_FILENAME).pcp %CDIR%

%COPY% ShaderRegistry.pcb %ODIR%
%COPY% ShaderRegistry.pcb %CDIR%

pause
