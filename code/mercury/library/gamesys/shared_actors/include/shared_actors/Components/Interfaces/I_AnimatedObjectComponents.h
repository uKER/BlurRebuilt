#ifndef I_ANIMATIONSTATECOMPONENT_H
#define I_ANIMATIONSTATECOMPONENT_H

#ifdef _MSC_VER
#pragma once
#endif

#include <actor_system/ActorComponent.h>
#include <animation/common/BlendModes.h>
#include <shared_actors/components/SharedComponents.h>

namespace Animation
{
	class C_Pose;
	class C_PoseMatrixCache;
	class C_Skeleton;
	class C_Cycle;
	class C_EvaluationState;
	class C_WindParams;
}

namespace GameResource
{
	class C_ModelElement;
}

namespace Components
{

/**
Interface for animation system based animation controllers.
*/
class I_AnimationStateComponent : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType()			{ return Components::E_IF_AnimationState; }
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const	{ return Components::E_IF_AnimationState; }
	
	//
	// Read only access to animation state.
	//
	
	/// Returns the current animation pose.
	virtual const Animation::C_Pose& GetPose() const = 0;
	
	/// Returns the current animation pose matrix cache.
	virtual const Animation::C_PoseMatrixCache& GetPoseMatrixCache() const = 0;
	
	/// Returns the skeleton used by the animation pose, or NULL if all transforms are in world space.
	virtual const Animation::C_Skeleton* GetSkeleton() const = 0;

	/// Returns optional wind parameters used for adding wind to dynamic joints.
	virtual const Animation::C_WindParams* GetWindParams() const	{ return NULL; }

	/// Returns true if animation state is changing.
	virtual bool IsAnimating() const=0;
	
	//
	// Warning: the following functions do not guarantee that their returned objects are in a suitable
	// state for reading and should therefore be avoided in most cases.
	//
	
	/// Returns the current animation pose without synchronising active multi-threaded update tasks.
	virtual const Animation::C_Pose& GetPoseNoSync() const = 0;
	
	/// Returns the current animation pose matrix cache without synchronising active multi-threaded update tasks.
	virtual const Animation::C_PoseMatrixCache& GetPoseMatrixCacheNoSync() const = 0;
};

/**
Interface for animation system driven visuals.
*/
class I_AnimatedObjectVisualComponent : public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType()			{ return Components::E_IF_AnimatedObjectVisual; }
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const	{ return Components::E_IF_AnimatedObjectVisual; }
	
	/// Finds the first model element with the given name.
	virtual const GameResource::C_ModelElement* FindModelElement(const CHAR8* elementName) const	{ return NULL; }
		
private:
};

/**
Interface for animation system driven particle effects.
*/
class I_AnimatedParticleEffectsComponent: public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType()			{ return Components::E_IF_AnimatedParticleEffects; }
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const	{ return Components::E_IF_AnimatedParticleEffects; }
private:

};

/**
Interface for animation system driven event triggers.
*/
class I_AnimatedEventTriggerComponent: public Actor::I_ActorComponent
{
public:
	static Component::C_ComponentInterfaceId	InterfaceType()			{ return Components::E_IF_AnimatedEventTrigger; }
	virtual Component::C_ComponentInterfaceId	GetInterfaceID() const	{ return Components::E_IF_AnimatedEventTrigger; }

};


}//namespace Components

#endif

