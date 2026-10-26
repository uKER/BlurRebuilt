#ifndef CROWD_SPECTATOR_UPDATE_DATA_H
#define CROWD_SPECTATOR_UPDATE_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <Core/Containers/String.h>

#include <crowd_system/CrowdActionSelect.h>
#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdEffectItem.h>
#include <crowd_system/CrowdFlagInstanceData.h>

#include <btl/containers/vector.h>

namespace Animation
{
	class C_PoseFormat;
	class C_Skeleton;
	class C_StringId;
};

namespace AnimatedCrowd
{
	class C_CrowdBlock;
	class C_CrowdBlockCache;
	class C_Random;
	class C_CrowdResources;
	class C_CrowdSharedAnimationPool;
	class C_CrowdSpectator;
	class C_CrowdUniqueAnimationPool;
};

namespace AnimatedCrowd
{

class ALIGN16 C_SpectatorUpdateInput
{
public:
	C_SpectatorUpdateInput()
		: m_pPoseFormat(NULL),
		m_pSkeleton(NULL),
		m_pEventStringIds(NULL),
		m_NumEvents(0),
		m_fDeltaTime(0.f),
		m_FlagBehaviourIndex(-1),
		m_pAnimationEventEffectJointIndexes(NULL),
		m_pActionSelect(NULL),
		m_pCycleGroupDataList(NULL),
		m_GlobalMood(static_cast<U32>(E_CROWD_MOOD_EXCITEMENT_LOW)),
		m_NumBlocks(0)
	{}

	Maths::Vector3					m_ZDir;									// No need to fetch
	const Animation::C_PoseFormat*	m_pPoseFormat;							// No need to fetch
	const Animation::C_Skeleton*	m_pSkeleton;							// No need to fetch
	const Animation::C_StringId*	m_pEventStringIds;						// No need to fetch
	U32								m_NumEvents;							// No need to fetch
	F32								m_fDeltaTime;							// No need to fetch
	const Btl::Vector<U8, Btl::VectorParameters_Static<CROWD_MAX_EVENT_EFFECTS> >*
									m_pAnimationEventEffectJointIndexes;	// Fetched
	const C_CrowdActionSelect*		m_pActionSelect;						// Fetched
	const Btl::Vector<CrowdCycleGroupData, Btl::VectorParameters_Static<CROWD_MAX_SHARED_ANIMATIONS> >*
									m_pCycleGroupDataList;					// Fetched
	U16								m_FlagBehaviourIndex;					// No need to fetch
	U16								m_GlobalMood;							// No need to fetch
	U32								m_NumBlocks;							// No need to fetch
};

class ALIGN16 C_SpectatorUpdateOutput
{
public:
	C_SpectatorUpdateOutput()
		: m_pBlockCache(NULL),
		m_pPaletteMap(NULL),
		m_pUniqueAnimationPool(NULL),
		m_pSharedAnimationPool(NULL),
		m_pEffectSpawnData(NULL),
		m_pFlagInstanceData(NULL),
		m_pRandom(NULL),
		m_pBlocks(NULL),
		m_pSpectators(NULL)
	{}

	C_CrowdBlockCache*				m_pBlockCache;									// Fetch and store!
	U8*								m_pPaletteMap;									// Fetch and store!
	C_CrowdUniqueAnimationPool*		m_pUniqueAnimationPool;							// Fetch and store!
	C_CrowdSharedAnimationPool*		m_pSharedAnimationPool;							// Fetch and store!
	EffectSpawnData*				m_pEffectSpawnData;								// Fetch and store!
	FlagInstanceData*				m_pFlagInstanceData;							// Fetch and store!
	AnimatedCrowd::C_Random*		m_pRandom;										// Fetch but don't store?
	C_CrowdBlock*					m_pBlocks;										// Fetch and store!
	C_CrowdSpectator*				m_pSpectators;									// Fetch and store!
};

}; // namespace AnimatedCrowd

#endif // CROWD_SPECTATOR_UPDATE_DATA_H
