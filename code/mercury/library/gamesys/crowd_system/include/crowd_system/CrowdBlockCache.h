#ifndef CROWD_BLOCK_CACHE_H
#define CROWD_BLOCK_CACHE_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_SPU
#ifndef CFG_GOLD
#include <system/Debug/I_DebugRender.h>
#endif
#endif // PLATFORM_SPU

#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdBlock.h>

namespace SceneGraph
{
	class C_SceneGraph;
};

namespace AnimatedCrowd
{
	class C_CrowdResources;
	class C_CrowdSpectators;

	class C_CrowdBlock;
	class C_UpdateContext;

	class C_Event;

	class C_CrowdTextureCache;
};

namespace AnimatedCrowd
{

//! C_CrowdBlockCache

class ALIGN16 C_CrowdBlockCache
{
public:

#ifndef PLATFORM_SPU
	C_CrowdBlockCache();
	~C_CrowdBlockCache();

	void				Create(C_CrowdResources* pResources, C_CrowdBlock* pBlocks, C_CrowdSpectators* pSpectators);
	void				Destroy();

#ifndef CFG_GOLD
	void				DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, const C_CrowdBlock* pBlocks, const U32 frameStamp, const Maths::HVec3& zDir, const Maths::Mat43* pSkinningMatrices = NULL) const;
#endif

	void				EmptyBuckets();

	void				Update(C_CrowdBlock* pBlocks, const C_CrowdTextureCache* pFlagTextureCache, U8* pPaletteMap);
#endif // !PLATFORM_SPU

	void				UpdateCachedActors(const C_SpectatorUpdateInput& spectatorUpdateInput, C_SpectatorUpdateOutput& spectatorUpdateOutput);

#ifndef PLATFORM_SPU
	void				InsertVisibleBlock(C_CrowdBlock* pBlocks, const S32 blockIndex, const U32 priority);

	const U16			GetNumFreeSpectators() const;
	const U16			GetNumSpectators() const;
	const U16			GetNumActiveSpectators() const;
	const U16			GetNumCachedBlocks() const;
	const U16			GetNumLowDetailBlocks() const;
	const U16			GetNumHighDetailBlocks() const;

	bool				Flush(C_CrowdBlock* pBlocks);
#endif // !PLATFORM_SPU

private:

#ifndef PLATFORM_SPU
	void				InsertBlock(C_CrowdBlock& block, const U32 priorityIndex, const C_CrowdTextureCache* pFlagTextureCache);
	void				RemoveBlock(C_CrowdBlock& block, C_CrowdUniqueAnimationPool* pUniqueAnimationPool, C_CrowdSharedAnimationPool* pSharedAnimationPool,
							const bool resetPaletteMapIndex = false, U8* pPaletteMap = NULL);

	void				LinkAfterBlock(C_CrowdBlock* pBlocks, const S32 blockIndex, const S32 otherIndex);
	void				UnlinkBlock(C_CrowdBlock* pBlocks, const S32 blockIndex);

	const S32			GetHead() const;
	const S32			GetTail() const;
#endif // !PLATFORM_SPU

	C_CrowdBlock		m_Sentinel;

	C_CrowdResources*	m_pResources;
	C_CrowdBlock*		m_pBlocks;
	C_CrowdSpectators*	m_pSpectators;

	U16					m_NumStaticSpectators;
	U16					m_MaxStaticSpectators;
	U16					m_NumActiveSpectators;
	
	U16					m_NumCachedBlocks;
	U16					m_NumHighDetailBlocks;
	U16					m_NumLowDetailBlocks;

	S16					m_Buckets[CROWD_BLOCK_CACHE_BUCKET_COUNT + 1];
};

}; // namespace AnimatedCrowd

#endif // CROWD_BLOCK_CACHE_H
