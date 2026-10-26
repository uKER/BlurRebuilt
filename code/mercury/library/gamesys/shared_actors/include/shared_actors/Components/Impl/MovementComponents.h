#ifndef _MERCURY_MOVEMENTCOMPONENT_H
#define _MERCURY_MOVEMENTCOMPONENT_H

#include <misc_shared/Messaging/MessageBase.h>

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/XtSharedLevelActorParams.h>

#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

#include <animation/pose/Pose.h>

namespace Components
{

// TODO: move to I_MovementComponent
// Movement Component Interface
class I_Movement : public Actor::I_ActorComponent
{
public:	// From I_ActorComponent
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_Movement;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_Movement;} // from Component::I_GameComponent

	// Interface Methods
	virtual bool	IsAnimating() const = 0;
	virtual U32		MovementCount() const = 0;
	virtual bool	IsEnabled() const = 0;
	virtual const Maths::C_QuatTrans& WorldTransform() const  = 0;
	virtual const Maths::HVec3& LinearVelocity() const   = 0;
	virtual bool HasValidParent() const  = 0;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Movement component base class.
//
class C_MovementBase : public I_Movement
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	C_MovementBase();
	virtual ~C_MovementBase() {}

public:
	// Interface Methods
	virtual bool	IsAnimating() const {return m_IsAnimating;}
	virtual U32		MovementCount() const {return m_MovementCount;}
	virtual bool	IsEnabled() const {return m_IsEnabled;}

	virtual const Maths::C_QuatTrans& WorldTransform() const {return m_WorldTransform;}
	virtual const Maths::HVec3& LinearVelocity() const {return m_LinearVelocity;}
	virtual bool HasValidParent() const;

public:	// From I_ActorComponent
// 	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_Movement;}
// 	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_Movement;} // from Component::I_GameComponent
	bool OnMessage(const Messaging::C_MessageBase* pMessage);

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

	const C_MovementCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementCreateInfo *>(GetCreator().GetData());}
#ifndef CFG_GOLD
	virtual void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected:
	virtual void Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform = E_MovementTransform_Update);
	virtual void Animate(const Maths::C_TimeS& timeStep) = 0;
	void SetIsAnimating(bool set=true);// {m_IsAnimating = set;}
	const Components::C_MovementBase* ActorMovementComponent(Actor::C_ActorHandle handle) const;
	void IncMovementCount(void);
	void DecMovementCount(void);
	void SetParent(Actor::C_ActorHandle parentHandle);
	void ClearParent();
	void AttachToParent(Actor::C_ActorHandle parentHandle, const Maths::C_QuatTrans& parentWorldTransform);
	virtual void CalcWorldTransform(const Maths::C_QuatTrans& parentWorldTransform);
	void AnimateActorTransform(const Maths::C_QuatTrans& transform, const Maths::C_TimeS& timeStep);
	void SetActorFinalTransform(const Maths::C_QuatTrans& transform);
	bool GetActorWorldMatrix(Actor::C_ActorHandle handle, Maths::HMat43& matrix) const;
	F32 CalcInterpolationFraction(F32 v1, F32 v2, ActorMsg::TMotionTypeFlags motionFlags);

	virtual bool		InitialiseImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent

	virtual void ExternalDependenciesCreatedImpl(); // from Component::I_GameComponent

private:
	C_MovementBase& operator=(const C_MovementBase& rhs);
	C_MovementBase(const C_MovementBase& rhs);

	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);

private:
	bool m_IsAnimating;
	U32	m_MovementCount;
	bool m_IsEnabled;
	bool m_DisableSlowMotion;

protected:
	Maths::C_QuatTrans m_LocalTransform;	// Current local animation transform.
	Maths::C_QuatTrans m_ParentOffset;		// Offset relative to parent.  This is the object's local-to-world transform if no parent (i.e. world is parent).
	Maths::C_QuatTrans m_WorldTransform;	// Current world transform.  Keep this here so we can have an animator without a body (e.g. for compositing animation types).
	Maths::HVec3 m_LinearVelocity;
	ActorMsg::TMotionTypeFlags m_MotionFlags;
	Maths::C_TimeS m_AnimateTime;			// Time spent animating so far.
	Maths::C_TimeS m_AnimateDuration;		// Target duration of animation.

	// Hierarchy info:
	//	C_MovementBase* m_pParent;		// Parent component, or NULL if root.
	Actor::C_ActorHandle m_NotificationHandle;	// Actor to notify that movement has been completed

	Actor::C_ActorHandle m_ParentHandle;
	Actor::C_ActorHandle m_AttachHandle;	// Actor to be attached to on construction.
	Btl::Vector<Actor::C_ActorHandle> m_Children;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// Attacher movement component.
