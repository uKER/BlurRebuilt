#include "SharedActorsPch.h"

#include <shared_actors/XtActorMessages.h>

#include <actor_system/Actor.h>
#include <actor_system/ActorDebug.h>
#include <actor_system/ActorList.h>
#include <display.h>
#include <framework/application/ApplicationBase.h>
#include <framework/timer/Timer.h>
#include <misc_shared/Messaging/GameMessaging.h>
#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/Components/Messages/MovementMessages.h>

Debug::Bool	gLogActorStudioMessages("ActorDebug.LogActorStudioMessages", false);
Debug::String	gLogActorStudioMessageFilter("ActorDebug.LogActorStudioMessageFilter", "");


XT_ABSTRACT_TYPE(ActorMsg::C_XtActorMessageBase, "ActorMessageBase")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtSendMessage, "SendMessage")
	XT_FIELD(m_DestinationActor, "Actor")
	//XT_FIELD(m_bBroadcast, "Broadcast")
	XT_FIELD(m_bDisable, "Disable")
	XT_FIELD(m_bDebugBreak, "DebugBreak")
	XT_FIELD(m_Message, "Message")
XT_END_TYPE

// class for holding a movement message to send in Xt
XT_TYPE(ActorMsg::C_XtMovementSendMessage, "MovementSendMessage")
XT_FIELD(m_DestinationActor, "Actor")
XT_FIELD(m_Message, "Message")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDelayedSendMessage, "DelayedSendMessage")
	XT_FIELD(m_Message, "Message")
	XT_FIELD(m_Delay, "Delay")
XT_END_TYPE

XT_TYPE(ActorMsg::C_XtDelayedSendMessageList, "DelayedSendMessageList")
	XT_FIELD(m_MessageList, "MessageList")
XT_END_TYPE


