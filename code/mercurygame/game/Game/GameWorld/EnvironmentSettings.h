#ifndef __GAMEWORLD_ENVIRONMENTSETTINGS_H__
#define __GAMEWORLD_ENVIRONMENTSETTINGS_H__

namespace Effect
{
namespace Sky
{
class C_TexturedSkyDome;
}
}

namespace Game
{

struct EnvironmentSettings;

/// pass in a Sorcery level settings record and return an environment settings parameter block
FX::I_EnvironmentSettings *CreateEnvironmentSettings(EnvironmentSettings *p, Effect::Sky::C_TexturedSkyDome *pSky);
void						InitialiseEnvironmentSettingsParams();

extern bool g_bEnvironmentSettingsChangedInSorcery;

} // namespace Game

#endif // GAME_WORLD_H
