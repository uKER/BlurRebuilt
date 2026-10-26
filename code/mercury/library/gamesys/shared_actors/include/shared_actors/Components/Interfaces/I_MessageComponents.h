#ifndef I_MESSAGE_COMPONENTS_H
#define I_MESSAGE_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Components
{

// Message Sequencer Interface
class I_MessageSequencer : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_MessageSequencer;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_MessageSequencer;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

};

// Counter Interface
class I_Counter : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_Counter;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_Counter;} // from Component::I_GameComponent
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

	// queries
	virtual S32		GetValue() const = 0;

};

// Timer Interface
class I_Timer : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_Timer;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_Timer;} // from Component::I_GameComponent
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

	virtual F32		GetValue(void) const = 0;
};

// Message Forwarder List Interface
class I_MessageForwarderList : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId InterfaceType() {return E_IF_MessageForwarderList;}
	Component::C_ComponentInterfaceId GetInterfaceID() const {return E_IF_MessageForwarderList;}
};

}

#endif
