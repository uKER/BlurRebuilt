#include "stdafx.h"

#include <btl/containers/List.h>
#include <btl/containers/BitVector.h>
#include <xt/xtgame.h>
#include <FX/FX.h>
#include <Render/State/RenderState.h>

#include <Fx/EnvironmentSettings.h>
#include <Fx/EnvironmentString.h>
#include <effect\sky\SkyDomeEnvSettings.h>
#include <Render/PostProcess/PostProcessEnvSettings.h>
#include <Game/GameDef/GameData.h>
#include <shader/EnvSettings/LightingSettings.h>
#include <shader/EnvSettings/FogSettings.h>
#include <shader/EnvSettings/CubemapSettings.h>
#include "Game/GameWorld/EnvironmentSettings.h"
#include "Game/GameWorld/EnvironmentSettingsInternal.h"

XT_TYPE(Game::EnvironmentSettings,"EnvironmentSettings")
	XT_CALLBACK(Game::EnvironmentSettingsChangedCallback)
	XT_FIELD(lightingSettings, "lightingSettings")
	XT_FIELD(fogSettings, "fogSettings")
	XT_FIELD(cubemapTexture, "cubemapTexture")
	XT_FIELD(skySettings, "skySettings")
	XT_FIELD(postProcessing, "postProcessing")
XT_END_TYPE

XT_LIBRARY(EnvironmentSettings)
	XT_REGISTER_TYPE(Game::EnvironmentSettings)
	XT_REGISTER_TYPE(Effect::Sky::SkyDome)
	XT_REGISTER_TYPE(PostProcess::Settings)
	XT_REGISTER_TYPE(EnvironmentSettings::FogSettings)
	XT_REGISTER_TYPE(EnvironmentSettings::HeightFogSettings)
	XT_REGISTER_TYPE(EnvironmentSettings::DepthFogSettings)
	XT_REGISTER_TYPE(EnvironmentSettings::LightingInfo)
XT_END_LIBRARY

namespace Game
{

bool g_bEnvironmentSettingsChangedInSorcery = false;

void EnvironmentSettingsChangedCallback(XtEventType event, XtRecord* record, EnvironmentSettings* object, XtCallbackContext*)
{
	g_bEnvironmentSettingsChangedInSorcery = true;
}

/// pass in a Sorcery level settings record and return an environment settings parameter block
FX::I_EnvironmentSettings *CreateEnvironmentSettings(EnvironmentSettings *pEnv, Effect::Sky::C_TexturedSkyDome *pSky)
{
	FX::EnvironmentSettings *p = new FX::EnvironmentSettings;

	p->AddBlock(new ::EnvironmentSettings::LightingEnvSettings(&pEnv->lightingSettings));
	p->AddBlock(new ::EnvironmentSettings::FogEnvSettings(&pEnv->fogSettings));
	p->AddBlock(new ::EnvironmentSettings::CubemapEnvSettings(pEnv->cubemapTexture));
	p->AddBlock(new Effect::Sky::SkyDomeEnvSettings(&pEnv->skySettings, pSky));
	p->AddBlock(new PostProcess::PostProcessEnvSettings(&pEnv->postProcessing));

	p->AssociateDebugVariables("Graphics.EnvironmentSettings");

	return p;
}

void InitialiseEnvironmentSettingsParams()
{
	::EnvironmentSettings::LightingEnvSettings::InitialiseParameters();
	::EnvironmentSettings::FogEnvSettings::InitialiseParameters();
	::EnvironmentSettings::CubemapEnvSettings::InitialiseParameters();
}	

} // namespace Game
