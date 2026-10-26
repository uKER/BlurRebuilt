#include <stdafx.h>


using namespace Aftk;

#include <Aftk/AftkStringTable.h>

#ifdef WIN64
#pragma warning(disable:4996)
#endif

// include FTKs
#include "Ftk.h"

#include "../native_io/NativeFtk.h"

//XSI FTK not available for 64-bit systems.
//Maya API not available either.
#ifdef AFTK_XSI
#include "../xsi_io/XsiIo.h"
#endif
#ifdef AFTK_MAYA_IO
#include "../maya_io/MayaIo.h"
#endif
#ifdef AFTK_FBX
#include "../fbx_io/FbxIo.h"
#endif

namespace Aftk
{
	typedef Ut::UtVector<Ut::DynamicVector<U32> > PlugLogicalIndexList;

	struct C_PlugInfo
	{
		//Can be a connection to an animation curve or a clip mixer node.
		C_Node* pAnimConnection;
		//The logical indices for the plug.
		PlugLogicalIndexList plugLogicalIndexList;
	};
}

C_Scene::C_Scene()
{
	OneTimeInit(0);
}

C_Scene::C_Scene(C_Scene* pRootScene)
{
	OneTimeInit(pRootScene);
}

C_Scene::~C_Scene()
{
	Clear();

	if (m_pRootScene == this)
		delete m_pActiveXRefScenes;
}

U32 C_Scene::GetNumSceneNodes() const
{
	return (U32)m_sceneNodes.Size();
}

U32 C_Scene::GetNumMaterialNodes() const
{
	return m_materialNodeManager.GetNumResources();
}

U32 C_Scene::GetNumBlindDataNodes() const
{
	return m_blindDataNodeManager.GetNumResources();
}

U32 C_Scene::GetNumMeshVertexPositions() const
{
	if (m_pRootSceneNode == NULL) {
		//g_aftkDebugManager << "C_Scene::GetNumVertexPositions() root node is null.\n";
		return 0;
	}

	C_SceneNodeIterator sceneNodeIterator(m_pRootSceneNode);
	U32 count = 0;
	for (U32 i = 0; i < (U32)m_sceneNodes.Size(); i++) 
	{
		C_SceneNode* pSceneNode = sceneNodeIterator;
		
		if (pSceneNode == NULL) {
//			g_aftkDebugManager << "C_Scene::GetNumVertexPositions() Error - scene node iterator is null.\n";
			return 0;
		}

		switch (pSceneNode->GetType()) {
			case E_ObjType_SceneMesh: {
				C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)pSceneNode;
				count += pSceneMeshNode->GetNumVertexPositions();
				break;
			}
			default:
				break;
		}
		sceneNodeIterator++;
	}
	return count;
}

U32 C_Scene::GetNumMeshVertexNormals() const
{
	if (m_pRootSceneNode == NULL) 
	{
		return 0;
	}

	C_SceneNodeIterator sceneNodeIterator(m_pRootSceneNode);
	U32 count = 0;
	for (U32 i = 0; i < (U32)m_sceneNodes.Size(); i++) 
	{
		C_SceneNode* pSceneNode = sceneNodeIterator;
		
		if (pSceneNode == NULL) 
		{
			return 0;
		}

		switch (pSceneNode->GetType()) 
		{
			case E_ObjType_SceneMesh: 
			{
				C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)pSceneNode;
				count += pSceneMeshNode->GetNumVertexNormals();
				break;
			}
			default:
				break;
		}

		sceneNodeIterator++;
	}
	return count;
}

U32 C_Scene::GetNumMeshVertexTangents() const
{
	if (m_pRootSceneNode == NULL) 
	{
		return 0;
	}

	C_SceneNodeIterator sceneNodeIterator(m_pRootSceneNode);
	U32 count = 0;
	for (U32 i = 0; i < (U32)m_sceneNodes.Size(); i++) 
	{
		C_SceneNode* pSceneNode = sceneNodeIterator;
		
		if (pSceneNode == NULL) 
		{
			return 0;
		}

		switch (pSceneNode->GetType()) 
		{
			case E_ObjType_SceneMesh: 
			{
				C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)pSceneNode;
				for (U32 set = 0; set < pSceneMeshNode->GetNumTextureCoordinateSets(); set++)
				{
					count += pSceneMeshNode->GetNumVertexTangents(set);
				}
				break;
			}
			default:
				break;
		}

		sceneNodeIterator++;
	}
	return count;
}

U32 C_Scene::GetNumMeshVertexBinormals() const
{
	if (m_pRootSceneNode == NULL) 
	{
		return 0;
	}

	C_SceneNodeIterator sceneNodeIterator(m_pRootSceneNode);
	U32 count = 0;
	for (U32 i = 0; i < (U32)m_sceneNodes.Size(); i++) 
	{
		C_SceneNode* pSceneNode = sceneNodeIterator;
		
		if (pSceneNode == NULL) 
		{
			return 0;
		}

		switch (pSceneNode->GetType()) 
		{
			case E_ObjType_SceneMesh: 
			{
				C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)pSceneNode;
				for (U32 set = 0; set < pSceneMeshNode->GetNumTextureCoordinateSets(); set++)
				{
					count += pSceneMeshNode->GetNumVertexBinormals(set);
				}

				break;
			}
			default:
				break;
		}

		sceneNodeIterator++;
	}
	return count;
}

U32 C_Scene::GetNumMeshTextureCoordinates() const
{
	if (m_pRootSceneNode == NULL) 
	{
		return 0;
	}

	C_SceneNodeIterator sceneNodeIterator(m_pRootSceneNode);
	U32 count = 0;
	for (U32 i = 0; i < (U32)m_sceneNodes.Size(); i++) 
	{
		C_SceneNode* pSceneNode = sceneNodeIterator;
		
		if (pSceneNode == NULL) 
		{
			return 0;
		}

		switch (pSceneNode->GetType()) 
		{
			case E_ObjType_SceneMesh: 
			{
				C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)pSceneNode;
				for (U32 set = 0; set < pSceneMeshNode->GetNumTextureCoordinateSets(); set++)
				{
					count += pSceneMeshNode->GetNumTextureCoordinates(set);
					}
				break;
			}
			default:
				break;
		}

		sceneNodeIterator++;
	}
	return count;
}


U32 C_Scene::GetNumMeshPolygonVertices() const
{
	if (m_pRootSceneNode == NULL) {
//		g_aftkDebugManager << "C_Scene::GetNumPolygonVertices() root node is null.\n";
		return 0;
	}

	C_SceneNodeIterator sceneNodeIterator(m_pRootSceneNode);
	U32 count = 0;
	for (U32 i = 0; i < (U32)m_sceneNodes.Size(); i++) {
		C_SceneNode* pSceneNode = sceneNodeIterator;

		switch (pSceneNode->GetType()) {
			case E_ObjType_SceneMesh: {
				C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)pSceneNode;
				count += pSceneMeshNode->GetNumPolygonVertices();
				break;
			}
			default:
				break;
		}
		sceneNodeIterator++;
	}
	return count;
}

U32 C_Scene::GetNumMeshPolygons() const
{
	if (m_pRootSceneNode == NULL) {
//		g_aftkDebugManager << "C_Scene::GetNumPolygons() root node is null.\n";
		return 0;
	}

	C_SceneNodeIterator sceneNodeIterator(m_pRootSceneNode);
	U32 count = 0;
	while(sceneNodeIterator)
	{
		C_SceneNode* pSceneNode = sceneNodeIterator;

		switch (pSceneNode->GetType()) {
			case E_ObjType_SceneMesh: 
				{
				C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)pSceneNode;
				for (U32 i = 0; i < pSceneMeshNode->GetNumLodLevels(); i++)
				{
					count += pSceneMeshNode->GetNumPolygons(i);
				}
				break;
			}
			default:
				break;
		}
		sceneNodeIterator++;
	}
	return count;
}

U32 C_Scene::GetNumImageFileNames() const
{
	return 0;
}

C_SceneNode* C_Scene::GetRootSceneNodePtr()
{
	return m_pRootSceneNode;
}

const C_SceneNode* C_Scene::GetRootSceneNodePtr() const
{
	return m_pRootSceneNode;
}

C_SceneNode* C_Scene::GetSceneNodePtrByUniqueID(S32 id)
{
#ifdef AFTK_USE_MEMORY_POOL
	/*
	std::map<
		S32, 
		C_SceneNode*,
		std::less<S32>,
		StdPoolAllocator<std::pair<S32, C_SceneNode*> > >::iterator it;
	*/
	Ut::UtMap<S32, C_SceneNode*>::Iterator it;
#else
	Ut::UtMap<S32, C_SceneNode*>::Iterator it;
#endif
	it = m_sceneNodes.Find(id);
	if (id < 0)
	{
		return 0;
	}
	if (it == m_sceneNodes.End())
	{
		return 0;
	}

	return it->GetValue();
}

const C_SceneNode* C_Scene::GetSceneNodePtrByUniqueID(S32 id) const
{
#ifdef AFTK_USE_MEMORY_POOL
	/*
	std::map<
		S32, 
		C_SceneNode*,
		std::less<S32>,
		StdPoolAllocator<std::pair<S32, C_SceneNode*> > >::ConstIterator it;
	*/
	Ut::UtMap<S32, C_SceneNode*>::ConstIterator it;
#else
	Ut::UtMap<S32, C_SceneNode*>::ConstIterator it;
#endif
	it = m_sceneNodes.Find(id);
	if (id < 0)
	{
		return 0;
	}
	if (it == m_sceneNodes.End())
	{
		return 0;
	}

	return it->GetValue();
}

