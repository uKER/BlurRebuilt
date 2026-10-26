#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <core/core.h>
#include <core/maths.h>

#include "Aftk/Aftk.h"
#include "AftkUt/AftkUt.h"
#include "Aftkut/AftkUtNodeFlagParser.h"

#include <tools_debug/DebugManager.h>

#include "renderingdata/renderingdata.h"
#include "renderingdata/archive.h"
#include "renderingdata/lods.h"
#include "system/serialise/C_FileSystem.h"
#include "system/serialise/C_FileBlock.h"

#include "converter_shared/RenderDataConverter.h"
#include <texture_export/texturemanager.h>
#include <texture_export/textureprocessing.h>
#include <converter_shared/helpers/AftkMaterialHelpers.h>
#include "converter_shared/renderdataconversion/MaterialManager.h"
#include "converter_shared/renderdataconversion/ConvertAftkMesh.h"
#include "converter_shared/renderdataconversion/EffectManager.h"
#include "converter_shared/renderdataconversion/ConvertAftkMaterial.h"

#include "converter_shared/renderdataconversion/Common.h"
#include "converter_shared/renderdataconversion/Effect.h"
#include "converter_shared/renderdataconversion/PolyInform.h"
#include "converter_shared/renderdataconversion/NodeInform.h"
#include "converter_shared/renderdataconversion/clustering.h"

#include "converter_shared/renderdataconversion/plugins.h"
#include <converter_shared/helpers/AftkHelpers.h>

#include <converter_shared\CustomSourceSemantics\DIFF_IND0_NORM_BUMP.h>
#include <converter_shared\CustomSourceSemantics\Lightmap.h>
#include <converter_shared/CustomSourceSemantics/LodFade.h>
#include <converter_shared\CustomSourceSemantics\SkinningProcessor.h>
#include <converter_shared/CustomSourceSemantics/ScreenFacingSprite.h>
#include <converter_shared/CustomSourceSemantics/MorphTargetExportPlugin.h>
#include <converter_shared/CustomSourceSemantics/LocalisedCubemap.h>
#include <converter_shared/CustomSourceSemantics/ControlPointDeformationExportPlugin.h>
#include <converter_shared/CustomSourceSemantics/DefaultConstantPlugin.h>
#include <converter_shared/CustomSourceSemantics/LiveryTexcoordGenerator.h>
#include <converter_shared/CustomSourceSemantics/VehicleParameterExposeExportPlugin.h>
#include <converter_shared/CustomSourceSemantics/WindWeights.h>

#include "converter_shared/pvs/pvs.h"
#include <Portal/Builder.h>

#include <core/maths\hvec\New\IncludeMinimal.h>
#include <core/maths\hvec\New\IncludeMinimal.inl>

//#pragma optimize("", off)

const int TEXTURE_CACHE_VERSION = 11;

// register the default source semantics
void RegisterPlugins()
{
	static bool bRegistered = false;
	if (!bRegistered)
	{
		DIFF_IND0_NORM_BUMP::Register();
		Lightmap::Register();
		SkinningProcessor::Register();
		ScreenFacingSprite::Register();
		LocalisedCubemap::Register();
		MeshDeform::MorphTarget::Register();
		MeshDeform::ControlPoint::Register();
		LodFadePlugin::Register();
		LiveryTexcoordGenerator::Register();
		VehicleShaders::Register();
		WindWeights::Register();
		DefaultConstantPlugin::Register();
		RenderDataConversion::RegisterDefaultTextureAnnotationReader();
		RenderDataConversion::RegisterDefaultVtxInputSemantics();
		
		bRegistered = true;
	}
}

namespace RenderDataConversion
{
Util::C_String g_szDefaultFXName = "default.fx";
Util::C_String g_szInvalidFXName = "invalid.fx";
Util::C_String g_szSkinningVariation = "";
Util::C_String g_szTensionMapVariation = "_tm";
Util::C_String g_szGlobalEffectVariation = "";
extern bool g_bExportingModel;
}

