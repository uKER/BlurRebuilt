#ifndef APP_CROWD_DATA_H
#define APP_CROWD_DATA_H

#ifdef _MSC_VER
#pragma once
#endif

#include <misc_shared/archive/archive.h>

#include <Crowd_System/CrowdCharacterRef.h>
#include <Crowd_System/CrowdInstance.h>
#include <Crowd_System/CrowdBlockRef.h>
#include <Crowd_System/CrowdBlockGrid.h>

namespace Aftk
{
	class C_Bounds;
};

namespace Pvs
{
	namespace Build
	{
		class PvsData;
	};
};

typedef Btl::Vector<U32>								C_CrowdBehaviourList;
typedef Btl::Vector<AnimatedCrowd::C_CrowdCharacterRef>	C_CrowdCharacterList;
typedef Btl::Vector<AnimatedCrowd::C_CrowdInstance>		C_CrowdInstanceList;
typedef Btl::Vector<AnimatedCrowd::C_CrowdBlockRef>		C_CrowdBlockList;
typedef Btl::Vector<AnimatedCrowd::C_CrowdSHNodeData>	C_CrowdInstanceSHNodeList;

//! C_DataCrowd
class C_DataCrowd : public Tools::ProcessGraph::C_Data
{
public:
	C_DataCrowd();
	~C_DataCrowd();

	const U32	Seed() const { return m_RandomSeed; }
	void		Seed(const U32 seed) { m_RandomSeed = seed; }

	// inherited from C_Data
	const Util::C_String
				GetName() const;

	const U32	GetCrowdBehavioursSize() const;
	const U32	GetCrowdCharactersSize() const;
	const U32	GetCrowdInstancesSize() const;

	void		AddCrowdBehaviour(const U32 behaviourNameHash);
	void		AddCrowdCharacter(const AnimatedCrowd::C_CrowdCharacterRef& character);
	void		AddCrowdInstance(const AnimatedCrowd::C_CrowdInstance& instance);
	void		AddCrowdInstanceSHNode(const AnimatedCrowd::C_CrowdSHNodeData& shNodeData);

	const AnimatedCrowd::C_CrowdCharacterRef*
				GetCrowdCharacter(const U32 index) const;

	void		BuildBlocks(const Pvs::Build::PvsData* pPVSData = NULL);
	void		SortBlocks(const S32 routeCount);
	void		ConstructBlockGrid(const F32 cellSize, const F32 cellRange);

	bool		Export( Sys::I_File* pFile, const Util::E_Platform platform ) const;

	const bool	ContainsData() const;

private:
	U32							m_RandomSeed;

	C_CrowdBehaviourList		m_BehaviourNameHashes;
	C_CrowdCharacterList		m_Characters;
	C_CrowdInstanceList			m_Instances;
	C_CrowdBlockList			m_Blocks;
	C_CrowdInstanceSHNodeList	m_InstanceSHNodes;
	
	Btl::Vector<Btl::Pair<S16, S16> >	m_RouteBlockAndInstanceCounts;

	AnimatedCrowd::C_CrowdBlockGrid		m_BlockGrid;
};

#endif

