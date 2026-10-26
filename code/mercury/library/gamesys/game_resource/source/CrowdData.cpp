#include "GameResourcePch.h"

#include <system/serialise/C_FileBlock.h>

#include <crowd_system/CrowdCharacterRef.h>
#include <crowd_system/CrowdInstance.h>
#include <crowd_system/CrowdBlock.h>
#include <crowd_system/CrowdBlockGrid.h>
#include <crowd_system/CrowdBlockRef.h>

#include <game_resource/GameResource.h>
#include <game_resource/CrowdData.h>

#include <system/Memory.h>

namespace GameResource
{

// C_CrowdData
C_CrowdData::C_CrowdData()
	: m_RandomSeed(0),
	m_pBlockGrid(NULL),
	m_ZDirection(0.f, 0.f, -1.f)
{
}

C_CrowdData::~C_CrowdData()
{
	delete m_pBlockGrid;
	m_pBlockGrid = NULL;
}

bool C_CrowdData::Read(Sys::I_File *pFile)
{
	bool success = true;

	RESOURCE_MEMORY_ID("CrowdData");

	{
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);

		S32 version;
		ar >> version;

		AnimatedCrowd::g_CrowdFileVersion = version;

		ar >> m_RandomSeed;

		if (version >= AnimatedCrowd::E_CrowdFileVersion_BehaviourHashes)
		{
			S32 behaviourNameHashListSize;
			ar >> behaviourNameHashListSize;

			if (behaviourNameHashListSize > 0)
			{
				m_BehaviourNameHashes.Resize(behaviourNameHashListSize);

				for (S32 i = 0, count = behaviourNameHashListSize; i < count; ++i)
					ar >> m_BehaviourNameHashes[i];
			}
		}
		else
		{
			m_BehaviourNameHashes.Resize(AnimatedCrowd::E_BEHAVIOUR_TYPE_COUNT);

			for (U32 i = 0, count = static_cast<U32>(AnimatedCrowd::E_BEHAVIOUR_TYPE_COUNT); i < count; ++i)
				m_BehaviourNameHashes[i] = Btl::Hash(AnimatedCrowd::GetBehaviourTypeName(static_cast<AnimatedCrowd::CrowdBehaviourType>(i)));
		}

		S32 characterListSize;
		ar >> characterListSize;

		if (characterListSize > 0)
		{
			m_CrowdCharacterRefs.Resize(characterListSize);

			AnimatedCrowd::C_CrowdCharacterRef characterRef;

			for (S32 i = 0, count = characterListSize; i < count; ++i)
			{
				ar >> characterRef;
				m_CrowdCharacterRefs[i] = static_cast<U8>(characterRef.GetBehaviourIndex());
			}
		}

		S32 instanceListSize;
		ar >> instanceListSize;

		if (instanceListSize > 0)
		{
			m_CrowdInstances.Resize(instanceListSize);

			for (S32 i = 0, count = instanceListSize; i < count; i++)
				ar >> m_CrowdInstances[i];
		}

		S32 blockListSize;
		ar >> blockListSize;

		if (blockListSize > 0)
		{
			m_CrowdBlockRefs.Resize(blockListSize);

			for (S32 i = 0, count = blockListSize; i < count; i++)
				ar >> m_CrowdBlockRefs[i];
		}

		if (version == AnimatedCrowd::E_CrowdFileVersion_Initial)
		{
			S32 sceneBlockMapSize;
			ar >> sceneBlockMapSize;

			S32 temp;

			for (S32 i = 0; i < sceneBlockMapSize; ++i)
			{
				ar >> temp;
				ar >> temp;
			}

			m_RouteBlockAndInstanceCounts.Resize(1);

			m_RouteBlockAndInstanceCounts[0].first = blockListSize;
			m_RouteBlockAndInstanceCounts[0].second = instanceListSize;
		}
		else // if (version <= E_CrowdFileVersion_BehaviourRefactor)
		{
			S32 numRoutes;
			ar >> numRoutes;

			if (numRoutes > 0)
			{
				m_RouteBlockAndInstanceCounts.Resize(numRoutes);

				for (S32 i = 0, count = numRoutes; i < count; ++i)
				{
					ar >> m_RouteBlockAndInstanceCounts[i].first;
					ar >> m_RouteBlockAndInstanceCounts[i].second;
				}
			}
		}

		if (!m_pBlockGrid)
			m_pBlockGrid = new AnimatedCrowd::C_CrowdBlockGrid();

		ar >> (*m_pBlockGrid);

		m_InstanceSHMap.Clear();

		if (version >= AnimatedCrowd::E_CrowdFileVersion_SHNodeData)
		{
			S32 shNodeDataSize;
			ar >> shNodeDataSize;

			if (shNodeDataSize > 0)
			{
				m_SHNodeData.Resize(shNodeDataSize);

				for (S32 i = 0, count = shNodeDataSize; i < count; ++i)
				{
					ar >> m_SHNodeData[i];

					m_InstanceSHMap.Insert(m_SHNodeData[i].GetInstanceIndex(), i);
				}
			}
		}
	}

	return success;
}

const S32 C_CrowdData::GetNumBehaviours() const
{
	return m_BehaviourNameHashes.Size();
}

const S32 C_CrowdData::GetNumCharacters() const
{
	return m_CrowdCharacterRefs.Size();
}

const S32 C_CrowdData::GetNumInstances() const
{
	return m_CrowdInstances.Size();
}

const S32 C_CrowdData::GetNumInstances(const U32 routeMask) const
{
	S32 numInstances = 0;

	for (S32 i = 0, count = static_cast<U32>(m_RouteBlockAndInstanceCounts.Size()); i < count; ++i)
	{
		if (routeMask & (1 << i))
			numInstances += m_RouteBlockAndInstanceCounts[i].second;
	}

	return numInstances;
}

const S32 C_CrowdData::GetNumBlocks() const
{
	return m_CrowdBlockRefs.Size();
}

const S32 C_CrowdData::GetNumSHNodes() const
{
	return m_SHNodeData.Size();
}

const U32 C_CrowdData::GetRandomSeed() const
{
	return m_RandomSeed;
}

const Maths::HVec3& C_CrowdData::GetZDirection() const
{
	return m_ZDirection;
}

const S32 C_CrowdData::GetNumBlocks(const U32 routeMask) const
{
	S32 numBlocks = 0;

	for (S32 i = 0, count = m_RouteBlockAndInstanceCounts.Size(); i < count; ++i)
	{
		if (routeMask & (1 << i))
			numBlocks += m_RouteBlockAndInstanceCounts[i].first;
	}
	
	return numBlocks;
}

const Btl::Vector<U32>& C_CrowdData::GetCrowdBehaviourNameHashes() const
{
	return m_BehaviourNameHashes;
}

#ifdef CFG_CONVERTER
const Btl::Vector<AnimatedCrowd::C_CrowdCharacterRef> &C_CrowdData::GetCrowdCharacterRefs() const
{
	return m_CrowdCharacterRefs;
}
#else
const Btl::Vector<U8> &C_CrowdData::GetCrowdCharacterRefs() const
{
	return m_CrowdCharacterRefs;
}
#endif

const Btl::Vector<AnimatedCrowd::C_CrowdInstance>& C_CrowdData::GetCrowdInstances() const
{
	return m_CrowdInstances;
}

const Btl::Vector<AnimatedCrowd::C_CrowdBlockRef>& C_CrowdData::GetCrowdBlockRefs() const
{
	return m_CrowdBlockRefs;
}

const U32 C_CrowdData::GetCrowdBehaviourNameHash(const U32 index) const
{
	BIZ_ASSERT(index < static_cast<U32>(m_BehaviourNameHashes.Size()));
	return m_BehaviourNameHashes[index];
}

#ifdef CFG_CONVERTER
const AnimatedCrowd::C_CrowdCharacterRef& C_CrowdData::GetCrowdCharacterRef(const U32 index) const
{
	BIZ_ASSERT(index < static_cast<U32>(m_CrowdCharacterRefs.Size()));
	return m_CrowdCharacterRefs[index];
}
#else
const U8 C_CrowdData::GetCrowdCharacterRef(const U32 index) const
{
	BIZ_ASSERT(index < static_cast<U32>(m_CrowdCharacterRefs.Size()));
	return m_CrowdCharacterRefs[index];
}
#endif

const AnimatedCrowd::C_CrowdInstance& C_CrowdData::GetCrowdInstance(const U32 index) const
{
	BIZ_ASSERT(index < static_cast<U32>(m_CrowdInstances.Size()));
	return m_CrowdInstances[index];
}

const AnimatedCrowd::C_CrowdBlockRef& C_CrowdData::GetCrowdBlockRef(const U32 index) const
{
	BIZ_ASSERT(index < static_cast<U32>(m_CrowdBlockRefs.Size()));
	return m_CrowdBlockRefs[index];
}

const F32* C_CrowdData::GetCrowdInstanceSH(const U32 instanceIndex) const
{
	Btl::OpenHashMap<U32, U32>::ConstIterator iter = m_InstanceSHMap.Find(instanceIndex);
	if (iter != m_InstanceSHMap.End())
	{
		BIZ_ASSERT(iter->GetValue() < static_cast<U32>(m_SHNodeData.Size()));
		return m_SHNodeData[iter->GetValue()].GetSHData();
	}

	return AnimatedCrowd::CROWD_DEFAULT_SH;
}

const Btl::OpenHashMap<U32, U32>& C_CrowdData::GetCrowdInstanceSHMap() const
{
	return m_InstanceSHMap;
}

const Btl::Vector<Btl::Pair<S16, S16> >& C_CrowdData::GetRouteBlockAndInstanceCounts() const
{
	return m_RouteBlockAndInstanceCounts;
}

const AnimatedCrowd::C_CrowdBlockGrid* C_CrowdData::GetBlockGrid() const
{
	return m_pBlockGrid;
}

} // namespace GameResource

