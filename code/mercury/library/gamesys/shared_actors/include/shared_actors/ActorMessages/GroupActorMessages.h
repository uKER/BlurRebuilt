#ifndef GROUPACTORMESSAGES_A25EB3DF_75D0_4113_836C_C7768439FD49_H
#define GROUPACTORMESSAGES_A25EB3DF_75D0_4113_836C_C7768439FD49_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageHandle.h>
#include <actor_system/ActorMessageValue.h>
#include <misc_shared/uid/FourCC.h>


namespace ActorMsg
{

/// Messages used with Group actors.
enum E_GroupMsg
{
	E_GroupMsg_HiBits = FOURCC_HI20('GRUP'), 

	E_GroupMsg_AddActor,		///< see C_GroupAddActorMsg.
	E_GroupMsg_RemoveActor,		///< see C_GroupRemoveActorMsg.
	E_GroupMsg_Inherit,			///< see C_GroupInheritMsg.
	E_GroupMsg_EnableMessages,	///< Group will forward messages to members.
	E_GroupMsg_DisableMessages,	///< Group will not forward messages to members.
	E_GroupMsg_DeleteActors,	///< Group deletes actors but not itself
	E_GroupMsg_SetForwardingVolume,	///< Set a volume to use to forward messages

	E_GroupMsg_CreateActorList,	///< Tell group to create it's actor list
	E_GroupMsg_NotifyActorListCreated,	///< Tell the actor that the actor list has been created

	E_GroupMsg_End, 
	E_GroupMsg_Begin = E_GroupMsg_HiBits + 1, 
	E_GroupMsg_Count = E_GroupMsg_End - E_GroupMsg_Begin
};


/// @brief Sent to group to add an actor.
/// GetActorHandle() gives the actor to add.
typedef Actor::C_MessageActorHandle<E_GroupMsg_AddActor> C_GroupAddActorMsg;

/// @brief Sent to group to remove an actor.
/// GetActorHandle() gives the actor to remove.
typedef Actor::C_MessageActorHandle<E_GroupMsg_RemoveActor> C_GroupRemoveActorMsg;

/// @brief Broadcast to put the child into the same groups as the parent.
/// GetActorHandle() gives the parent actor.
/// GetValue() gives the child actor.
typedef Actor::C_MessageActorValue<Actor::C_ActorHandle, E_GroupMsg_Inherit> C_GroupInheritMsg;

/// @brief Sent to group to restrict messages to only the actors in a volume.
/// GetActorHandle() gives the actor to use as the volume.
typedef Actor::C_MessageActorHandle<E_GroupMsg_SetForwardingVolume> C_GroupSetForwardingVolumeMsg;

} // namespace ActorMsg


#endif // GROUPACTORMESSAGES_A25EB3DF_75D0_4113_836C_C7768439FD49_H