void C_Scene::SetRootSceneNodePtr(C_SceneNode *pNode)
{
	if (pNode)
	{
		pNode->m_pParent = 0;
		AFTK_ASSERT(pNode->m_pPrevious == 0);

		//Roots cannot have siblings - create a new root.
		if (pNode->GetNextPtr())
		{
			C_SceneNode* pNewRoot = new C_SceneNode(this);
			pNewRoot->SetName("AftkRoot");
			pNewRoot->AddChildPtr(pNode);
			m_pRootSceneNode = pNewRoot;
			return;
		}
	}
	m_pRootSceneNode = pNode;
}

void C_Scene::SetOrigin(const std::string& origin)
{
	m_origin = origin;
}

const std::string& C_Scene::GetOrigin() const
{
	return m_origin;
}

void C_Scene::SetMaterialAttachData(
	const C_MaterialResource* pMaterialResource,
	const C_AttachData& attachData)
{
	m_materialNodeManager.SetAttachData(pMaterialResource, attachData);
}

void C_Scene::SetMaterialAttachData(
	U32 instance,
	const C_AttachData& attachData)
{
	m_materialNodeManager.SetAttachData(instance, attachData);
}

const C_MaterialResource* C_Scene::AddMaterialNode(
	const C_MaterialNode& materialNode,
	bool enforceUnique)
{
	return m_materialNodeManager.AddResource(materialNode, enforceUnique);
}

void C_Scene::AddMaterialNodeNoTest(C_MaterialResource* pResource)
{
	return m_materialNodeManager.AddResourceNoTest(pResource);
}

void C_Scene::SetSceneBlindDataResource(const C_BlindDataResource* pRes)
{
	m_pSceneBlindDataResource = pRes;
}

const C_BlindDataResource* C_Scene::GetSceneBlindDataResourcePtr() const
{
	return m_pSceneBlindDataResource;
}

const C_BlindDataResource* C_Scene::AddBlindDataNode(
	const C_BlindDataNode &blindDataNode)
{
	return m_blindDataNodeManager.AddResource(blindDataNode);
}
void C_Scene::AddBlindDataNodeNoTest(C_BlindDataResource* pResource)
{
	return m_blindDataNodeManager.AddResourceNoTest(pResource);
}

void C_Scene::ExtractSceneNodeAndReparentChildren(
	C_SceneNode* pNode, bool updateChildMatrices)
{
	//Remove from any lod group.
	if (pNode->GetParentPtr())
	{
		if (pNode->GetParentPtr()->GetType() == E_ObjType_SceneLodGroup)
		{
			C_SceneLodGroupNode* pLodGroup = (C_SceneLodGroupNode*)pNode->GetParentPtr();
			pLodGroup->DeleteLodLevelInfo(pNode);
		}
	}

	if (pNode == 0)
	{
		AFTK_ASSERT(pNode == 0);
		return;
	}

	C_SceneNode* pPreviousNode = pNode->m_pPrevious;
	C_SceneNode* pNextNode = pNode->m_pNext;
	C_SceneNode* pParentNode = pNode->m_pParent;
	C_SceneNode* pChildNode = pNode->m_pChild;

	if (updateChildMatrices && pChildNode)
	{
		for (U32 iChild = 0; iChild < pNode->GetNumChildren(); iChild++)
		{
			C_SceneNode* pChild = pNode->GetChildPtr(iChild);
			pChild->SetLocalMatrix(pChild->GetLocalMatrix() * pNode->GetLocalMatrix());
		}
	}

	if (pParentNode)
	{
		//Adjust sibling array, shuffling the nodes along
		pParentNode->DeleteChildPtr(pNode);
	}
	else
	{
		//No siblings to adjust
	}

	//Does the node to be deleted have any children?
	if (pChildNode)
	{
		if (pParentNode == 0)
		{
			if (pNode == GetRootSceneNodePtr())
			{
				SetRootSceneNodePtr(pChildNode);
			}
			else
			//Just shuffle upward.
			if (!GetRootSceneNodePtr())
			{
				SetRootSceneNodePtr(pChildNode);
			}
			else
			{
				GetRootSceneNodePtr()->SetChildPtr(pChildNode);
			}
		}
		else
		{
			while (pChildNode)
			{
				C_SceneNode* pChildNext = pChildNode->GetNextPtr();
				pParentNode->AddChildPtr(pChildNode);
				pChildNode = pChildNext;
			}
		}
	}
	else if (pNode == m_pRootSceneNode)
	{
		//We're deleting a root with no children.
		SetRootSceneNodePtr(0);
	}

	pNode->SetNullHierachyPtrs();
}

void C_Scene::DeleteSceneNodeAndReparentChildren(
	C_SceneNode* pNode, bool updateChildMatrices)
{
	ExtractSceneNodeAndReparentChildren(pNode, updateChildMatrices);
	delete pNode;
}

void C_Scene::DeleteSceneNodeAndChildren(C_SceneNode* pNode)
{
	if (!pNode)
	{
		return;
	}

	while(pNode->GetChildPtr())
	{
		C_SceneNode* pChild = pNode->GetChildPtr();
		DeleteSceneNodeAndChildren(pChild);
	}

	ExtractSceneNodeAndReparentChildren(pNode, false);
	delete pNode;
}

void C_Scene::DeleteChildren(C_SceneNode* pNode)
{
	if (!pNode)
		return;

	while(pNode->GetChildPtr())
	{
		C_SceneNode* pChild = pNode->GetChildPtr();
		DeleteSceneNodeAndChildren(pChild);
	}
}

void C_Scene::ExtractSceneNodeAndChildren(C_SceneNode* pNode)
{
	//Remove from any lod group.
	if (pNode->GetParentPtr())
	{
		if (pNode->GetParentPtr()->GetType() == E_ObjType_SceneLodGroup)
		{
			C_SceneLodGroupNode* pLodGroup = (C_SceneLodGroupNode*)pNode->GetParentPtr();
			pLodGroup->DeleteLodLevelInfo(pNode);
		}
	}

	if (pNode == 0)
	{
		AFTK_ASSERT(pNode == 0);
		return;
	}

	C_SceneNode* pPreviousNode = pNode->m_pPrevious;
	C_SceneNode* pNextNode = pNode->m_pNext;
	C_SceneNode* pParentNode = pNode->m_pParent;
	C_SceneNode* pChildNode = pNode->m_pChild;

	if (pParentNode)
	{
#ifdef _DEBUG
		for (U32 iChild = 0; iChild < pParentNode->GetNumChildren(); iChild++)
		{
			C_SceneNode* pChild = pParentNode->GetChildPtr(iChild);
			AFTK_ASSERT(pChild->m_pParent == pParentNode);
		}
#endif

		//Adjust sibling array, shuffling the nodes along
		pParentNode->DeleteChildPtr(pNode);
	}
	else if (pNode == m_pRootSceneNode)
	{
		//We're extracting a root with no children.
		SetRootSceneNodePtr(0);
	}
}

const C_BlindDataResource* C_Scene::GetIthBlindDataResourcePtr(U32 instance) const
{
	return m_blindDataNodeManager.GetIthResourcePtr(instance);
}

C_BlindDataResource* C_Scene::GetIthBlindDataResourcePtr(U32 instance)
{
	return m_blindDataNodeManager.GetIthResourcePtr(instance);
}

void C_Scene::Clear()
{
	//Delete layers first so that we don't do needless work unhooking scene members.
	{
		Ut::UtVector<Ut::DynamicVector<C_LayerNode*> > layers = m_layers;
		for (U32 i = 0; i < (U32)layers.Size(); i++)
			delete layers[i];
	}

	//Delete scene graph.
	DeleteSceneNodeAndChildren(m_pRootSceneNode);
	m_pRootSceneNode = 0;
	m_nextSceneNodeUniqueId = 0;

	//Delete remaining unmanaged dependency graph nodes.
	{
		//Build a list of nodes to delete.
		Ut::UtVector<Ut::DynamicVector< C_Node* > > toDelete;
		toDelete.Reserve(m_unmanagedDgNodes.Size());
		for (
			Ut::UtSet<C_Node*>::Iterator dgIt = m_unmanagedDgNodes.Begin(); 
			dgIt != m_unmanagedDgNodes.End(); 
			++dgIt)
			toDelete.PushBack(*dgIt);

		//Delete nodes.
		for (U32 i = 0; i < (U32)toDelete.Size(); i++)
			delete toDelete[i];

		//ASSERT that all nodes have been deleted.
		AFTK_ASSERT(m_unmanagedDgNodes.Size() == 0);
		m_unmanagedDgNodes.Clear();
	}
	//Delete managed resources.
	m_materialNodeManager.Clear();
	m_blindDataNodeManager.Clear();

	//Unload any x-ref scenes.
	UnloadAllXRefScenes();

	m_origin.clear();
	m_pSceneBlindDataResource = 0;

	//Clear animation related proporties
	m_frameRate = 0;
	m_startTime = 0;
	m_endTime = 0;
	m_currentTime = 0;

	//Note that we don't modify the x-ref root scene pointer. This means that even after invoking clear, 
	//a referenced scene retains its attachment to the parent scene. This attachment cannot be
	//ever be broken.
	if (m_pRootScene == this)
	{
		AFTK_ASSERT(m_pActiveXRefScenes);
		//Delete x-ref scenes.
		for (U32 iXRefScene = 0; iXRefScene < (U32)m_pActiveXRefScenes->Size(); iXRefScene++)
		{
			C_XRefScene* pXRefScene = (*m_pActiveXRefScenes)[iXRefScene].pXRefScene;
			delete pXRefScene;
		}
		m_pActiveXRefScenes->Clear();
	}
	m_xRefScenes.Clear();
	Aftk::C_MemoryManager::CollectGarbage();
}

void C_Scene::OutputSceneGraph()
{
	AFTK_DO_BEGIN << "Dumping hierachy:\n" AFTK_DO_INFO_END;
	if (m_pRootSceneNode)
	{
		OutputSceneGraph(m_pRootSceneNode, 0);
	}
	AFTK_DO_BEGIN << "Hierachy dump complete.\n" AFTK_DO_INFO_END;
}