namespace RenderingData
{

static Debug::Logger &g_Log = Debug::GetLogger("converter_shared");
	
/// constructor
C_Converter::C_Converter(const Util::C_String sTextureCachePath, const Util::C_String sFXBPath, bool bDeleteOldTextureCache)
{
	m_pSceneExport = new RenderingData::Build::Scene;
	m_pTextureManager = new TextureExport::TextureManager(sTextureCachePath, TEXTURE_CACHE_VERSION, bDeleteOldTextureCache);
	m_pMtlManager = new RenderDataConversion::MaterialManager(m_pSceneExport, m_pTextureManager, sFXBPath);
	m_pBatchIDGenerator = new BatchIDGenerator;
	m_pPolyInform = NULL;
	m_pNodeInform = NULL;
	m_pClusteringScene = 0;
	m_bSkinning = false;
	m_pSkinningProcessor = NULL;
	m_pvsData = NULL;
	m_pPortalBuilder = NULL;
	m_streamed = true;
	SetLODFadeDistance(0.2f);

	RegisterPlugins();
}

/// destructor
C_Converter::~C_Converter()
{
	delete m_pSceneExport;
	delete m_pTextureManager;
	delete m_pMtlManager;
	delete m_pBatchIDGenerator;
	assert(!m_pSkinningProcessor);
}

/// set the filename of the texture that will be used if the real texture can't be found on disk
void C_Converter::SetMissingTexture(const char *pszFilename)
{
	m_pTextureManager->SetMissingTextureFilename(pszFilename);
}

/// enable skinning on this converter
void C_Converter::EnableSkinning(bool bEnable, U32 maxMatrices, U32 maxVertexBlend,
								 const char *psz, const bool rotateY180)
{
	m_bSkinning = bEnable;
	m_iSkinningMaxMatrices = maxMatrices;
	m_iSkinningMaxVertexBlend = maxVertexBlend;
	m_bSkinningRotateY180 = rotateY180;
	RenderDataConversion::g_szSkinningVariation = psz;
}

void C_Converter::AddSkinningSkeletonNode(const Aftk::C_SceneNode* pSceneNode)
{
	if (pSceneNode)
		m_SkeletonNodes.PushBack(pSceneNode);
}

/// set the name of the effect to use when no specific effect was specified (for example in the HLSL shader)
void C_Converter::SetDefaultEffect(const char *psz)
{
	RenderDataConversion::g_szDefaultFXName = psz;	
}

/// set the name of the effect to use when an invalid effect was specified
void C_Converter::SetInvalidEffect(const char *psz)
{
	RenderDataConversion::g_szInvalidFXName = psz;	
}

/// set the variation to use for all shaders. this variation will be appended to the name of each .fx file
void C_Converter::SetEffectVariation(const char *psz)
{
	RenderDataConversion::g_szGlobalEffectVariation = psz;	
}

/// set a callback class which passes polyflag information to the renderdata builder
void C_Converter::SetPolyInform( const RenderDataConversion::I_PolyInform *p )
{
	m_pPolyInform = p;
}

/// set a callback class which passes node information to the renderdata builder
void C_Converter::SetNodeInform( const RenderDataConversion::I_NodeInform *p )
{
	m_pNodeInform = p;
}

/// set a callback class which can be used to query what cubemap texture filename is associated with a particular polygon
void C_Converter::SetCubemapInform( const RenderDataConversion::I_CubemapInform *p )
{
	LocalisedCubemap::SetCubemapInform(p);
}

/// set LOD fade distance. default is 0.2, 0 disables LOD fading (only works with models)
void C_Converter::SetLODFadeDistance( float f )
{
	m_fLODFadeDistance = f;
	LodFadePlugin::SetLODFadeDistance(f);
	float x = (1.f-f);
	m_fInvLODFadeDistanceSquared = x*x;
}

/// create a new cullnode and add it into the root node
RenderingData::ResourceIndex C_Converter::AddCullNode(const Btl::String &name, RenderingData::Node& parent, S32 blockIndex, S32 groupIndex)
{
	Btl::AutoPointer<RenderingData::CullNode> pNode;
	Allocate(pNode);
	pNode->SetName(name);
	pNode->SetGroupIndex(groupIndex);

	S32 nodeIndex = m_pSceneExport->InsertResourceBlock(blockIndex).AddNode(pNode);
	RenderingData::ResourceIndex nodeResourceIndex = RenderingData::ResourceIndex::BlockIndex(blockIndex, nodeIndex);

	parent.AddSubNode(nodeResourceIndex);

	return nodeResourceIndex;
}

/// begin the building of a new model
void C_Converter::BeginModel()
{
	m_ModelData.elementIndex = 0;
	m_ModelData.elementRootNode = NULL;

	Btl::String s;
	s.Printf("Model%d", m_ModelData.modelIndex);

	m_iCurrentNode = AddCullNode(s, m_pSceneExport->GetRootNode(), 0, -1);
}

/// add an element to the model that is currently being built
void C_Converter::AddElement( const Maths::HMat43 &worldMtx, const C_AftkSceneNodeList& aftkSceneNodeList, const Aftk::C_SceneNode* pRootNode, bool bSharedTextures)
{
	m_ModelData.elementMaterialIndex = GenerateElementMaterial(m_ModelData.elementIndex);
	m_ModelData.elementMatrix = worldMtx;
	m_ModelData.elementRootNode = pRootNode;

	m_pTextureManager->SetShared(bSharedTextures);

	Process(aftkSceneNodeList);

	//m_iCurrentElement++;
	m_ModelData.elementIndex++;
}

/// finish building the current model
void C_Converter::EndModel()
{
	m_ModelData.modelIndex++;
}

/// this is called once to build the rendering data
void C_Converter::Process()
{
	m_ModelData.elementIndex = -1;
	m_ModelData.elementMaterialIndex = -1;

	m_iCurrentNode = AddCullNode("Static", m_pSceneExport->GetRootNode(), 0, -1);

	m_pTextureManager->SetShared(false);

	Process(m_AftkSceneNodes);
}

/// returns true if we're exporting a model, false if we're exporting a scene
bool C_Converter::ExportingModel()
{
	return (m_ModelData.elementIndex != -1);
}

void C_Converter::SetStreamed(bool b)
{
	m_streamed = b;
}

/// process a list of aftk nodes and add them to the renderdata scene
void C_Converter::Process(const C_AftkSceneNodeList& aftkSceneNodes)
{
	RenderDataConversion::Clustering::Create(m_pClusteringScene);
	if (m_bSkinning)
	{
		m_pSkinningProcessor = SkinningProcessor::CreateSkinningProcessor(m_iSkinningMaxMatrices, m_iSkinningMaxVertexBlend);
		SkinningProcessor::SetSkinningProcessor(m_pSkinningProcessor);

		if (m_SkeletonNodes.Size())
		{
			Btl::Vector<const Aftk::C_SceneNode*> joints;

			AftkUt::CrowdHelper::BuildSkeletonJointMap(joints, m_SkeletonNodes);

			m_pSkinningProcessor->CreateJointList(joints);
		}
	}
	
	m_pClusteringScene->Begin();

	m_pClusteringScene->SetPVSData(m_pvsData);
	m_pClusteringScene->SetPortalData(m_pPortalBuilder);

	// first dummy pass just runs through the polygons to collect a list of all of the .fx files used
	// second pass gets the actual clustering data out
	for (int iPass=0; iPass<2; iPass++)
	{
		// process mesh nodes
		for(S32 i = 0; i < aftkSceneNodes.Size(); ++i)
		{
			const Aftk::C_SceneNode* pAftkSceneNode = aftkSceneNodes[i];

			if(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
			{
				Process1((const Aftk::C_SceneMeshNode*)pAftkSceneNode, iPass==0);
			}
		}

		// process instances
		for(S32 i = 0; i < aftkSceneNodes.Size(); ++i)
		{
			const Aftk::C_SceneNode* pAftkSceneNode = aftkSceneNodes[i];

			if(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneInstance ||
			   pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance)
			{
				if (!ExportingModel())
				{
					Process1(pAftkSceneNode, iPass==0);
				}
				else
				{
					LOGWARN( g_Log, "Attempt to export instance '%s' as part of a model/platform. This is currently not supported.\n", pAftkSceneNode->GetName().c_str());
				}
			}
		}
	}

	m_pClusteringScene->End();

	CreateModelNodes();

	if (m_pSkinningProcessor)
		m_pSkinningProcessor->Process();

	// process mesh nodes
	for(S32 i = 0; i < aftkSceneNodes.Size(); ++i)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = aftkSceneNodes[i];

		if(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			Process2((const Aftk::C_SceneMeshNode*)pAftkSceneNode);
		}
	}

	// process instances
	for(S32 i = 0; i < aftkSceneNodes.Size(); ++i)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = aftkSceneNodes[i];

		if(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneInstance ||
		   pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance)
		{
			if (!ExportingModel())
				Process2(pAftkSceneNode);
			else
				LOGDEBUG( g_Log, "Attempt to export instance '%s' as part of a model/platform. This is currently not supported.\n", pAftkSceneNode->GetName().c_str());
		}
	}

	m_modelPointers.Clear();
	m_instancePointers.Clear();
	RenderDataConversion::Clustering::Destroy(m_pClusteringScene);
	if (m_pSkinningProcessor)
	{
		SkinningProcessor::DestroySkinningProcessor(m_pSkinningProcessor);
		m_pSkinningProcessor = NULL;
	}
}

S32 C_Converter::InsertBaseModel(const Aftk::C_SceneNode* pNode)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	if (pNode->HasType(Aftk::E_ObjType_SceneMesh))
	{
		BaseInstanceMap::InsertionReturnType r = m_BaseInstanceMap.Insert(pNode);
		
		if (r.inserted)
		{
			BaseInstanceData baseInstanceData;
			baseInstanceData.iBaseModelIdx = sceneExport.AddBaseModel();
			baseInstanceData.pMesh = (const Aftk::C_SceneMeshNode*) pNode;

			r.iterator->GetValue() = baseInstanceData;

			RenderDataConversion::Clustering::Instance clusteringInstance;
			GetClusteringData(*pNode, baseInstanceData, &clusteringInstance);
			m_pClusteringScene->Add(clusteringInstance);
		}

		return r.iterator->GetValue().iBaseModelIdx;
	}

	return -1;
}

