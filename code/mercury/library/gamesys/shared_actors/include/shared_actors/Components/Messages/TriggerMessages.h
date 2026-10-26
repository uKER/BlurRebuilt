#ifndef TRIGGERMESSAGES_5315EEDE_F7E6_44DA_AB0F_F905C2116872_H
#define TRIGGERMESSAGES_5315EEDE_F7E6_44DA_AB0F_F905C2116872_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageHandle.h>
#include <actor_system/ActorMessageValue.h>
#include <misc_shared/messaging/MessageValue.h>
#include <misc_shared/uid/FourCC.h>


namespace ActorMsg
{

/// Messages used by tracking components.
enum E_TriggerMsg
{
	E_TriggerMsg_HiBits = FOURCC_HI20('STGM'), // Shared actor TriGger Messages

	E_TriggerMsg_Enter,		///< see C_TriggerEnterMsg.
	E_TriggerMsg_Exit,		///< see C_TriggerExitMsg.
	E_TriggerMsg_Crossed,	///< see C_TriggerCrossedMsg.

	E_TriggerMsg_AddVolume,	///< see C_TriggerAddVolumeMsg
	E_TriggerMsg_VolumeParent,	///< see C_TriggerVolumeParentMsg

	E_TriggerMsg_SetMask,	///< see C_TriggerSetMaskMsg

	E_TriggerMsg_End, 
	E_TriggerMsg_Begin = E_TriggerMsg_HiBits + 1, 
	E_TriggerMsg_Count = E_TriggerMsg_End - E_TriggerMsg_Begin
};

/**
@brief Sent when an actor enters a trigger.
GetActorHandle gives the actor.
*/
typedef Actor::C_MessageActorHandle<E_TriggerMsg_Enter> C_TriggerEnterMsg;

/**
@brief Sent when an actor exits a trigger.
GetActorHandle gives the actor.
*/
typedef Actor::C_MessageActorHandle<E_TriggerMsg_Exit> C_TriggerExitMsg;

/**
@brief Sent when an actor crosses a trigger.
GetActorHandle gives the actor.
*/
typedef Actor::C_MessageActorHandle<E_TriggerMsg_Crossed> C_TriggerCrossedMsg;

/**
@brief Add an additional trigger volume actor to the trigger volume
GetActorHandle gives the actor which contains a trigger volume component.
*/
typedef Actor::C_MessageActorHandle<E_TriggerMsg_AddVolume> C_TriggerAddVolumeMsg;


/**
@brief Tell an additional trigger volume who his daddy is
GetActorHandle gives the parent trigger actor handle.
GetValue gives the parent's trigger mask
*/
typedef Actor::C_MessageActorValue<U32, E_TriggerMsg_VolumeParent> C_TriggerVolumeParentMsg;

/**
@brief Set the collision filter mask of the trigger volume
GetValue gives the new trigger mask
*/
typedef Messaging::C_MessageValue<U32, E_TriggerMsg_SetMask> C_TriggerSetMaskMsg;

} // namespace ActorMsg


#endif // TRIGGERMESSAGES_5315EEDE_F7E6_44DA_AB0F_F905C2116872_H
