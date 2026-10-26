#include "stdafx.h"
#include <Game\GameActors\Components\VehicleLogic\VLC_Network_Abilities.h>

#include <GameLogic\Structures\LevelState.h>
#include <GameApp.h>
#include <Game/game.h>
#include <GameLogic\Structures\VehicleScoringInfo.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/GameData/MultiplayerData.h>
#include <Game/Messaging/PickupActorMessages.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

namespace Components
{
	bool C_VehicleComponent_Network_Abilities::CreateImpl()
	{
		const I_VehicleComponent_Network_Abilities_CreateInfo* pCreationData = static_cast<const I_VehicleComponent_Network_Abilities_CreateInfo *>(GetCreator().GetData());
		BIZ_ASSERT_MSG(pCreationData, "C_VehicleComponent_Network_Abilities::CreateImpl() > Couldn't get a pointer to the creation data of VLC_Network_Abilities");

		mActorHandle = pCreationData->mParentActorHandle;
		mVehicleIndex = pCreationData->mVehicleIndex;
		m_pLevelState = pCreationData->m_pLevelState;
		m_pRLC = pCreationData->m_pRLC;
		m_pConfig = pCreationData->m_pConfig;

		m_pSilentRunningAbility = pCreationData->m_pSilentRunningAbility;
		m_pXtAmaxVehicle = pCreationData->m_pXtAmaxVehicle;

		m_pVehicleLogicActor = pCreationData->m_pVehicleLogicActor;
		m_pVehicleGraphicalActor = pCreationData->m_pVehicleGraphicalActor;

		//Cache pointers to abilities
		m_pMultiplayerAbilities = m_pLevelState->mCars[mVehicleIndex].mUpgrades.GetMultiplayerAbilities();
		m_pSilentRunningVehicleAbility = m_pMultiplayerAbilities->GetAbility( UI::E_MultiplayerVehicleAbilities_Silent_Running );

		AddSubscriber( Game::E_GameflowMessage_RaceStarted );

		Initialise();
		
		return true;
	}

	const bool C_VehicleComponent_Network_Abilities::ShouldBeSilentlyRunning() const
	{
		return m_pMultiplayerAbilities->GetAbilityFiring(UI::E_MultiplayerVehicleAbilities_Silent_Running) && GetSilentRunningSFXEnabled();
	}

	void C_VehicleComponent_Network_Abilities::SetDesiredVehicleAlphas()
	{
		//For network vehicles we want the car, flares, light trails etc to entirely disappear
		m_pVehicleGraphicalActor->SetAlpha(E_VAC_TYPE_ALL, 0.f, E_VAC_TARGET_TYPE_DESIRED);

		//We still want to see particles kicked up though
		m_pVehicleGraphicalActor->SetAlpha(E_VAC_TYPE_PARTICLES, 1.f, E_VAC_TARGET_TYPE_DESIRED);
	}


	void C_VehicleComponent_Network_Abilities::Update()
	{
		PERF_EVENT("VehicleComponent.Network_Abilities");
		
#ifndef CFG_GOLD
		UpdateDebugFunctionality();
#endif

		const float elapsedTime = C_GameApp::GetInstance()->GetTimeStep();

		if( m_pSilentRunningVehicleAbility->GetActive() || g_SilentRunningForceAbilityActive )
		{
			UpdateSilentRunning(false);
		}

		m_pMultiplayerAbilities->Update(elapsedTime, mVehicleIndex);
	}
} // namespace Components
