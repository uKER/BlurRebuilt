#ifndef LOGIC_MESSAGES_H
#define LOGIC_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/uid/FourCC.h>
#include <misc_shared/Messaging/MessageValue.h>
#include <misc_shared/Messaging/MessageValue2Tuple.h>
//#include <shared_actors/SharedMessageValues.h>
//#include <shared_actors/Components/Creation/LogicComponentsCreate.h>

namespace ActorMsg
{
enum E_LogicMsg
{
	E_LogicMsg_HiBits = FOURCC_HI20('MLGK'), 

	E_LogicMsg_SetInputTrue,
	E_LogicMsg_SetInputFalse,
	E_LogicMsg_SetInputActor,
	E_LogicMsg_SetGateType,
	E_LogicMsg_CalculateExpression,

	E_LogicMsg_OutputTrue,
	E_LogicMsg_OutputFalse,

	E_LogicMsg_End, 
	E_LogicMsg_Begin = E_LogicMsg_HiBits + 1, 
	E_LogicMsg_Count = E_LogicMsg_End - E_LogicMsg_Begin

};


typedef Messaging::C_MessageValue<U32, E_LogicMsg_SetInputTrue>		C_LogicSetInputTrue;
typedef Messaging::C_MessageValue<U32, E_LogicMsg_SetInputFalse>	C_LogicSetInputFalse;
typedef Messaging::C_MessageValue2Tuple<U32,U32, E_LogicMsg_SetInputActor>	C_LogicSetInputActor;
typedef Messaging::C_MessageValue<U32, E_LogicMsg_SetGateType>		C_LogicSetGateType;


}

#endif