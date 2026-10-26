#include "SharedActorsPch.h"

#include <shared_actors/Components/Impl/MessageComponents.h>

#include <actor_system/ActorDebug.h>
#include <actor_system/Actor.h>
#include <actor_system/ActorList.h>
#include <misc_shared/messaging/GameMessaging.h>
#include <shared_actors/Components/Creation/MessageComponentsCreate.h>
#include <shared_actors/Components/Messages/MovementMessages.h>
#include <shared_actors/Components/Messages/MessageComponentMessages.h>
#include <shared_actors/XtSharedLevelActorParams.h>
#include <shared_actors/XtActorBase.h>
#include <system/Perf.h>
#include <system/misc/Random.h>


#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif


namespace Components
{

C_MessageScript::C_MessageScript():m_Enabled(true),m_CurrentItem(0),m_State(E_Idle),m_InitialState(E_Idle),m_WaitOnBroadcastMessage(0)
{

}

C_MessageScript::~C_MessageScript()
{
}

bool C_MessageScript::InitialiseImpl()
{
	if(GetCreationData()->m_bStartRunning)
		m_InitialState = E_Process_Messages;
	else
		m_InitialState = E_Wait_Activate;

	if(GetCreationData()->m_bStartDisabled)
		m_Enabled = false;

	return true;
}

bool C_MessageScript::CreateImpl()
{
	ChangeState(m_InitialState);
	return true;
}

bool C_MessageScript::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	PERF_EVENT("C_MessageScript.OnMessage");
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_Update:
		if(m_Enabled)
		{
			F32 timeStep = Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue();

			if(m_State == E_Wait_Delay)
			{
				m_Timer -= timeStep;

				if(m_Timer <= 0 && m_DelayStartFrame != GetActorList()->GetCurrentFrameNo())
					ProcessScript();
			}
			else if(m_State == E_Wait_Broadcast)
			{
				AddSubscriber(m_WaitOnBroadcastMessage);
				RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			}
			else
				ProcessScript();
		}
		break;
	case ActorMsg::E_GameActorMsg_ActionBegin:
		// Store actor to notify and then behave like an activate
		if(m_Enabled)
			m_NotifyActor = Messaging::MessageCast<ActorMsg::C_ActionBeginMsg>(pMessage)->GetActorHandle();
		// intentional drop through
	case ActorMsg::E_GameActorMsg_Activate:
		if(m_Enabled)
		{
			if(m_State == E_Wait_Activate)
			{
				ProcessScript();
			}
			else if(m_State == E_Deactivated)
			{
				ChangeState(m_DeactivatedState);
			}
		}
		break;
	case ActorMsg::E_GameActorMsg_Enable:
		m_Enabled = true;
		break;
	case ActorMsg::E_GameActorMsg_Disable:
		m_Enabled = false;
		break;
	case ActorMsg::E_GameActorMsg_Deactivate:
		if(m_Enabled)
			ChangeState(E_Deactivated);
		break;
	case ActorMsg::E_GameActorMsg_Reset:
		if(m_Enabled)
			Restart(GetCreationData()->m_bStartRunning);
		break;

	case ActorMsg::E_MovementNotificationMsg_MovementComplete:
		if(m_Enabled)
		{

			if(m_State == E_Wait_Movement)
			{
				if(--m_NoNotifyItems == 0)
					ProcessScript();
			}
		}
		break;
	case ActorMsg::E_GameActorMsg_ActionCompleted:
		if(m_Enabled)
		{
			if(m_State == E_Wait_Action)
			{
				if(--m_NoNotifyItems == 0)
					ProcessScript();
			}
		}
		break;
	default:
		if(m_Enabled)
		{
			//[PK] may need to move this out if you are waiting for a message in this switch
			if(m_WaitOnBroadcastMessage == pMessage->Type())
			{
				RemoveSubscriber(m_WaitOnBroadcastMessage);

				// Delay for a frame
				ProcessScript();
			}

			return I_MessageSequencer::OnMessage(pMessage);
		}
	}

	return true;
}

void C_MessageScript::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_Enabled;
	stream << m_CurrentItem;
	stream.WriteAs<U32>(m_State);
	stream.WriteAs<U32>(m_DeactivatedState);
	stream << m_Timer;
	stream << m_NoNotifyItems;
	stream << m_WaitOnBroadcastMessage;
}

