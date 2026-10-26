#include "stdafx.h"

#include <Btl/Containers/List.h>
#include <Btl/Containers/BitVector.h>
#include <Xt/Xtgame.h>
#include <FX/FX.h>
#include <Render/State/RenderState.h>

#include <Fx/EnvironmentSettings.h>
#include <Effect/Sky/SkyDomeEnvSettings.h>
#include <Render/PostProcess/PostProcessEnvSettings.h>
#include <Graphics/PostFxMatrix.h>
#include <Graphics/PostFxSettings.h>
#include <Shader/EnvSettings/LightingSettings.h>
#include <Shader/EnvSettings/FogSettings.h>
#include <Shader/EnvSettings/CubemapSettings.h>
#include "Game/GameWorld/EnvironmentSettings.h"
#include "Game/GameWorld/EnvironmentSettingsInternal.h"
#include <Graphics/LightFlareSettings.h>
#include <Graphics/RoadTrackSettings.h>
#include <Graphics/ShadowSettings.h>
#include <Graphics/CarLightSettings.h>
#include <Graphics/Wind.h>

XT_TYPE(Game::EnvironmentSettings,"EnvironmentSettings")
	XT_FIELD(lightingSettings, "lightingSettings")
	XT_FIELD(lightFlareSettings, "lightFlareSettings")
	XT_FIELD(postFxSettings, "postFxSettings")
	XT_FIELD(roadTrackSettings, "roadTrackSettings")
	XT_FIELD(shadowSettings, "shadowSettings")
	XT_FIELD(carLightSettings, "carLightSettings")
	XT_FIELD(windSettings, "windSettings")
XT_END_TYPE

XT_TYPE(Game::LightingSettings, "LightingSettings")
	XT_FIELD(lightingSettings, "lightingSettings")
	XT_FIELD(fogSettings, "fogSettings")
	XT_FIELD(cubemapTexture, "cubemapTexture")
	XT_FIELD(skySettings, "skySettings")
	XT_FIELD(hdrSettings, "hdrSettings")
XT_END_TYPE

XT_LIBRARY(EnvironmentSettings)
	XT_REGISTER_TYPE(Game::EnvironmentSettings)
	XT_REGISTER_TYPE(::EnvironmentSettings::LightingInfo)
	XT_REGISTER_TYPE(::EnvironmentSettings::FogSettings)
	XT_REGISTER_TYPE(::EnvironmentSettings::HeightFogSettings)
	XT_REGISTER_TYPE(::EnvironmentSettings::DepthFogSettings)
	XT_REGISTER_TYPE(Effect::Sky::SkyDome)
	XT_REGISTER_TYPE(Render::HermiteKeyFrame)
	XT_REGISTER_TYPE(Render::Curve)
	XT_REGISTER_TYPE(PostProcess::Settings)
	XT_REGISTER_TYPE(Game::BlurType)
	XT_REGISTER_TYPE(Game::PostFxLensFlare)
	XT_REGISTER_TYPE(Game::PostFxLensFlares)
	XT_REGISTER_TYPE(Game::PostFxSettings)
	XT_REGISTER_TYPE(Game::PostFxMatrix)
	XT_REGISTER_TYPE(Game::PostFxMatrixRoot)
	XT_REGISTER_TYPE(Game::PostFxMatrixBrightness)
	XT_REGISTER_TYPE(Game::PostFxMatrixContrast)
	XT_REGISTER_TYPE(Game::PostFxMatrixHue)
	XT_REGISTER_TYPE(Game::PostFxMatrixSaturation)
	XT_REGISTER_TYPE(Game::PostFxMatrixLightness)
	XT_REGISTER_TYPE(Game::PostFxMatrixAdd)
	XT_REGISTER_TYPE(Game::PostFxMatrixMultiply)
	XT_REGISTER_TYPE(Game::PostFxMatrixBasis)
	XT_REGISTER_TYPE(Game::PostFxMatrixSepia)
	XT_REGISTER_TYPE(Game::PostFxMatrixRotate)
	XT_REGISTER_TYPE(Game::PostFxMatrixHint)
	XT_REGISTER_TYPE(Game::PostFxMatrixCustom)
	XT_REGISTER_TYPE(Game::LightingSettings);
	XT_REGISTER_TYPE(Game::LightFlareSettings);
	XT_REGISTER_TYPE(Game::RoadTrackSettings)
	XT_REGISTER_TYPE(Game::ShadowSettings)
	XT_REGISTER_TYPE(Game::CarLightSettings)
	XT_REGISTER_TYPE(Game::WindSettings)
XT_END_LIBRARY

namespace Game
{

bool g_bEnvironmentSettingsChangedInSorcery = false;

/// pass in a Sorcery level settings record and return an environment settings parameter block
FX::I_EnvironmentSettings *CreateEnvironmentSettings(EnvironmentSettings *pEnv, Effect::Sky::C_TexturedSkyDome *pSky, C_Wind* pWind)
{
	FX::EnvironmentSettings *p = new FX::EnvironmentSettings;

	if (pEnv->lightingSettings)
	{
		p->AddBlock(new ::EnvironmentSettings::LightingEnvSettings(&pEnv->lightingSettings->lightingSettings));
		p->AddBlock(new ::EnvironmentSettings::FogEnvSettings(&pEnv->lightingSettings->fogSettings));
		p->AddBlock(new ::EnvironmentSettings::CubemapEnvSettings(pEnv->lightingSettings->cubemapTexture));
		p->AddBlock(new Effect::Sky::SkyDomeEnvSettings(&pEnv->lightingSettings->skySettings, pSky));
		p->AddBlock(new PostProcess::PostProcessEnvSettings(&pEnv->lightingSettings->hdrSettings));
	}

	if (Game::LightFlareSettings* pLightFlareSettings = &*pEnv->lightFlareSettings)
		p->AddBlock(new LightFlareEnvSettings(pLightFlareSettings));

	if (Game::PostFxSettings* pPostFxSettings = &*pEnv->postFxSettings)
		p->AddBlock(new PostFxEnvSettings(pPostFxSettings));

	if (Game::RoadTrackSettings* pRoadTrackSettings = &*pEnv->roadTrackSettings)
		p->AddBlock(new RoadTrackEnvSettings(pRoadTrackSettings));

	if (Game::ShadowSettings* pShadowSettings = &*pEnv->shadowSettings)
		p->AddBlock(new ShadowEnvSettings(pShadowSettings));
	
	if (Game::CarLightSettings* pCarLightSettings = &*pEnv->carLightSettings)
		p->AddBlock(new CarLightEnvSettings(pCarLightSettings));

	if (Game::WindSettings* pWindSettings = &*pEnv->windSettings)
		p->AddBlock(new WindEnvSettings(pWindSettings, pWind));

	p->AssociateDebugVariables("Graphics.EnvironmentSettings");

	return p;
}

void InitialiseEnvironmentSettingsParameters()
{
	::EnvironmentSettings::LightingEnvSettings::InitialiseParameters();
	::EnvironmentSettings::FogEnvSettings::InitialiseParameters();
	::EnvironmentSettings::CubemapEnvSettings::InitialiseParameters();
	RoadTrackEnvSettings::InitialiseParameters();
}

} // namespace Game