/// perform clustering on the Aftk mesh
void C_Converter::Process1(const Aftk::C_SceneMeshNode* pNode, bool bDummyRun)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	//if (GetLod(*pNode) > 0)
	//	return;

	bool bBaseInstance = pNode->GetNumInstancesOfThisNode() > 0;
	const U32 numPolys = pNode->GetNumPolygons(0);
	S32 baseModelIndex = -1;
	S32 iInstanceResetMtl = -1;

	if (bBaseInstance)
	{
		baseModelIndex = InsertBaseModel(pNode);
		if (!bDummyRun)
			m_BaseInstanceMap[pNode].process = 1;
	}
	else
	{
		// calculate instance reset material index for this aftk scene node
		if (!bDummyRun)
			iInstanceResetMtl = m_pMtlManager->GetInstanceResetMaterialIdx(pNode);
	}

	float dmin, dmax;
	bool lodFade = AftkHelpers::GetPolygonDrawDistance(*pNode, dmin, dmax);

	// Allow for fade distance.
	if (dmin > 0.f) dmin *= m_fInvLODFadeDistanceSquared;

	for (U32 i = 0; i < numPolys; i++)
	{
		const Aftk::C_Polygon& poly = pNode->GetPolygon(0, i);

		// if this polygon has the 'invisible' polyflag on then skip it
		if (m_pPolyInform && !m_pPolyInform->IsVisible(&poly))
			continue;

		RenderDataConversion::Clustering::Polygon clusteringPolygon;
		GetClusteringData(*pNode, i, baseModelIndex, &clusteringPolygon, lodFade);
		clusteringPolygon.bounds.distanceMin = dmin;
		clusteringPolygon.bounds.distanceMax = dmax;
		clusteringPolygon.instanceMtlIndex = iInstanceResetMtl;

		if (bDummyRun)
			continue;

		m_pClusteringScene->Add(clusteringPolygon);
	
		if (pNode->GetNumInfluenceClusters() > 0 && m_pSkinningProcessor)
		{
			SkinningProcessor::Polygon skinningPolygon(pNode, i);
			U32 mtlhash = 0;
			for (int i=0; i<clusteringPolygon.materialIndices.Size(); i++)
				Btl::Hash(mtlhash, clusteringPolygon.materialIndices[i]);
			
			m_pSkinningProcessor->Insert(skinningPolygon, mtlhash);
		}
	}
}

void C_Converter::Process1(const Aftk::C_SceneNode* pAftkInstance, bool bDummyRun)
{
	const Aftk::C_SceneNode* pSource = AftkHelpers::GetSource(pAftkInstance);

	if (pSource)
		InsertBaseModel(pSource);

	BaseInstanceMap::Iterator find = m_BaseInstanceMap.Find((const Aftk::C_SceneMeshNode*)pSource);

	if (find != m_BaseInstanceMap.End())
	{
		if (find->GetValue().process == 0)
		{
			Process1(find->GetValue().pMesh, bDummyRun);
			if (!bDummyRun)
				find->GetValue().process = 1;
		}

		RenderDataConversion::Clustering::Instance clusteringInstance;
		GetClusteringData(*pAftkInstance, find->GetValue(), &clusteringInstance);
		if (!bDummyRun)
			m_pClusteringScene->Add(clusteringInstance);
	}
	else
		int breakhere = 0; // base instance was not a mesh node
}

void C_Converter::GetInstanceBounds(const Aftk::C_SceneNode *pNode, const BaseInstanceData& data, RenderingData::Bounds& bounds)
{
	Aftk::C_Bounds aftkBounds = pNode->GetWorldBounds();

	bounds.SetUniversal();
	bounds.aabbMin = aftkBounds.GetMin();
	bounds.aabbMax = aftkBounds.GetMax();

	AftkHelpers::GetInstanceDrawDistance(*pNode, bounds.distanceMin, bounds.distanceMax);

	// Allow for fade distance.
	if (bounds.distanceMin > 0.f)
		bounds.distanceMin *= m_fInvLODFadeDistanceSquared;
}

void C_Converter::GetClusteringData(const Aftk::C_SceneNode& node, const BaseInstanceData& baseInstanceData, RenderDataConversion::Clustering::Instance* clusteringInstance)
{
	Aftk::C_Bounds bounds = node.GetEvaluatedWorldBounds();

	S32 routeNo = -1;
	if(m_pNodeInform)
		routeNo = m_pNodeInform->GetRouteNo( &node );

	GetInstanceBounds(&node, baseInstanceData, clusteringInstance->bounds);

	clusteringInstance->centroid = bounds.GetCentre();
	clusteringInstance->baseModelIndex = baseInstanceData.iBaseModelIdx;	
	clusteringInstance->groupIndex = routeNo;
	clusteringInstance->renderPass = RenderingData::RENDER_PASS_ALPHA_TEST;

	if (m_pvsData)
		clusteringInstance->pvsIndex = m_pvsData->GetPvsIndex(node.GetUniqueId(), -1, node.GetName().c_str());
	else 
		clusteringInstance->pvsIndex = -1;

	// Calculate portal room containment bit vector for the current instance (uses 
	// bounding box, accuracy potentially could be improved at cost of speed).
	if(m_pPortalBuilder)
		clusteringInstance->portalIndex = m_pPortalBuilder->CalculateContainmentBitVector(bounds);
	else
		clusteringInstance->portalIndex = -1;

	// get instance material data from Aftk node
	RenderDataConversion::Material convertedMtl;
	RenderDataConversion::AftkMaterialConverter converter(m_pMtlManager, &node, false);
	converter.Convert(&convertedMtl, &node);
	assert(convertedMtl.iMaterialID > -1);

	// if this instance should have instance-specific vertex data...
	if (convertedMtl.pEffect->GetNumVertexStreams(RenderDataConversion::E_NodeType_Instance) > 0)
		clusteringInstance->nPolygons = baseInstanceData.pMesh->GetNumPolygons(0);
	else
		clusteringInstance->nPolygons = 0;

	return;
}

