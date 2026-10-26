#include "SharedActorsPch.h"
#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/Components/Messages/PhysicsMessages.h>



C_PhysicsBodyApplyForce::C_PhysicsBodyApplyForce(const Maths::HVec3& force, const Maths::C_Vertex& worldPosition)
{
	Maths::Store(force, &m_Force);
	Maths::Store(worldPosition, &m_WorldPosition);
}



Messaging::C_MessageSerialiser C_PhysicsBodyApplyImpulse::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_PhysicsBodyApplyImpulse::C_PhysicsBodyApplyImpulse(const Maths::C_ImpulseKgMpS& impulse, const Maths::C_Vertex& worldPosition)
{
	Maths::Store(impulse.Get(), &m_Impulse);
	Maths::Store(worldPosition, &m_WorldPosition);
}

C_PhysicsBodyApplyImpulse::~C_PhysicsBodyApplyImpulse()
{
}

void C_PhysicsBodyApplyImpulse::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_PhysicsBodyApplyImpulse* pMsg = Messaging::MessageCast<C_PhysicsBodyApplyImpulse>(pMessage);

	stream << pMsg->m_Impulse;
	stream << pMsg->m_WorldPosition;
}

void C_PhysicsBodyApplyImpulse::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_PhysicsBodyApplyImpulse* pMsg = static_cast<C_PhysicsBodyApplyImpulse*>(pMessage);

	stream >> pMsg->m_Impulse;
	stream >> pMsg->m_WorldPosition;
}





Messaging::C_MessageSerialiser C_PhysicsBodyApplyLocalImpulse::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_PhysicsBodyApplyLocalImpulse::C_PhysicsBodyApplyLocalImpulse()
{
}

C_PhysicsBodyApplyLocalImpulse::C_PhysicsBodyApplyLocalImpulse(const Maths::C_ImpulseKgMpS& impulse, const Maths::C_Vertex& position, bool localImpulse, bool localPosition) :
	m_LocalImpulse(localImpulse), 
	m_LocalPos(localPosition)
{
	Maths::Store(impulse.Get(), &m_Impulse);
	Maths::Store(position, &m_Position);
}

C_PhysicsBodyApplyLocalImpulse::~C_PhysicsBodyApplyLocalImpulse()
{
}

void C_PhysicsBodyApplyLocalImpulse::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_PhysicsBodyApplyLocalImpulse* pMsg = Messaging::MessageCast<C_PhysicsBodyApplyLocalImpulse>(pMessage);

	stream << pMsg->m_Impulse;
	stream << pMsg->m_Position;
	stream << pMsg->m_LocalImpulse;
	stream << pMsg->m_LocalPos;
}

void C_PhysicsBodyApplyLocalImpulse::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_PhysicsBodyApplyLocalImpulse* pMsg = static_cast<C_PhysicsBodyApplyLocalImpulse*>(pMessage);

	stream >> pMsg->m_Impulse;
	stream >> pMsg->m_Position;
	stream >> pMsg->m_LocalImpulse;
	stream >> pMsg->m_LocalPos;
}




Messaging::C_MessageSerialiser C_PhysicsEventCollisionMsg::s_Serialiser(StaticType(), &SerialiseContents, &DeserialiseContents);

C_PhysicsEventCollisionMsg::C_PhysicsEventCollisionMsg(const Maths::HVec3& worldPos, const Maths::C_Normal& worldNormal, SharedPhysics::C_ContactType contactType, Maths::HF32 impulseMagnitude, Maths::HVec3 friction) : 
	m_ContactType(contactType), 
	m_BodyIndex(-1),
	m_OtherBodyIndex(-1),
	m_EffectMaterialIndex(0),
	m_OtherEffectMaterialIndex(0),
	m_ImpulseMagnitude(impulseMagnitude), 
	m_OtherBodyType(ActorPhysics::E_BodyType_Invalid)
{
	Maths::Store(friction, &m_Friction);
	Maths::Store(worldPos, &m_WorldPos);
	Maths::Store(worldNormal, &m_WorldNormal);
}

C_PhysicsEventCollisionMsg::~C_PhysicsEventCollisionMsg()
{
}

void C_PhysicsEventCollisionMsg::SerialiseContents(Sys::C_ContextualStream& stream, const C_MessageBase* pMessage)
{
	const C_PhysicsEventCollisionMsg* pMsg = Messaging::MessageCast<C_PhysicsEventCollisionMsg>(pMessage);

	stream.WriteAs<U32>(pMsg->m_ContactType);
	stream << pMsg->m_BodyIndex;
	stream << pMsg->m_OtherBodyIndex;
	stream << pMsg->m_EffectMaterialIndex;
	stream << pMsg->m_OtherEffectMaterialIndex;
	stream << pMsg->m_ImpulseMagnitude;
	stream << pMsg->m_OtherActorHandle;
	stream.WriteAs<U32>(pMsg->m_OtherBodyType);
	stream << pMsg->m_OtherBodyFlags;
	stream << pMsg->m_Friction;
	stream << pMsg->m_WorldPos;
	stream << pMsg->m_WorldNormal;
}

