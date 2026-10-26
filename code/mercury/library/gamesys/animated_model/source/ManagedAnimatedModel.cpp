#include "AnimatedModelPch.h"
#include <animated_model/ManagedAnimatedModel.h>
#include <animated_model/AnimatedModelInstance.h>
#include <animated_model/TaskManager.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/deferred/CommandList.h>
#include <animation/cycle/CycleInstance.h>
#include <animation/skeleton/SkeletonInstance.h>
#include <animation/task/EvaluationState.h>
#include <animation/task/TaskManager.h>


namespace AnimatedModel
{

struct C_ManagedAnimatedModel::C_Impl
{
	C_Impl():
		evaluationState(Animation::C_EvaluationState::New(16))
	{
	}
	
	~C_Impl()
	{
		delete evaluationState;
		evaluationState = NULL;
		
		for (S32 i = 0; i < cycleInstances.Size(); ++i)
		{
			Animation::C_CycleInstance* inst = cycleInstances[i];
			delete inst;
			cycleInstances[i] = NULL;
		}
	}

	Animation::C_EvaluationState*				evaluationState;
	AnimatedModel::C_AnimatedModelInstance		animatedModelInstance;
	Btl::Vector<Animation::C_CycleInstance*>	cycleInstances;
};

//
// C_ManagedAnimatedModel
//

C_ManagedAnimatedModel::C_ManagedAnimatedModel():
	m_impl(NULL)
{
}
		
C_ManagedAnimatedModel::C_ManagedAnimatedModel(
	const GameResource::C_ModelResource* modelResource,
	U32 modelIndex,
	SceneGraph::C_SceneGraph* pSceneGraph,
	const Animation::C_PoseFormat& poseFormat,
	const Animation::C_Skeleton* skeleton):
	m_impl(NULL)
{
	Create(modelResource, modelIndex, pSceneGraph, poseFormat, skeleton);
}
			
C_ManagedAnimatedModel::~C_ManagedAnimatedModel()
{
	Destroy();
}
	
void C_ManagedAnimatedModel::Create(
	const GameResource::C_ModelResource* modelResource,
	U32 modelIndex,
	SceneGraph::C_SceneGraph* pSceneGraph,
	const Animation::C_PoseFormat& poseFormat,
	const Animation::C_Skeleton* skeleton)
{
	Create(pSceneGraph, poseFormat, skeleton);
	AddPart(modelResource, modelIndex);
}
		
void C_ManagedAnimatedModel::Create(
	SceneGraph::C_SceneGraph* pSceneGraph,
	const Animation::C_PoseFormat& poseFormat,
	const Animation::C_Skeleton* skeleton)
{
	Destroy();
	
	m_impl = new C_Impl;

	if (skeleton != NULL)
	{
		m_impl->evaluationState->GetSkeletonInstance().Create(*skeleton);
	}
	
	Animation::C_Pose& pose = m_impl->evaluationState->GetPose();
	
	pose.Create(poseFormat);
	pose.Clear();
	pose.Normalise(skeleton);
	
	m_impl->evaluationState->GetPoseMatrixCache().Create(pose);
	m_impl->evaluationState->GetPoseMatrixCache().Update(pose, skeleton);
	m_impl->animatedModelInstance.Create(pSceneGraph, poseFormat);
}
			
void C_ManagedAnimatedModel::AddPart(const GameResource::C_ModelResource* modelResource, U32 modelIndex)
{
	BIZ_ASSERT(m_impl != NULL);
	if (m_impl != NULL)
	{
		m_impl->animatedModelInstance.AddPart(modelResource, modelIndex);
	}
}

void C_ManagedAnimatedModel::Destroy()
{
	delete m_impl;
	m_impl = NULL;
}

U32 C_ManagedAnimatedModel::GetNumParts() const
{
	BIZ_ASSERT(m_impl);
	if (!m_impl) return 0;
	return m_impl->animatedModelInstance.GetNumParts();
}

void C_ManagedAnimatedModel::SetWorldMatrix(const Maths::HMat43& worldMatrix)
{
	BIZ_ASSERT(m_impl);
	if (m_impl)
	{
		m_impl->animatedModelInstance.SetWorldMatrix(worldMatrix);
	}
}

void C_ManagedAnimatedModel::InsertIntoSceneGraph()
{
	BIZ_ASSERT(m_impl);
	m_impl->animatedModelInstance.InsertIntoSceneGraph();
}

void C_ManagedAnimatedModel::RemoveFromSceneGraph()
{
	BIZ_ASSERT(m_impl);
	m_impl->animatedModelInstance.RemoveFromSceneGraph();
}

const WorldRender::C_ModelInstance* C_ManagedAnimatedModel::GetModelInstance(U32 partIndex) const
{
	BIZ_ASSERT(m_impl);
	return m_impl->animatedModelInstance.GetModelInstance(partIndex);
}

WorldRender::C_ModelInstance* C_ManagedAnimatedModel::GetModelInstance(U32 partIndex)
{
	BIZ_ASSERT(m_impl);
	return m_impl->animatedModelInstance.GetModelInstance(partIndex);
}

Animation::C_CycleInstance* C_ManagedAnimatedModel::CreateCycleInstance(const Animation::C_Cycle* cycle)
{
	BIZ_ASSERT(m_impl);
	if (!m_impl) return NULL;
	
	m_impl->cycleInstances.PushBack(new Animation::C_CycleInstance(cycle, &m_impl->evaluationState->GetPose()));
	return m_impl->cycleInstances.Back();
}

U32 C_ManagedAnimatedModel::GetNumCycleInstances() const
{
	BIZ_ASSERT(m_impl);
	if (!m_impl) return 0;
	
	return m_impl->cycleInstances.Size();
}

Animation::C_CycleInstance* C_ManagedAnimatedModel::GetCycleInstance(U32 index)
{
	BIZ_ASSERT(m_impl);
	if (!m_impl) return NULL;
	if (index >= static_cast<U32>(m_impl->cycleInstances.Size()))
	{
		BIZ_ASSERT_FAIL("Out of range cycle index (%d >= %d)", index, m_impl->cycleInstances.Size());
		return NULL;
	}
	return m_impl->cycleInstances[index];
}
		
void C_ManagedAnimatedModel::UpdateAsync(
	E_Sync syncPoint,
	const Maths::HMat43& worldMatrix,
	const Animation::C_Skeleton* skeleton,
	bool forceSchedule
)
{
	if (!m_impl)
	{
		return;
	}
	
	if (!m_impl->evaluationState->GetCommandList().IsEmpty())
	{
		m_impl->evaluationState->ScheduleEvaluationTask();
	}
	m_impl->animatedModelInstance.ScheduleUpdateTask(worldMatrix, m_impl->evaluationState->GetPoseNonBlocking(), m_impl->evaluationState->GetPoseMatrixCacheNonBlocking());
}
		
Animation::C_Pose& C_ManagedAnimatedModel::GetPose()
{
	BIZ_ASSERT(m_impl);
	return m_impl->evaluationState->GetPose();
}

const Animation::C_Pose& C_ManagedAnimatedModel::GetPose() const
{
	BIZ_ASSERT(m_impl);
	return m_impl->evaluationState->GetPose();
}

Animation::C_PoseMatrixCache& C_ManagedAnimatedModel::GetPoseMatrixCache()
{
	BIZ_ASSERT(m_impl);
	return m_impl->evaluationState->GetPoseMatrixCache();
}

const Animation::C_PoseMatrixCache&	 C_ManagedAnimatedModel::GetPoseMatrixCache() const
{
	BIZ_ASSERT(m_impl);
	return m_impl->evaluationState->GetPoseMatrixCache();
}

Animation::C_CommandList& C_ManagedAnimatedModel::GetCommandList()
{
	BIZ_ASSERT(m_impl);
	return m_impl->evaluationState->GetCommandList();
}
	
const Animation::C_CommandList& C_ManagedAnimatedModel::GetCommandList() const
{
	BIZ_ASSERT(m_impl);
	return m_impl->evaluationState->GetCommandList();
}
		
AnimatedModel::C_AnimatedModelInstance&	C_ManagedAnimatedModel::GetAnimatedModelInstance()
{
	BIZ_ASSERT(m_impl);
	return m_impl->animatedModelInstance;
}

const AnimatedModel::C_AnimatedModelInstance&	C_ManagedAnimatedModel::GetAnimatedModelInstance() const
{
	BIZ_ASSERT(m_impl);
	return m_impl->animatedModelInstance;
}

const Animation::C_SkeletonInstance& C_ManagedAnimatedModel::GetSkeletonInstance() const
{
	BIZ_ASSERT(m_impl);
	return m_impl->evaluationState->GetSkeletonInstance();
}

Animation::C_SkeletonInstance& C_ManagedAnimatedModel::GetSkeletonInstance()
{
	BIZ_ASSERT(m_impl);
	return m_impl->evaluationState->GetSkeletonInstance();
}


} // end AnimatedModel



