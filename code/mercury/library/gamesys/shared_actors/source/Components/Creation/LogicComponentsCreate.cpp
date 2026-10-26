#include "SharedActorsPch.h"

#include <shared_actors/Components/Creation/LogicComponentsCreate.h>
#include <actor_system/Actor.h>
#include <shared_actors/XtActorBase.h>

XT_TYPE(Components::E_LogicGateType, "ELogicGateType")
	XT_OPTION(Components::E_LogicGateType_AND,	"AND")
	XT_OPTION(Components::E_LogicGateType_NAND,	"NAND")
	XT_OPTION(Components::E_LogicGateType_OR,	"OR")
	XT_OPTION(Components::E_LogicGateType_NOR,	"NOR")
	XT_OPTION(Components::E_LogicGateType_XOR,	"XOR")
XT_END_TYPE

// component creator structure
XT_TYPE(Components::C_LogicGateInput, "LogicGateInput")
	XT_FIELD(m_InputActor,	"InputActor")
	XT_FIELD(m_StartTrue,	"StartTrue")
XT_END_TYPE

XT_TYPE(Components::C_LogicGateComponentCreator, "LogicGateComp")
XT_BASE(Components::I_GameComponentCreator)
	XT_FIELD(m_GateType,	"Type")
	XT_FIELD(m_Inputs,	"Inputs")
XT_END_TYPE

XT_TYPE(Components::C_BoolExpressionEvaluatorCreator, "BoolExpressionEvalComp")
XT_BASE(Components::I_GameComponentCreator)
XT_FIELD(m_BoolExpression,	"Expression")
XT_END_TYPE


XT_LIBRARY(LogicComponentsCreation)
// Logic Component Messages
XT_DEPENDS_ON(LogicComponentMessages)

XT_REGISTER_TYPE(Components::E_LogicGateType)
XT_REGISTER_TYPE(Components::C_LogicGateInput)
XT_REGISTER_TYPE(Components::C_LogicGateComponentCreator)
XT_REGISTER_TYPE(Components::C_BoolExpressionEvaluatorCreator)
XT_END_LIBRARY