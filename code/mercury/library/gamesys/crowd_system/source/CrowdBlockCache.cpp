#include "CrowdSystemPch.h"

#include <crowd_system/CrowdBlock.h>
#include <crowd_system/CrowdBlockCache.h>
#include <crowd_system/CrowdSpectatorUpdateData.h>
#ifndef PLATFORM_SPU
#include <crowd_system/CrowdSpectators.h>
#include <crowd_system/CrowdEvent.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdInstance.h>
#include <crowd_system/CrowdMaths.h>

#include <crowd_system/CrowdBlockRef.h>
#include <game_resource/CrowdData.h>

#include <system/Perf.h>
#endif // !PLATFORM_SPU

namespace AnimatedCrowd
{

#ifndef PLATFORM_SPU
#ifndef CFG_GOLD
Debug::Bool			g_DebugRenderBlocks("GameSys.CrowdSystem.World.DrawBlocks", false, Debug::Persist());
extern Debug::Bool	g_DebugRenderBlocksInstances;
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU

#ifndef PLATFORM_SPU
C_CrowdBlockCache::C_CrowdBlockCache()
	: m_pResources(NULL),
	m_pBlocks(NULL),
	m_pSpectators(NULL),
	m_NumStaticSpectators(0),
	m_NumActiveSpectators(0),
	m_NumCachedBlocks(0),
	m_NumHighDetailBlocks(0),
	m_NumLowDetailBlocks(0)
{
	m_MaxStaticSpectators = CROWD_MAX_SPECTATORS;

	m_Sentinel.Destroy();
	m_Sentinel.SetNextInCache(-1);
	m_Sentinel.SetPrevInCache(-1);

	EmptyBuckets();
}

C_CrowdBlockCache::~C_CrowdBlockCache()
{
	Destroy();
}

void C_CrowdBlockCache::Create(C_CrowdResources* pResources, C_CrowdBlock* pBlocks, C_CrowdSpectators* pSpectators)
{
	m_pResources = pResources;
	m_pBlocks = pBlocks;
	m_pSpectators = pSpectators;
}

void C_CrowdBlockCache::Destroy()
{
	EmptyBuckets();

	if (m_pBlocks)
	{
		S32 blockIndex = GetHead();

		while (blockIndex >= 0)
		{
			C_CrowdBlock& block = m_pBlocks[blockIndex];
			S32 nextIndex = block.GetNextInCache();

			RemoveBlock(block, m_pResources->GetUniqueAnimationPool(), m_pResources->GetSharedAnimationPool());
			UnlinkBlock(m_pBlocks, blockIndex);
			
			blockIndex = nextIndex;
		}
	}

	m_pBlocks = NULL;
}

#ifndef CFG_GOLD
Maths::RGBA ShadesOfGrey(const F32 greyScale)
{
	U8 colour = (U8)((1.f - greyScale) * 255.f);
	return Maths::RGBA(colour, colour, colour);
}

void C_CrowdBlockCache::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, const C_CrowdBlock* pBlocks, const U32 frameStamp, const Maths::HVec3& zDir, const Maths::Mat43* pSkinningMatrices) const
{
	if (!m_pResources || !pBlocks)
		return;

	for (U32 i = 0, count = CROWD_BLOCK_CACHE_BUCKET_COUNT; i <= count; ++i)
	{
		S32 blockIndex = m_Buckets[i];

		while (blockIndex >= 0)
		{
			const C_CrowdBlock& block = pBlocks[blockIndex];
			BIZ_ASSERT(block.GetBlockRefIndex() >= 0);
			
			const C_CrowdBlockRef& blockRef = m_pResources->GetCrowdData()->GetCrowdBlockRef(block.GetBlockRefIndex());

			if (g_DebugRenderBlocks)
			{
				debugRenderPtr->Draw3dBox(
					blockRef.GetPosition() - blockRef.GetExtents(),
					blockRef.GetPosition() + blockRef.GetExtents(),
					Maths::Constants<Maths::HMat43>::Identity(),
					block.GetDebugColour()
				);

				if (g_DebugRenderBlocksInstances)
				{
					for (U32 iInstance = 0, instanceCount = blockRef.GetNumInstances(); iInstance < instanceCount; ++iInstance)
					{
						const C_CrowdInstance& instance = m_pResources->GetCrowdData()->GetCrowdInstance(blockRef.GetInstanceIndex(iInstance));
						debugRenderPtr->Draw3dPoint(instance.GetPosition(), 0.25f, GetRouteColour(blockRef.GetRoute()));
					}
				}
			}

			block.DebugRender(debugRenderPtr, m_pResources, m_pSpectators->GetSpectators(), zDir, pSkinningMatrices);

			blockIndex = block.GetNextInBucket();
		}
	}
}
#endif // CFG_GOLD

