#include "CrowdSystemPch.h"

#include <crowd_system/CrowdBlock.h>
#include <crowd_system/CrowdSpectator.h>
#ifndef PLATFORM_SPU
#include <crowd_system/CrowdBlockRef.h>
#include <crowd_system/CrowdContext.h>
#include <crowd_system/CrowdInstance.h>
#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdEvent.h>
#endif // !PLATFORM_SPU
#include <crowd_system/CrowdSpectatorUpdateData.h>

#ifndef PLATFORM_SPU
#include <crowd_system/CrowdCharacterRef.h>

#include <animation/Pose/PoseMatrixCache.h>

#include <game_resource/CrowdData.h>

#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>

#include <animation/common/JointTransform.inl>

#ifndef CFG_GOLD
#include <animation/pose/Pose.h>
#include <animation/pose/SkeletonPose.h>
#include <animation/skeleton/Skeleton.h>
#endif

#include <scenegraph/SceneGraph.h>

#include <system/Perf.h>
#endif // !PLATFORM_SPU

namespace AnimatedCrowd
{

#ifndef PLATFORM_SPU
#ifndef CFG_GOLD
Debug::Bool						g_DebugRenderSpectators("GameSys.CrowdSystem.Spectators.Draw", false);
Debug::Bool						g_DebugRenderSkeleton("GameSys.CrowdSystem.Spectators.DrawSkeleton", false);
Debug::Bool						g_DebugRenderHighDetailOnly("GameSys.CrowdSystem.Spectators.HighDetailOnly", false, Debug::Persist());
Debug::Bool						g_DebugRenderMember("GameSys.CrowdSystem.Spectators.HighlightSpecificCharacter", false);
Debug::Int						g_DebugRenderBehaviourIndex("GameSys.CrowdSystem.Spectators.HighlightedCharacterIndex", 0);
Debug::Bool						g_DebugRenderSharedAnimation("GameSys.CrowdSystem.Spectators.HighlightThoseUsingSharedAnimation", false);
Debug::Int						g_DebugRenderSharedAnimationIndex("GameSys.CrowdSystem.Spectators.HighlightedSharedAnimationIndex", 0);
Debug::Bool						g_DebugRenderAnimationCount("GameSys.CrowdSystem.Spectators.DisplayHighlightedAnimCount", false);
Debug::Bool						g_DebugRenderCycleName("GameSys.CrowdSystem.Spectators.DisplayHighlightedCycleName", false);
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU

#ifndef PLATFORM_SPU
C_CrowdBlock::C_CrowdBlock()
	: m_BlockRefIndex(-1),
	m_BlockRefType(0),
	m_NextInCache(-1),
	m_PrevInCache(-1),
	m_NextInBucket(-1),
	m_IsCached(0),
	m_IsVisible(0),
	m_HasHighDetailSpectators(0),
	m_DetailLevel(E_DETAIL_LEVEL_LOW),
	m_DestDetailLevel(E_DETAIL_LEVEL_LOW),
	m_NumInstances(0),
	m_NumSpectators(0),
	m_FrameNumber(0)
{ }

C_CrowdBlock::~C_CrowdBlock()
{
	Destroy();
}

void C_CrowdBlock::Create(const U32 blockRefIndex, const U32 blockRefType, const U32 numInstances)
{
	Destroy();

	m_BlockRefIndex = static_cast<S32>(blockRefIndex);
	m_BlockRefType = blockRefType;

	m_NumInstances = numInstances;
}

void C_CrowdBlock::Destroy()
{
	m_BlockRefIndex = -1;

	memset(m_Spectators, -1, sizeof(S16) * CROWD_MAX_INSTANCES_PER_BLOCK);
	m_NumInstances = 0;
	m_NumSpectators = 0;
	
	m_NextInCache = -1;
	m_PrevInCache = -1;

	m_DetailLevel = static_cast<U32>(E_DETAIL_LEVEL_LOW);
	m_DestDetailLevel = static_cast<U32>(E_DETAIL_LEVEL_LOW);
}

#ifndef CFG_GOLD
void C_CrowdBlock::DebugRender(DbUtil::I_DebugRenderPtr debugRenderPtr, const C_CrowdResources* pResources, const C_CrowdSpectator* pSpectators,
							   const Maths::HVec3& zDir, const Maths::Mat43* pSkinningMatrices) const
{
	if (!pResources || !pSpectators)
		return;

	if (!g_DebugRenderSpectators && !g_DebugRenderSkeleton && !g_DebugRenderMember)
		return;

	for (U32 i = 0, count = CROWD_MAX_INSTANCES_PER_BLOCK; i < count; ++i)
	{
		if (m_Spectators[i] < 0)
			continue;

		const C_CrowdSpectator& spectator = pSpectators[m_Spectators[i]];
		
		if (pResources->GetPopulation().GetSkeleton() && pResources->GetPopulation().GetSkeleton()->IsValid())
		{
			if (!g_DebugRenderHighDetailOnly || (g_DebugRenderHighDetailOnly && spectator.HasLocalAnimation()))
			{
				if (g_DebugRenderSpectators)
				{
					Maths::HVec3 position = spectator.GetPosition();
					Maths::HVec3 direction = spectator.GetDirection();

					debugRenderPtr->Draw3dArrow(position, direction, 1.f, 0.15f, CROWD_SPECTATOR_HIGHLIGHT_COLOUR);
				}
				
				if (g_DebugRenderSkeleton)
				{
					DebugRenderSkeleton(
						spectator.GetPose(*pResources),
						spectator.GetTransform(zDir),
						pResources->GetPopulation().GetSkeleton()->GetSkeletonResource()->GetSkeleton(),
						spectator.GetColour(),
						debugRenderPtr,
						pSkinningMatrices
					);
				}

				if (g_DebugRenderMember && spectator.GetBehaviourIndex() == g_DebugRenderBehaviourIndex)
				{
					DebugRenderBox(
						spectator.GetPosition(),
						Maths::HVec3(0.05f, 0.05f, 0.05f),
						CROWD_SPECTATOR_HIGHLIGHT_COLOUR,
						debugRenderPtr
					);

					if (g_DebugRenderAnimationCount)
					{
						Util::C_String text;
						text.Printf("%d", pResources->GetNumSharedAnimationsByBehaviour(spectator.GetBehaviourIndex()));
						
						debugRenderPtr->Draw3dText(spectator.GetPosition(), text);
					}

					if (g_DebugRenderSharedAnimation && spectator.HasSharedAnimation())
					{
						const C_CrowdSharedAnimation* pSharedAnimation = pResources->GetSharedAnimation(spectator.GetBehaviourIndex(), g_DebugRenderSharedAnimationIndex);

						if (pSharedAnimation == spectator.GetSharedAnimation(*pResources))
						{
							DebugRenderBox(
								spectator.GetPosition(),
								Maths::HVec3(0.15f, 2.f, 0.15f),
								CROWD_SPECTATOR_UNIQUE_ANIMATION,
								debugRenderPtr
							);
						}
					}

					if (g_DebugRenderCycleName)
					{
						debugRenderPtr->Draw3dText(spectator.GetPosition(), spectator.GetMainCycleName(*pResources));
					}
				}
			}
		}
	}
}

void C_CrowdBlock::DebugRenderSkeleton(const Animation::C_Pose* pPose, const Maths::HMat43& worldMatrix, const Animation::C_Skeleton* pSkeleton,
									   const Maths::RGBA& colour, DbUtil::I_DebugRenderPtr debugRenderPtr, const Maths::Mat43* pSkinningMatrices)
{
	if (!g_DebugRenderSkeleton)
		return;

	if (!pPose || !pSkeleton)
		return;

	U32 numJoints = pSkeleton->GetNumJoints();
	
	if (numJoints == 0)
		return;
	
	Animation::C_SkeletonPose skeletonPose(*pSkeleton, const_cast<Animation::C_Pose&>(*pPose));

	Btl::Vector<Animation::C_JointTransform> modelJointTransforms;
	modelJointTransforms.Resize(numJoints);
	
	skeletonPose.GetModelJointTransforms(&modelJointTransforms[0], modelJointTransforms.Size());

	debugRenderPtr->SetDepthEnable(false);
	
	for (U32 i = 0, count = numJoints; i < count; ++i)
	{
		Animation::C_JointTransform jointTransform = modelJointTransforms[i];
		Maths::Point3 jointWorldPos = jointTransform.GetPosition() * worldMatrix;
		
		S32 parentIndex = pSkeleton->GetJointParentIndex(i);
		if (parentIndex < 0)
			continue;

		Animation::C_JointTransform parentJointTransform = modelJointTransforms[parentIndex];
		Maths::Point3 parentWorldPos = parentJointTransform.GetPosition() * worldMatrix;

		debugRenderPtr->Draw3dLine(Maths::HVec3(jointWorldPos), Maths::HVec3(parentWorldPos), colour);
	}
	
	debugRenderPtr->SetDepthEnable(true);
}

void C_CrowdBlock::DebugRenderBox(const Maths::HVec3& position, const Maths::HVec3& halfSize, const Maths::RGBA& colour, DbUtil::I_DebugRenderPtr debugRenderPtr)
{
	debugRenderPtr->SetDepthEnable(false);

	debugRenderPtr->Draw3dBox(
		position - halfSize,
		position + halfSize,
		Maths::Constants<Maths::HMat43>::Identity(),
		colour
	);

	debugRenderPtr->Draw3dArrow(
		position,
		Maths::HVec3(0.f, 1.f, 0.f),
		0.5f,
		0.25f,
		colour
	);
	
	debugRenderPtr->SetDepthEnable(true);
}

const Maths::RGBA C_CrowdBlock::GetDebugColour() const
{
	if (m_BlockRefType == static_cast<U32>(E_BLOCK_TYPE_HIGH_DETAIL))
	{
		if (m_DetailLevel == E_DETAIL_LEVEL_HIGH)
			return CROWD_BLOCK_HIGH_DETAIL_COLOUR;
		else
			return CROWD_BLOCK_LOW_DETAIL_COLOUR;
	}
	
	return CROWD_BLOCK_BACKGROUND_COLOUR;
}
#endif // !CFG_GOLD
#endif // !PLATFORM_SPU

void C_CrowdBlock::Update(const C_SpectatorUpdateInput& input, C_SpectatorUpdateOutput& output)
{
	m_HasHighDetailSpectators = 0;

	BIZ_ASSERT(m_BlockRefIndex >= 0);

	if (m_BlockRefIndex < 0)
		return;

	for (U32 i = 0, count = CROWD_MAX_INSTANCES_PER_BLOCK; i < count; ++i)
	{
		if (m_Spectators[i] < 0)
			continue;

		C_CrowdSpectator& spectator = output.m_pSpectators[m_Spectators[i]];
		
		if (m_BlockRefType != E_BLOCK_TYPE_LOW_DETAIL)
		{
			if (m_DestDetailLevel != m_DetailLevel)
			{
				if (!((m_DestDetailLevel == E_DETAIL_LEVEL_HIGH) && (spectator.ForceShared())))
					spectator.SetDetailLevel(static_cast<CrowdDetailLevel>(m_DestDetailLevel), *output.m_pRandom);
			}
		}

		spectator.Update(input, output);

		if (spectator.HasLocalAnimation())
			m_HasHighDetailSpectators = 1;

		output.m_pPaletteMap[spectator.GetInstanceUniqueId()] = (spectator.GetAnimationIndex(output.m_pUniqueAnimationPool, output.m_pSharedAnimationPool) + 1);
	}

	m_DetailLevel = m_DestDetailLevel;
	
	SetDetailLevel(E_DETAIL_LEVEL_LOW);

	IsVisible(false);
}

#ifndef PLATFORM_SPU
void C_CrowdBlock::HandleEvent(const C_CrowdResources* pResources, C_CrowdSpectator* pSpectators, const C_Event& crowdEvent, const CrowdGlobalMood globalMood)
{
	if (!pResources || !pSpectators)
		return;

	if (crowdEvent.GetAreaType() != C_Event::E_AREA_TYPE_GLOBAL)
		SetDetailLevel(E_DETAIL_LEVEL_HIGH);
	else if (GetDetailLevel() != E_DETAIL_LEVEL_HIGH)
		return;

	for (U32 i = 0, count = CROWD_MAX_INSTANCES_PER_BLOCK; i < count; ++i)
	{
		if (m_Spectators[i] < 0)
			continue;

		C_CrowdSpectator& spectator = pSpectators[m_Spectators[i]];
		spectator.HandleEvent(pResources, crowdEvent, globalMood);
	}
}

void C_CrowdBlock::AddSpectator(const S16 spectator)
{
	BIZ_ASSERT(m_NumSpectators < CROWD_MAX_INSTANCES_PER_BLOCK);
	m_Spectators[m_NumSpectators++] = spectator;
}

const S16 C_CrowdBlock::GetSpectator(const U32 index) const
{
	BIZ_ASSERT(index < CROWD_MAX_INSTANCES_PER_BLOCK);
	return m_Spectators[index];
}

void C_CrowdBlock::ClearSpectators()
{
	memset(m_Spectators, -1, sizeof(S16) * CROWD_MAX_INSTANCES_PER_BLOCK);
	m_NumSpectators = 0;
}
#endif // !PLATFORM_SPU

const U32 C_CrowdBlock::GetNumSpectators() const
{
	return m_NumSpectators;
}

#ifndef PLATFORM_SPU
const U32 C_CrowdBlock::GetNumInstances() const
{
	return m_NumInstances;
}

void C_CrowdBlock::SetPrevInCache(const S16 block)
{
	m_PrevInCache = block;
}

const S16 C_CrowdBlock::GetPrevInCache() const
{
	return m_PrevInCache;
}

void C_CrowdBlock::SetNextInCache(const S16 block)
{
	m_NextInCache = block;
}

const S16 C_CrowdBlock::GetNextInCache() const
{
	return m_NextInCache;
}
#endif // !PLATFORM_SPU

void C_CrowdBlock::SetNextInBucket(const S16 block)
{
	m_NextInBucket = block;
}

const S16 C_CrowdBlock::GetNextInBucket() const
{
	return m_NextInBucket;
}

const S32 C_CrowdBlock::GetBlockRefIndex() const
{
	return m_BlockRefIndex;
}

void C_CrowdBlock::IsCached(const bool isCached)
{
	m_IsCached = isCached;
}

const bool C_CrowdBlock::IsCached() const
{
	return m_IsCached;
}

void C_CrowdBlock::IsVisible(const bool isVisible)
{
	m_IsVisible = isVisible;
}

const bool C_CrowdBlock::IsVisible() const
{
	return m_IsVisible;
}

#ifndef PLATFORM_SPU
const bool C_CrowdBlock::HasHighDetailSpectators() const
{
	return m_HasHighDetailSpectators;
}

void C_CrowdBlock::ForceDetailLevel(C_CrowdSpectator* pSpectators, const CrowdDetailLevel detailLevel,
									C_CrowdUniqueAnimationPool* pUniqueAnimationPool, C_CrowdSharedAnimationPool* pSharedAnimationPool)
{
	BIZ_ASSERT(pSpectators);

	m_DetailLevel = detailLevel;
	m_DestDetailLevel = detailLevel;

	m_HasHighDetailSpectators = 0;

	for (U32 i = 0, count = CROWD_MAX_INSTANCES_PER_BLOCK; i < count; ++i)
	{
		if (m_Spectators[i] < 0)
			continue;

		C_CrowdSpectator& spectator = pSpectators[m_Spectators[i]];

		spectator.ForceDetailLevel(detailLevel, pUniqueAnimationPool, pSharedAnimationPool);

		if (spectator.HasLocalAnimation())
			m_HasHighDetailSpectators = 1;
	}
}
#endif // !PLATFORM_SPU

void C_CrowdBlock::SetDetailLevel(const CrowdDetailLevel detailLevel)
{
	m_DestDetailLevel = static_cast<U32>(detailLevel);
}

const CrowdDetailLevel C_CrowdBlock::GetDetailLevel() const
{
	return static_cast<CrowdDetailLevel>(m_DetailLevel);
}

#ifndef PLATFORM_SPU
void C_CrowdBlock::UpdateFrameNumber(const U32 frameNumber)
{
	m_FrameNumber = frameNumber;
}

const U32 C_CrowdBlock::GetFrameNumber() const
{
	return m_FrameNumber;
}

bool C_CrowdBlock::Flush()
{
	return true;
}
#endif // !PLATFORM_SPU

}; // namespace AnimatedCrowd
