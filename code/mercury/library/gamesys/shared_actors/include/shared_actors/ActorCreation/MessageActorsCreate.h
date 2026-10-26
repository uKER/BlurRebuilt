#ifndef MERCURY_MESSAGE_ACTORS_CREATE_H
#define MERCURY_MESSAGE_ACTORS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/XtActorBase.h>
#include <shared_actors/XtActorMessages.h>
#include <shared_actors/Components/Creation/LogicComponentsCreate.h>
#include <shared_actors/Components/Creation/MessageComponentsCreate.h>

namespace Actors
{

// Message Script Actor
struct C_MessageScriptCreate : public C_GameActorCreate
{
	const Actor::C_ActorTypeId					GetType() const {return E_ActorTypeId_MessageScript;}

	Components::C_MessageScriptCreator	m_SorceryScriptComponent;

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};

// Message Forwarder List Actor
struct C_MessageForwarderListCreate : public C_GameActorCreate
{
	const Actor::C_ActorTypeId					GetType() const {return E_ActorTypeId_MessageForwarderList;}

	Components::C_MessageForwarderListCreator	m_ForwarderComponent;

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};

// Counter Actor
struct C_CounterCreate : public C_GameActorCreate
{
	const Actor::C_ActorTypeId					GetType() const {return E_ActorTypeId_Counter;}

	Components::C_CounterComponentCreator		m_SorceryCounterComponent;

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};

// Countdown Timer Actor
struct C_CountdownTimerCreate : public C_GameActorCreate
{
	const Actor::C_ActorTypeId					GetType() const {return E_ActorTypeId_CountdownTimer;}

	Components::C_CountdownTimerCreator		m_SorceryCountdownTimerComponent;

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};


// Message Queue Actor
struct C_MessageQueueCreate : public C_GameActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_MessageQueue;}

	// from Actor::C_ActorCreationData
	//virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);

	Actors::C_GameActorCreate* m_MessageDest;
};

// Logic Gate Message Sender
struct C_LogicGateMessageSenderCreate : public C_GameActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_LogicGateMessageSender;}

	Components::C_LogicGateComponentCreator		m_SorceryLogicGateComponent;
	ActorMsg::C_XtDelayedSendMessageList		m_OnTrue;
	ActorMsg::C_XtDelayedSendMessageList		m_OnFalse;

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};

// Bool Expression Message Forwarder
struct C_BoolExpressionMessageForwarderCreate : public C_GameActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_BoolExpressionMessageForwarder;}

	Components::C_BoolExpressionEvaluatorCreator	m_SorceryBoolExpressionComponent;
	C_GameActorCreate *								m_Destination;

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};

// Bool Expression Message Sender
struct C_BoolExpressionMessageSenderCreate : public C_GameActorCreate
{
	const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_BoolExpressionMessageSender;}

	Components::C_BoolExpressionEvaluatorCreator		m_SorceryBoolExpressionComponent;
	ActorMsg::C_XtDelayedSendMessageList		m_OnTrue;
	ActorMsg::C_XtDelayedSendMessageList		m_OnFalse;

	// from Actor::C_ActorCreationData
	virtual void DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn);
};

} // namespace Actors

#endif
