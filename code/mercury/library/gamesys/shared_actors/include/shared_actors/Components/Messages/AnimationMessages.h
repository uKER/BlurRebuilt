#ifndef ANIMATION_MESSAGES_H
#define ANIMATION_MESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageValue.h>
#include <misc_shared/uid/FourCC.h>
#include <shared_actors/SharedActorMessages.h>
#include <shared_actors/SharedMessageValues.h>
#include <shared_actors/XtLevelAnimation.h>

// Animation messages
// ------------------
namespace ActorMsg
{

enum E_AnimationMsg
{
	E_AnimationMsg_HiBits = FOURCC_HI20('MANM'), 

	E_AnimationMsg_SetTValue,				
	E_AnimationMsg_MoveToTValue,				
	E_AnimationMsg_SetAnimationController,

	E_AnimationMsg_End, 
	E_AnimationMsg_Begin = E_AnimationMsg_HiBits + 1, 
	E_AnimationMsg_Count = E_AnimationMsg_End - E_AnimationMsg_Begin

};

typedef Messaging::C_MessageValue<Level::C_AnimTValue, E_AnimationMsg_SetTValue> C_AnimationSetTValueMsg;
typedef Messaging::C_MessageValue<Level::C_AnimTValue, E_AnimationMsg_MoveToTValue> C_AnimationMoveToTValueMsg;
typedef Actor::C_MessageActorValue<S32,E_AnimationMsg_SetAnimationController> C_AnimationSetControllerMsg;

}//namespace ActorMsg

#endif