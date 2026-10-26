#include "CrowdSystemPch.h"

#include <crowd_system/CrowdResources.h>
#include <crowd_system/CrowdDefines.h>
#include <crowd_system/CrowdEvent.h>
#include <crowd_system/CrowdInstance.h>
#include <crowd_system/CrowdCharacterRef.h>

#include <animation/Pose/Pose.h>
#include <animation/Pose/PoseMatrixCache.h>

#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>

#include <animation/cycle/Cycle.h>
#include <animation/skeleton/Skeleton.h>

#include <crowd_system/CrowdBlockRef.h>
#include <game_resource/CrowdData.h>
#include <game_resource/ModelResource.h>

#include <scenegraph/SceneGraph.h>

#include <Dynamic/Particle/Event.h>
#include <Dynamic/Particle/EventInstance.h>
#include <Dynamic/Particle/EventInstanceHandle.h>

#include <system/Perf.h>

namespace AnimatedCrowd
{

#ifndef CFG_GOLD
Debug::Bool g_DebugSharedAnimationsOnly("GameSys.CrowdSystem.Animation.SharedAnimationsOnly", false);
Debug::Bool g_DebugDisableBehaviourCacheForDistantCrowd("GameSys.CrowdSystem.World.DisableDistantCrowdBehaviourCache", false);
#endif

Debug::Float	g_HipsXZScale("GameSys.CrowdSystem.Animation.HipsXZScale", 0.3f, Debug::Range(0.f, 1.f));
Debug::Float	g_HipsYScale("GameSys.CrowdSystem.Animation.HipsYScale", 1.f, Debug::Range(0.f, 1.f));

/////////////////////////////////////////////////////////////////////////////////////////////

C_CrowdCachedPalette::C_CrowdCachedPalette()
	: m_bValid(false),
	m_pBoneMatrices(NULL)
{ }

/////////////////////////////////////////////////////////////////////////////////////////////

C_CrowdResources::C_CrowdResources(const GameResource::C_CrowdData* pCrowdData, SceneGraph::C_SceneGraph* pSceneGraph, const U32 routeMask)
	: m_pCrowdData(pCrowdData),
	m_pSceneGraph(pSceneGraph),
	m_Loaded(0),
	m_pEventStringIds(NULL),
	m_NumEventStringIds(0),
	m_NumFlags(0),
	m_FlagBehaviourIndex(0xffff),
	m_PropJointIndex(0xffff)
{
	if (m_pSceneGraph)
	{
		SceneGraph::SetNumMatrixPalettes(m_pSceneGraph, CROWD_MAX_TOTAL_ANIMATIONS);
	}

	m_Population.Create(pCrowdData->GetCrowdBehaviourNameHashes());
	m_SharedAnimationPool.Create(this, pCrowdData->GetNumBehaviours(), CrowdMaskRoutes(routeMask), pCrowdData);
	m_BackupPaletteCache.Resize(pCrowdData->GetNumBehaviours());

	//m_EventEnabledPoseFieldMask = Animation::C_PoseFieldMask();
	m_EventEnabledPoseFieldMask.EnableAll(true);

	//m_EventDisabledPoseFieldMask = Animation::C_PoseFieldMask();
	m_EventDisabledPoseFieldMask.Disable(Animation::E_PoseFieldElement_CustomScalar);

	m_EffectSpawnData.m_NumItems = 0;
}

C_CrowdResources::~C_CrowdResources()
{
	Destroy();
}

void C_CrowdResources::Destroy()
{
	for (S32 i = 0, count = CROWD_MAX_LOCAL_ANIMATIONS; i < count; ++i)
	{
		C_CrowdAnimation* pAnimation = m_UniqueAnimationPool.GetAnimation(i);
		if (!pAnimation || pAnimation->GetIndex() == 0xff)
			continue;

		if (pAnimation->IsZombie())
			m_UniqueAnimationPool.RemoveAnimation(pAnimation->GetIndex());
	}

	m_UniqueAnimationPool.Destroy();

	BIZ_ASSERT(!m_UniqueAnimationPool.GetNumAnimations());

	DestroySharedAnimations();

	DestroyPoseFormat();

	for (U32 i = 0, count = static_cast<U32>(m_BackupPaletteCache.Size()); i < count; ++i)
	{
		if (m_BackupPaletteCache[i].m_pBoneMatrices)
			delete [] m_BackupPaletteCache[i].m_pBoneMatrices;
		m_BackupPaletteCache[i].m_pBoneMatrices = NULL;
	}
	m_BackupPaletteCache.Destroy();

	m_FlagAnimations.Destroy();
	m_FlagAnimationMap.Clear();

	m_EffectSpawnData.m_NumItems = 0;
}

const GameResource::C_CrowdData* C_CrowdResources::GetCrowdData() const
{
	return m_pCrowdData;
}

SceneGraph::C_SceneGraph* C_CrowdResources::GetSceneGraph() const
{
	return m_pSceneGraph;
}

C_CrowdPopulation& C_CrowdResources::GetPopulation()
{
	return m_Population;
}

const C_CrowdPopulation& C_CrowdResources::GetPopulation() const
{
	return m_Population;
}

bool C_CrowdResources::CreatePoseFormat()
{
	if (m_Population.GetSkeleton() && m_Population.GetSkeleton()->IsValid())
	{
		if (m_Population.GetSkeleton()->GetSkeletonResource() && m_Population.GetSkeleton()->GetSkeletonResource()->GetSkeleton())
		{
			DestroyPoseFormat();

			m_PoseFormat.SetSkeleton(m_Population.GetSkeleton()->GetSkeletonResource()->GetSkeleton());

			return true;
		}
	}

	return false;
}

void C_CrowdResources::CreateAnimationEvents()
{
	if (m_Population.GetSkeleton() && m_Population.GetSkeleton()->IsValid())
	{
		if (m_Population.GetSkeleton()->GetSkeletonResource() && m_Population.GetSkeleton()->GetSkeletonResource()->GetSkeleton())
		{
			BIZ_ASSERT(!m_pEventStringIds);

			m_PropJointIndex = m_Population.GetSkeleton()->GetSkeletonResource()->GetSkeleton()->FindJointByName("prop");

			m_NumEventStringIds = m_Population.GetNumAnimationEventEffects();
			m_pEventStringIds = new Animation::C_StringId[m_NumEventStringIds];

			for (U32 i = 0, count = m_NumEventStringIds; i < count; ++i)
			{
				const C_CrowdAnimationEventEffect& eventEffect = m_Population.GetAnimationEventEffect(i);
				m_PoseFormat.AddCustomScalarField(eventEffect.GetEventName());
				m_pEventStringIds[i].Set(eventEffect.GetEventName());
			}
		}
	}
}

void C_CrowdResources::DestroyPoseFormat()
{
	m_PoseFormat.Clear();

	SAFE_DELETE_ARY(m_pEventStringIds);
	m_NumEventStringIds = 0;
}

const Animation::C_PoseFormat& C_CrowdResources::GetPoseFormat() const
{
	return m_PoseFormat;
}

const bool C_CrowdResources::IsLoaded() const
{
	return m_Loaded;
}

void C_CrowdResources::SetLoaded(U8* pPaletteMap)
{
	BIZ_ASSERT(pPaletteMap);

	m_Loaded = 1;

	if (m_pSceneGraph)
	{
		SceneGraph::SetPoseFormat(m_pSceneGraph, &GetPoseFormat());

		U32 numJoints = m_Population.GetSkeleton()->GetSkeletonResource()->GetSkeleton()->GetNumJoints();

		Maths::Mat43* pIdentityPalette = new Maths::Mat43[numJoints];
		for (U32 i = 0, count = numJoints; i < count; ++i)
			pIdentityPalette[i] = CROWD_IDENTITY_MATRIX;

		for (U32 i = 0, count = static_cast<U32>(m_BackupPaletteCache.Size()); i < count; ++i)
		{
			if (m_BackupPaletteCache[i].m_pBoneMatrices)
			{
				delete [] m_BackupPaletteCache[i].m_pBoneMatrices;
				m_BackupPaletteCache[i].m_pBoneMatrices = NULL;
			}

			m_BackupPaletteCache[i].m_pBoneMatrices = new Maths::Mat43[numJoints];
			memcpy(m_BackupPaletteCache[i].m_pBoneMatrices, pIdentityPalette, sizeof(Maths::Mat43) * numJoints);

			SceneGraph::SetMatrixPalette(m_pSceneGraph, CROWD_MAX_TOTAL_ANIMATIONS + i, m_BackupPaletteCache[i].m_pBoneMatrices);
		}

		delete [] pIdentityPalette;

		BIZ_ASSERT(m_pCrowdData);

		for (U32 i = 0, count = static_cast<U32>(m_pCrowdData->GetNumInstances()); i < count; ++i)
		{
			const C_CrowdInstance& instance = m_pCrowdData->GetCrowdInstance(i);
			U8 behaviourIndex = m_pCrowdData->GetCrowdCharacterRef(instance.GetCharacterIndex());

			pPaletteMap[instance.GetUniqueId()] = static_cast<U32>(behaviourIndex) + 1;
		}
	}
}

S32 C_CrowdResources::GetUniqueAnimation(const U16 behaviourIndex)
{
	S32 animationIndex = -1;

#ifndef CFG_GOLD
	if (!g_DebugSharedAnimationsOnly)
#endif
	{
		animationIndex = m_UniqueAnimationPool.CreateAnimation(m_PoseFormat, m_Population.GetSkeleton()->GetSkeletonResource()->GetSkeleton(), behaviourIndex, m_pEventStringIds, m_NumEventStringIds);
	}

	return animationIndex;
}

S32 C_CrowdResources::GetSharedAnimation(const U16 behaviourIndex)
{
	return m_SharedAnimationPool.CreateAnimation(m_PoseFormat, m_Population.GetSkeleton()->GetSkeletonResource()->GetSkeleton(), behaviourIndex, m_pEventStringIds, m_NumEventStringIds);
}

void C_CrowdResources::UpdateAnimations(const C_UpdateContext& context, const CrowdGlobalMood globalMood)
{
	Maths::Vector3 hipScale(Maths::HVec3(g_HipsXZScale.Value(), g_HipsYScale.Value(), g_HipsXZScale.Value()));

	Animation::C_JointTransform hipJointTransform;
	hipJointTransform.SetIdentity();
	hipJointTransform.SetScale(hipScale);
	
	{
		PERF_EVENT("CrowdUpdate.World.Animations.Unique");
		UpdateUniqueAnimations(context.m_DeltaTime, context.m_HipJointIndex, hipJointTransform);
	}

	{
		PERF_EVENT("CrowdUpdate.World.Animations.Shared");
		UpdateSharedAnimations(context, hipJointTransform, globalMood);
	}
}

void C_CrowdResources::UpdateUniqueAnimations(const F32 deltaTime, const U32 iHipJointIndex, const Animation::C_JointTransform& hipJointTransform)
{
	m_UniqueAnimationPool.AllocateAnimations();

	if (m_pSceneGraph && m_UniqueAnimationPool.GetNumAnimations())
	{
		for (S32 i = 0, count = CROWD_MAX_LOCAL_ANIMATIONS; i < count; ++i)
		{
			C_CrowdAnimation* pAnimation = m_UniqueAnimationPool.GetAnimation(i);
			if (!pAnimation || pAnimation->GetIndex() == 0xff)
				continue;

			if (pAnimation->IsZombie())
				m_UniqueAnimationPool.RemoveAnimation(pAnimation->GetIndex());
			else
			{
				if (pAnimation->IsDirty())
					pAnimation->Update(deltaTime, *this, iHipJointIndex, hipJointTransform);
				if (pAnimation->IsValid() && pAnimation->GetPoseMatrixCacheNonBlocking())
				{
					SceneGraph::SetMatrixPalette(m_pSceneGraph, i + CROWD_MAX_SHARED_ANIMATIONS,
						m_UniqueAnimationPool.GetAnimation(i)->GetPoseMatrixCacheNonBlocking()->GetSkinningMatrices());
				}
			}
		}
	}
}

const U32 C_CrowdResources::UpdateSharedAnimations(const C_UpdateContext& context, const Animation::C_JointTransform& hipJointTransform, const CrowdGlobalMood globalMood)
{
	m_SharedAnimationPool.AllocateAnimations();

	U32 animationsUpdated = 0;

	if (m_pSceneGraph && m_SharedAnimationPool.GetNumAnimations())
	{
		for (S32 i = 0, count = CROWD_MAX_SHARED_ANIMATIONS; i < count; ++i)
		{
			C_CrowdSharedAnimation* pSharedAnimation = m_SharedAnimationPool.GetAnimation(i);
			
			if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() != 0xff)
			{
				if (m_SharedAnimationPool.GetRefCount(i) < 1)
				{
					if (i < m_SharedAnimationPool.GetNumBehaviours())
					{
						const Maths::Mat43* pMatrices = pSharedAnimation->GetAnimation().GetPoseMatrixCacheNonBlocking()->GetSkinningMatrices();
						
						memcpy(m_BackupPaletteCache[i].m_pBoneMatrices, pMatrices, (sizeof(Maths::Mat43) * pSharedAnimation->GetAnimation().GetPoseNonBlocking()->GetNumJointTransforms()));
						m_BackupPaletteCache[i].m_bValid = true;

						SceneGraph::SetMatrixPalette(m_pSceneGraph, i, m_BackupPaletteCache[i].m_pBoneMatrices);
					}

					m_SharedAnimationPool.RemoveAnimation(i);
				}
				else
				{
					if (m_SharedAnimationPool.UpdateAnimation(context, i, hipJointTransform, globalMood))
					{
						animationsUpdated++;

						if (pSharedAnimation->GetAnimation().GetPoseMatrixCacheNonBlocking())
						{
							SceneGraph::SetMatrixPalette(m_pSceneGraph, i,
								pSharedAnimation->GetAnimation().GetPoseMatrixCacheNonBlocking()->GetSkinningMatrices());
						}
					}
				}
			}
		}
	}