void C_MessageScript::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_Enabled;
	stream >> m_CurrentItem;
	stream.ReadAs<U32>(m_InitialState);
	stream.ReadAs<U32>(m_DeactivatedState);
	stream >> m_Timer;
	stream >> m_NoNotifyItems;
	stream >> m_WaitOnBroadcastMessage;
}

void C_MessageScript::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_MessageSequencer::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MessageScript::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_MessageSequencer::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MessageScript::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_MessageSequencer::SerialiseForSync(stream);
	Serialise(stream);
}

void C_MessageScript::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_MessageSequencer::DeserialiseForSync(stream);
	Deserialise(stream);
	ChangeState(m_InitialState);
}

void C_MessageScript::ProcessScript(void)
{
	bool bStartedFromBegining = m_CurrentItem == 0;

	ChangeState(E_Process_Messages);

	BIZ_ASSERT(GetCreationData());
	while(m_CurrentItem != GetCreationData()->m_ScriptItems.count)
	{
		C_MessageScriptItem *pScriptItem = GetCreationData()->m_ScriptItems[m_CurrentItem];
		m_CurrentItem++;

		if(pScriptItem == NULL || (pScriptItem && pScriptItem->m_Disable))	// skip empty or disabled items
			continue;

		switch(pScriptItem->GetItemType())
		{
		case E_MessageScriptItem_None:
			break;

		case E_MessageScriptItem_Comment:
			{
				const C_MessageScriptCommentItem *pCommentItem = (const C_MessageScriptCommentItem *)pScriptItem;
#ifndef CFG_GOLD
				LOGDEBUG(Actor::GetDebugLogLevelDesign(GetActor()), pCommentItem->m_CommentText.Text());
#endif
			}
			break;

		case E_MessageScriptItem_Messages:
			{
				const C_MessageScriptMessageItem *pMessageItem = (const C_MessageScriptMessageItem *)pScriptItem;

				// send delayed messages
				for(S32 messageNo = 0; messageNo < pMessageItem->m_Messages.count; messageNo++)
				{
					pMessageItem->m_Messages[messageNo].Send(GetActor());
				}
			}
			break;
		case E_MessageScriptItem_Delay:
			{
				const C_MessageScriptDelayItem * pDelayMessage = (const C_MessageScriptDelayItem *)pScriptItem;

				m_Timer = pDelayMessage->m_Delay ? pDelayMessage->m_Delay->GetFloat(GetActor()):0;
				m_DelayStartFrame = GetActorList()->GetCurrentFrameNo();
				ChangeState(E_Wait_Delay);		
				return;
			}
			break;
		case E_MessageScriptItem_WaitActivate:
			{
				ChangeState(E_Wait_Activate);		
				return;
			}
			break;

		case E_MessageScriptItem_WaitBroadcast:
			{
				const C_MessageScriptWaitBroadcastItem * pWaitBroadcastMessage = (const C_MessageScriptWaitBroadcastItem *)pScriptItem;

				if (!pWaitBroadcastMessage->m_Message)
				{
					LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), " NULL message type for MessageScriptWaitForBroadcastItem (item %d) in message script '%s'", m_CurrentItem - 1, GetActor()->GetActorName());
					// continue round while loop
				}
				else
				{
					m_WaitOnBroadcastMessage = pWaitBroadcastMessage->m_Message->GetMessage(GetActor())->Type();
					//AddSubscriber(m_WaitOnBroadcastMessage);
					ChangeState(E_Wait_Broadcast);
					return;
				}
			}
			break;

		case E_MessageScriptItem_WaitMovement:
			{
				const C_MessageScriptMovementMessageItem *pMessageItem = (const C_MessageScriptMovementMessageItem *)pScriptItem;

				m_NoNotifyItems = 1;
				pMessageItem->m_Message.Send(GetActor());
				ChangeState(E_Wait_Movement);
				return;
			}
			break;
		case E_MessageScriptItem_WaitMovementList:
			{
				const C_MessageScriptMovementMessageListItem *pMessageItem = (const C_MessageScriptMovementMessageListItem *)pScriptItem;

				m_NoNotifyItems = pMessageItem->m_Messages.count;

				for(U32 messNo = 0; messNo < m_NoNotifyItems; messNo++)
					pMessageItem->m_Messages[messNo].Send(GetActor());

				ChangeState(E_Wait_Movement);
				return;
			}
			break;

		case E_MessageScriptItem_WaitAction:
			{
				const C_MessageScriptWaitActionItem *pMessageItem = (const C_MessageScriptWaitActionItem *)pScriptItem;

				if(pMessageItem->DoAction(GetActor()))
				{
					m_NoNotifyItems = 1;
					ChangeState(E_Wait_Action);
					return;
				}

			}
			break;
		case E_MessageScriptItem_WaitActionList:
			{
				const C_MessageScriptWaitActionListItem *pMessageItem = (const C_MessageScriptWaitActionListItem *)pScriptItem;

				m_NoNotifyItems = 0;

				for(int messNo = 0; messNo < pMessageItem->m_Actions.count; messNo++)
				{
					const Components::C_MessageScriptWaitActionItem* pAction = pMessageItem->m_Actions[messNo];

					if (pAction == NULL)
					{
						LOGWARN(Actor::GetDebugLogLevelDesign(GetActor()), " NULL action %d for MessageScriptActionListItem (item %d) in message script '%s'", messNo, m_CurrentItem - 1, GetActor()->GetActorName());
					}
					else
					{
						if(pAction->DoAction(GetActor()))
						{
							m_NoNotifyItems++;
						}
					}
				}

				if(m_NoNotifyItems!=0)
					ChangeState(E_Wait_Action);
				return;
			}
			break;
		}
	}

	if(bStartedFromBegining == true && GetCreationData()->m_bRestartWhenFinished)
		LOGERROR(Actor::GetDebugLogLevelDesign(GetActor()), "Inifinte Loop Condition detected in Message Script %s - Script Aborted.",GetActor()->GetActorName());
	else if(GetCreationData()->m_bOneShot)
	{
		ChangeState(E_Finished);
		
		// Send action completed message
		ActorMsg::C_ActionCompletedMsg acMsg(GetActorHandle());
		SendMessageToActor(m_NotifyActor, &acMsg);
	}
	else
	{
		Restart(GetCreationData()->m_bRestartWhenFinished);
	}
	//	ChangeState(E_Finished);
}