//
// A non-animating movement placeholder for hierarchy formation, or attaching an actor to another.
//
class C_MovementAttacher : public C_MovementBase
{
public:
	C_MovementAttacher() {}

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementAttacher;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementAttacher;} // from Component::I_GameComponent
	const C_MovementAttacherCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementAttacherCreateInfo *>(GetCreator().GetData());}
	F32 m_DopplerSpeed;

protected: // From C_MovementBase
	void Animate(const Maths::C_TimeS& timeStep) {} // Do nothing.
	

protected:
	bool CreateImpl();
};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Translator movement component class.
//
class C_MovementTranslator : public C_MovementBase
{
public:
	C_MovementTranslator() : m_BaseTranslation(Maths::ZERO_INIT), m_TargetTranslation(Maths::ZERO_INIT) {}

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementTranslator;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementTranslator;} // from Component::I_GameComponent
	bool OnMessage(const Messaging::C_MessageBase* pMessage);
	const C_MovementTranslatorCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementTranslatorCreateInfo *>(GetCreator().GetData());}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);
#ifndef CFG_GOLD
	void DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const;
#endif

protected: // From C_MovementBase
	void Animate(const Maths::C_TimeS& timeStep);

protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent

private:
	void Translate(const ActorMsg::C_ActorMovementTranslateMsg& translateMsg);

	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);

private:
	Maths::C_Vertex m_BaseTranslation;	// Translation from matrix.
	Maths::C_Vertex m_TargetTranslation;
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// Rotator movement component class.
//
class C_MovementRotator : public C_MovementBase
{
public:
	C_MovementRotator() : m_BaseRotation(0.0f), m_TargetRotation(0.0f) {}

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementRotator;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementRotator;} // from Component::I_GameComponent
	bool OnMessage(const Messaging::C_MessageBase* pMessage);
	const C_MovementRotatorCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementRotatorCreateInfo *>(GetCreator().GetData());}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	void RotateAxis(const ActorMsg::C_ActorMovementRotateMsg& msg);

protected: // From C_MovementBase
	void Animate(const Maths::C_TimeS& timeStep);

protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent

protected:
	Maths::C_Normal m_Axis;
	Maths::HF32 m_BaseRotation;
	Maths::HF32 m_Rotation;
	Maths::HF32 m_TargetRotation;

private:
	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);

};



///////////////////////////////////////////////////////////////////////////////////////////////////
// RotTrans movement component class.
//
class C_MovementRotTrans : public C_MovementBase
{
public:
	C_MovementRotTrans() {}

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementRotTrans;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementRotTrans;} // from Component::I_GameComponent
	bool OnMessage(const Messaging::C_MessageBase* pMessage);
	const C_MovementRotTransCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementRotTransCreateInfo *>(GetCreator().GetData());}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

protected: // From C_MovementBase
	void Animate(const Maths::C_TimeS& timeStep);

protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent

private:
	void MoveAndAlignToNode(const Maths::C_QuatTrans& quatTrans, const Maths::C_TimeS PCREF time, ActorMsg::TMotionTypeFlags flags=ActorMsg::E_MovementFlag_Smooth);
	void Translate(const Maths::HVec3 PCREF translation, const Maths::C_TimeS PCREF time, ActorMsg::TMotionTypeFlags flags=ActorMsg::E_MovementFlag_Smooth);
	void Rotate(const Maths::C_Normal PCREF axis, const F32 angle, const Maths::C_TimeS PCREF time, ActorMsg::TMotionTypeFlags flags=ActorMsg::E_MovementFlag_Smooth);

protected:
	Maths::HVec3 m_TargetTranslation;
	Maths::C_Quaternion m_qRot; // quaternion difference

private:
	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);

};



///////////////////////////////////////////////////////////////////////////////////////////////////
// Spinner movement component class.
//
class C_MovementSpinner : public C_MovementBase
{
public:
	C_MovementSpinner() : m_SpinState(EStopped), m_Angle(0), m_RequiredSpinRadiansPerSec(0), m_CurrentRPM(0) {}

