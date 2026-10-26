#include "SharedActorsPch.h"

#include <shared_actors/Components/Impl/LogicComponents.h>
#include <shared_actors/Components/Messages/LogicMessages.h>

#include <shared_actors/XtActorBase.h>

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", off)
#endif


namespace Components
{

C_LogicGate::C_LogicGate()
{
}
C_LogicGate::~C_LogicGate()
{

}

bool C_LogicGate::InitialiseImpl()
{
	const C_LogicGateComponentCreator	*pCreationData = GetCreationData();

	if(I_LogicGate::InitialiseImpl() == false)
		return false;

	m_GateType = pCreationData->m_GateType;

	// Setup initial input states
	for(S32 inputNo = 0; inputNo < pCreationData->m_Inputs.count; inputNo++)
	{
		m_Inputs.PushBack(C_Input(pCreationData->m_Inputs[inputNo].m_InputActor->m_SorceryUid, pCreationData->m_Inputs[inputNo].m_StartTrue));
	}

	return true;
}

bool C_LogicGate::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	// Handle logic Messages
	switch(pMessage->Type())
	{
	case ActorMsg::E_LogicMsg_SetInputTrue:
		UpdateInputValue(Messaging::MessageCast<ActorMsg::C_LogicSetInputTrue>(pMessage)->GetValue(), true);
		break;
	case ActorMsg::E_LogicMsg_SetInputFalse:
		UpdateInputValue(Messaging::MessageCast<ActorMsg::C_LogicSetInputFalse>(pMessage)->GetValue(), false);
		break;
	case ActorMsg::E_LogicMsg_SetInputActor:
		{
			U32 inputIndex = Messaging::MessageCast<ActorMsg::C_LogicSetInputActor>(pMessage)->GetFirst();

			if(inputIndex < (U32)m_Inputs.Size())
			{
				m_Inputs[inputIndex].m_ActorId = Messaging::MessageCast<ActorMsg::C_LogicSetInputActor>(pMessage)->GetSecond();
			}
		}
		break;
	case ActorMsg::E_LogicMsg_SetGateType:
		m_GateType = (E_LogicGateType)Messaging::MessageCast<ActorMsg::C_LogicSetGateType>(pMessage)->GetValue();
		break;
	default:
		return I_LogicGate::OnMessage(pMessage);
	}

	return true;
}

bool C_LogicGate::SetInputValue(U32 actorId, bool inputValue)
{
	// Search for inputs - linear search due to small list (2-4 items typical)
	for(S32 inputNo = 0; inputNo < m_Inputs.Size(); inputNo++)
	{
		if(m_Inputs[inputNo].m_ActorId == actorId)
		{
			m_Inputs[inputNo].m_Value = inputValue;
			return true;
		}
	}

	return false;	// Input not found
}

bool C_LogicGate::UpdateInputValue(U32 actorId, bool inputValue)
{
	if(SetInputValue(actorId, inputValue))
	{
		bool newOutput = EvaluateInputs();

		if(newOutput != m_Output)
		{
			m_Output = newOutput;
			if(m_Output == true)
				SendMessageToActor(ActorMsg::E_LogicMsg_OutputTrue);
			else
				SendMessageToActor(ActorMsg::E_LogicMsg_OutputFalse);
		}

		return true;
	}
	else
	{
		// input not found
		return false;
	}
}

bool C_LogicGate::EvaluateInputs(void) const
{
	bool bOutput = false;
	bool bInvert = (m_GateType == E_LogicGateType_NAND) || (m_GateType == E_LogicGateType_NOR);

	// Evaluation logic
	switch(m_GateType)
	{
	case E_LogicGateType_AND:
	case E_LogicGateType_NAND:
		{
			bOutput = true;

			for(S32 inputNo = 0; inputNo < m_Inputs.Size(); inputNo++)
			{
				bOutput &= m_Inputs[inputNo].m_Value;
			}
		}
	break;
	
	case E_LogicGateType_OR:
	case E_LogicGateType_NOR:
	{
			bOutput = false;

			for(S32 inputNo = 0; inputNo < m_Inputs.Size(); inputNo++)
			{
				bOutput |= m_Inputs[inputNo].m_Value;
			}
		}
		break;
	case E_LogicGateType_XOR:
		if(m_Inputs.Size() == 2)	// only works with 2 elements
		{
			bOutput = m_Inputs[0].m_Value ^ m_Inputs[1].m_Value;
		}
		break;

	}

	return bInvert ? !bOutput : bOutput;
}


// Serialisation

void C_LogicGate::Serialise(Sys::C_ContextualStream& stream) const
{
	stream.WriteAs<U32>(m_GateType);
	stream << m_Inputs.Size();
	for(S32 inputNo = 0; inputNo < m_Inputs.Size(); inputNo++)
	{
		stream << m_Inputs[inputNo];
	}
}

void C_LogicGate::Deserialise(Sys::C_ContextualStream& stream)
{
	U32 noInputs;
	stream >> noInputs;

	m_GateType = (E_LogicGateType)stream.ReadAs<U32>();
	m_Inputs.Clear();
	for(U32 inputNo = 0; inputNo < noInputs; inputNo++)
	{
		C_Input input;

		stream >> input;
		m_Inputs.PushBack(input);
	}
}

void C_LogicGate::SerialiseForSave(Sys::C_ContextualStream& stream) const
{
	I_LogicGate::SerialiseForSave(stream);
	Serialise(stream);
}

void C_LogicGate::DeserialiseForSave(Sys::C_ContextualStream& stream)
{
	I_LogicGate::DeserialiseForSave(stream);
	Deserialise(stream);
}

void C_LogicGate::SerialiseForSync(Sys::C_ContextualStream& stream) const
{
	I_LogicGate::SerialiseForSync(stream);
	Serialise(stream);
}

void C_LogicGate::DeserialiseForSync(Sys::C_ContextualStream& stream)
{
	I_LogicGate::DeserialiseForSync(stream);
	Deserialise(stream);
}

void C_LogicGate::C_Input::Serialise(Sys::C_ContextualStream& stream) const
{
	stream << m_ActorId;
	stream << m_Value;
}
void C_LogicGate::C_Input::Deserialise(Sys::C_ContextualStream& stream)
{
	stream >> m_ActorId;
	stream >> m_Value;
}

// Boolean Expression Evaluator
C_BoolExpressionEvaluator::C_BoolExpressionEvaluator()
{
}
C_BoolExpressionEvaluator::~C_BoolExpressionEvaluator()
{

}

bool C_BoolExpressionEvaluator::OnMessage(const Messaging::C_MessageBase *pMessage )
{
	switch(pMessage->Type())
	{
		case ActorMsg::E_LogicMsg_CalculateExpression:
		{
			m_LastResult = GetCreationData()->m_BoolExpression.GetBool(GetActor());

			if(m_LastResult == true)
				SendMessageToActor(ActorMsg::E_LogicMsg_OutputTrue);
			else
				SendMessageToActor(ActorMsg::E_LogicMsg_OutputFalse);
		}

		default:
			return I_BoolExpressionEvaluator::OnMessage(pMessage);

	}

	return true;
}

} // namespace Components

#ifdef SHARED_ACTORS_OPTIMIZE_OFF
#pragma optimize("", on)
#endif