void C_MessageScript::Restart(bool bRestartRunning)
{
	m_CurrentItem = 0;

	if(bRestartRunning)
		ProcessScript();
	else
		ChangeState(E_Wait_Activate);		

}


void C_MessageScript::ChangeState(E_State newState)
{
	if(newState == m_State)
		return;

	switch(newState)
	{
		case E_Deactivated:
			if(m_State!=E_Idle)
				m_DeactivatedState = m_State;
			RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			break;
		case E_Wait_Delay:
		case E_Wait_Broadcast:
		case E_Process_Messages:
			AddSubscriber(GameMessaging::E_GameMsg_Update);
			break;
		default:
			RemoveSubscriber(GameMessaging::E_GameMsg_Update);
			break;
	}

	m_State = newState;
}

// Counter
C_Counter::C_Counter():m_bEnabled(true),m_CounterValue(0)
{

}

C_Counter::~C_Counter()
{
}

bool C_Counter::InitialiseImpl()
{
	m_CounterValue = GetCreationData()->m_StartValue;

	return true;
}

// Process the counter value
bool C_Counter::ProcessValue(S32 value)
{
	bool bHandled = false;

	BIZ_ASSERT(GetCreationData());

	for(S32 valNo = 0; valNo < GetCreationData()->m_ThresholdMessages.count; valNo++)
	{
		if(GetCreationData()->m_ThresholdMessages[valNo].m_CounterValue == value)
			bHandled = GetCreationData()->m_ThresholdMessages[valNo].m_Messages.Send(GetActor());
	}

	// Go through dynamic threshold messages
	for(S32 valNo = 0; valNo < GetCreationData()->m_DynamicThresholdMessages.count; valNo++)
	{
		if(GetCreationData()->m_DynamicThresholdMessages[valNo].m_CounterValue &&
			GetCreationData()->m_DynamicThresholdMessages[valNo].m_CounterValue->GetInteger(GetActor()) == value)
			bHandled = GetCreationData()->m_DynamicThresholdMessages[valNo].m_Messages.Send(GetActor());
	}

	return bHandled;
}