void C_Converter::GetClusteringData(const Aftk::C_SceneMeshNode& node, S32 polygonIndex, S32 baseModelIndex, RenderDataConversion::Clustering::Polygon* clusteringPolygon, bool lodFade)
{
	const Aftk::C_Polygon& aftkPolygon = node.GetPolygon(0, polygonIndex);

	Aftk::Normal3 normal = aftkPolygon.GetNormal(&node.GetWorldMatrix());

	const Aftk::C_PolygonVertex aftkPolygonVertex0 = aftkPolygon.GetPolygonVertex(0);
	const Aftk::C_PolygonVertex aftkPolygonVertex1 = aftkPolygon.GetPolygonVertex(1);
	const Aftk::C_PolygonVertex aftkPolygonVertex2 = aftkPolygon.GetPolygonVertex(2);

	Aftk::Position3 position0 = Aftk::ApplyMatrix(node.GetWorldMatrix(), node.GetVertexPosition(aftkPolygonVertex0.GetPositionIndex()));
	Aftk::Position3 position1 = Aftk::ApplyMatrix(node.GetWorldMatrix(), node.GetVertexPosition(aftkPolygonVertex1.GetPositionIndex()));
	Aftk::Position3 position2 = Aftk::ApplyMatrix(node.GetWorldMatrix(), node.GetVertexPosition(aftkPolygonVertex2.GetPositionIndex()));

	RenderDataConversion::Material convertedMtl;
	RenderDataConversion::E_NodeType nodeType = baseModelIndex > -1?RenderDataConversion::E_NodeType_Base:RenderDataConversion::E_NodeType_Mesh;

	S32 iCustomKey = -1;
	GetMaterial(aftkPolygon, nodeType, &convertedMtl, iCustomKey);

	clusteringPolygon->normal = normal;
	clusteringPolygon->bounds.SetAABBTriangle(position0, position1, position2);
	clusteringPolygon->centroid = Lerp(clusteringPolygon->bounds.aabbMax, clusteringPolygon->bounds.aabbMin, 0.5f);
	clusteringPolygon->baseModelIndex = baseModelIndex;

	int n = convertedMtl.materialIDs.Size();
	clusteringPolygon->materialIndices.Resize(n);
	for (int i=0; i<n; i++)
		clusteringPolygon->materialIndices[i] = convertedMtl.materialIDs[i];

	int routeNo = -1;

	if(m_pNodeInform)
		routeNo = m_pNodeInform->GetRouteNo( &node );

	if (lodFade)
	{
		if (!convertedMtl.blendState.GetAlphaBlendEnable() && m_fLODFadeDistance > 0.f)
		{
			convertedMtl.blendState.SetAlphaTestEnable(true);
			convertedMtl.blendState.SetAlphaToMaskEnable(true);
		}
	}

	clusteringPolygon->groupIndex = routeNo;
	clusteringPolygon->renderPass = RenderingData::Build::GetRenderPass(convertedMtl.sortType, convertedMtl.blendState);

	if (m_pvsData)
		clusteringPolygon->pvsIndex = m_pvsData->GetPvsIndex(node.GetUniqueId(), polygonIndex, node.GetName().c_str());
	else
		clusteringPolygon->pvsIndex = -1;

	// Calculate portal containment bit vector for current Aftk polygon.
	if(m_pPortalBuilder)
	{
		// Check if this polygon should be portal culled.
		if (m_pPolyInform && !m_pPolyInform->IsPortalCulled(&aftkPolygon))
			clusteringPolygon->portalIndex = Portal::C_Builder::GetWorldRoomOnlyBitVector();
		else
			clusteringPolygon->portalIndex = m_pPortalBuilder->CalculateContainmentBitVector(node, aftkPolygon);
	}
	else
		clusteringPolygon->portalIndex = -1;			// No portal data.

	return;
}

/// function for external code to query the material on a polygon (used by PVS tool to detect alpha/double sided)
void C_Converter::GetMaterial(const Aftk::C_Polygon& aftkPolygon, RenderDataConversion::Material* convertedMtl)
{
	GetMaterial(aftkPolygon, RenderDataConversion::E_NodeType_Base, convertedMtl, -1);
}

/// get material for a given polygon
void C_Converter::GetMaterial(const Aftk::C_Polygon& aftkPolygon, RenderDataConversion::E_NodeType nodetype, RenderDataConversion::Material* pConvertedMtl, int iCustomKey)
{
	const Aftk::C_MaterialResource *pMtlRes = AftkHelpers::GetMaterialResourcePtr(&aftkPolygon);
	if (pMtlRes)
		m_pMtlManager->GetConvertedAftkMaterial(aftkPolygon.GetMeshNodePtr(), &pMtlRes->GetContent(), pConvertedMtl, iCustomKey, nodetype);
	else
		m_pMtlManager->GetConvertedAftkMaterial(aftkPolygon.GetMeshNodePtr(), NULL, pConvertedMtl, iCustomKey, nodetype);
}


/// returns a material index for the material containing element data for the nth element
int C_Converter::GenerateElementMaterial(int iElement)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	RenderingData::MaterialType materialType;
	materialType.SetName("Element");
	materialType.SetDynamicListIndex(0);
	RenderingData::MaterialElement materialElement;
	materialElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_U32;
	materialElement.count = 1;
	materialElement.name = "Index";
	materialType.AddElement(materialElement);
	int elementMtlType = sceneExport.GetMaterialList().InsertMaterialType(materialType);

	RenderingData::Material material;
	sceneExport.GetMaterialList().CreateMaterial(material, elementMtlType);
	material.SetData(0, iElement);
	return sceneExport.GetMaterialList().InsertMaterial(material);
}

void C_Converter::CreateClusterNode(RenderingData::ResourceIndex parent, const RenderDataConversion::Clustering::SceneNode& node)
{
	// Create model nodes.
	for (int i = 0; i < node.models.Size(); i++)
	{
		if (node.models[i].clusterType == RenderDataConversion::Clustering::SceneCluster::CLUSTER_TYPE_MODEL)
		{
			// Create a node and add it to the relevant subnode
			RenderingData::ResourceIndex nodeResourceIndex = m_pSceneExport->AddRenderNode(parent.GetBlock());
			m_pSceneExport->GetNodePtr(parent)->AddSubNode(nodeResourceIndex);
			RenderingData::RenderNode* pRenderNode = RenderingData::RenderNode::GetPointer(m_pSceneExport->GetNodePtr(nodeResourceIndex));

			// Add a model to the node.
			if (pRenderNode)
			{
				Btl::AutoPointer<RenderingData::Build::Model> pModel;
				Allocate(pModel);
				pRenderNode->SetRenderObject(pModel);
			}

			RenderingData::Build::Model* pModel = RenderingData::Build::Model::GetPointer(pRenderNode->GetRenderObjectPtr());

			// if this is a model element set the element material
			if (m_ModelData.elementIndex != -1)
				pModel->SetBaseMaterialIndex(m_ModelData.elementMaterialIndex);

			m_modelPointers[node.models[i].itemIndex] = pModel;

			// Copy the pvs data from the cluster to the render node.
			if (m_pvsData)
				pRenderNode->GetPvsBits() = node.models[i].pvsBits;

			if(m_pPortalBuilder)
				pRenderNode->GetPortalBits() = node.models[i].roomBits;
		}
	}

	// Create instance nodes.
	for (int i = 0; i < node.instances.Size(); i++)
	{
		// Create a node and add it to the relevant subnode
		RenderingData::ResourceIndex nodeResourceIndex = m_pSceneExport->AddRenderNode(parent.GetBlock());
		m_pSceneExport->GetNodePtr(parent)->AddSubNode(nodeResourceIndex);
		RenderingData::RenderNode* pRenderNode = RenderingData::RenderNode::GetPointer(m_pSceneExport->GetNodePtr(nodeResourceIndex));

		m_instancePointers[node.instances[i].itemIndex] = pRenderNode;

		// Copy the pvs data from the cluster to the render node.
		if (m_pvsData)
			pRenderNode->GetPvsBits() = node.instances[i].pvsBits;

		if(m_pPortalBuilder)
			pRenderNode->GetPortalBits() = node.instances[i].roomBits;
	}

	for (int i = 0; i < node.subNodes.Size(); i++)
	{
		RenderingData::ResourceIndex nodeResourceIndex;

		// The cull node acts as a separator for render list nodes.
		// With no cull node, all the models will be collapsed to a single list.
		if (!ExportingModel())
			nodeResourceIndex = AddCullNode("", *m_pSceneExport->GetNodePtr(parent), node.subNodes[i]->blockIndex + m_iBlockIndexOffset, node.subNodes[i]->groupIndex);
		else
			nodeResourceIndex = parent;

		CreateClusterNode(nodeResourceIndex, *node.subNodes[i]);
	}
}

