#include "SharedActorsPch.h"

#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/Components/Messages/ElementManagerMessages.h>


// Xt Messaging
class C_XtElementMgrSetElementIndestructibleMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtElementMgrSetElementIndestructibleMsg() {}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		assert(pActorContext);
		m_Msg = C_ElementMgrSetElementIndestructibleMsg(m_ElementName.Text(), m_Set);
		return &m_Msg;
	}

	XtSz8 m_ElementName;
	bool m_Set;

	C_ElementMgrSetElementIndestructibleMsg m_Msg;
};


class C_XtElementMgrSetAllElementsIndestructibleMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		m_Msg.SetValue(m_Set);
		return &m_Msg;
	}

	bool m_Set;

private:
	C_ElementMgrSetAllElementsIndestructibleMsg m_Msg;
};


class C_XtElementMgrBreakNamedElementMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtElementMgrBreakNamedElementMsg() {}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		assert(pActorContext);
		m_Msg = C_ElementMgrBreakNamedElementMsg(m_ElementName.Text(), m_RespectIndestructible);
		return &m_Msg;
	}

	XtSz8 m_ElementName;
	bool m_RespectIndestructible;

	C_ElementMgrBreakNamedElementMsg m_Msg;
};


class C_XtElementMgrDestroyNamedElementMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtElementMgrDestroyNamedElementMsg() {}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		assert(pActorContext);
		m_Msg = C_ElementMgrDestroyNamedElementMsg(m_ElementName.Text(), m_RespectIndestructible);
		return &m_Msg;
	}

	XtSz8 m_ElementName;
	bool m_RespectIndestructible;

	C_ElementMgrDestroyNamedElementMsg m_Msg;
};


class C_XtElementMgrDetachElementMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtElementMgrDetachElementMsg() : m_Impulse(NULL) {}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		BIZ_ASSERT(pActorContext);
		m_Msg = C_ElementMgrDetachElementMsg(m_ElementName.Text(), m_Impulse ? Maths::HVec3(m_Impulse->GetVector(pActorContext)) : Maths::HVec3());
		return &m_Msg;
	}

	XtSz8 m_ElementName;
	Actors::C_XtParamVector* m_Impulse;
	C_ElementMgrDetachElementMsg m_Msg;
};


class C_XtElementChangeElementDamageLevelMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtElementChangeElementDamageLevelMsg() {}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		m_Msg = C_ElementMgrChangeElementDamageLevelMsg(m_ElementName.Text(), m_DamageLevel);
		return &m_Msg;
	}

	XtSz8 m_ElementName;
	S32 m_DamageLevel;

	C_ElementMgrChangeElementDamageLevelMsg m_Msg;
};


class C_XtScriptedPivotUpdateNamedElementMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	C_XtScriptedPivotUpdateNamedElementMsg(){}
	const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);   

	XtSz8 m_ElementName;
	Actors::C_XtParamVector *				m_Rotation;
	F32										m_Time;

	C_ElementMgrUpdateScriptedPivotMsg	m_Msg;

};

const Messaging::C_MessageBase *	C_XtScriptedPivotUpdateNamedElementMsg::GetMessage(const Actor::C_Actor *pActorContext)   
{ 
	assert(pActorContext);

	m_Msg.m_ElementName = m_ElementName;
	m_Msg.m_Rotation = m_Rotation->GetVector(pActorContext);
	m_Msg.m_Time = m_Time;

	return &m_Msg;
}


XT_TYPE(C_XtElementMgrSetElementIndestructibleMsg, "SetElementIndestructible")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_ElementName, "ElementName")
	XT_FIELD(m_Set, "Set")
XT_END_TYPE

XT_TYPE(C_XtElementMgrSetAllElementsIndestructibleMsg, "SetAllElementsIndestructible")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_Set, "Set")
XT_END_TYPE

XT_TYPE(C_XtElementMgrBreakNamedElementMsg, "BreakNamedElement")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_ElementName, "ElementName")
	XT_FIELD(m_RespectIndestructible, "RespectIndestructible")
XT_END_TYPE

XT_TYPE(C_XtElementMgrDestroyNamedElementMsg, "DestroyNamedElement")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_ElementName, "ElementName")
	XT_FIELD(m_RespectIndestructible, "RespectIndestructible")
XT_END_TYPE

XT_TYPE(C_XtElementMgrDetachElementMsg, "DetachElement")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_ElementName, "ElementName")
	XT_FIELD(m_Impulse, "Impulse")
XT_END_TYPE

XT_TYPE(C_XtElementChangeElementDamageLevelMsg, "ElementChangeDamageLevel")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_ElementName, "ElementName")
	XT_FIELD(m_DamageLevel, "DamageLevel")
XT_END_TYPE

XT_TYPE(C_XtScriptedPivotUpdateNamedElementMsg, "ScriptedPivotUpdateNamedElement")
	XT_BASE(ActorMsg::C_XtActorMessageBase)
	XT_FIELD(m_ElementName, "ElementName")
	XT_FIELD(m_Rotation, "Rotation")
	XT_FIELD(m_Time, "Time")
XT_END_TYPE


XT_LIBRARY(ElementManagerMessages)
	XT_REGISTER_TYPE(C_XtElementMgrSetElementIndestructibleMsg)
	XT_REGISTER_TYPE(C_XtElementMgrSetAllElementsIndestructibleMsg)
	XT_REGISTER_TYPE(C_XtElementMgrBreakNamedElementMsg)
	XT_REGISTER_TYPE(C_XtElementMgrDestroyNamedElementMsg)
	XT_REGISTER_TYPE(C_XtElementMgrDetachElementMsg)
	XT_REGISTER_TYPE(C_XtElementChangeElementDamageLevelMsg)
	XT_REGISTER_TYPE(C_XtScriptedPivotUpdateNamedElementMsg)
XT_END_LIBRARY