U32 C_Scene::MakeUniqueSceneNodeNames()
{
	U32 numRenamings = 0;
	Ut::UtSet<std::string> nameSet;

	C_SceneNodeIterator it(m_pRootSceneNode);

	while (it)
	{
		C_SceneNode* pSceneNode = it;

		while (true)
		{
			Ut::UtSet<std::string>::Iterator setIt;
			setIt = nameSet.Find(pSceneNode->GetName());

			if (setIt == nameSet.End())
			{
				nameSet.Insert(pSceneNode->GetName());
				break;
			}
			else
			{
				std::string oldName = pSceneNode->GetName();
				pSceneNode->SetName(GetUniqueName(oldName));
				numRenamings++;
			}
		}
		++it;
	};

	return numRenamings;
}

C_LayerNode* C_Scene::GetLayerPtr(U32 instance)
{
	if (instance == 0 && m_layers.Size() == 0)
		return NULL;

	return m_layers[instance];
}

const C_LayerNode* C_Scene::GetLayerPtr(U32 instance) const
{
	if (instance == 0 && m_layers.Size() == 0)
		return NULL;

	return m_layers[instance];
}

S32 C_Scene::GetLayerIndex(const C_LayerNode* pLayer) const
{
	for (U32 i = 0; i < (U32)m_layers.Size(); i++)
	{
		if (m_layers[i] == pLayer)
		{
			return (S32)i;
		}
	}

	// we didn't find the layer!
	return -1;
}

U32 C_Scene::GetNumLayers() const
{
	return (U32)m_layers.Size();
}

const C_MaterialResource* C_Scene::GetMaterialResourcePtrBySubString(const std::string& _subString) const
{
	for (U32 iMat = 0; iMat < m_materialNodeManager.GetNumResources(); iMat++)
	{
		const C_MaterialResource* pMatRes = m_materialNodeManager.GetResourcePtr(iMat);

		std::string subString(_subString);
		std::transform(
			subString.begin(), 
			subString.end(),
			subString.begin(),
			tolower);

		std::string searchString(pMatRes->GetContent().GetName());
		std::transform(
			searchString.begin(),
			searchString.end(),
			searchString.begin(),
			tolower);

		if (searchString.find(subString) != std::string::npos)
		{
			return pMatRes;
		}
	}
	return 0;
}

const C_MaterialResource* C_Scene::GetMaterialResourcePtrByName(const std::string& name) const
{
	for (U32 iMat = 0; iMat < m_materialNodeManager.GetNumResources(); iMat++)
	{
		const C_MaterialResource* pMatRes = m_materialNodeManager.GetResourcePtr(iMat);

		std::string searchName(name);
		std::transform(
			searchName.begin(), 
			searchName.end(),
			searchName.begin(),
			tolower);

		std::string searchString(pMatRes->GetContent().GetName());
		std::transform(
			searchString.begin(),
			searchString.end(),
			searchString.begin(),
			tolower);

		if (searchName == searchString)
			return pMatRes;
	}

	return 0;
}

const C_MaterialResource* C_Scene::GetMaterialResourcePtr(U32 instance) const
{
	return m_materialNodeManager.GetResourcePtr(instance);
}

void C_Scene::SetFrameRate(F32 frameRate)
{
	m_frameRate = frameRate;
}

void C_Scene::SetStartTime(F32 startTime)
{
	m_startTime = startTime;
}

void C_Scene::SetEndTime(F32 endTime)
{
	m_endTime = endTime;
}

void C_Scene::SetCurrentTime(F32 currentTime, bool invalidateCache)
{
	if (fabsf(m_currentTime - currentTime) < AFTK_EPSILON)
		return;

	m_currentTime = currentTime;

	if (invalidateCache)
	{
		for (Ut::UtSet<C_Node*>::Iterator it = m_unmanagedDgNodes.Begin(); 
			it != m_unmanagedDgNodes.End();
			++it)
		{
			C_Node* pNode = it->GetKey();
			if (pNode->GetType() == E_ObjType_AnimationCurve)
			{
				C_AnimationCurveNode* pCurve = (C_AnimationCurveNode*)pNode;
				if (pCurve->IsTimeInput())
				{
					//The on-update callback used by the curve will ensure that all nodes
					//which are time-dependant will be updated.
					pCurve->GetInputPlug().OnChanged();
				}
			}

//			it->GetValue()->DirtyDynamicTransformDependantCaches(false);
		}
	}
}

F32 C_Scene::GetFrameRate() const
{
	return m_frameRate;
}

F32 C_Scene::GetStartTime() const
{
	return m_startTime;
}

F32 C_Scene::GetEndTime() const
{
	return m_endTime;
}

F32 C_Scene::GetCurrentTime() const
{
	return m_currentTime;
}

F32 C_Scene::GetDuration() const
{
	return m_endTime - m_startTime;
}

C_Bounds C_Scene::GetEvaluatedBounds(const ObjTypeArray* pFilter) const
{
	C_Bounds sceneBounds;

	for (Ut::UtMap<
		S32, 
		C_SceneNode*>::ConstIterator it = m_sceneNodes.Begin(); 
		it != m_sceneNodes.End();
		++it)
	{
		C_SceneNode* pSceneNode = it->GetValue();
		bool filtered = false;
		if (pFilter)
			for (U32 i = 0; i < (U32)pFilter->Size(); i++)
				if (pSceneNode->GetType() == (*pFilter)[i])
				{
					filtered = true;
					break;
				}

		if (!filtered)
			sceneBounds = sceneBounds + pSceneNode->GetEvaluatedWorldBounds();
	}

	return sceneBounds;

}

void C_Scene::SetRootFilePath(const Sys::C_FilePath& filePath)
{
	if (m_pRootSceneNode)
	{
		Aftk::C_DynamicPlug* pPlug = new Aftk::C_DynamicPlug;
		m_pRootSceneNode->AddPlugPtr(pPlug);
		pPlug->SetName("originalFilePath");
		pPlug->SetValue(std::string(filePath.Chars()));
	}
}

Sys::C_FilePath C_Scene::GetRootFilePath()
{
	if (m_pRootSceneNode)
	{
		Aftk::I_Plug* pPlug = m_pRootSceneNode->GetPlugPtr("originalFilePath");
		if (pPlug)
		{
			std::string fp;
			pPlug->GetValue(fp);
			return fp.c_str();
		}
	}
	return "";
}

C_Bounds C_Scene::GetBounds(const ObjTypeArray* pFilter) const
{
	C_Bounds sceneBounds;

	for (Ut::UtMap<
		S32, 
		C_SceneNode*>::ConstIterator it = m_sceneNodes.Begin(); 
		it != m_sceneNodes.End();
		++it)
	{
		C_SceneNode* pSceneNode = it->GetValue();
		bool filtered = false;
		if (pFilter)
			for (U32 i = 0; i < (U32)pFilter->Size(); i++)
				if (pSceneNode->GetType() == (*pFilter)[i])
				{
					filtered = true;
					break;
				}

		if (!filtered)
			sceneBounds = sceneBounds + pSceneNode->GetWorldBounds();
	}

	return sceneBounds;
}

U32 C_Scene::DeleteUnusedMaterialNodes()
{
	U32 deletedCount = 0;
	Ut::UtVector<Ut::DynamicVector< const C_MaterialResource*> > resourceList;
	U32 nMtlNodes = GetNumMaterialNodes();
	resourceList.Resize(nMtlNodes);
	for (U32 i = 0; i < nMtlNodes; i++)
		resourceList[i] = GetMaterialResourcePtr(i);;

	for (U32 i = 0; i < (U32)resourceList.Size(); i++)
	{
		if (resourceList[i]->GetReferenceCount() == 0)
		{
			m_materialNodeManager.DeleteResource(resourceList[i]);
			deletedCount++;
		}
	}
	return deletedCount;
}

U32 C_Scene::DeleteUnusedBlindDataNodes()
{
	U32 deletedCount = 0;
	Ut::UtVector<Ut::DynamicVector< const C_BlindDataResource*> > resourceList;
	resourceList.Resize(m_blindDataNodeManager.GetCapacity());

	for (U32 i = 0; i < (U32)resourceList.Size(); i++)
		resourceList[i] = GetIthBlindDataResourcePtr(i);

	for (U32 i = 0; i < (U32)resourceList.Size(); i++)
	{
		if (!resourceList[i])
			continue;
		if (resourceList[i]->GetReferenceCount() == 0)
		{
			U32 oldSize = m_blindDataNodeManager.GetNumResources();
			m_blindDataNodeManager.DeleteResource(resourceList[i]);
			U32 newSize = m_blindDataNodeManager.GetNumResources();
			AFTK_ASSERT(newSize == oldSize -1);
			deletedCount++;
		}
	}

	//Rehash blind data manager to remove deleted entries.
	m_blindDataNodeManager.Optimise();

	return deletedCount;
}

U32 C_Scene::DeleteUnusedMeshPolygonMaterialMappings()
{
	U32 removed = 0;
	C_SceneNodeIterator it(this);
	while(it)
	{
		C_SceneNode* pSceneNode = it;

		if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
		{
			C_SceneMeshRemappableNode* pMesh = (C_SceneMeshRemappableNode*)pSceneNode;
			removed += pMesh->DeleteUnusedPolygonMaterialMappings();
		}

		++it;
	}

	return removed;
}

U32 C_Scene::DeleteUnusedMeshPolygonBlindDataMappings()
{
	U32 removed = 0;
	C_SceneNodeIterator it(this);
	while(it)
	{
		C_SceneNode* pSceneNode = it;

		if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
		{
			C_SceneMeshRemappableNode* pMesh = (C_SceneMeshRemappableNode*)pSceneNode;
			removed += pMesh->DeleteUnusedPolygonBlindDataMappings();
		}

		++it;
	}

	return removed;
}

