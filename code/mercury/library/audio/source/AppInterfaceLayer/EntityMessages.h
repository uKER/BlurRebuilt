#ifndef _ENTITY_MESSAGES_H_
#define _ENTITY_MESSAGES_H_


struct EntityActionInMessage
{
	EntityActionInMessage() { u32DestinationInstances = 1; }

	U32 u32DestinationInstances;	// each bit, if set, could indicate the message is to be sent to the instance interface with that index
	U32 u32DefinitionActionId;
};


#endif	//	_ENTITY_MESSAGES_H_
