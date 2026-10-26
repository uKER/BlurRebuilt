#include "SharedActorsPch.h"

#include <shared_actors/XtSharedActorMessages.h>

#include <actor_system/actor.h>
#include <actor_system/actorlist.h>
#include <actor_system/ActorMessages.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <shared_actors/XtSharedLevelActorParams.h>


namespace ActorMsg
{
// Xt Actor Messaging classes
class C_XtActivateMessage : public C_XtGenericMessage
{
public:
	C_XtActivateMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Activate){}
};

class C_XtDeactivateMessage : public C_XtGenericMessage
{
public:
	C_XtDeactivateMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Deactivate){}
};

class C_XtEnableMessage : public C_XtGenericMessage
{
public:
	C_XtEnableMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Enable){}
};

class C_XtDisableMessage : public C_XtGenericMessage
{
public:
	C_XtDisableMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Disable){}
};

class C_XtResetMessage : public C_XtGenericMessage
{
public:
	C_XtResetMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Reset){}
};

class C_XtDeleteMessage : public C_XtGenericMessage
{
public:
	C_XtDeleteMessage():C_XtGenericMessage(Actor::E_ActorMsg_Kill){}
};

class C_XtOpenMessage : public C_XtGenericMessage
{
public:
	C_XtOpenMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Open){}
};

class C_XtCloseMessage : public C_XtGenericMessage
{
public:
	C_XtCloseMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Close){}
};

class C_XtIncrementMessage : public C_XtGenericMessage
{
public:
	C_XtIncrementMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Increment){}
};

class C_XtDecrementMessage : public C_XtGenericMessage
{
public:
	C_XtDecrementMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_Decrement){}
};

class C_XtFlipFlopMessage : public C_XtGenericMessage
{
public:
	C_XtFlipFlopMessage():C_XtGenericMessage(ActorMsg::E_GameActorMsg_FlipFlop){}
};

class C_XtActionBeginMessage : public C_XtActorMessageBase
{
public:
	::Actors::C_XtParamActor*			m_pActorToNotifyOnCompletion;
	ActorMsg::C_ActionBeginMsg		m_Msg;

	C_XtActionBeginMessage();

	// from C_XtActorMessageBase
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext);
};

C_XtActionBeginMessage::C_XtActionBeginMessage() : 
m_pActorToNotifyOnCompletion(NULL)
{
	// intentionally blank
}

const Messaging::C_MessageBase* C_XtActionBeginMessage::GetMessage(const Actor::C_Actor* pActorContext)
{
	if (m_pActorToNotifyOnCompletion != NULL)
		m_Msg.SetActorHandle(m_pActorToNotifyOnCompletion->GetActorHandle(pActorContext));
	// 	else
	// 		LOGERROR(Actors::g_Log, "no actor specified to notify on completion of ActionBegin");

	return &m_Msg;
}

class C_XtActionCompletedMessage : public C_XtActorMessageBase
{
public:
	::Actors::C_XtParamActor*			m_pActorThatCompletedAction;
	ActorMsg::C_ActionCompletedMsg	m_Msg;

	C_XtActionCompletedMessage();

	// from C_XtActorMessageBase
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext);
};

C_XtActionCompletedMessage::C_XtActionCompletedMessage() :
m_pActorThatCompletedAction(NULL)
{
	// intentionally blank
}

const Messaging::C_MessageBase* C_XtActionCompletedMessage::GetMessage(const Actor::C_Actor* pActorContext)
{
	if (m_pActorThatCompletedAction != NULL)
		m_Msg.SetActorHandle(m_pActorThatCompletedAction->GetActorHandle(pActorContext));
	// 	else
	// 		LOGERROR(Actors::g_Log, "no actor specified as source of ActionCompleted");

	return &m_Msg;
}

class C_XtControlStartedMessage : public C_XtActorMessageBase
{
public:
	::Actors::C_XtParamActor*			m_pControllingActor;
	C_ControlStartedMsg		m_Msg;

	C_XtControlStartedMessage();

	// from C_XtActorMessageBase
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext);
};

C_XtControlStartedMessage::C_XtControlStartedMessage() : 
m_pControllingActor(NULL)
{
	// intentionally blank
}

const Messaging::C_MessageBase* C_XtControlStartedMessage::GetMessage(const Actor::C_Actor* pActorContext)
{
	if (m_pControllingActor != NULL)
		m_Msg.SetActorHandle(m_pControllingActor->GetActorHandle(pActorContext));
	// 	else
	// 		LOGERROR(Actors::g_Log, "no actor specified to notify on completion of ActionBegin");

	return &m_Msg;
}

class C_XtControlEndedMessage : public C_XtActorMessageBase
{
public:
	::Actors::C_XtParamActor*			m_pControllingActor;
	C_ControlEndedMsg	m_Msg;

	C_XtControlEndedMessage();

	// from C_XtActorMessageBase
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext);
};

C_XtControlEndedMessage::C_XtControlEndedMessage() : 
m_pControllingActor(NULL)
{
	// intentionally blank
}

const Messaging::C_MessageBase* C_XtControlEndedMessage::GetMessage(const Actor::C_Actor* pActorContext)
{
	if (m_pControllingActor != NULL)
		m_Msg.SetActorHandle(m_pControllingActor->GetActorHandle(pActorContext));
	// 	else
	// 		LOGERROR(Actors::g_Log, "no actor specified to notify on completion of ActionBegin");

	return &m_Msg;
}

