#ifndef XTACTORMESSAGES_65FD99CE_2B55_4BD0_94C8_92FD829118E5_H
#define XTACTORMESSAGES_65FD99CE_2B55_4BD0_94C8_92FD829118E5_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <misc_shared/messaging/MessageBase.h>
#include <xt/XtGame.h>


namespace Actors 
{
	class C_XtParamActor; 
	class C_XtParamBool;
	class C_XtParamFloat;
	class C_XtParamInteger;
	class C_XtParamOrientation;
	class C_XtParamPosition;
	class C_XtParamVector;
}


namespace ActorMsg
{
// forward decs
class C_XtActorMovementMsgBase;

/// Base class for actor messages defined with Xt.
class C_XtActorMessageBase
{
public:
	C_XtActorMessageBase();
	virtual ~C_XtActorMessageBase();

	/**
	@param pActorContext actor for which the message is created.
	@returns weak pointer to the message.
	*/
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext) = 0;

};

/// Implements C_XtActorMessageBase for messages with no additional data.
class C_XtGenericMessage : public C_XtActorMessageBase
{
public:
	Messaging::C_MessageBase m_Msg;

	explicit C_XtGenericMessage(const Messaging::TMsgType msgType);

	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext);

};

// class for holding a message to send in Xt
class C_XtSendMessage
{
public:
	bool			Send(const Actor::C_Actor *pActorContext) const;
	bool			SendDelayed(const Actor::C_Actor *pActorContext, F32 delay) const;

	::Actors::C_XtParamActor *		m_DestinationActor;
	//bool					m_bBroadcast;
	bool					m_bDisable;
	bool					m_bDebugBreak;
	C_XtActorMessageBase *	m_Message;
};

class C_XtMovementSendMessage
{
public:
	bool			Send(const Actor::C_Actor *pActorContext) const;
	bool			SendDelayed(const Actor::C_Actor *pActorContext, F32 delay) const;

	::Actors::C_XtParamActor *		m_DestinationActor;
	C_XtActorMovementMsgBase *	m_Message;
};

class C_XtDelayedSendMessage
{
public:
	bool			Send(const Actor::C_Actor *pActorContext) const {return (m_Delay == 0.0f) ? m_Message.Send(pActorContext) : m_Message.SendDelayed(pActorContext, m_Delay);}

	C_XtSendMessage		m_Message;
	F32					m_Delay;
};

class C_XtDelayedSendMessageList 
{
public:
	bool			Send(const Actor::C_Actor *pActorContext) const;

	XtArray<C_XtDelayedSendMessage>	m_MessageList;
};

} // namespace ActorMsg


#endif // XTACTORMESSAGES_65FD99CE_2B55_4BD0_94C8_92FD829118E5_H
