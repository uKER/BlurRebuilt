#ifndef MULTIPLAYER_ABILITY_H
#define MULTIPLAYER_ABILITY_H

#include <GameLogic/GameData/MultiplayerData.h>
#include <GameLogic\Structures\LevelState.h>

namespace Game
{

class C_MultiplayerAbility
{
public:
	C_MultiplayerAbility()
	{
		m_hHandle.Clear();
	}

	C_MultiplayerAbility( XtHandle<MultiplayerAbilityData> hAbility);

	void ApplyAbility(Game::LevelStateCar& levelCar) const;
	const MultiplayerAbilityData* GetData() const;
	XtArray<RLPickupSpecialFX>& GetSpecialFX();
	const XtArray<RLPickupSpecialFX>& GetSpecialFX() const;

private:
	XtHandle<MultiplayerAbilityData> m_hHandle;
};

}

#endif
