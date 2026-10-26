#ifndef XT_SHARED_ACTORMESSAGES_H
#define XT_SHARED_ACTORMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/components/messages/TrackingMessages.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/XtActorMessages.h>
#include <xt/XtGame.h>
#include <core/xtstdx.h>


namespace ActorMsg
{


// Send Message Trigger Action
class C_XtTriggerMessageEvent
{
public:
	bool		Activate(const Actor::C_Actor *pActor, Actor::C_ActorHandle triggerActor) const;
	ActorMsg::C_XtDelayedSendMessageList	m_MessageList;
	ActorMsg::C_XtActorMessageBase *		m_TriggerActorMessage;
};

}	// namespace ActorMsg

#endif