void C_Converter::CreateModelNodes()
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	m_iBlockIndexOffset = sceneExport.GetNumResourceBlocks();

	const RenderDataConversion::Clustering::SceneNode& node = m_pClusteringScene->GetRootNode();
	CreateClusterNode(m_iCurrentNode, node);
}

RenderingData::PrimitiveInfo::StreamInfo *GetStream(RenderingData::PrimitiveInfo *pPrim, int iStream)
{
	for (int i=0; i<pPrim->streams.Size(); i++)
		if (pPrim->streams[i].streamIndex == iStream)
			return &pPrim->streams[i];

	pPrim->streams.PushBack();
	pPrim->streams.Back().streamIndex = iStream;
	return &pPrim->streams.Back();
}

/// construct a RenderingData::PrimitiveInfo from a given Aftk polygon
bool C_Converter::GetPrimitiveInfo(const RenderDataConversion::AftkMeshConverter *pMesh, int iPoly, RenderDataConversion::E_NodeType nodeType, RenderingData::PrimitiveInfo *pPrimitiveInfo, RenderDataConversion::Material *pConvertedMtl)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;
	const Aftk::C_SceneMeshNode *pNode = pMesh->GetMeshNode();
	const Aftk::C_Polygon& poly = pNode->GetPolygon(0, iPoly);

	// if this polygon has the 'invisible' polyflag on then skip it
	if (m_pPolyInform && !m_pPolyInform->IsVisible(&poly))
		return false;

	// Create a primitive
	pPrimitiveInfo->primitiveType = RenderingData::PrimitiveInfo::PRIMITIVE_TYPE_TRIANGLE;

	// get the material for this polygon
	int iCustomKey = -1;
	if (pNode->GetNumInfluenceClusters() > 0 && m_pSkinningProcessor)
	{
		iCustomKey = m_pSkinningProcessor->GetMatrixPaletteIndex(SkinningProcessor::Polygon(pMesh->GetMeshNode(), iPoly));
	}
	GetMaterial(poly, nodeType, pConvertedMtl, iCustomKey);

	// get vertex data for this polygon
	pMesh->GetVertexData(pConvertedMtl->pEffect, iPoly, pPrimitiveInfo, nodeType);

	// set material to primitive
	int nActiveStreams = 0;
	for (int iStream=0; iStream<pConvertedMtl->materialIDs.Size(); iStream++)
	{
		int iMtl = pConvertedMtl->materialIDs[iStream];
		if (iMtl == -1)
			continue;
		nActiveStreams++;
		RenderingData::PrimitiveInfo::StreamInfo *pStream = GetStream(pPrimitiveInfo, iStream);
		pStream->materialIndex = iMtl;
	}
	// if there are any vertex streams with no materials on them, put empty materials on those streams
	for (int i=0; i<pPrimitiveInfo->streams.Size(); i++)
	{
		if (pPrimitiveInfo->streams[i].materialIndex == -1)
			pPrimitiveInfo->streams[i].materialIndex = m_pMtlManager->CreateEmptyMaterial();
	}
	
	pPrimitiveInfo->base.sortType = pConvertedMtl->sortType;
	pPrimitiveInfo->base.blendState = pConvertedMtl->blendState;
	pPrimitiveInfo->base.effect = pConvertedMtl->pEffect->GetEffectIndex();
	pPrimitiveInfo->textureInfo = pConvertedMtl->textureInfo;
	int iCustomFlags = (pConvertedMtl->iCustomFlags | m_pPolyInform->GetCustomFlags(&poly));
	if (iCustomFlags&128)	// apply the "force include in prepass" polyflag
		iCustomFlags&=~(4|128);
	iCustomFlags<<=16;
	if (m_ModelData.elementIndex != -1)
	{
		assert((iCustomFlags & m_ModelData.elementIndex) == 0); // custom flags and model index overlap
		pPrimitiveInfo->base.flags = m_ModelData.elementIndex | iCustomFlags;
	}
	else
		pPrimitiveInfo->base.flags = iCustomFlags;

	// if this polygon has the 'invisible' polyflag on then skip it
	if (m_pPolyInform && m_pPolyInform->DoNotBackfaceCull(&poly))
		pPrimitiveInfo->base.blendState.SetCullMode(CULL_MODE_NONE);

	return true;
}

/// copy the UV/texture info into the texture data
/// (this is for the streamer so it knows how big each texture appears on the polygon)
void C_Converter::SetTextureInfo(RenderingData::Build::Scene &sceneExport, RenderDataConversion::AftkMeshConverter &mesh, RenderingData::PrimitiveInfo &primitiveInfo, int poly, const Aftk::C_SceneMeshNode *pNode)
{
	for (int i = 0; i < primitiveInfo.textureInfo.Size(); i++)
	{
		RenderingData::PrimitiveInfo::TextureInfo *pTI = &primitiveInfo.textureInfo[i];
		RenderingData::ResourceIndex r = sceneExport.GetTexture(pTI->textureIndex);
		const RenderingData::Build::Texture& texture = sceneExport.GetTexture(r);
		RenderDataConversion::AftkMeshConverter::TextureScale textureScale;
		// find the named UV set and use that if possible

		bool found = false;

		if (pTI->sUVSet != "")
		{
			for (U32 iUV=0; iUV<pNode->GetNumTextureCoordinateSets(); iUV++)
			{
				if (!stricmp(pNode->GetTextureCoordinateSetName(iUV).c_str(), pTI->sUVSet.Chars()))
				{
					mesh.CalculateTextureScale(pNode->GetPolygon(poly), iUV, textureScale);
					texture.UpdateScale(textureScale.uScale, textureScale.vScale, textureScale.weight);
					found = true;
					break;
				}
			}
			// this could happen when looking for "luxlightmap" on an unlit scene, or because of artist error
			// use numbered UV-set or hard-coded defaults instead
		}

		if (!found)
		{
			// either use preset values or numbered UV-set
			if (pTI->uvSet == -1)
			{
				textureScale.uScale = pTI->uScale;
				textureScale.vScale = pTI->vScale;
				textureScale.weight = pTI->weight;
			}
			else
			{
				mesh.CalculateTextureScale(pNode->GetPolygon(poly), pTI->uvSet, textureScale);
			}
			texture.UpdateScale(textureScale.uScale, textureScale.vScale, textureScale.weight);
		}
	}
}

