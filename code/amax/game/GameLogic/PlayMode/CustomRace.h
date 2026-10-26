#ifndef GAME_CUSTOM_RACE_H
#define GAME_CUSTOM_RACE_H

#include "GameLogic/PlayMode/PlayModeBase.h"
#include "Ui/game/Multiplayer/MultiplayerDefines.h"
#include "Ui/game/UIXt.h"

// Forward declare
struct lua_State;

namespace Game
{
	class C_LevelState;
}
namespace RacerNet
{
	struct NetRaceSettings;
}

namespace Game
{

class C_CustomRace : public I_PlayModeBase
{
public:
	C_CustomRace();

	void 				Initialise();

						/// Fills out NetRaceSettings with the playlist data
	void 				SetupNetRaceSettings( RacerNet::NetRaceSettings& settings );

						/// Fills out the level state with the playlist data
	void 				SetupLevelState( C_LevelState& levelState );

						/// Fills out the race data from net race settings
	void				SetupRace( RacerNet::NetRaceSettings& settings );

						/// Fills out the race data from lua
	void				SetupRace( lua_State* pState );

	void				SendToLua( lua_State* pState ) const;

	UI::E_RaceType		GetRaceMode() const { return (UI::E_RaceType)m_RaceMode; }

	U32					GetCity() const { return m_CityID; }

	U32					GetRoute() const { return m_RouteID; }

	S8					GetLapLimit() const { return m_LapLimit; }

	S32					GetTimeLimit() const { return m_TimeLimit; }
	
	UI::E_VehicleClass	GetCarClass( const S16 sRaceIndex = -1 ) const { return (UI::E_VehicleClass)m_CarClass; }

	S8					GetDamageMode() const { return m_DamageMode; }

	S8					GetMods() const { return m_Mods; }

	S8					GetAI() const { return m_AI; }

	S8					GetAIMaxPadding() const { m_AIMaxPadding; }

	S8					GetRespawn() const { return m_Respawn; }

	S8					GetUpgrades() const { return m_Upgrades; }

	S8					GetHandicap() const { return m_Handicap; }

	S8					GetTimeout() const { return m_Timeout; }

	S8					GetLockToHostCar() const { return m_LockToHostCar; }

	U16					GetPowerups() const { return m_Powerups; }

	S8					GetPowerupRespawnMode() const { return m_PowerupRespawnMode; }

	S8					GetRandomPowerups() const { return m_RandomPowerups; }

private:
	U32					m_CityID;
	U32					m_RouteID;
	S8					m_RaceMode;
	S8					m_LapLimit;
	S32					m_TimeLimit;
	U8					m_MaxPlayers;
	S8					m_CarClass;
	S8					m_DamageMode;
	S8					m_Respawn;		
	S8					m_Timeout;
	S8					m_Mods;			
	S8					m_Upgrades;
	S8					m_LockToHostCar;
	S8					m_AI;
	S8					m_AIMaxPadding;
	S8					m_Handicap;
	U16					m_Powerups;
	S8					m_PowerupRespawnMode;
	S8					m_RandomPowerups;

	UI::UiXt::C_Allocator m_Allocator;
};

};// namespace Game

#endif // GAME_CUSTOM_RACE_H