#include "SpecialFXPch.h"
#include "SpecialFX/AnimatedMesh.h"
#include <animated_model/AnimatedModelInstance.h>
#include <animated_model/TaskManager.h>
#include <animation/common/ObjectTransform.h>
#include <game_resource/ModelResource.h>
#include <game_resource/ModelData.h>
#include <animation/common/StringUtil.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/cycle/CycleInstance.h>
#include <animation/cycle/Cycle.h>
#include <animation/deferred/CommandList.h>
#include <animation/resource/CycleResource.h>
#include <animation/resource/SkeletonResource.h>
#include <animation/task/EvaluationState.h>
#include <scenegraph/SceneGraph.h>
#include <game_context/RenderContext.h>
#include <misc_shared/context/ContextProvider.h>
#include <resource_utils/ResourceBagUtils.h>
#include <system/Memory.h>
#include <world_render/ModelInstance.h>
#include <world_render/RenderModel.h>

XT_TYPE(SpecialFX::C_InternalSpecialFX, "InternalSpecialFX")
XT_FIELD(m_AnimationNodeName, "AnimationNodeName")
XT_FIELD(m_SpecialFX, "SpecialFX")
XT_END_TYPE

XT_TYPE(SpecialFX::C_AnimatedMeshSpecialFX,	"AnimatedMeshSpecialFX")
	XT_BASE(SpecialFX::C_BaseSpecialFXComponent)
	XT_FIELD(m_AnimatedMeshFileName,	"AnimatedMeshFileName")
	XT_FIELD(m_AnimationFileName,		"AnimationFileName")
	XT_FIELD(m_SkeletonFileName,		"SkeletonFileName")
	XT_FIELD(m_FarDistance,				"FarDistance")
	XT_FIELD(m_ExcludeFromCubemap,		"ExcludeFromCubemap")
	XT_FIELD(m_ExcludeFromReflection,	"ExcludeFromReflection")
	XT_FIELD(m_ExcludeFromMirror,		"ExcludeFromMirror")
	XT_FIELD(m_InternalSpecialFXVector,	"InternalSpecialFX")
XT_END_TYPE

Debug::Float g_AnimMeshDrawDistanceFactor("GameSys.SpecialFX.AnimMeshDrawDistanceFactor", 150.0f);

namespace SpecialFX
{

Debug::Logger &g_Log = Debug::GetLogger("SpecialFX");

struct C_InternalSpecialFXInstance
{
	U32						m_TransformIndex;
	I_SpecialFXInstance*	m_pSpecialFX;
};

//Internal class implementation
class C_AnimatedMeshInstance : public I_SpecialFXComponentInstance, public C_PostUpdateCallBack
{
	const GameResource::C_ModelResource*		m_pModelResource;
	const Animation::C_CycleResource*			m_pCycleResource;
	const Animation::C_SkeletonResource*		m_pSkeletonResource;
	Animation::C_EvaluationState*				m_pAnimEvaluationState;
	AnimatedModel::C_AnimatedModelInstance		m_AnimatedModelInstance;
	Animation::C_CycleInstance					m_CycleInstance;
	F32											m_FarDistance;
	
	Maths::HMat43								m_WorldMatrix;

	SceneGraph::C_SceneGraph*					m_pSceneGraph;

	bool										m_Killed;
	bool										m_AddedInScenegraph;
	U32											m_VisibilityFlags;

	F32											m_Time;
	Btl::Vector<C_InternalSpecialFXInstance>	m_InternalSpecialFXVector;
	S32											m_PostUpdateIndex;

	F32											m_Intensity;
	WorldRender::MaterialParameter				m_GlobalColourParameter;
	
public:
	C_AnimatedMeshInstance(const GameResource::C_ModelResource* pModelResource,
		const Animation::C_CycleResource* pCycleResource,
		const Animation::C_SkeletonResource* pSkeletonResource,
		SceneGraph::C_SceneGraph* pSceneGraph,
		F32 farDistance,
		U32 visibilityFlags);
	virtual ~C_AnimatedMeshInstance();
	