void C_Converter::Process2(const Aftk::C_SceneMeshNode* pNode, bool addInstance)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	std::string name = pNode->GetName();

	//if (GetLod(*pNode) > 0)
	//	return;

	// get instances of this node
	const U32 numInstances = pNode->GetNumInstancesOfThisNode();
	int iInstanceMtlTypeIdx = -1;
	
	bool bBaseInstance = false;
	S32 baseModelIndex = -1;

	BaseInstanceMap::Iterator pBaseInstanceData = m_BaseInstanceMap.Find(pNode);

	if (pBaseInstanceData != m_BaseInstanceMap.End())
	{
		bBaseInstance = true;
		baseModelIndex = pBaseInstanceData->GetValue().iBaseModelIdx;
	}

	S32 effectIndex = -1;
	Util::C_String sInstanceEffectName;

	const U32 numPolys = pNode->GetNumPolygons(0);
	
	RenderDataConversion::AftkMeshConverter mesh(pNode);
	
	// set mesh converter matrices depending on whether we are processing an element or not.
	// Skinned meshes need to have their vertices stored in world space...?
	const bool skinned = (pNode->GetNumInfluenceClusters() > 0 && m_pSkinningProcessor);
	
	if (skinned)
	{
		mesh.SetBasePoseMatrices(m_bSkinningRotateY180);
	}
	else if(m_ModelData.elementIndex != -1)
	{
		mesh.SetMatrices(Maths::HMat44(m_ModelData.elementMatrix));
	}
	else
	{
		mesh.SetMatrices();
	}

	RenderDataConversion::Material convertedMtl;

	float dmin, dmax;
	bool lodFade = AftkHelpers::GetPolygonDrawDistance(*pNode, dmin, dmax);

	// Allow for fade distance.
	if (dmin > 0.f) dmin *= m_fInvLODFadeDistanceSquared;

	// calculate instance reset material index for this aftk scene node
	S32 iInstanceResetMtl = -1;
	if (!bBaseInstance)
		iInstanceResetMtl = m_pMtlManager->GetInstanceResetMaterialIdx(pNode);

	// add polygons into rendernode
	RenderDataConversion::Effect *pLastEffect = NULL;
	for (U32 poly = 0; poly < numPolys; poly++)
	{
		// get mesh data for this polygon
		RenderingData::PrimitiveInfo primitiveInfo;
		RenderDataConversion::E_NodeType nodeType = bBaseInstance?RenderDataConversion::E_NodeType_Base:RenderDataConversion::E_NodeType_Mesh;
		// [get primitive info]
		if (GetPrimitiveInfo(&mesh, poly, nodeType, &primitiveInfo, &convertedMtl))
		{
			// if this is a base instance, check that the data that goes on the instance is consistent across the effects on the mesh
			if (bBaseInstance)
			{
				if (poly == 0)
				{
					pBaseInstanceData->GetValue().pEffect = convertedMtl.pEffect;
				}

				// update the list of parameters that are required to be on the instance
				if (convertedMtl.pEffect != pLastEffect)
				{
					pLastEffect = convertedMtl.pEffect;
					pBaseInstanceData->GetValue().instanceMtl.AddAnnotationReadersFromEffect(convertedMtl.pEffect, RenderDataConversion::E_NodeType_Instance);
				}
			}

			// get cluster
			RenderDataConversion::Clustering::Polygon clusteringPolygon;
			GetClusteringData(*pNode, poly, baseModelIndex, &clusteringPolygon, lodFade);
			clusteringPolygon.bounds.distanceMin = dmin;
			clusteringPolygon.bounds.distanceMax = dmax;
			clusteringPolygon.instanceMtlIndex = iInstanceResetMtl;
			const RenderDataConversion::Clustering::SceneCluster& cluster = m_pClusteringScene->Find(clusteringPolygon);

			if (lodFade)
			{
				primitiveInfo.base.bounds.distanceMin = dmin;
				primitiveInfo.base.bounds.distanceMax = dmax;

				if (!primitiveInfo.base.blendState.GetAlphaBlendEnable() && m_fLODFadeDistance > 0.f)
				{
					primitiveInfo.base.blendState.SetAlphaTestEnable(true);
					primitiveInfo.base.blendState.SetAlphaToMaskEnable(true);
				}
			}

			// set block/model
			int iBlock, instanceID;
			
			if (m_streamed) // put into separate blocks
			{
				iBlock = cluster.blockIndex + m_iBlockIndexOffset;
				instanceID = -1;
			}
			else // put in the same block and use instanceID to separate vertex buffers
			{
				iBlock = 0;
				instanceID = cluster.blockIndex + (m_ModelData.elementIndex << 16);
			}

			for (int i=0; i<primitiveInfo.streams.Size(); i++)
			{
				if (primitiveInfo.streams[i].vertices.Size() > 0)
				{
					primitiveInfo.streams[i].resourceBlockIndex = iBlock;
					primitiveInfo.streams[i].instanceID = instanceID;
				}
			}
			primitiveInfo.base.resourceBlockIndex = iBlock;

			primitiveInfo.batchID = m_pBatchIDGenerator->GetBatchID(primitiveInfo, baseModelIndex);
			
			// remember the batch ID for use when building the mesh for the instance
			if (bBaseInstance)
				pBaseInstanceData->GetValue().m_iBatchIDs.PushBack(primitiveInfo.batchID);

			RenderingData::Build::Model* pModel;
			
			if (bBaseInstance)
				pModel = &sceneExport.GetBaseModel(baseModelIndex);
			else
				pModel = m_modelPointers.Find(cluster.itemIndex)->GetValue();

			pModel->SetInstanceMaterialIndex( clusteringPolygon.instanceMtlIndex );

			if (pModel->GetBaseMaterialIndex() != -1)
				primitiveInfo.GenerateSortCode( convertedMtl.decalOrder, pModel->GetBaseMaterialIndex() ); 
			else
				primitiveInfo.GenerateSortCode( convertedMtl.decalOrder );

			sceneExport.AddPrimitive(primitiveInfo, pModel);

			// copy the UV/texture info into the texture data
			// (this is for the streamer so it knows how big each texture appears on the polygon)
			SetTextureInfo(sceneExport, mesh, primitiveInfo, poly, pNode);
		}
		else
		{
			if (bBaseInstance)
				pBaseInstanceData->GetValue().m_iBatchIDs.PushBack(-1);
		}
	}

	if (bBaseInstance && addInstance)
	{
		// add an instance at the base model's location
		CreateInstance(pNode, pBaseInstanceData->GetValue());
		m_BaseInstanceMap[pNode].process = 2;
	}
}

/// this is called once to build the rendering data
void C_Converter::Process2(const Aftk::C_SceneNode* pAftkInstance)
{
	const Aftk::C_SceneNode* pSource = AftkHelpers::GetSource(pAftkInstance);

	if(pSource->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		BaseInstanceMap::Iterator find = m_BaseInstanceMap.Find((const Aftk::C_SceneMeshNode*)pSource);

		if(find != m_BaseInstanceMap.End())
		{
			if (find->GetValue().process == 1)
			{
				Process2(find->GetValue().pMesh, false);
				find->GetValue().process = 2;
			}

			CreateInstance(pAftkInstance, find->GetValue());
		}
	}
}

