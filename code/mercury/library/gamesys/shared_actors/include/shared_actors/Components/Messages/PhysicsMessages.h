#ifndef PHYSICS_MESSAGES_H
#define PHYSICS_MESSAGES_H


#include <actor_system/ActorMessageHandle.h>
#include <Gravity/sharedphysics/MemoryAllocator.h>
#include <Gravity/sdk/BodyEvents.h>
#include <gravity/sharedphysics/ContactStreamIterator.h>
#include <misc_shared/messaging/MessageValue.h>
#include <misc_shared/uid/FourCC.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/SharedMessageValues.h>
#include <shared_actors/XtActorMessages.h>
#include <shared_actors/ActorPhysics.h>
#include <shared_actors/Components/Creation/PhysicsBodyComponentCreate.h>


// Messages
enum E_PhysicsBodyMsg
{
	E_PhysicsBodyMsg_HiBits = FOURCC_HI20('PHBD'), 

	E_PhysicsBodyMsg_WorldInsert,
	E_PhysicsBodyMsg_WorldRemove,
	E_PhysicsBodyMsg_Deactivate,
	E_PhysicsBodyMsg_Activate,
	E_PhysicsBodyMsg_SetPositionAndOrientation,
	E_PhysicsBodyMsg_ApplyForce,
	E_PhysicsBodyMsg_ApplyImpulse,
	E_PhysicsBodyMsg_ApplyLocalImpulse,
	E_PhysicsBodyMsg_AddTorque,
	E_PhysicsBodyMsg_SetLinearMomentum,
	E_PhysicsBodyMsg_SetAngularMomentum,
	E_PhysicsBodyMsg_SetLinearVelocity,
	E_PhysicsBodyMsg_SetAngularVelocity,
	E_PhysicsBodyMsg_SetMaxLinearVelocity,
	E_PhysicsBodyMsg_SetMaxAngularVelocity,
	E_PhysicsBodyMsg_SetLinearDampingFactor,
	E_PhysicsBodyMsg_SetAngularDampingFactor,
	E_PhysicsBodyMsg_SetMinResolveIterations,
	E_PhysicsBodyMsg_UpdateShape,
	E_PhysicsBodyMsg_MakeStatic,
	E_PhysicsBodyMsg_MakeDynamic,
	E_PhysicsBodyMsg_MakeAnimated,
	E_PhysicsBodyMsg_SetCollideGroup,
	E_PhysicsBodyMsg_SetCollideGroupMask,
	E_PhysicsBodyMsg_SetCollideGroupFlags,
	E_PhysicsBodyMsg_SetCollideGlobalFlags,
	E_PhysicsBodyMsg_SetCollideGlobalMask,
	E_PhysicsBodyMsg_SetCollideMaterialMask,
	E_PhysicsBodyMsg_SetBodyFlag,
	E_PhysicsBodyMsg_ClearBodyFlag,
	E_PhysicsBodyMsg_SetDynamicsFlag,
	E_PhysicsBodyMsg_SetRaytraceOnlyGroup,
	E_PhysicsBodyMsg_ForceActive,
	E_PhysicsBodyMsg_AllowDeactivate,
	E_PhysicsBodyMsg_SetActivationThreshold,
	E_PhysicsBodyMsg_SetMaxEvents,
	E_PhysicsBodyMsg_AcceptEventType,
	E_PhysicsBodyMsg_RejectEventType,
	E_PhysicsBodyMsg_SetMinCollisionEventImpulse,
	E_PhysicsBodyMsg_CollisionEvent,
	E_PhysicsBodyMsg_MakeFrameOfReference,
	E_PhysicsBodyMsg_NoBuoyancy,
	E_PhysicsBodyMsg_SetCentreOfMassOffset,

	// non PBC messages
	E_PhysicsMsg_MakeStaticOnSettled,

	// Notifications
	E_PhysicsBodyNotification_TransformChanged,
	E_PhysicsBodyNotification_BodyCreated,
	E_PhysicsBodyNotification_BodyShapeChanged,

	E_PhysicsTriggerNotification_Enter,
	E_PhysicsTriggerNotification_Exit,

	E_PhysObjPhysicsBodyMsg_UpdateShape,
	E_PhysObjPhysicsBodyMsg_OverrideCollision,
	E_PhysObjPhysicsBodyMsg_UpdatePivotShapes,

