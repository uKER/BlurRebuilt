#include "AftkRenderPch.h"
#include "common/TextureManager.h"
#include "common/TextureState.h"
#include "common/ShaderManager.h"
#include "common/Material.h"

#include "common/Scene.h"
#include "common/Renderer.h"
#include "common/SceneGraphNode.h"
#include "common/GeometryNode.h"

#include "mesh/MeshSceneBuilder.h"
#include "mesh/Ray.h"
#include "render/SceneRenderer.h"
#include "scene/AftkSceneBuilder.h"
#include "scene/MaterialBuilder.h"


namespace AftkRender
{
	extern Debug::Logger& g_Log;
}

using namespace AftkRender;

C_Scene::C_Scene()
: m_pRootSceneGraphNode()
{
	m_Flags.sceneBoundsDirty = true;
}

C_Scene::~C_Scene()
{
	m_GeometryNodes.Clear();

	DeleteSceneGraphFromNode(m_pRootSceneGraphNode);
}

bool C_Scene::Load(Aftk::C_Scene *pScene, bool useInternalMeshData)
{
	assert(pScene);
	
	Clear();

	DeleteSceneGraphFromNode(m_pRootSceneGraphNode);

	Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*> sceneNodeMap;
	if (useInternalMeshData)
	{
		if (!C_MeshSceneBuilder::BuildScene(pScene, this, m_GeometryNodes, sceneNodeMap))
			return false;
	}
	else
	{
		if (!C_AftkSceneBuilder::BuildScene(pScene, this, m_GeometryNodes, sceneNodeMap))
			return false;
	}

	//find root scene node pointer
	{
		Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator it = sceneNodeMap.Find(pScene->GetRootSceneNodePtr());
		assert (it != sceneNodeMap.End() && "Failed to get root scene graph node");

		m_pRootSceneGraphNode = it->GetValue();
	}

	MaterialBuilder::Build(pScene, m_Materials, m_MaterialMap);

	return true;
}

bool C_Scene::Load(Aftk::C_SceneNode *pSceneNode, bool useInternalMeshData)
{
	assert(pSceneNode);

	Clear();

	//TODO: bring in baseinstance nodes if the tree contains (indirect) instances

	Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*> sceneNodeMap;
	//this will not load the base instance if pSceneNode is an instance node...
	if (useInternalMeshData)
	{
		if (!C_MeshSceneBuilder::BuildNode(pSceneNode, this, m_GeometryNodes, sceneNodeMap))
			return false;
	}
	else
	{
		if (!C_AftkSceneBuilder::BuildNode(pSceneNode, this, m_GeometryNodes, sceneNodeMap))
			return false;
	}
	Ut::UtMap<Aftk::C_SceneNode*, C_SceneGraphNode*>::Iterator it = sceneNodeMap.Find(pSceneNode);
	assert (it != sceneNodeMap.End() && "Failed to get root scene graph node");

	m_pRootSceneGraphNode = it->GetValue();

	const Aftk::Matrix44& m = pSceneNode->GetWorldMatrix();
	Maths::HMat44 hm(
		Maths::HVec4(m[0][0], m[0][1], m[0][2], m[0][3]),
		Maths::HVec4(m[1][0], m[1][1], m[1][2], m[1][3]),
		Maths::HVec4(m[2][0], m[2][1], m[2][2], m[2][3]),
		Maths::HVec4(m[3][0], m[3][1], m[3][2], m[3][3]));

	m_pRootSceneGraphNode->SetWorldMatrix(hm);

	MaterialBuilder::Build(pSceneNode, m_Materials, m_MaterialMap);

	return true;
}


void C_Scene::Clear()
{
	m_GeometryNodes.Clear();

	m_Materials.Clear();
	m_MaterialMap.Clear();

	DeleteSceneGraphFromNode(m_pRootSceneGraphNode);
	m_pRootSceneGraphNode = 0;
}

