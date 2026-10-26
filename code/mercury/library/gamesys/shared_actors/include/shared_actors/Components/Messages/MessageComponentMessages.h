#ifndef MESSAGE_COMPONENT_MESSAGES_H
#define MESSAGE_COMPONENT_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/uid/FourCC.h>
#include <shared_actors/SharedMessageValues.h>

namespace ActorMsg
{

enum E_MessageForwarderMsg
{
	E_MessageForwarderMsg_HiBits = FOURCC_HI20('MFMS'), 

	E_MessageForwarderMsg_SetSlot,
	E_MessageForwarderMsg_IncrementSlot,
	E_MessageForwarderMsg_DecrementSlot,
	E_MessageForwarderMsg_SetSelectionType,

	E_MessageForwarderMsg_End, 
	E_MessageForwarderMsg_Begin = E_MessageForwarderMsg_HiBits + 1, 
	E_MessageForwarderMsg_Count = E_MessageForwarderMsg_End - E_MessageForwarderMsg_Begin
};


typedef Messaging::C_MessageValue<U32, E_MessageForwarderMsg_SetSlot> C_MessageForwarderSetSlot;
typedef Messaging::C_MessageValue<U32, E_MessageForwarderMsg_SetSelectionType> C_MessageForwarderSetSelection;

}

#endif
