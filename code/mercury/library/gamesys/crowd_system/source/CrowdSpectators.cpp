#include "CrowdSystemPch.h"

#include <crowd_system/CrowdSpectators.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdBlock.h>
#include <crowd_system/CrowdBlockRef.h>
#include <crowd_system/CrowdCharacterRef.h>
#include <crowd_system/CrowdInstance.h>
#include <crowd_system/CrowdRandom.h>
#include <crowd_system/CrowdTextureCache.h>

#include <animation/Pose/PoseMatrixCache.h>

#include <game_resource/CrowdData.h>

#include <scenegraph/SceneGraph.h>

#include <system/Perf.h>

namespace AnimatedCrowd
{

C_CrowdSpectators::C_CrowdSpectators(C_CrowdResources* pResources)
	: m_pResources(pResources),
	m_NumUsedSpectators(0)
{
	m_UsedSpectators.ClearBits();

	m_FlagWaverBehaviourHash = Btl::Hash("Flag_Waver");
}

C_CrowdSpectators::~C_CrowdSpectators()
{
	Reinitialise();
}

void C_CrowdSpectators::Reinitialise()
{
	BIZ_ASSERT(m_NumUsedSpectators == 0);
}

void C_CrowdSpectators::CreateStaticSpectators(C_CrowdBlock& block, const C_CrowdTextureCache* pFlagTextureCache)
{
	if (block.GetBlockRefIndex() < 0)
		return;

	const C_CrowdBlockRef& blockRef = m_pResources->GetCrowdData()->GetCrowdBlockRef(block.GetBlockRefIndex());

	const CrowdDetailLevel& detailLevel(block.GetDetailLevel());

	SpectatorSpawnParams spawnParams;

	spawnParams.m_pResources = m_pResources;

	const GameResource::C_CrowdData* pCrowdData = m_pResources->GetCrowdData();

	if (pCrowdData)
	{
		const Btl::Vector<AnimatedCrowd::C_CrowdInstance>&		instances(pCrowdData->GetCrowdInstances());
		const Btl::Vector<U8>&									behaviourIndexes(pCrowdData->GetCrowdCharacterRefs());

		for (U32 i = 0, count = blockRef.GetNumInstances(); i < count; ++i)
		{
			U32 instanceIndex = blockRef.GetInstanceIndex(i);

			if (instanceIndex < static_cast<U32>(instances.Size()))
			{
				const AnimatedCrowd::C_CrowdInstance& instance(instances[instanceIndex]);

				spawnParams.m_Position = instance.GetPosition();
				spawnParams.m_Direction = instance.GetDirection();
				spawnParams.m_RandomSeed = instance.GetRandomSeed();
				spawnParams.m_InstanceIndex = instanceIndex;
				spawnParams.m_InstanceUniqueId = instance.GetUniqueId();

				spawnParams.m_BehaviourIndex = behaviourIndexes[instance.GetCharacterIndex()];

				spawnParams.m_ForceShared = (pCrowdData->GetCrowdBehaviourNameHash(spawnParams.m_BehaviourIndex) == m_FlagWaverBehaviourHash);
				spawnParams.m_FlagTextureIndex = -1;
				if (pFlagTextureCache && pFlagTextureCache->GetNumTextures() && (pCrowdData->GetCrowdBehaviourNameHash(spawnParams.m_BehaviourIndex) == m_FlagWaverBehaviourHash))
					spawnParams.m_FlagTextureIndex = AnimatedCrowd::C_Random::Instance().RandomU32Seeded(instanceIndex, pFlagTextureCache->GetNumTextures());

				spawnParams.m_UniqueAnimation = -1;
				spawnParams.m_SharedAnimation = -1;
				if (detailLevel == E_DETAIL_LEVEL_HIGH)
				{
					spawnParams.m_UniqueAnimation = m_pResources->GetUniqueAnimation(spawnParams.m_BehaviourIndex);
					if (spawnParams.m_UniqueAnimation < 0)
						spawnParams.m_SharedAnimation = m_pResources->GetSharedAnimation(spawnParams.m_BehaviourIndex);
				}
				else
					spawnParams.m_SharedAnimation = m_pResources->GetSharedAnimation(spawnParams.m_BehaviourIndex);
					
				block.AddSpectator(CreateStaticSpectator(spawnParams));
			}
		}
	}
}

void C_CrowdSpectators::DestroyStaticSpectators(C_CrowdBlock& block, const bool resetPaletteMapIndex, U8* pPaletteMap)
{
	// TODO: [rthomas]	block.GetNumSpectators() ok?
	for (U32 i = 0, count = /*block.GetNumSpectators()*/ CROWD_MAX_INSTANCES_PER_BLOCK; i < count; ++i)
	{
		const S32 spectatorIndex = block.GetSpectator(i);

		if (spectatorIndex < 0)
			continue;

		BIZ_ASSERT(spectatorIndex < CROWD_MAX_SPECTATORS);

		C_CrowdSpectator& spectator = m_Spectators[spectatorIndex];

		if (resetPaletteMapIndex && pPaletteMap)
			pPaletteMap[spectator.GetInstanceUniqueId()] = CROWD_MAX_TOTAL_ANIMATIONS + static_cast<U32>(spectator.GetBehaviourIndex()) + 1;

		DestroyStaticSpectator(spectatorIndex);
	}

	block.ClearSpectators();
}

bool C_CrowdSpectators::Flush()
{
	return true;
}

const U32 C_CrowdSpectators::GetNumSpectators() const
{
	return m_NumUsedSpectators;
}

const U32 C_CrowdSpectators::GetNumFreeSpectators() const
{
	return (CROWD_MAX_SPECTATORS - m_NumUsedSpectators);
}

const S32 C_CrowdSpectators::CreateStaticSpectator(const SpectatorSpawnParams& spawnParams)
{
	if (m_NumUsedSpectators >= CROWD_MAX_SPECTATORS)
		return -1;

	S32 firstFree = -1;

	for (U32 i = 0, count = CROWD_MAX_SPECTATORS; i < count; ++i)
	{
		if (!m_UsedSpectators[i])
		{
			firstFree = i;
			break;
		}
	}

	BIZ_ASSERT(firstFree >= 0);
	m_Spectators[firstFree].Create(spawnParams);
	m_UsedSpectators.SetBit(firstFree);

	++m_NumUsedSpectators;
	
	return firstFree;
}

void C_CrowdSpectators::DestroyStaticSpectator(const S32 spectatorIndex)
{
	if (spectatorIndex < 0)
		return;

	BIZ_ASSERT(m_pResources);
	BIZ_ASSERT(spectatorIndex < CROWD_MAX_SPECTATORS);

	C_CrowdSpectator& spectator = m_Spectators[spectatorIndex];

	S32 localAnimation = spectator.m_LocalAnimation;
	spectator.Destroy(*m_pResources);

	if (localAnimation >= 0)
	{
		C_CrowdAnimation* pAnimation = m_pResources->GetUniqueAnimationPool()->GetAnimation(localAnimation);
		if (pAnimation)
			pAnimation->Zombify();
		spectator.m_LocalAnimation = -1;
	}

	BIZ_ASSERT(m_NumUsedSpectators > 0);
	--m_NumUsedSpectators;

	m_UsedSpectators.ClearBit(spectatorIndex);
}

}; // namespace AnimatedCrowd