void C_PhysicsEventCollisionMsg::DeserialiseContents(Sys::C_ContextualStream& stream, C_MessageBase* pMessage)
{
	C_PhysicsEventCollisionMsg* pMsg = static_cast<C_PhysicsEventCollisionMsg*>(pMessage);

	stream.ReadAs<U32>(pMsg->m_ContactType);
	stream >> pMsg->m_BodyIndex;
	stream >> pMsg->m_OtherBodyIndex;
	stream >> pMsg->m_EffectMaterialIndex;
	stream >> pMsg->m_OtherEffectMaterialIndex;
	stream >> pMsg->m_ImpulseMagnitude;
	stream >> pMsg->m_OtherActorHandle;
	stream.ReadAs<U32>(pMsg->m_OtherBodyType);
	stream >> pMsg->m_OtherBodyFlags;
	stream >> pMsg->m_Friction;
	stream >> pMsg->m_WorldPos;
	stream >> pMsg->m_WorldNormal;
}


// Xt Messaging
class C_XtPhysicsApplyImpulseMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtPhysicsApplyImpulseMsg (){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		BIZ_ASSERT(pActorContext);
		m_Msg.Impulse(m_Impulse->GetVector(pActorContext));
		m_Msg.Position(m_Position ? m_Position->GetVector(pActorContext).GetHVec() : Maths::HVec3());
		m_Msg.SetLocalPosition(true);
		m_Msg.SetLocalImpulse(m_LocalImpulse->GetBool(pActorContext));
		return &m_Msg;
	}

	Actors::C_XtParamVector* m_Position;
	Actors::C_XtParamVector* m_Impulse;
	Actors::C_XtParamBool* m_LocalImpulse;

	C_PhysicsBodyApplyLocalImpulse m_Msg;
};


class C_XtPhysicsBodyMakeStaticMsg : public ActorMsg::C_XtGenericMessage
{
public:
	C_XtPhysicsBodyMakeStaticMsg() : C_XtGenericMessage(E_PhysicsBodyMsg_MakeStatic) {}
};


class C_XtPhysicsBodyMakeDynamicMsg : public ActorMsg::C_XtGenericMessage
{
public:
	C_XtPhysicsBodyMakeDynamicMsg() : C_XtGenericMessage(E_PhysicsBodyMsg_MakeDynamic) {}
};


class C_XtPhysicsBodyMakeAnimatedMsg : public ActorMsg::C_XtGenericMessage
{
public:
	C_XtPhysicsBodyMakeAnimatedMsg() : C_XtGenericMessage(E_PhysicsBodyMsg_MakeAnimated) {}
};


class C_XtPhysicsMakeStaticOnSettled : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtPhysicsMakeStaticOnSettled (){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		assert(pActorContext);
		m_Msg.SetValue(m_MakeStaticOnSettled ? m_MakeStaticOnSettled->GetBool(pActorContext) : true);

		return &m_Msg;
	}

	Actors::C_XtParamBool* m_MakeStaticOnSettled;
private:
	C_PhysicsMsgMakeStaticOnSettled m_Msg;

};


XT_TYPE(C_XtPhysicsApplyImpulseMsg, "PhysicsApplyImpulse")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_Position, "Position")
XT_FIELD(m_Impulse, "Impulse")
XT_FIELD(m_LocalImpulse, "LocalImpulse")
XT_END_TYPE

XT_TYPE(C_XtPhysicsBodyMakeStaticMsg, "PhysicsMakeStatic")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(C_XtPhysicsBodyMakeDynamicMsg, "PhysicsMakeDynamic")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(C_XtPhysicsBodyMakeAnimatedMsg, "PhysicsMakeAnimated")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(C_XtPhysicsMakeStaticOnSettled, "PhysicsMakeStaticOnSettled")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_MakeStaticOnSettled, "MakeStaticOnSettled")
XT_END_TYPE

XT_LIBRARY(PhysicsMessages)
XT_REGISTER_TYPE(C_XtPhysicsApplyImpulseMsg)
XT_REGISTER_TYPE(C_XtPhysicsBodyMakeStaticMsg)
XT_REGISTER_TYPE(C_XtPhysicsBodyMakeDynamicMsg)
XT_REGISTER_TYPE(C_XtPhysicsBodyMakeAnimatedMsg)
XT_REGISTER_TYPE(C_XtPhysicsMakeStaticOnSettled)
XT_END_LIBRARY