class C_XtTrackActorMsg : public C_XtActorMessageBase
{
public:
	C_XtTrackActorMsg(){}
	virtual const Messaging::C_MessageBase *	GetMessage(const Actor::C_Actor *pActorContext);

	::Actors::C_XtParamActor	*		m_pActor;
	C_TrackingActorMsg				m_Msg;
};

const Messaging::C_MessageBase *	C_XtTrackActorMsg::GetMessage(const Actor::C_Actor *pActorContext) 
{ 
	m_Msg.SetActorHandle(m_pActor->GetActorHandle(pActorContext));
	return &m_Msg;
}

class C_XtActorDebugRenderOnMsg : public C_XtGenericMessage
{
public:
	C_XtActorDebugRenderOnMsg():C_XtGenericMessage(GameMessaging::E_GameMsg_DebugRenderOn){}
};

class C_XtActorDebugRenderOffMsg : public C_XtGenericMessage
{
public:
	C_XtActorDebugRenderOffMsg():C_XtGenericMessage(GameMessaging::E_GameMsg_DebugRenderOff){}
};

class C_XtSetProxyActor : public C_XtActorMessageBase
{
public:
	virtual const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor* pActorContext)
	{
		if(m_pActorToProxy != NULL)
			m_Msg.SetActorHandle(m_pActorToProxy->GetActorHandle(pActorContext));
		//else
		//	LOGERROR(Actors::g_Log, "No actor specified in message 'SetActorProxy'");

		return &m_Msg;
	}

	::Actors::C_XtParamActor*			m_pActorToProxy;
	C_SetProxyActorMsg	m_Msg;
};

// Disable Slow Motion
class C_XtDisableSlowMotion :  public C_XtActorMessageBase
{
public:
	C_XtDisableSlowMotion(){}
	const Messaging::C_MessageBase* GetMessage(const Actor::C_Actor *pActorContext)  
	{
		assert(pActorContext);
		m_Msg.SetValue(m_Disable ? m_Disable->GetBool(pActorContext) : true);

		return &m_Msg;
	}

	Actors::C_XtParamBool *					m_Disable;

	C_DisableSlowMotionMsg					m_Msg;
};


bool C_XtTriggerMessageEvent::Activate(const Actor::C_Actor *pActor, Actor::C_ActorHandle triggerActor) const
{
	bool noComplications = true;

	noComplications &= m_MessageList.Send(pActor);

	// send a message to the actor that triggered it
	if(m_TriggerActorMessage)
		noComplications &= pActor->GetActorList()->SendMessageToActor(triggerActor,m_TriggerActorMessage->GetMessage(pActor)) == Actor::E_ActorMessageSendResult_OK;

	return noComplications;
}

} // namespace ActorMsg

// Message Sending classes
XT_TYPE(ActorMsg::C_XtActivateMessage, "ActivateMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDeactivateMessage, "DeactivateMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtEnableMessage, "EnableMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDisableMessage, "DisableMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtResetMessage, "ResetMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDeleteMessage, "DeleteMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtOpenMessage, "OpenMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtCloseMessage, "CloseMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtIncrementMessage, "IncrementMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDecrementMessage, "DecrementMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtFlipFlopMessage, "FlipFlopMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActionBeginMessage, "ActionBegin")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pActorToNotifyOnCompletion, "ActorToNotifyOnCompletion")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActionCompletedMessage, "ActionCompleted")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pActorThatCompletedAction, "ActorThatCompletedAction")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtControlStartedMessage, "ControlStarted")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pControllingActor, "ControllingActor")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtControlEndedMessage, "ControlEnded")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pControllingActor, "ControllingActor")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtTrackActorMsg, "TrackActorMessage")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pActor, "Actor")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorDebugRenderOnMsg, "DebugRenderOn")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtActorDebugRenderOffMsg, "DebugRenderOff")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSetProxyActor, "SetProxyActor")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_pActorToProxy, "Actor")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDisableSlowMotion, "DisableSlowMotion")
XT_BASE(ActorMsg::C_XtActorMessageBase)
XT_FIELD(m_Disable, "Disable")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtTriggerMessageEvent, "TriggerMessageEvent")
XT_FIELD(m_MessageList, "MessageList")
XT_FIELD(m_TriggerActorMessage, "TriggerActorMessage")
XT_END_TYPE


XT_LIBRARY(XtSharedActorMessages)
XT_DEPENDS_ON(GroupActorMessages)
XT_DEPENDS_ON(MovementMessages)
XT_DEPENDS_ON(AnimatedObjectMessages)
XT_DEPENDS_ON(ConstraintActorMessages)
XT_REGISTER_TYPE(ActorMsg::C_XtActorMessageBase)
XT_REGISTER_TYPE(ActorMsg::C_XtActivateMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtDeactivateMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtEnableMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtDisableMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtResetMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtDeleteMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtOpenMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtCloseMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtIncrementMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtDecrementMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtFlipFlopMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtActionBeginMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtActionCompletedMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtControlStartedMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtControlEndedMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtTrackActorMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorDebugRenderOnMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtActorDebugRenderOffMsg)
XT_REGISTER_TYPE(ActorMsg::C_XtSetProxyActor)
XT_REGISTER_TYPE(ActorMsg::C_XtDisableSlowMotion)

XT_REGISTER_TYPE(ActorMsg::C_XtSendMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtMovementSendMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtDelayedSendMessage)
XT_REGISTER_TYPE(ActorMsg::C_XtDelayedSendMessageList)
XT_REGISTER_TYPE(ActorMsg::C_XtTriggerMessageEvent)

XT_END_LIBRARY

