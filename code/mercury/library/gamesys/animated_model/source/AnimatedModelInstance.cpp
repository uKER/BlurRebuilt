#include "AnimatedModelPch.h"
#include <string.h>
#include <animated_model/AnimatedModelInstance.h>
#include <animated_model/TaskManager.h>
#include <scenegraph/renderingdata/PortalCullNode.h>

#ifdef PLATFORM_SPU
#include <scenegraph/renderingdata/ModelInstanceCullNode.h>
#include <system/perf.h>
#else
#include <world_render/RenderModelData.h>
#include <world_render/ModelInstance.h>
#include <game_resource/ModelData.h>
#include <game_resource/ModelResource.h>
#include <scenegraph/SceneGraph.h>
#define SCENEGRAPH_ALLOW_INCLUDE
#include <scenegraph/SceneGraphImpl.h>
#undef SCENEGRAPH_ALLOW_INCLUDE
#endif

#include <animation/deferred/CommandProcessor.h>
#include <animation/skeleton/Skeleton.h>
#include <animation/pose/Pose.h>
#include <animation/pose/PoseFields.inl>
#include <animation/pose/PoseFormat.h>
#include <animation/pose/PoseMatrixCache.h>
#include <animation/pose/SkeletonPose.h>
#include <system/Perf/PerfEvent.h>
#include <core/maths/hvec/new/IncludeGeometry.h>
#include <core/maths/hvec/new/IncludeGeometry.inl>

