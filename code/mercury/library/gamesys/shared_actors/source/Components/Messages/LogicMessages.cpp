#include "SharedActorsPch.h"

#include <actor_system/ActorCreation.h>
#include <shared_actors/Components/Messages/LogicMessages.h>
#include <shared_actors/Components/Creation/LogicComponentsCreate.h>

#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorMessages.h>

#include <shared_actors/XtSharedLevelActorParams.h>


#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

namespace ActorMsg
{

//E_LogicMsg_SetInputTrue,
class C_XtLogicSetInputTrueMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		if(pActorContext->GetCreationData().GetData())
			m_Msg.SetValue(pActorContext->GetCreationData().GetData()->GetSorceryUid());

		return &m_Msg;
	}

	C_LogicSetInputTrue	m_Msg;
};

//E_LogicMsg_SetInputFalse,
class C_XtLogicSetInputFalseMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		if(pActorContext->GetCreationData().GetData())
			m_Msg.SetValue(pActorContext->GetCreationData().GetData()->GetSorceryUid());

		return &m_Msg;
	}

	C_LogicSetInputFalse	m_Msg;
};

//E_LogicMsg_SetInputActor,
class C_XtLogicSetInputActorMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		m_Msg.SetFirst(m_SlotNo);
		if(m_InputActor)
			m_Msg.SetSecond(m_InputActor->GetSorceryUid());

		return &m_Msg;
	}

	C_LogicSetInputActor	m_Msg;

	U32							m_SlotNo;
	Actors::C_GameActorCreate *	m_InputActor;
};

//E_LogicMsg_SetGateType,
class C_XtLogicSetGateTypeMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		m_Msg.SetValue(m_GateType);

		return &m_Msg;
	}

	C_LogicSetGateType	m_Msg;

	Components::E_LogicGateType		m_GateType;
};

}//namespace ActorMsg


XT_TYPE(ActorMsg::C_XtLogicSetInputTrueMsg, "LogicSetInputTrue")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtLogicSetInputFalseMsg, "LogicSetInputFalse")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtLogicSetInputActorMsg, "LogicSetInputActor")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_SlotNo, "SlotNo")
XT_FIELD(m_InputActor, "InputActor")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtLogicSetGateTypeMsg, "LogicSetGateType")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_GateType, "GateType")
XT_END_TYPE

XT_LIBRARY(LogicComponentMessages)
XT_REGISTER_TYPE(ActorMsg::C_XtLogicSetInputTrueMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtLogicSetInputFalseMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtLogicSetInputActorMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtLogicSetGateTypeMsg)
XT_END_LIBRARY


#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif

