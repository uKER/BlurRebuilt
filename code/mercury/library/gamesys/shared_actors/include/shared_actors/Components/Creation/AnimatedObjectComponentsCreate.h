#ifndef ANIMATIONOBJECTCOMPONENTSCREATE_H
#define ANIMATIONOBJECTCOMPONENTSCREATE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <shared_actors/Components/SharedComponents.h>
#include <shared_actors/XtActorMessages.h>


namespace Actors
{
	class C_GameActorCreate;
}

namespace ResourceUtils
{
	struct C_GameModel;
	struct C_XtSkeletonResourceDef;
	struct C_XtCycleResourceDef;
}

namespace DynamicEffect
{
	namespace Particle
	{
		namespace XtInterface
		{
			struct Event;
		}
	}
}

namespace Animation
{
	class C_WindParams;
}

namespace SpecialFX
{
	class C_SpecialFX;
}

namespace Components
{

/**
Animation player creation info.
*/
class C_AnimationPlayerCreator : public Components::I_GameComponentCreator
{
public:

	class C_CycleListItem
	{
	public:
		XtHandle<ResourceUtils::C_XtCycleResourceDef>	m_Cycle;
		F32												m_Speed;
	};

	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimationPlayer; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimationPlayer; }

	class C_XtAnimationWindParams
	{
	public:
		XtVec3	m_Direction;
		F32		m_DirectionVariation;
		F32		m_Strength;
		F32		m_StrengthVariation;
		F32		m_VariationRate;

		void CopyToWindParams(Animation::C_WindParams& windParams) const;

	private:
	};
	
	S32							m_DefaultCycleIndex;
	XtArray<C_CycleListItem>	m_CycleList;
	
	XtHandle<ResourceUtils::C_XtSkeletonResourceDef>	m_Skeleton;
	XtHandle<C_XtAnimationWindParams>					m_WindParams;
	bool	m_StartPlaying;
	bool	m_AutoRestart;
	bool	m_Looping;
	bool	m_LoadCyclesLater;
	F32		m_StartTime;		// Random offset?
};

class C_AnimatedObjectVisualCreator: public Components::I_GameComponentCreator
{
public:
};

/**
Animated model creation info.
*/
class C_AnimatedModelCreator : public C_AnimatedObjectVisualCreator
{
public:
	C_AnimatedModelCreator():
		m_StartHidden(false),
		m_EnableCollision(false)
	{
	}

	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedModel; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedModel; }
	
	enum E_LightSamplerPositionType
	{
		E_LightSamplerPositionType_Actor	= 0,
		E_LightSamplerPositionType_BoundingSphereCentre,
		E_LightSamplerPositionType_Bone,
		E_LightSamplerPositionType_WorldPoint
	};
	
	struct LightSamplerPosition
	{
		virtual E_LightSamplerPositionType	GetType() const = 0;
	};
	
	struct ActorLsp: public LightSamplerPosition
	{
		virtual E_LightSamplerPositionType	GetType() const { return E_LightSamplerPositionType_Actor; }
		
		XtVec3	localOffset;
	};
	
	struct BoundingSphereCentreLsp: public LightSamplerPosition
	{
		virtual E_LightSamplerPositionType	GetType() const { return E_LightSamplerPositionType_BoundingSphereCentre; }
	};
	
	struct BoneLsp: public LightSamplerPosition
	{
		virtual E_LightSamplerPositionType	GetType() const { return E_LightSamplerPositionType_Bone; }
		
		XtSz8	boneName;
	};

	struct WorldPointLsp: public LightSamplerPosition
	{
		virtual E_LightSamplerPositionType	GetType() const { return E_LightSamplerPositionType_WorldPoint; }
		
		XtVec3	worldPoint;
	};
	
	struct ModelPart
	{
		XtHandle<ResourceUtils::C_GameModel> model;
		S32	modelIndex;
	};

	enum E_DropShadowPlacementType
	{
		E_DropShadowPlacementType_Invalid	= 0,
		E_DropShadowPlacementType_OnBone,
		E_DropShadowPlacementType_BetweenBones
	};

	class C_XtDropShadowPlacement
	{
	public:
		virtual E_DropShadowPlacementType	Type() const=0;
	};

	class C_XtPlaceOnBone: public C_XtDropShadowPlacement
	{
	public:
		virtual E_DropShadowPlacementType	Type() const	{ return E_DropShadowPlacementType_OnBone; }
		XtSz8	boneName;
	};

	class C_XtPlaceBetweenBones: public C_XtDropShadowPlacement
	{
	public:
		virtual E_DropShadowPlacementType	Type() const	{ return E_DropShadowPlacementType_BetweenBones; }
		XtSz8	boneName1;
		XtSz8	boneName2;
		F32		blend;
		bool	useLowestYPosition;
	};


	class C_XtDropShadow
	{
	public:
		C_XtDropShadowPlacement* placement;
		XtVec3	offset;
		F32		radius;
		F32		intensity;
	};

	class C_XtDropShadowList
	{
	public:
		XtArray<C_XtDropShadow>	dropShadows;

		U32	checksum;

		static void XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtDropShadowList* pDropShadowList, XtCallbackContext* pContext);
	};

	struct ModelInstance
	{
		XtArray<ModelPart>	modelParts;
		XtHandle<ResourceUtils::C_XtSkeletonResourceDef>	skeleton;
		XtSz8	namespacePrefix;
		F32		drawDistance;
		LightSamplerPosition* lightSamplerPosition;
		XtHandle<C_XtDropShadowList>	dropShadows;
		bool	disableMotionBlur;
		bool	enableScale;
	};

	bool					m_StartHidden;
	bool					m_EnableCollision;
	XtArray<ModelInstance>	m_ModelInstances;
};