void C_Converter::CreateInstance(const Aftk::C_SceneNode *pNode, BaseInstanceData &data)
{
	RenderDataConversion::Clustering::Instance clusteringInstance;
	GetClusteringData(*pNode, data, &clusteringInstance);
	const RenderDataConversion::Clustering::SceneCluster& cluster = m_pClusteringScene->Find(clusteringInstance);

	RenderingData::RenderNode* pInstanceRenderNode = m_instancePointers.Find(cluster.itemIndex)->GetValue();
	
	assert(pInstanceRenderNode);
	//assert(pInstanceRenderNode->GetRenderObjectPtr() == 0);

	if (pInstanceRenderNode->GetRenderObjectPtr() == 0) // = RenderingData::RenderNode::GetPointer(m_pSceneExport->GetNodePtr(nodeResourceIndex)))
	{
		std::string instanceName = pNode->GetName();
		pInstanceRenderNode->SetName(instanceName.c_str());

		// store bounds on RenderNode
		RenderingData::Bounds bounds;
		GetInstanceBounds(pNode, data, bounds);

		{
			F32 boundsCentreX = 0.f;
			F32 boundsCentreY = 0.f;
			F32 boundsCentreZ = 0.f;
			
			F32 boundsExtentsMinXZ = 0.f;
			F32 boundsExtentsMaxXZ = 0.f;
			F32 boundsExtentsMinY = 0.f;
			F32 boundsExtentsMaxY = 0.f;
			
			F32 temp = 0.f;

			AftkUt::C_NodeFlagParser nodeFlags(*pNode);
			if (nodeFlags.GetParameterValue("BoundsCentre", boundsCentreX, boundsCentreY, boundsCentreZ, temp))
			{
				RenderingData::Point3 boundsMin = bounds.aabbMin;
				RenderingData::Point3 boundsMax = bounds.aabbMax;
				RenderingData::Point3 boundsCentre = bounds.GetCentre();

				RenderingData::Point3 centre(boundsCentreX, boundsCentreY, boundsCentreZ);

				for (S32 i = 0, count = 3; i < count; ++i)
				{
					bounds.aabbMin[i] = centre[i] + (boundsMin[i] - boundsCentre[i]);
					bounds.aabbMax[i] = centre[i] + (boundsMax[i] - boundsCentre[i]);
				}
			}
			if (nodeFlags.GetParameterValue("BoundsExtents", boundsExtentsMinXZ, boundsExtentsMaxXZ, boundsExtentsMinY, boundsExtentsMaxY))
			{
				RenderingData::Point3 boundsMin(boundsExtentsMinXZ, boundsExtentsMinY, boundsExtentsMinXZ);
				RenderingData::Point3 boundsMax(boundsExtentsMaxXZ, boundsExtentsMaxY, boundsExtentsMaxXZ);

				RenderingData::Point3 centre = bounds.GetCentre();

				for (S32 i = 0, count = 3; i < count; ++i)
				{
					bounds.aabbMin[i] = centre[i] + boundsMin[i];
					bounds.aabbMax[i] = centre[i] + boundsMax[i];
				}
			}
		}
		
		pInstanceRenderNode->SetBounds(bounds);

		Btl::AutoPointer<RenderingData::Build::Instance> pInstance;
		Allocate(pInstance);
		pInstance->SetBaseModelIndex(data.iBaseModelIdx);

		// generate instance material
		S32 paletteSortId = -1;
		RenderDataConversion::AftkMaterialConverter mtlConverter(m_pMtlManager, NULL, pNode, paletteSortId, RenderDataConversion::E_NodeType_Instance);
		int iMtlIdx = m_pMtlManager->GetMaterialIndex(&mtlConverter, &data.instanceMtl, "Instance");
		pInstance->SetMaterialIndex(iMtlIdx);

		// some instances have mesh data on the instance itself e.g. vertex colours
		// here we collect this data
		const U32 numPolys = data.pMesh->GetNumPolygons(0);
		RenderDataConversion::AftkMeshConverter mesh(data.pMesh, pNode);

		// if this instance requires instance-specific vertex data...
		if (data.pEffect && data.pEffect->GetNumVertexStreams(RenderDataConversion::E_NodeType_Instance) > 0)
		{
			// add polygons to instance
			for (U32 i = 0; i < numPolys; i++)
			{
				if (data.m_iBatchIDs[i] < 0)
					continue;

				RenderingData::PrimitiveInfo primitiveInfo;

				// get vertex data for this polygon
				mesh.GetVertexData(data.pEffect, i, &primitiveInfo, RenderDataConversion::E_NodeType_Instance);

				//
				RenderingData::PrimitiveInfo::StreamInfo& stream = primitiveInfo.streams.Back();
				
				// set material to primitive
				for (int iStream=0; iStream<primitiveInfo.streams.Size(); iStream++)
				{
					RenderingData::PrimitiveInfo::StreamInfo &stream = primitiveInfo.streams[iStream];
					assert(stream.vertices.Size() > 0);

					stream.resourceBlockIndex = cluster.blockIndex + m_iBlockIndexOffset;
					// offset the stream index by the number of streams on the base material, so they don't conflict
					stream.streamIndex += data.pEffect->GetNumVertexStreams(RenderDataConversion::E_NodeType_Base);
					assert(stream.streamIndex<MAX_VERTEX_STREAMS);

					stream.instanceID = cluster.itemIndex;
				}
				primitiveInfo.batchID = data.m_iBatchIDs[i];

				m_pSceneExport->AddPrimitive(primitiveInfo, &(*pInstance));
			}
		}

		pInstanceRenderNode->SetRenderObject(pInstance);
	}
	//m_iCurrentInstanceId++;
}

void C_Converter::SetSceneName(const char* name)
{
	m_pSceneExport->SetName(name);
}

/// add list of Aftk nodes that are to be built into a single static mesh
void C_Converter::AddStaticNodes(const C_AftkSceneNodeList& aftkSceneNodeList)
{
	RenderDataConversion::g_bExportingModel = false;

	for(S32 i = 0; i < aftkSceneNodeList.Size(); ++i)
	{
		m_AftkSceneNodes.PushBack(aftkSceneNodeList[i]);
	}
}