// Process counter Messages
bool C_Counter::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_GameActorMsg_Enable:
		m_bEnabled = true;
		break;
	case ActorMsg::E_GameActorMsg_Disable:
		m_bEnabled = false;
		break;
	case ActorMsg::E_GameActorMsg_Reset:
		if(m_bEnabled)
			m_CounterValue = GetCreationData()->m_StartValue;
		break;
	case ActorMsg::E_GameActorMsg_Increment:
		if(m_bEnabled)
		{
			m_CounterValue++;
			ProcessValue(m_CounterValue);
		}
		break;
	case ActorMsg::E_GameActorMsg_Decrement:
		if(m_bEnabled)
		{
			m_CounterValue--;
			ProcessValue(m_CounterValue);
		}
		break;
	case ActorMsg::E_GameActorMsg_SetIntegerValue:
		if(m_bEnabled)
		{
			m_CounterValue = Messaging::MessageCast<ActorMsg::C_IntegerValueMsg>(pMessage)->GetValue();
			ProcessValue(m_CounterValue);
		}
		break;
	case ActorMsg::E_GameActorMsg_FlipFlop:
		if(m_bEnabled)
		{
			m_CounterValue = m_CounterValue==0 ? 1 : 0;
			ProcessValue(m_CounterValue);
		}
		break;
	default:
		return I_Counter::OnMessage(pMessage);
		break;
	}

	return true;
}

void C_Counter::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_CounterValue;
}

void C_Counter::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_CounterValue;
}

void C_Counter::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_Counter::SerialiseForSave(stream);
	Serialise(stream);
}

void C_Counter::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_Counter::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_Counter::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_Counter::SerialiseForSync(stream);
	Serialise(stream);
}

void C_Counter::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_Counter::DeserialiseForSync(stream);
	Deserialise(stream);
}


// Countdown Timer
C_CountdownTimer::C_CountdownTimer():m_TimerValue(0),m_bActive(false),m_bFinished(false)
{

}

C_CountdownTimer::~C_CountdownTimer()
{
}

bool C_CountdownTimer::InitialiseImpl()
{
	m_TimerValue = GetCreationData()->m_TimeoutValue;
	m_bActive = GetCreationData()->m_bStartRunning;
	return true;
}

bool C_CountdownTimer::CreateImpl()
{
	if(m_bActive)
		AddSubscriber(GameMessaging::E_GameMsg_Update);

	return true;
}

// Process the counter value
bool C_CountdownTimer::Update(F32 timeStep)
{
	bool bHandled = false;
	F32 oldTimer = m_TimerValue;
	const C_CountdownTimerCreator *pCreateData = GetCreationData();

	BIZ_ASSERT(pCreateData);

	m_TimerValue -= timeStep;

	for(S32 valNo = 0; valNo < pCreateData->m_ThresholdMessages.count; valNo++)
	{
		if(
			pCreateData->m_ThresholdMessages[valNo].m_TimerValue > Maths::Min(oldTimer,m_TimerValue) && 
			pCreateData->m_ThresholdMessages[valNo].m_TimerValue < Maths::Max(oldTimer,m_TimerValue)
		)
			bHandled = pCreateData->m_ThresholdMessages[valNo].m_Messages.Send(GetActor());
	}

	if(m_TimerValue <= 0.0f)
	{
		bHandled |= pCreateData->m_TimeoutMessages.Send(GetActor());

		if(pCreateData->m_bRestartWhenFinished)
		{
			m_TimerValue = GetCreationData()->m_TimeoutValue;
		}
		else
		{
			m_bFinished = true;
			SetActive(false);
		}

		//if(pCreateData->m_bMissionFailTimer)
		//	GetActorList()->BroadcastMessage(E_MissionEventMsg_OutOfTime);

	}

	return bHandled;
}

void C_CountdownTimer::SetActive(bool bActive)
{
	if(bActive != m_bActive)
	{
		if(bActive)
			AddSubscriber(GameMessaging::E_GameMsg_Update);
		else
			RemoveSubscriber(GameMessaging::E_GameMsg_Update);

		m_bActive = bActive;
	}
}


// Process countdown timer Messages
bool C_CountdownTimer::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
	case GameMessaging::E_GameMsg_Update:
		Update(Messaging::MessageCast<GameMessaging::C_UpdateMsg>(pMessage)->GetValue());
		break;
	case ActorMsg::E_GameActorMsg_Activate:
		SetActive(true);
		break;
	case ActorMsg::E_GameActorMsg_Deactivate:
		SetActive(false);
		break;
	case ActorMsg::E_GameActorMsg_Reset:
		{
			m_TimerValue = GetCreationData()->m_TimeoutValue;
			SetActive(GetCreationData()->m_bStartRunning);
		}
		break;
	default:
		return I_Timer::OnMessage(pMessage);
		break;
	}

	return true;
}

void C_CountdownTimer::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_TimerValue;
	stream << m_bActive;
	stream << m_bFinished;

}

void C_CountdownTimer::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_Timer::SerialiseForSave(stream);
	Serialise(stream);
}

