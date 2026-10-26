#ifndef GAME_RESOURCE_CROWD_DATA_H
#define GAME_RESOURCE_CROWD_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <system/Serialise.h>

#include <btl/containers/vector.h>
#include <btl/containers/map.h>
#include <btl/containers/OpenHashMap.h>

namespace AnimatedCrowd
{
	class C_CrowdCharacterRef;
	class C_CrowdInstance;
	class C_CrowdBlockRef;
	class C_CrowdBlockGrid;
	class C_CrowdSHNodeData;
};

namespace GameResource
{

//! C_CrowdData
class C_CrowdData
{
public:
	C_CrowdData();
	~C_CrowdData();

	bool Read( Sys::I_File* pFile );

	const S32	GetNumBehaviours() const;
	const S32	GetNumCharacters() const;
	const S32	GetNumInstances() const;
	const S32	GetNumInstances(const U32 routeMask) const;
	const S32	GetNumBlocks() const;
	const S32	GetNumBlocks(const U32 routeMask) const;
	const S32	GetNumSHNodes() const;

	const U32	GetRandomSeed() const;
	const Maths::HVec3&
				GetZDirection() const;

	const Btl::Vector<U32>&									GetCrowdBehaviourNameHashes() const;
#ifdef CFG_CONVERTER
	const Btl::Vector<AnimatedCrowd::C_CrowdCharacterRef>&	GetCrowdCharacterRefs() const;
#else
	const Btl::Vector<U8>&									GetCrowdCharacterRefs() const;
#endif
	const Btl::Vector<AnimatedCrowd::C_CrowdInstance>&		GetCrowdInstances() const;
	const Btl::Vector<AnimatedCrowd::C_CrowdBlockRef>&		GetCrowdBlockRefs() const;

	const U32												GetCrowdBehaviourNameHash(const U32 index) const;
#ifdef CFG_CONVERTER
	const AnimatedCrowd::C_CrowdCharacterRef&				GetCrowdCharacterRef(const U32 index) const;
#else
	const U8												GetCrowdCharacterRef(const U32 index) const;
#endif
	const AnimatedCrowd::C_CrowdInstance&					GetCrowdInstance(const U32 index) const;
	const AnimatedCrowd::C_CrowdBlockRef&					GetCrowdBlockRef(const U32 index) const;

	const F32*												GetCrowdInstanceSH(const U32 instanceIndex) const;
	const Btl::OpenHashMap<U32, U32>&						GetCrowdInstanceSHMap() const;

	const Btl::Vector<Btl::Pair<S16, S16> >&				GetRouteBlockAndInstanceCounts() const;

	const AnimatedCrowd::C_CrowdBlockGrid*					GetBlockGrid() const;

	//void													DeleteGrid() const { SAFE_DELETE_PTR(m_pBlockGrid); }

private:

	Maths::HVec3	m_ZDirection;
	U32				m_RandomSeed;

	Btl::Vector<U32>								m_BehaviourNameHashes;
	Btl::Vector<AnimatedCrowd::C_CrowdInstance>		m_CrowdInstances;
	Btl::Vector<AnimatedCrowd::C_CrowdBlockRef>		m_CrowdBlockRefs;
	Btl::Vector<AnimatedCrowd::C_CrowdSHNodeData>	m_SHNodeData;
	Btl::OpenHashMap<U32, U32>						m_InstanceSHMap;
#ifdef CFG_CONVERTER
	Btl::Vector<AnimatedCrowd::C_CrowdCharacterRef>	m_CrowdCharacterRefs;
#else
	Btl::Vector<U8>									m_CrowdCharacterRefs;
#endif
	
	Btl::Vector<Btl::Pair<S16, S16> >				m_RouteBlockAndInstanceCounts;

	AnimatedCrowd::C_CrowdBlockGrid*				m_pBlockGrid;
};

} // namespace GameResource

#endif // GAME_RESOURCE_CROWD_DATA_H
