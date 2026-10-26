#include "stdafx.h"

#define WIN32_LEAN_AND_MEAN
#include <core/core.h>
#include <core/maths.h>
#include "Aftk/Aftk.h"
#include "AftkUt/AftkUt.h"
#include <tools_debug/DebugManager.h>

#include "renderingdata/renderingdata.h"

#include "renderingdata/archive.h"
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
#include "converter_shared/renderdataconversion/clustering.h"

#include <converter_shared\CustomSourceSemantics\DIFF_IND0_NORM_BUMP.h>

#pragma optimize("", off)

// register the default source semantics
void RegisterPlugins()
{
	static bool bRegistered = false;
	if (!bRegistered)
	{
		DIFF_IND0_NORM_BUMP::Register();
		bRegistered = true;
	}
}

namespace RenderDataConversion
{
Util::C_String g_szDefaultFXName = "default.fx";
Util::C_String g_szGlobalEffectVariation = "";
}

namespace RenderingData
{

static Debug::Logger &g_Log = Debug::GetLogger("converter_shared");
	
/// constructor
C_Converter::C_Converter(const Util::C_String sTextureCachePath, const Util::C_String sFXBPath)
{
	m_pSceneExport = new RenderingData::Build::Scene;
	m_pTextureManager = new TextureExport::TextureManager(sTextureCachePath, 3);
	m_pMtlConverter = new RenderDataConversion::MaterialManager(m_pSceneExport, m_pTextureManager, sFXBPath);
	m_pBatchIDGenerator = new BatchIDGenerator;
	m_pPolyInform = NULL;
	m_pClusteringScene = 0;//RenderDataConversion::Clustering::CreateScene();
	m_pMtlConverter->CreateStandardMaterials();
	m_pSceneExport->SetName("TestScene");
//	m_iCurrentModel = 0;
	//m_iCurrentInstanceId = 0;

	RegisterPlugins();
}

/// destructor
C_Converter::~C_Converter()
{
	delete m_pSceneExport;
	delete m_pTextureManager;
	delete m_pMtlConverter;
	delete m_pBatchIDGenerator;
	//RenderDataConversion::Clustering::DestroyScene(m_pClusteringScene);
}

/// set the name of the effect to use when no specific effect was specified (for example in the HLSL shader)
void C_Converter::SetDefaultEffect(const char *psz)
{
	RenderDataConversion::g_szDefaultFXName = psz;	
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

/// create a new cullnode and add it into the root node
RenderingData::ResourceIndex C_Converter::AddCullNode(const Btl::String &name, RenderingData::Node& parent, S32 blockIndex)
{
	Btl::AutoPointer<RenderingData::CullNode> pNode;
	Allocate(pNode);
	pNode->SetName(name);

	S32 nodeIndex = m_pSceneExport->InsertResourceBlock(blockIndex).AddNode(pNode);
	RenderingData::ResourceIndex nodeResourceIndex = RenderingData::ResourceIndex::BlockIndex(blockIndex, nodeIndex);

	parent.AddSubNode(nodeResourceIndex);

	return nodeResourceIndex;
}

/// begin the building of a new model
void C_Converter::BeginModel()
{
	m_ModelData.elementIndex = 0;

	Btl::String s;
	s.Printf("Model%d", m_ModelData.modelIndex);

	m_iCurrentNode = AddCullNode(s, m_pSceneExport->GetRootNode(), 0);
}

/// add an element to the model that is currently being built
void C_Converter::AddElement( const Maths::HMat43 &worldMtx, const C_AftkSceneNodeList& aftkSceneNodeList)
{
	m_ModelData.elementMaterialIndex = GenerateElementMaterial(m_ModelData.elementIndex);
	m_ModelData.elementMatrix = worldMtx;

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

	m_iCurrentNode = AddCullNode("Static", m_pSceneExport->GetRootNode(), 0);

	Process(m_AftkSceneNodes);
}

/// process a list of aftk nodes and add them to the renderdata scene
void C_Converter::Process(const C_AftkSceneNodeList& aftkSceneNodes)
{
	m_pClusteringScene = RenderDataConversion::Clustering::CreateScene();

	m_pClusteringScene->Begin();

	// process mesh nodes
	for(S32 i = 0; i < aftkSceneNodes.Size(); ++i)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = aftkSceneNodes[i];

		if(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMesh)
		{
			Process1((const Aftk::C_SceneMeshNode*)pAftkSceneNode);
		}
	}

	// process instances
	for(S32 i = 0; i < aftkSceneNodes.Size(); ++i)
	{
		const Aftk::C_SceneNode* pAftkSceneNode = aftkSceneNodes[i];

		if(pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneInstance ||
		   pAftkSceneNode->GetType() == Aftk::E_ObjType_SceneMeshRemappedInstance)
		{
			if (m_ModelData.elementIndex == -1)
				Process1(pAftkSceneNode);
			else
				LOGWARN( g_Log, "Attempt to export an instance node as part of a model. This is currently not supported.\n");
		}
	}

	m_pClusteringScene->End();
	CreateModelNodes();
	
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
			if (m_ModelData.elementIndex == -1)
				Process2(pAftkSceneNode);
			else
				LOGWARN( g_Log, "Attempt to export an instance node as part of a model. This is currently not supported.\n");
		}
	}

	m_modelPointers.Clear();
	m_instancePointers.Clear();
	RenderDataConversion::Clustering::DestroyScene(m_pClusteringScene);
}

S32 GetLod(const Aftk::C_SceneNode& node)
{
	const Aftk::C_SceneNode* pParent = node.GetParentPtr();

	S32 lod = -1;

	if (pParent && pParent->GetType() == Aftk::E_ObjType_SceneLodGroup)
	{
		const Aftk::C_SceneLodGroupNode* pLodGroup = (Aftk::C_SceneLodGroupNode*) pParent;

		for (U32 i=0; i < pLodGroup->GetNumLodLevels(); i++)
		{
			if (pLodGroup->GetLodLevelInfo(i).GetSceneNodePtr() == &node)
			{
				lod = i;
				break;
			}
		}

		//if (lod == -1)
		//	LOGERROR( g_Log, "GetLod: Lod not found in lod node.");
	}

	return lod;
}

S32 C_Converter::InsertBaseModel(const Aftk::C_SceneNode* pNode)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	Btl::Map<const Aftk::C_SceneNode*, S32>::InsertionReturnType r = m_baseModels.Insert(pNode);
	if (r.inserted)
		r.iterator->GetValue() = sceneExport.AddBaseModel();

	return r.iterator->GetValue();
}

/// perform clustering on the Aftk mesh
void C_Converter::Process1(const Aftk::C_SceneMeshNode* pNode)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	if (GetLod(*pNode) > 0)
		return;

	bool bBaseInstance = pNode->GetNumInstancesOfThisNode() > 0;
	const U32 numPolys = pNode->GetNumPolygons(0);
	S32 baseModelIndex = -1;
	
	if (bBaseInstance)
		baseModelIndex = InsertBaseModel(pNode);

	for (U32 i = 0; i < numPolys; i++)
	{
		const Aftk::C_Polygon& poly = pNode->GetPolygon(0, i);

		// if this polygon has the 'invisible' polyflag on then skip it
		if (m_pPolyInform && m_pPolyInform->IsVisible(&poly))
			continue;

		RenderDataConversion::Clustering::Polygon clusteringPolygon;
		GetClusteringData(*pNode, i, baseModelIndex, &clusteringPolygon);

		m_pClusteringScene->Add(clusteringPolygon);
	}

	if (bBaseInstance)
	{
		BaseInstanceData baseInstanceData;

		// insert model into instance map
		baseInstanceData.iBaseModelIdx = baseModelIndex;
		baseInstanceData.pMesh = pNode;
		m_BaseInstanceMap.Insert(pNode, baseInstanceData);

		RenderDataConversion::Clustering::Instance clusteringInstance;
		GetClusteringData(*pNode, baseInstanceData, &clusteringInstance);
		m_pClusteringScene->Add(clusteringInstance);
	}
}

void C_Converter::Process1(const Aftk::C_SceneNode* pAftkInstance)
{
	const Aftk::C_SceneNode* pSource;
	if (pAftkInstance->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		pSource = ((Aftk::C_SceneMeshRemappedInstanceNode*)pAftkInstance)->GetSourceNodePtr();
	else if (pAftkInstance->HasType(Aftk::E_ObjType_SceneInstance))
		pSource = ((Aftk::C_SceneInstanceNode*)pAftkInstance)->GetSourceNodePtr();
	else
		assert(0);

	BaseInstanceMap::Iterator find = m_BaseInstanceMap.Find((const Aftk::C_SceneMeshNode*)pSource);

	if (find != m_BaseInstanceMap.End())
	{
		RenderDataConversion::Clustering::Instance clusteringInstance;
		GetClusteringData(*pAftkInstance, find->GetValue(), &clusteringInstance);
		m_pClusteringScene->Add(clusteringInstance);
	}
	else
		int breakhere = 0; // base instance was not a mesh node
}

void C_Converter::GetClusteringData(const Aftk::C_SceneNode& node, const BaseInstanceData& baseInstanceData, RenderDataConversion::Clustering::Instance* clusteringInstance)
{
	Aftk::C_Bounds bounds = node.GetEvaluatedWorldBounds();

	clusteringInstance->centroid = bounds.GetCentre();
	clusteringInstance->bounds.aabbMin = bounds.GetMin();
	clusteringInstance->bounds.aabbMax = bounds.GetMax();
	clusteringInstance->baseModelIndex = baseInstanceData.iBaseModelIdx;
	clusteringInstance->lightmapIndex = -1;
	clusteringInstance->groupIndex = -1;
	clusteringInstance->pvsIndex = -1;

	// get instance material data from Aftk node
	RenderDataConversion::Material convertedMtl;
	RenderDataConversion::AftkMaterialConverter converter(m_pMtlConverter, &node);
	converter.Convert(&convertedMtl);
	assert(convertedMtl.iMaterialID > -1);

	// if this instance should have instance-specific vertex data...
	if (convertedMtl.pEffect->GetVertexFormat(RenderDataConversion::E_NodeType_Instance)->GetDataSize() > 0)
		clusteringInstance->nPolygons = baseInstanceData.pMesh->GetNumPolygons(0);
	else
		clusteringInstance->nPolygons = 0;
}

void C_Converter::GetClusteringData(const Aftk::C_SceneMeshNode& node, S32 polygonIndex, S32 baseModelIndex, RenderDataConversion::Clustering::Polygon* clusteringPolygon)
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
	GetMaterial(aftkPolygon, nodeType, &convertedMtl);

	clusteringPolygon->normal = normal;
	clusteringPolygon->bounds.SetAABBTriangle(position0, position1, position2);
	clusteringPolygon->centroid = Lerp(clusteringPolygon->bounds.aabbMax, clusteringPolygon->bounds.aabbMin, 0.5f);
	clusteringPolygon->materialIndex = convertedMtl.iMaterialID;
	clusteringPolygon->baseModelIndex = baseModelIndex;

	// to do
	clusteringPolygon->lightmapIndex = -1;
	clusteringPolygon->groupIndex = -1;
	clusteringPolygon->pvsIndex = -1;
}

void C_Converter::GetMaterial(const Aftk::C_Polygon& aftkPolygon, RenderDataConversion::E_NodeType nodetype, RenderDataConversion::Material* pConvertedMtl)
{
	const Aftk::C_MaterialResource *pMtlRes = AftkHelpers::GetMaterialResourcePtr(&aftkPolygon);
	if (pMtlRes)
		m_pMtlConverter->GetConvertedAftkMaterial(aftkPolygon.GetMeshNodePtr(), &pMtlRes->GetContent(), pConvertedMtl, nodetype);
	else
		m_pMtlConverter->GetConvertedAftkMaterial(aftkPolygon.GetMeshNodePtr(), NULL, pConvertedMtl, nodetype);
}


/// returns a material index for the material containing element data for the nth element
int C_Converter::GenerateElementMaterial(int iElement)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	RenderingData::MaterialType materialType;
	materialType.SetName("Element");
	materialType.SetProcessType(RenderingData::MaterialType::MATERIAL_PROCESS_TYPE_DYNAMIC);
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

void C_Converter::CreateClusterNode(RenderingData::ResourceIndex parent, const RenderDataConversion::Clustering::Node& node)
{
	// Create model nodes.
	for (int i = 0; i < node.models.Size(); i++)
	{
		if (node.models[i].clusterType == RenderDataConversion::Clustering::Cluster::CLUSTER_TYPE_MODEL)
		{
			// Create a node and add it to the relevant subnode
			RenderingData::ResourceIndex nodeResourceIndex = m_pSceneExport->AddRenderNode(node.blockIndex);
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
				pModel->SetMaterialIndex(m_ModelData.elementMaterialIndex);
				
			m_modelPointers[node.models[i].itemIndex] = pModel;
		}
	}

	// Create instance nodes.
	for (int i = 0; i < node.instances.Size(); i++)
	{
		// Create a node and add it to the relevant subnode
		RenderingData::ResourceIndex nodeResourceIndex = m_pSceneExport->AddRenderNode(node.blockIndex);
		m_pSceneExport->GetNodePtr(parent)->AddSubNode(nodeResourceIndex);
		RenderingData::RenderNode* pRenderNode = RenderingData::RenderNode::GetPointer(m_pSceneExport->GetNodePtr(nodeResourceIndex));

		m_instancePointers[node.instances[i].itemIndex] = pRenderNode;
	}

	for (int i = 0; i < node.subNodes.Size(); i++)
	{
		RenderingData::ResourceIndex nodeResourceIndex = AddCullNode("", *m_pSceneExport->GetNodePtr(parent), node.subNodes[i]->blockIndex);

		CreateClusterNode(nodeResourceIndex, *node.subNodes[i]);
	}
}

void C_Converter::CreateModelNodes()
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	m_iBlockIndexOffset = sceneExport.GetNumResourceBlocks();

	const RenderDataConversion::Clustering::Node& node = m_pClusteringScene->GetRootNode();
	CreateClusterNode(m_iCurrentNode, node);
}

/// construct a RenderingData::PrimitiveInfo from a given Aftk polygon
bool C_Converter::GetPrimitiveInfo(const RenderDataConversion::AftkMeshConverter *pMesh, int iPoly, RenderDataConversion::E_NodeType nodeType, RenderingData::PrimitiveInfo *pPrimitiveInfo, RenderDataConversion::Material *pConvertedMtl)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;
	const Aftk::C_SceneMeshNode *pNode = pMesh->GetMeshNode();
	const Aftk::C_Polygon& poly = pNode->GetPolygon(0, iPoly);

	// if this polygon has the 'invisible' polyflag on then skip it
	if (m_pPolyInform && m_pPolyInform->IsVisible(&poly))
		return false;

	// Create a primitive
	pPrimitiveInfo->primitiveType = RenderingData::PrimitiveInfo::PRIMITIVE_TYPE_TRIANGLE;

	// get the material for this polygon
	GetMaterial(poly, nodeType, pConvertedMtl);

	// get vertex data for this polygon
	pMesh->GetVertexData(pConvertedMtl->pEffect, iPoly, pPrimitiveInfo, nodeType);

	//
	RenderingData::PrimitiveInfo::StreamInfo& stream = pPrimitiveInfo->streams.Back();
	
	// set material to primitive
	stream.materialIndex = pConvertedMtl->iMaterialID;
	assert(pConvertedMtl->iMaterialID > -1);
	pPrimitiveInfo->base.sortType = pConvertedMtl->sortType;
	pPrimitiveInfo->base.blendState = pConvertedMtl->blendState;

	S32 materialType = sceneExport.GetMaterialList().GetMaterial(stream.materialIndex).GetTypeIndex();
	S32 effect = sceneExport.GetMaterialList().GetMaterialType(materialType).GetEffect();

	pPrimitiveInfo->base.effect = effect;
	return true;
}

void C_Converter::Process2(const Aftk::C_SceneMeshNode* pNode)
{
	RenderingData::Build::Scene& sceneExport = *m_pSceneExport;

	std::string name = pNode->GetName();

	if (GetLod(*pNode) > 0)
		return;

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
	
	// set mesh converter matrices depending on whether we are processing an element or not
	if(m_ModelData.elementIndex != -1)
		mesh.SetMatrices(Maths::HMat44(m_ModelData.elementMatrix));
	else
		mesh.SetMatrices();

	RenderDataConversion::Material convertedMtl;

	// add polygons into rendernode
	for (U32 poly = 0; poly < numPolys; poly++)
	{
		// get mesh data for this polygon
		RenderingData::PrimitiveInfo primitiveInfo;
		RenderDataConversion::E_NodeType nodeType = bBaseInstance?RenderDataConversion::E_NodeType_Base:RenderDataConversion::E_NodeType_Mesh;
		if (GetPrimitiveInfo(&mesh, poly, nodeType, &primitiveInfo, &convertedMtl))
		{
			// if this is a base instance, check that the data that goes on the instance is consistent across the effects on the mesh
			if (bBaseInstance)
			{
				int x = convertedMtl.pEffect->GetMaterialTypeIndex(RenderDataConversion::E_NodeType_Instance);
				if (poly == 0)
				{
					iInstanceMtlTypeIdx = x;
					sInstanceEffectName = convertedMtl.pEffect->GetName();
					pBaseInstanceData->GetValue().pEffect = convertedMtl.pEffect;
				}
				else
				{
					if (iInstanceMtlTypeIdx != x)
					{
						LOGERROR( g_Log, "Base instance materials don't have matching instance data: %s and %s\n",
							convertedMtl.pEffect->GetName().Chars(),
							sInstanceEffectName.Chars());
					}

					assert(iInstanceMtlTypeIdx == x);
				}
			}

			// set block/model
			RenderingData::PrimitiveInfo::StreamInfo& stream = primitiveInfo.streams.Back();

			RenderDataConversion::Clustering::Polygon clusteringPolygon;
			GetClusteringData(*pNode, poly, baseModelIndex, &clusteringPolygon);
			RenderDataConversion::Clustering::Cluster cluster = m_pClusteringScene->FindCluster(clusteringPolygon);

			if (cluster.itemIndex == 1247)
				int breakhere = 0;

			int iBlock = cluster.blockIndex + m_iBlockIndexOffset;
			stream.resourceBlockIndex = iBlock;

			primitiveInfo.base.resourceBlockIndex = iBlock;

			primitiveInfo.GenerateSortCode();
			primitiveInfo.batchID = m_pBatchIDGenerator->GetBatchID(primitiveInfo, baseModelIndex);
			
			// remember the batch ID for use when building the mesh for the instance
			if (bBaseInstance)
				pBaseInstanceData->GetValue().m_iBatchIDs.PushBack(primitiveInfo.batchID);

			RenderingData::Build::Model* pModel;
			
			if (bBaseInstance)
				pModel = &sceneExport.GetBaseModel(baseModelIndex);
			else
				pModel = m_modelPointers.Find(cluster.itemIndex)->GetValue();

			sceneExport.AddPrimitive(primitiveInfo, pModel);

			// Update texture scales.
			Btl::Vector<S32> textureIndices;
			sceneExport.GetTextureIndices(primitiveInfo, textureIndices);

			for (int i = 0; i < textureIndices.Size(); i++)
			{
				RenderingData::ResourceIndex r = sceneExport.GetTexture(textureIndices[i]);
				const RenderingData::Build::Texture& texture = sceneExport.GetTexture(r);
				RenderDataConversion::AftkMeshConverter::TextureScale textureScale;
				mesh.CalculateTextureScale(pNode->GetPolygon(poly), 0, textureScale);
				texture.UpdateScale(textureScale.uScale, textureScale.vScale, textureScale.weight);
			}
		}
		else
		{
			if (bBaseInstance)
				pBaseInstanceData->GetValue().m_iBatchIDs.PushBack(-1);
		}
	}

	if (bBaseInstance)
	{
		// add an instance at the base model's location
		CreateInstance(pNode, pBaseInstanceData->GetValue());
	}
}

/// this is called once to build the rendering data
void C_Converter::Process2(const Aftk::C_SceneNode* pAftkInstance)
{
	const Aftk::C_SceneNode* pSource;
	if (pAftkInstance->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
		pSource = ((Aftk::C_SceneMeshRemappedInstanceNode*)pAftkInstance)->GetSourceNodePtr();
	else if (pAftkInstance->HasType(Aftk::E_ObjType_SceneInstance))
		pSource = ((Aftk::C_SceneInstanceNode*)pAftkInstance)->GetSourceNodePtr();
	else
		assert(0);

	if(pSource->GetType() == Aftk::E_ObjType_SceneMesh)
	{
		BaseInstanceMap::Iterator find = m_BaseInstanceMap.Find((const Aftk::C_SceneMeshNode*)pSource);

		if(find != m_BaseInstanceMap.End())
		{
			CreateInstance(pAftkInstance, find->GetValue());
		}
	}
}

void C_Converter::CreateInstance(const Aftk::C_SceneNode *pNode, BaseInstanceData &data)
{
	RenderDataConversion::Clustering::Instance clusteringInstance;
	GetClusteringData(*pNode, data, &clusteringInstance);
	RenderDataConversion::Clustering::Cluster cluster = m_pClusteringScene->FindCluster(clusteringInstance);

	// found instance in map, add render node
	//RenderingData::ResourceIndex nodeResourceIndex = m_pSceneExport->AddRenderNode();
	//m_pSceneExport->GetNodePtr(m_iCurrentNode)->AddSubNode(nodeResourceIndex);
	
	RenderingData::RenderNode* pInstanceRenderNode = m_instancePointers.Find(cluster.itemIndex)->GetValue();
	
	assert(pInstanceRenderNode);
	//assert(pInstanceRenderNode->GetRenderObjectPtr() == 0);

	if (pInstanceRenderNode->GetRenderObjectPtr() == 0) // = RenderingData::RenderNode::GetPointer(m_pSceneExport->GetNodePtr(nodeResourceIndex)))
	{
		std::string instanceName = pNode->GetName();
		pInstanceRenderNode->SetName(instanceName.c_str());

		// store bounds on RenderNode
		{
			RenderingData::Bounds bounds;
			Aftk::C_Bounds aftkBounds = pNode->GetWorldBounds();

			bounds.SetUniversal();
			bounds.aabbMin = aftkBounds.GetMin();
			bounds.aabbMax = aftkBounds.GetMax();

			RenderingData::Vector3 size = bounds.aabbMax - bounds.aabbMin;

			// Get the middle value.
			float v0 = Maths::Min(Maths::Min(size.X(), size.Y()), size.Z());
			float v1 = Maths::Max(Maths::Min(size.X(), size.Y()), size.Z());
			float v2 = Maths::Max(v1, Maths::Max(size.X(), size.Y()));
			float s = Maths::Min(v1, Maths::Max(size.X(), size.Y()));

			bounds.distanceMax = s * 300.f;

			pInstanceRenderNode->SetBounds(bounds);
		}

		Btl::AutoPointer<RenderingData::Build::Instance> pInstance;
		Allocate(pInstance);
		pInstance->SetBaseModelIndex(data.iBaseModelIdx);

		// get instance material data from Aftk node
		RenderDataConversion::Material convertedMtl;
		RenderDataConversion::AftkMaterialConverter converter(m_pMtlConverter, pNode);
		converter.Convert(&convertedMtl);
		assert(convertedMtl.iMaterialID > -1);
		pInstance->SetMaterialIndex(convertedMtl.iMaterialID);

		// some instances have mesh data on the instance itself e.g. vertex colours
		// here we collect this data
		const U32 numPolys = data.pMesh->GetNumPolygons(0);

		RenderDataConversion::AftkMeshConverter mesh(data.pMesh, pNode);

		// if this instance should have instance-specific vertex data...
		if (convertedMtl.pEffect->GetVertexFormat(RenderDataConversion::E_NodeType_Instance)->GetDataSize() > 0)
		{
			// add polygons to instance
			for (U32 i = 0; i < numPolys; i++)
			{
				if (data.m_iBatchIDs[i] > -1)
				{
					RenderingData::PrimitiveInfo primitiveInfo;

					// get vertex data for this polygon
					mesh.GetVertexData(convertedMtl.pEffect, i, &primitiveInfo, RenderDataConversion::E_NodeType_Instance);

					//
					RenderingData::PrimitiveInfo::StreamInfo& stream = primitiveInfo.streams.Back();
					
					// set material to primitive
					stream.materialIndex = convertedMtl.iMaterialID;
					stream.resourceBlockIndex = cluster.blockIndex;
					stream.instanceID = cluster.itemIndex;
					stream.streamIndex = 1;

					primitiveInfo.batchID = data.m_iBatchIDs[i];

					m_pSceneExport->AddPrimitive(primitiveInfo, &(*pInstance));
				}
			}
		}

		pInstanceRenderNode->SetRenderObject(pInstance);
	}
	//m_iCurrentInstanceId++;
}

/// add list of Aftk nodes that are to be built into a single static mesh
void C_Converter::AddStaticNodes(const C_AftkSceneNodeList& aftkSceneNodeList)
{
	for(S32 i = 0; i < aftkSceneNodeList.Size(); ++i)
	{
		m_AftkSceneNodes.PushBack(aftkSceneNodeList[i]);
	}
}

void C_Converter::BuildStreamData(RenderingData::Build::Scene& scene)
{
	RenderingData::Build::StreamData& streamData = scene.GetStreamData();

	S32 chunk = 0;

	for (int block = 0; block < scene.GetNumResourceBlocks(); block++)
		streamData.AddBlockToChunk(chunk++, block, 0);

	S32 lod1 = chunk;

	for (int block = 0; block < scene.GetNumResourceBlocks(); block++)
	{
		chunk = lod1;
		streamData.AddBlockToChunk(chunk++, block, 1);
		streamData.AddBlockToChunk(chunk++, block, 2);
		streamData.AddBlockToChunk(chunk++, block, 3);
		streamData.AddBlockToChunk(chunk++, block, 4);
		streamData.AddBlockToChunk(chunk++, block, 5);
		streamData.AddBlockToChunk(chunk++, block, 6);
		streamData.AddBlockToChunk(chunk++, block, 7);
		streamData.AddBlockToChunk(chunk++, block, 8);
		streamData.AddBlockToChunk(chunk++, block, 9);
	}
}

/// write out the rendering data for a particular platform (call Process first)
bool C_Converter::ExportBinary(Sys::I_File* pSceneFile, Sys::I_File* pStreamFile, RenderingData::PlatformType platformType)
{
	// Copy and process the scene for the output platform.
	RenderingData::Build::Scene scene(*m_pSceneExport);
	scene.Finish(platformType);

	// Write the stream data.
	if (pStreamFile)
	{
		BuildStreamData(scene);
		scene.GetStreamData().WriteChunkData(pStreamFile, platformType);
	}

	// Create archive
	bool endianSwap = RenderingData::GetEndianSwap(platformType);
	Arch::BinaryOutputArchive ar(*pSceneFile, endianSwap);
	const static int VERSION = 0;
	ar << VERSION;

	// Write scene.
	RenderingData::Build::SceneWriter sceneWriter(scene, platformType);
	ar << sceneWriter;

	// If there's no stream file, write the blocks after the scene data.
	bool storeBlocks = (pStreamFile == 0);
	ar << storeBlocks;

	if (storeBlocks)
	{
		for (int i = 0; i < scene.GetNumResourceBlocks(); i++)
		{
			RenderingData::Build::ResourceBlockWriter blockWriter(scene.GetResourceBlock(i), platformType);
			ar << blockWriter;
		}
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

 // --------------------------------------------------------------------------------

void C_Converter::TestProcess()
{
	TextureExport::TextureManager& textureManager(*m_pTextureManager);

	// Create the scene.
	RenderingData::Build::Scene& sceneExport(*m_pSceneExport);
	sceneExport.SetName("TestScene");

	// Create a node and add it to the scene.
	S32 resourceBlockIndex = sceneExport.AddResourceBlock();
	RenderingData::ResourceIndex nodeResourceIndex = sceneExport.AddRenderNode(0);
	sceneExport.GetRootNode().AddSubNode(nodeResourceIndex);
	RenderingData::RenderNode* pRenderNode = RenderingData::RenderNode::GetPointer(sceneExport.GetNodePtr(nodeResourceIndex));
	pRenderNode->SetName("TestModel");

	// Add a model to the node.
	if (pRenderNode)
	{
		Btl::AutoPointer<RenderingData::Build::Model> pModel;
		Allocate(pModel);
		pRenderNode->SetRenderObject(pModel);
	}

	BatchIDGenerator batchIDGenerator;

	//Add a primitive to the model.
	RenderingData::Build::Model* pModel = RenderingData::Build::Model::GetPointer(pRenderNode->GetRenderObjectPtr());
	if (pModel)
	{
		for (int iMtl=0; iMtl<2; iMtl++)
		{
			// Create an effect.
			RenderingData::Build::Effect effect;
			if (iMtl == 0)
				effect.SetName("Default.fx");
			else
				effect.SetName("Default.fx");

			S32 effectIndex = sceneExport.AddEffect(effect);

			// Create a material type.
			RenderingData::MaterialType materialType;
			materialType.SetName("TestMaterial");
			RenderingData::MaterialElement materialElement;
			materialElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
			materialElement.count = 4;
			materialElement.name = "TestPixelShaderConstant";
			materialType.AddElement(materialElement);
			materialElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_TEXTURE_INDEX;
			materialElement.count = 1;
			materialElement.name = "g_MeshTexture";
			materialType.AddElement(materialElement);
			materialType.SetEffect(effectIndex);
			S32 materialTypeIndex = sceneExport.GetMaterialList().InsertMaterialType(materialType);

			struct TestMaterial
			{
				RenderingData::DataTypes::F32_4 pixelShaderConstant;
				S32 textureIndex;
			};

			S32 materialIndex[4];

			// Create a material.
			for (int i = 0; i < 4; i++)
			{
				RenderingData::Material material;
				sceneExport.GetMaterialList().CreateMaterial(material, materialTypeIndex);
				TestMaterial testMaterial;
				testMaterial.pixelShaderConstant = RenderingData::DataTypes::F32_4(1.f, 2.f, 3.f, 4.f);
				RenderingData::Build::Texture t1;
				TextureExport::TextureInfo info;

				TextureExport::InputFile item;
				item.bRequired = true;
				switch (i)
				{
					case 0:item.sName = "c:\\temp\\test1.tga"; break;
					case 1:item.sName = "c:\\temp\\test2.tga"; break;
					case 2:item.sName = "c:\\temp\\test3.tga"; break;
					case 3:item.sName = "c:\\temp\\test4.tga"; break;
				}
				info.m_InputFilenames.PushBack(item);

				int textureCacheIndex = textureManager.GetTextureIndex(&info);
				t1.Initialise("Texture", textureCacheIndex, i, &textureManager);
				testMaterial.textureIndex = sceneExport.InsertSharedTexture(t1);
				material.SetData(&testMaterial, sizeof(TestMaterial));
				materialIndex[i] = sceneExport.GetMaterialList().InsertMaterial(material);
			}

			// Create a primitive
			RenderingData::PrimitiveInfo primitiveInfo;
			primitiveInfo.batchID = 0;
			primitiveInfo.primitiveType = RenderingData::PrimitiveInfo::PRIMITIVE_TYPE_TRIANGLE;
			primitiveInfo.base.sortType = RenderingData::SORT_TYPE_KEY;
			primitiveInfo.base.effect = effectIndex;
			primitiveInfo.base.sortCode = 2;
			primitiveInfo.base.resourceBlockIndex = resourceBlockIndex;

			primitiveInfo.streams.PushBack();
			RenderingData::PrimitiveInfo::StreamInfo& stream = primitiveInfo.streams.Back();
			stream.materialIndex = -1;
			stream.streamIndex = iMtl;
			stream.resourceBlockIndex = resourceBlockIndex;

			RenderingData::VertexFormat vertexFormat;
			RenderingData::VertexFormat::VertexElement vertexElement;
			vertexElement.type = RenderingData::VERTEX_ELEMENT_TYPE_F32_3;
			vertexElement.usage = RenderingData::VERTEX_ELEMENT_USAGE_POSITION;
			vertexElement.method = RenderingData::VERTEX_ELEMENT_METHOD_DEFAULT;
			vertexElement.usageIndex = 0;
			vertexFormat.AddVertexElement(vertexElement);
			vertexElement.type = RenderingData::VERTEX_ELEMENT_TYPE_F16_2;
			vertexElement.usage = RenderingData::VERTEX_ELEMENT_USAGE_TEXCOORD;
			vertexElement.method = RenderingData::VERTEX_ELEMENT_METHOD_DEFAULT;
			vertexElement.usageIndex = 0;
			vertexFormat.AddVertexElement(vertexElement);
			vertexElement.type = RenderingData::VERTEX_ELEMENT_TYPE_U8_4N;
			vertexElement.usage = RenderingData::VERTEX_ELEMENT_USAGE_COLOR;
			vertexElement.method = RenderingData::VERTEX_ELEMENT_METHOD_DEFAULT;
			vertexElement.usageIndex = 0;
			vertexFormat.AddVertexElement(vertexElement);
			stream.vertices.Create(vertexFormat);
			stream.vertices.Resize(3);

			struct Vertex
			{
				RenderingData::DataTypes::F32_3 position;
				RenderingData::DataTypes::F16_2 uv;
				RenderingData::DataTypes::U8_4N colour;
			};

			for (int i = 0; i < 10; i++)
			{
				float x = float(i % 100);
				float z = float(i / 100) + ((float)iMtl*20.f);

				Vertex vertex;
				vertex.position = RenderingData::DataTypes::F32_3(x, 0.f, z);
				vertex.uv = RenderingData::DataTypes::F16_2(0.f, 0.f);
				vertex.colour = 0xffff0000;
				stream.vertices[0].SetData(&vertex, sizeof(Vertex));

				vertex.position = RenderingData::DataTypes::F32_3(x, 1.f, z);
				vertex.uv = RenderingData::DataTypes::F16_2(0.f, 1.f);
				vertex.colour = 0xff00ff00;
				stream.vertices[1].SetData(&vertex, sizeof(Vertex));

				vertex.position = RenderingData::DataTypes::F32_3(x + 1.f, 0.f, z);
				vertex.uv = RenderingData::DataTypes::F16_2(1.f, 1.f);
				vertex.colour = 0xff0000ff;
				stream.vertices[2].SetData(&vertex, sizeof(Vertex));

				stream.materialIndex = materialIndex[i % 4];

				primitiveInfo.batchID = batchIDGenerator.GetBatchID(primitiveInfo, -1);
				primitiveInfo.CalculateBoundsFromVertices();

				sceneExport.AddPrimitive(primitiveInfo, pModel);
			}
		}
	}

	// add an instance
	{
		// Create a model
		sceneExport.AddBaseModel();
		RenderingData::Build::Model& baseModel = sceneExport.GetBaseModel(0);
		baseModel = *pModel; // copy from above

		S32 instancePositionType, materialIndexType;

		// Create a material type.
		{
			RenderingData::MaterialType materialType;
			materialType.SetName("InstancePosition");
			materialType.SetProcessType(RenderingData::MaterialType::MATERIAL_PROCESS_TYPE_DYNAMIC);
			RenderingData::MaterialElement materialElement;
			materialElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_F32;
			materialElement.count = 3;
			materialElement.name = "Position";
			materialType.AddElement(materialElement);
			instancePositionType = sceneExport.GetMaterialList().InsertMaterialType(materialType);
		}

		{
			RenderingData::MaterialType materialType;
			materialType.SetName("MaterialIndex");
			materialType.SetProcessType(RenderingData::MaterialType::MATERIAL_PROCESS_TYPE_CONSTANT);
			RenderingData::MaterialElement materialElement;
			materialElement.type = RenderingData::MATERIAL_ELEMENT_TYPE_MATERIAL_INDEX;
			materialElement.count = 1;
			materialElement.name = "Index";
			materialType.AddElement(materialElement);
			materialIndexType = sceneExport.GetMaterialList().InsertMaterialType(materialType);
		}

		for (int i = 0; i < 2; i++)
		{
			RenderingData::ResourceIndex nodeResourceIndex = sceneExport.AddRenderNode(0);
			sceneExport.GetRootNode().AddSubNode(nodeResourceIndex);
			RenderingData::RenderNode* pRenderNode = RenderingData::RenderNode::GetPointer(sceneExport.GetNodePtr(nodeResourceIndex));
			pRenderNode->SetName("TestInstance");

			// Add an instance to the node.
			if (pRenderNode)
			{
				Btl::AutoPointer<RenderingData::Build::Instance> pInstance;
				Allocate(pInstance);
				pInstance->SetBaseModelIndex(0);

				S32 positionIndex;

				// Create a material.
				{
					RenderingData::Material material;
					const RenderingData::MaterialType& materialType = sceneExport.GetMaterialList().GetMaterialType(instancePositionType);
					sceneExport.GetMaterialList().CreateMaterial(material, instancePositionType);
					material.SetData(materialType.GetOffset(0, 0), 0.f);
					material.SetData(materialType.GetOffset(0, 1), float(i + 1));
					material.SetData(materialType.GetOffset(0, 2), 0.f);
					positionIndex = sceneExport.GetMaterialList().InsertMaterial(material);
				}

				// Create a material.
				{
					RenderingData::Material material;
					const RenderingData::MaterialType& materialType = sceneExport.GetMaterialList().GetMaterialType(instancePositionType);
					sceneExport.GetMaterialList().CreateMaterial(material, materialIndexType);
					material.SetData(0, positionIndex);
					S32 materialIndex = sceneExport.GetMaterialList().InsertMaterial(material);
					pInstance->SetMaterialIndex(materialIndex);
				}

				pRenderNode->SetRenderObject(pInstance);
			}
		}
	}

	sceneExport.Finish(PLATFORM_TYPE_UNKNOWN);
}
	 
}
