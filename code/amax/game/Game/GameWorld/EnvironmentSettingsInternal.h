#ifndef __GAMEWORLD_ENVIRONMENTSETTINGSINTERNAL_H__
#define __GAMEWORLD_ENVIRONMENTSETTINGSINTERNAL_H__

namespace Game
{

struct LightingInfo
{
	XtArray<float> bin;
};

struct FogSettings
{
	XtArray<float> bin;
};

struct LightingSettings
{
	::EnvironmentSettings::LightingInfo		lightingSettings;
	::EnvironmentSettings::FogSettings		fogSettings;
	XtSz8									cubemapTexture;
	Effect::Sky::SkyDome	skySettings;
	PostProcess::Settings	hdrSettings;
};

template<typename T>
void EnvironmentSettingsChangedCallback(XtEventType event, XtRecord* record, T* object, XtCallbackContext*);

} // namespace Game

#endif // GAME_WORLD_H