	virtual void Start();
	virtual void UpdateWorldMatrix(const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal);
	virtual void Stop();
	virtual bool Update(F32 time);
	virtual F32	 GetDuration();
	virtual void SetParameter(U32 paramID, void* params);
	virtual void SetVisibilityDetect(SceneGraph::C_VisibleDetect* pVisibilityDetect);
	virtual void UpdateIntensity(F32 intensity);

	const Animation::C_Pose* GetAnimationPose() const;
	const Animation::C_PoseMatrixCache& GetAnimationPoseMatrixCache() const;

	//Post update
	void AddInternalSpecialFX(U32 transformIndex, I_SpecialFXInstance* pSpecialFX);
	void PostUpdate();
};

C_AnimatedMeshInstance::C_AnimatedMeshInstance(const GameResource::C_ModelResource* pModelResource,	const Animation::C_CycleResource* pCycleResource,
											   const Animation::C_SkeletonResource* pSkeletonResource,	SceneGraph::C_SceneGraph* pSceneGraph, F32 farDistance, U32 visibilityFlags) :
m_pModelResource(pModelResource),
m_pCycleResource(pCycleResource),
m_pSkeletonResource(pSkeletonResource),
m_pAnimEvaluationState(NULL),
m_pSceneGraph(pSceneGraph),
m_FarDistance(farDistance),
m_Killed(false),
m_VisibilityFlags(visibilityFlags),
m_Time(0.f),
m_PostUpdateIndex(-1),
m_Intensity(1.f)
{
	if (!m_pModelResource)
		return;
	if (!m_pCycleResource)
		return;

	const Animation::C_Skeleton* pSkeleton = NULL;
	if (m_pSkeletonResource)
	{
		pSkeleton = m_pSkeletonResource->GetSkeleton();
	}
	const Animation::C_Cycle* pCycle = m_pCycleResource->GetCycle();

	// Create PoseFormat with the optional skeleton's fields first (important) and then any fields in the cycle.
	Animation::C_PoseFormat poseFormat(pSkeleton);
	poseFormat.AddCycleFields(pCycle);

	// Create an EvaluationState with enough CommandList space for evaluating a single cycle.
	m_pAnimEvaluationState = Animation::C_EvaluationState::New(1);
	m_pAnimEvaluationState->Create(poseFormat, pSkeleton);

	Animation::C_Pose& pose = m_pAnimEvaluationState->GetPose();
	pose.Clear();
	pose.Normalise(pSkeleton);
	
	U32 modelIndex = pModelResource->GetModelData()->GetModelCount() > 1 ? 1 : 0;
	m_AnimatedModelInstance.Create(pModelResource, modelIndex, pSceneGraph, poseFormat);

	m_AnimatedModelInstance.SetVisibilityFlags(m_VisibilityFlags);

	m_CycleInstance.Create(pCycle, &m_pAnimEvaluationState->GetPose());

	m_WorldMatrix.SetIdentity();

	if (m_FarDistance > 0.1f)
		m_AnimatedModelInstance.SetNearFarDistance(0.f, m_FarDistance);

	m_GlobalColourParameter = m_AnimatedModelInstance.GetModelInstance()->GetRenderModel()->GetParameter("g_GlobalColour");
	m_AddedInScenegraph = false;
}

C_AnimatedMeshInstance::~C_AnimatedMeshInstance()
{
	delete m_pAnimEvaluationState;
	m_pAnimEvaluationState = NULL;

	for (S32 i = 0; i < m_InternalSpecialFXVector.Size(); i++)
		SAFE_DELETE_PTR(m_InternalSpecialFXVector[i].m_pSpecialFX);

	m_InternalSpecialFXVector.Clear();

	if (m_PostUpdateIndex >= 0)
	{
		C_AnimatedMeshSpecialFX::ClearPostUpdate(m_PostUpdateIndex);
	}
}

void C_AnimatedMeshInstance::Start()
{
	if (m_Killed)
		return;
	if (!m_pModelResource)
		return;
	if (!m_pCycleResource)
		return;
	
	BIZ_ASSERT(!m_AddedInScenegraph);

	//Remap PostResolve pass if needed
	S32 renderPass = (S32)(INTPTR)C_SpecialFXManager::GetInitParam(E_InitParamID_PostResolveRenderPass);	// [Blur2PC][x64] integer init param
	if (renderPass != -1)
	{
		for (U32 i = 0; i < m_AnimatedModelInstance.GetNumParts(); i++)
			m_AnimatedModelInstance.GetModelInstance(i)->SetRenderingDataRenderPassMapping(RenderingData::RENDER_PASS_POST_RESOLVE, renderPass);
	}

	m_AnimatedModelInstance.InsertIntoSceneGraph();
	m_AddedInScenegraph = true;

	for (S32 i = 0; i < m_InternalSpecialFXVector.Size(); i++)
		m_InternalSpecialFXVector[i].m_pSpecialFX->Start();
}

void C_AnimatedMeshInstance::UpdateWorldMatrix( const Maths::HMat43& worldMatrix, const Maths::HVec3& velocity, const Maths::HVec3& planePos, const Maths::C_Normal& planeNormal )
{
	m_WorldMatrix = worldMatrix;
}

void C_AnimatedMeshInstance::Stop()
{
	if (m_Killed)
		return;

	if (!m_pModelResource)
		return;
	if (!m_pCycleResource)
		return;

	if (m_AddedInScenegraph)
	{
		m_AnimatedModelInstance.RemoveFromSceneGraph();
		m_AddedInScenegraph = false;
	}

	for (S32 i = 0; i < m_InternalSpecialFXVector.Size(); i++)
		m_InternalSpecialFXVector[i].m_pSpecialFX->Stop();
}

bool C_AnimatedMeshInstance::Update( F32 time )
{
	m_Time = time;

	if (!m_pModelResource)
		return false;
	if (!m_pCycleResource)
		return false;
	if (m_Killed)
		return false;
		
	if (time >= m_CycleInstance.GetDuration())
	{
		// Skip update.
		//Remove for scenegraph if needed, + 1 frame animation round
		if (time > (m_CycleInstance.GetDuration() + 1.f/24.f) && m_AddedInScenegraph)
		{
			m_AnimatedModelInstance.RemoveFromSceneGraph();
			m_AddedInScenegraph = false;
		}
		return false;
	}
	
	if (!m_AddedInScenegraph)
	{
		//Add to scenegraph if needed
		m_AnimatedModelInstance.InsertIntoSceneGraph();
		m_AddedInScenegraph = true;
	}

	Animation::C_CommandList& cmdList = m_pAnimEvaluationState->GetCommandList();
		
	cmdList.Clear();
	cmdList.BlendCycle(
		Animation::E_BlendMode_Accumulate,
		&m_CycleInstance,
		time,
		1.f,
		Animation::E_LoopMode_Once
	);
	
	//Update global colour
	MathsStorage::Float4 globalColour(1.f, 1.f, 1.f, m_Intensity);
	m_AnimatedModelInstance.GetModelInstance()->SetParameter(m_GlobalColourParameter, (void*)&globalColour.x);
	

	//Update far distant culling
	if (!(m_FarDistance > 0.1f))
	{
		//Automatic
		const Maths::C_Sphere& wSphere = m_AnimatedModelInstance.GetWorldSphere();
		m_AnimatedModelInstance.SetNearFarDistance(0.f, Maths::Min(500.f, wSphere.Radius() * g_AnimMeshDrawDistanceFactor) + wSphere.Radius());
	}

	//Add post update if needed
	if (m_InternalSpecialFXVector.Size() > 0)
	{
		m_PostUpdateIndex = C_AnimatedMeshSpecialFX::PushPostUpdate(this);
	}
	
	// Update the animated model asynchronously, to finish at some point before rendering.
	m_pAnimEvaluationState->ScheduleEvaluationTask();
	m_AnimatedModelInstance.ScheduleUpdateTask(m_WorldMatrix, m_pAnimEvaluationState->GetPoseNonBlocking(), m_pAnimEvaluationState->GetPoseMatrixCacheNonBlocking());

	return true;
}

F32 C_AnimatedMeshInstance::GetDuration()
{
	if (!m_pModelResource)
		return 0.f;
	if (!m_pCycleResource)
		return 0.f;
	return m_CycleInstance.GetDuration();	
}

void C_AnimatedMeshInstance::SetParameter( U32 paramID, void* params )
{
	if (paramID == E_ParamID_SetMeshViewFlags)
	{
		U32* pParamMask = (U32*) params;
		m_AnimatedModelInstance.SetVisibilityFlags((m_AnimatedModelInstance.GetVisibilityFlags() & ~pParamMask[1]) | (pParamMask[0]&pParamMask[1]) );
	}
	else if (paramID == E_ParamID_KillMeshEffect)
	{
		//Remove from scenegraph
		Stop();
		m_Killed = true;
	}
	else if (paramID == E_ParamID_CalculateMeshAxisBox)
	{
		//Remove from scenegraph
		Maths::C_AxisBox* pAxisBox = (Maths::C_AxisBox*) params;
		pAxisBox->UpdateExtents(m_AnimatedModelInstance.GetWorldSphere());
	}
	else if (paramID == E_ParamID_ImmediateUpdate)
	{
		m_AnimatedModelInstance.ScheduleUpdateTask(m_WorldMatrix, m_pAnimEvaluationState->GetPoseNonBlocking(), m_pAnimEvaluationState->GetPoseMatrixCacheNonBlocking());
	}
}

void C_AnimatedMeshInstance::SetVisibilityDetect( SceneGraph::C_VisibleDetect* pVisibilityDetect )
{
	for (U32 i = 0; i < m_AnimatedModelInstance.GetNumParts(); i++)
		m_AnimatedModelInstance.GetModelInstance(i)->SetVisDetect(pVisibilityDetect);
}

const Animation::C_Pose* C_AnimatedMeshInstance::GetAnimationPose() const
{
	if (!m_pAnimEvaluationState)
		return NULL;

	return &m_pAnimEvaluationState->GetPoseReadOnly();
}

const Animation::C_PoseMatrixCache& C_AnimatedMeshInstance::GetAnimationPoseMatrixCache() const
{
	return m_pAnimEvaluationState->GetPoseMatrixCacheReadOnly();
}


void C_AnimatedMeshInstance::PostUpdate()
{
	//Update special FXs
	const Animation::C_PoseMatrixCache& poseMatrixCache = GetAnimationPoseMatrixCache();
	for (S32 i = 0; i < m_InternalSpecialFXVector.Size(); i++)
	{
		Maths::HMat43 childMatrix = poseMatrixCache.GetSkinningMatrices()[m_InternalSpecialFXVector[i].m_TransformIndex] * m_WorldMatrix;
		m_InternalSpecialFXVector[i].m_pSpecialFX->UpdateWorldMatrix(childMatrix);
		m_InternalSpecialFXVector[i].m_pSpecialFX->Update(m_Time);
	}
	m_PostUpdateIndex = -1;
}

void C_AnimatedMeshInstance::AddInternalSpecialFX( U32 transformIndex, I_SpecialFXInstance* pSpecialFX )
{
	m_InternalSpecialFXVector.PushBack();
	m_InternalSpecialFXVector.Back().m_TransformIndex = transformIndex;
	m_InternalSpecialFXVector.Back().m_pSpecialFX = pSpecialFX;
}

void C_AnimatedMeshInstance::UpdateIntensity( F32 intensity )
{
	m_Intensity = intensity;
}
Btl::Vector<C_PostUpdateCallBack* > C_AnimatedMeshSpecialFX::s_PostUpdateCallBackVector;

C_AnimatedMeshSpecialFX::C_AnimatedMeshSpecialFX()
{
	//m_pModelResource = NULL;
	//m_pCycleResource = NULL;
}

I_SpecialFXComponentInstance* C_AnimatedMeshSpecialFX::CreateInstance(const Sys::I_ContextProvider* pContextProvider, const ResourceUtils::I_ResourceBag* pResourceBag) const
{
	const GameResource::C_ModelResource* pModelResource = NULL;
	const Animation::C_CycleResource* pCycleResource = NULL;
	const Animation::C_SkeletonResource* pSkeletonResource = NULL;
	if(pResourceBag != NULL)
	{
		pModelResource = pResourceBag->GetResource<GameResource::C_ModelResource>( MakeResourceBagTag(m_AnimatedMeshFileName.Text()) );
		pCycleResource = pResourceBag->GetResource<Animation::C_CycleResource>( MakeResourceBagTag(m_AnimationFileName.Text()) );

		if (HasSkeleton())
		{
			pSkeletonResource = pResourceBag->GetResource<Animation::C_SkeletonResource>( MakeResourceBagTag(m_SkeletonFileName.Text()) );
		}
	}

	if (pModelResource == NULL)
	{
		LOGWARN(g_Log, "AnimatedMesh specialFX: Model %s doesn't exist", m_AnimatedMeshFileName.Text());
	}
	if (pCycleResource == NULL)
	{
		LOGWARN(g_Log, "AnimatedMesh specialFX: Cycle %s doesn't exist", m_AnimationFileName.Text());
	}
	if (HasSkeleton() && pSkeletonResource == NULL)
	{
		LOGWARN(g_Log, "AnimatedMesh specialFX: Skeleton %s doesn't exist", m_SkeletonFileName.Text());
	}

	// Get SceneGraph
	const GameContext::I_RenderContext* pContext = pContextProvider->GetContext<GameContext::I_RenderContext>();
	if (pContext != NULL)
	{
		U32 viewFlags = 0xFFFFFFFF;
		if (m_ExcludeFromCubemap) viewFlags &= ~U32(INTPTR(C_SpecialFXManager::GetInitParam(E_InitParamID_CubemapViewFlag)));	// [Blur2PC][x64] integer init param
		if (m_ExcludeFromReflection) viewFlags &= ~U32(INTPTR(C_SpecialFXManager::GetInitParam(E_InitParamID_ReflectionViewFlag)));	// [Blur2PC][x64] integer init param
		if (m_ExcludeFromMirror) viewFlags &= ~U32(INTPTR(C_SpecialFXManager::GetInitParam(E_InitParamID_MirrorViewFlag)));	// [Blur2PC][x64] integer init param
		//Create an instance
		C_AnimatedMeshInstance *pAnimatedMeshInstance = new C_AnimatedMeshInstance(pModelResource, pCycleResource, pSkeletonResource, pContext->GetSceneGraph(), m_FarDistance, viewFlags);
		
		//Create and add internal SpecialFX if needed
		const Animation::C_Pose* pPose = pAnimatedMeshInstance->GetAnimationPose();
		if (pPose)
		{
			for (S32 i = 0; i < m_InternalSpecialFXVector.count; ++i)
			{
				if (m_InternalSpecialFXVector[i].m_SpecialFX && m_InternalSpecialFXVector[i].m_AnimationNodeName[0])
				{
					const char* nodeName = m_InternalSpecialFXVector[i].m_AnimationNodeName;
					for (U32 transformIndex = 0, numTransforms = pPose->GetNumJointTransforms(); transformIndex < numTransforms; ++transformIndex)
					{
						if (Animation::StringUtil::Match(pPose->GetJointTransformName(transformIndex).GetString(), nodeName))
						{
							I_SpecialFXInstance* pSpecialFXInstance = C_SpecialFXManager::CreateSpecialFXInstance(m_InternalSpecialFXVector[i].m_SpecialFX,pContextProvider, pResourceBag);
							pAnimatedMeshInstance->AddInternalSpecialFX(transformIndex, pSpecialFXInstance);
						}
					}
				}
			}
		}
		return pAnimatedMeshInstance;		
	}

	return NULL;
}

void C_AnimatedMeshSpecialFX::AddToResourceBag( ResourceUtils::I_ResourceBag& pResourceBag ) const
{
	Util::C_String modelName = Util::C_String(m_AnimatedMeshFileName.Text()) + "|specialfx";
	//HACK: Skip based on language code?
	{
		S32 idx = modelName.RevSearch('.');
		if (idx != -1 && modelName[idx-3] == '_')
		{
			char code[] = { modelName[idx-2], modelName[idx-1], NULL };
			code[0] = tolower(code[0]);
			code[1] = tolower(code[1]);
			if (SfxIsLanguageCode(code) && SfxIgnoreLanguageCode(code))
			{
				//LOGINFO(g_Log, "***** ignoring %s", m_AnimatedMeshFileName.Text());
				return;
			}
		}
	}
	pResourceBag.AddResource( MakeResourceBagTag(m_AnimatedMeshFileName.Text()), modelName.Chars(), GameResource::C_ModelResource::StaticGetResourceTypeId());
	pResourceBag.AddResource( MakeResourceBagTag(m_AnimationFileName.Text()), m_AnimationFileName.Text(), Animation::C_CycleResource::StaticGetResourceTypeId());

	if (HasSkeleton())
	{
		pResourceBag.AddResource( MakeResourceBagTag(m_SkeletonFileName.Text()), m_SkeletonFileName.Text(), Animation::C_SkeletonResource::StaticGetResourceTypeId());
	}

}

bool C_AnimatedMeshSpecialFX::HasSkeleton() const
{
	if (!m_SkeletonFileName.Text()) return false;
	return m_SkeletonFileName.Text()[0] != 0;
}

//Post Update
void C_AnimatedMeshSpecialFX::CreatePostUpdateSystem()
{

}

void C_AnimatedMeshSpecialFX::DestroyPostUpdateSystem()
{
	//Deallocate all post update system memory
	s_PostUpdateCallBackVector.Destroy();
}

void C_AnimatedMeshSpecialFX::PostUpdate()
{
	PERF_EVENT("AnimatedMeshPostUpdate");
	S32 numPostUpdate = s_PostUpdateCallBackVector.Size();
	for (S32 i = 0; i < numPostUpdate; i++) //If post update generate more post update, it will be removed (not supported)
	{
		if (s_PostUpdateCallBackVector[i])
			s_PostUpdateCallBackVector[i]->PostUpdate();
	}

	s_PostUpdateCallBackVector.Clear();
}

S32 C_AnimatedMeshSpecialFX::PushPostUpdate( C_PostUpdateCallBack* pPostUpdate )
{
	s_PostUpdateCallBackVector.PushBack(pPostUpdate);
	return s_PostUpdateCallBackVector.Size() - 1;
}

void C_AnimatedMeshSpecialFX::ClearPostUpdate(S32 index)
{
	BIZ_ASSERT(index >= 0);
	s_PostUpdateCallBackVector[index] = NULL;
}

class C_ModelTypeSpecialFX : public GameResource::I_ModelType
{
public:
	C_ModelTypeSpecialFX(){}
	void OnLoad(GameResource::C_ModelResource* p_pModelResource)
	{
		if (!p_pModelResource)
			return;

		GameResource::C_ModelData *pModelDesc = p_pModelResource->GetModelData();
		WorldRender::C_RenderModelData *pGRModel = p_pModelResource->GetRenderModelData();

		// for each model inside the file
		for (int iModel = 0; iModel < pModelDesc->GetModelCount(); iModel++)
		{
			GameResource::C_Model *pModel = pModelDesc->GetModel(iModel);
			WorldRender::C_Model *pWRModel = &p_pModelResource->GetRenderModelData()->GetModel(iModel);

			pWRModel->ExposeParameter("g_GlobalColour", RenderingData::MATERIAL_ELEMENT_TYPE_F32, 4);
		}
	}
public:
};

void C_AnimatedMeshSpecialFX::RegisterModelType()
{
	GameResource::g_ModelTypeManager.RegisterModelType("specialfx", new C_ModelTypeSpecialFX());
}


}