	E_PhysicsBodyMsg_End, 
	E_PhysicsBodyMsg_Begin = E_PhysicsBodyMsg_HiBits + 1, 
	E_PhysicsBodyMsg_Count = E_PhysicsBodyMsg_End - E_PhysicsBodyMsg_Begin
};

namespace Actor
{

// moved out of core actor_system library, it doesn't need to know about physics
enum E_PhysicsGeneralMsg
{
	E_PhysicsGeneralMsg_HiBits = FOURCC_HI20('PGNL'),

	// General physics messages
    E_ActorMsg_RemoveCollision,
	E_ActorMsg_PhysicsEventCollision,
	E_ActorMsg_PhysicsBodyEvent,

	E_PhysicsGeneralMsg_End,
	E_PhysicsGeneralMsg_Begin = E_PhysicsGeneralMsg_HiBits + 1,
	E_PhysicsGeneralMsg_Count = E_PhysicsGeneralMsg_End - E_PhysicsGeneralMsg_Begin
};

};


typedef Messaging::C_MessageValue<S32, E_PhysicsBodyMsg_UpdateShape>  C_PhysicsUpdateShapeMsg;


class C_PhysicsBodyApplyForce : public Messaging::C_MessageDerived<C_PhysicsBodyApplyForce, E_PhysicsBodyMsg_ApplyForce>
{
public:
	C_PhysicsBodyApplyForce(const Maths::HVec3& force, const Maths::C_Vertex& worldPosition);

	Maths::HVec3 Force() const {return Maths::Load<Maths::HVec3>(&m_Force);}
	Maths::C_Vertex WorldPosition() const {return Maths::Load<Maths::C_Vertex>(&m_WorldPosition);}

private:
	MathsStorage::Float3 m_Force;
	MathsStorage::Float3 m_WorldPosition;
};


class C_PhysicsBodyApplyImpulse : public Messaging::C_MessageDerived<C_PhysicsBodyApplyImpulse, E_PhysicsBodyMsg_ApplyImpulse>
{
public:
	C_PhysicsBodyApplyImpulse(const Maths::C_ImpulseKgMpS& impulse, const Maths::C_Vertex& worldPosition);
	~C_PhysicsBodyApplyImpulse();

	Maths::C_ImpulseKgMpS Impulse() const {return Maths::Load<Maths::HVec3>(&m_Impulse);}
	Maths::C_Vertex WorldPosition() const {return Maths::Load<Maths::C_Vertex>(&m_WorldPosition);}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	MathsStorage::Float3 m_Impulse;	// Without mass factored in.
	MathsStorage::Float3 m_WorldPosition;

	static Messaging::C_MessageSerialiser s_Serialiser;
};


class C_PhysicsBodyApplyLocalImpulse : public Messaging::C_MessageDerived<C_PhysicsBodyApplyLocalImpulse, E_PhysicsBodyMsg_ApplyLocalImpulse>
{
public:
	C_PhysicsBodyApplyLocalImpulse();
	C_PhysicsBodyApplyLocalImpulse(const Maths::C_ImpulseKgMpS& impulse, const Maths::C_Vertex& position, bool localImpulse=true, bool localPosition=true);
	~C_PhysicsBodyApplyLocalImpulse();

	Maths::C_ImpulseKgMpS Impulse() const {return Maths::Load<Maths::HVec3>(&m_Impulse);}
	Maths::C_Vertex Position() const {return Maths::Load<Maths::C_Vertex>(&m_Position);}
	void Impulse(const Maths::C_ImpulseKgMpS& impulse)  {Maths::Store(impulse.Get(), &m_Impulse);}
	void Position(const Maths::C_Vertex& position)  {Maths::Store(position, &m_Position);}
	void SetLocalImpulse(bool local) {m_LocalImpulse=local;}
	void SetLocalPosition(bool local) {m_LocalPos=local;}
	bool LocalImpulse() const {return m_LocalImpulse;}
	bool LocalPosition() const {return m_LocalPos;}

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	MathsStorage::Float3	m_Impulse;
	MathsStorage::Float3	m_Position;
	bool					m_LocalImpulse;
	bool					m_LocalPos;

	static Messaging::C_MessageSerialiser s_Serialiser;

};

