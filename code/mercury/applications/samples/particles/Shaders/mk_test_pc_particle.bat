
set COMP="%DXSDK_DIR%Utilities\Bin\x86\fxc"
set COPY=copy
set IDIR=%SRC_ROOT%\code\mercury\library\render\include\render\shaders
set SDIR=%SRC_ROOT%\code\mercury\library\render\include\render\shaders\particles
set ODIR=.\pc
set CDIR=..\Data\Shaders

cls

%COMP% /T vs_2_0 /E Main /nologo /D_PARTICLE_TYPE_LINE_=1 /I %IDIR% /Fo %ODIR%/vsParticleLine.pcv %SDIR%\vsParticle.vsh
%COMP% /T vs_2_0 /E Main /nologo /D_PARTICLE_TYPE_SPRITE_=1 /I %IDIR% /Fo %ODIR%/vsParticleSprite.pcv %SDIR%\vsParticle.vsh
%COMP% /T vs_2_0 /E Main /nologo /D_PARTICLE_TYPE_SRRITE_ROTATING_=1 /I %IDIR% /Fo %ODIR%/vsParticleSpriteRotating.pcv %SDIR%\vsParticle.vsh
%COMP% /T vs_2_0 /E Main /nologo /D_PARTICLE_TYPE_TRIANGLE_=1 /I %IDIR% /Fo %ODIR%/vsParticleTriangle.pcv %SDIR%\vsParticle.vsh
%COMP% /T vs_2_0 /E Main /nologo /D_PARTICLE_TYPE_POINT_SPRITE_=1 /I %IDIR% /Fo %ODIR%/vsParticlePointSprite.pcv %SDIR%\vsParticle.vsh
%COMP% /T vs_2_0 /E Main /nologo /D_PARTICLE_TYPE_DEFAULT_=1 /I %IDIR% /Fo %ODIR%/vsParticleDefault.pcv %SDIR%\vsParticle.vsh
%COMP% /T vs_2_0 /E Main /nologo /D_BUMPMAP_ /D_PARTICLE_TYPE_DEFAULT_=1 /I %IDIR% /Fo %ODIR%/vsParticleDefaultBumpmap.pcv %SDIR%\vsParticle.vsh
%COMP% /T vs_2_0 /E Main /nologo /D_NDOTL_ /D_PARTICLE_TYPE_DEFAULT_=1 /I %IDIR% /Fo %ODIR%/vsParticleNdotL.pcv %SDIR%\vsParticle.vsh


%COMP% /T ps_2_0 /E Main /nologo /D_TEX0_ /D_DIFFUSE_ /D_BUMPMAP_ /I %IDIR% /Fo %ODIR%/psParticleDiffuseBumpTex.pcp %SDIR%\psParticle.psh
%COMP% /T ps_2_0 /E Main /nologo /D_TEX0_ /D_DIFFUSE_ /D_ADD_ /I %IDIR% /Fo %ODIR%/psParticleDiffuseTex.pcp %SDIR%\psParticle.psh
%COMP% /T ps_2_0 /E Main /nologo /D_DIFFUSE_ /D_ADD_ /I %IDIR% /Fo %ODIR%/psParticleDiffuse.pcp %SDIR%\psParticle.psh
%COMP% /T ps_2_0 /E Main /nologo /D_ALPHA_ /I %IDIR% /Fo %ODIR%/psParticleAlpha.pcp %SDIR%\psParticle.psh

%COPY% %ODIR%\vsParticleLine.pcv %CDIR%
%COPY% %ODIR%\vsParticleSprite.pcv %CDIR%
%COPY% %ODIR%\vsParticleSpriteRotating.pcv %CDIR%
%COPY% %ODIR%\vsParticleTriangle.pcv %CDIR%
%COPY% %ODIR%\vsParticlePointSprite.pcv %CDIR%
%COPY% %ODIR%\vsParticleDefault.pcv %CDIR%
%COPY% %ODIR%\vsParticleNdotL.pcv %CDIR%
%COPY% %ODIR%\vsParticleDefaultBumpmap.pcv %CDIR%

%COPY% %ODIR%\psParticleDiffuseBumpTex.pcp %CDIR%
%COPY% %ODIR%\psParticleDiffuseTex.pcp %CDIR%
%COPY% %ODIR%\psParticleDiffuse.pcp %CDIR%
%COPY% %ODIR%\psParticleAlpha.pcp %CDIR%


pause
