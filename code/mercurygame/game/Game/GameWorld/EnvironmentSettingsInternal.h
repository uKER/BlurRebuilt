#ifndef __GAMEWORLD_ENVIRONMENTSETTINGSINTERNAL_H__
#define __GAMEWORLD_ENVIRONMENTSETTINGSINTERNAL_H__

namespace Game
{

struct EnvironmentSettings
{
	::EnvironmentSettings::LightingInfo		lightingSettings;
	::EnvironmentSettings::FogSettings		fogSettings;
	XtSz8					cubemapTexture;
	Effect::Sky::SkyDome	skySettings;
	PostProcess::Settings	postProcessing;
};

void EnvironmentSettingsChangedCallback(XtEventType event, XtRecord* record, EnvironmentSettings* object, XtCallbackContext*);

} // namespace Game

#endif // GAME_WORLD_H
