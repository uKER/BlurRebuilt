#include "SharedActorsPch.h"

#include <actor_system/Actor.h>
#include <shared_actors/Components/Creation/MessageComponentsCreate.h>
#include <shared_actors/XtActorBase.h>

XT_TYPE(Components::C_MessageScriptItem, "MessageScriptItem")
XT_FIELD(m_Disable, "Disable")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptCommentItem ,"MessageScriptCommentItem")
XT_BASE(Components::C_MessageScriptItem)
XT_FIELD(m_CommentText, "comment")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptMessageItem ,"MessageScriptMessageItem")
XT_BASE(Components::C_MessageScriptItem)
XT_FIELD(m_Messages, "Messages")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptDelayItem ,"MessageScriptDelayItem")
XT_BASE(Components::C_MessageScriptItem)
XT_FIELD(m_Delay, "DelayTime")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptWaitActivateItem ,"MessageScriptWaitForActivateItem")
XT_BASE(Components::C_MessageScriptItem)
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptWaitBroadcastItem ,"MessageScriptWaitForBroadcastItem")
XT_BASE(Components::C_MessageScriptItem)
XT_FIELD(m_Message, "Message")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptMovementMessageItem ,"MessageScriptMovementMessageItem")
XT_BASE(Components::C_MessageScriptItem)
XT_FIELD(m_Message, "Message")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptMovementMessageListItem ,"MessageScriptMovementMessageListItem")
XT_BASE(Components::C_MessageScriptItem)
XT_FIELD(m_Messages, "Messages")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptWaitActionItem ,"MessageScriptActionItem")
XT_BASE(Components::C_MessageScriptItem)
XT_FIELD(m_pActionActor, "ActionActor")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptWaitActionListItem ,"MessageScriptActionListItem")
XT_BASE(Components::C_MessageScriptItem)
XT_FIELD(m_Actions, "Actions")
XT_END_TYPE

XT_TYPE(Components::C_MessageScriptCreator, "MessageScriptComp")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_bStartRunning, "StartRunning")
XT_FIELD(m_bStartDisabled, "StartDisabled")
XT_FIELD(m_bRestartWhenFinished, "RestartWhenFinished")
XT_FIELD(m_bOneShot, "OneShot")
XT_FIELD(m_ScriptItems, "Sequence")
XT_END_TYPE

// message forwarder list
XT_TYPE(Components::E_MessageForwarderListSelection, "EMessageForwarderSelectionType")
XT_OPTION(Components::E_MessageForwarderListSelection_Slot, "Slot")
XT_OPTION(Components::E_MessageForwarderListSelection_Random, "Random")
XT_OPTION(Components::E_MessageForwarderListSelection_Sequence, "Sequence")
XT_OPTION(Components::E_MessageForwarderListSelection_SequenceLoop, "SequenceLoop")
XT_OPTION(Components::E_MessageForwarderListSelection_SequenceLoopLast, "SequenceLoopLast")
XT_END_TYPE

XT_TYPE(Components::C_MessageForwarderListCreator, "MessageForwarderListComp")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_SelectionType,		"SelectionType")
XT_FIELD(m_DestinationActors,	"DestinationActors")
XT_END_TYPE

// counter component
XT_TYPE(Components::C_CounterThresholdMessage, "CounterThresholdMessage")
XT_FIELD(m_CounterValue, "CounterValue")
XT_FIELD(m_Messages, "Messages")
XT_END_TYPE

XT_TYPE(Components::C_CounterDynamicThresholdMessage, "CounterDynamicThresholdMessage")
XT_FIELD(m_CounterValue, "CounterValue")
XT_FIELD(m_Messages, "Messages")
XT_END_TYPE

XT_TYPE(Components::C_CounterComponentCreator, "CounterComp")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_StartValue, "StartValue")
XT_FIELD(m_ThresholdMessages, "ThresholdMessages")
XT_FIELD(m_DynamicThresholdMessages, "DynamicThresholdMessages")
XT_END_TYPE

XT_TYPE(Components::C_TimerThresholdMessage, "TimerThresholdMessage")
XT_FIELD(m_TimerValue, "TimerValue")
XT_FIELD(m_Messages, "Messages")
XT_END_TYPE

XT_TYPE(Components::C_CountdownTimerCreator, "CountdownTimerComp")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_TimeoutValue, "TimeoutValue")
XT_FIELD(m_bStartRunning, "StartRunning")
XT_FIELD(m_bMissionFailTimer, "MissionFailTimer")
XT_FIELD(m_bRestartWhenFinished, "RestartWhenFinished")
XT_FIELD(m_TimeoutMessages, "TimeoutMessages")
XT_FIELD(m_ThresholdMessages, "ThresholdMessages")
XT_END_TYPE

XT_LIBRARY(MessageComponentCreation)

// Message Component Messages
XT_DEPENDS_ON(MessageComponentMessages)

XT_REGISTER_TYPE(Components::C_MessageScriptItem)
XT_REGISTER_TYPE(Components::C_MessageScriptCommentItem)
XT_REGISTER_TYPE(Components::C_MessageScriptMessageItem)
XT_REGISTER_TYPE(Components::C_MessageScriptDelayItem)
XT_REGISTER_TYPE(Components::C_MessageScriptWaitActivateItem)
XT_REGISTER_TYPE(Components::C_MessageScriptWaitBroadcastItem)
XT_REGISTER_TYPE(Components::C_MessageScriptMovementMessageItem)
XT_REGISTER_TYPE(Components::C_MessageScriptMovementMessageListItem)
XT_REGISTER_TYPE(Components::C_MessageScriptWaitActionItem)
XT_REGISTER_TYPE(Components::C_MessageScriptWaitActionListItem)
XT_REGISTER_TYPE(Components::C_MessageScriptCreator)
XT_REGISTER_TYPE(Components::E_MessageForwarderListSelection)
XT_REGISTER_TYPE(Components::C_MessageForwarderListCreator)
XT_REGISTER_TYPE(Components::C_CounterThresholdMessage)
XT_REGISTER_TYPE(Components::C_CounterDynamicThresholdMessage)
XT_REGISTER_TYPE(Components::C_CounterComponentCreator)
XT_REGISTER_TYPE(Components::C_TimerThresholdMessage)
XT_REGISTER_TYPE(Components::C_CountdownTimerCreator)
XT_END_LIBRARY

// Methods
namespace Components
{

bool	C_MessageScriptWaitActionItem::DoAction(const Actor::C_Actor *pContextActor) const
{
	if(m_pActionActor)
	{
		ActorMsg::C_ActionBeginMsg startActionMsg;

		startActionMsg.SetActorHandle(pContextActor->GetHandle());
		pContextActor->SendMessageToActor(m_pActionActor->GetActorHandle(), &startActionMsg);
		return true;
	}

	return false;
}

};//namespace Components