	return animationsUpdated;
}

void C_CrowdResources::DestroySharedAnimations()
{
	m_SharedAnimationPool.Destroy();
}

void C_CrowdResources::PurgeAnimationPools()
{
	m_UniqueAnimationPool.Purge();
	m_SharedAnimationPool.Purge();
}

void C_CrowdResources::ClearMatrixPalettes() const
{
	if (m_pSceneGraph)
		SceneGraph::ClearMatrixPalettes(m_pSceneGraph, m_SharedAnimationPool.GetNumBehaviours());
}

void C_CrowdResources::UpdateEffects(const F32 deltaTime)
{
	U32 index = 0;

	BIZ_ASSERT(m_Population.GetSkeleton() && m_Population.GetSkeleton()->GetSkeletonResource());
	const Animation::C_Skeleton* pSkeleton = m_Population.GetSkeleton()->GetSkeletonResource()->GetSkeleton();

	while (index < m_EffectSpawnData.m_NumItems)
	{
		EffectSpawnItem& item = m_EffectSpawnData.m_Items[index];
		item.StartTime -= deltaTime;

		if (item.StartTime <= 0.f)
		{
			if (item.EventIndex >= 0 && pSkeleton)
			{
				const C_CrowdAnimation* pAnimation = NULL;
				if (item.AnimPool == 1)
				{
					const C_CrowdSharedAnimation* pSharedAnimation = m_SharedAnimationPool.GetAnimation(item.AnimIndex);
					if (pSharedAnimation)
						pAnimation = &pSharedAnimation->GetAnimation();
				}
				else if (item.AnimPool == 2)
				{
					pAnimation = m_UniqueAnimationPool.GetAnimation(item.AnimIndex);
				}

				if (pAnimation && pAnimation->HasPose())
				{
					BIZ_ASSERT(pAnimation->GetIndex() != 0xff);

					DynamicEffect::C_Node node;
					Maths::HMat43 jointTransform = pAnimation->GetPose()->GetModelJointTransform(item.JointIndex, pSkeleton).AsMat43();
					Maths::HVec3 position = jointTransform.Translation() * item.Transform;
					node.SetPosition(position);
					GetPopulation().GetAnimationEventEffect(item.EventIndex).GetEvent()->Create(node);
				}
			}
			m_EffectSpawnData.RemoveItem(index);
		}
		else
		{
			index++;
		}
	}
}

