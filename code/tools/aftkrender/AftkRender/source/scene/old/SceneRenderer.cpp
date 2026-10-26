#include "stdafx.h"
#include "AftkRender.h"

#include "Shaders/vsConstants.h"
#include "Shaders/psConstants.h"


using namespace AftkRender;


class HashKeyOpMaterials
{
public:
	//Hashing function.
	static unsigned int GetHash(const AftkRender::C_RenderMaterial* pMaterial)
	{
		return pMaterial->GetHash();
	}

	static bool Equal(
		const AftkRender::C_RenderMaterial* material1,
		const AftkRender::C_RenderMaterial* material2)
	{
		if (material1->GetHash() == material2->GetHash())
			return true;

		return false;
	}
};


C_SceneRenderer::C_SceneRenderer()
:
	Display::I_Renderer("AftkScene"),
	m_pAftkScene(0), m_pRenderTaskFilter(0), m_pShaderDelegate(0)
{
	memset(&m_RenderSceneFlags, 0, sizeof(C_SceneRenderer::C_Flags));
	memset(&m_RenderShaderEffects[0], 0, sizeof(I_RenderShaderEffect*)*(U32)E_SceneRendererEffectType_Count);
	m_RenderSceneFlags.frustumCulling = true;
	m_RenderSceneConstants.vertexColourScale = 1.f;
}


C_SceneRenderer::C_SceneRenderer(Aftk::C_Scene* pAftkScene)
:
	Display::I_Renderer("AftkScene"),
	m_pAftkScene(pAftkScene), m_pRenderTaskFilter(0), m_pShaderDelegate(0)
{
	memset(&m_RenderSceneFlags, 0, sizeof(C_SceneRenderer::C_Flags));
	memset(&m_RenderShaderEffects[0], 0, sizeof(I_RenderShaderEffect*)*(U32)E_SceneRendererEffectType_Count);
	m_RenderSceneFlags.frustumCulling = true;
	m_RenderSceneConstants.vertexColourScale = 1.f;
	BuildFromScene(pAftkScene);
}

C_SceneRenderer::~C_SceneRenderer()
{
	Clear();
}

bool C_SceneRenderer::OnCreate()
{
	if (m_pAftkScene)
	{
		if (BuildFromScene(m_pAftkScene))
			return true;
		else
			return false;
	}
	return true;
}

Aftk::C_Status C_SceneRenderer::BuildFromScene(Aftk::C_Scene *pScene)
{
	Aftk::C_Status status;

	Clear();
	ASSERT(pScene);
	m_pAftkScene = pScene;

	C_RenderNodeBuilder nodeBuilder(this, &m_AftkToRenderNodeMap);
	status = Aftk::C_SceneNodeVisit::Traverse(pScene, nodeBuilder);
	if (!status)
	{
		Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*>::Iterator it;
		it = m_AftkToRenderNodeMap.Begin();
		while (it != m_AftkToRenderNodeMap.End())
		{
			delete it->GetValue();
			it++;
		}
		return status;
	}

	// Copy node pointers into our vector
	{
		Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*>::Iterator it;
		it = m_AftkToRenderNodeMap.Begin();
		while (it != m_AftkToRenderNodeMap.End())
		{
			m_RenderNodes.PushBack(it->GetValue());
			it++;
		}
	}

	// Hook up parent pointers 
	C_RenderNodeHierarchyBuilder hierarchyBuilder(&m_AftkToRenderNodeMap);
	status = Aftk::C_SceneNodeVisit::Traverse(pScene, hierarchyBuilder);
	if (!status)
	{
		ClearSceneRenderNodes();
		return status;
	}

	// Hook up source node pointers for instances
	C_RenderNodeInstanceBuilder instanceBuilder(&m_AftkToRenderNodeMap);
	status = Aftk::C_SceneNodeVisit::Traverse(pScene, instanceBuilder);
	if (!status)
	{
		ClearSceneRenderNodes();
		return status;
	}

	UpdateNodes();

	status = BuildMaterialList();
	if (!status)
		return status;

	status = LoadTextures();
	if (!status)
		return status;

	status = CreateDefaultShaders();
	if (!status)
		return status;

	GenerateRenderQueue();
	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneRenderer::BuildFromSceneNode(Aftk::C_SceneNode *pSceneNode)
{
	Aftk::C_Status status;

	Clear();
	ASSERT(pSceneNode);
	m_pAftkScene = pSceneNode->GetScenePtr();

	C_RenderNodeBuilder nodeBuilder(this, &m_AftkToRenderNodeMap);

	Aftk::C_SceneNodeIterator it(pSceneNode);

	do
	{
		Aftk::C_SceneNode *pNode = it;

		// Make sure sources of instances have been built
		if (pNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance) ||
			pNode->GetType() == Aftk::E_ObjType_SceneInstance)
		{
			Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*>::Iterator createdNodeIt = m_AftkToRenderNodeMap.End();

			if (pNode->HasType(Aftk::E_ObjType_SceneMeshRemappedInstance))
			{
				Aftk::C_SceneMeshRemappedInstanceNode *pInstanceNode = (Aftk::C_SceneMeshRemappedInstanceNode*) pNode;
				if (pInstanceNode->GetSourceNodePtr())
				{
					createdNodeIt = m_AftkToRenderNodeMap.Find(pInstanceNode->GetSourceNodePtr());
					if (createdNodeIt == m_AftkToRenderNodeMap.End())
						status = nodeBuilder.Visit(pInstanceNode->GetSourceNodePtr(), C_NullMemoryManager());
				}
			}
			else //if (pSceneNode->GetType() == Aftk::E_ObjType_SceneInstance())
			{
				Aftk::C_SceneInstanceNode *pInstanceNode = (Aftk::C_SceneInstanceNode*) pNode;
				if (pInstanceNode->GetSourceNodePtr())
				{
					createdNodeIt = m_AftkToRenderNodeMap.Find(pInstanceNode->GetSourceNodePtr());
					if (createdNodeIt == m_AftkToRenderNodeMap.End())
						status = nodeBuilder.Visit(pInstanceNode->GetSourceNodePtr(), C_NullMemoryManager());
				}
			}
		}

		if (!status)
		{
			ClearSceneRenderNodes();
			return status;
		}

		status = nodeBuilder.Visit(pNode, C_NullMemoryManager());
		if (!status)
		{
			ClearSceneRenderNodes();
			return status;
		}

		if (it == pSceneNode)
			it = pSceneNode->GetChildPtr();
		else
			it++;
	}
	while(it);


	// Copy node pointers into our vector
	{
		Ut::UtMap<const Aftk::C_SceneNode*, C_RenderNode*>::Iterator it;
		it = m_AftkToRenderNodeMap.Begin();
		while (it != m_AftkToRenderNodeMap.End())
		{
			m_RenderNodes.PushBack(it->GetValue());
			it++;
		}
	}

	// Hook up parent pointers in our scene subtree
	{
		C_RenderNodeHierarchyBuilder hierarchyBuilder(&m_AftkToRenderNodeMap);
		for (U32 iChild=0; iChild<pSceneNode->GetNumChildren(); ++iChild)
		{
			status = Aftk::C_SceneNodeVisit::Traverse(pSceneNode->GetChildPtr(iChild), hierarchyBuilder);
			if (!status)
			{
				ClearSceneRenderNodes();
				return status;
			}
		}
	}

	// Hook up source nodes of instances
	C_RenderNodeInstanceBuilder instanceBuilder(&m_AftkToRenderNodeMap);
	instanceBuilder.Visit(pSceneNode, C_NullMemoryManager());
	status = Aftk::C_SceneNodeVisit::Traverse(pSceneNode->GetChildPtr(), instanceBuilder);
	if (!status)
	{
		ClearSceneRenderNodes();
		return status;
	}

	UpdateNodes();

	status = BuildMaterialList();
	if (!status)
		return status;

	status = LoadTextures();
	if (!status)
		return status;

	status = CreateDefaultShaders();
	if (!status)
		return status;

	GenerateRenderQueue();
	return Aftk::C_Status::E_Status_Success;
	
}

bool C_SceneRenderer::OnCreateVolatile()
{
	return true;
}

void C_SceneRenderer::OnDestroyVolatile()
{
}

void C_SceneRenderer::OnDestroy()
{
	for (U32 iNode=0; iNode<(U32)m_RenderNodes.Size(); ++iNode)
	{
		C_RenderNode *pNode = m_RenderNodes[iNode];

		if (pNode->GetType() == E_RenderNodeType_Mesh)
		{
			C_RenderMeshNode *pMeshNode = (C_RenderMeshNode*)pNode;
			pMeshNode->Clear();
		}
	}
}

void C_SceneRenderer::Update(
	const Maths::C_TimeS& timeStep)
{
	GenerateRenderQueue();
}

void C_SceneRenderer::Render()
{
	GPU_EVENT(0xFFFFFFFF, "RenderScene");

	Display::Device()->SetColourWriteEnable(
		Display::E_ColourWriteEnable_Red |
		Display::E_ColourWriteEnable_Green |
		Display::E_ColourWriteEnable_Blue |
		Display::E_ColourWriteEnable_Alpha);

	Display::Device()->SetTextureFilterMag(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilterMin(0, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilterMip(0, Display::E_TextureFilter_None);
	Display::Device()->SetTextureAddressU(0, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureAddressV(0, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureAddressW(0, Display::E_TextureAddress_Wrap);

	Display::Device()->SetTextureFilterMag(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilterMin(1, Display::E_TextureFilter_Linear);
	Display::Device()->SetTextureFilterMip(1, Display::E_TextureFilter_None);
	Display::Device()->SetTextureAddressU(1, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureAddressV(1, Display::E_TextureAddress_Wrap);
	Display::Device()->SetTextureAddressW(1, Display::E_TextureAddress_Wrap);

	Display::Device()->SetCullMode(Display::E_CullMode_Ccw);

	E_SceneRendererEffectType currentEffect = E_SceneRendererEffectType_Count;
	I_RenderShaderEffect::SetGlobalConstants();

	for (U32 i=0; i<m_RenderQueue.GetSize(); ++i)
	{
		const C_RenderTask& task = m_RenderQueue.GetTask(i);
		I_RenderMeshNode *pMeshNode = task.GetRenderMeshNode();

		if (m_RenderSceneFlags.filterRenderTasks && !m_pRenderTaskFilter->IsOKToRender(task))
			continue;

		Display::C_Matrix worldMatrix;
		Maths::Store(pMeshNode->GetWorldMatrix(), &worldMatrix);

		Display::C_Matrix worldITMatrix;
		Maths::Store(pMeshNode->GetWorldInverseTransposeMatrix(), &worldITMatrix);

		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);
		Display::Device()->SetVertexShaderConstantF(CV_AFTKRENDER_WORLD_INV_TRANS_MATRIX, (F32*)&worldITMatrix, 4);
		Display::Device()->SetPixelShaderConstantF(CP_AFTKRENDER_VTX_LIGHTING_SCALE, &m_RenderSceneConstants.vertexColourScale, 1);

		// Use our default shaders
		if (!m_RenderSceneFlags.overrideshaders)
		{
			E_SceneRendererEffectType newEffect;
			if (task.GetRenderMeshClusterPtr()->GetClusterFlags().speedtree_leaf)
				newEffect = m_RenderSceneFlags.lightUsingVertexColours ? E_SceneRendererEffectType_SPTBillboard_LeafVertexLit : E_SceneRendererEffectType_SPTBillboard_Leaf;
			else
				newEffect = m_RenderSceneFlags.lightUsingVertexColours ? E_SceneRendererEffectType_DiffuseAlphaVertexLit : E_SceneRendererEffectType_DiffuseAlpha;

			if (currentEffect != newEffect)
			{
				if (task.GetRenderMeshClusterPtr()->GetClusterFlags().speedtree_leaf)
					Display::Device()->SetCullMode(Display::E_CullMode_None);
				else
					Display::Device()->SetCullMode(Display::E_CullMode_Ccw);

				currentEffect = newEffect;
				m_RenderShaderEffects[currentEffect]->OnEnable();
			}
		}
		else m_pShaderDelegate->Execute(task);

		pMeshNode->Render(task.GetLodLevel(), task.GetRenderMeshClusterIndex());		
	}

	// no backface culling for transparent objects
	Display::Device()->SetCullMode(Display::E_CullMode_None);

	Display::Device()->SetAlphaTestEnable(Display::E_Bool_True);
	Display::Device()->SetAlphaTestFunc(Display::E_CmpFunc_Greater);
	Display::Device()->SetAlphaTestRef(0x80);
	Display::Device()->SetBlendEnable(Display::E_Bool_False);
	Display::Device()->SetBlendSrc(Display::E_Blend_SrcAlpha);
	Display::Device()->SetBlendDst(Display::E_Blend_InvSrcAlpha);
	Display::Device()->SetBlendOp(Display::E_BlendOp_Add);

	//AA alpha testing
	if (Display::Device()->m_pd3dObject->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_X8R8G8B8, 0, D3DRTYPE_SURFACE, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A')) == S_OK)
		Display::Device()->m_pd3dDevice->SetRenderState(D3DRS_ADAPTIVETESS_Y, (D3DFORMAT)MAKEFOURCC('S', 'S', 'A', 'A'));

	for (U32 i=0; i<m_RenderQueue_Alpha.GetSize(); ++i)
	{
		const C_RenderTask& task = m_RenderQueue_Alpha.GetTask(i);
		I_RenderMeshNode *pMeshNode = task.GetRenderMeshNode();

		if (m_RenderSceneFlags.filterRenderTasks && !m_pRenderTaskFilter->IsOKToRender(task))
			continue;

		Display::C_Matrix worldMatrix;
		Maths::Store(pMeshNode->GetWorldMatrix(), &worldMatrix);

		Display::C_Matrix worldITMatrix;
		Maths::Store(pMeshNode->GetWorldInverseTransposeMatrix(), &worldITMatrix);

		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&worldMatrix, 4);
		Display::Device()->SetVertexShaderConstantF(CV_AFTKRENDER_WORLD_INV_TRANS_MATRIX, (F32*)&worldITMatrix, 4);

		// Use our default shaders
		if (!m_RenderSceneFlags.overrideshaders)
		{
			E_SceneRendererEffectType newEffect;
			if (task.GetRenderMeshClusterPtr()->GetClusterFlags().speedtree_leaf)
				newEffect = m_RenderSceneFlags.lightUsingVertexColours ? E_SceneRendererEffectType_SPTBillboard_LeafVertexLit : E_SceneRendererEffectType_SPTBillboard_Leaf;
			else
				newEffect = m_RenderSceneFlags.lightUsingVertexColours ? E_SceneRendererEffectType_DiffuseAlphaVertexLit : E_SceneRendererEffectType_DiffuseAlpha;

			if (currentEffect != newEffect)
			{
				if (task.GetRenderMeshClusterPtr()->GetClusterFlags().speedtree_leaf)
					Display::Device()->SetCullMode(Display::E_CullMode_None);
				else
					Display::Device()->SetCullMode(Display::E_CullMode_Ccw);

				currentEffect = newEffect;
				m_RenderShaderEffects[currentEffect]->OnEnable();
			}
		}
		else m_pShaderDelegate->Execute(task);

		pMeshNode->Render(task.GetLodLevel(), task.GetRenderMeshClusterIndex());
	}

	Display::Device()->m_pd3dDevice->SetRenderState(D3DRS_ADAPTIVETESS_Y, D3DFMT_UNKNOWN);
}

const Aftk::C_Scene* C_SceneRenderer::GetAftkScenePtr()
{
	return m_pAftkScene;
}

const Aftk::C_Scene* C_SceneRenderer::GetAftkScenePtr() const
{
	return m_pAftkScene;
}

void C_SceneRenderer::Clear()
{
	ClearSceneRenderNodes();
	ClearMaterialList();
	DestroyDefaultShaders();
}

void C_SceneRenderer::UpdateNodes()
{
	for (U32 iNode=0; iNode<(U32)m_RenderNodes.Size(); ++iNode)
	{
		C_RenderNode *pNode = m_RenderNodes[iNode];

		if (pNode->GetType() == E_RenderNodeType_Mesh)
		{
			C_RenderMeshNode *pMeshNode = (C_RenderMeshNode*)pNode;
			pMeshNode->Rebuild();
		}
	}
}


Aftk::C_Status C_SceneRenderer::BuildMaterialList()
{
	Aftk::C_Status status;
	Ut::UtVector< Ut::UtDynamicVector<bool> > materialsInUse;

	Ut::OpenHashSet<
		C_RenderMaterial*, 
		HashKeyOpMaterials > materials;

	if (m_pAftkScene->GetNumMaterialNodes() == 0)
		return Aftk::C_Status::E_Status_Success;

	materialsInUse.Resize(m_pAftkScene->GetNumMaterialNodes());
	memset(&materialsInUse[0], false, sizeof(bool)*m_pAftkScene->GetNumMaterialNodes());

	m_AftkMaterialMapping.Resize(m_pAftkScene->GetNumMaterialNodes());
	memset(&m_AftkMaterialMapping[0], 0, sizeof(C_RenderMaterial*)*m_pAftkScene->GetNumMaterialNodes());

	//Store a list of the aftk materials in use.
	for (U32 i = 0; i < (U32)m_RenderNodes.Size(); i++)
	{
		C_RenderNode* pRenderNode = m_RenderNodes[i];
		if (!pRenderNode->HasType(E_RenderNodeType_Mesh_Interface))
			continue;

		if (pRenderNode->GetAftkSceneNodePtr()->GetType() == Aftk::E_ObjType_SceneInstance)
			continue;

		Aftk::C_SceneMeshRemappableNode* pAftkMesh = 
			(Aftk::C_SceneMeshRemappableNode*)pRenderNode->GetAftkSceneNodePtr();

		for (U16 jMaterial = 0; jMaterial < pAftkMesh->GetNumPolygonMaterialMappings(); jMaterial++)
		{
			const Aftk::C_MaterialResource* pRes = pAftkMesh->GetPolygonMaterialMappingPtr(jMaterial);
			if (pRes)
				materialsInUse[pRes->GetIndex()] = true;
		}
	}

	// Build materials & check for duplicates
	for (U32 i = 0; i < (U32)m_AftkMaterialMapping.Size(); i++)
	{
		const Aftk::C_MaterialNode* pAftkMaterial = &m_pAftkScene->GetMaterialResourcePtr(i)->GetContent();
		if (!materialsInUse[i])
			continue;

		C_RenderMaterial* pMaterial = new C_RenderMaterial(this);
		if (!pMaterial)
			return Aftk::C_Status::E_Status_Failure;

		status = pMaterial->BuildFromAftkMaterial(pAftkMaterial);
		if (!status) return status;

		Ut::OpenHashSet<
			C_RenderMaterial*, 
			HashKeyOpMaterials >::ConstIterator hashIt = 
			materials.Find(pMaterial);

		if (hashIt == materials.End())
		{
			materials.Insert(pMaterial);
			m_RenderMaterials.PushBack(pMaterial);
		}
		else
		{
			delete pMaterial;
			pMaterial = hashIt->GetKey();
		}

		m_AftkMaterialMapping[i] = pMaterial;
	}

	return Aftk::C_Status::E_Status_Success;
}


const C_RenderMaterial* C_SceneRenderer::GetRenderMaterial(U32 aftkMaterialIndex)
{
	if (aftkMaterialIndex<(U32)m_AftkMaterialMapping.Size())
		return m_AftkMaterialMapping[aftkMaterialIndex];
	else
		return 0;
}

const C_RenderMaterial* C_SceneRenderer::GetRenderMaterial(U32 aftkMaterialIndex) const
{
	if (aftkMaterialIndex<(U32)m_AftkMaterialMapping.Size())
		return m_AftkMaterialMapping[aftkMaterialIndex];
	else
		return 0;
}

void C_SceneRenderer::GenerateRenderQueue()
{
	m_RenderQueue.Clear();
	m_RenderQueue_Alpha.Clear();

	if (m_RenderSceneFlags.texturesLoaded)
	{
		for (U32 iNode=0; iNode<(U32)m_RenderNodes.Size(); ++iNode)
		{
			C_RenderNode *pNode = m_RenderNodes[iNode];

			if (pNode->HasType(E_RenderNodeType_Mesh_Interface))
			{
				I_RenderMeshNode *pMeshNode = (I_RenderMeshNode*)pNode;
				S32 visible = Display::Device()->GetView(0)->GetViewFrustum().CullAxisBox(pMeshNode->GetWorldAABBox()).Visible();
				if (visible || !m_RenderSceneFlags.frustumCulling)
				{
					//TODO: Heuristic for selecting LODs
					for (U32 jCluster=0; jCluster<pMeshNode->GetNumClusters(0); ++jCluster)
					{
						if (!pMeshNode->GetMeshClusterPtr(0,jCluster))
							continue;

						const C_RenderMeshCluster *pCluster = pMeshNode->GetMeshClusterPtr(0,jCluster);
						S32 materialIndex = pCluster->GetAftkMaterialIndex();
						if (materialIndex<0 || !m_AftkMaterialMapping[materialIndex]->HasAlpha())
							m_RenderQueue.PushTask(C_RenderTask(0, pMeshNode, jCluster));
						else
							m_RenderQueue_Alpha.PushTask(C_RenderTask(0, pMeshNode, jCluster));
					}
				}
			}
		}
	}
	else
	{
		for (U32 iNode=0; iNode<(U32)m_RenderNodes.Size(); ++iNode)
		{
			C_RenderNode *pNode = m_RenderNodes[iNode];

			if (pNode->HasType(E_RenderNodeType_Mesh_Interface))
			{
				I_RenderMeshNode *pMeshNode = (I_RenderMeshNode*)pNode;
				S32 visible = Display::Device()->GetView(0)->GetViewFrustum().CullAxisBox(pMeshNode->GetWorldAABBox()).Visible();
				if (visible || !m_RenderSceneFlags.frustumCulling)
				{
					for (U32 jCluster=0; jCluster<pMeshNode->GetNumClusters(0); ++jCluster)
					{
						if (pMeshNode->GetMeshClusterPtr(0,jCluster))
							m_RenderQueue.PushTask(C_RenderTask(0, pMeshNode, jCluster));
					}
				}
			}
		}
	}
	m_RenderQueue.Sort();
	m_RenderQueue_Alpha.SortReverse();
}

C_RenderQueue& C_SceneRenderer::GetRenderQueue()
{
	return m_RenderQueue;
}

const C_RenderQueue& C_SceneRenderer::GetRenderQueue() const
{
	return m_RenderQueue;
}

C_RenderQueue& C_SceneRenderer::GetRenderQueueAlpha()
{
	return m_RenderQueue_Alpha;
}
const C_RenderQueue& C_SceneRenderer::GetRenderQueueAlpha() const
{
	return m_RenderQueue_Alpha;
}

const Display::C_Texture2D& C_SceneRenderer::GetDummyTexture() const
{
	return m_DummyTexture;
}


void C_SceneRenderer::RenderAllTasks()
{
	m_RenderSceneFlags.filterRenderTasks = false;
	m_pRenderTaskFilter = 0;
}

void C_SceneRenderer::SetRenderTaskFilter(I_SceneRendererTaskFilter *pTaskFilter)
{
	m_pRenderTaskFilter = pTaskFilter;
	m_RenderSceneFlags.filterRenderTasks = true;
}

void C_SceneRenderer::UseDefaultShaders()
{
	m_RenderSceneFlags.overrideshaders = false;
	m_pShaderDelegate = 0;
}

void C_SceneRenderer::OverrideDefaultShaders(I_SceneRendererShaderDelegate* pShaderDelegate)
{
	m_pShaderDelegate = pShaderDelegate;
	m_RenderSceneFlags.overrideshaders = true;
}

const C_SceneRenderer::C_Flags& C_SceneRenderer::GetRendererFlags() const
{
	return m_RenderSceneFlags;
}

C_SceneRenderer::C_Flags& C_SceneRenderer::GetRendererFlags()
{
	return m_RenderSceneFlags;
}

const C_SceneRenderer::C_Constants& C_SceneRenderer::GetRendererConstants() const
{
	return m_RenderSceneConstants;
}

C_SceneRenderer::C_Constants& C_SceneRenderer::GetRendererConstants()
{
	return m_RenderSceneConstants;
}


void C_SceneRenderer::ClearSceneRenderNodes()
{
	for (U32 iNode=0; iNode<(U32)m_RenderNodes.Size(); ++iNode)
	{
		delete m_RenderNodes[iNode];
	}
	m_RenderNodes.Clear();
	m_AftkToRenderNodeMap.Clear();
}

void C_SceneRenderer::ClearMaterialList()
{
	for (U32 iMaterial=0; iMaterial<(U32)m_RenderMaterials.Size(); ++iMaterial)
	{
		delete m_RenderMaterials[iMaterial];
	}
	m_RenderMaterials.Clear();
	m_AftkMaterialMapping.Clear();

	Display::Device()->Destroy(&m_DummyTexture);
	m_RenderSceneFlags.texturesLoaded = false;
}


Aftk::C_Status C_SceneRenderer::LoadTextures()
{
	Aftk::C_Status status;

	if (!m_DummyTexture.IsCreated() &&
		!Display::Device()->CreateTexture2D(1, 1, 1, Display::E_Usage_Managed, Display::E_Format_A8R8G8B8, &m_DummyTexture))
		LOGWARN(g_Log, "Failed to create dummy texture\n");
	else
	{
		Display::C_LockedRect rect = m_DummyTexture.Lock(0);
		if(rect.pBits)
		{
			rect.pBits[0] = 255;
			rect.pBits[1] = 0;
			rect.pBits[2] = 0;
			rect.pBits[3] = 255;
		}
		m_DummyTexture.Unlock(0);
	}
	for (U32 iMaterial=0; iMaterial<(U32)m_RenderMaterials.Size(); ++iMaterial)
	{
		m_RenderMaterials[iMaterial]->loadTextures2D();
	}
	m_RenderSceneFlags.texturesLoaded = true;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_SceneRenderer::CreateDefaultShaders()
{
	m_RenderShaderEffects[E_SceneRendererEffectType_DiffuseAlpha] = 
		new C_RenderShaderEffect_DiffuseTexturedAlpha();

	m_RenderShaderEffects[E_SceneRendererEffectType_SPTBillboard_Leaf] = 
		new C_RenderShaderEffect_SPTBillboardLeaf();

	m_RenderShaderEffects[E_SceneRendererEffectType_DiffuseAlphaVertexLit] = 
		new C_RenderShaderEffect_DiffuseTexturedAlphaVertexLit();

	m_RenderShaderEffects[E_SceneRendererEffectType_SPTBillboard_LeafVertexLit] = 
		new C_RenderShaderEffect_SPTBillboardLeafVertexLit();

	if (!m_RenderShaderEffects[E_SceneRendererEffectType_DiffuseAlpha] ||
		!m_RenderShaderEffects[E_SceneRendererEffectType_SPTBillboard_Leaf] ||
		!m_RenderShaderEffects[E_SceneRendererEffectType_DiffuseAlphaVertexLit] ||
		!m_RenderShaderEffects[E_SceneRendererEffectType_SPTBillboard_LeafVertexLit])
		return Aftk::C_Status::E_Status_OutOfMemory;

	m_RenderShaderEffects[E_SceneRendererEffectType_DiffuseAlpha]->OnCreate();
	m_RenderShaderEffects[E_SceneRendererEffectType_SPTBillboard_Leaf]->OnCreate();
	m_RenderShaderEffects[E_SceneRendererEffectType_DiffuseAlphaVertexLit]->OnCreate();
	m_RenderShaderEffects[E_SceneRendererEffectType_SPTBillboard_LeafVertexLit]->OnCreate();

	return Aftk::C_Status::E_Status_Success;
}

void C_SceneRenderer::DestroyDefaultShaders()
{
	for (U32 i=0; i<(U32)E_SceneRendererEffectType_Count; ++i)
		delete m_RenderShaderEffects[i];

	memset(&m_RenderShaderEffects[0], 0, sizeof(I_RenderShaderEffect*)*(U32)E_SceneRendererEffectType_Count);
}