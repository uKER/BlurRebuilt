#ifndef I_LOGIC_COMPONENTS_H
#define I_LOGIC_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Components
{

// Logic Gate Interface
class I_LogicGate : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_LogicGate;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_LogicGate;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	

	// interface methods
	virtual bool		GetOutput(void) const = 0;

};


// Boolean Expression Evaluator Interface
class I_BoolExpressionEvaluator : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType() {return Components::E_IF_BooleanExpressionEval;}
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const {return Components::E_IF_BooleanExpressionEval;}
	virtual bool	OnMessage(const Messaging::C_MessageBase *pMessage ) {return Actor::I_ActorComponent::OnMessage(pMessage);}	
};


}//namespace Components

#endif