const Animation::C_PoseFieldMask& C_CrowdResources::GetAnimationEventMask(bool enableEvents) const
{
	return (enableEvents ? m_EventEnabledPoseFieldMask : m_EventDisabledPoseFieldMask);
}

void C_CrowdResources::SetFlagInstanceNodeTypes(const C_CrowdFlags* pFlags)
{
	if (!pFlags)
		return;

	for (U32 i = 0, count = CROWD_MAX_FLAG_INSTANCES; i < count; ++i)
		pFlags->SetNodeType(&(m_Flags[i]));
}

void C_CrowdResources::AddFlagAnimation(const S32 sharedAnimation)
{
	if (sharedAnimation >= 0)
	{
		Btl::OpenHashMap<U32, U32>::Iterator iter = m_FlagAnimationMap.Find(sharedAnimation);
		if (iter == m_FlagAnimationMap.End())
		{
			m_FlagAnimationMap.Insert(sharedAnimation, static_cast<U32>(m_FlagAnimations.Size()));
			m_FlagAnimations.PushBack(sharedAnimation);
		}
	}
}

void C_CrowdResources::RemoveFlagAnimation(const S32 sharedAnimation)
{
	if (sharedAnimation >= 0)
	{
		Btl::OpenHashMap<U32, U32>::Iterator iter = m_FlagAnimationMap.Find(sharedAnimation);
		if (iter != m_FlagAnimationMap.End())
		{
			BIZ_ASSERT(m_FlagAnimations[iter->GetValue()] == sharedAnimation);
			m_FlagAnimations.EraseByIndexNonOrdered(iter->GetValue());

			m_FlagAnimationMap.Clear();
			for (U32 i = 0, count = GetNumFlagAnimations(); i < count; ++i)
				m_FlagAnimationMap.Insert(static_cast<U32>(m_FlagAnimations[i]), i);
		}
	}
}