void C_CrowdBlockCache::EmptyBuckets()
{
	// Empty buckets
	for (U32 i = 0, count = CROWD_BLOCK_CACHE_BUCKET_COUNT; i <= count; ++i)
		m_Buckets[i] = -1;
}

void C_CrowdBlockCache::Update(C_CrowdBlock* pBlocks, const C_CrowdTextureCache* pFlagTextureCache, U8* pPaletteMap)
{
	S32 insertIndex = GetTail();

	bool cacheFull = false;

	for (U32 i = 0, count = CROWD_BLOCK_CACHE_BUCKET_COUNT; i <= count; ++i)
	{
		S32 blockIndex = m_Buckets[i];

		while (blockIndex >= 0 && !cacheFull)
		{
			C_CrowdBlock& block = pBlocks[blockIndex];
			S32 nextIndex = block.GetNextInBucket();

			if (block.IsCached())
			{
				if (blockIndex == insertIndex)
					insertIndex = block.GetPrevInCache();

				UnlinkBlock(pBlocks, blockIndex);
				LinkAfterBlock(pBlocks, blockIndex, insertIndex);
			}
			else
			{
				while ((GetNumFreeSpectators() < block.GetNumInstances()) && (insertIndex != -1))
				{
					if (GetHead() == insertIndex)
						insertIndex = -1;

					BIZ_ASSERT(GetHead() >= 0);

					RemoveBlock(pBlocks[GetHead()], m_pResources->GetUniqueAnimationPool(), m_pResources->GetSharedAnimationPool(), true, pPaletteMap);
					UnlinkBlock(pBlocks, GetHead());
				}

				if (GetNumFreeSpectators() >= block.GetNumInstances())
				{
					InsertBlock(block, i, pFlagTextureCache);
					LinkAfterBlock(pBlocks, blockIndex, insertIndex);
				}
				else
				{
					cacheFull = true;
				}
			}

			blockIndex = nextIndex;
		}
	}
}
#endif // !PLATFORM_SPU

void C_CrowdBlockCache::UpdateCachedActors(const C_SpectatorUpdateInput& spectatorUpdateInput, C_SpectatorUpdateOutput& spectatorUpdateOutput)
{
	if (!spectatorUpdateOutput.m_pPaletteMap)
		return;

	BIZ_ASSERT(spectatorUpdateOutput.m_pBlocks);

	m_NumActiveSpectators = 0;
	m_NumCachedBlocks = 0;
	m_NumHighDetailBlocks = 0;
	m_NumLowDetailBlocks = 0;

	for (U32 i = 0, count = CROWD_BLOCK_CACHE_BUCKET_COUNT; i < count; ++i)
	{
		S32 blockIndex = m_Buckets[i];

		while (blockIndex >= 0)
		{
			C_CrowdBlock& block = spectatorUpdateOutput.m_pBlocks[blockIndex];

			if (block.IsCached())
			{
				block.Update(spectatorUpdateInput, spectatorUpdateOutput);
				m_NumActiveSpectators += block.GetNumSpectators();
				m_NumCachedBlocks++;

				m_NumHighDetailBlocks += (block.GetDetailLevel() == static_cast<U32>(E_DETAIL_LEVEL_HIGH));
				m_NumLowDetailBlocks += (block.GetDetailLevel() == static_cast<U32>(E_DETAIL_LEVEL_LOW));
			}

			blockIndex = block.GetNextInBucket();
		}
	}
}

#ifndef PLATFORM_SPU
void C_CrowdBlockCache::InsertVisibleBlock(C_CrowdBlock* pBlocks, const S32 blockIndex, const U32 priority)
{
	BIZ_ASSERT(pBlocks && blockIndex >= 0);

	if (priority <= CROWD_BLOCK_CACHE_BUCKET_COUNT)
	{
		C_CrowdBlock& block = pBlocks[blockIndex];

		block.SetNextInBucket(m_Buckets[priority]);
		
		m_Buckets[priority] = blockIndex;
	}
}