void C_Scene::Optimise()
{
//	#pragma message("Optimised function commented out for testing purposes. If we find this fixes our GMIG issues then I'll stick it back in. It might be removing mappings that barrier instances require.")
//	return;

	C_SceneNodeIterator it(this);
	while(it)
	{
		C_SceneNode* pSceneNode = it;

		if (pSceneNode->HasType(Aftk::E_ObjType_SceneMeshRemappable))
		{
			C_SceneMeshRemappableNode* pMesh = (C_SceneMeshRemappableNode*)pSceneNode;
			pMesh->DeleteUnusedPolygonMaterialMappings();
			pMesh->DeleteUnusedPolygonBlindDataMappings();
		}

		++it;
	}

	DeleteUnusedBlindDataNodes();
	DeleteUnusedMaterialNodes();
	DeleteUnusedDgNodes();
}

const C_SceneNode* C_Scene::GetSceneNodePtrByPartialPath(const std::string& path) const
{
	return GetSceneNodePtrByPartialPath(path, m_pRootSceneNode);
}

C_SceneNode* C_Scene::GetSceneNodePtrByPartialPath(const std::string& path)
{
	//This way we avoid writing the above function twice.
	const C_SceneNode* pSceneNode = 
		((const C_Scene*)this)->GetSceneNodePtrByPartialPath(path);
	return const_cast<C_SceneNode*>(pSceneNode);
}

const C_SceneNode* C_Scene::GetSceneNodePtrByFullPath(const std::string& path) const
{
	return GetSceneNodePtrByFullPath(path, m_pRootSceneNode);
}

C_SceneNode* C_Scene::GetSceneNodePtrByPathByFullPath(const std::string& path)
{
	//This way we avoid writing the above function twice.
	const C_SceneNode* pSceneNode = 
		((const C_Scene*)this)->GetSceneNodePtrByFullPath(path);
	return const_cast<C_SceneNode*>(pSceneNode);
}

C_SceneNode* C_Scene::GetSceneNodePtr(const std::string& name)
{
	//This way we avoid writing the above function twice.
	const C_SceneNode* pSceneNode = 
		((const C_Scene*)this)->GetSceneNodePtr(name);
	return const_cast<C_SceneNode*>(pSceneNode);
}

const C_SceneNode* C_Scene::GetSceneNodePtr(const std::string& name) const
{
	C_SceneNodeConstIterator it(this);
	while(it)
	{
		const C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetName() == name)
		{
			return pSceneNode;
		}
		++it;
	}
	return 0;
}

C_SceneXRefNode* C_Scene::GetSceneXRefNodePtr(const std::string& name)
{
	//This way we avoid writing the above function twice.
	const C_SceneXRefNode* pSceneXRefNode = 
		((const C_Scene*)this)->GetSceneXRefNodePtr(name);
	return const_cast<C_SceneXRefNode*>(pSceneXRefNode);
}

const C_SceneXRefNode* C_Scene::GetSceneXRefNodePtr(const std::string& name) const
{
	C_SceneNodeConstIterator it(this);
	while(it)
	{
		const C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetType() == E_ObjType_SceneXRef)
		{
			const C_SceneXRefNode* pSceneXRefNode = 
				(const C_SceneXRefNode*)pSceneNode;
			if (pSceneXRefNode->GetSourceSceneFileName() == name)
			{
				return pSceneXRefNode;
			}
		}
		++it;
	}
	return 0;
}

C_Status C_Scene::ReadExternalScene(
	E_ExternalScene target,
	const char* pOptions,
	I_Progress* pProgressCallback)
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("");
		pProgressCallback->OnProgress(0);
	}
	C_Status status;
	Clear();

	#ifdef AFTK_MAYA_IO
	if (target != Aftk::E_ExternalScene_Maya)
		return C_Status(C_Status::E_Status_Failure, AFTK_E_FIND_EXTERNAL_SOURCE_SCENE_FAIL);
	#endif

	#ifdef AFTK_MAYA_IO
	{
		MayaIo::C_Main mayaIoMain;
		return mayaIoMain.ReadExternalScene(
			(C_Scene*)this, 
			pOptions,
			pProgressCallback);
	}
	#endif

	return C_Status::E_Status_Failure;
}

C_Status C_Scene::Read(
	const std::string& fileName, 
	E_FileType format, 
	const char* pOptions,
	I_Progress* pProgressCallback)
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("");
		pProgressCallback->OnProgress(0);
	}
	C_Status status;
	Clear();

	if (format == E_FileType_BestGuess)
	{
		status = C_Global::GetFileType(fileName, format);
		if (!status) return status;
	}

	switch (format)
	{
		case E_FileType_A3d:
		#ifdef AFTK_XSI
		case E_FileType_Xsi:
		#endif
		#ifdef AFTK_MAYA_IO
		case E_FileType_Ma:
		case E_FileType_Mb:
		#endif
		#ifdef AFTK_FBX
		case E_FileType_Fbx:
		#endif
		{
			status = ReadFile(fileName, format, pOptions, pProgressCallback);
			return status;
		}

		default:
		{
			status.Set(C_Status::E_Status_Failure, "Could not determine file type.", 1);
			return status;
		}
	}
    
	return C_Status::E_Status_Success;
}

C_Status C_Scene::WriteExternalScene(
	E_ExternalScene target,
	const char* pOptions,
	I_Progress* pProgressCallback) const
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("");
		pProgressCallback->OnProgress(0);
	}

	#ifdef AFTK_MAYA_IO
	if (target != Aftk::E_ExternalScene_Maya)
		return C_Status(C_Status::E_Status_Failure, AFTK_E_FIND_EXTERNAL_TARGET_SCENE_FAIL);
	#endif

	#ifdef AFTK_MAYA_IO
	{
		MayaIo::C_Main mayaIoMain;
		return mayaIoMain.WriteExternalScene(
			(C_Scene*)this,
			pOptions, 
			pProgressCallback);
	}
	#endif

	return C_Status::E_Status_Failure;	
}


C_Status C_Scene::Write(
	const std::string& fileName, 
	E_FileType format, 
	U32 version, 
	const char* pOptions,
	I_Progress* pProgressCallback) const
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("");
		pProgressCallback->OnProgress(0);
	}

	C_Status status;

	if (format == E_FileType_BestGuess)
	{
		status = C_Global::GetFileType(fileName, format);
		if (!status)
		{
			return status;
		}
	}
		
	switch (format)
	{
		case E_FileType_A3d:
		case E_FileType_Mb:
		case E_FileType_Ma:
		case E_FileType_Xsi:
			return WriteFile(fileName, format, version, pOptions, pProgressCallback);

		default:
		{
			status.Set(C_Status::E_Status_Failure, "Failed to write file. Unknown file extension specified.", 1);
			return status;
		}
	}

	return C_Status::E_Status_Failure;
}

// Protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_Scene::DeleteDgNode(C_Node* pNode)
{
	Ut::UtSet<C_Node*>::Iterator it;
	it = m_unmanagedDgNodes.Find(pNode);
	if (it != m_unmanagedDgNodes.End())
        m_unmanagedDgNodes.Erase(it);
}

void C_Scene::OutputSceneGraph(const C_SceneNode* pSceneNode, U32 level)
{
	for (U32 i = 0; i < level; i++)
	{
		AFTK_DO_BEGIN << "   " AFTK_DO_INFO_END;
	}
	AFTK_DO_BEGIN << "[" << pSceneNode->GetName() << "]" AFTK_DO_INFO_END;
	if (pSceneNode->GetType() == E_ObjType_SceneXRef)
	{
		const C_SceneXRefNode* pSceneXRefNode = (const C_SceneXRefNode*)pSceneNode;
		if ((pSceneXRefNode->GetSourceNodePath().length() != 0) 
			&& 
			(pSceneXRefNode->GetSourceSceneFileName().length() != 0))
		{
			AFTK_DO_BEGIN << " references [" << pSceneXRefNode->GetSourceNodePath() << "] from [" << pSceneXRefNode->GetSourceSceneFileName() << "]" AFTK_DO_INFO_END;
			if (pSceneXRefNode->GetSourceNodePtr() != 0)
			{
				AFTK_DO_BEGIN << " *ONLINE* " AFTK_DO_INFO_END;
			}
			else
			{
				AFTK_DO_BEGIN << " *OFFLINE* " AFTK_DO_INFO_END;
			}
		}
		else
		{
			if (pSceneXRefNode->GetSourceNodePath().length() == 0)
			{
				AFTK_DO_BEGIN << " ERROR - invalid reference node filename.\n" AFTK_DO_INFO_END;
			}
			if (pSceneXRefNode->GetSourceSceneFileName().length() == 0)
			{
				AFTK_DO_BEGIN << " ERROR - invalid reference scene filename.\n" AFTK_DO_INFO_END;
			}
		}
	}
	else if (pSceneNode->GetType() == E_ObjType_SceneInstance)
	{
		const C_SceneInstanceNode* pSceneInstanceNode = (const C_SceneInstanceNode*)pSceneNode;
		AFTK_ASSERT(pSceneInstanceNode->GetSourceNodePtr());
		if (pSceneInstanceNode->GetSourceNodePtr())
		{
			AFTK_DO_BEGIN << " instances [" << pSceneInstanceNode->GetSourceNodePtr()->GetName() << "]" AFTK_DO_INFO_END;
		}
		else
		{
			AFTK_DO_BEGIN << " ERROR - invalid instance source.\n" AFTK_DO_INFO_END;
		}
	}
	else if (pSceneNode->GetType() == E_ObjType_SceneMeshBonedInstance)
	{
		const C_SceneMeshBonedInstanceNode* pSceneMeshBonedInstanceNode = (const C_SceneMeshBonedInstanceNode*)pSceneNode;
		AFTK_ASSERT(pSceneMeshBonedInstanceNode->GetSourceNodePtr());
		if (pSceneMeshBonedInstanceNode->GetSourceNodePtr())
		{
			AFTK_DO_BEGIN << " instances [" << pSceneMeshBonedInstanceNode->GetSourceNodePtr()->GetName() << "]" AFTK_DO_INFO_END;
		}
		else
		{
			AFTK_DO_BEGIN << " ERROR - invalid instance source.\n" AFTK_DO_INFO_END;
		}
	}
	else if (pSceneNode->GetType() == E_ObjType_SceneMeshRemappedInstance)
	{
		const C_SceneMeshRemappedInstanceNode* pSceneMeshRemappedInstanceNode = (const C_SceneMeshRemappedInstanceNode*)pSceneNode;
		
		AFTK_ASSERT(pSceneMeshRemappedInstanceNode->GetSourceNodePtr());
		if (pSceneMeshRemappedInstanceNode->GetSourceNodePtr())
		{
			AFTK_DO_BEGIN << " instances [" << pSceneMeshRemappedInstanceNode->GetSourceNodePtr()->GetName() << "]" AFTK_DO_INFO_END;
		}
		else
		{
			AFTK_DO_BEGIN << " ERROR - invalid instance source.\n" AFTK_DO_INFO_END;
		}
	}
	AFTK_DO_BEGIN << "\n" AFTK_DO_INFO_END;

	const C_SceneNode* pChild = pSceneNode->GetChildPtr();
	while(pChild)
	{
		if (pSceneNode->GetChildPtr())
		{
			OutputSceneGraph(pChild, level+1);
		}
		pChild = pChild->GetNextPtr();
	}
}

