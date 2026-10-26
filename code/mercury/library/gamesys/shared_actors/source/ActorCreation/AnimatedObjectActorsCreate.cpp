#include "SharedActorsPch.h"

#include <shared_actors/ActorCreation/AnimatedObjectActorsCreate.h>
#include <shared_actors/Components/Creation/MovementComponentsCreate.h>

XT_TYPE(Actors::C_AnimatedObjectActorCreate, "AnimatedObjectActor")
XT_BASE(Actors::C_PosActorCreate)
XT_FIELD(m_AnimationPlayer, "AnimationPlayer")
XT_FIELD(m_pVisual, "Visual")
XT_FIELD(m_pMovement, "Movement")
XT_FIELD(m_pParticleEffects, "ParticleEffects")
XT_FIELD(m_pAnimatedEventTrigger, "EventTriggers")
XT_FIELD(m_SlaveController, "SlaveController")
XT_END_TYPE

namespace Actors
{


void C_AnimatedObjectActorCreate::DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn)
{
	C_GameActorCreate::DoSorceryCallback(pCallbackFn);
	DoComponentSorceryCallback(pCallbackFn, m_AnimationPlayer);
	DoComponentSorceryCallback(pCallbackFn, m_SlaveController);
	
	if (m_pVisual != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pVisual);

	if (m_pMovement != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pMovement);
		
	if (m_pParticleEffects != NULL)
		DoComponentSorceryCallback(pCallbackFn, *m_pParticleEffects);
		
	if (m_pAnimatedEventTrigger != NULL)
	{
		DoComponentSorceryCallback(pCallbackFn, *m_pAnimatedEventTrigger);
	}
}



}//namespace Actors