const U16 C_CrowdBlockCache::GetNumFreeSpectators() const
{
	return (m_NumStaticSpectators <= CROWD_MAX_SPECTATORS) ? CROWD_MAX_SPECTATORS - m_NumStaticSpectators : 0;
}

const U16 C_CrowdBlockCache::GetNumSpectators() const
{
	return m_NumStaticSpectators;
}

const U16 C_CrowdBlockCache::GetNumActiveSpectators() const
{
	return m_NumActiveSpectators;
}

const U16 C_CrowdBlockCache::GetNumCachedBlocks() const
{
	return m_NumCachedBlocks;
}

const U16 C_CrowdBlockCache::GetNumHighDetailBlocks() const
{
	return m_NumHighDetailBlocks;
}

const U16 C_CrowdBlockCache::GetNumLowDetailBlocks() const
{
	return m_NumLowDetailBlocks;
}

bool C_CrowdBlockCache::Flush(C_CrowdBlock* pBlocks)
{
	BIZ_ASSERT(pBlocks);

	for (U32 i = 0, count = CROWD_BLOCK_CACHE_BUCKET_COUNT; i <= count; ++i)
	{
		S32 blockIndex = m_Buckets[i];

		while (blockIndex >= 0)
		{
			C_CrowdBlock& block = pBlocks[blockIndex];

			if (block.IsCached())
			{
				if (!block.Flush())
					return false;
			}

			blockIndex = block.GetNextInBucket();
		}
	}

	return true;
}

void C_CrowdBlockCache::InsertBlock(C_CrowdBlock& block, const U32 priorityIndex, const C_CrowdTextureCache* pFlagTextureCache)
{
	block.IsCached(true);

	m_pSpectators->CreateStaticSpectators(block, pFlagTextureCache);

	m_NumStaticSpectators += block.GetNumSpectators();
}

void C_CrowdBlockCache::RemoveBlock(C_CrowdBlock& block, C_CrowdUniqueAnimationPool* pUniqueAnimationPool, C_CrowdSharedAnimationPool* pSharedAnimationPool,
									const bool resetPaletteMapIndex, U8* pPaletteMap)
{
	m_NumStaticSpectators -= block.GetNumSpectators();

	m_pSpectators->DestroyStaticSpectators(block, resetPaletteMapIndex, pPaletteMap);

	block.IsCached(false);

	block.ForceDetailLevel(m_pSpectators->GetSpectators(), E_DETAIL_LEVEL_LOW, pUniqueAnimationPool, pSharedAnimationPool);
}

void C_CrowdBlockCache::LinkAfterBlock(C_CrowdBlock* pBlocks, const S32 blockIndex, const S32 otherIndex)
{
	BIZ_ASSERT(pBlocks);

	C_CrowdBlock& block = (blockIndex >= 0) ? pBlocks[blockIndex] : m_Sentinel;
	C_CrowdBlock& other = (otherIndex >= 0) ? pBlocks[otherIndex] : m_Sentinel;

	block.SetNextInCache(other.GetNextInCache());
	block.SetPrevInCache(otherIndex);

	C_CrowdBlock& otherNext = (other.GetNextInCache() >= 0) ? pBlocks[other.GetNextInCache()] : m_Sentinel;

	otherNext.SetPrevInCache(blockIndex);
	other.SetNextInCache(blockIndex);
}

void C_CrowdBlockCache::UnlinkBlock(C_CrowdBlock* pBlocks, const S32 blockIndex)
{
	BIZ_ASSERT(pBlocks && blockIndex >= 0);

	C_CrowdBlock& block = pBlocks[blockIndex];

	C_CrowdBlock& blockPrev = (block.GetPrevInCache() >= 0) ? pBlocks[block.GetPrevInCache()] : m_Sentinel;
	blockPrev.SetNextInCache(block.GetNextInCache());

	C_CrowdBlock& blockNext = (block.GetNextInCache() >= 0) ? pBlocks[block.GetNextInCache()] : m_Sentinel;
	blockNext.SetPrevInCache(block.GetPrevInCache());
}

const S32 C_CrowdBlockCache::GetHead() const
{
	return m_Sentinel.GetNextInCache();
}

const S32 C_CrowdBlockCache::GetTail() const
{
	return m_Sentinel.GetPrevInCache();
}
#endif // !PLATFORM_SPU

}; // namespace AnimatedCrowd
