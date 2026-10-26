#include "SharedActorsPch.h"

#include <shared_actors/ActorCreation/AnimationActorsCreate.h>

XT_TYPE(Actors::C_AnimationPlaybackActorCreate, "AnimationPlaybackActor")
XT_BASE(Actors::C_PosActorCreate)
XT_FIELD(m_AnimationController, "AnimationController")
XT_END_TYPE

XT_TYPE(Actors::C_AnimationSlaveControllerCreate, "AnimationSlaveControllerActor")
XT_BASE(Actors::C_PosActorCreate)
XT_FIELD(m_AnimationController, "AnimationController")
XT_END_TYPE

namespace Actors
{

void C_AnimationPlaybackActorCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_AnimationController);
}

void C_AnimationSlaveControllerCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_AnimationController);
}

}//namespace Actors

