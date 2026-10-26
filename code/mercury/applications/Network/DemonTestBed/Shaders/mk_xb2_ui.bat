set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR="..\..\..\library\ui\include\ui\shaders"
set SDIR="..\..\..\library\ui\include\ui\shaders\ui"
set ODIR=".\xb2"
set CDIR="xe:\uitestbed\Image\Shaders"

mkdir %ODIR%
mkdir %CDIR%

cls

%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsFontEffects.xev %SDIR%\vsFontEffects.vsh /D XB2 /D PLATFORM_XENON
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsMultiTexture.xev %SDIR%\vsMultiTexture.vsh /D XB2 /D PLATFORM_XENON
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsNormalMapOffset.xev %SDIR%\vsNormalMapOffset.vsh /D XB2 /D PLATFORM_XENON
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsPixelLight.xev %SDIR%\vsPixelLight.vsh /D XB2 /D PLATFORM_XENON
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsPolygonFlat.xev %SDIR%\vsPolygonFlat.vsh /D XB2 /D PLATFORM_XENON
%COMP% /T vs_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/vsPolygonGradient.xev %SDIR%\vsPolygonGradient.vsh /D XB2 /D PLATFORM_XENON

%COPY% %ODIR%\vsFontEffects.xev %CDIR%
%COPY% %ODIR%\vsMultiTexture.xev %CDIR%
%COPY% %ODIR%\vsNormalMapOffset.xev %CDIR%
%COPY% %ODIR%\vsPixelLight.xev %CDIR%
%COPY% %ODIR%\vsPolygonFlat.xev %CDIR%
%COPY% %ODIR%\vsPolygonGradient.xev %CDIR%

%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psFontEffects.xep %SDIR%\psFontEffects.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psFontIntensityOutline.xep %SDIR%\psFontIntensityOutline.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psFontIntensityAsAlpha.xep %SDIR%\psFontIntensityAsAlpha.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psMultiTexture.xep %SDIR%\psMultiTexture.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psMultiTexture2.xep %SDIR%\psMultiTexture.psh /D XB2 /D PLATFORM_XENON /D SAMPLERS_2
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psMultiTexture3.xep %SDIR%\psMultiTexture.psh /D XB2 /D PLATFORM_XENON /D SAMPLERS_3
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psMultiTextureFullAlpha.xep %SDIR%\psMultiTextureFullAlpha.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psMultiTextureFullAlpha2.xep %SDIR%\psMultiTextureFullAlpha.psh /D XB2 /D PLATFORM_XENON /D DSAMPLERS_2
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psMultiTextureFullAlpha3.xep %SDIR%\psMultiTextureFullAlpha.psh /D XB2 /D PLATFORM_XENON /D DSAMPLERS_3
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psGreyScale.xep %SDIR%\psGreyScale.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psNormalMapOffset.xep %SDIR%\psNormalMapOffset.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psBlobbyFont.xep %SDIR%\psBlobbyFont.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psMovie.xep %SDIR%\psMovie.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psMovieWithAlpha.xep %SDIR%\psMovieWithAlpha.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psPolygonFlat.xep %SDIR%\psPolygonFlat.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psPolygonGradient.xep %SDIR%\psPolygonGradient.psh /D XB2 /D PLATFORM_XENON
%COMP% /T ps_3_0 /E main /nologo /I %IDIR% /Fo %ODIR%/psPolygonGradient_Param.xep %SDIR%\psPolygonGradient.psh /D XB2 /D PLATFORM_XENON /D _USE_PARAM_

%COPY% %ODIR%\psFontEffects.xep %CDIR%
%COPY% %ODIR%\psFontIntensityOutline.xep %CDIR%
%COPY% %ODIR%\psFontIntensityAsAlpha.xep %CDIR%
%COPY% %ODIR%\psMultiTexture.xep %CDIR%
%COPY% %ODIR%\psMultiTexture2.xep %CDIR%
%COPY% %ODIR%\psMultiTexture3.xep %CDIR%
%COPY% %ODIR%\psMultiTextureFullAlpha.xep %CDIR%
%COPY% %ODIR%\psMultiTextureFullAlpha2.xep %CDIR%
%COPY% %ODIR%\psMultiTextureFullAlpha3.xep %CDIR%
%COPY% %ODIR%\psGreyScale.xep %CDIR%
%COPY% %ODIR%\psNormalMapOffset.xep %CDIR%
%COPY% %ODIR%\psBlobbyFont.xep %CDIR%
%COPY% %ODIR%\psMovie.xep %CDIR%
%COPY% %ODIR%\psMovieWithAlpha.xep %CDIR%
%COPY% %ODIR%\psPolygonFlat.xep %CDIR%
%COPY% %ODIR%\psPolygonGradient.xep %CDIR%
%COPY% %ODIR%\psPolygonGradient_Param.xep %CDIR%

pause