#include "SharedActorsPch.h"

#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/ActorCreation/MessageActorsCreate.h>

XT_TYPE(Actors::C_MessageScriptCreate, "MessageScript")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_SorceryScriptComponent, "ScriptComponent")
XT_END_TYPE

XT_TYPE(Actors::C_MessageForwarderListCreate, "MessageForwarderList")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_ForwarderComponent, "ForwarderListComponent")
XT_END_TYPE

XT_TYPE(Actors::C_CounterCreate, "Counter")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_SorceryCounterComponent, "CounterComponent")
XT_END_TYPE

XT_TYPE(Actors::C_CountdownTimerCreate, "CountdownTimer")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_SorceryCountdownTimerComponent, "CountdownTimerComponent")
XT_END_TYPE

XT_TYPE(Actors::C_MessageQueueCreate, "MovementMessageQueue")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_MessageDest, "MessageDest")
XT_END_TYPE

XT_TYPE(Actors::C_LogicGateMessageSenderCreate, "LogicGateMessageSender")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_SorceryLogicGateComponent, "LogicGateComponent")
XT_FIELD(m_OnTrue, "OnTrue")
XT_FIELD(m_OnFalse, "OnFalse")
XT_END_TYPE

XT_TYPE(Actors::C_BoolExpressionMessageForwarderCreate, "BoolExpressionMessageForwarder")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_SorceryBoolExpressionComponent, "Expression")
XT_FIELD(m_Destination, "Destination")
XT_END_TYPE

XT_TYPE(Actors::C_BoolExpressionMessageSenderCreate, "BoolExpressionMessageSender")
XT_BASE(Actors::C_GameActorCreate)
XT_FIELD(m_SorceryBoolExpressionComponent, "Expression")
XT_FIELD(m_OnTrue, "OnTrue")
XT_FIELD(m_OnFalse, "OnFalse")
XT_END_TYPE

XT_LIBRARY(MercuryMessageActorsCreate)
XT_REGISTER_TYPE(Actors::C_MessageScriptCreate)
XT_REGISTER_TYPE(Actors::C_MessageForwarderListCreate)
XT_REGISTER_TYPE(Actors::C_CounterCreate)
XT_REGISTER_TYPE(Actors::C_CountdownTimerCreate)
XT_REGISTER_TYPE(Actors::C_MessageQueueCreate)
XT_REGISTER_TYPE(Actors::C_LogicGateMessageSenderCreate)
XT_REGISTER_TYPE(Actors::C_BoolExpressionMessageForwarderCreate)
XT_REGISTER_TYPE(Actors::C_BoolExpressionMessageSenderCreate)
XT_END_LIBRARY

namespace Actors
{

// Message Script Actor
void C_MessageScriptCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_SorceryScriptComponent);
}


void C_MessageForwarderListCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_ForwarderComponent);
}

// Counter Actor
void C_CounterCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_SorceryCounterComponent);
}

// Countdown Timer
void C_CountdownTimerCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_SorceryCountdownTimerComponent);
}

// Logic Gate Message Sender
void C_LogicGateMessageSenderCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_SorceryLogicGateComponent);
}

// Bool Expression Message Forwarder
void C_BoolExpressionMessageForwarderCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_SorceryBoolExpressionComponent);
}

// Bool Expression Message Sender
void C_BoolExpressionMessageSenderCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_SorceryBoolExpressionComponent);
}

}//namespace Actors


