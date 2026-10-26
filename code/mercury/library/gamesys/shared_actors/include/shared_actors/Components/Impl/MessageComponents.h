#ifndef MESSAGE_COMPONENTS_H
#define MESSAGE_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Creation/MessageComponentsCreate.h>
#include <shared_actors/Components/Interfaces/I_MessageComponents.h>
//#include <shared_actors/Components/Messages/MovementMessages.h>

namespace Components
{

// Component
class C_MessageScript : public I_MessageSequencer
{
	enum E_State
	{
		E_Idle = 0,
		E_Process_Messages,
		E_Wait_Delay,
		E_Wait_Activate,
		E_Wait_Movement,
		E_Wait_Action,
		E_Finished,
		E_Deactivated,
		E_Wait_Broadcast,
	};

public:
	C_MessageScript();
	~C_MessageScript();

	bool				OnMessage(const Messaging::C_MessageBase *pMessage );	// message handler

	static Component::C_ComponentTypeId		ClassType() {return Components::E_MessageScript;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_MessageScript;} // from Component::I_GameComponent
	void				ChangeState(E_State newState);
	void				ProcessScript(void);
	void				Restart(bool bRestartRunning);

	virtual void		SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void		SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSync(Sys::C_ContextualStream& stream);

	const C_MessageScriptCreator	*GetCreationData(void) const {return static_cast<const C_MessageScriptCreator *>(GetCreator().GetData());}

protected:
	virtual bool		InitialiseImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent

private:
	bool				m_Enabled;
	S32					m_CurrentItem;
	E_State				m_InitialState;
	E_State				m_State;
	E_State				m_DeactivatedState;
	F32					m_Timer;
	U32					m_DelayStartFrame;
	U32					m_NoNotifyItems;
	Messaging::TMsgType	m_WaitOnBroadcastMessage;
	Actor::C_ActorHandle	m_NotifyActor;	///< Actor to notify on script completion

	void				Serialise(Sys::C_ContextualStream& stream) const;
	void				Deserialise(Sys::C_ContextualStream& stream);

};

// Counter component
class C_Counter : public I_Counter
{
public:
	C_Counter();
	~C_Counter();

	bool				OnMessage(const Messaging::C_MessageBase *pMessage );	// message handler

	static Component::C_ComponentTypeId		ClassType() {return Components::E_Counter;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_Counter;} // from Component::I_GameComponent
	const C_CounterComponentCreator	*GetCreationData(void) const {return static_cast<const C_CounterComponentCreator *>(GetCreator().GetData());}

	virtual void		SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void		SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSync(Sys::C_ContextualStream& stream);

	bool				ProcessValue(S32 value);

	// interface implementation
	virtual S32			GetValue() const {return m_CounterValue;}

protected:
	virtual bool		InitialiseImpl(); // from Component::I_GameComponent

private:
	bool				m_bEnabled;
	S32					m_CounterValue;

	void				Serialise(Sys::C_ContextualStream& stream) const;
	void				Deserialise(Sys::C_ContextualStream& stream);

};




// Countdown timer component

class C_CountdownTimer : public I_Timer
{
public:
	C_CountdownTimer();
	~C_CountdownTimer();

	bool				OnMessage(const Messaging::C_MessageBase *pMessage );	// message handler

	virtual void		SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void		SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSync(Sys::C_ContextualStream& stream);

	static Component::C_ComponentTypeId		ClassType() {return Components::E_CountdownTimer;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_CountdownTimer;} // from Component::I_GameComponent
	const C_CountdownTimerCreator	*GetCreationData(void) const {return static_cast<const C_CountdownTimerCreator *>(GetCreator().GetData());}

	bool				Update(F32 timeStep);
	void				SetActive(bool bActive);

	virtual F32			GetValue(void) const {return m_TimerValue;}

protected:
	virtual bool		InitialiseImpl(); // from Component::I_GameComponent
	virtual bool		CreateImpl(); // from Component::I_GameComponent

private:
	F32					m_TimerValue;
	bool				m_bActive;
	bool				m_bFinished;

	void				Serialise(Sys::C_ContextualStream& stream) const;

};



///////////////////////////////////////////////////////////////////////////////////////////////////
// Message queue component class.
//
class C_MessageQueue : public Actor::I_ActorComponent
{
public:
	C_MessageQueue() : m_AwaitingNotification(false) {}
	~C_MessageQueue();

public: // From I_ActorComponent
	static Component::C_ComponentTypeId ClassType() {return E_MessageQueue;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_MessageQueue;}
	static Component::C_ComponentInterfaceId InterfaceType() {return E_IF_MessageQueue;}
	Component::C_ComponentInterfaceId GetInterfaceID() const {return E_IF_MessageQueue;}
	const C_MessageQueueCreator* GetCreationData(void) const {return static_cast<const C_MessageQueueCreator*>(GetCreator().GetData());}

	bool OnMessage(const Messaging::C_MessageBase* pMessage);

	void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	void DeserialiseForSave(Sys::C_ContextualStream& stream);
	void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	void DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	// From Component::I_GameComponent
	bool InitialiseImpl();

private:
	void QueueMessage(const Messaging::C_MessageBase* pMessage);
 	void Serialise(Sys::C_ContextualStream& stream) const;
 	void Deserialise(Sys::C_ContextualStream& stream);

private:
	Actor::C_ActorHandle m_MessageDestHandle;
	typedef Btl::DynamicPool<ActorMsg::C_MovementNotifierMessageBase*> TMessagePool;
	typedef Btl::List<const ActorMsg::C_MovementNotifierMessageBase*, Btl::ListParameters<Btl::DoubleEndedList, Btl::OnDemandCounter, Btl::ListElementSingle, TMessagePool, Btl::External> > TMessageQueue;
	TMessageQueue m_MessageQueue;
	bool m_AwaitingNotification;
};

// Message Forwarder List Component
class C_MessageForwarderList : public I_MessageForwarderList
{
public:
	C_MessageForwarderList();
	~C_MessageForwarderList();
public: // From I_ActorComponent
	static Component::C_ComponentTypeId ClassType() {return E_MessageForwarderList;}
	virtual Component::C_ComponentTypeId GetComponentTypeID() const {return E_MessageForwarderList;}
	const C_MessageForwarderListCreator* GetCreationData(void) const {return static_cast<const C_MessageForwarderListCreator*>(GetCreator().GetData());}

	bool OnMessage(const Messaging::C_MessageBase* pMessage);
	bool ForwardMessage(const Messaging::C_MessageBase* pMessage);

	void SerialiseForSave(Sys::C_ContextualStream& stream) const;
	void DeserialiseForSave(Sys::C_ContextualStream& stream);
	void SerialiseForSync(Sys::C_ContextualStream& stream) const;
	void DeserialiseForSync(Sys::C_ContextualStream& stream);

protected:
	// From Component::I_GameComponent
	bool InitialiseImpl();
private:
	S32		m_CurrentSlot;
	E_MessageForwarderListSelection	m_SelectionType;
};

};	// namespace Components

#endif
