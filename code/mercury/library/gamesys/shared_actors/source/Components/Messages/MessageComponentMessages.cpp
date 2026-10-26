#include "SharedActorsPch.h"

#include <shared_actors/Components/Messages/MessageComponentMessages.h>
#include <shared_actors/Components/Creation/MessageComponentsCreate.h>

#include <shared_actors/XtSharedLevelActorParams.h>


#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif

namespace ActorMsg
{

class C_XtMessageForwarderSetSlotMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		if(m_SlotNo)
			m_Msg.SetValue(m_SlotNo->GetInteger(pActorContext));
	
		return &m_Msg;
	}

	Actors::C_XtParamInteger *	m_SlotNo;

	C_MessageForwarderSetSlot	m_Msg;
};

class C_XtMessageForwarderIncrementSlotMsg : public ActorMsg::C_XtGenericMessage
{
public:
	C_XtMessageForwarderIncrementSlotMsg() : C_XtGenericMessage(E_MessageForwarderMsg_IncrementSlot) {}
};


class C_XtMessageForwarderDecrementSlotMsg : public ActorMsg::C_XtGenericMessage
{
public:
	C_XtMessageForwarderDecrementSlotMsg() : C_XtGenericMessage(E_MessageForwarderMsg_IncrementSlot) {}
};


class C_XtMessageForwarderSetSelectionMsg : public ActorMsg::C_XtActorMessageBase
{
public:
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)
	{
		m_Msg.SetValue((U32)m_SelectionType);
		return &m_Msg;
	}

	Components::E_MessageForwarderListSelection	m_SelectionType;

	C_MessageForwarderSetSlot	m_Msg;
};

}

XT_TYPE(ActorMsg::C_XtMessageForwarderSetSlotMsg, "MessageForwarderListSetSlotMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_SlotNo, "SlotNo")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtMessageForwarderIncrementSlotMsg, "MessageForwarderListIncSlotMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtMessageForwarderDecrementSlotMsg, "MessageForwarderListDecSlotMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtMessageForwarderSetSelectionMsg, "MessageForwarderListSetSelectionMsg")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_SelectionType, "SelectionType")
XT_END_TYPE


XT_LIBRARY(MessageComponentMessages)
XT_REGISTER_TYPE(ActorMsg::C_XtMessageForwarderSetSlotMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtMessageForwarderIncrementSlotMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtMessageForwarderDecrementSlotMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtMessageForwarderSetSelectionMsg)
XT_END_LIBRARY

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif