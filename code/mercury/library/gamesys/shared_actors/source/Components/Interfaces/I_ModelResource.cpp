#include "SharedActorsPch.h"
#include <shared_actors/components/interfaces/I_ModelResource.h>

#include <misc_shared/messaging/MessageTypeList.h>


namespace Components
{

I_ModelResource::I_ModelResource()
	: Actor::I_ActorComponent()
{
}

I_ModelResource::~I_ModelResource()
{
}

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(I_ModelResource)
ACTOR_COMPONENT_MESSAGE_TYPES_END(I_ModelResource, Actor::I_ActorComponent)

bool I_ModelResource::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	return Actor::I_ActorComponent::OnMessage(pMessage);
}

} // namespace Components
