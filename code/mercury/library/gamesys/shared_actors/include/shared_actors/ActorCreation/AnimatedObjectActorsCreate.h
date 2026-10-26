#ifndef ANIMATEDOBJECTACTORSCREATE_H
#define ANIMATEDOBJECTACTORSCREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/Creation/AnimatedObjectComponentsCreate.h>
#include <shared_actors/SharedActorTypes.h>
#include <shared_actors/XtActorBase.h>

namespace Components
{
	class C_MovementCreateInfo;
}

namespace Actors
{

class C_AnimatedObjectActorCreate : public C_PosActorCreate
{
public:
	static const Actor::C_ActorTypeId ClassType()			{ return E_ActorTypeId_AnimatedObjectActor; }
	virtual const Actor::C_ActorTypeId GetType() const	{ return E_ActorTypeId_AnimatedObjectActor; }

	virtual void	DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn); // from Actor::C_ActorCreationData

public:
	Components::C_AnimationPlayerCreator			m_AnimationPlayer;
	Components::C_XtAnimatedObjectAnimationControllerCreator	m_SlaveController;
	Components::C_AnimatedObjectVisualCreator*		m_pVisual;
	Components::C_MovementCreateInfo*				m_pMovement;
	Components::C_AnimatedParticleEffectsCreator*	m_pParticleEffects;
	Components::C_AnimatedEventTriggerCreator*		m_pAnimatedEventTrigger;
	
};

}//namespace Actors

#endif