typedef Messaging::C_MessageValue<Maths::HVec3, E_PhysicsBodyMsg_AddTorque> C_PhysicsBodyAddTorque;

typedef Messaging::C_MessageValue<Maths::C_MomentumKgMpS, E_PhysicsBodyMsg_SetLinearMomentum> C_PhysicsBodySetLinearMomentum;


typedef Messaging::C_MessageValue<Maths::C_AngularMomentumKgM2RpS, E_PhysicsBodyMsg_SetAngularMomentum> C_PhysicsBodySetAngularMomentum;

typedef Messaging::C_MessageValue<Maths::C_QuatTrans, E_PhysicsBodyMsg_SetPositionAndOrientation> C_PhysicsBodySetPositionAndOrientation;


typedef Messaging::C_MessageValue<Maths::C_VelocityMpS, E_PhysicsBodyMsg_SetLinearVelocity> C_PhysicsBodySetLinearVelocity;


typedef Messaging::C_MessageValue<Maths::C_AngularVelocityRpS, E_PhysicsBodyMsg_SetAngularVelocity> C_PhysicsBodySetAngularVelocity;


typedef Messaging::C_MessageValue<F32, E_PhysicsBodyMsg_SetMaxLinearVelocity> C_PhysicsBodySetMaxLinearVelocity;


typedef Messaging::C_MessageValue<F32, E_PhysicsBodyMsg_SetMaxAngularVelocity> C_PhysicsBodySetMaxAngularVelocity;


typedef Messaging::C_MessageValue<F32, E_PhysicsBodyMsg_SetLinearDampingFactor> C_PhysicsBodySetLinearDampingFactor;


typedef Messaging::C_MessageValue<F32, E_PhysicsBodyMsg_SetAngularDampingFactor> C_PhysicsBodySetAngularDampingFactor;



typedef Messaging::C_MessageValue<S32, E_PhysicsBodyMsg_SetCollideGroup> C_PhysicsBodySetCollideGroup;


typedef Messaging::C_MessageValue<U32, E_PhysicsBodyMsg_SetCollideGroupMask> C_PhysicsBodySetCollideGroupMask;


typedef Messaging::C_MessageValue<U32, E_PhysicsBodyMsg_SetCollideGroupFlags> C_PhysicsBodySetCollideGroupFlags;


typedef Messaging::C_MessageValue<U32, E_PhysicsBodyMsg_SetCollideGlobalFlags> C_PhysicsBodySetCollideGlobalFlags;

typedef Messaging::C_MessageValue<bool, E_PhysicsBodyMsg_NoBuoyancy> C_PhysicsBodyDisableBuoyancy;

typedef Messaging::C_MessageValue<Maths::HVec3, E_PhysicsBodyMsg_SetCentreOfMassOffset> C_PhysicsBodySetCentreOfMassOffset;



// Sets the body flags of what it will collide with.
typedef Messaging::C_MessageValue<U32, E_PhysicsBodyMsg_SetCollideGlobalMask> C_PhysicsBodySetCollideGlobalMask;


typedef Messaging::C_MessageValue<U32, E_PhysicsBodyMsg_SetCollideMaterialMask> C_PhysicsBodySetCollideMaterialMask;


typedef Messaging::C_MessageValue<U32, E_PhysicsBodyMsg_SetBodyFlag> C_PhysicsBodySetBodyFlagMsg;


typedef Messaging::C_MessageValue<U32, E_PhysicsBodyMsg_ClearBodyFlag> C_PhysicsBodyClearBodyFlagMsg;


typedef Messaging::C_MessageValue<Maths::HMat43, E_PhysicsBodyNotification_TransformChanged> C_PhysicsBodyTransformChanged;

typedef Actor::C_MessageActorHandle <E_PhysicsTriggerNotification_Enter> C_PhysicsTriggerEnterMsg;
typedef Actor::C_MessageActorHandle <E_PhysicsTriggerNotification_Exit> C_PhysicsTriggerExitMsg;

typedef Actor::C_MessageActorHandle <E_PhysicsBodyMsg_Activate>			C_PhysicsBodyActivateMsg;
typedef Actor::C_MessageActorHandle <E_PhysicsBodyMsg_Deactivate>		C_PhysicsBodyDeactivateMsg;

typedef Messaging::C_MessageValue<bool, E_PhysicsMsg_MakeStaticOnSettled> C_PhysicsMsgMakeStaticOnSettled;

