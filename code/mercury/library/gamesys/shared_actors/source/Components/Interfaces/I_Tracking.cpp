#include "SharedActorsPch.h"

#include <shared_actors/components/interfaces/I_Tracking.h>

#include <shared_actors/components/messages/TrackingMessages.h>


namespace Components
{

I_ActorTracking::I_ActorTracking() :
	Actor::I_ActorComponent()
{
	// intentionally blank
}

I_ActorTracking::~I_ActorTracking()
{
	// intentionally blank
}

/**
@param actorHandle actor being tracked.
@param position current position of actor.
*/
void I_ActorTracking::NotifyActorPosition(const Actor::C_ActorHandle& actorHandle, const Maths::C_Vertex& position)
{
	const ActorMsg::C_TrackingPositionMsg message(actorHandle, position);
	SendMessageToActor(&message);
}

void I_ActorTracking::NotifyActorLost()
{
	const Messaging::C_MessageBase message(Messaging::BuildSimpleMessage(ActorMsg::E_TrackingMsg_Lost));
	SendMessageToActor(&message);
}

} // namespace Components