	F32 CurrentRPM() const {return m_CurrentRPM;}

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementSpinner;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementSpinner;} // from Component::I_GameComponent
	bool OnMessage(const Messaging::C_MessageBase* pMessage);
	const C_MovementSpinnerCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementSpinnerCreateInfo *>(GetCreator().GetData());}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	void SpinAxisY(F32 revsPerMinute, const Maths::C_TimeS& spinupTime);
	void Stop(const ActorMsg::C_ActorMovementSpinStopMsg& msg);

protected: // From C_MovementBase
	void Animate(const Maths::C_TimeS& timeStep);

protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent

protected:
	enum TSpinState
	{
		EStopped,
		ESpinningUp,
		ESpinning,
		ESpinningDown
	};

	TSpinState m_SpinState;
	F32 m_Angle;
	F32 m_RequiredSpinRadiansPerSec;
	F32	m_CurrentRPM;

private:
	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);

};


///////////////////////////////////////////////////////////////////////////////////////////////////
// Wheel movement component class.
//
class C_MovementWheel : public C_MovementBase
{
public:
	C_MovementWheel() : m_Radius(1.0f), m_Angle(0.0f) {}

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementWheel;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementWheel;} // from Component::I_GameComponent
	const C_MovementWheelCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementWheelCreateInfo *>(GetCreator().GetData());}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);

protected: // From C_MovementBase
	virtual void Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform = E_MovementTransform_Update);
	//void Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, bool recalcTransform=true);
	void Animate(const Maths::C_TimeS& timeStep);

protected:
	virtual bool		CreateImpl(); // from Component::I_GameComponent

protected:
	F32 m_Radius;
	F32 m_Angle;

private:
	void	Serialise(Sys::C_ContextualStream& stream) const;
	void	Deserialise(Sys::C_ContextualStream& stream);
};



///////////////////////////////////////////////////////////////////////////////////////////////////
// Constrains movement in various ways.
//
class C_MovementConstrainer : public C_MovementBase
{
public:
	enum TConstrainFlags
	{
		EPitch		= 0x01,
		ERoll		= 0x02,
		EHdg		= 0x04,
		ETransX		= 0x08,
		ETransY		= 0x10,
		ETransZ		= 0x20		
	};

public:
	C_MovementConstrainer() : m_ConstrainFlags(0) {}

public: // From C_MovementBase
	static Component::C_ComponentTypeId ClassType() {return Components::E_MovementConstrainer;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return Components::E_MovementConstrainer;}
	const C_MovementConstrainerCreateInfo* GetCreationData(void) const {return static_cast<const C_MovementConstrainerCreateInfo*>(GetCreator().GetData());}

	bool OnMessage(const Messaging::C_MessageBase* pMessage);

	void SerialiseForSave(Sys::C_ContextualStream& stream) const {C_MovementBase::SerialiseForSave(stream); Serialise(stream);}
	void DeserialiseForSave(Sys::C_ContextualStream& stream) {C_MovementBase::DeserialiseForSave(stream); Deserialise(stream);}
	void SerialiseForSync(Sys::C_ContextualStream& stream) const {C_MovementBase::SerialiseForSync(stream); Serialise(stream);}
	void DeserialiseForSync(Sys::C_ContextualStream& stream) {C_MovementBase::DeserialiseForSync(stream); Deserialise(stream);}

protected: // From C_MovementBase
	void CalcWorldTransform(const Maths::C_QuatTrans& parentWorldTransform);
	void Animate(const Maths::C_TimeS& timeStep) {}

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

private:
	U32 m_ConstrainFlags;
	Maths::HVec3 m_TransConstrain;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
// Animation Slave movement component.
//
// This component will query it's animation controller for its position each frame
//
class C_MovementAnimationSlave : public C_MovementBase
{
public:
	ACTOR_COMPONENT_MESSAGE_TYPES_DECLARE()

public:
	C_MovementAnimationSlave();

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementAnimationSlave;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementAnimationSlave;} // from Component::I_GameComponent
	const C_MovementAnimationSlaveCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementAnimationSlaveCreateInfo *>(GetCreator().GetData());}

	bool OnMessage(const Messaging::C_MessageBase* pMessage);

	bool							CalculateNewOrientation(Maths::C_Vertex &newTrans,Maths::C_Quaternion &newRot);

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);
	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);


