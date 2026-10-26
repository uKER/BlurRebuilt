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

set VS_OPTIONS=-q -p sce_vp_rsx -I %IDIR%
set PS_OPTIONS=-q -p sce_fp_rsx -I %IDIR%

%COMP% %VS_OPTIONS% -o %ODIR%/$(VERTEX_FILENAME).p3v  %SDIR%/vsWorld.fx %DEFINES% -DHLSL $(DEFINES)

%COMP% %PS_OPTIONS% -o %ODIR%/$(PIXEL_FILENAME).p3p  %SDIR%/psWorld.fx %DEFINES% -DHLSL $(DEFINES)

pause

echo.
echo Copying %SHADER_NAME% Shaders to %PLATFORM_NAME%
echo.

%COPY% %ODIR%\$(VERTEX_FILENAME).p3v %CDIR%

%COPY% %ODIR%\$(PIXEL_FILENAME).p3p %CDIR%

%COPY% ShaderRegistry.p3b %ODIR%
%COPY% ShaderRegistry.p3b %CDIR%

pause
