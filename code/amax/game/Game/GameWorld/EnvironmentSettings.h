#ifndef __GAMEWORLD_ENVIRONMENTSETTINGS_H__
#define __GAMEWORLD_ENVIRONMENTSETTINGS_H__

namespace Effect
{
namespace Sky
{
class C_TexturedSkyDome;
}
}

namespace FX
{
class I_EnvironmentSettings;
};

namespace Game
{

struct LightingSettings;
struct LightFlareSettings;
struct PostFxSettings;
struct RoadTrackSettings;
struct ShadowSettings;
struct CarLightSettings;
class C_Wind;
struct WindSettings;

struct EnvironmentSettings
{
	XtHandle<LightingSettings>		lightingSettings;
	XtHandle<LightFlareSettings>	lightFlareSettings;
	XtHandle<PostFxSettings>		postFxSettings;
	XtHandle<RoadTrackSettings>		roadTrackSettings;
	XtHandle<ShadowSettings>		shadowSettings;
	XtHandle<CarLightSettings>		carLightSettings;
	XtHandle<WindSettings>			windSettings;
};

/// pass in a Sorcery level settings record and return an environment settings parameter block
FX::I_EnvironmentSettings *CreateEnvironmentSettings(EnvironmentSettings *p, Effect::Sky::C_TexturedSkyDome *pSky, C_Wind* pWind);
void	InitialiseEnvironmentSettingsParameters();

extern bool g_bEnvironmentSettingsChangedInSorcery;
} // namespace Game

#endif // GAME_WORLD_H
