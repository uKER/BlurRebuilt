#include "SharedActorsPch.h"
#include <shared_actors/components/interfaces/I_ModelCustomiser.h>

#include <misc_shared/messaging/MessageTypeList.h>



namespace Components
{

I_ModelCustomiser::I_ModelCustomiser() :
	Actor::I_ActorComponent()
{
}

I_ModelCustomiser::~I_ModelCustomiser()
{
}

ACTOR_COMPONENT_MESSAGE_TYPES_BEGIN(I_ModelCustomiser)
ACTOR_COMPONENT_MESSAGE_TYPES_END(I_ModelCustomiser, Actor::I_ActorComponent)

} // namespace Components
