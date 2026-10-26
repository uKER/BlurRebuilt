#include "MultiplayerAbility.h"
#include <GameLogic/Structures/LevelState.h>

namespace Game
{
	void C_MultiplayerAbility::ApplyAbility(Game::LevelStateCar& levelCar) const
	{
		// check for whether we've already activated this
		for ( S32 i = 0; i < levelCar.mLoadOuts.Size(); i++ )
			if ( levelCar.mLoadOuts[ i ] == GetData()->mAbilityId )
				return;

		// For every ability applied, set it to active and set its ability 'value' (it may or may not actually use this)
		// So .. do this for the right type
		Game::C_MultiplayerVehicleAbility* ability = levelCar.mUpgrades.GetMultiplayerAbilities()->GetAbility(GetData()->mAbilityId);
		ability->SetActive(true);
		ability->SetValue(GetData()->mAbilityValue);
		levelCar.mLoadOuts.PushBack(GetData()->mAbilityId);
	}

	C_MultiplayerAbility::C_MultiplayerAbility( XtHandle<MultiplayerAbilityData> hAbility )
		: m_hHandle( hAbility )
	{
		BIZ_ASSERT( hAbility );
	}

	const MultiplayerAbilityData* C_MultiplayerAbility::GetData() const
	{
		BIZ_ASSERT_MSG( m_hHandle, "Ability has not been created properly" );

		return m_hHandle.Object();
	}

	XtArray<RLPickupSpecialFX>& C_MultiplayerAbility::GetSpecialFX()
	{
		BIZ_ASSERT_MSG( m_hHandle, "Ability has not been created properly" );

		return m_hHandle.Object()->mAbilitySpecialFX;
	}

	const XtArray<RLPickupSpecialFX>& C_MultiplayerAbility::GetSpecialFX() const
	{
		BIZ_ASSERT_MSG( m_hHandle, "Ability has not been created properly" );

		return m_hHandle.Object()->mAbilitySpecialFX;
	}

} //Namespace Game
