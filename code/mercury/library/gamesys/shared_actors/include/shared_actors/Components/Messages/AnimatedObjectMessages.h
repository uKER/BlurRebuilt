#ifndef ANIMATEDOBJECTMESSAGES_H
#define ANIMATEDOBJECTMESSAGES_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorMessageHandle.h>
#include <misc_shared/uid/FourCC.h>
#include <misc_shared/messaging/MessageValue.h>
#include <shared_actors/XtActorMessages.h>

namespace ResourceUtils
{
	struct C_XtCycleResourceDef;
}

namespace ActorMsg
{

/**
Animation messages.
*/
enum E_AnimatedObjectMsg
{
	E_AnimatedObjectMsg_HiBits = FOURCC_HI20('MAOM'), 
	
	E_AnimatedObjectMsg_SetController,
	E_AnimatedObjectMsg_Update,
	E_AnimatedObjectMsg_SetNormalisedTime,
	E_AnimatedObjectMsg_SetTime,
	E_AnimatedObjectMsg_LoadCycle,
	E_AnimatedObjectMsg_PlayCycle,
	

	E_AnimatedObjectMsg_End, 
	E_AnimatedObjectMsg_Begin = E_AnimatedObjectMsg_HiBits + 1, 
	E_AnimatedObjectMsg_Count = E_AnimatedObjectMsg_End - E_AnimatedObjectMsg_Begin

};

typedef Actor::C_MessageActorHandle<E_AnimatedObjectMsg_Update>					C_AnimatedObjectUpdateMsg;
typedef Actor::C_MessageActorHandle<E_AnimatedObjectMsg_SetController>			C_AnimatedObjectSetControllerMsg;
typedef Messaging::C_MessageValue<F32, E_AnimatedObjectMsg_SetNormalisedTime>	C_AnimatedObjectSetNormalisedTimeMsg;
typedef Messaging::C_MessageValue<F32, E_AnimatedObjectMsg_SetTime>				C_AnimatedObjectSetTimeMsg;
typedef Messaging::C_MessageValue<S32, E_AnimatedObjectMsg_LoadCycle>			C_AnimatedObjectLoadCycleMsg;

class C_AnimatedObjectPlayCycleMsg: public Messaging::C_MessageDerived<C_AnimatedObjectPlayCycleMsg, E_AnimatedObjectMsg_PlayCycle>
{
public:
	S32		m_CycleIndex;
	F32		m_Speed;
	U32		m_LoopCount;
	F32		m_FadeInTime;
};


}//namespace ActorMsg

#endif