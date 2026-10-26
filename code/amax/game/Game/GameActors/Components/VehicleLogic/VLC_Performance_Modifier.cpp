#include "stdafx.h"

#include <Game/Game.h>

#include <Game\GameActors\Components\GameComponents.h>
#include "VLC_Performance_Modifier.h"

#include <Game/Messaging/GameMessages.h>
#include <Game/Messaging/VehicleActorMessages.h>

#include <actor_system/ActorSys.h>

#include <Game\GameActors\Vehicles\VehicleActor.h>
#include <Game\GameActors\VehicleLogic\VehicleLogicActor.h>

namespace Components
{

bool C_VehicleComponent_Performance_Modifier::CreateImpl()
{
	const I_VehicleComponent_Performance_Modifier_CreateInfo* creation_data = static_cast<const I_VehicleComponent_Performance_Modifier_CreateInfo *>(GetCreator().GetData());

	if(creation_data)
	{

	}

	AddSubscriber( GameMessaging::E_GameMsg_Update );
	AddSubscriber( Game::E_VehicleMessage_UpdatePerformanceModifiers );

	return true;
}

void C_VehicleComponent_Performance_Modifier::Update()
{

}

// Message Handlers
bool C_VehicleComponent_Performance_Modifier::OnMessage(const Messaging::C_MessageBase *pMessage) 
{
	switch(pMessage->Type())
	{
		case GameMessaging::E_GameMsg_Update:
			{
				mDeltaTime = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();
				Update();
			}
			break;

		case Game::E_VehicleMessage_UpdatePerformanceModifiers:
			break;
	}

	return I_GameActorComponent::OnMessage(pMessage);
}

} // namespace Components