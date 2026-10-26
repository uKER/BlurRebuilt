#ifndef ANIMATION_CONTROLLER_CREATE_H
#define ANIMATION_CONTROLLER_CREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/XtLevelAnimation.h>

namespace Components
{

class I_AnimationControllerCreator : public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType() {return Components::E_AnimationController;}
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const {return Components::E_AnimationController;}

	virtual U32						GetNumSlaves() const = 0;
	virtual Actor::C_ActorHandle	GetSlaveActorHandle(U32 slaveIndex) const = 0;
	//virtual const CHAR8*			GetSlaveName(U32 slaveIndex) const=0;

	Level::C_XtAnimationClip *	m_AnimationClip;	// Weak pointer to animation
};

// xt component creator structure
class C_AnimationControllerCreator : public I_AnimationControllerCreator
{
public:
	

	virtual U32						GetNumSlaves() const;
	virtual Actor::C_ActorHandle	GetSlaveActorHandle(U32 slaveIndex) const;

};


// dynamic component creator structure
class C_AnimationControllerDynamicCreator : public I_AnimationControllerCreator
{
public:

	virtual U32						GetNumSlaves() const;
	virtual Actor::C_ActorHandle	GetSlaveActorHandle(U32 slaveIndex) const;
};

}//namespace Components

#endif