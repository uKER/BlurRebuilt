#include "stdafx.h"

#include <Aftk/Aftk.h>
#include <AftkUt/AftkUtQuery.h>
#include <AftkUt/AftkUtNodeFlagParser.h>

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Aftk/AftkRefHelpers.h>

#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/SceneAftkNode.h>
#include <EditorLib/Dg/SceneAftkActorNode.h>
#include <EditorLib/Dg/SceneNodeHelpers.h>
#include <EditorLib/Dg/SceneNodeIterator.h>

#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Level/LevelManager.h>
#include <EditorLib/Level/ReferenceLevel.h>

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>
#include <EditorLib/Level/XtLevel.h>

namespace EditorLib
{

C_AftkRef::C_AftkRef(const Btl::String& filepath, bool isVisualisation)
{
	m_originalFilePath = filepath;
	m_pSceneNode = 0;
	m_isVisualisation = isVisualisation;
}

C_AftkRef::~C_AftkRef()
{
	BIZ_ASSERT(!m_pSceneNode);
}

const Btl::String& C_AftkRef::GetOriginalFilePath() const
{
	return m_originalFilePath;
}

const Btl::String& C_AftkRef::GetCacheFilePath() const
{
	return m_cacheFilePath;
}

C_SceneNode* C_AftkRef::GetSceneRootNode()
{
	return m_pSceneNode;
}

const C_SceneNode* C_AftkRef::GetSceneRootNode() const
{
	return m_pSceneNode;
}

bool C_AftkRef::IsLoaded() const
{
	return m_pSceneNode ? true : false;
}

bool C_AftkRef::IsVisualisation() const
{
	return m_isVisualisation;
}

bool C_AftkRef::IsCacheOutOfDate() const
{
	return AftkRefHelpers::IsSceneOutOfDate(m_originalFilePath, m_cacheFilePath);
}

C_Status C_AftkRef::Load() const
{
	if (IsLoaded())
		return E_Status_Success;

	C_ReferenceLevel* pLevel = C_Core::Instance().GetLevelManager()->GetReferenceLevel();
	if (!pLevel)
		return E_Status_Failure;

	//Perform Maya conversion here.
	if (!AftkRefHelpers::ConvertScene(m_originalFilePath, m_cacheFilePath))
		return E_Status_Failure;

	AftkDgMap aftkDgMap;

	Aftk::C_Scene scene;
	if (!scene.Read(m_cacheFilePath.Chars()) || !scene.GetRootSceneNodePtr())
		return E_Status_Failure;

	//Triangulate entire scene.
	if (!Aftk::C_SceneNodeVisit::Traverse(
		&scene,
		Aftk::C_MeshTriangulate::C_TriangulateVisitor()))
		return E_Status_Failure;

	//Create a root node to parent our aftk content to.
	m_pSceneNode = (C_SceneNode*)C_Core::Instance().GetDgManager().CreateNode(pLevel, E_ObjType_SceneNode);
	m_pSceneNode->GetSceneNodeFlags().canAddChildren = 1;
	
	//Parent to level root node.
	C_SceneNode* pRoot = C_Core::Instance().GetLevelManager()->GetReferenceLevel()->GetRootSceneNode();
	m_pSceneNode->GetSceneNodeFlags().canReparent = 1;
	pRoot->AddChild(m_pSceneNode);
	m_pSceneNode->GetSceneNodeFlags().canReparent = 0;

	//Use the filename as the name of the root for the scene content.
	m_pSceneNode->SetName(Misc::ExtractFilename(m_originalFilePath));

	//Recurse and mirror the aftk's scene graph.
	C_SceneNode* pMirrorRoot = MirrorAftkScene(aftkDgMap, scene.GetRootSceneNodePtr());
	if (!pMirrorRoot)
		return E_Status_Failure;

	pMirrorRoot->GetSceneNodeFlags().canReparent = 1;
	m_pSceneNode->AddChild(pMirrorRoot);
	pMirrorRoot->GetSceneNodeFlags().canReparent = 0;

	//set level pointer
	m_pSceneNode->GetNodeFlags().readOnly = 1;
	m_pSceneNode->GetSceneNodeFlags().canAddChildren = 0;
	pRoot->GetNodeFlags().userDeletable = 0;

	if (!IsVisualisation())
		CreateLayers(aftkDgMap, scene);

	C_Core::Instance().GetGlobalSignals().aftkRefLoaded(
		m_originalFilePath, 
		scene, 
		*pMirrorRoot,
		aftkDgMap);

	return E_Status_Success;
}

C_Status C_AftkRef::Unload() const
{
	if (!m_pSceneNode)
		return E_Status_Success;

	//Erase our reference scene from the scene-graph.
	C_SceneNodeHelpers::DeleteSceneNodeAndChildren(m_pSceneNode, true);
	m_pSceneNode = 0;

	C_Core::Instance().GetGlobalSignals().aftkRefUnloaded(m_originalFilePath);

	//clean up any empty layers
	{
		I_LayerSet* pReferenceLayers = C_Core::Instance().GetLayerManager().GetLayerSet(C_Core::Instance().GetLevelManager()->GetReferenceLevel());

		if (!pReferenceLayers)
			return E_Status_Success;

		LayerList::ConstIterator layerIt = pReferenceLayers->GetLayerIterator();
		Ut::Vector<I_Layer*> emptyLayers;
		while (layerIt.Pointer())
		{
			if ((*layerIt)->GetType() == E_LayerType_Reference && (*layerIt)->GetNodes().Size() == 0)
				emptyLayers.PushBack(*layerIt);
			++layerIt;
		}

		for (S32 i=0; i<emptyLayers.Size(); ++i)
			pReferenceLayers->RemoveLayer(emptyLayers[i]);
	}

	return E_Status_Success;
}

C_SceneAftkNode* C_AftkRef::MirrorAftkScene(
	AftkDgMap& aftkDgMap, 
	Aftk::C_SceneNode* pAftkSceneNode) const
{
	I_Level* pLevel = C_Core::Instance().GetLevelManager()->GetReferenceLevel();
	if (!pLevel)
		return 0;

	//Look for aftk nodes that have ActorStudio properties. ActorStudio properties are custom attributes
	//which have a AS_ prefix in their name.
	Btl::Vector<Util::C_KeyValue> attrs;
	AftkRefHelpers::GetActorStudioAttrs(
		attrs,
		pAftkSceneNode);

	//Second pass, create actor node type if appropriate and copy attribute data.
	C_SceneAftkNode* pDgAftkSceneNode(0);
	if (attrs.Size() > 0)
	{
		const Util::C_KeyValue* pAsDynamic(0);
		const Util::C_KeyValue* pAsActor(0);
		if (AftkRefHelpers::GetAftkActorPlaceholderProperties(
			&pAsDynamic, &pAsActor, attrs, pLevel->GetName()))
		{
			//Create new aftk mirror node and attach it to this reference.
			pDgAftkSceneNode = 
				(C_SceneAftkNode*)C_Core::Instance().GetDgManager().CreateNode(pLevel, E_ObjType_SceneAftkActorNode);
			//Store (non globally unique) id.
			((C_SceneAftkActorNode*)pDgAftkSceneNode)->SetAftkUniqueId(pAftkSceneNode->GetUniqueId());
			//Store attrs
			((C_SceneAftkActorNode*)pDgAftkSceneNode)->GetAftkProperties() = attrs;
		}
	}

	//Create new aftk mirror node and attach it to this reference.
	if (!pDgAftkSceneNode) 
		pDgAftkSceneNode = 
			(C_SceneAftkNode*)C_Core::Instance().GetDgManager().CreateNode(
				C_Core::Instance().GetLevelManager()->GetReferenceLevel(), E_ObjType_SceneAftkNode);
	pDgAftkSceneNode->GetSceneNodeFlags().canReparent = 0;
	pDgAftkSceneNode->GetSceneNodeFlags().canAddChildren = 1;
	pDgAftkSceneNode->SetRef(*this);

	//Copy Aftk scene node name.
	pDgAftkSceneNode->SetName(pAftkSceneNode->GetName().c_str());

	pDgAftkSceneNode->SetLocalMatrix(pAftkSceneNode->GetLocalMatrix());

	//Add to our node mapping.
	aftkDgMap[pAftkSceneNode] = pDgAftkSceneNode;

	//Mirror our children.
	for (U32 i = 0; i < pAftkSceneNode->GetNumChildren(); i++)
	{
		Aftk::C_SceneNode* pAftkChild = pAftkSceneNode->GetChildPtr(i);
		//Recurse and mirror the aftk's scene graph.
		C_SceneAftkNode* pResult = MirrorAftkScene(aftkDgMap, pAftkChild);
		if (!pResult)
		{
			LOGERROR(C_Core::Instance().GetLogger(), "Failed to mirror aftk scene node sub-tree from %s", pAftkChild->GetName().c_str());
			continue;
		}

		//Attach to parent.
		pResult->GetSceneNodeFlags().canReparent = 1;
		pDgAftkSceneNode->AddChild(pResult);
		pResult->GetSceneNodeFlags().canReparent = 0;
	}

	pDgAftkSceneNode->GetSceneNodeFlags().canAddChildren = 0;
	pDgAftkSceneNode->GetNodeFlags().userDeletable = 0;

	//Nodes are exportable by default.
	if (AftkRefHelpers::IsToBeHiddenPermanently(pAftkSceneNode))
		pDgAftkSceneNode->GetNodeFlags().hidePermanently = 1;
	//Flag base instances.
	if (AftkUt::Query::IsBaseInstance(pAftkSceneNode))
		pDgAftkSceneNode->GetNodeFlags().baseInstance = 1;

	pDgAftkSceneNode->GetSceneNodeFlags().canAddChildren = 0;
	return pDgAftkSceneNode;
}

bool C_AftkRef::CreateLayers(const AftkDgMap &aftkDgMap, const Aftk::C_Scene &scene) const
{
	I_Level *pLevel = C_Core::Instance().GetLevelManager()->GetReferenceLevel();

	I_LayerSet* layerSet = C_Core::Instance().GetLayerManager().GetLayerSet(pLevel);

	int numLayers = scene.GetNumLayers();
	
	for (U32 i=0; i<scene.GetNumLayers(); ++i)
	{
		const Aftk::C_LayerNode *pLayerNode = scene.GetLayerPtr(i);
		I_Layer *pLayer = 0;

		{
			//try to merge with existing layer...
			LayerList::ConstIterator layerIt = layerSet->GetLayerIterator();
			while (layerIt.Pointer() && !pLayer)
			{
				if ((*layerIt)->GetType() == E_LayerType_Reference && pLayerNode->GetName() ==(*layerIt)->GetName().Chars())
					pLayer = (*layerIt);
				++layerIt;
			}
		}

		if (!pLayer)
		{
			pLayer = new C_RefLayer(layerSet, pLevel);
			layerSet->AddLayer(pLayer);
			pLayer->SetName(pLayerNode->GetName().c_str());
		}

		assert(pLayer);

		for (U32 j=0; j<pLayerNode->GetNumMembers(); ++j)
		{
			AftkDgMap::ConstIterator nodeIt = aftkDgMap.Find(const_cast<Aftk::C_SceneNode*>(pLayerNode->GetMemberPtr(j)));
			if (nodeIt != aftkDgMap.End())
			{
				C_Core::Instance().GetLayerManager().AddNodeToLayer(pLayer, nodeIt->GetValue());
			}
		}
	}
	
	return true;
}


}
