#ifndef ANIMATION_ACTORS_CREATE_H
#define ANIMATION_ACTORS_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/Creation/AnimationControllerCreate.h>
#include <shared_actors/SharedActorTypes.h>


namespace Actors
{

class C_AnimationPlaybackActorCreate : public C_PosActorCreate
{
public:
	static const Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_AnimationPlaybackActor;}
	virtual const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_AnimationPlaybackActor;}

	virtual void	DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn); // from Actor::C_ActorCreationData

public:
	Components::C_AnimationControllerCreator	m_AnimationController;
};

class C_AnimationSlaveControllerCreate : public C_PosActorCreate
{
public:
	static const Actor::C_ActorTypeId ClassType() {return E_ActorTypeId_AnimationSlaveController;}
	virtual const Actor::C_ActorTypeId GetType() const {return E_ActorTypeId_AnimationSlaveController;}

	virtual void	DoSorceryCallback(Component::F_ComponentCreatorSorceryCallback pCallbackFn); // from Actor::C_ActorCreationData

public:
	Components::C_AnimationControllerCreator	m_AnimationController;
};

}//namespace Actors

#endif