const bool C_CrowdResources::RequiresFlag(const U16 behaviourIndex) const
{
	return (behaviourIndex == m_FlagBehaviourIndex);
}

void C_CrowdResources::ClearFlags()
{
	m_NumFlags = 0;
}

void C_CrowdResources::AddFlag(const Maths::HMat43& transform, const F32* sh, const U8 animationIndex, const S32 textureIndex) const
{
	if (m_NumFlags < CROWD_MAX_FLAG_INSTANCES)
	{
		m_Flags[m_NumFlags].Transform = transform;
		memcpy(m_Flags[m_NumFlags].SH, sh, sizeof(F32) * 16);
		
		m_Flags[m_NumFlags].BaseFlagIndex = -1; // TODO: [rthomas]	Speed this up (map?)
		Btl::OpenHashMap<U32, U32>::ConstIterator iter = m_FlagAnimationMap.Find(animationIndex);
		if (iter != m_FlagAnimationMap.End())
			m_Flags[m_NumFlags].BaseFlagIndex = iter->GetValue();

		m_Flags[m_NumFlags].TextureIndex = textureIndex;

		m_NumFlags++;
	}
}

const S32 C_CrowdResources::GetFlagAnimation(const U32 index) const
{
	if (index < GetNumFlagAnimations())
		return m_FlagAnimations[index];
	return -1;
}

