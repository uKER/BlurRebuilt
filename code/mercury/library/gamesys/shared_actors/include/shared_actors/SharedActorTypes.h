#ifndef SHARED_ACTORTYPES_H
#define SHARED_ACTORTYPES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorDeclare.h>
#include <misc_shared/UID/FourCC.h>


namespace Actors
{
enum E_SharedActorTypeId
{
	E_SharedActorTypeId_HiBits = FOURCC_HI20('MSAT'), // Mercury Shared Actor Type

	E_ActorTypeId_NullActor,
	E_ActorTypeId_ProxyActor,
	E_ActorTypeId_Node,

	E_ActorTypeId_MessageScript,
	E_ActorTypeId_MessageForwarderList,
	E_ActorTypeId_MessageQueue,
	E_ActorTypeId_LogicGateMessageSender,
	E_ActorTypeId_BoolExpressionMessageSender,
	E_ActorTypeId_BoolExpressionMessageForwarder,
	E_ActorTypeId_Counter,
	E_ActorTypeId_CountdownTimer,

	E_ActorTypeId_Constraint,
	E_ActorTypeId_FixedConstraint,
	E_ActorTypeId_DistanceConstraint,
	E_ActorTypeId_SphericalConstraint,
	E_ActorTypeId_SliderConstraint,
	E_ActorTypeId_HingeConstraint,

	E_ActorTypeId_Group,			///< see C_GroupActor.

	E_ActorTypeId_AnimationPlaybackActor,
	E_ActorTypeId_AnimationSlaveController,
	
	E_ActorTypeId_AnimatedObjectActor,

	E_ActorTypeId_Spline
};

void RegisterSharedActorTypes(Actor::C_ActorFactory& actorFactory);

}	// namespace Actors

#endif
