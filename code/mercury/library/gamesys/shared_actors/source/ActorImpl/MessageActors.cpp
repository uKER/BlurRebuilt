#include "SharedActorsPch.h"
// Message Actors


#include <shared_actors/ActorImpl/MessageActors.h>
#include <shared_actors/Components/Creation/MessageComponentsCreate.h>
#include <shared_actors/Components/Impl/MessageComponents.h>

#include <shared_actors/Components/Messages/LogicMessages.h>

namespace Actors
{

// Message Script Actor
bool C_MessageScript::InitialiseImpl()
{
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_SorceryScriptComponent) == NULL)
		return false;

	return true;
}

// Message Forwarder List Actor
bool C_MessageForwarderList::InitialiseImpl()
{
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_ForwarderComponent) == NULL)
		return false;

	return true;
}

// Counter Actor
bool C_Counter::InitialiseImpl()
{
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_SorceryCounterComponent) == NULL)
		return false;

	return true;
}

// Countdown Timer Actor
bool C_CountdownTimer::InitialiseImpl()
{
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_SorceryCountdownTimerComponent) == NULL)
		return false;

	return true;
}

// Message Queue
bool C_MessageQueue::InitialiseImpl()
{
	Component::C_ComponentCreatorFactoryHelper<Components::C_MessageQueueCreator> pCreator;
	if (GetCreationData()->m_MessageDest)
	{
		pCreator->m_MessageDest = GetCreationData()->m_MessageDest;
		return (InitialiseComponent(pCreator.Get()) != NULL);
	}

	return false;
}


// Logic Gate Message Sender

bool C_LogicGateMessageSender::InitialiseImpl()
{
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_SorceryLogicGateComponent) == NULL)
		return false;

	return true;
}

bool C_LogicGateMessageSender::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_LogicMsg_OutputTrue:
		GetCreationData()->m_OnTrue.Send(this);
		break;
	case ActorMsg::E_LogicMsg_OutputFalse:
		GetCreationData()->m_OnFalse.Send(this);
		break;

	default:
		return C_SharedActorBase::OnMessage(pMessage);
	}

	return true;
}

// Bool Expression Message Forwarder
C_BoolExpressionMessageForwarder::C_BoolExpressionMessageForwarder():m_pMessageBuffer(NULL)
{

}

bool C_BoolExpressionMessageForwarder::InitialiseImpl()
{
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_SorceryBoolExpressionComponent) == NULL)
		return false;

	return true;
}

bool C_BoolExpressionMessageForwarder::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	// TODO: forward message on
	switch(pMessage->Type())
	{
		case ActorMsg::E_LogicMsg_OutputTrue:
			if(GetCreationData()->m_Destination)
			{
				SendMessageToActor(GetCreationData()->m_Destination->GetActorHandle(),(Messaging::C_MessageBase *)m_pMessageBuffer);
				SAFE_DELETE_ARY(m_pMessageBuffer);
			}
		break;

		default:
			if(GetCreationData()->m_Destination)
			{
				if(m_pMessageBuffer)
					LOGWARN(Actor::GetDebugLogGeneral(), DBSTR "Forwarding message when one is already buffered - discarding old message");

				SAFE_DELETE_ARY(m_pMessageBuffer);	// delete old buffer if there is one

				m_pMessageBuffer = new U8[pMessage->Size()];
				memcpy(m_pMessageBuffer,pMessage,pMessage->Size());

				SendMessageToComponent(Components::E_IF_BooleanExpressionEval, ActorMsg::E_LogicMsg_CalculateExpression);
			}
			else
				LOGERROR(Actor::GetDebugLogGeneral(), DBSTR "no destination actor so message can't be forwarded");

			break;
	}
	return true;
}

// Bool Expression Message Sender

bool C_BoolExpressionMessageSender::InitialiseImpl()
{
	if (InitialiseComponentFromSorcery(&GetCreationData()->m_SorceryBoolExpressionComponent) == NULL)
		return false;

	return true;
}

bool C_BoolExpressionMessageSender::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_Activate:
		// Send calculate message to component
		SendMessageToComponent(Components::E_IF_BooleanExpressionEval, ActorMsg::E_LogicMsg_CalculateExpression);
		break;
	case ActorMsg::E_LogicMsg_OutputTrue:
		GetCreationData()->m_OnTrue.Send(this);
		break;
	case ActorMsg::E_LogicMsg_OutputFalse:
		GetCreationData()->m_OnFalse.Send(this);
		break;

	default:
		return C_SharedActorBase::OnMessage(pMessage);
	}

	return true;
}


} // namespace Actors
