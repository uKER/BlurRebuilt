#include "stdafx.h"
#include <Game\GameActors\Components\VehicleLogic\Multiplayer\VLC_MP_Pickups_AI.h>
#include "System/misc/Random.h"

#include <misc_shared/Messaging/GameMessaging.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <Game/GameActors/Components/Messages/VehicleLogicMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>
#include <GameLogic/Messaging/GameLogicMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>

#include <Game/Game.h>

#include <Game/GameActors/Vehicles/VehicleActor.h>
#include <Game/GameActors/VehicleLogic/VehicleLogicActor.h>

#include <GameLogic/RaceLogicCore/Data/RLVehicleData.h>
#include <GameLogic/RaceLogicCore/Core/RaceLogicCore.h>
#include <GameLogic/RaceLogicCore/Database/RLDatabase.h>
#include <GameLogic/Structures/LevelState.h>

#include <AI\AI_Amax.h>

extern Debug::Bool gDebugEnableAIPerks;
extern Debug::Logger &g_PickupAILogger;


namespace Components
{


bool C_VehicleComponent_MP_Pickups_AI::CreateImpl()
{
	const I_VehicleComponent_MP_Pickup_AI_CreateInfo* creation_data = static_cast<const I_VehicleComponent_MP_Pickup_AI_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{
		mActorHandle=creation_data->mParentActorHandle;
		mVehicleIndex=creation_data->mVehicleIndex;
		mpRaceLogicLayer=creation_data->mpRaceLogicLayer;
	}

	Initialise();

	return true;
}

bool C_VehicleComponent_MP_Pickups_AI::InRangeOfHumanCheck()
{
	//	Tell the logic we're in range a human - who cares if we are or not?!
	return true;
}

bool C_VehicleComponent_MP_Pickups_AI::PlayerHitRecentlyCheck()
{
	//	Nope, the "player" hasn't been hit recently. This could be a LIE, but we don't care.
	return false;
}


bool C_VehicleComponent_MP_Pickups_AI::IsAiControlled()
{
	//	Returns true if this is strictly an AI vehicle (Level State dictates it as such) OR
	//	this is a player vehicle that we've toggled to act like an AI
	return(((Game::Actors::C_VehicleLogicActor*)GetActor())->GetRLC()->IsVehicleAIControlled(mVehicleIndex));
}


void C_VehicleComponent_MP_Pickups_AI::Update()
{
	return;

#if 0
	if(!mEnabled || !IsAiControlled())
		return;

	PERF_EVENT("RLPickupController::EvaluateAIPerkUse");

	if( gDebugEnableAIPerks && mAccuracyCurves.Size() )
	{
		airacingtech::AIEventSegmentData* balancingData = AIAmax::AmaxAIR::Instance().GetEventBalancingSegmentData(mVehicleIndex);

		if(balancingData)
			mCurrCurveIdx = balancingData->m_PerkTemplateIndex;

		if(mCurrCurveIdx < 0 || mCurrCurveIdx >= mAccuracyCurves.Size() )
		{
			mCurrCurveIdx = 0;
			LOGWARN(g_PickupAILogger, "Failed to handle balancing data index - resetting to zero.");
		}

		DecrementTimers();
		CyclePickups();
		EvaluateShieldUse();
		EvaluatePickupUse();
		SetTimers();
	}
#endif

}


bool C_VehicleComponent_MP_Pickups_AI::CheckLevel2PickupTriggers(E_PICKUP_TYPE pickupType)
{
	return false;
}


bool C_VehicleComponent_MP_Pickups_AI::EvaluatePickupChange( E_PICKUP_TYPE pickupType )
{
	if(mEnabled && IsAiControlled())
		return (C_VehicleComponent_Pickups_AI::EvaluatePickupChange(pickupType));
	else
		return false;
}


bool C_VehicleComponent_MP_Pickups_AI::OnMessage( const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case Game::E_GameflowMessage_RaceStarted:
		{
			mEnabled = true;
			return true;
		}
		break;

	case Game::E_GameflowMessage_RaceFinished:
		{
			mEnabled = false;
			return true;
		}
		break;

	//case GameMessaging::E_GameMsg_Update:
	//	{
	//		if(mEnabled && IsAiControlled())
	//			Update();
	//		return true;
	//	}
	//	break;

	case Game::E_GameLogicMessage_Failed_Rule:
		{
			const Game::C_FailedRuleMsg* pMsg = Messaging::MessageCast<Game::C_FailedRuleMsg>(pMessage);

			if( pMsg->mVehicleIndex == mVehicleIndex )
				mEnabled = false;

			return true;
		}
		break;


	case E_VehicleLogicMsg_FiredPerk:
		{
			if(mEnabled && IsAiControlled())
			{
				const C_VehicleLogicMsg_FiredPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_FiredPerkMsg >(pMessage);

				if( msg->mActorHandle == mActorHandle )
					mTimer_PerkAIDisable += Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedFiredPerkDisableTime;
			}
			return true;
		}
		break;

	case E_VehicleLogicMsg_HitByPerk:
		{
			if(mEnabled && IsAiControlled())
			{
				const C_VehicleLogicMsg_HitByPerkMsg* msg = Messaging::MessageCast< C_VehicleLogicMsg_HitByPerkMsg >(pMessage);

				if( msg->mVehicleIndex == mVehicleIndex )
					mTimer_HitWithPerkDisableTime += Game::C_Game::GetInstance()->GetLevelState().mAIController->mSharedBeenHitWithPerkDisableTime;
			}
			return true;
		}
		break;

	case ActorMsg::E_DynamicObjMsg_SetVehicleLogic:
		{
			const ActorMsg::C_SetVehicleLogicMsg* pMsg = Messaging::MessageCast<ActorMsg::C_SetVehicleLogicMsg>(pMessage);
			mEnabled = pMsg->m_Pickups;			
			return true;
		}
	}

	return C_VehicleComponent_Pickups_AI::OnMessage(pMessage);
}

}	//	namespace Components
