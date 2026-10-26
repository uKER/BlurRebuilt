#include "stdafx.h"

#include "ScoringRules.h"
#include <GameApp.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Components\Messages\VehicleLogicMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <Game/Messaging/PickupActorMessages.h>
#include <GameLogic\Structures\MpResults.h>

#include <GameLogic\Layers\RaceBookLayer.h>
#include <GameLogic\Layers\RaceLogicLayer.h>

#include <GameLogic\Structures\MpResults.h>

float MultiplayerScoringRuleHitByPerk::ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex)
{
	if(pMessage->Type() != E_VehicleLogicMsg_HitByPerk)
		return 0;

	const C_VehicleLogicMsg_HitByPerkMsg* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_HitByPerkMsg>(pMessage);

	if(pMsg->mFiringVehicleIndex == vehicleIndex && pMsg->mPickupType == mPickupType)
	{
		return mNumPointsScored;
	}

	return 0;
}

float MultiplayerScoringRuleImpact::ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex)
{
	if(pMessage->Type() != Game::E_GameLogicMessage_GameplayImpact)
		return 0;

	const Game::C_GameplayImpactMsg* pMsg = Messaging::MessageCast<Game::C_GameplayImpactMsg>(pMessage);

	if( (pMsg->mAggressiveVehicleIdx == vehicleIndex || pMsg->mAggressiveVehicleIdx == INVALID_VEHICLE_IDX) && pMsg->mImpactLevel == mImpactLevel && pMsg->mOtherVehicleIdx != INVALID_VEHICLE_IDX )
	{
		return mNumPointsScored;
	}

	return 0;
}


float MultiplayerScoringScoringEvent::ProcessMessage(const Messaging::C_MessageBase* pMessage, int vehicleIndex)
{
	if(pMessage->Type() != E_VehicleLogicMsg_Scored)
		return 0;

	const C_VehicleLogicMsg_Scored* pMsg = Messaging::MessageCast<C_VehicleLogicMsg_Scored>(pMessage);

	if(pMsg->mVehicleIndex == vehicleIndex && pMsg->mScore == mScoringEvent)
	{
		return mNumPointsScored;
	}

	return 0;
}
float MultiplayerScoringRulePickedUpPickup::ProcessMessage(const Messaging::C_MessageBase *pMessage, int vehicleIndex)
{
	if(pMessage->Type() != Game::E_PickupMessage_PickupPickedUp)
		return 0;

	const Game::Actors::PickupPickedUpMsg* pMsg = Messaging::MessageCast<Game::Actors::PickupPickedUpMsg>(pMessage);

	if( pMsg->mVehicleIndex == vehicleIndex && pMsg->mePickupType == mPickupType )
	{
		return mNumPointsScored;
	}

	return 0;
}

float MultiplayerScoringRuleCrossedLineFirst::ProcessMessage(const Messaging::C_MessageBase *pMessage, int vehicleIndex)
{
	if(pMessage->Type() != Game::E_GameflowMessage_VehicleFinished)
		return 0;

	if( Game::C_MpResults::Instance().GetCrossedFinishLinePosition( vehicleIndex ) == 1 )
	{
		return mNumPointsScored;
	}

	return 0;
}