U32 C_Scene::ResourceIndirectInstances()
{
	U32 count = 0;
	//Remove instances of instances.
	C_SceneNodeIterator it(this);
	for (; it ; ++it)
	{
		C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetType() == E_ObjType_SceneInstance)
		{
			C_SceneInstanceNode* pInst = (C_SceneInstanceNode*)pSceneNode;
			//Correct source node if needed.
			C_SceneNode* pCorrectSourceNode = pInst->GetTopLevelSourceNodePtr();
			if (pInst->GetSourceNodePtr() != pCorrectSourceNode)
			{
				pInst->SetSourceNodePtr(pCorrectSourceNode);
				count++;
			}
		}
	}

	return count;
}


S32 C_Scene::OnCreateSceneNode(C_SceneNode* pSceneNode)
{
	AFTK_ASSERT(pSceneNode);
	if (!pSceneNode)
		return -1;

	S32 newNodeInstance = m_nextSceneNodeUniqueId;
	m_sceneNodes[newNodeInstance] = pSceneNode;
	m_nextSceneNodeUniqueId++;
	return newNodeInstance;
}

void C_Scene::OnDestroySceneNode(C_SceneNode* pSceneNode)
{
	AFTK_ASSERT(pSceneNode);
	if (!pSceneNode)
		return;
	
	// Remove this node from our array of scene nodes
	S32 sceneNodeInstance = pSceneNode->GetUniqueId();
	if (sceneNodeInstance == -1)
	{
		AFTK_DO_BEGIN << "The node [" << pSceneNode->GetName() << "] has a sceneNodeInstance of -1.\n" AFTK_DO_INFO_END;
//		g_aftkDebugManager << "Hopefully this is as it was an iterator, and not a proper scene node.\n";
		return;
	}
	m_sceneNodes.Erase(sceneNodeInstance);
}

void C_Scene::OnCreateUnmanagedDgNode(C_Node* pNode)
{
	AFTK_ASSERT(pNode);
	if (!pNode)
		return;

	m_unmanagedDgNodes.Insert(pNode);;
}

void C_Scene::OnDestroyUnmanagedDgNode(C_Node* pNode)
{
	AFTK_ASSERT(pNode);
	if (!pNode)
		return;

	m_unmanagedDgNodes.Erase(pNode);
}

void C_Scene::IncrementResourceReferenceCount(const C_BlindDataResource* pRes)
{
	if (pRes)
		m_blindDataNodeManager.IncrementIthReferenceCount(pRes->GetIndex(), 1);
}

void C_Scene::IncrementResourceReferenceCount(const C_MaterialResource* pRes)
{
	if (pRes)
		m_materialNodeManager.IncrementReferenceCount(pRes->GetIndex());
}

void C_Scene::DecrementResourceReferenceCount(const C_BlindDataResource* pRes)
{
	if (pRes)
		m_blindDataNodeManager.DecrementIthReferenceCount(pRes->GetIndex(), 1);
}

void C_Scene::DecrementResourceReferenceCount(const C_MaterialResource* pRes)
{
	if (pRes)
	{
		m_materialNodeManager.DecrementReferenceCount(pRes->GetIndex());
	}
}

U32 C_Scene::Repair(I_Progress* pProgressCallback)
{
	if (pProgressCallback)
		if (pProgressCallback->OnProgress(0))
			return 0;

	U32 errorCount = 0;
	C_SceneNodeIterator aftkIt(m_pRootSceneNode);

	U32 numProcessingStagesCompleted = 0;
	U32 numSceneNodes = GetNumSceneNodes();

	while (aftkIt != NULL)
	{
		C_SceneNode* pSceneNode = aftkIt;
		C_SceneMeshNode* pSceneMeshNode = (C_SceneMeshNode*)pSceneNode;
		if (pProgressCallback)
			pProgressCallback->OnPrintProgress(pSceneMeshNode->GetName());

		errorCount += pSceneNode->Repair();

		numProcessingStagesCompleted++;
		F32 progress = ((F32)numProcessingStagesCompleted / (F32)numSceneNodes) * 100.0f;
		if (pProgressCallback)
			if (pProgressCallback->OnProgress(progress))
				return errorCount;
		aftkIt++;
	}

	return errorCount;
}


U32 C_Scene::Validate(I_Progress* pProgressCallback) const
{
	C_Scene* pThis = const_cast<C_Scene*>(this);
	return pThis->ProcessProblems(false, pProgressCallback);
}

U32 C_Scene::ProcessProblems(bool fix, I_Progress* pProgressCallback) 
{
	if (pProgressCallback)
	{
		pProgressCallback->OnPrintProgress("Validating scene...");
		if (pProgressCallback->OnProgress(0))
		{
			return 0;
		}
	}

	C_SceneNodeConstIterator it(this);

	U32 errorCount = 0;

#ifdef AFTK_USE_MEMORY_POOL
	/*
	std::map<
		S32, 
		C_SceneNode*,
		std::less<S32>,
		StdPoolAllocator<std::pair<S32, C_SceneNode*> > > sceneNodesNotVisited = m_sceneNodes;
	*/
	Ut::UtMap<
		S32, 
		C_SceneNode*> sceneNodesNotVisited = m_sceneNodes;
#else
	Ut::UtMap<
		S32, 
		C_SceneNode*> sceneNodesNotVisited = m_sceneNodes;
#endif
	U32 numProcessingStagesCompleted = 0;
	U32 numSceneNodes = GetNumSceneNodes();

	while (it)
	{
		const C_SceneNode* pSceneNode = it;
		F32 progress = ((F32)numProcessingStagesCompleted / (F32)numSceneNodes) * 100.0f;

		if (pProgressCallback)
		{
			pProgressCallback->OnPrintProgress(pSceneNode->GetName());
		}

		errorCount += pSceneNode->Validate();
#ifdef _DEBUG
		AFTK_ASSERT(errorCount == 0);
#endif
		++it;
		numProcessingStagesCompleted++;
		S32 instance = pSceneNode->GetUniqueId();
		sceneNodesNotVisited.Erase(instance);

		if (pProgressCallback)
		{
			if (pProgressCallback->OnProgress(progress))
			{
				return errorCount;
			}
		}
	}

#ifdef AFTK_USE_MEMORY_POOL
/*
	std::map<
			S32, 
			C_SceneNode*,
			std::less<S32>,
			StdPoolAllocator<std::pair<S32, C_SceneNode*> > >::const_iterator mapIt;
			*/
	Ut::UtMap<
			S32, 
			C_SceneNode*>::ConstIterator mapIt;
#else
	Ut::UtMap<
			S32, 
			C_SceneNode*>::const_iterator mapIt;
#endif

	for (mapIt = sceneNodesNotVisited.Begin(); mapIt != sceneNodesNotVisited.End(); mapIt++)
	{
		const C_SceneNode* pSceneNode = mapIt->GetValue();
		#ifdef _DEBUG
		AFTK_DO_BEGIN << mapIt->GetKey() << " Orphaned scene node inst " << pSceneNode->GetUniqueId() << " " << pSceneNode->GetName() << " found.\n" AFTK_DO_INFO_END;
		#endif
		#ifdef _DEBUG
		AFTK_ASSERT(!("Orphaned scene node!"));
		#endif
		errorCount++;
	}

	errorCount += ValidateBlindDataReferenceCounts();
	errorCount += ValidateMaterialReferenceCounts();

	if (pProgressCallback)
	{
		pProgressCallback->OnProgress(100.f);
	}

	errorCount += ValidateLayers();

	return errorCount;
}

U32 C_Scene::OnCreateLayer(C_LayerNode* newLayer)
{
	m_layers.PushBack(newLayer);
	return (U32)(m_layers.Size() -1);
}

void C_Scene::OnDestroyLayer(C_LayerNode* pLayer)
{
	for (U32 i = 0; i < (U32)m_layers.Size(); i++)
	{
		if (m_layers[i] == pLayer)
		{
			m_layers.EraseByIndex(i);
			return;
		}
	}
}

