#ifndef I_TRACKING_2F17E883_BB39_4E00_9AF8_F4CE8F9A85D3_H
#define I_TRACKING_2F17E883_BB39_4E00_9AF8_F4CE8F9A85D3_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <shared_actors/components/TrackingComponents.h>


namespace Components
{

class I_ActorTracking : public Actor::I_ActorComponent
{
public:
	virtual ~I_ActorTracking();

	static Component::C_ComponentInterfaceId InterfaceType() {return E_TrackingIF_Actor;}
	virtual Component::C_ComponentInterfaceId GetInterfaceID() const {return E_TrackingIF_Actor;} // from Component::I_GameComponent

protected:
	I_ActorTracking();

	void NotifyActorPosition(const Actor::C_ActorHandle& actorHandle, const Maths::C_Vertex& position); // Call when a tracked actor position is updated.
	void NotifyActorLost(); // Call when a tracked actor handle is lost.

};

} // namespace Components


#endif // I_TRACKING_2F17E883_BB39_4E00_9AF8_F4CE8F9A85D3_H