void C_Converter::BuildStreamData(RenderingData::Build::Scene& scene, RenderingData::PlatformType platformType)
{
	RenderingData::Build::StreamData& streamData = scene.GetStreamData();

	S32 chunk = 0;
	S32 blockOffset = m_iBlockIndexOffset;

	for (int block = blockOffset; block < scene.GetNumResourceBlocks(); block++)
	{
		if (scene.GetResourceBlock(block).GetNumNodes() > 0)
		{
			for (int lod = 0; lod < RenderingData::LodCalculator::GetNumLods(); lod++)
				streamData.AddBlockToChunk(chunk, block, lod);

			chunk++;
		}
	}

	//for (int i = 0; i < 10; i++)
	{
		RenderDataConversion::Clustering::I_StreamData* pStreamData;
		RenderDataConversion::Clustering::Create(pStreamData);

		pStreamData->Begin(chunk, 0);//i);

		S32 totalSkipped = 0;

		for (int block = blockOffset; block < scene.GetNumResourceBlocks(); block++)
		{
			RenderingData::Build::ResourceBlock& resourceBlock(scene.GetResourceBlock(block));

			if (!resourceBlock.Empty()
				&& resourceBlock.GetNumNodes() == 0)
			{
				RenderDataConversion::Clustering::StreamElement element;
				element.blockIndex = block;
				element.bounds = resourceBlock.GetFullBounds();
				element.centroid = element.bounds.GetCentre();
				element.groupBits = resourceBlock.GetGroupBits();

				resourceBlock.CreateLodPvs();

				// Create Pvs for each lod of the block.
				if (m_pvsData)
				{
					for (int cellIndex = 0; cellIndex < m_pvsData->GetNumPvsCells(); cellIndex++)
					{
						float fmin = FLT_MAX;

						for (int ib = 0; ib < resourceBlock.GetNumBounds(); ib++)
						{
							// Get the distance from the cell to the bounding box.
							RenderingData::Point3 centreBlock = resourceBlock.GetSubBounds(ib).GetCentre();
							RenderingData::Vector3 sizeBlock = resourceBlock.GetSubBounds(ib).GetSize() * 0.5f;

							Maths::Mat44 mat44 = Maths::Load<Maths::HMat44, MathsStorage::Matrix44>(&m_pvsData->GetCellMatrix(cellIndex));
							Maths::Vector4 centreBlock4 = Maths::MakeVector4(centreBlock.X(), centreBlock.Y(), centreBlock.Z(), 1.f);
							Maths::Vector4 sizeBlock4 = Maths::MakeVector4(sizeBlock.X(), sizeBlock.Y(), sizeBlock.Z(), 0.f);
							Maths::Vector4 centreCell4 = Maths::MakeVector4(0.f, 0.f, 0.f, 1.f) * mat44;
							Maths::Vector4 sizeCell4 = Maths::MakeVector4(1.f, 1.f, 1.f, 0.f) * mat44;
							Maths::Vector4 separation4 = Maths::Abs(centreBlock4 - centreCell4) - sizeBlock4 - sizeCell4;
							
							Maths::VF32 distanceToCell = Maths::Max(Maths::Max(X(separation4), Y(separation4)), Maths::Max(Z(separation4), Maths::MakeVF32(0.f)));
							Maths::VF32 minDrawDistance = Maths::MakeVF32(resourceBlock.GetSubBounds(ib).distanceMin);
							Maths::VF32 closest = Maths::Max(distanceToCell, minDrawDistance);

							float fd;
							Maths::Store(closest, &fd);
							fmin = Maths::Min(fmin, fd);
						}
						
						resourceBlock.UpdateLodPvs(cellIndex, fmin);
					}
				}

				for (int lod = 0; lod < RenderingData::LodCalculator::GetNumLods(); lod++)
				{
					element.pvsBits = resourceBlock.GetPvsBits(lod);
					element.roomBits = resourceBlock.GetRoomBits();
					element.sizeInBytes = resourceBlock.GetSize(lod, platformType);

					if (element.pvsBits.Empty() || element.pvsBits.AnyBitsSet())
					{
						//if (lod > 0) element.hash = 0;
						element.lod = lod;
						element.nElements = resourceBlock.GetNumIndexBuffers() +
											resourceBlock.GetNumVertexBuffers() +
											resourceBlock.GetNumTextures();
						element.bounds.distanceMax = RenderingData::LodCalculator::LodDrawDistance(lod, 1280.f, 1.f);
					
						pStreamData->Add(element);
					}
					else
					{
						totalSkipped += element.sizeInBytes;
					}
				}
			}
		}

		LOGINFO(Tools::g_log, "Total bytes skipped due to PVS: %i", totalSkipped);

		pStreamData->End();

		//if (i == 9)
		{
			for (int block = blockOffset; block < scene.GetNumResourceBlocks(); block++)
			{
				if (!scene.GetResourceBlock(block).Empty()
					&& scene.GetResourceBlock(block).GetNumNodes() == 0)
				{
					RenderDataConversion::Clustering::StreamElement element;
					element.blockIndex = block;
					
					for (int lod = 0; lod < RenderingData::LodCalculator::GetNumLods(); lod++)
					{
						element.lod = lod;
						RenderDataConversion::Clustering::StreamCluster cluster = pStreamData->Find(element);

						if (cluster.chunkIndex > -1)
							streamData.AddBlockToChunk(cluster.chunkIndex, block, lod);
					}
				}
			}
		}

		RenderDataConversion::Clustering::Destroy(pStreamData);
	}
}

// Write out the rendering data for a particular platform (call Process first).
// Read by WorldRender::ReadScene().
bool C_Converter::ExportBinary(Sys::I_File* pSceneFile, Sys::I_File* pStreamFile, RenderingData::PlatformType platformType)
{
	// Copy and process the scene for the output platform.
	RenderingData::Build::Scene scene(*m_pSceneExport);

	scene.CreateGlobalDynamicMaterialList(2);

	// Use one render list per pass if there is a stream file (level export).
	scene.Finish(platformType);

	// Write the stream data.
	if (pStreamFile)
	{
		BuildStreamData(scene, platformType);
		scene.GetStreamData().WriteChunkData(pStreamFile, platformType);
	}

	// Create archive
	bool endianSwap = RenderingData::GetEndianSwap(platformType);
	Arch::BinaryOutputArchive ar(*pSceneFile, endianSwap);
	const static int VERSION = 1;
	ar << VERSION;

	// Write scene.
	RenderingData::Build::SceneWriter sceneWriter(scene, platformType);
	ar << sceneWriter;

	// If there's no stream file, write all the blocks after the scene data.
	// Otherwise write all the blocks that came before the final Process() call.
	S32 nBlocks = (pStreamFile == 0) ? scene.GetNumResourceBlocks() : m_iBlockIndexOffset;
	ar << nBlocks;

	for (int i = 0; i < nBlocks; i++)
	{
		RenderingData::Build::ResourceBlockWriter blockWriter(scene.GetResourceBlock(i), platformType);
		ar << blockWriter;
	}

	return true;
}

bool C_Converter::ExportXml( Sys::I_File* pBinaryStream, RenderingData::PlatformType platformType)
{
	LOGINFO(g_Log, "C_Converter::ExportXml\n");

	RenderingData::Build::Scene scene(*m_pSceneExport);
	scene.Finish(platformType);

	Arch::XmlOutputArchive ar(*pBinaryStream);
	RenderingData::Build::SceneWriter sceneWriter(scene, platformType);
	ar << sceneWriter;

	for (int i = 0; i < scene.GetNumResourceBlocks(); i++)
	{
		RenderingData::Build::ResourceBlockWriter blockWriter(scene.GetResourceBlock(i), platformType);
		ar << blockWriter;
	}

	return true;
}

void C_Converter::SetLodGenerationOptions(S32 lodsToGenerate, float reductionFactor, S32 minVertices, bool reduceVertices)
{
	m_pSceneExport->SetLodGenerationOptions(lodsToGenerate, reductionFactor, minVertices, reduceVertices);
}

void C_Converter::SetPVSData( const Pvs::Build::PvsData* pvsData )
{
	m_pvsData = pvsData;
}

void C_Converter::SetPortalData(Portal::C_Builder *pPortalData)
{
	m_pPortalBuilder = pPortalData;
	return;
}
}