namespace ActorMsg
{

C_XtActorMessageBase::C_XtActorMessageBase() 
{
}

C_XtActorMessageBase::~C_XtActorMessageBase()
{
}

C_XtGenericMessage::C_XtGenericMessage(const Messaging::TMsgType msgType) : 
	m_Msg(msgType, sizeof(m_Msg)) 
{
}

const Messaging::C_MessageBase* C_XtGenericMessage::GetMessage(const Actor::C_Actor* pActorContext) 
{
	return &m_Msg;
}


bool	LogActorStudioMessage(const Actor::C_Actor *pSource, const Actor::C_Actor *pDest, const Messaging::C_MessageBase *pMessage)
{
#ifndef CFG_SPEED
	const Messaging::C_MessageDebugInfo* pDebugInfo = pMessage->GetDebugInfo();

	if (pDebugInfo != NULL)
	{
		const F32 time = Framework::C_ApplicationBase::GetInstance()->GetTimer().GetElapsedTime();

		if (pDest)
		{
			Util::C_String logText;
			Util::C_String logFilter(gLogActorStudioMessageFilter);

			logText.Printf("%s --> %s (%s) time=%8.2f",pSource->GetActorName(),pDest->GetActorName(),pDebugInfo->m_MsgName,time);

			if((logFilter.Length() == 0) || (logText.Search(logFilter) >= 0))
			{
				LOGINFO(Actor::GetDebugLogLevelDesign(pSource),logText.Chars());
			}
			return true;
		}
		else
			LOGERROR(Actor::GetDebugLogLevelDesign(pSource), DBSTR"NULL msg destination actor ('%s' from '%s') time=%8.2f", pDebugInfo->m_MsgName, pSource->GetActorName(), time);

	}
#endif
	return false;
}

// send the Xt Defined message to it's destination Actor
bool C_XtSendMessage::Send(const Actor::C_Actor *pActorContext) const
{
	if(m_bDisable)
		return false;
	BIZ_ASSERT(pActorContext);
	if(m_Message && m_DestinationActor)
	{
		const Messaging::C_MessageBase *pMessage = m_Message->GetMessage(pActorContext);
#ifndef CFG_SPEED
		if(gLogActorStudioMessages)
		{
			const Actor::C_Actor *pDestActor = pActorContext->GetActorList()->GetActorPtr(m_DestinationActor->GetActorHandle(pActorContext));
			LogActorStudioMessage(pActorContext, pDestActor, pMessage);
		}
#endif
// 		if(m_bBroadcast)
// 		{
// 			pActorContext->GetActorList()->BroadcastMessage(pMessage);
// 			return true;
// 		}
// 		else 

		Actor::E_ActorMessageSendResult res = pActorContext->GetActorList()->SendMessageToActor(m_DestinationActor->GetActorHandle(pActorContext),pMessage); 
			
		if(m_bDebugBreak)
			pActorContext->GetActorList()->BroadcastMessage(GameMessaging::E_GameMsg_DebugBreak);

		return res == Actor::E_ActorMessageSendResult_OK;
	}

	return false;
}

// send the Xt Defined message to it's destination Actor - with delay
bool C_XtSendMessage::SendDelayed(const Actor::C_Actor *pActorContext, F32 delay) const
{
	if(m_bDisable)
		return false;
	BIZ_ASSERT(pActorContext);
	if(m_DestinationActor && m_Message)	// if it is a valid message
	{
		const Messaging::C_MessageBase *pMessage = m_Message->GetMessage(pActorContext);

#ifndef CFG_SPEED
		if(gLogActorStudioMessages)
		{
			const Actor::C_Actor *pDestActor = pActorContext->GetActorList()->GetActorPtr(m_DestinationActor->GetActorHandle(pActorContext));
			LogActorStudioMessage(pActorContext, pDestActor, pMessage);
		}
#endif
		if(m_bDebugBreak)
			pActorContext->GetActorList()->BroadcastMessage(GameMessaging::E_GameMsg_DebugBreak);

		pActorContext->GetActorList()->DelayMessageToActorBySeconds(delay, m_DestinationActor->GetActorHandle(pActorContext),pMessage);
		return true;
	}
	else
		return false;
}

// send the Xt Defined message to it's destination Actor
bool C_XtMovementSendMessage::Send(const Actor::C_Actor *pActorContext) const
{
	BIZ_ASSERT(pActorContext);
	if(m_DestinationActor && m_Message)	// if it is a valid message
	{
		const Messaging::C_MessageBase *pMessage = m_Message->GetMessage(pActorContext);
#ifndef CFG_SPEED
		if(gLogActorStudioMessages)
		{
			const Actor::C_Actor *pDestActor = pActorContext->GetActorList()->GetActorPtr(m_DestinationActor->GetActorHandle(pActorContext));
			LogActorStudioMessage(pActorContext, pDestActor, pMessage);
		}
#endif
		return pActorContext->GetActorList()->SendMessageToActor(m_DestinationActor->GetActorHandle(pActorContext),pMessage) == Actor::E_ActorMessageSendResult_OK;
	}
	else
		return false;
}

// send the Xt Defined message to it's destination Actor - with delay
bool C_XtMovementSendMessage::SendDelayed(const Actor::C_Actor *pActorContext, F32 delay) const
{
	BIZ_ASSERT(pActorContext);
	if(m_DestinationActor && m_Message)	// if it is a valid message
	{
		const Messaging::C_MessageBase *pMessage = m_Message->GetMessage(pActorContext);

#ifndef CFG_SPEED
		if(gLogActorStudioMessages)
		{
			const Actor::C_Actor *pDestActor = pActorContext->GetActorList()->GetActorPtr(m_DestinationActor->GetActorHandle(pActorContext));
			LogActorStudioMessage(pActorContext, pDestActor, pMessage);
		}
#endif
		pActorContext->GetActorList()->DelayMessageToActorBySeconds(delay, m_DestinationActor->GetActorHandle(pActorContext),pMessage);
		return true;
	}
	else
		return false;
}

// Delayed SendMessage List
bool C_XtDelayedSendMessageList::Send(const Actor::C_Actor *pActorContext) const
{
	bool noComplications = true;

	for(S32 messageNo = 0; messageNo<m_MessageList.count; messageNo++)
	{	
		noComplications &= m_MessageList[messageNo].Send(pActorContext);
	}

	return noComplications;
}

} // namespace ActorMsg