/***************************** X-Ref related functions *****************************/
/*
S32 C_Scene::AddXRefScene(C_Status& status)
{
	AFTK_ASSERT(m_pActiveXRefScenes);

	//We've not encountered this scene before, add it.
	C_XRefScene* pNewXRefScene = new C_XRefScene(m_pRootScene);

	//Add to global scene list - this deallocates C_XRefScene memory.
	AddActiveXRefScene(pNewXRefScene, status);
	if (!status) return -1;

	//Add this x-ref to our local scene list.
	m_xRefScenes.PushBack(pNewXRefScene);

	//Return the local index of the new scene entry.
	status = C_Status::E_Status_Success;
	return m_xRefScenes.Size()-1;
}
*/
S32 C_Scene::AddXRefScene(const std::string& fileName, C_Status& status)
{
	AFTK_ASSERT(m_pActiveXRefScenes);
	for (U32 iScene = 0; iScene < (U32)m_xRefScenes.Size(); iScene++)
	{
		C_XRefScene* pXRefScene = m_xRefScenes[iScene];
		if (pXRefScene->GetFileName() == fileName)
		{
			//We've already added this scene before, return its index.
			status = C_Status::E_Status_Success;
			return iScene;
		}
	}

	//We've not encountered this scene before, add it.
	C_XRefScene* pNewXRefScene = new C_XRefScene(m_pRootScene);
	status = pNewXRefScene->SetSourceScene(fileName);
	if (!status) return -1;

	//Add to global scene list - this deallocates C_XRefScene memory.
	AddActiveXRefScene(pNewXRefScene, status);
	if (!status) return -1;

	//Add this x-ref to our local scene list.
	m_xRefScenes.PushBack(pNewXRefScene);

	//Return the local index of the new scene entry.
	status = C_Status::E_Status_Success;
	return m_xRefScenes.Size()-1;
}

S32 C_Scene::AddXRefScene(const std::string& fileName, U32 crc, C_Status& status)
{
	AFTK_ASSERT(m_pActiveXRefScenes);
	for (U32 iScene = 0; iScene < (U32)m_xRefScenes.Size(); iScene++)
	{
		C_XRefScene* pXRefScene = m_xRefScenes[iScene];
		if (pXRefScene->GetFileName() == fileName)
		{
			//We've already added this scene before, return its index.
			status = C_Status::E_Status_Success;
			return iScene;
		}
	}

	C_XRefScene* pNewXRefScene = 0;
	for (U32 iScene = 0; iScene < (U32)m_pActiveXRefScenes->Size(); iScene++)
	{
		ActiveXRefEntry* pActiveXRefEntry = &(*m_pActiveXRefScenes)[iScene];
		if (pActiveXRefEntry->pXRefScene->GetFileName() == fileName)
		{
			//We've already added this scene before globally.
			pNewXRefScene = pActiveXRefEntry->pXRefScene;
			//Mark this scene as using the active entry.
			pActiveXRefEntry->usageList.PushBack(this);
		}
	}
	//We need to add a new scene.
	if (!pNewXRefScene)
	{
		//We've not encountered this scene before, add it.
		pNewXRefScene = new C_XRefScene(m_pRootScene);
		status = pNewXRefScene->SetSourceScene(fileName, crc);
		if (!status) return -1;

		//Add to global scene list - this deallocates C_XRefScene memory.
		AddActiveXRefScene(pNewXRefScene, status);
		if (!status) return -1;
	}

	//Add this x-ref to our local scene list.
	m_xRefScenes.PushBack(pNewXRefScene);

	//Return the local index of the new scene entry.
	status = C_Status::E_Status_Success;
	return m_xRefScenes.Size()-1;
}

S32 C_Scene::AddActiveXRefScene(C_XRefScene* pNewXRefScene, C_Status& status)
{
	for (U32 iScene = 0; iScene < (U32)m_pActiveXRefScenes->Size(); iScene++)
	{
		C_XRefScene* pXRefScene = (*m_pActiveXRefScenes)[iScene].pXRefScene;
		if (pXRefScene->GetFileName() == pNewXRefScene->GetFileName())
		{
			//We've already added this scene before, increase its reference count and
			//return its index.
			ActiveXRefEntry& entry = (*m_pActiveXRefScenes)[iScene];
			entry.usageList.PushBack(this);
			status = C_Status::E_Status_Success;
			return iScene;
		}
	}

	//We've not encountered this scene before, add it.
	ActiveXRefEntry entry;
	entry.pXRefScene = pNewXRefScene;
	entry.usageList.PushBack(this);
	m_pActiveXRefScenes->PushBack(entry);
	return m_pActiveXRefScenes->Size()-1;
}

void C_Scene::DeleteActiveXRefScene(C_XRefScene* pNewXRefScene)
{
	for (U32 iScene = 0; iScene < (U32)m_pActiveXRefScenes->Size(); iScene++)
	{
		C_XRefScene* pXRefScene = (*m_pActiveXRefScenes)[iScene].pXRefScene;
		if (pXRefScene == pNewXRefScene)
		{
			//We've already added this scene before, increase its reference count and
			//return its index.
			ActiveXRefEntry& entry = (*m_pActiveXRefScenes)[iScene];
			for (U32 jUse = 0; jUse < (U32)entry.usageList.Size(); jUse++)
			{
				if (entry.usageList[jUse] == this)
				{
					entry.usageList.EraseByIndex(jUse);
					break;
				}
			}

			if (entry.usageList.Size() == 0)
			{
				//This x-ref scene information is no longer required.
				delete entry.pXRefScene;
				m_pActiveXRefScenes->EraseByIndex(iScene);
			}
			//We're done here.
			return;
		}
	}

	//We didn't find the scene to erase - throw debug assert.
	AFTK_ASSERT(!("We didn't find the scene to erase!"));
}

C_Status C_Scene::OnLoadXRefScene(C_XRefScene* pXRefScene)
{
	C_Status status;

	std::string loadedXRefFileNameLower = pXRefScene->GetFileName();
	std::transform(
		loadedXRefFileNameLower.begin(), 
		loadedXRefFileNameLower.end(), 
		loadedXRefFileNameLower.begin(), 
		tolower);

	C_SceneNodeIterator it(this);
	for (; it; ++it)
	{
		C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetType() == E_ObjType_SceneXRef)
		{
			C_SceneXRefNode* pSceneXRefNode = (C_SceneXRefNode*)pSceneNode;
			std::string xRefNodeFileNameLower = pSceneXRefNode->GetSourceSceneFileName();
			std::transform(
				xRefNodeFileNameLower.begin(), 
				xRefNodeFileNameLower.end(), 
				xRefNodeFileNameLower.begin(), 
				tolower);

			//Perform case insensitive string comparison.
			if (loadedXRefFileNameLower == xRefNodeFileNameLower)
			{
				status = pSceneXRefNode->SetSourceScenePtr(pXRefScene->GetScenePtr());
				if (!status) return status;
			}
		}
	}

	return C_Status::E_Status_Success;
}

void C_Scene::OnUnloadXRefScene(C_XRefScene* pXRefScene)
{
	std::string loadedXRefFileNameLower = pXRefScene->GetFileName();
	std::transform(
		loadedXRefFileNameLower.begin(), 
		loadedXRefFileNameLower.end(), 
		loadedXRefFileNameLower.begin(), 
		tolower);

	C_SceneNodeIterator it(this);
	for (; it; ++it)
	{
		C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetType() == E_ObjType_SceneXRef)
		{
			C_SceneXRefNode* pSceneXRefNode = (C_SceneXRefNode*)pSceneNode;
			std::string xRefNodeFileNameLower = pSceneXRefNode->GetSourceSceneFileName();
			std::transform(
				xRefNodeFileNameLower.begin(), 
				xRefNodeFileNameLower.end(), 
				xRefNodeFileNameLower.begin(), 
				tolower);

			//Perform case insensitive string comparison.
			if (loadedXRefFileNameLower == xRefNodeFileNameLower)
			{
				pSceneXRefNode->SetSourceScenePtr(0);
			}
		}
	}
}

void C_Scene::DeleteXRefScene(const std::string& fileName)
{
	AFTK_ASSERT(m_pActiveXRefScenes);
	for (U32 i = 0; i < (U32)m_xRefScenes.Size(); i++)
	{
		if (m_xRefScenes[i]->GetFileName() == fileName)
		{
			//Disassociate scene members from the scene to be deleted.
			//todo
			Ut::UtVector<Ut::DynamicVector< C_SceneNode* > > sceneNodeList;
			C_SceneNodeIterator it(this);
			while(it)
			{
				sceneNodeList.PushBack(it);
				++it;
			}

			for (U32 jNode = 0; jNode < (U32)sceneNodeList.Size(); jNode++)
			{
				if (sceneNodeList[jNode]->GetType() == E_ObjType_SceneXRef)
				{
					//Delete this x-ref node and reparent its children if it uses the xref that
					//we are removing.
					C_SceneXRefNode* pXRefNode = (C_SceneXRefNode*)sceneNodeList[jNode];
					if (pXRefNode->GetSourceSceneFileName() == fileName)
					{
						DeleteSceneNodeAndReparentChildren(pXRefNode, true);
					}
				}
			}

			//Decrease global scene reference count.
			DeleteActiveXRefScene(m_xRefScenes[i]);

			//Remove from local scene node array.
			m_xRefScenes.EraseByIndex(i);
		}
	}
}

void C_Scene::DeleteXRefScene(C_XRefScene* pXRefScene)
{
	AFTK_ASSERT(m_pActiveXRefScenes);
	for (U32 i = 0; i < (U32)m_xRefScenes.Size(); i++)
	{
		if (m_xRefScenes[i] == pXRefScene)
		{
			//Disassociate scene members from the scene to be deleted.
			//todo
			Ut::UtVector<Ut::DynamicVector< C_SceneNode* > > sceneNodeList;
			C_SceneNodeIterator it(this);
			while(it)
			{
				sceneNodeList.PushBack(it);
				++it;
			}

			for (U32 jNode = 0; jNode < (U32)sceneNodeList.Size(); jNode++)
			{
				if (sceneNodeList[jNode]->GetType() == E_ObjType_SceneXRef)
				{
					//Delete this x-ref node and reparent its children if it uses the xref that
					//we are removing.
					C_SceneXRefNode* pXRefNode = (C_SceneXRefNode*)sceneNodeList[jNode];
					if (pXRefNode->GetSourceScenePtr() == pXRefScene->GetScenePtr())
					{
						DeleteSceneNodeAndReparentChildren(pXRefNode, true);
					}
				}
			}

			//Decrease global scene reference count.
			DeleteActiveXRefScene(m_xRefScenes[i]);

			//Remove from local scene node array.
			m_xRefScenes.EraseByIndex(i);
		}
	}
}