void C_CountdownTimer::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_Timer::DeserialiseForSave(stream);
	stream >> m_TimerValue;
	stream >> m_bActive;
	stream >> m_bFinished;
}

void C_CountdownTimer::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_Timer::SerialiseForSync(stream);
	Serialise(stream);
}

void C_CountdownTimer::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_Timer::DeserialiseForSync(stream);
	stream >> m_TimerValue;
	SetActive(stream.ReadAs<bool>());
	stream >> m_bFinished;
}


C_MessageQueue::~C_MessageQueue()
{
	Messaging::C_DefaultMessageAllocator messageAllocator;
	const S32 count = m_MessageQueue.Size();
	for (S32 i=0 ; i<count ; i++)
	{
		messageAllocator.Destroy(const_cast<ActorMsg::C_MovementNotifierMessageBase*>(m_MessageQueue.Front()));
		m_MessageQueue.PopFront();
	}
}


bool C_MessageQueue::InitialiseImpl()
{
	if(!Actor::I_ActorComponent::InitialiseImpl())
		return false;

	if (GetCreationData()->m_MessageDest)
	{
		m_MessageDestHandle = GetCreationData()->m_MessageDest->GetActorHandle();
		if (GetActorList()->ActorExists(m_MessageDestHandle))
			return true;
	}

	return false;
}


bool C_MessageQueue::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_MovementMsg_Translate:
		case ActorMsg::E_MovementMsg_Rotate:
		case ActorMsg::E_MovementMsg_MoveAndAlignToActor:
		case ActorMsg::E_MovementMsg_SpinAxisY:
		case ActorMsg::E_MovementMsg_Spline:
		case ActorMsg::E_MovementMsg_SpinStop:
			QueueMessage(pMessage);
			break;

		case ActorMsg::E_MovementNotificationMsg_MovementComplete:
		{
			// Forward notification message to original actor:
			const ActorMsg::C_MovementNotifierMessageBase* msg = m_MessageQueue.Front();
			SendMessageToActor(msg->Notifier(), pMessage);

			Messaging::C_DefaultMessageAllocator messageAllocator;
			messageAllocator.Destroy(const_cast<ActorMsg::C_MovementNotifierMessageBase*>(msg));

			m_MessageQueue.PopFront();
			m_AwaitingNotification = false;
		}	break;

		default:
			return Actor::I_ActorComponent::OnMessage(pMessage);
	}

	if (!m_AwaitingNotification && !m_MessageQueue.Empty())
	{
		// Send next queued message:
		const ActorMsg::C_MovementNotifierMessageBase* msg = m_MessageQueue.Front();

		// [MGB] need to poolify this?
		Messaging::C_DefaultMessageAllocator messageAllocator;
		ActorMsg::C_MovementNotifierMessageBase* msgCopy = static_cast<ActorMsg::C_MovementNotifierMessageBase*>(messageAllocator.Copy(msg));
 		msgCopy->SetNotifier(GetActorHandle());
 		SendMessageToActor(m_MessageDestHandle, msgCopy);
		messageAllocator.Destroy(msgCopy);

		m_AwaitingNotification = true;
	}

	return true;
}


void C_MessageQueue::QueueMessage(const Messaging::C_MessageBase* pMessage)
{
	Messaging::C_DefaultMessageAllocator messageAllocator;
	const ActorMsg::C_MovementNotifierMessageBase* msgCopy = static_cast<ActorMsg::C_MovementNotifierMessageBase*>(messageAllocator.Copy(pMessage));
	m_MessageQueue.PushBack(msgCopy);
}


void C_MessageQueue::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_MessageDestHandle;

	const S32 msgCount = m_MessageQueue.Size();
	stream << msgCount;

	for (TMessageQueue::ConstIterator it = m_MessageQueue.Begin(); it != m_MessageQueue.End(); ++it)
		Messaging::C_MessageSerialiser::Serialise(stream, (*it));

	stream << m_AwaitingNotification;
}

void C_MessageQueue::Deserialise(Sys::C_ContextualStream& stream)
{
	Messaging::C_DefaultMessageAllocator messageAllocator;

	stream >> m_MessageDestHandle;

	S32 msgCount = 0;
	stream >> msgCount;
	for (S32 i=0 ; i<msgCount ; i++)
	{
		Messaging::C_MessageBase* msg = Messaging::C_MessageSerialiser::Deserialise(stream, messageAllocator);
		ActorMsg::C_MovementNotifierMessageBase* notifierMsg = static_cast<ActorMsg::C_MovementNotifierMessageBase*>(msg);
		m_MessageQueue.PushBack(notifierMsg);
	}

	stream >> m_AwaitingNotification;
}