/**
Animated particle effects creation info.
*/
class C_AnimatedParticleEffectsCreator: public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedParticleEffects; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedParticleEffects; }
	
	struct Effect
	{
		XtSz8		boneNamePattern;
		XtHandle<DynamicEffect::Particle::XtInterface::Event>	particleEvent;
		XtHandle<SpecialFX::C_SpecialFX>	specialFx;
		XtSz8		intensityAttributeName;
		F32			initialIntensity;
		F32			intensityScale;
		bool		spawnFromCollisionMesh;
	};
	
	XtArray<Effect>	m_Effects;
	bool			m_StartHidden;
};

/**
Animated event trigger creation info.
*/
class C_AnimatedEventTriggerCreator: public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedEventTrigger; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedEventTrigger; }

	class Event
	{
	public:
		XtSz8	curveNamePattern;
		F32		threshold;
		ActorMsg::C_XtDelayedSendMessageList	onGreaterThan;
		ActorMsg::C_XtDelayedSendMessageList	onLessThan;
	};
	
	XtArray<Event>							m_Events;
	ActorMsg::C_XtDelayedSendMessageList	m_AnimationFinishedMsgList;	// Messages to be sent when animation has finished.
};

class I_AnimatedObjectAnimationControllerCreator: public Components::I_GameComponentCreator
{
public:
	static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedObjectAnimationController; }
	virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedObjectAnimationController; }

	virtual U32						GetNumSlaves() const=0;
	virtual Actor::C_ActorHandle	GetSlaveActorHandle(U32 slaveIndex) const=0;
	virtual const CHAR8*			GetSlaveName(U32 slaveIndex) const=0;
};

/**
Xt version of the animated object actor's animation controller component creation info.
*/
class C_XtAnimatedObjectAnimationControllerCreator: public I_AnimatedObjectAnimationControllerCreator
{
public:
	//static Component::C_ComponentTypeId		ClassType()					{ return Components::E_AnimatedObjectAnimationController; }
	//virtual Component::C_ComponentTypeId	GetComponentTypeID() const	{ return Components::E_AnimatedObjectAnimationController; }

	virtual U32						GetNumSlaves() const;
	virtual Actor::C_ActorHandle	GetSlaveActorHandle(U32 slaveIndex) const;
	virtual const CHAR8*			GetSlaveName(U32 slaveIndex) const;

	class Object
	{
	public:
		XtSz8							name;
		Actors::C_GameActorCreate*		actor;
	};
	
	XtArray<Object>		m_Objects;
};

/**
Runtime version of the animated object actor's animation controller component creation info.
*/
class C_AnimatedObjectAnimationControllerCreator: public I_AnimatedObjectAnimationControllerCreator
{
public:
	/// Clears the slave list.
	void ClearSlaves();

	/// Adds a named slave to the slave list.
	void AddSlave(Actor::C_ActorHandle actorHandle, const Util::C_String& name);

	virtual U32						GetNumSlaves() const;
	virtual Actor::C_ActorHandle	GetSlaveActorHandle(U32 slaveIndex) const;
	virtual const CHAR8*			GetSlaveName(U32 slaveIndex) const;

private:

	class C_Slave
	{
	public:
		Actor::C_ActorHandle	actorHandle;
		Util::C_String			name;
	};

	Btl::Vector<C_Slave>	m_Slaves;
};

}//namespace Components

#endif
