#ifndef MERCURY_MESSAGE_ACTORS_H
#define MERCURY_MESSAGE_ACTORS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/SharedActorBase.h>
#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/ActorCreation/MessageActorsCreate.h>

namespace Actors
{

// Message Script Actor
class C_MessageScript : public C_SharedActorBase
{
public:
	C_MessageScript(){}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_MessageScript; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "MessageScript"; }

	const C_MessageScriptCreate*	GetCreationData() const {return static_cast<const C_MessageScriptCreate*>(Actor::C_Actor::GetCreationData().GetData());}

protected:
	virtual bool					InitialiseImpl(); // from Actor::C_Actor

};

// Message Script Actor
class C_MessageForwarderList : public C_SharedActorBase
{
public:
	C_MessageForwarderList(){}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_MessageForwarderList; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "MessageForwarderList"; }

	const C_MessageForwarderListCreate*	GetCreationData() const {return static_cast<const C_MessageForwarderListCreate*>(Actor::C_Actor::GetCreationData().GetData());}

protected:
	virtual bool					InitialiseImpl(); // from Actor::C_Actor

};
// Counter Actor
class C_Counter : public C_SharedActorBase
{
public:
	C_Counter(){}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_Counter; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "Counter"; }

	const C_CounterCreate*	GetCreationData() const {return static_cast<const C_CounterCreate*>(Actor::C_Actor::GetCreationData().GetData());}

protected:
	virtual bool			InitialiseImpl(); // from Actor::C_Actor

};

// Countdown Timer Actor
class C_CountdownTimer : public C_SharedActorBase
{
public:
	C_CountdownTimer(){}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_CountdownTimer; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "CountdownTimer"; }

	const C_CountdownTimerCreate*	GetCreationData() const {return static_cast<const C_CountdownTimerCreate*>(Actor::C_Actor::GetCreationData().GetData());}

protected:
	virtual bool					InitialiseImpl(); // from Actor::C_Actor

};


// Message Queue Actor
class C_MessageQueue : public C_SharedActorBase
{
public:
	C_MessageQueue(){}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_MessageQueue; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "MessageQueue"; }

	const C_MessageQueueCreate* GetCreationData() const {return static_cast<const C_MessageQueueCreate*>(Actor::C_Actor::GetCreationData().GetData());}

protected:
	bool InitialiseImpl();

private:
};


// Message Queue Actor
class C_LogicGateMessageSender : public C_SharedActorBase
{
public:
	C_LogicGateMessageSender(){}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_LogicGateMessageSender; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "LogicGateMessageSender"; }

	const C_LogicGateMessageSenderCreate* GetCreationData() const {return static_cast<const C_LogicGateMessageSenderCreate*>(Actor::C_Actor::GetCreationData().GetData());}

	bool					OnMessage(const Messaging::C_MessageBase* pMessage);

protected:
	bool InitialiseImpl();

private:
};


// Bool Expression Message Forwarder
class C_BoolExpressionMessageForwarder : public C_SharedActorBase
{
public:
	C_BoolExpressionMessageForwarder();

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_BoolExpressionMessageForwarder; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "BoolExpressionMessageForwarder"; }

	const C_BoolExpressionMessageForwarderCreate* GetCreationData() const {return static_cast<const C_BoolExpressionMessageForwarderCreate*>(Actor::C_Actor::GetCreationData().GetData());}

	bool					OnMessage(const Messaging::C_MessageBase* pMessage);

protected:
	bool InitialiseImpl();

private:
	U8 *		m_pMessageBuffer;		///< Buffer to store message to be forwarded
};

// Bool Expression Message Sender
class C_BoolExpressionMessageSender : public C_SharedActorBase
{
public:
	C_BoolExpressionMessageSender(){}

	static const Actor::C_ActorTypeId		TypeId()	{ return E_ActorTypeId_BoolExpressionMessageSender; }
	static const Actor::C_ActorTypeName		TypeName()	{ return "BoolExpressionMessageSender"; }

	const C_BoolExpressionMessageSenderCreate* GetCreationData() const {return static_cast<const C_BoolExpressionMessageSenderCreate*>(Actor::C_Actor::GetCreationData().GetData());}

	bool					OnMessage(const Messaging::C_MessageBase* pMessage);

protected:
	bool InitialiseImpl();

private:
};

} //namespace Actors

#endif
