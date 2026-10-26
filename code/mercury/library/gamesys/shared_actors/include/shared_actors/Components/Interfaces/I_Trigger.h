#ifndef I_TRIGGER_64994598_CC9F_422B_BA99_5962312A91D2_H
#define I_TRIGGER_64994598_CC9F_422B_BA99_5962312A91D2_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <btl/containers/Map.h>
#include <shared_actors/Components/TriggerComponents.h>


namespace Components
{

/**
@brief Interface for trigger components using collision volumes.

Trigger components are designed to work alongside other code that tracks the
actors of interest and notifies the trigger about the movements of these 
actors using C_TrackingPositionMsg and C_TrackingLostMsg.

Depending on how the trigger has been configured, notifications will be sent
to the owning actor: C_TriggerEnterMsg; C_TriggerExitMsg; C_TriggerCrossedMsg.

Derived classes should implement IsInside and  CheckLineIntersection as
appropriate to the shape of the volume.
*/
class I_VolumeTrigger : public Actor::I_ActorComponent
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	struct C_TrackedState
	{
		Maths::C_Vertex	m_Position;		///< last known position
		bool			m_InTrigger;	///< last known state
	};

	virtual ~I_VolumeTrigger();

	static Component::C_ComponentInterfaceId InterfaceType() {return E_TriggerIF_Volume;}
	virtual Component::C_ComponentInterfaceId GetInterfaceID() const {return E_TriggerIF_Volume;} // from Component::I_GameComponent

	bool			OnMessage(const Messaging::C_MessageBase* pMessage);

	// from Component::I_GameComponent
	virtual void	SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void	SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void	DeserialiseForSync(Sys::C_ContextualStream& stream);

	/// @param point point to test.
	/// @returns true if the point is inside the volume.
	virtual bool	IsInside(const Maths::C_Vertex& point) const = 0;
	/// @param point point to test.
	/// @param position vertex to set to inside position
	/// @returns true if the point is inside the volume.
	virtual bool	GetPositionInside(const Maths::C_Vertex& point, Maths::C_Vertex& position) const = 0;

	/// @param lineStart start of line.
	/// @param lineEnd end of line.
	/// @returns true if the line crosses the volume. 
	virtual bool	CheckLineIntersection(const Maths::C_Vertex &lineStart, const Maths::C_Vertex &lineEnd) const = 0;

	virtual bool	GetBoxDimensions(Maths::C_Vector &dimensions) const = 0;
	virtual bool	GetSphereRadius(F32 &radius) const = 0;

protected:
	enum E_Flag
	{
		E_Flag_HandleEnterExit	= 1 << 0,	///< detect when an actor enters or leaves the volume
		E_Flag_HandleCrossing	= 1 << 1,	///< detect when an actor crosses the volume
		E_Flag_OneShot			= 1 << 2,	///< only work one time
		E_Flag_Active			= 1 << 3,	///< currently active
		E_Flag_EnterHandled		= 1 << 4,	///< actor has entered the trigger
		E_Flag_ItemInTrigger	= 1 << 5,   ///< there is an item in the trigger
	};

	explicit I_VolumeTrigger(const U32 flags = 0);

	bool			TestFlag(const E_Flag flag) const {return (m_Flags & flag) != 0;}
	void			SetFlag(const E_Flag flag) {m_Flags |= flag;}
	void			ClearFlag(const E_Flag flag) {m_Flags &= ~flag;}

protected:
	//struct													C_TrackedState; ///< state of an actor being tracked
	typedef Btl::Map<Actor::C_ActorHandle, C_TrackedState>	C_TrackedActors; ///< set of actors being tracked

	C_TrackedActors	m_TrackedActors;	///< set of actors being tracked
	U32				m_Flags;			///< E_Flag bitfield
private:

	void			OnActorPositionNotify(const Actor::C_ActorHandle& actorHandle, const Maths::C_Vertex& position); ///< Called when an actor position notification is received.
	void			OnActorLostNotify(); ///< Called when an actor lost notification is received.

	void			NotifyEnter(const Actor::C_ActorHandle& actorHandle); ///< Called when an actor enters the volume.
	void			NotifyExit(const Actor::C_ActorHandle& actorHandle); ///< Called when an actor exits the volume.
	void			NotifyCrossed(const Actor::C_ActorHandle& actorHandle); ///< Called when an actor crosses the volume.

	void			PhysicsNotifyEnter(const Actor::C_ActorHandle& actorHandle); ///< Called when a physics trigger source enters the volume.
	void			PhysicsNotifyExit(const Actor::C_ActorHandle& actorHandle); ///< Called when an physics trigger source exits the volume.

	void			Serialise(Sys::C_ContextualStream& stream) const;
	void			Deserialise(Sys::C_ContextualStream& stream);

};

class I_TriggerSourceVolume : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId InterfaceType() {return E_TriggerIF_SourceVolume;}
	virtual Component::C_ComponentInterfaceId GetInterfaceID() const {return E_TriggerIF_SourceVolume;} // from Component::I_GameComponent
};

} // namespace Components


#endif // I_TRIGGER_64994598_CC9F_422B_BA99_5962312A91D2_H
