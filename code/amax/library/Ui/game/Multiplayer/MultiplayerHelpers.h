
#ifndef MULTIPLAYER_HELPERS_H
#define MULTIPLAYER_HELPERS_H

#include "Ui/Game/Multiplayer/MultiplayerDefines.h"
#include <GameLogic\RaceLogicCore\Data\RLEnums.h>
#include <GameLogic\RaceModes\RaceModeDefines.h>

// Forward declarations
struct lua_State;

namespace UI
{

//	UI-side stuff - WillB
void CreateMpLuaEnums( lua_State* pState );

//	Game-side stuff - GavinY
Game::E_RaceMode				ConvertUiToGame_RaceMode(E_RaceType p_Event);
E_PICKUP_TYPE					ConvertUiToGame_Pickup(E_Pickup p_Pickup);
E_DAMAGE_MODE					ConvertUiToGame_DamageMode(E_DamageMode p_DamageMode);
F32								ConvertUiToGame_DamageMultiplier( E_DamageMode eDamageMode );
const E_PICKUP_RESPAWN_TYPE		ConvertUiToGame_PowerUpRespawnMode(const E_PowerupRespawnMode ePowerupRespawnMode);

}; // namespace UI

#endif // MULTIPLAYER_HELPERS_H
