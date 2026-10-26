#ifndef MERCURY_MOVEMENT_KINEMATIC_SPLINE_H
#define MERCURY_MOVEMENT_KINEMATIC_SPLINE_H

#include <misc_shared/Messaging/MessageBase.h>

#include <shared_actors/Components/Impl/MovementComponents.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

namespace Actors
{
	class C_SplineActor;
}


namespace Components
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//Kinematic Spline movement component class.
	//
	class C_MovementKinematicSpline : public C_MovementBase
	{
	public:
		C_MovementKinematicSpline() {}

	public: // From C_MovementBase
		static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementKinematicSpline;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementKinematicSpline;} // from Component::I_GameComponent
		bool OnMessage(const Messaging::C_MessageBase* pMessage);
		const C_MovementKinematicSplineCreateInfo*		GetCreationData(void) const {return static_cast<const C_MovementKinematicSplineCreateInfo *>(GetCreator().GetData());}

		virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

		virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);
#ifndef CFG_GOLD
		virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

	protected: // From C_MovementBase
		virtual void Animate(const Maths::C_TimeS& timeStep);
		void SetInitialPosition(Actor::C_ActorHandle splineHandle, bool moveToNearest);
		void MoveAlongSpline(Actor::C_ActorHandle splineHandle, F32 startSpeed, F32 minSpeed, F32 maxSpeed, F32 accel);

		void UpdateSplineProgression(const Actors::C_SplineActor* pSpline, F32 displacement);
		bool UpdateEndOfSplineBraking(const Actors::C_SplineActor* pSpline);
		void UpdateSlaves(const Maths::C_TimeS& timeStep);
		

		Actor::C_ActorHandle m_SplineActorHandle;
		F32 m_MaxAcceleration;		
		F32 m_Acceleration;		
		F32 m_MaxSpeed;
		F32 m_MinSpeed;		
		F32 m_StartSpeed;
		F32 m_Velocity;
		F32 m_ArcLengthProgression;
		F32 m_StartOffset;
		F32 m_MinArcProgression;
		bool m_Loop;
		bool m_BrakeAtEnd;

		Btl::Vector<Actor::C_ActorHandle> m_Slaves;

	protected:
		virtual bool		InitialiseImpl(); // from Component::I_GameComponent
		virtual bool		CreateImpl(); // from Component::I_GameComponent

		virtual void ExternalDependenciesCreatedImpl(); // from Component::I_GameComponent

		void Serialise(Sys::C_ContextualStream& stream) const;
		void Deserialise(Sys::C_ContextualStream& stream);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//Kinematic Spline Tracker movement component class.
	//
	class C_MovementKinematicSplineTracker : public C_MovementKinematicSpline
	{
	public:
		C_MovementKinematicSplineTracker() {}

	public: // From C_MovementBase
		static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementKinematicSplineTracker;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementKinematicSplineTracker;} // from Component::I_GameComponent
		bool OnMessage(const Messaging::C_MessageBase* pMessage);
		const C_MovementKinematicSplineTrackerCreateInfo*		GetCreationData(void) const {return static_cast<const C_MovementKinematicSplineTrackerCreateInfo *>(GetCreator().GetData());}

		virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

		virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);

#ifndef CFG_GOLD
		virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

	protected: // From C_MovementBase
		virtual void Animate(const Maths::C_TimeS& timeStep);
		
		Actor::C_ActorHandle m_TrackerActorHandle;
		F32 m_IdealDistance;	
		F32	m_TrackerActorPos;
		S32 m_TrackerNearestIndex;

		F32 m_TrackerVel;
		F32	m_ReactionDistance;
		

	protected:
		virtual bool		InitialiseImpl(); // from Component::I_GameComponent
		virtual bool		CreateImpl(); // from Component::I_GameComponent

		void Serialise(Sys::C_ContextualStream& stream) const;
		void Deserialise(Sys::C_ContextualStream& stream);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//Kinematic Spline Slave movement component class.
	//
	class C_MovementKinematicSplineSlave : public C_MovementBase
	{
	public:
		C_MovementKinematicSplineSlave() {}

	public: // From C_MovementBase
		static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementKinematicSplineSlave;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementKinematicSplineSlave;} // from Component::I_GameComponent
		bool OnMessage(const Messaging::C_MessageBase* pMessage);
		const C_MovementKinematicSplineSlaveCreateInfo*		GetCreationData(void) const {return static_cast<const C_MovementKinematicSplineSlaveCreateInfo *>(GetCreator().GetData());}

		virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

		virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);
#ifndef CFG_GOLD
		virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

	protected: // From C_MovementBase
		void Animate(const Maths::C_TimeS& timeStep) {};
		virtual void Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform = E_MovementTransform_Update);
		void	SlaveUpdate(F32 timeStep, F32 slaveMasterPosition);
		void SetInitialPosition(Actor::C_ActorHandle splineHandle, F32 slaveMasterPosition);
		


		Actor::C_ActorHandle m_SplineActorHandle;
		Actor::C_ActorHandle m_SlaveMasterHandle;
		F32 m_ArcLengthProgression;
		F32 m_StartOffset;
		F32 m_SlaveMasterOffset;
		
		

	protected:
		virtual bool		InitialiseImpl(); // from Component::I_GameComponent
		virtual bool		CreateImpl(); // from Component::I_GameComponent

		void Serialise(Sys::C_ContextualStream& stream) const;
		void Deserialise(Sys::C_ContextualStream& stream);
	};

}

#endif
