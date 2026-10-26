#ifndef LOGIC_COMPONENTS_CREATE_H
#define LOGIC_COMPONENTS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/XtExpressionEval.h>


// forward decs
namespace Actors
{
	class C_GameActorCreate;
};

namespace Components
{

enum E_LogicGateType
{
	E_LogicGateType_AND = 0,
	E_LogicGateType_NAND,
	E_LogicGateType_OR,
	E_LogicGateType_NOR,
	E_LogicGateType_XOR,
};

// component creator structure
struct C_LogicGateInput
{
	Actors::C_GameActorCreate *	m_InputActor;
	bool						m_StartTrue;
};

class C_LogicGateComponentCreator : public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_LogicGate;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_LogicGate;}

	E_LogicGateType				m_GateType;
	XtArray<C_LogicGateInput>	m_Inputs;
};


class C_BoolExpressionEvaluatorCreator : public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_BooleanExpressionEval;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_BooleanExpressionEval;}

	Actors::C_XtParamBoolExpression		m_BoolExpression;

};

} //namespace Components

#endif

