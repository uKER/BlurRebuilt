#include "stdafx.h"
#include "VLC_AIController.h"

#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/GameflowMessages.h>
#include <GameLogic/ActorStudioActors/DynamicObjectMessages.h>
#include <GameLogic\RaceLogicCore\Core\RaceLogicCore.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>
#include <Game\GameActors\Vehicles\VehicleActor.h>

Debug::Bool gDebugOutOfRangeGhosting("Vehicle.Components.AIController.OutOfRangeGhostingEnabled", true, Debug::Persist());
Debug::Bool gDebugForceHumanOutOfRange("Vehicle.Components.AIController.ForceHumanOutOfRange", false);

namespace Components
{

	bool C_VehicleComponent_AIController::CreateImpl()
	{
		const I_VehicleComponent_AIController_CreateInfo* creation_data = static_cast<const I_VehicleComponent_AIController_CreateInfo *>(GetCreator().GetData());

		if(creation_data)
		{
			mActorHandle=creation_data->mParentActorHandle;
			mVehicleIndex=creation_data->mVehicleIndex;
		}

		//AddSubscriber( GameMessaging::E_GameMsg_Update );
		AddSubscriber( ActorMsg::E_DynamicObjMsg_SetVehicleLogic );
		AddSubscriber( Game::E_GameflowMessage_RaceStarted );

		mInitialDisable = true;
		mEnabled = false;

		return true;
	}

	void C_VehicleComponent_AIController::Update()
	{
		PERF_EVENT("VehicleComponent.AIController");

		if(!mEnabled)
			return;

		Game::Actors::C_VehicleLogicActor* pActor = ((Game::Actors::C_VehicleLogicActor*)GetActor());
		Game::Actors::RaceLogicCore* pRLC = pActor->GetRLC();

		if(!gDebugOutOfRangeGhosting)
		{
			if(mInitialDisable)
			{
				if(pActor->IsGhosted())
					pActor->Deghost();

				mInitialDisable = false;
			}

			return;
		}
		else
			mInitialDisable = true;

		if( !pActor || !pActor->GetGraphicalActor(mVehicleIndex) )
			return;

		if(pActor->GetPositionInfo().mInRangeOfHuman && !gDebugForceHumanOutOfRange) 
		{
			if(pActor->IsSecretlyGhosted())
			{
				pActor->SecretlyDeGhost();
			}
		}
		else
		{
			if(!pActor->IsSecretlyGhosted())
			{
				pActor->SecretlyGhost();
			}
		}
	}

	// -------------------------------------------------------------------------------------------------

	bool C_VehicleComponent_AIController::OnMessage(const Messaging::C_MessageBase *pMessage) 
	{
		PERF_EVENT("C_VehicleComponent.AIController.MessageHandler");
		switch(pMessage->Type())
		{
		//case GameMessaging::E_GameMsg_Update:
		//	{
		//		if(mEnabled)
		//			Update();

		//		return true;
		//	}

		case ActorMsg::E_DynamicObjMsg_SetVehicleLogic:
			{
				const ActorMsg::C_SetVehicleLogicMsg* pMsg = Messaging::MessageCast<ActorMsg::C_SetVehicleLogicMsg>(pMessage);
				mEnabled = pMsg->m_Ghosting;			
				return true;
			}

		case Game::E_GameflowMessage_RaceStarted:
			{
				mEnabled = true;
				return true;
			}
			break;
		}

		return I_GameActorComponent::OnMessage(pMessage);
	}

} // namespace Components