protected: // From C_MovementBase
	void CalcWorldTransform(const Maths::C_QuatTrans& parentWorldTransform);
	void Animate(const Maths::C_TimeS& timeStep) {}

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

protected:
	bool CreateImpl();

private:
	Actor::C_ActorHandle	m_AnimationController;	// Handle of animation controller actor
	S32						m_AnimationObjectID;	// Index of which animation object we a bound to
};

/**
Places an actor relative to the current world space position of a bone being animated
by an object with an I_AnimationState component.
*/
class C_MovementRelativeToBone: public C_MovementBase
{
public:
	C_MovementRelativeToBone();
	
	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_MovementRelativeToBone;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_MovementRelativeToBone;}
	
	const C_MovementRelativeToBoneCreateInfo* GetCreationData(void) const
	{
		return static_cast<const C_MovementRelativeToBoneCreateInfo *>(GetCreator().GetData());
	}

	virtual bool OnMessage(const Messaging::C_MessageBase* pMessage);

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);
	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);


protected: // From C_MovementBase
	void CalcWorldTransform(const Maths::C_QuatTrans& parentWorldTransform);
	void Animate(const Maths::C_TimeS& timeStep);
	void CalcOffsetTransform();

	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

protected:
	virtual bool InitialiseImpl();
	virtual bool CreateImpl();

	virtual void ExternalDependenciesCreatedImpl();

private:
	void AttachToBone(const Actor::C_Actor* pParent);

	Maths::C_Quaternion			m_OriginalRotation;

	S16							m_JointIndex;
	bool						m_AttachedToBone;
	bool						m_IgnoreBoneRotation;
};




///////////////////////////////////////////////////////////////////////////////////////////////////
// Aimer movement component class.
//
class C_MovementAimer : public C_MovementBase
{
public:
	C_MovementAimer() : m_TrackingSpeed(0.1f), m_IsAligned(false), m_LimitAngleAmount(Maths::Pi2), m_LimitAngle(false) {}

public: // From C_MovementBase
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MovementAimer;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MovementAimer;} // from Component::I_GameComponent
	bool OnMessage(const Messaging::C_MessageBase* pMessage);
	const C_MovementAimerCreateInfo	*GetCreationData(void) const {return static_cast<const C_MovementAimerCreateInfo *>(GetCreator().GetData());}

	virtual void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void DeserialiseForSync(Sys::C_ContextualStream& stream);

protected: // From C_MovementBase
	virtual void Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, const E_MovementTransform transform = E_MovementTransform_Update);
	//void Update(const Maths::C_TimeS& timeStep, const Maths::C_QuatTrans& parentWorldTransform, bool recalcTransform=true);
	void Animate(const Maths::C_TimeS& timeStep);

protected:
	virtual bool		InitialiseImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent

protected:
	Actor::C_ActorHandle m_TargetHandle;
	F32 m_TrackingSpeed;
	F32 m_FutureTime;
	F32 m_RetrackingSpeed;
	F32 m_LimitAngleAmount;
	bool m_UseRetrackingSpeed;
	
	Maths::C_Quaternion m_TargetQuatLs;	// Local space.
	Maths::HVec3 m_OffsetPos;
	Maths::HVec3 m_AimPos;
	bool m_LocalOffset;
	bool m_IsAligned;
	bool m_InitialLock;
	bool m_LimitAngle;


private:
	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);
};

class C_MovementBobbingBoat: public C_MovementBase
{
public:
	
	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_MovementBobbingBoat;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_MovementBobbingBoat;}

	const C_MovementBobbingBoatCreateInfo* GetCreationData(void) const
	{
		return static_cast<const C_MovementBobbingBoatCreateInfo *>(GetCreator().GetData());
	}

	virtual void					SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSave(Sys::C_ContextualStream& stream);
	virtual void					SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void					DeserialiseForSync(Sys::C_ContextualStream& stream);


protected: // From C_MovementBase
	void Animate(const Maths::C_TimeS& timeStep);
	
	void Serialise(Sys::C_ContextualStream& stream) const;
	void Deserialise(Sys::C_ContextualStream& stream);

protected:
	//virtual bool InitialiseImpl();
	virtual bool CreateImpl();

private:
	F32 m_PitchTimer;
	F32 m_RollTimer;
	F32 m_PitchWavelength;
	F32 m_RollWavelength;
	F32 m_PitchWaveAmplitude;
	F32 m_RollWaveAmplitude;
};

}//namespace Components

#endif