U32 C_Scene::GetNumXRefScenes() const
{
	return (U32)m_xRefScenes.Size();
}

U32 C_Scene::GetNumActiveXRefScenes() const
{
	AFTK_ASSERT(m_pRootScene);
	if (m_pRootScene == this)
	{
		return (U32)m_pActiveXRefScenes->Size();
	}

	return m_pRootScene->GetNumActiveXRefScenes();
}

C_Scene::ActiveXRefEntry* C_Scene::GetActiveXRefEntryPtr(U32 instance)
{
	//This way we avoid writing the above function twice.
	const C_Scene::ActiveXRefEntry* pXRefScene = 
		((const C_Scene*)this)->GetActiveXRefEntryPtr(instance);
	return const_cast<C_Scene::ActiveXRefEntry*>(pXRefScene);
}

const C_Scene::ActiveXRefEntry* C_Scene::GetActiveXRefEntryPtr(U32 instance) const
{
	AFTK_ASSERT(m_pRootScene);
	if (m_pRootScene == this)
	{
		return &(*m_pActiveXRefScenes)[instance];
	}

	return m_pRootScene->GetActiveXRefEntryPtr(instance);
}

C_XRefScene* C_Scene::GetXRefScenePtr(U32 instance)
{
	return m_xRefScenes[instance];
}

const C_XRefScene* C_Scene::GetXRefScenePtr(U32 instance) const
{
	return m_xRefScenes[instance];
}

C_Scene* C_Scene::GetXRefRootScenePtr()
{
	AFTK_ASSERT(m_pRootScene);
	return m_pRootScene;
}

const C_Scene* C_Scene::GetXRefRootScenePtr() const
{
	AFTK_ASSERT(m_pRootScene);
	return m_pRootScene;
}
/*
C_Status C_Scene::LoadAllGlobalXRefScenes()
{
	//This way we avoid writing the above function twice.
	AFTK_ASSERT(m_pRootScene);
	if (m_pRootScene == this)
	{
		for (U32 iXRefScene = 0; iXRefScene < (U32)m_pActiveXRefScenes->Size(); iXRefScene++)
		{
			C_Status status = (*m_pActiveXRefScenes)[iXRefScene].pXRefScene->Load();
			if (!status) return status;
		}
		return C_Status::E_Status_Success;
	}

	return m_pRootScene->LoadAllGlobalXRefScenes();
}

void C_Scene::UnloadAllGlobalXRefScenes()
{
	//This way we avoid writing the above function twice.
	AFTK_ASSERT(m_pRootScene);
	if (m_pRootScene == this)
	{
		for (U32 iXRefScene = 0; iXRefScene < (U32)m_pActiveXRefScenes->Size(); iXRefScene++)
		{
			(*m_pActiveXRefScenes)[iXRefScene].pXRefScene->Unload();
		}
		return;
	}

	m_pRootScene->UnloadAllGlobalXRefScenes();
}
*/
C_Status C_Scene::LoadAllXRefScenes()
{
	for (U32 iXRefScene = 0; iXRefScene < (U32)m_xRefScenes.Size(); iXRefScene++)
	{
		m_xRefScenes[iXRefScene]->Load();
	}
	return C_Status::E_Status_Success;
}

void C_Scene::UnloadAllXRefScenes()
{
	for (U32 iXRefScene = 0; iXRefScene < (U32)m_xRefScenes.Size(); iXRefScene++)
	{
		m_xRefScenes[iXRefScene]->Unload();
	}
}

/***************************** End X-Ref related functions *************************/

U32 C_Scene::ValidateBlindDataReferenceCounts() const
{
	U32 errorCount = 0;

	U32 nBlindData = GetNumBlindDataNodes();
	if (nBlindData == 0)
		return 0;

	Ut::UtVector<Ut::DynamicVector<U32> >refCountList;

	refCountList.Resize(m_blindDataNodeManager.GetCapacity());

	memset(&refCountList[0], 0, sizeof(U32)*refCountList.Size());

	C_SceneNodeConstIterator it(this);
	while(it)
	{
		const C_SceneNode* pNode = it;
		{
			const C_BlindDataResource* pRes = pNode->GetBlindDataResourcePtr();
			if (pRes)
			{
				refCountList[pRes->GetIndex()]++;
			}
		}
		if (
			(pNode->GetType() == E_ObjType_SceneMesh) ||
			(pNode->GetType() == E_ObjType_SceneMeshRemappedInstance) ||
			(pNode->GetType() == E_ObjType_SceneMeshBonedInstance))
		{
			C_SceneMeshNode* pMesh = (C_SceneMeshNode*)pNode;
			for (U16 i = 0; i < pMesh->GetNumPolygonBlindDataMappings(); i++)
			{
				const C_BlindDataResource* pRes = pMesh->GetPolygonBlindDataMappingPtr(i);
				if (pRes)
				{
					refCountList[pRes->GetIndex()]++;
				}
			}
		}

		++it;
	}

	for (U32 i = 0; i < (U32)refCountList.Size(); i++)
	{
		const C_BlindDataResource* pRes = GetIthBlindDataResourcePtr(i);
		if (pRes)
		{
			U32 currentRefCount = refCountList[i];
			AFTK_ASSERT(pRes->GetReferenceCount() == currentRefCount);
			if (pRes->GetReferenceCount() != refCountList[i])
			{
				AFTK_DO_BEGIN
					<< "Blind-data validation problem " << "["
					<< pRes->GetContent().GetName() << "] real ref count = " 
					<< refCountList[i] << " manager ref count " << pRes->GetReferenceCount() << "\n" AFTK_DO_INFO_END;
				errorCount++;
			}
		}
	}

	return errorCount;
}

U32 C_Scene::ValidateMaterialReferenceCounts() const
{
	U32 errorCount = 0;

	Ut::UtVector<Ut::DynamicVector<U32> >refCountList;
	for (U32 i = 0; i < GetNumMaterialNodes(); i++)
	{
		refCountList.PushBack(0);
	}

	C_SceneNodeConstIterator it(this);
	while(it)
	{
		const C_SceneNode* pNode = it;
		switch (pNode->GetType())
		{
			case E_ObjType_SceneMesh:
			case E_ObjType_SceneMeshRemappedInstance:
			case E_ObjType_SceneMeshBonedInstance:
			{
				C_SceneMeshRemappableNode* pMesh = (C_SceneMeshRemappableNode*)pNode;

				for (U16 i = 0; i < pMesh->GetNumPolygonMaterialMappings(); i++)
				{
					const C_MaterialResource* pRes = pMesh->GetPolygonMaterialMappingPtr(i);
					if (pRes)
					{
						refCountList[pRes->GetIndex()]++;
					}
				}
			}
		}

		++it;
	}

	for (U32 i = 0; i < (U32)refCountList.Size(); i++)
	{
		const C_MaterialResource* pRes = GetMaterialResourcePtr(i);
		U32 currentRefCount = refCountList[i];
		AFTK_ASSERT(pRes->GetReferenceCount() == refCountList[i]);
		if (pRes->GetReferenceCount() != refCountList[i])
		{
			errorCount++;
			AFTK_DO_BEGIN
				<< "Blind-data validation problem " << "["
				<< pRes->GetContent().GetName() << "] real ref count = " 
				<< refCountList[i] << " manager ref count " << pRes->GetReferenceCount() << "\n" AFTK_DO_INFO_END;
		}
	}

	return errorCount;
}

void C_Scene::SyncMaterials(
	const C_MaterialResource* pMaterialRes,
	C_ShaderNode* pShaderToReplace, 
	C_ShaderImageNode* pShaderToUse)
{
	if (pShaderToReplace->GetType() != E_ObjType_ShaderImage)
		return;

	// Iterate over entire scene to find all remappableMeshes
	C_SceneNodeIterator iter(this);
	while (iter) 
	{
		C_SceneNode* pNode = iter;

		// Check the node's type
		if (pNode->HasType(E_ObjType_SceneMeshRemappable)) {

			C_SceneMeshRemappableNode* pRemappableMesh = (C_SceneMeshRemappableNode*) pNode;

			// Search the mesh's material map for the current material
			Ut::UtMap<const C_MaterialResource*, C_SceneMeshRemappableNode::UVLinkageList>::Iterator iter;
			iter = pRemappableMesh->m_uvLinkInfo.Find(pMaterialRes);
			
			// If the mesh does not use the given material, continue
			// checking the rest of the scene
			if (iter != pRemappableMesh->m_uvLinkInfo.End()) {

				C_SceneMeshRemappableNode::UVLinkageList uvLinkageListToCheck = iter->GetValue();

				for (U32 i = 0; i < (U32)uvLinkageListToCheck.Size(); i ++) {

					if (uvLinkageListToCheck[i].pImageNode == pShaderToReplace) {

						if (!pShaderToUse)
							uvLinkageListToCheck.Erase(uvLinkageListToCheck.Begin()+i);
						else
							uvLinkageListToCheck[i].pImageNode = pShaderToUse;
					}
				}
			}
		}
		iter++;
	}
}

void C_Scene::OneTimeInit(C_Scene* pRootScene)
{
	m_pRootSceneNode = 0;
	m_nextSceneNodeUniqueId = 0;

	m_origin.clear();
	m_pSceneBlindDataResource = 0;

	m_frameRate = 0;
	m_startTime = 0;
	m_endTime = 0;
	m_currentTime = 0;

	//x-ref stuff
	m_pRootScene = pRootScene;
	if (!pRootScene)
	{
		//We are the root scene.
		m_pRootScene = this;
	}

	if (m_pRootScene == this)
	{
		//We are the root x-ref scene, allocate x-ref scene storage.
		m_pActiveXRefScenes = new Ut::UtVector<Ut::DynamicVector<ActiveXRefEntry> >();
	}
	else
	{
		m_pActiveXRefScenes = m_pRootScene->m_pActiveXRefScenes;
	}
}

