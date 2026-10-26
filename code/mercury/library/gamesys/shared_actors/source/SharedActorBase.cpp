#include "SharedActorsPch.h"

#include <shared_actors/SharedActorBase.h>

#include <actor_studio_msg_game/ActorStudioMessaging.h>

namespace Actors
{

bool C_SharedActorBase::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		// Handle these message to avoid errors
		case ActorStudioMsgShared::E_ActorStudioMsg_Drag:
		case ActorStudioMsgShared::E_ActorStudioMsg_EndDrag:
		case ActorStudioMsgShared::E_ActorStudioMsg_Selected:
		case ActorStudioMsgShared::E_ActorStudioMsg_Deselected:
			break;
		default:
			return Actor::C_Actor::OnMessage(pMessage);
	}

	return true;
}

}