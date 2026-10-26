#ifndef XT_CROWD_H
#define XT_CROWD_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Xt/XtGame.h>

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/XtCrowdPopulationListener.h>
#include <crowd_system/XtCrowdTextureListListener.h>

#include <resource_utils/CycleResourceDef.h>
#include <resource_utils/SkeletonResourceDef.h>

#include <Dynamic/Particle/Generic/XtInterface.h>
#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>

#include <btl/containers/list.h>
#include <btl/containers/vector.h>

namespace AnimatedCrowd
{
	class C_CrowdResources;
	class C_CrowdPopulation;
};

namespace AnimatedCrowd
{

void CreateXt(XtDb* database = NULL);
void DestroyXt();

typedef Btl::List<C_XtCrowdPopulationListener>	PopulationListenerList;
typedef Btl::Vector<C_XtCrowdPopulation*>		PopulationVector;

typedef Btl::List<C_XtCrowdTextureListListener>	TextureListListenerList;
typedef Btl::Vector<C_XtCrowdTextureList*>		TextureListVector;

//! C_XtCrowdEffectSystem
class C_XtCrowdEffectSystem
{
public:
	CrowdEffectSystemType									m_Type;
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_ParticleEvent;

	static void XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdEffectSystem* pEffectSystem, XtCallbackContext* pContext);

	bool Build(C_CrowdPopulation& population) const;
};

//! C_XtCrowdAnimationEventEffect
class C_XtCrowdAnimationEventEffect
{
public:
	XtHandle<DynamicEffect::Particle::XtInterface::Event>	m_ParticleEvent;
	XtSz8													m_AnimationEvent;
	XtSz8													m_JointName;

	static void XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdAnimationEventEffect* pAnimationEventEffect, XtCallbackContext* pContext);

	bool Build(C_CrowdPopulation& population) const;
};

//! C_XtCrowdCycle
class C_XtCrowdCycle: public ResourceUtils::C_XtCycleResourceDef
{
public:
	CrowdAction							m_Action;
	CrowdActionDirection				m_Direction;
};

//! C_XtCrowdSkeleton
class C_XtCrowdSkeleton: public ResourceUtils::C_XtSkeletonResourceDef
{
};

//! C_XtCrowdCycleGroupItem
class C_XtCrowdCycleGroupItem
{
public:
	XtHandle<C_XtCrowdCycle>			m_Cycle;
	XtF32								m_Weight;

	C_XtCrowdCycleGroupItem() : m_Cycle(NULL), m_Weight(1.f) {}

	static void XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdCycleGroupItem* pCycleGroupItem, XtCallbackContext* pContext);
};

//! C_XtCrowdCycleGroup
class C_XtCrowdCycleGroup
{
public:
	XtArray<C_XtCrowdCycleGroupItem>	m_Cycles;

	static void XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdCycleGroup* pCycleGroup, XtCallbackContext* pContext);
};

//! C_XtCrowdCharacter
class C_XtCrowdCharacter
{
public:
	CrowdBehaviourType									m_BehaviourType;
	XtSz8												m_BehaviourName;
	XtHandle<C_XtCrowdCycleGroup>						m_CycleGroup;

	static void XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdCharacter* pCharacter, XtCallbackContext* pContext);

	bool Build(C_CrowdPopulation& population) const;
};

//! C_XtCrowdPopulation
class C_XtCrowdPopulation
{
public:
	XtSz8												m_Name;
	XtHandle<C_XtCrowdSkeleton>							m_Skeleton;
	XtArray<XtHandle<C_XtCrowdCharacter> >				m_Characters;
	XtArray<XtHandle<C_XtCrowdAnimationEventEffect> >	m_AnimationEventEffects;
	XtArray<XtHandle<C_XtCrowdEffectSystem> >			m_EffectSystems;

	static void XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdPopulation* pPopulation, XtCallbackContext* pContext);

	bool Build(C_CrowdPopulation& population) const;
	void PostLoadResources(C_CrowdPopulation& population) const;

	static void AddListener(C_XtCrowdPopulationListener* pListener);
	static void RemoveListener(C_XtCrowdPopulationListener* pListener);
};

//! C_XtCrowdTextureList
class C_XtCrowdTextureList
{
public:
	CrowdTextureListUsage	m_Usage;
	XtArray<XtSz8>			m_Textures;

	static void XtCallback(XtEventType eventType, XtRecord* pRecord, C_XtCrowdTextureList* pTextureList, XtCallbackContext* pContext);

	bool Build(C_CrowdResources& resources) const;

	static void AddListener(C_XtCrowdTextureListListener* pListener);
	static void RemoveListener(C_XtCrowdTextureListListener* pListener);
};

/*
//! C_XtCrowdExcitementLevel
class C_XtCrowdExcitementLevel
{
public:
};

//! C_XtCrowdExcitementGroup
class C_XtCrowdExcitementGroup
{
public:
	XtArray<XtHandle<C_XtCrowdExcitementLevel> >	m_ExcitementLevels;
};
*/

}; // namespace AnimatedCrowd

#endif // XT_CROWD_H
