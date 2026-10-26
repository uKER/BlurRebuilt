#include "SharedActorsPch.h"

#include <misc_shared/Messaging/GameMessaging.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/ActorImpl/AnimationActors.h>
#include <shared_actors/Components/Interfaces/I_AnimationController.h>
#include <shared_actors/Components/Messages/AnimationMessages.h>

#include <System/debug/I_DebugRender.h>



Debug::Bool 	g_DebugAnimationPlaybackActorDisplayTime		("ActorDebug.Components.AnimationController.DebugRender.DisplayTime",				false);
Debug::Bool 	g_DebugAnimationPlaybackActorDisplayTimeToOutput("ActorDebug.Components.AnimationController.DebugRender.DisplayTimeToOutput",		false);


namespace Actors
{

C_AnimationPlaybackActor::C_AnimationPlaybackActor():m_State(E_State_Uninitialised),m_CurrTValue(0),m_AnimSpeedScale(1.0f)
{
}

C_AnimationPlaybackActor::~C_AnimationPlaybackActor()
{
}

bool C_AnimationPlaybackActor::InitialiseImpl()
{
	m_WorldMatrix = GetCreationData()->GetCreationMatrix();

	// create animation controller component
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_AnimationController) == NULL)
		return false;

	return true;
}

bool C_AnimationPlaybackActor::CreateImpl()
{
	return true;
}

void C_AnimationPlaybackActor::ChangeState(E_State newState)
{
	if(newState != m_State)
	{
		switch(newState)
		{
		case E_State_Stopped:
			RemoveSubscriber(GameMessaging::E_GameMsg_PreUpdate);
			RemoveSubscriber(GameMessaging::E_GameMsg_PausedUpdate);
			break;
		case E_State_Playing:
			AddSubscriber(GameMessaging::E_GameMsg_PreUpdate);
			AddSubscriber(GameMessaging::E_GameMsg_PausedUpdate);
			break;
		case E_State_Finished:
			RemoveSubscriber(GameMessaging::E_GameMsg_PreUpdate);
			RemoveSubscriber(GameMessaging::E_GameMsg_PausedUpdate);
			break;
		}

		m_State = newState;
	}
}

// Start Animation Playing
void C_AnimationPlaybackActor::Start(void)
{
	ChangeState(E_State_Playing);
}

// Stop Animation Playing
void C_AnimationPlaybackActor::Stop(void)
{
	ChangeState(E_State_Stopped);
}

// Reset animation to begining
void C_AnimationPlaybackActor::Reset(void)
{
	m_CurrTValue = 0;
	// Send new TValue to animation controller
	ActorMsg::C_AnimationSetTValueMsg	setTValueMsg(m_CurrTValue);
	SendMessageToComponent(Components::E_IF_AnimationController,&setTValueMsg);

	ChangeState(E_State_Stopped);
}

void C_AnimationPlaybackActor::DebugOutput(bool paused)
{
	if (g_DebugAnimationPlaybackActorDisplayTime)
	{
		Util::C_String text;
		text.Printf("Animation Time: %f", m_CurrTValue);
		DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(20,150), text);
		if (!paused && g_DebugAnimationPlaybackActorDisplayTimeToOutput)
		{
			LOGINFO(Actors::g_Log, "Animation Time: %f", m_CurrTValue);
		}
	}
}

void C_AnimationPlaybackActor::Update(F32 timeStep)
{
	const Components::I_AnimationController *animationControllerIF = GetAnimationController();
	BIZ_ASSERT(animationControllerIF);

	// Update animation TValue
	m_CurrTValue += timeStep * m_AnimSpeedScale;

	DebugOutput();

	// Check if animation has finished
	bool bFinished = false;
	if(m_CurrTValue > animationControllerIF->GetAnimationLength())
	{
		bFinished = true;
		m_CurrTValue = animationControllerIF->GetAnimationLength();
	}
	else if (m_CurrTValue < 0)
	{
		bFinished = true;
		m_CurrTValue = 0;
	}

	// Send new TValue to animation controller
	ActorMsg::C_AnimationMoveToTValueMsg	moveTValueMsg(m_CurrTValue);
	SendMessageToComponent(Components::E_IF_AnimationController, &moveTValueMsg);

	if(bFinished)
	{
		// Notify notifcation actor the action has been completed
		ActorMsg::C_ActionCompletedMsg actionCompletedMsg;
		actionCompletedMsg.SetActorHandle(GetHandle());
		SendMessageToActor(m_NotificationActor, &actionCompletedMsg);
		m_NotificationActor = Actor::C_ActorHandle();	// clear notification actor
		ChangeState(E_State_Finished);
	}
}

bool C_AnimationPlaybackActor::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_Activate:
		Start();
		break;
	case ActorMsg::E_GameActorMsg_Deactivate:
		Stop();
		break;
	case ActorMsg::E_GameActorMsg_Reset:
		Reset();
		break;

	case ActorMsg::E_GameActorMsg_ActionBegin:
		{
			// Store actor to inform upon completion
			m_NotificationActor = Messaging::MessageCast<ActorMsg::C_ActionBeginMsg>(pMessage)->GetActorHandle();
			Start();
		}
		break;

	case GameMessaging::E_GameMsg_PreUpdate:
		Update(Messaging::MessageCast<GameMessaging::C_PreUpdateMsg>(pMessage)->GetValue());
		break;
	case GameMessaging::E_GameMsg_PausedUpdate:
		DebugOutput(true);
		break;
	default:
		return C_SharedActorBase::OnMessage(pMessage);
	}

	return true;
}



// Serialisation Functions
// -----------------------

void C_AnimationPlaybackActor::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	Actor::C_Actor::SerialiseForSave(stream);
	Serialise(stream);
}

void C_AnimationPlaybackActor::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	Actor::C_Actor::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_AnimationPlaybackActor::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	Actor::C_Actor::SerialiseForSync(stream);
	Serialise(stream);
}

void C_AnimationPlaybackActor::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	Actor::C_Actor::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_AnimationPlaybackActor::Serialise(Sys::C_ContextualStream& stream) const
{
	stream.WriteAs<U32>(m_State);
	stream.WriteAs<F32>(m_CurrTValue);
	stream << m_AnimSpeedScale;
}

void C_AnimationPlaybackActor::Deserialise(Sys::C_ContextualStream& stream)
{
	stream.ReadAs<U32>(m_State);
	stream.ReadAs<F32>(m_CurrTValue);
	stream >> m_AnimSpeedScale;
}

#ifndef CFG_GOLD

void C_AnimationPlaybackActor::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr) const
{
	const Components::I_AnimationController *animationControllerIF = GetAnimationController();

	if (animationControllerIF)
		animationControllerIF->DebugRender(debugRenderPtr);
}

#endif

// Animation Slave Controller

// This actor pulls it's TValue from another actor's animation controller component
bool C_AnimationSlaveController::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_Activate:
		break;
	case ActorMsg::E_GameActorMsg_Deactivate:
		break;
	case ActorMsg::E_GameActorMsg_Reset:
		break;

	default:
		return C_SharedActorBase::OnMessage(pMessage);
	}

	return true;
}

bool C_AnimationSlaveController::InitialiseImpl()
{
	m_WorldMatrix = GetCreationData()->GetCreationMatrix();

	// create animation controller component
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_AnimationController) == NULL)
		return false;

	return true;
}

bool C_AnimationSlaveController::CreateImpl()
{

	return true;
}



}//namespace Actors