C_Status C_Scene::ReadFile(
	const std::string &fileName, 
	E_FileType format, 
	const char* pOptions,
	I_Progress* pProgressCallback)
{
	switch (format)
	{
		case E_FileType_A3d:
		{
			C_NativeFtk nativeFtk;
			return nativeFtk.Read(fileName.c_str(), (C_Scene*)this, pOptions, pProgressCallback);
		}
		case E_FileType_Xsi:
		{
			#ifdef AFTK_XSI
			XsiIo::C_Main xsiIoMain;
			return (xsiIoMain.Read(fileName, (C_Scene*)this, pOptions, pProgressCallback));
			#else
			return C_Status(C_Status::E_Status_NotImplemented, "XSI file format is not supported when using 64-bit code.");
			#endif
		}
		case E_FileType_Mb:
		case E_FileType_Ma:
		{
			#ifdef AFTK_MAYA_IO
			C_Status status;
			{
				MayaIo::C_Main mayaIoMain;
				status = mayaIoMain.Read(fileName, (C_Scene*)this, pOptions, pProgressCallback);
			}
			return status;
			#else
			return C_Status(C_Status::E_Status_NotImplemented, "This version of the  was not built with Maya support.");
			#endif
		}
		case E_FileType_Fbx:
		{
			#ifdef AFTK_FBX
			FbxIo::C_Main fbxIoMain;
			return (fbxIoMain.Read(fileName, (C_Scene*)this, pOptions, pProgressCallback));
			#else
			return C_Status(C_Status::E_Status_NotImplemented, "FBX file format is not supported.");
			#endif
		}
		default:
		{
			return C_Status::E_Status_Failure;
		}
	}
}

C_Status C_Scene::WriteFile(
	const std::string &fileName, 
	E_FileType format, 
	U32 version, 
	const char* pOptions,
	I_Progress* pProgressCallback) const
{
	switch (format)
	{
		case E_FileType_A3d:
		{
			C_NativeFtk nativeFtk;
			return nativeFtk.Write(fileName, version, (C_Scene*)this, pOptions, pProgressCallback);
		}
		case E_FileType_Mb:
		case E_FileType_Ma:
		{
			#ifdef AFTK_MAYA_IO
			MayaIo::C_Main mayaIoMain;
			return mayaIoMain.Write(fileName, version, (C_Scene*)this, pOptions, pProgressCallback);
			#else
			return C_Status(C_Status::E_Status_NotImplemented, "This version of the  was not built with Maya support.");
			#endif
		}
		case E_FileType_Xsi:
		{
			#ifdef AFTK_XSI
			XsiIo::C_Main xsiIoMain;
			return xsiIoMain.Write(fileName, version, (C_Scene*)this, pOptions, pProgressCallback);
			#endif
		}
		default:
		{
			C_Status status;
			status.Set(C_Status::E_Status_Failure, "Unknown file type specified.", 1);
			return status;
		}
	}
}

U32 C_Scene::ValidateLayers() const
{
	C_SceneNodeConstIterator it(this);
	U32 errorCount = 0;

	while (it)
	{
		const C_SceneNode* pSceneNode = it;
		for (U32 iLayer = 0; iLayer < pSceneNode->GetNumLayers(); iLayer++)
		{
			bool found = false;
			const C_LayerNode* pLayer = pSceneNode->GetLayerPtr(iLayer);
			for (U32 i = 0; i < (U32)pLayer->GetNumMembers(); i++)
			{
				if (pLayer->GetMemberPtr(i) == pSceneNode)
				{
					found = true;
					break;
				}
			}
			if (!found)
			{
				AFTK_DO_BEGIN 
					<< "Corrupt layer found. C_Node " 
					<< pSceneNode->GetName() 
					<< " layer pointer does not match the layer containing it.\n" AFTK_DO_DEBUG_END;
				AFTK_ASSERT(false);
				errorCount++;
			}
		}
		++it;
	}

	for (U32 i = 0; i < (U32)m_layers.Size(); i++)
	{
		for (U32 jMember = 0; jMember < m_layers[i]->GetNumMembers(); jMember++)
		{
			const C_SceneNode* pSceneNode = m_layers[i]->GetMemberPtr(jMember);
			//Check that this node is a member of the required layer.
			bool foundLayer = false;
			for (U32 kLayer = 0; kLayer < pSceneNode->GetNumLayers(); kLayer++)
			{
				if (pSceneNode->GetLayerPtr(kLayer) == m_layers[i])
					foundLayer = true;
			}

			if (!foundLayer)
			{
				AFTK_DO_BEGIN 
					<< "Corrupt layer found. C_Node " 
					<< pSceneNode->GetName() 
					<< " layer pointer does not match the layer containing it.\n" AFTK_DO_DEBUG_END;
				AFTK_ASSERT(false);
				errorCount++;
			}
		}
	}

	return errorCount;
}

C_SceneNode* C_Scene::GetSceneNodePtrByFullPath(const std::string& path, const C_SceneNode* pRoot)
{
	//This way we avoid writing the above function twice.
	const C_SceneNode* pSceneNode = 
		((const C_Scene*)this)->GetSceneNodePtrByFullPath(path, pRoot);
	return const_cast<C_SceneNode*>(pSceneNode);
}

const C_SceneNode* C_Scene::GetSceneNodePtrByFullPath(const std::string& path, const C_SceneNode* pRoot) const
{
	if (path.length() == 0)
	{
		return 0;
	}

	//Get first token
  	char delimiter[] = "|";
	
	std::string referencedPathFragment;
	char* tempToken;
	//Get the first token:
	char* pBuffer = new char[path.length()+1];
	strcpy(pBuffer, path.c_str());

	tempToken = strtok(pBuffer, delimiter);
	std::string token = tempToken;
	U32 charsToCopy = (U32)path.length() - (U32)token.length();
	U32 offset = (U32)token.length();
	if (path[0] == '|')
	{
		offset++;
		charsToCopy--;
	}
	std::string newPath;
	if (charsToCopy > 0)
	{
		newPath = path.substr(offset, charsToCopy);
	}
	delete[] pBuffer;

	//We're getting the full path, therefore we must be the first token.
	if (pRoot->GetName() == token)
	{
		if (newPath.length() == 0)
		{
			return pRoot;
		}
	}
	else
	{
		return 0;
	}

	//Get child token to search for.
	for (U32 iChild = 0; iChild < pRoot->GetNumChildren(); iChild++)
	{
		const C_SceneNode* pChild = pRoot->GetChildPtr(iChild);
		const C_SceneNode* pResult = GetSceneNodePtrByFullPath(newPath, pChild);
		if (pResult != 0)
		{
			return pResult;
		}
	}
	
	return 0;
}

C_SceneNode* C_Scene::GetSceneNodePtrByPartialPath(const std::string& path, const C_SceneNode* pRoot)
{
	//This way we avoid writing the above function twice.
	const C_SceneNode* pSceneNode = 
		((const C_Scene*)this)->GetSceneNodePtrByPartialPath(path, pRoot);
	return const_cast<C_SceneNode*>(pSceneNode);
}

const C_SceneNode* C_Scene::GetSceneNodePtrByPartialPath(const std::string& path, const C_SceneNode* pRoot) const
{
	if (path.length() == 0)
	{
		return 0;
	}

	//Get first token
  	char delimiter[] = "|";
	
	std::string referencedPathFragment;
	std::string firstToken;
	{
		char* token;
		//Get the first token:
		char* pBuffer = new char[path.length()+1];
		strcpy(pBuffer, path.c_str());

		token = strtok(pBuffer, delimiter);
		firstToken = token;
		delete[] pBuffer;
	}

	//Find first token in scene graph. For each case where we find it, perform a partial path search.
	C_SceneNodeConstIterator it(pRoot);
	while(it)
	{
		const C_SceneNode* pSceneNode = it;
		if (pSceneNode->GetName() == firstToken)
		{
			const C_SceneNode* pResult = GetSceneNodePtrByFullPath(path, pSceneNode);
			if (pResult)
			{
				return pResult;
			}
		}
		++it;
	}

	return 0;
}

U32 C_Scene::DeleteUnusedDgNodes(E_ObjType nodeType)
{
	//Build a list of nodes to delete.
	Ut::UtVector<Ut::DynamicVector< C_Node* > > toDelete;
	for (
		Ut::UtSet<C_Node*>::Iterator dgIt = m_unmanagedDgNodes.Begin(); 
		dgIt != m_unmanagedDgNodes.End(); 
		++dgIt)
	{
		C_Node* pNode = *dgIt;
		//If the node is of the correct type, and it's unused, then added it to our 'toDelete' list.
		if (((pNode->GetType() == nodeType) || (nodeType == Aftk::E_ObjType_Unknown)) && (!pNode->IsUsed()))
			toDelete.PushBack(*dgIt);
	}

	//Delete unused nodes
	for (U32 i = 0; i < (U32)toDelete.Size(); i++)
		delete toDelete[i];
	return (U32)toDelete.Size();
}

U32 C_Scene::GetNumDgNodes(E_ObjType nodeType) const
{
	if (nodeType == E_ObjType_Unknown)
		return (U32)m_unmanagedDgNodes.Size();

	//Count by type.
	U32 count = 0;
	for (
		Ut::UtSet<C_Node*>::ConstIterator dgIt = m_unmanagedDgNodes.Begin(); 
		dgIt != m_unmanagedDgNodes.End(); 
		++dgIt)
	{
		if (dgIt->GetKey()->GetType() == nodeType)
			count++;
	}
	return count;
}

const Ut::UtSet<C_Node*>& C_Scene::GetDgNodeSet() const
{
	return m_unmanagedDgNodes;
}

#endif