void C_MessageQueue::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_ActorComponent::SerialiseForSave(stream);
	Serialise(stream);
}

void C_MessageQueue::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_ActorComponent::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_MessageQueue::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_ActorComponent::SerialiseForSync(stream);
	//? Serialise(stream);
}

void C_MessageQueue::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_ActorComponent::DeserialiseForSync(stream);
	//? Deserialise(stream);
}

// Message Forwarder List

C_MessageForwarderList::C_MessageForwarderList()
{

}

C_MessageForwarderList::~C_MessageForwarderList()
{

}

bool C_MessageForwarderList::InitialiseImpl()
{
	if(!I_MessageForwarderList::InitialiseImpl())
		return false;

	m_CurrentSlot = 0;
	m_SelectionType = GetCreationData()->m_SelectionType;

	return true;
}

bool C_MessageForwarderList::OnMessage(const Messaging::C_MessageBase* pMessage)
{
	switch(pMessage->Type())
	{
	case ActorMsg::E_MessageForwarderMsg_SetSlot:
		m_CurrentSlot = Messaging::MessageCast<ActorMsg::C_MessageForwarderSetSlot>(pMessage)->GetValue();
		break;
	case ActorMsg::E_MessageForwarderMsg_IncrementSlot:
		m_CurrentSlot = (m_CurrentSlot + 1) % GetCreationData()->m_DestinationActors.count;
		break;
	case ActorMsg::E_MessageForwarderMsg_DecrementSlot:
		m_CurrentSlot = (m_CurrentSlot == 0) ? GetCreationData()->m_DestinationActors.count - 1:m_CurrentSlot - 1;
		break;
	case ActorMsg::E_MessageForwarderMsg_SetSelectionType:
		m_SelectionType = (E_MessageForwarderListSelection)Messaging::MessageCast<ActorMsg::C_MessageForwarderSetSelection>(pMessage)->GetValue();
		break;
	default:
		return ForwardMessage(pMessage);

	}

	return true;
}

static Util::C_Random g_Rnd;

bool C_MessageForwarderList::ForwardMessage(const Messaging::C_MessageBase* pMessage)
{
	const C_MessageForwarderListCreator* pCreationData = GetCreationData();
	S32 sendSlot = m_CurrentSlot;

	// Choose the message slot
	switch(m_SelectionType)
	{
	case E_MessageForwarderListSelection_Slot:
		break;
	case E_MessageForwarderListSelection_Random:		
		// Choose a random slot
		sendSlot = g_Rnd.Rand(pCreationData->m_DestinationActors.count - 1);
		m_CurrentSlot = sendSlot;
		break;
	case E_MessageForwarderListSelection_Sequence:
		m_CurrentSlot++;

		if(m_CurrentSlot == pCreationData->m_DestinationActors.count)
			m_CurrentSlot = -1;
		break;
	case E_MessageForwarderListSelection_SequenceLoop:
		if(pCreationData->m_DestinationActors.count)
			m_CurrentSlot = (m_CurrentSlot + 1) % pCreationData->m_DestinationActors.count;
		break;
	case E_MessageForwarderListSelection_SequenceLoopLast:
		if(m_CurrentSlot < pCreationData->m_DestinationActors.count -1)
			m_CurrentSlot++;
		break;
	}

	// Send the message
	if(sendSlot != -1 && pCreationData->m_DestinationActors[sendSlot])
		SendMessageToActor(pCreationData->m_DestinationActors[sendSlot]->GetActorHandle(GetActor()), pMessage);
	
	return true;
}

// Serialisation
void C_MessageForwarderList::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_MessageForwarderList::SerialiseForSave(stream);
	stream << m_CurrentSlot;
	stream.WriteAs<U32>(m_SelectionType);
}

void C_MessageForwarderList::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_MessageForwarderList::DeserialiseForSave(stream);
	stream >> m_CurrentSlot;
	m_SelectionType = (E_MessageForwarderListSelection)stream.ReadAs<U32>();
}

void C_MessageForwarderList::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_MessageForwarderList::SerialiseForSync(stream);
	stream << m_CurrentSlot;
	stream.WriteAs<U32>(m_SelectionType);

}
void C_MessageForwarderList::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_MessageForwarderList::DeserialiseForSync(stream);
	stream >> m_CurrentSlot;
	m_SelectionType = (E_MessageForwarderListSelection)stream.ReadAs<U32>();
}


} // namespace Components

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif

