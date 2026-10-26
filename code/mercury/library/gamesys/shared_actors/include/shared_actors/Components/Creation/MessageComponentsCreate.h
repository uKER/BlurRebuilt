#ifndef MESSAGE_COMPONENTS_CREATE_H
#define MESSAGE_COMPONENTS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/XtActorMessages.h>
#include <shared_actors/Components/Messages/MovementMessages.h>

// forward declarations
namespace Actors
{
	class C_GameActorCreate;
}

namespace Components
{
// 
// Message Sequencer Creator


enum E_MessageScriptItem
{
	E_MessageScriptItem_None = 0,
	E_MessageScriptItem_Comment,
	E_MessageScriptItem_Messages,
	E_MessageScriptItem_Delay,
	E_MessageScriptItem_WaitActivate,
	E_MessageScriptItem_WaitMovement,
	E_MessageScriptItem_WaitMovementList,
	E_MessageScriptItem_WaitAction,
	E_MessageScriptItem_WaitActionList,
	E_MessageScriptItem_WaitBroadcast,
};

class C_MessageScriptItem
{
public:
	C_MessageScriptItem():m_Disable(false){}
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_None;}	
	bool			m_Disable;
};

class C_MessageScriptCommentItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_Comment;}	

	XtSz8			m_CommentText;
};

class C_MessageScriptMessageItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_Messages;}	

	XtArray<ActorMsg::C_XtDelayedSendMessage>	m_Messages;
};

class C_MessageScriptDelayItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_Delay;}	

	Actors::C_XtParamFloat *				m_Delay;
};

class C_MessageScriptWaitActivateItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_WaitActivate;}	
};

class C_MessageScriptWaitBroadcastItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_WaitBroadcast;}	

	ActorMsg::C_XtActorMessageBase *	m_Message;
};

class C_MessageScriptMovementMessageItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_WaitMovement;}	

	ActorMsg::C_XtMovementSendMessage		m_Message;
};

class C_MessageScriptMovementMessageListItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_WaitMovementList;}	

	XtArray<ActorMsg::C_XtMovementSendMessage>		m_Messages;
};

class C_MessageScriptWaitActionItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_WaitAction;}	

	Actors::C_GameActorCreate	*m_pActionActor; 

	virtual bool	DoAction(const Actor::C_Actor *pContextActor) const;
};

class C_MessageScriptWaitActionListItem : public C_MessageScriptItem
{
public:
	virtual E_MessageScriptItem		GetItemType(){return E_MessageScriptItem_WaitActionList;}	

	XtArray<C_MessageScriptWaitActionItem *>		m_Actions; 
};

// component creator structure
class C_MessageScriptCreator : public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MessageScript;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MessageScript;}

	bool				m_bStartRunning;
	bool				m_bStartDisabled;
	bool				m_bRestartWhenFinished;
	bool				m_bOneShot;
	XtArray<C_MessageScriptItem *>	m_ScriptItems;
};

// Counter Component
class C_CounterThresholdMessage
{
public:
	S32							m_CounterValue;
	ActorMsg::C_XtDelayedSendMessageList	m_Messages;
};

class C_CounterDynamicThresholdMessage
{
public:
	Actors::C_XtParamInteger *				m_CounterValue;
	ActorMsg::C_XtDelayedSendMessageList	m_Messages;
};

class C_CounterComponentCreator : public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_Counter;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_Counter;}

	S32									m_StartValue;
	XtArray<C_CounterThresholdMessage>	m_ThresholdMessages;
	XtArray<C_CounterDynamicThresholdMessage>	m_DynamicThresholdMessages;
};

// Countdown Timer Component
class C_TimerThresholdMessage
{
public:
	F32							m_TimerValue;
	ActorMsg::C_XtDelayedSendMessageList	m_Messages;
};

class C_CountdownTimerCreator : public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_CountdownTimer;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_CountdownTimer;}

	F32							m_TimeoutValue;
	bool						m_bStartRunning;
	bool						m_bMissionFailTimer;
	bool						m_bRestartWhenFinished;
	ActorMsg::C_XtDelayedSendMessageList	m_TimeoutMessages;

	XtArray<C_TimerThresholdMessage>	m_ThresholdMessages;
};


// Message queue component creator.
class C_MessageQueueCreator : public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MessageQueue;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MessageQueue;}

	const Actors::C_GameActorCreate* m_MessageDest;
};


// Message Forwarder List

enum E_MessageForwarderListSelection
{
	E_MessageForwarderListSelection_Slot = 0,			// Always use the same slot unless messaged to use a new slot
	E_MessageForwarderListSelection_Random,				// Forward message to random slot
	E_MessageForwarderListSelection_Sequence,			// Use the next slot in the sequence, don't send when the sequence has finished
	E_MessageForwarderListSelection_SequenceLoop,		// Use the next slot in the sequence and loop back to the beginning when finished
	E_MessageForwarderListSelection_SequenceLoopLast,	// Use the next slot in the sequence and loop on the last slot
};

class C_MessageForwarderListCreator :  public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_MessageForwarderList;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MessageForwarderList;}

	E_MessageForwarderListSelection			m_SelectionType;
	XtArray<Actors::C_XtParamActor *>		m_DestinationActors;
};

} // namespace Components

#endif