// Gravity
class C_PhysicsEventCollisionMsg : public Messaging::C_MessageDerived<C_PhysicsEventCollisionMsg, Actor::E_ActorMsg_PhysicsEventCollision>
{
public:
	C_PhysicsEventCollisionMsg(const Maths::HVec3& worldPos, const Maths::C_Normal& worldNormal, SharedPhysics::C_ContactType contactType, Maths::HF32 impulseMagnitude, Maths::HVec3 friction);
	~C_PhysicsEventCollisionMsg();

	Maths::HVec3	Friction() const		{return Maths::Load<Maths::HVec3>(&m_Friction);}
	Maths::HVec3	WorldPos() const		{return Maths::Load<Maths::HVec3>(&m_WorldPos);}
	Maths::C_Normal WorldNormal() const {return Maths::Load<Maths::C_Normal>(&m_WorldNormal);}
	Maths::HVec3	Impulse() const		{return m_ImpulseMagnitude*WorldNormal();}

	SharedPhysics::C_ContactType	m_ContactType;

	S16								m_BodyIndex;
	S16								m_OtherBodyIndex;
	U16								m_EffectMaterialIndex;
	U16								m_OtherEffectMaterialIndex;

	F32								m_ImpulseMagnitude;

	Actor::C_ActorHandle			m_OtherActorHandle;
	ActorPhysics::TBodyType			m_OtherBodyType;	// e.g. for camera events.
	U32								m_OtherBodyFlags;

	static void SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage);
	static void DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage);

private:
	MathsStorage::Float3 m_Friction;
	MathsStorage::Float3 m_WorldPos;
	MathsStorage::Float3 m_WorldNormal;

	static Messaging::C_MessageSerialiser s_Serialiser;

};


class C_PhysicsBodyEventMsg : public Messaging::C_MessageDerived<C_PhysicsBodyEventMsg, Actor::E_ActorMsg_PhysicsBodyEvent>
{
public:
	C_PhysicsBodyEventMsg(Gravity::C_BodyEvent bodyEvent) : m_BodyEvent(bodyEvent) {}
	const Gravity::C_BodyEvent m_BodyEvent;
};


typedef Messaging::C_MessageValue<F32, E_PhysicsBodyMsg_SetActivationThreshold> C_PhysicsBodySetActivationThresholdMsg;

typedef Messaging::C_MessageValue<bool, E_PhysicsBodyMsg_MakeFrameOfReference> C_PhysicsBodyMakeFrameOfReferenceMsg;

typedef Messaging::C_MessageValue<bool, E_PhysicsBodyMsg_NoBuoyancy> C_PhysicsBodyDisableBuoyancyMsg;


//////////////////////////////////////////////////////////////////////////////////////////////
// XT
//


//////////////////////////////////////////////////////////////////////////
// Physical Object
//
class C_PhysicalObjectPhysicsBodyUpdateShapeMsg : public Messaging::C_MessageDerived<C_PhysicalObjectPhysicsBodyUpdateShapeMsg, E_PhysObjPhysicsBodyMsg_UpdateShape>
{
public:
	C_PhysicalObjectPhysicsBodyUpdateShapeMsg(int overrideIndex=-1) : m_OverrideIndex(overrideIndex) {}
	C_PhysicalObjectPhysicsBodyUpdateShapeMsg(const Components::C_BasePhysicalObjectPhysicsBodyCreateInfo::TModelElemDescList& elemDescList, int overrideIndex=-1) : m_ElemDescList(elemDescList), m_OverrideIndex(overrideIndex) {}

public:
	Components::C_BasePhysicalObjectPhysicsBodyCreateInfo::TModelElemDescList m_ElemDescList;
	int m_OverrideIndex;
};

class C_PhysicalObjectPhysicsBodyOverrideCollisionMsg : public Messaging::C_MessageDerived<C_PhysicalObjectPhysicsBodyOverrideCollisionMsg, E_PhysObjPhysicsBodyMsg_OverrideCollision>
{
public:
	C_PhysicalObjectPhysicsBodyOverrideCollisionMsg(S32 index, U32 flags, U32 mask): m_OverrideIndex(index), m_Flags(flags), m_Mask(mask) {}
	S32 m_OverrideIndex;
	U32 m_Flags;
	U32 m_Mask;
};



#endif
