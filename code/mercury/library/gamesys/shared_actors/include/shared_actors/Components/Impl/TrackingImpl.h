#ifndef TRACKINGIMPL_DD267D92_85ED_48D1_B161_F6E9FD006DD7_H
#define TRACKINGIMPL_DD267D92_85ED_48D1_B161_F6E9FD006DD7_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/components/creation/TrackingCreate.h>
#include <shared_actors/components/interfaces/I_Tracking.h>


namespace Components
{

class C_SpecificActorTracking : public I_ActorTracking
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	C_SpecificActorTracking();
	virtual ~C_SpecificActorTracking();

	static Component::C_ComponentTypeId ClassType() {return E_Tracking_SpecificActor;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_Tracking_SpecificActor;} // from Component::I_GameComponent

	virtual bool OnMessage(const Messaging::C_MessageBase* pMessage); // from Messaging::C_MessageHandler
	
	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

private:
	Actor::C_ActorHandle	m_SpecificActor;	///< actor being tracked
	bool					m_IsTracking;		///< is there an actor being tracked

	void SetSpecificActor(const Actor::C_ActorHandle& actorHandle);

	void OnUpdate();

	void Serialise(Sys::C_ContextualStream& stream) const; // from Component::I_GameComponent
	void Deserialise(Sys::C_ContextualStream& stream); // from Component::I_GameComponent

};

} // namespace Components


#endif // TRACKINGIMPL_DD267D92_85ED_48D1_B161_F6E9FD006DD7_H
