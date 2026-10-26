#ifndef LOGIC_COMPONENTS_H
#define LOGIC_COMPONENTS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/Components/Creation/LogicComponentsCreate.h>
#include <shared_actors/Components/Interfaces/I_LogicComponents.h>

// forward decs
namespace Sys
{
	class C_ContextualStream;
};

namespace Components
{

// Logic Gate
class C_LogicGate : public I_LogicGate
{
public:
	C_LogicGate();
	~C_LogicGate();

	bool				OnMessage(const Messaging::C_MessageBase *pMessage );	// message handler

	static Component::C_ComponentTypeId		ClassType() {return Components::E_LogicGate;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_LogicGate;} // from Component::I_GameComponent

	const C_LogicGateComponentCreator	*GetCreationData(void) const {return static_cast<const C_LogicGateComponentCreator *>(GetCreator().GetData());}

	bool				EvaluateInputs(void) const;
	bool				SetInputValue(U32 actorId, bool inputValue);
	bool				UpdateInputValue(U32 actorId, bool inputValue);

	// Interface Methods
	virtual bool		GetOutput(void) const { return m_Output;}


	// Serialisation
	virtual void		SerialiseForSave(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSave(Sys::C_ContextualStream& stream);

	virtual void		SerialiseForSync(Sys::C_ContextualStream& stream) const;
	virtual void		DeserialiseForSync(Sys::C_ContextualStream& stream);
protected:
	virtual bool		InitialiseImpl(); // from Component::I_GameComponent
private:
	void				Serialise(Sys::C_ContextualStream& stream) const;
	void				Deserialise(Sys::C_ContextualStream& stream);

	struct C_Input
	{
		C_Input():m_ActorId(0),m_Value(0){}
		C_Input(U32 actorId, bool value):m_ActorId(actorId),m_Value(value){}

		U32		m_ActorId;
		bool	m_Value;

		void		Serialise(Sys::C_ContextualStream& stream) const;
		void		Deserialise(Sys::C_ContextualStream& stream);
	};

	E_LogicGateType			m_GateType;
	Btl::Vector<C_Input>	m_Inputs;
	bool					m_Output;

};


// Logic Gate
class C_BoolExpressionEvaluator : public I_BoolExpressionEvaluator
{
public:
	C_BoolExpressionEvaluator();
	~C_BoolExpressionEvaluator();

	bool				OnMessage(const Messaging::C_MessageBase *pMessage );	// message handler

	static Component::C_ComponentTypeId		ClassType() {return Components::E_BooleanExpressionEval;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_BooleanExpressionEval;} // from Component::I_GameComponent

	const C_BoolExpressionEvaluatorCreator	*GetCreationData(void) const {return static_cast<const C_BoolExpressionEvaluatorCreator *>(GetCreator().GetData());}
private:
	bool	m_LastResult;
};

} // namespace Components

#endif

