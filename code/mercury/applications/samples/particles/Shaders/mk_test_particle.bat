
set COMP="%XEDK%\bin\win32\fxc"
set COPY="%XEDK%\bin\win32\xbcp" /f /y 
set IDIR=%SRC_ROOT%\code\mercury\library\render\include\render\shaders
set SDIR=%SRC_ROOT%\code\mercury\library\render\include\render\shaders\particles
set ODIR=.\pc
set CDIR=xe:\particles_xe\Data\Shaders

cls

%COMP% /T vs_3_0 /E main /nologo /DPARTICLE_TYPE_LINE /I %IDIR% /Fo %ODIR%/vsParticleLine.xev %SDIR%\vsParticle.vsh
%COMP% /T vs_3_0 /E main /nologo /DPARTICLE_TYPE_SPRITE /I %IDIR% /Fo %ODIR%/vsParticleSprite.xev %SDIR%\vsParticle.vsh
%COMP% /T vs_3_0 /E main /nologo /DPARTICLE_TYPE_SRRITE_ROTATING /I %IDIR% /Fo %ODIR%/vsParticleSpriteRotating.xev %SDIR%\vsParticle.vsh
%COMP% /T vs_3_0 /E main /nologo /DPARTICLE_TYPE_TRIANGLE /I %IDIR% /Fo %ODIR%/vsParticleTriangle.xev %SDIR%\vsParticle.vsh
%COMP% /T vs_3_0 /E main /nologo /DPARTICLE_TYPE_POINT_SPRITE /I %IDIR% /Fo %ODIR%/vsParticlePointSprite.xev %SDIR%\vsParticle.vsh
%COMP% /T vs_3_0 /XZi /E main /nologo /DPARTICLE_TYPE_DEFAULT /I %IDIR% /Fo %ODIR%/vsParticleDefault.xev %SDIR%\vsParticle.vsh
%COMP% /T vs_3_0 /E main /nologo /DBUMPMAP /DPARTICLE_TYPE_DEFAULT /I %IDIR% /Fo %ODIR%/vsParticleDefaultBumpmap.xev %SDIR%\vsParticle.vsh
%COMP% /T vs_3_0 /E main /nologo /DNDOTL /DPARTICLE_TYPE_DEFAULT /I %IDIR% /Fo %ODIR%/vsParticleNdotL.xev %SDIR%\vsParticle.vsh

%COMP% /T ps_3_0 /E main /nologo /DPOINTSPRITE /DDIFFUSETEX /DADD /I %IDIR% /Fo %ODIR%/psParticleDiffuseTexPointSprite.xep %SDIR%\psParticle.psh
%COMP% /T ps_3_0 /E main /nologo /DDIFFUSETEX /DADD /I %IDIR% /Fo %ODIR%/psParticleDiffuseTex.xep %SDIR%\psParticle.psh
%COMP% /T ps_3_0 /E main /nologo /DDIFFUSETEX /DBUMPMAP /I %IDIR% /Fo %ODIR%/psParticleDiffuseBumpTex.xep %SDIR%\psParticle.psh
%COMP% /T ps_3_0 /E main /nologo /DDIFFUSE /DADD /I %IDIR% /Fo %ODIR%/psParticleDiffuse.xep %SDIR%\psParticle.psh
%COMP% /T ps_3_0 /E main /nologo /DALPHA /I %IDIR% /Fo %ODIR%/psParticleAlpha.xep %SDIR%\psParticle.psh

%COPY% %ODIR%\vsParticleLine.xev %CDIR%
%COPY% %ODIR%\vsParticleSprite.xev %CDIR%
%COPY% %ODIR%\vsParticleSpriteRotating.xev %CDIR%
%COPY% %ODIR%\vsParticleTriangle.xev %CDIR%
%COPY% %ODIR%\vsParticlePointSprite.xev %CDIR%
%COPY% %ODIR%\vsParticleDefault.xev %CDIR%
%COPY% %ODIR%\vsParticleNdotL.xev %CDIR%
%COPY% %ODIR%\vsParticleDefaultBumpmap.xev %CDIR%

%COPY% %ODIR%\psParticleDiffuseTexPointSprite.xep %CDIR%
%COPY% %ODIR%\psParticleDiffuse.xep %CDIR%
%COPY% %ODIR%\psParticleDiffuseTex.xep %CDIR%
%COPY% %ODIR%\psParticleDiffuseBumpTex.xep %CDIR%
%COPY% %ODIR%\psParticleAlpha.xep %CDIR%


pause
