#include "stdafx.h"

#include "VLC_PointsSystem.h"
#include "VLC_Statistics.h"
#include <GameLogic/Structures/ScoringRules.h>

#include <Game/Messaging/GameflowMessages.h>
#include <misc_shared\Messaging\GameMessaging.h>
#include <GameLogic\Messaging\GameLogicMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>

namespace Components
{

	bool C_VehicleComponent_PointsSystem::CreateImpl()
	{
		const I_VehicleComponent_PointsSystem_CreateInfo* creation_data = static_cast<const I_VehicleComponent_PointsSystem_CreateInfo *>(GetCreator().GetData());

		if(creation_data)
		{
			mActorHandle=creation_data->mParentActorHandle;
			mVehicleIndex=creation_data->mVehicleIndex;
			mRaceLogicLayerPtr=creation_data->mpRaceLogicLayer;
		}	

		mEnabled = false;

		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_GameflowMessage_QuitLevel );
		AddSubscriber( Game::E_GameflowMessage_QuitRace );
		AddSubscriber( Game::E_GameflowMessage_RaceStarted );
		AddSubscriber( Game::E_VehicleMessage_FatallyDamaged );
		AddSubscriber( Game::E_GameflowMessage_VehicleFinished );
		AddSubscriber( Game::E_PickupMessage_PickupPickedUp );

		AddSubscriber( E_VehicleLogicMsg_HitByPerk );
		AddSubscriber( Game::E_GameLogicMessage_GameplayImpact );
		AddSubscriber( E_VehicleLogicMsg_Scored );

		mCurrentPoints = 0.0f;

		Game::LevelStateMultiplayerSettings& mpSettings = Game::C_Game::GetInstance()->GetLevelState().GetMultiplayerSettings();

		for( int i = 0; i < UI::E_ScoringType_Count; i++ )
		{
			BIZ_ASSERT_MSG( i < C_MultiplayerData::Instance().m_pConfig->m_ScoringRules.count, "Attempting to add a scoring rule that hasn't been loaded from Sorcery");
			if( mpSettings.mScoringRules & (1 << i))
			{
				mScoringRules.PushBack(i);
			}
		}

		return true;
	}

	bool C_VehicleComponent_PointsSystem::OnMessage(const Messaging::C_MessageBase *pMessage)
	{
		switch(pMessage->Type())
		{
		case Game::E_GameflowMessage_RaceStarted:
			{
				mEnabled = true;
				return true;
			}
		
		case Game::E_GameflowMessage_RaceFinished:
		case Game::E_GameflowMessage_QuitRace:
		case Game::E_GameflowMessage_QuitLevel:
			{
				mEnabled = false;
				return true;
			}

		case E_VehicleLogicMsg_Scored:
		case Game::E_PickupMessage_PickupPickedUp:
		case Game::E_GameLogicMessage_GameplayImpact:
			{
				if(mEnabled)
				{
					for( int i = 0; i < mScoringRules.Size(); i++ )
					{
						int idx = mScoringRules[i];
						mCurrentPoints += C_MultiplayerData::Instance().m_pConfig->m_ScoringRules[idx]->ProcessMessage(pMessage, mVehicleIndex);
					}
				}

				return true;
			}
		case Game::E_GameflowMessage_VehicleFinished:
			{
				if(mEnabled)
				{
					for( int i = 0; i < mScoringRules.Size(); i++ )
					{
						int idx = mScoringRules[i];
						mCurrentPoints += C_MultiplayerData::Instance().m_pConfig->m_ScoringRules[idx]->ProcessMessage(pMessage, mVehicleIndex);
					}
				}

				// if the vehicle has finished, we want to handle it as above but we also want to disable further scoring
				mEnabled = false;
				return true;
			}
		}

		return false;
	}
}