namespace AnimatedModel
{

inline Maths::Sphere TransformSphere(const Maths::Sphere& sphere, const Maths::Mat43& m)
{
	Maths::Point3 centre  = Maths::Centre(sphere) * m;
	Maths::Vector3 radiusChoices = Maths::MakeVector3(Maths::Radius(sphere)) * m;
	Maths::VF32 radius = Maths::MaxAbsValue(radiusChoices);
	return Maths::MakeSphere(centre, radius);
}

inline Maths::Sphere MergeSpheres(const Maths::Sphere& sphereA, const Maths::Sphere& sphereB)
{
	Maths::VF32 radiusA = Maths::Radius(sphereA);
	Maths::VF32 radiusB = Maths::Radius(sphereB);
	const Maths::VF32 zero = Maths::Const<Maths::VF32>::Zero();
	
	if (radiusA == zero)
	{
		return sphereB;
	}
	
	Maths::VF32 distSquared = Maths::DistanceSquared(Maths::Centre(sphereA), Maths::Centre(sphereB));
	Maths::VF32 radiusDiff = Maths::Radius(sphereB) - Maths::Radius(sphereA);
	Maths::VF32 radiusDiffSquared = radiusDiff * radiusDiff;
	
	if (radiusDiffSquared > distSquared)
	{
		return (radiusDiff > zero) ? sphereB : sphereA;
	}
	
	Maths::VF32 dist = Maths::Sqrt(distSquared);
	
	const Maths::VF32 two = Maths::Const<Maths::VF32>::Two();
	const Maths::VF32 half = Maths::Const<Maths::VF32>::Half();
	
	Maths::VF32 mergedRadius = half * (dist + Maths::Radius(sphereA) + Maths::Radius(sphereB));
	
	Maths::VF32 k = (half * (dist + radiusDiff)) * Maths::Rcp(dist);
	Maths::Point3 mergedCentre = Maths::Lerp(Maths::Centre(sphereA), Maths::Centre(sphereB), k);
	
	return Maths::MakeSphere(mergedCentre, mergedRadius);
}

//
// C_AnimatedModelInstance
//


C_AnimatedModelInstance::C_AnimatedModelInstance():
	m_pSceneGraph(NULL),
	m_pPoseFormat(NULL),
	m_NearDist(0.f),
	m_FarDist(500.f),
	m_VisibilityFlags(0xFFFFFFFF),
	m_FirstSkinnedJointIndex(-1),
	m_Locked(0),
	m_Flags(E_Flag_HasMoved | E_Flag_FirstUpdate)
{
}

#ifndef PLATFORM_SPU

C_AnimatedModelInstance::C_AnimatedModelInstance(const GameResource::C_ModelResource* modelResource, U32 modelIndex, SceneGraph::C_SceneGraph* pSceneGraph, const Animation::C_PoseFormat& poseFormat):
	m_Locked(0),
	m_pSceneGraph(NULL),
	m_pPoseFormat(NULL),
	m_NearDist(0.f),
	m_FarDist(500.f),
	m_FirstSkinnedJointIndex(-1),
	m_Flags(E_Flag_HasMoved | E_Flag_FirstUpdate)
{
	Create(pSceneGraph, poseFormat);
	AddPart(modelResource, modelIndex);
}

C_AnimatedModelInstance::~C_AnimatedModelInstance()
{
	Destroy();
}

void C_AnimatedModelInstance::Create(SceneGraph::C_SceneGraph* pSceneGraph, const Animation::C_PoseFormat& poseFormat)
{
	Destroy();

	for (U32 fieldTypeIndex = 0; fieldTypeIndex < Animation::E_PoseFieldType_Count; ++fieldTypeIndex)
	{
		m_AnimatedParameterCounts[fieldTypeIndex] = 0;
	}
	
	m_AnimatedParameters.Clear();
	m_Elements.Clear();
	m_Parts.Clear();
	
	m_pPoseFormat = &poseFormat; // .... replace with pose layout, since that's ref counted?
	m_pSceneGraph = pSceneGraph;
	m_Locked = 0;
	
	m_Flags = E_Flag_HasMoved | E_Flag_FirstUpdate;
}

void C_AnimatedModelInstance::Create(const GameResource::C_ModelResource* modelResource, U32 modelIndex, SceneGraph::C_SceneGraph* pSceneGraph, const Animation::C_PoseFormat& poseFormat)
{
	Create(pSceneGraph, poseFormat);
	AddPart(modelResource, modelIndex);
}

S32 C_AnimatedModelInstance::AddPart(const GameResource::C_ModelResource* modelResource, U32 modelIndex)
{
	if (modelIndex >= static_cast<U32>(modelResource->GetRenderModelData()->GetNumModels()))
	{
		LOGWARN(Debug::GetLogger("mercury.gamesys.animated_model"), "Invalid model index %d used with model '%s'", modelIndex, modelResource->GetResourceFilename().Chars());
		return -1;
	}

	FlushIfLocked();
	
	Part part;
	MEMORY_TRACKER_PUSH("worldrender.modelinstance");
	part.modelInstance = new WorldRender::C_ModelInstance(modelResource->GetRenderModelData(), modelIndex, m_pSceneGraph);
	MEMORY_TRACKER_POP("worldrender.modelinstance");
	
	const GameResource::C_Model* gameModel = modelResource->GetModelData()->GetModel(modelIndex);
	const WorldRender::C_Model& worldRenderModel = modelResource->GetRenderModelData()->GetModel(modelIndex);
	
	//Maths::C_Sphere localSphere = gameModel->GetLocalBounds().BoundingSphere();
	Maths::C_Sphere localSphere = gameModel->GetBoundingSphere();
	part.localSphere = Maths::MakeSphere(localSphere.Centre(), localSphere.Radius());
	part.cullNode = NULL; // will be set when inserted into scene graph.
	part.firstElementIndex = m_Elements.Size();
	m_Elements.Resize(m_Elements.Size() + gameModel->GetElementCount());
	
	if (m_Parts.Size() > 0)
	{
		m_OriginalLocalBoundingSphere = MergeSpheres(m_LocalBoundingSphere, part.localSphere);
	}
	else
	{
		m_OriginalLocalBoundingSphere = part.localSphere;
	}
	m_LocalBoundingSphere = m_OriginalLocalBoundingSphere;
	m_WorldSphere = Maths::C_Sphere(Maths::Centre(m_OriginalLocalBoundingSphere), Maths::Radius(m_OriginalLocalBoundingSphere));
	
	m_Parts.PushBack(part);
	
	BindParameters(gameModel, &worldRenderModel, part.modelInstance, part.firstElementIndex);

	return m_Parts.Size() - 1;
}


#endif

bool C_AnimatedModelInstance::PreUpdate()
{
#ifndef PLATFORM_SPU
	bool inSceneGraph = true;
	for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
	{
		// Cache cull node pointer to avoid the need to fetch the ModelInstance object.
		m_Parts[i].cullNode = m_Parts[i].modelInstance->GetCullNode();
		inSceneGraph = inSceneGraph && (m_Parts[i].cullNode != NULL);
	}
	return inSceneGraph;
#else
	return true;
#endif
}

void C_AnimatedModelInstance::PostUpdate(const Animation::C_Pose& pose)
{
#ifndef PLATFORM_SPU
	if (m_Flags & E_Flag_AwaitingPostUpdate)
	{
		if (m_Flags & (E_Flag_HasAnimatedVisibility | E_Flag_HasMoved))
		{
			for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
			{
				const Part& p = m_Parts[i];
				
				if (m_Flags & E_Flag_HasMoved && p.cullNode != NULL)
				{
					if (p.cullNode->GetType() == SceneGraph::I_ModelInstanceCullNode::MODELINSTANCECULLNODE_TYPE_PORTAL)
					{
						SceneGraph::C_PortalCullNode* pPortalCullNode = (SceneGraph::C_PortalCullNode*)p.cullNode;
						BIZ_ASSERT(pPortalCullNode);		// Sanity check.
						pPortalCullNode->SetSpatialProbeAtIndex(0, p.modelInstance->GetWorldSphere().Centre());
					}			
					m_pSceneGraph->OnMove((SceneGraph::C_ModelInstance*)p.modelInstance);
				}
							
				if (m_Flags & E_Flag_HasAnimatedVisibility)
				{
					const F32 kVisibilityThreshold = 0.f;// 1.f / 128.f;
					U32 firstElementIndex = p.firstElementIndex;
					for (U32 j = 0, count = p.modelInstance->GetNumElements(); j < count; ++j)
					{
						const Element& e = m_Elements[firstElementIndex + j];
						
						if (e.visFieldIndex >= 0)
						{
							F32 fieldValue = pose.GetCustomScalar(e.visFieldIndex);
							bool visible = fieldValue > kVisibilityThreshold;
							p.modelInstance->SetElementVisibility(j, visible);	// could change to 0.5...
						}
					}
				}
			}
		}
		
		// Prevent multiple post updates
		m_Flags &= ~E_Flag_AwaitingPostUpdate;
	}
#endif // PLATFORM_SPU
}

FORCE_INLINE void StoreMat43(MathsStorage::MatrixRow44* dest, const Maths::Mat43& src)
{
	Maths::Store(Maths::MakeVector4(Maths::RowX(src)), &dest->v[0]);
	Maths::Store(Maths::MakeVector4(Maths::RowY(src)), &dest->v[4]);
	Maths::Store(Maths::MakeVector4(Maths::RowZ(src)), &dest->v[8]);
	Maths::Store(Maths::MakeVector4(Maths::MakePoint3(Maths::RowW(src))), &dest->v[12]);
}

FORCE_INLINE void StoreMat43(MathsStorage::MatrixRow43* dest, const Maths::Mat43& src)
{
	Maths::Store(Maths::ColX(src), &dest->v[0]);
	Maths::Store(Maths::ColY(src), &dest->v[4]);
	Maths::Store(Maths::ColZ(src), &dest->v[8]);
}


void C_AnimatedModelInstance::Update(const Maths::HMat43& worldMatrix, const Animation::C_Pose& pose, const Animation::C_PoseMatrixCache& poseMatrixCache)
{
	if (0 == m_Parts.Size() || 0 == m_Elements.Size())
	{
		// Nothing to do.
		return;
	}
	
	const Maths::Mat43 worldMatrixMat43 = worldMatrix;
	
	{
		Spu::MemBaseMarker elementMarker;
		
		const Element* pElements = (const Element*) Spu::FetchAllocUnaligned(&m_Elements[0], m_Elements.Size() * sizeof(Element));
			
		#if 0
		for (U32 i = 0; i < m_Parts.Size(); ++i)
		{
			m_Parts[i].modelInstance->SetElementMatrix(-1, worldMatrix);
		}
		#else
		//
		// Initially set all element matrices to be the instance's world matrix.
		// TODO: optimise to only set matrix for elements that don't have directly animated matrices i.e. skinned elements or the default element.
		//
		MathsStorage::MatrixRow44 rowMajorWorldMatrix;
		StoreMat43(&rowMajorWorldMatrix, worldMatrixMat43);
		
		for (U32 i = 0, count = m_Elements.Size(); i < count; ++i)
		{
			if (pElements[i].worldMatrix != NULL)
			{
			#ifndef PLATFORM_SPU
				*pElements[i].worldMatrix = rowMajorWorldMatrix;
				#if PLATFORM_PC
				WorldRender::Blur2PCInterpRecordTickStore(this, pElements[i].worldMatrix, worldMatrix);	// [Blur2PC] render interpolation
				#endif
			#else
				// TODO: assuming destination is 16-byte aligned... definitely a multiple of 16-bytes, but is it aligned?!
				Spu::Store(pElements[i].worldMatrix, &rowMajorWorldMatrix, sizeof(MathsStorage::MatrixRow44));
			#endif
			}
		}
		#endif
				
		//
		// Set parameters by type.
		//
		if (m_AnimatedParameters.Size() > 0)
		{
			const AnimatedParameter* pParams = (const AnimatedParameter*) Spu::FetchAllocUnaligned(&m_AnimatedParameters[0], sizeof(AnimatedParameter) * m_AnimatedParameters.Size());
			const AnimatedParameter* pFirstParam = pParams;
			
			Maths::Sphere boundingSphere = m_OriginalLocalBoundingSphere;
			
			if (m_AnimatedParameterCounts[Animation::E_PoseFieldType_JointTransform] > 0)
			{
				boundingSphere = Maths::MakeSphere(Maths::Const<Maths::Point3>::Zero(), Maths::Const<Maths::VF32>::Zero());

				{
					// Translate the original bounding sphere by the position of the first joint that influences a skinned mesh.
					// In the case of a single skinned element, this will create a reasonable conservative bounding sphere in most cases.
					// However, if joints translate outside the original bounds or there are multiple skinned elements, this bounding sphere
					// may be too small. Therefore, it might be worth using a more accurate (but costlier) bounding sphere computed
					// from the model-space positions of all skinned joints.
					const Maths::Mat43* pRootMatrix = &poseMatrixCache.GetSkinningMatrices()[m_FirstSkinnedJointIndex];
					
					boundingSphere = m_OriginalLocalBoundingSphere * (*pRootMatrix);
				}
				
				SetJointTransformParameters(pParams, m_AnimatedParameterCounts[Animation::E_PoseFieldType_JointTransform], pElements, pose, worldMatrixMat43, poseMatrixCache.GetSkinningMatrices(), boundingSphere);
				pParams += m_AnimatedParameterCounts[Animation::E_PoseFieldType_JointTransform];
			}

			BIZ_ASSERT(m_AnimatedParameterCounts[Animation::E_PoseFieldType_TrajectoryDeltaTransform] == 0);
			
			SetGenericParameters(pParams, m_AnimatedParameterCounts[Animation::E_PoseFieldType_CustomVector3], pose, Animation::E_PoseFieldType_CustomVector3);
			pParams += m_AnimatedParameterCounts[Animation::E_PoseFieldType_CustomVector3];

			SetGenericParameters(pParams, m_AnimatedParameterCounts[Animation::E_PoseFieldType_CustomScalar], pose, Animation::E_PoseFieldType_CustomScalar);
			pParams += m_AnimatedParameterCounts[Animation::E_PoseFieldType_CustomScalar];

			BIZ_ASSERT(m_AnimatedParameterCounts[Animation::E_PoseFieldType_TrajectoryTransform] == 0);
			
			SetVector4Parameters(pParams, m_AnimatedParameterCounts[Animation::E_PoseFieldType_CustomVector4], pose);
			pParams += m_AnimatedParameterCounts[Animation::E_PoseFieldType_CustomVector4];
			
			BIZ_ASSERT((pParams - pFirstParam) == m_AnimatedParameters.Size());

			m_LocalBoundingSphere = boundingSphere;
		}
	}

	// Convert local bounding sphere into world space and apply to the model instance
	Maths::Sphere worldBoundingSphere = m_LocalBoundingSphere * worldMatrixMat43;
	
	Maths::C_Sphere worldSphere(Maths::Centre(worldBoundingSphere), Maths::Radius(worldBoundingSphere));
	
	// Check to see if world matrix has changed.
	if(m_WorldSphere != worldSphere)
	{
		m_Flags |= E_Flag_HasMoved;
	}
	else
	{
		m_Flags &= ~E_Flag_HasMoved;
	}
		
	m_WorldSphere = worldSphere;
	
	Spu::MemBaseMarker animatedModelsMarker;
	
	const Part* pParts = (const Part*) Spu::FetchAllocUnaligned(&m_Parts[0], m_Parts.Size() * sizeof(Part));
		
	Maths::HVec4 worldSphereCentre(worldSphere.Centre());
	Maths::HVec4 worldSphereSize(worldSphere.Radius());
	
	for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
	{
		Spu::MemBaseMarker modelMarker;
		
		// TODO: cullnode alignment??? 
		if (pParts[i].cullNode != NULL)
		{
			SceneGraph::I_ModelInstanceCullNode* cullNode = (SceneGraph::I_ModelInstanceCullNode*) Spu::FetchAlloc(pParts[i].cullNode, sizeof(SceneGraph::I_ModelInstanceCullNode));

			Maths::Store(worldSphereCentre, &cullNode->centre);
			Maths::Store(worldSphereSize, &cullNode->size);
			
			#ifdef PLATFORM_SPU
			Spu::Store(pParts[i].cullNode, cullNode, sizeof(SceneGraph::I_ModelInstanceCullNode));
			#endif
		}
		else
		{
			// Ensure that portal classification occurs once the model has actually been inserted
			// into the scene graph.
			m_Flags |= E_Flag_HasMoved;
		}
	}
	
	if (m_Flags & E_Flag_FirstUpdate)
	{
		m_Flags |= E_Flag_HasMoved;
		m_Flags &= ~E_Flag_FirstUpdate;
	}
	
	m_Flags |= E_Flag_AwaitingPostUpdate;
}

#ifndef PLATFORM_SPU

void C_AnimatedModelInstance::SetWorldMatrix(const Maths::HMat43& worldMatrix)
{
	FlushIfLocked();

	Maths::Sphere worldBoundingSphere = m_LocalBoundingSphere * Maths::Mat43(worldMatrix);
	Maths::C_Sphere worldSphere(Maths::Centre(worldBoundingSphere), Maths::Radius(worldBoundingSphere));
	m_WorldSphere = worldSphere;
	for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
	{
		m_Parts[i].modelInstance->SetElementMatrix(-1, worldMatrix);
		m_Parts[i].modelInstance->SetWorldSphere(worldSphere);
	}
}

void C_AnimatedModelInstance::InsertIntoSceneGraph()
{
	if (!m_InSceneGraph)
	{
		FlushIfLocked();
		for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
		{
			SceneGraph::Insert(m_pSceneGraph, m_Parts[i].modelInstance);
			if (m_Parts[i].modelInstance->GetCullNode())
			{
				m_Parts[i].modelInstance->SetNearFarDistance(m_NearDist, m_FarDist);
			}
		}
		m_Flags |= E_Flag_FirstUpdate;
		m_InSceneGraph = true;
	}
}

void C_AnimatedModelInstance::RemoveFromSceneGraph()
{
	if (m_InSceneGraph)
	{
		FlushIfLocked();
		for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
		{
			SceneGraph::Remove(m_pSceneGraph, m_Parts[i].modelInstance);
			
			// Clear cullNode pointer since it will have been deleted.
			m_Parts[i].cullNode = NULL;
		}
		
		// No need to post update (for visibility, culling etc.) now that model is no longer in scene graph.
		m_Flags &= ~E_Flag_AwaitingPostUpdate;
		m_InSceneGraph = false;
	}
}

void C_AnimatedModelInstance::ScheduleUpdateTask(const Maths::HMat43& worldMatrix, const Animation::C_Pose& pose, const Animation::C_PoseMatrixCache& poseMatrixCache)
{
	// Trying to update the model instance more than once per frame will force a flush here.
	FlushIfLocked();
	
	AnimatedModel::TaskManager().ScheduleUpdateTask(
		AnimatedModel::E_Sync_PreRender,
		*this,
		worldMatrix,
		pose,
		poseMatrixCache
	);
}

#endif

void C_AnimatedModelInstance::SpuStore(C_AnimatedModelInstance* ppuInstance) const
{
#ifdef PLATFORM_SPU
	// Store just the variables that are modified during Update().
	// NB. these should be 16 byte aligned.
	const U32 kBytesToTransfer = (U8*)(&m_Flags + 1) - (U8*)(&m_LocalBoundingSphere);
	Spu::Store(&ppuInstance->m_LocalBoundingSphere, &m_LocalBoundingSphere, (kBytesToTransfer + 15) & ~15);
#endif
}


void C_AnimatedModelInstance::Lock(C_AnimatedModelInstance* ppuInstance)
{
	U32 prevLocked = MT::AtomicOr(&ppuInstance->m_Locked, 1);
	BIZ_ASSERT(prevLocked == 0);
	MT::MemoryFence();
}

void C_AnimatedModelInstance::Unlock(C_AnimatedModelInstance* ppuInstance)
{
	MT::MemoryFence();
	MT::AtomicAnd(&ppuInstance->m_Locked, ~1);
}

bool C_AnimatedModelInstance::IsLocked(const C_AnimatedModelInstance* ppuInstance)
{
	S32 isLocked = MT::AtomicGet((volatile U32*)&ppuInstance->m_Locked);
	
	if (isLocked == 0)
	{
		return false;
	}
	MT::MemoryFence();
	return true;
}

#ifndef PLATFORM_SPU

const WorldRender::C_ModelInstance* C_AnimatedModelInstance::GetModelInstance(U32 partIndex) const
{
	return (partIndex < (U32)m_Parts.Size()) ? m_Parts[partIndex].modelInstance : NULL;
}

WorldRender::C_ModelInstance* C_AnimatedModelInstance::GetModelInstance(U32 partIndex)
{
	return (partIndex < (U32)m_Parts.Size()) ? m_Parts[partIndex].modelInstance : NULL;
}

U32 C_AnimatedModelInstance::GetNumParts() const
{
	return m_Parts.Size();
}

bool C_AnimatedModelInstance::IsAnimated() const
{
	return m_AnimatedParameters.Size() > 0;
}

U32 C_AnimatedModelInstance::GetNumAnimatedParameters() const
{
	return m_AnimatedParameters.Size();
}

void C_AnimatedModelInstance::SetVisibilityFlags(U32 visibilityFlags)
{
	if (visibilityFlags != m_VisibilityFlags)
	{
		m_VisibilityFlags = visibilityFlags;
		for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
		{
			m_Parts[i].modelInstance->SetVisibilityFlags(visibilityFlags);
		}
	}
}

void C_AnimatedModelInstance::EnableVisibilityFlags(U32 visibilityFlags)
{
	SetVisibilityFlags(m_VisibilityFlags | visibilityFlags);
}

void C_AnimatedModelInstance::DisableVisibilityFlags(U32 visibilityFlags)
{
	SetVisibilityFlags(m_VisibilityFlags & ~visibilityFlags);
}

void C_AnimatedModelInstance::SetVisibility(bool visible)
{
	for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
	{
		for (U32 elementIndex = 0, numElements = m_Parts[i].modelInstance->GetNumElements(); elementIndex < numElements; ++elementIndex)
		{
			m_Parts[i].modelInstance->SetElementVisibility(elementIndex, visible);
		}
	}
}

void C_AnimatedModelInstance::SetVisibility(U32 partIndex, bool visible)
{
	if (partIndex >= GetNumParts())
	{
		return;
	}

	for (U32 elementIndex = 0, numElements = m_Parts[partIndex].modelInstance->GetNumElements(); elementIndex < numElements; ++elementIndex)
	{
		m_Parts[partIndex].modelInstance->SetElementVisibility(elementIndex, visible);
	}
}

void C_AnimatedModelInstance::SetNearFarDistance(F32 nearDist, F32 farDist)
{
	BIZ_ASSERT(Maths::Finite(farDist) && Maths::Finite(nearDist) &&
			farDist <= 10000.f && nearDist >= -10000.f);

	m_NearDist = nearDist;
	m_FarDist = farDist;
	
	if (m_InSceneGraph)
	{
		for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
		{
			if (m_Parts[i].modelInstance->GetCullNode())
			{
				m_Parts[i].modelInstance->SetNearFarDistance(m_NearDist, m_FarDist);
			}
		}
	}
}

Maths::C_Sphere C_AnimatedModelInstance::GetLocalSphereWithoutAnimation() const
{
	return Maths::C_Sphere(Maths::Centre(m_OriginalLocalBoundingSphere), Maths::Radius(m_OriginalLocalBoundingSphere));
}

void C_AnimatedModelInstance::SetRenderingDataRenderPassMapping(RenderingData::RenderPass renderingDataPass, S32 renderPass)
{
	for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
	{
		m_Parts[i].modelInstance->SetRenderingDataRenderPassMapping(renderingDataPass, renderPass);
	}
}

#if defined(PLATFORM_PS3)// || defined(PLATFORM_XENON)
#define ASSERT_PS3_ALIGN16(ptr)	BIZ_ASSERT((((U32)ptr) & 0xf) == 0)
#else
#define ASSERT_PS3_ALIGN16(ptr)
#endif

void C_AnimatedModelInstance::BindParameters(
	const GameResource::C_Model* gameModel,
	const WorldRender::C_Model* worldRenderModel,
	WorldRender::C_ModelInstance* modelInstance,
	U32 firstElementIndex)
{
	const Animation::C_PoseFormat* poseFormat = m_pPoseFormat;

	// Set up element information.
	Btl::String128 visFieldName;
	Btl::String16 visFieldSuffix(".visibility");
	for (S32 elementIndex = 0, count = gameModel->GetElementCount(); elementIndex < count; ++elementIndex)
	{
		const GameResource::C_ModelElement* pElement = gameModel->GetElement(elementIndex);
		
		Element& e = m_Elements[elementIndex + firstElementIndex];
		
		// Set bounding sphere.
		//Maths::C_Sphere localSphere = pElement->GetLocalBounds().BoundingSphere();
		Maths::C_Sphere localSphere = pElement->GetBoundingSphere();
		e.localSphere = Maths::MakeSphere(localSphere.Centre(), localSphere.Radius());
		
		// Get world matrix pointer.
		const WorldRender::MaterialParameter* elementMatrixParam = worldRenderModel->GetWorldMatrixParameter(elementIndex);
		e.worldMatrix = NULL;
		if (elementMatrixParam != NULL)
		{
			if (elementMatrixParam->index >= 0)
			{
				e.worldMatrix = (MathsStorage::MatrixRow44*) modelInstance->GetParameter(*elementMatrixParam);
				ASSERT_PS3_ALIGN16(e.worldMatrix);
			}
		}
		
		// Find optional visibility field index.
		visFieldName = pElement->GetName().Chars();
		visFieldName += visFieldSuffix;
		
		e.visFieldIndex = poseFormat->FindFieldByName(Animation::E_PoseFieldType_CustomScalar, visFieldName.Chars());
		
		if (e.visFieldIndex >= 0)
		{
			m_Flags |= E_Flag_HasAnimatedVisibility;
		}
	}

	// Bind animation pose fields onto animated parameters and element world matrices.		
	for (U32 fieldTypeIndex = 0; fieldTypeIndex < Animation::E_PoseFieldType_Count; ++fieldTypeIndex)
	{
		Animation::E_PoseFieldType fieldType = static_cast<Animation::E_PoseFieldType>(fieldTypeIndex);
		const U32 expectedParameterSize = GetParameterSize(fieldType);
		
		bool isMatrixField = false;
		if (fieldType == Animation::E_PoseFieldType_JointTransform)
		{
			isMatrixField = true;
		}
		
		const U32 firstParameterIndex = m_AnimatedParameters.Size();

		AnimatedParameter animatedParam;

		for (U32 fieldIndex = 0, count = poseFormat->GetFieldCount(fieldType); fieldIndex < count; ++fieldIndex)
		{
			const char* fieldName = poseFormat->GetFieldName(fieldType, fieldIndex).GetString();
			
			WorldRender::MaterialParameter materialParameter = worldRenderModel->GetParameter(fieldName);
						
			animatedParam.poseFieldIndex = fieldIndex;
			animatedParam.isElement = 0;

			// If there's no matching material parameter, try matching up the animation field name with an element name.
			if (materialParameter.index < 0 && isMatrixField && gameModel != NULL)
			{
				const GameResource::C_ModelElement* pElement = gameModel->GetElement(fieldName);
				
				if (pElement != NULL)
				{
					// Found a matching element, so try and obtain its world matrix parameter, which may or may not be present.
					S32 elementIndex = pElement->GetIndex();
					const WorldRender::MaterialParameter* elementMatrixParam = worldRenderModel->GetWorldMatrixParameter(elementIndex);
					if (elementMatrixParam != NULL)
					{
						materialParameter = *elementMatrixParam;
						animatedParam.isElement = 1;
						animatedParam.elementIndex = firstElementIndex + elementIndex;
					}
				}
			}
			
			if (materialParameter.index >= 0)
			{
				// Found a matching material parameter, now validate its size.
				//if (materialParameter.size == expectedParameterSize)
				{
					if (fieldType == Animation::E_PoseFieldType_JointTransform && m_FirstSkinnedJointIndex < 0)
					{
						m_FirstSkinnedJointIndex = fieldIndex;
					}
					animatedParam.data = modelInstance->GetParameter(materialParameter);
					ASSERT_PS3_ALIGN16(animatedParam.data);
					m_AnimatedParameters.PushBack(animatedParam);
				}
				/*else
				{
					BIZ_ASSERT_FAIL("Material parameter is an unexpected size");
				}*/
			}
			else
			{
#if defined(CFG_DEBUG)
				LOGDEBUG(Debug::GetLogger("mercury.gamesys.animated_model"), "No material parameter/element maps onto field %d, type %d: %s\n", fieldIndex, fieldTypeIndex, poseFormat->GetFieldName(fieldType,fieldIndex).GetString());
#endif
			}
		}
		m_AnimatedParameterCounts[fieldType] += m_AnimatedParameters.Size() - firstParameterIndex;
	}
}

#endif

void C_AnimatedModelInstance::SetJointTransformParameters(
	const AnimatedParameter* params,
	U32 count,
	const Element* elements,
	const Animation::C_Pose& pose,
	const Maths::Mat43& worldMatrix,
	const Maths::Mat43* skinningMatrices,
	Maths::Sphere& outBoundingSphere) const
{
	if (0 == count || skinningMatrices == NULL) return;
	
	for (U32 i = 0; i < count; ++i)
	{
		const AnimatedParameter* pParam = &params[i];
	
		S32 jointIndex = pParam->poseFieldIndex;
		Maths::Mat43 jointMatrix = skinningMatrices[jointIndex];
				
		if (pParam->isElement)
		{
			// Update overall bounding sphere using element's radius.
			Maths::Sphere modelSphere = TransformSphere(elements[pParam->elementIndex].localSphere, jointMatrix);
			Maths::Sphere mergedSphere = MergeSpheres(outBoundingSphere, modelSphere);
			outBoundingSphere = mergedSphere;
		
			// NB. assuming here that the element is not skinned. alas... pose matrix cache will have pre-multipled by the bind pose :(
			jointMatrix = jointMatrix * worldMatrix;

			U8* destParamData = pParam->data;

#ifdef PLATFORM_SPU
			MathsStorage::MatrixRow44 destMatrix;
			StoreMat43(&destMatrix, jointMatrix);
			Spu::Store(destParamData, &destMatrix, sizeof(MathsStorage::MatrixRow44));
#else
			MathsStorage::MatrixRow44* destMatrix = reinterpret_cast<MathsStorage::MatrixRow44*>(destParamData);
			StoreMat43(destMatrix, jointMatrix);
			#if PLATFORM_PC
			WorldRender::Blur2PCInterpRecordTickStore(this, destMatrix, Maths::HMat43(jointMatrix));	// [Blur2PC] render interpolation
			#endif
#endif

		}
		else
		{
			// Store matrix directly into material parameter using row-major order.
			U8* destParamData = pParam->data;
			
#ifdef PLATFORM_SPU
			MathsStorage::MatrixRow43 destMatrix;
			StoreMat43(&destMatrix, jointMatrix);
			Spu::Store(destParamData, &destMatrix, sizeof(MathsStorage::MatrixRow43));
#else
			MathsStorage::MatrixRow43* destMatrix = reinterpret_cast<MathsStorage::MatrixRow43*>(destParamData);
			StoreMat43(destMatrix, jointMatrix);
			#if PLATFORM_PC
			WorldRender::Blur2PCInterpRecordTickStore43(this, destMatrix, Maths::HMat43(jointMatrix));	// [Blur2PC] render interpolation
			#endif
#endif
		}
	}
}

void C_AnimatedModelInstance::SetVector4Parameters(const AnimatedParameter* params, U32 count, const Animation::C_Pose& pose) const
{
	if (0 == count) return;
	//WorldRender::C_ModelInstance* modelInstance = m_ModelInstance;
	for (U32 i = 0; i < count; ++i)
	{
		const AnimatedParameter& param = params[i];
		Maths::Vector4 v = pose.GetCustomVector4(param.poseFieldIndex);
#ifdef PLATFORM_SPU
		BIZ_STATIC_ASSERT(sizeof(Maths::Vector4) == sizeof(F32) * 4);
		Spu::Store(param.data, &v, sizeof(Maths::Vector4));
#else	
		Maths::Store(v, (F32*)param.data);
#endif
	}
}

void C_AnimatedModelInstance::SetGenericParameters(const AnimatedParameter* params, U32 count, const Animation::C_Pose& pose, Animation::E_PoseFieldType poseFieldType) const
{
	if (0 == count) return;
	const U32 parameterSize = GetParameterSize(poseFieldType);
	//WorldRender::C_ModelInstance* modelInstance = m_ModelInstance;
	for (U32 i = 0; i < count; ++i)
	{
		const AnimatedParameter& param = params[i];
		const void* fieldData = pose.GetField(poseFieldType, param.poseFieldIndex);
#ifdef PLATFORM_SPU
		Spu::Store(param.data, fieldData, parameterSize);
#else
		memcpy(param.data, fieldData, parameterSize);
#endif
	}
}

#ifndef PLATFORM_SPU

void C_AnimatedModelInstance::Destroy()
{
	FlushIfLocked();
	
	// Notify task manager so that PostUpdate doesn't get called on this object after it has been deleted.
	AnimatedModel::TaskManager().OnAnimatedModelInstanceDestroyed(this);
#if PLATFORM_PC
	WorldRender::Blur2PCInterpForget(this);	// [Blur2PC] render interpolation
#endif
		
	RemoveFromSceneGraph();

	for (U32 i = 0, count = m_Parts.Size(); i < count; ++i)
	{
		delete m_Parts[i].modelInstance;
		m_Parts[i].modelInstance = NULL;
	}
	m_Parts.Clear();
	m_Elements.Clear();
	m_AnimatedParameters.Clear();
	m_FirstSkinnedJointIndex = -1;
}

#endif

U32 C_AnimatedModelInstance::GetParameterSize(Animation::E_PoseFieldType fieldType) const
{
	static const U32 kJointTransformSize	= 12 * sizeof(F32);
	static const U32 kMatrixParameterSize	= 16 * sizeof(F32);
	static const U32 kVector4Size			= 4 * sizeof(F32);
	static const U32 kParameterSizes[] =
	{
		kJointTransformSize,	// JointTransform
		kMatrixParameterSize,	// TrajectoryDeltaTransform
		kVector4Size,			// Custom Vector3.
		kVector4Size,			// Custom scalar.
		kMatrixParameterSize,	// TrajectoryTransform
		kVector4Size			// Custom Vector4.
	};
	BIZ_STATIC_ASSERT(sizeof(kParameterSizes)/sizeof(kParameterSizes[0]) == Animation::E_PoseFieldType_Count);
	return kParameterSizes[fieldType];
}

void C_AnimatedModelInstance::FlushIfLocked() const
{
	if (IsLocked(this))
	{
		AnimatedModel::TaskManager().Flush();
	}
	return;
}

}
