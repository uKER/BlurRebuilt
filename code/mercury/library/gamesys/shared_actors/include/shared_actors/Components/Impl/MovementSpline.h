#ifndef MERCURY_MOVEMENT_SPLINE_H
#define MERCURY_MOVEMENT_SPLINE_H

#include <misc_shared/Messaging/MessageBase.h>

#include <shared_actors/Components/Impl/MovementComponents.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

namespace Components
{
	///////////////////////////////////////////////////////////////////////////////////////////////////
	//Spline movement component class.
	//
	class C_MovementSpline : public C_MovementBase
	{
	public:
		C_MovementSpline() {}

	public: // From C_MovementBase
		static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementSpline;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementSpline;} // from Component::I_GameComponent
		bool OnMessage(const Messaging::C_MessageBase* pMessage);
		const C_MovementSplineCreateInfo*		GetCreationData(void) const {return static_cast<const C_MovementSplineCreateInfo *>(GetCreator().GetData());}

		virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

		virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);

	protected: // From C_MovementBase
		virtual void Animate(const Maths::C_TimeS& timeStep);
		void SetInitialPosition(Actor::C_ActorHandle splineHandle);
		void MoveAlongSpline(
			Actor::C_ActorHandle splineHandle, 
			F32 moveSpeed, 
			F32 rotation, 
			ActorMsg::TMotionTypeFlags motionFlags);

		Actor::C_ActorHandle m_SplineActorHandle;
		F32 m_Rotation;
		F32 m_MoveSpeed;	
		F32 m_ArcLengthProgression;
		F32 m_StartOffset;
		bool m_Loop;

	protected:
		virtual bool		InitialiseImpl(); // from Component::I_GameComponent
		virtual bool		CreateImpl(); // from Component::I_GameComponent

		virtual void ExternalDependenciesCreatedImpl(); // from Component::I_GameComponent

		void Serialise(Sys::C_ContextualStream& stream) const;
		void Deserialise(Sys::C_ContextualStream& stream);
	};

	///////////////////////////////////////////////////////////////////////////////////////////////////
	//Spline aim movement component class.
	//
	class C_MovementAimSpline : public C_MovementSpline
	{
	public:
		C_MovementAimSpline();

	public: // From C_MovementBase
		static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementAimSpline;}
		virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementAimSpline;} // from Component::I_GameComponent
		bool OnMessage(const Messaging::C_MessageBase* pMessage);
		const C_MovementAimSplineCreateInfo*		GetCreationData(void) const {return static_cast<const C_MovementAimSplineCreateInfo *>(GetCreator().GetData());}

		virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

		virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
		virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);

	protected: // From C_MovementBase
		virtual void Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform = E_MovementTransform_Update);
		//virtual void Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, bool recalcTransform);
		virtual void Animate(const Maths::C_TimeS& timeStep);

	protected:
		virtual bool		InitialiseImpl(); // from Component::I_GameComponent

		Actor::C_ActorHandle m_TargetHandle;
		F32 m_TrackingSpeed;
		F32 m_FutureTime;
		F32 m_RetrackingSpeed;
		bool m_UseRetrackingSpeed;
		
		Maths::C_Quaternion m_TargetQuatLs;	// Local space.
		Maths::HVec3 m_OffsetPos;
		Maths::HVec3 m_AimPos;
		bool m_LocalOffset;
		bool m_IsAligned;
		bool m_InitialLock;

	private:
		void Serialise(Sys::C_ContextualStream& stream) const;
		void Deserialise(Sys::C_ContextualStream& stream);
	};
}

#endif
