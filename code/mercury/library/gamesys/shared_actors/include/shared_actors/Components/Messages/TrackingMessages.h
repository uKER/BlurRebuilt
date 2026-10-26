#ifndef TRACKINGMESSAGES_0FE212A0_BAD4_471D_B2CE_E9E2A1FAD7AB_H
#define TRACKINGMESSAGES_0FE212A0_BAD4_471D_B2CE_E9E2A1FAD7AB_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageHandle.h>
#include <actor_system/ActorMessageValue.h>
#include <misc_shared/uid/FourCC.h>


namespace ActorMsg
{

/// Messages used by tracking components.
enum E_TrackingMsg
{
	E_TrackingMsg_HiBits = FOURCC_HI20('STKM'), // Shared actor TracKing Messages

	E_TrackingMsg_Position,		///< see C_TrackingPositionMsg.
	E_TrackingMsg_Lost,			///< Sent when a tracked actor handle becomes invalid.
	E_TrackingMsg_Actor,		///< see C_TrackingActorMsg.

	E_TrackingMsg_End, 
	E_TrackingMsg_Begin = E_TrackingMsg_HiBits + 1, 
	E_TrackingMsg_Count = E_TrackingMsg_End - E_TrackingMsg_Begin
};

/**
@brief Sent when a tracked actor position is updated.
GetActorHandle gives the tracked actor and GetValue gives the position.
*/
typedef Actor::C_MessageActorValue<Maths::HVec3, E_TrackingMsg_Position> C_TrackingPositionMsg;

/**
@brief Sent to request that an actor be tracked.
GetActorHandle gives the actor to track.
*/
typedef Actor::C_MessageActorHandle<E_TrackingMsg_Actor> C_TrackingActorMsg;

} // namespace ActorMsg


#endif // TRACKINGMESSAGES_0FE212A0_BAD4_471D_B2CE_E9E2A1FAD7AB_H