bool C_Scene::BuildMeshClusters()
{
	for (S32 i=0; i<m_GeometryNodes.Size(); ++i)
	{
		if (!m_GeometryNodes[i]->Build())
			LOGWARN(g_Log, "Failed to build mesh...");
	}

	return true;
}

bool C_Scene::LoadTextures(S32 clampSize, C_TextureManager *pTextureManager)
{
	C_TextureManager& manager = (pTextureManager) ? *pTextureManager : C_Renderer::GetTextureManager();

	for (S32 i=0; i<m_Materials.Size(); ++i)
		m_Materials[i].LoadTextures(manager, clampSize);

	return true;
}

C_SceneGraphNode* C_Scene::GetRootSceneGraphNodePtr()
{
	return m_pRootSceneGraphNode;
}

const C_SceneGraphNode* C_Scene::GetRootSceneGraphNodePtr() const
{
	return m_pRootSceneGraphNode;
}

C_SceneGraphNode* C_Scene::GetSceneGraphNodeById(S32 id)
{
	Ut::UtMap<S32, C_SceneGraphNode*>::Iterator it = m_SceneGraphNodeIdMap.Find(id);
	if (it != m_SceneGraphNodeIdMap.End())
		return it->GetValue();

	return 0;
}

const C_SceneGraphNode* C_Scene::GetSceneGraphNodeById(S32 id) const
{
	Ut::UtMap<S32, C_SceneGraphNode*>::ConstIterator it = m_SceneGraphNodeIdMap.Find(id);
	if (it != m_SceneGraphNodeIdMap.End())
		return it->GetValue();

	return 0;
}


S16 C_Scene::GetMaterialIndexMapping(S16 index) const
{
	if (index<0 || index>=m_MaterialMap.Size())
		return -1;

	return m_MaterialMap[index];
}

C_Material* C_Scene::GetMaterial(S16 materialMappingIndex)
{
	if (materialMappingIndex<0 || materialMappingIndex>=m_Materials.Size())
		return 0;

	return &m_Materials[materialMappingIndex];
}

const C_Material* C_Scene::GetMaterial(S16 materialMappingIndex) const
{
	if (materialMappingIndex<0 || materialMappingIndex>=m_Materials.Size())
		return 0;

	return &m_Materials[materialMappingIndex];
}

S32 C_Scene::OnCreateSceneGraphNode(C_SceneGraphNode *pSgNode, S32 preferredId)
{
	if (!pSgNode)
		return -1;

	Ut::UtMap<S32, C_SceneGraphNode*>::ConstIterator it = m_SceneGraphNodeIdMap.Find(preferredId);
	if (it == m_SceneGraphNodeIdMap.End() && preferredId != -1)
	{
		m_SceneGraphNodeIdMap.Insert(Ut::UtMap<S32, C_SceneGraphNode*>::DataType(preferredId, pSgNode));
		return preferredId;
	}
	else
	{
		//need to enforce id uniqueness ... so find one not in use
		S32 id = m_SceneGraphNodeIdMap.Size();
		
		it = m_SceneGraphNodeIdMap.Find(id);
		while (it != m_SceneGraphNodeIdMap.End())
			it = m_SceneGraphNodeIdMap.Find(++id);

		m_SceneGraphNodeIdMap.Insert(Ut::UtMap<S32, C_SceneGraphNode*>::DataType(id, pSgNode));
		return id;
	}
}

void C_Scene::OnDestroySceneGraphNode(C_SceneGraphNode* pSgNode)
{
	if (!pSgNode)
		return;

	S32 id = pSgNode->GetId();
	if (id == -1)
	{
		LOGERROR(g_Log, "Scenegraph node %s has an invalid identifer -1", pSgNode->GetName().Chars());
		return;
	}
	m_SceneGraphNodeIdMap.Erase(id);
}

