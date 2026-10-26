#ifndef CROWD_BLOCK_H
#define CROWD_BLOCK_H

#ifdef _MSC_VER
#pragma once
#endif

#ifndef PLATFORM_SPU
#include <Core/Containers/String.h>
#endif // !PLATFORM_SPU

#include <crowd_system/CrowdDefines.h>

#include <btl/containers/Vector.h>

#ifndef PLATFORM_SPU
#ifndef CFG_GOLD
#include <system/Debug/I_DebugRender.h>
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU

namespace SceneGraph
{
	class C_SceneGraph;
};

namespace AnimatedCrowd
{
	class C_CrowdBlockRef;
	class C_CrowdResources;
	class C_CrowdSpectator;
	class C_UpdateContext;
	class C_CrowdAnimation;
	class C_Event;
	class C_SpectatorUpdateInput;
	class C_SpectatorUpdateOutput;
	class C_CrowdSharedAnimationPool;
	class C_CrowdUniqueAnimationPool;
};

namespace Animation
{
	class C_Pose;
	class C_Skeleton;
};

namespace AnimatedCrowd
{

//! C_CrowdBlock

class ALIGN16 C_CrowdBlock
{
public:

#ifndef PLATFORM_SPU
	C_CrowdBlock();
	~C_CrowdBlock();

	void					Create(const U32 blockRefIndex, const U32 blockRefType, const U32 numInstances);
	void					Destroy();

#ifndef CFG_GOLD
	void					DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, const C_CrowdResources* pResources, const C_CrowdSpectator* pSpectators,
								const Maths::HVec3& zDir, const Maths::Mat43* pSkinningMatrices = NULL) const;
	
	static void				DebugRenderSkeleton(const Animation::C_Pose* pPose, const Maths::HMat43& worldMatrix, const Animation::C_Skeleton* pSkeleton,
												const Maths::RGBA& colour, DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::Mat43* pSkinningMatrices = NULL);
	
	static void				DebugRenderBox(const Maths::HVec3& position, const Maths::HVec3& halfSize, const Maths::RGBA& colour, DbUtil::I_DebugRenderPtr debugRenderPtr);

	const Maths::RGBA		GetDebugColour() const;
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU

	void					Update(const C_SpectatorUpdateInput& input, C_SpectatorUpdateOutput& output);
	
#ifndef PLATFORM_SPU
	void					HandleEvent(const C_CrowdResources* pResources, C_CrowdSpectator* pSpectators, const C_Event& crowdEvent, const CrowdGlobalMood globalMood);

	void					AddSpectator(const S16 spectator);
	const S16				GetSpectator(const U32 index) const;

	void					ClearSpectators();
#endif // !PLATFORM_SPU
	
	const U32				GetNumSpectators() const;

#ifndef PLATFORM_SPU
	const U32				GetNumInstances() const; // Instances in the block ref (used to decide whether block will fit in cache)

	void					SetPrevInCache(const S16 block);
	const S16				GetPrevInCache() const;

	void					SetNextInCache(const S16 block);
	const S16				GetNextInCache() const;
#endif // !PLATFORM_SPU

	void					SetNextInBucket(const S16 block);
	const S16				GetNextInBucket() const;

	const S32				GetBlockRefIndex() const;

	void					IsCached(const bool isCached);
	const bool				IsCached() const;

	void					IsVisible(const bool isVisible);
	const bool				IsVisible() const;

#ifndef PLATFORM_SPU
	const bool				HasHighDetailSpectators() const;

	void					ForceDetailLevel(C_CrowdSpectator* pSpectators, const CrowdDetailLevel detailLevel,
								C_CrowdUniqueAnimationPool* pUniqueAnimationPool, C_CrowdSharedAnimationPool* pSharedAnimationPool);
#endif // !PLATFORM_SPU

	void					SetDetailLevel(const CrowdDetailLevel detailLevel);
	const CrowdDetailLevel	GetDetailLevel() const;

#ifndef PLATFORM_SPU
	void					UpdateFrameNumber(const U32 frameNumber);
	const U32				GetFrameNumber() const;

	bool					Flush();
#endif // !PLATFORM_SPU

private:
	S16		m_BlockRefIndex;
	U16		m_BlockRefType;

	S16		m_Spectators[CROWD_MAX_INSTANCES_PER_BLOCK];

	S16		m_NextInCache;
	S16		m_PrevInCache;

	S32		m_NextInBucket				: 15;

	U32		m_DetailLevel				: 2;
	U32		m_DestDetailLevel			: 2;
	U32		m_NumInstances				: 5;
	U32		m_NumSpectators				: 5;
	U32		m_IsCached					: 1;
	U32		m_IsVisible					: 1;
	U32		m_HasHighDetailSpectators	: 1;

	U32		m_FrameNumber;
};

}; // namespace AnimatedCrowd

#endif // CROWD_BLOCK_H