void C_CrowdResources::HandleEvent(const C_Event& globalEvent, const CrowdGlobalMood globalMood)
{
	if (!m_SharedAnimationPool.GetNumAnimations())
		return;

	for (U32 i = 0, count = CROWD_MAX_SHARED_ANIMATIONS; i < count; ++i)
	{
		C_CrowdSharedAnimation* pSharedAnimation = m_SharedAnimationPool.GetAnimation(i);
		
		if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() != 0xff)
			pSharedAnimation->HandleEvent(this, globalEvent, globalMood);
	}
}

bool C_CrowdResources::Flush()
{
	if (!m_UniqueAnimationPool.Flush())
		return false;

	if (!m_SharedAnimationPool.Flush())
		return false;

	m_Loaded = 0;

	DestroyPoseFormat();

	return true;
}

const U32 C_CrowdResources::GetNumUsedUniqueAnimations() const
{
	return static_cast<U32>(m_UniqueAnimationPool.GetNumAnimations());
}

const U32 C_CrowdResources::GetNumFreeUniqueAnimations() const
{
	return static_cast<U32>(m_UniqueAnimationPool.GetNumFreeAnimations());
}

const U32 C_CrowdResources::GetNumUsedSharedAnimations() const
{
	return static_cast<U32>(m_SharedAnimationPool.GetNumAnimations());
}

const U32 C_CrowdResources::GetNumFreeSharedAnimations() const
{
	return static_cast<U32>(m_SharedAnimationPool.GetNumFreeAnimations());
}

const U32 C_CrowdResources::GetMaxSharedAnimationsByBehaviour(const U16 behaviourIndex) const
{
	return m_SharedAnimationPool.GetMaxAnimationsByBehaviour(behaviourIndex);
}

const U32 C_CrowdResources::GetNumSharedAnimationsByBehaviour(const U16 behaviourIndex) const
{
	return m_SharedAnimationPool.GetNumAnimationsByBehaviour(behaviourIndex);
}

#ifndef CFG_GOLD
const C_CrowdSharedAnimation* C_CrowdResources::GetSharedAnimation(const U16 behaviourIndex, const S32 animationIndex) const
{
	S32 sharedAnimationIndex = m_SharedAnimationPool.GetSharedAnimationIndex(behaviourIndex, animationIndex);
	
	if (sharedAnimationIndex >= 0)
	{
		const C_CrowdSharedAnimation* pSharedAnimation = m_SharedAnimationPool.GetAnimation(sharedAnimationIndex);
		if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() != 0xff)
			return pSharedAnimation;
	}
	
	return NULL;
}
#endif // CFG_GOLD

const C_CrowdAnimation* C_CrowdResources::GetSharedAnimationByIndex(const S32 animationIndex) const
{
	if (animationIndex < 0)
		return NULL;

	const C_CrowdSharedAnimation* pSharedAnimation = m_SharedAnimationPool.GetAnimation(animationIndex);
	if (pSharedAnimation && pSharedAnimation->GetAnimation().GetIndex() != 0xff)
		return &pSharedAnimation->GetAnimation();

	return NULL;
}

}; // namespace AnimatedCrowd