void C_Scene::DeleteSceneGraphFromNode(C_SceneGraphNode *pSgNode)
{
	if (!pSgNode)
		return;

	while (pSgNode->GetChildPtr())
	{
		C_SceneGraphNode *pChildNode = pSgNode->GetChildPtr();
		DeleteSceneGraphFromNode(pChildNode);
	}

	DetachSceneGraphFromNode(pSgNode);
	delete pSgNode;

	m_Flags.sceneBoundsDirty = true;
}

void C_Scene::DetachSceneGraphFromNode(C_SceneGraphNode *pSgNode)
{
	if (!pSgNode)
		return;

	C_SceneGraphNode *pParent = pSgNode->m_pParent;
	C_SceneGraphNode *pPrevious = pSgNode->m_pPrevious;
	C_SceneGraphNode *pNext = pSgNode->m_pNext;
	C_SceneGraphNode *pChild = pSgNode->m_pChild;

	if (pParent)
		pParent->DetachChild(pSgNode);
	else if (pSgNode == m_pRootSceneGraphNode)
		m_pRootSceneGraphNode = 0;

	m_Flags.sceneBoundsDirty = true;
}

void C_Scene::InvalidateCache() const
{
	m_Flags.sceneBoundsDirty = true;
}

const Maths::C_AxisBox& C_Scene::GetAABBox() const
{
	if (m_Flags.sceneBoundsDirty)
	{
		m_SceneBounds.SetInvalidExtents();

		for (
			Ut::UtMap<S32, C_SceneGraphNode*>::ConstIterator it = m_SceneGraphNodeIdMap.Begin();
			it != m_SceneGraphNodeIdMap.End(); ++it)
		{
			C_SceneGraphNode* p = it->GetValue();
			if (!p || p->GetFlags().hidden)
				continue;
			m_SceneBounds.UpdateExtents(p->GetWorldAABBox());
		}
		m_Flags.sceneBoundsDirty = false;
	}

	return m_SceneBounds;
}

void C_Scene::RayIntersectionTest(const C_Ray& ray, Ut::UtSet<C_SceneGraphNode*>& result) const
{
	Ut::UtMap<S32, C_SceneGraphNode*>::ConstIterator it = m_SceneGraphNodeIdMap.Begin();

	while (it != m_SceneGraphNodeIdMap.End())
	{
		if (result.Find(it->GetValue())==result.End() && it->GetValue()->IsA(E_SceneGraphNode_NativeMesh))
		{
			MeshIntersection r;
			IntersectMesh(ray, r, (C_MeshSceneGraphNode*)it->GetValue());

			if (r.polygon != -1)
			{
				result.Insert(it->GetValue());
				//LOGINFO(g_Log, "Hit %s", it->GetValue()->GetName().Chars());
			}
		}
		it++;
	}
}

void C_Scene::VisibilityTest(const RECT& region, AftkRender::C_SceneRenderer *pRenderer, Ut::UtSet<C_SceneGraphNode*> &result) const
{
	assert(pRenderer);

	Ut::UtMap<S32, C_SceneGraphNode*>::ConstIterator it = m_SceneGraphNodeIdMap.Begin();

	while (it != m_SceneGraphNodeIdMap.End())
	{
		if (result.Find(it->GetValue())==result.End() && it->GetValue()->IsA(E_SceneGraphNode_NativeMesh))
		{
			if (pRenderer->IsVisible(region, it->GetValue()))
			{
				result.Insert(it->GetValue());
				//LOGINFO(g_Log, "Hit %s", it->GetValue()->GetName().Chars());
			}
		}
		it++;
	}
}

void C_Scene::Hide(C_SceneGraphNode* pNode, U8 hideState) const
{
	pNode->m_Flags.hidden = hideState;
	
	for (C_SceneGraphNode* pChild = pNode->GetChildPtr(); pChild != 0; pChild = pChild->GetNextSiblingPtr())
	{
		Hide(pChild, hideState);
	}
}
