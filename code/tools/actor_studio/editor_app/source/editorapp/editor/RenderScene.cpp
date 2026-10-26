#include "stdafx.h"

#include <algorithm>

#include <EditorApp/Editor/RenderScene.h>
#include <EditorApp/Editor/RenderCore.h>
#include <EditorApp/Editor/RenderPipeline.h>
#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/ActorScene.h>
#include <EditorApp/Editor/ActorSceneNode.h>
#include <EditorApp/Editor/AftkRefScene.h>
#include <EditorApp/Editor/Primitives.h>
#include <EditorApp/Editor/Manipulators/Manipulator.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/AftkSceneRenderer.h>
#include <EditorApp/Editor/SimpleRenderer.h>


#include <Editorlib/Core.h>
#include <Editorlib/GlobalSignals.h>
#include <EditorLib/Level/ILevel.h>
#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LayerManager.h>
#include <EditorLib/Misc/Definitions.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Misc/Selection.h>
#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Xt/XtVisualisation.h>
#include <EditorLib/Collision/Ray.h>
#include <EditorLib/Collision/CollisionStructs.h>
#include <EditorLib/Collision/CollisionResult.h>
#include <EditorLib/Collision/CollisionManager.h>

namespace Editor
{
#define VISIBILITY_NODE			0x1
#define VISIBILITY_LAYER		0x2
#define VISIBILITY_SELECTION	0x4
#define VISIBILITY_MISC			0x8

C_RenderScene::C_Slots::C_Slots(C_RenderScene *pRenderScene)
: m_pRenderScene(pRenderScene)
{
}

void C_RenderScene::C_Slots::Connect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelOpening.connect(this, &C_RenderScene::C_Slots::OnLevelOpening);
	s.levelOpened.connect(this, &C_RenderScene::C_Slots::OnLevelOpened);
	s.levelClosing.connect(this, &C_RenderScene::C_Slots::OnLevelClosing);
	s.levelClosed.connect(this, &C_RenderScene::C_Slots::OnLevelClosed);

	s.coreSelectionChanged.connect(this, &C_RenderScene::C_Slots::OnSelectionChangedSlot);

	s.dgNodeDestroy.connect(this, &C_RenderScene::C_Slots::OnDgNodeDestroySlot);
	s.dgNodeUpdated.connect(this, &C_RenderScene::C_Slots::OnDgNodeUpdatedSlot);

	s.aftkRefLoaded.connect(this, &C_RenderScene::C_Slots::OnAftkRefLoadedSlot);
	s.aftkRefUnloaded.connect(this, &C_RenderScene::C_Slots::OnAftkRefUnloadedSlot);

	s.coreEnableViews.connect(this, &C_RenderScene::C_Slots::OnCoreEnableViews);
	s.coreDirtyViews.connect(this, &C_RenderScene::C_Slots::OnCoreDirtyViews);
}

void C_RenderScene::C_Slots::Disconnect()
{
	EditorLib::C_GlobalSignals& s = EditorLib::C_Core::Instance().GetGlobalSignals();

	s.levelOpening.disconnect(this);
	s.levelOpened.disconnect(this);
	s.levelClosing.disconnect(this);
	s.levelClosed.disconnect(this);

	s.coreSelectionChanged.disconnect(this);

	s.dgNodeDestroy.disconnect(this);
	s.dgNodeUpdated.disconnect(this);

	s.aftkRefLoaded.disconnect(this);
	s.aftkRefUnloaded.disconnect(this);

	s.coreEnableViews.disconnect(this);
	s.coreDirtyViews.disconnect(this);
}

void C_RenderScene::C_Slots::OnLevelOpening(const EditorLib::I_Level& level)
{
	m_pRenderScene->OnLevelOpening(level);
}

void C_RenderScene::C_Slots::OnLevelOpened(const EditorLib::I_Level& level)
{
	m_pRenderScene->OnLevelOpened(level);
}

void C_RenderScene::C_Slots::OnLevelClosing(const EditorLib::I_Level& level)
{
	m_pRenderScene->OnLevelClosing(level);
}

void C_RenderScene::C_Slots::OnLevelClosed(const EditorLib::I_Level& level)
{
	m_pRenderScene->OnLevelClosed(level);
}

void C_RenderScene::C_Slots::OnSelectionChangedSlot()
{
	m_pRenderScene->OnSelectionChanged();
}

void C_RenderScene::C_Slots::OnDgNodeDestroySlot(const EditorLib::I_DgNode& dgNode)
{
	m_pRenderScene->OnDgNodeDestroy(dgNode);
}

void C_RenderScene::C_Slots::OnDgNodeUpdatedSlot(const EditorLib::I_DgNode& dgNode, const EditorLib::C_UpdateHintCollection* pHc)
{
	m_pRenderScene->OnDgNodeResolved(dgNode);
	m_pRenderScene->OnDgNodeUpdated(dgNode);
}
			
void C_RenderScene::C_Slots::OnAftkRefLoadedSlot(const Btl::String& originalFilePath, const Aftk::C_Scene& aftkScene, const EditorLib::C_SceneNode& sceneNode, const EditorLib::AftkDgMap& aftkDgMap)
{
	m_pRenderScene->OnAftkRefLoaded(originalFilePath, aftkScene, sceneNode, aftkDgMap);
}

void C_RenderScene::C_Slots::OnAftkRefUnloadedSlot(const Btl::String& originalFilePath)
{
	m_pRenderScene->OnAftkRefUnloaded(originalFilePath);
}

void C_RenderScene::C_Slots::OnCoreEnableViews(bool enable)
{
	m_pRenderScene->OnCoreEnableViews(enable);
}

void C_RenderScene::C_Slots::OnCoreDirtyViews(EditorLib::E_DirtyEventType e)
{
	m_pRenderScene->OnCoreDirtyViews(e);
}






C_RenderScene::C_RenderScene()
: m_slots(this)
{
	m_pActorScene = new C_ActorScene();
	m_pSelection = new C_DgNodeSelection();
	m_pDgNodeFactory = new C_DgNodeFactory();

	m_flags.pendingSync = false;
	m_flags.viewEnabled = false;
	m_flags.viewDirty = true;

	m_slots.Connect();
	EditorLib::C_Core::Instance().GetCollisionManager().RegisterCollidableHandler(
		Ut::Delegate<bool(const EditorLib::I_DgNode*)>(this, &C_RenderScene::IsVisible));
}


C_RenderScene::~C_RenderScene()
{
	SAFE_DELETE_PTR(m_pActorScene);
	SAFE_DELETE_PTR(m_pSelection);
	SAFE_DELETE_PTR(m_pDgNodeFactory);

	for (S32 i=0; i<m_aftkRefScenes.Size(); ++i)
		delete m_aftkRefScenes[i];

	m_aftkRefScenes.Clear();

	m_slots.Disconnect();
	EditorLib::C_Core::Instance().GetCollisionManager().UnregisterCollidableHandler(
		Ut::Delegate<bool(const EditorLib::I_DgNode*)>(this, &C_RenderScene::IsVisible));
}

C_DgNodeFactory& C_RenderScene::GetDgNodeFactory()
{
	return *m_pDgNodeFactory;
}

AftkRender::C_SceneGraphNode* C_RenderScene::GetSceneGraphNode(const EditorLib::I_DgNode* pNode) const
{
	if (!pNode)
		return 0;

	AftkRender::C_SceneGraphNode *pSgNode = 0;
	if (pNode->IsA(EditorLib::E_ObjType_SceneAftkNode))
	{
		for (S32 i=0; i<m_aftkRefScenes.Size(); ++i)
		{
			pSgNode = m_aftkRefScenes[i]->Find(pNode);
			if (pSgNode)
				return pSgNode;
		}

		return 0;
	}
	else
		return m_pActorScene->Find(pNode);
}

C_ActorScene& C_RenderScene::GetActorScene()
{
	return *m_pActorScene;
}
const C_ActorScene& C_RenderScene::GetActorScene() const
{
	return *m_pActorScene;
}

const Ut::Vector<C_AftkRefScene*>& C_RenderScene::GetAftkRefScenes() const
{
	return m_aftkRefScenes;
}

const C_DgNodeSelection& C_RenderScene::GetSelection() const
{
	return *m_pSelection;
}

const Maths::C_AxisBox& C_RenderScene::GetSelectionBounds() const
{
	//TODO: cache this instead of recomputing it indiscriminately
	m_selectionBounds.SetInvalidExtents();

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

	EditorLib::C_Selection::ItemArray::ConstIterator selectionIt = items.Begin();
	while (selectionIt != items.End())
	{
		const EditorLib::C_Selection::C_Item& item = *selectionIt;
		const EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(selectionIt->id);

		if (pNode)
		{
			AftkRender::C_SceneGraphNode *pSgNode = GetSceneGraphNode(pNode);
			if (pSgNode)
			{
				if (pSgNode->GetType() == AftkRender::E_SceneGraphNode_Custom)
				{
					m_selectionBounds.UpdateExtents(((Editor::C_DgNodeBase*)pSgNode)->GetSelectionAABBox());
				}
				else
				{
					m_selectionBounds.UpdateExtents(pSgNode->GetWorldAABBox());
				}
			}
		}
		selectionIt++;
	}
	return m_selectionBounds;
}

const Maths::C_AxisBox& C_RenderScene::GetWorldBounds() const
{
	//TODO: cache this instead of recomputing it indiscriminately
	m_worldBounds.SetInvalidExtents();
	for (S32 i=0; i<m_aftkRefScenes.Size(); ++i)
	{
		const EditorLib::C_AftkRef *pRef = EditorLib::C_Core::Instance().GetAftkRefManager().GetRef(m_aftkRefScenes[i]->GetOriginalFilePath());
		if (pRef && !pRef->IsVisualisation())
			m_worldBounds.UpdateExtents(m_aftkRefScenes[i]->GetAABBox());
	}

	m_worldBounds.UpdateExtents(m_pActorScene->GetAABBox());

	return m_worldBounds;
}

bool C_RenderScene::IsVisible(const EditorLib::I_DgNode* pNode)
{
	AftkRender::C_SceneGraphNode *pSceneNode = GetSceneGraphNode(pNode);
	if (!pSceneNode)
		return true;

	AftkRender::C_SceneGraphNode::Flags flags = pSceneNode->GetFlags();
	return flags.hidden == 0;
}

void C_RenderScene::AttachToRenderers(AftkRender::E_RendererPass pass)
{
	if (m_flags.pendingSync)
		return;

	if (m_flags.viewDirty)
	{
		UpdateLayerVisibility(0x1, VISIBILITY_LAYER);
		m_flags.viewDirty = false;
	}

	if (m_pActorScene->GetRootSceneGraphNodePtr())
		m_pActorScene->GetRootSceneGraphNodePtr()->AttachToRenderer(0, pass);
}

void C_RenderScene::SetNodeVisibility(bool visible)
{
	assert(!m_flags.pendingSync);

	DgNodeArray changelist;

	Btl::String objIdStr;
	EditorLib::C_DgNodeIterator it = EditorLib::C_Core::Instance().GetDgManager().GetDgIterator();
	while ((EditorLib::I_DgNode*)it)
	{
		if (it->IsA(EditorLib::E_ObjType_SceneNode))
		{
			AftkRender::C_SceneGraphNode *pNode = GetSceneGraphNode(it);
			if (pNode && ((pNode->GetFlags().hidden & VISIBILITY_NODE) == 0) != visible)
			{
				Btl::String tmp;
				tmp.Printf("%s,", it->GetId().AsString().Chars());
				objIdStr = objIdStr + tmp;
				changelist.PushBack(it);
			}
		}
		it++;
	}

	if (!objIdStr.Length())
		return;

	objIdStr = objIdStr(0, objIdStr.Length()-1);

	Btl::String args;
	
	if(visible)
		args = objIdStr + " -s";
	else
		args = objIdStr + " -h";
	
	//save list so we can undo
	if (changelist.Size())
	{
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("visibility", args);
		EditorLib::C_Core::Instance().DirtyViews();

		if (!visible)
			m_hiddenNodes = changelist;
	}	
}

void C_RenderScene::SetSelectionVisibility(bool visible)
{
	assert(!m_flags.pendingSync);

	DgNodeArray changelist;
	
	Btl::String objIdStr;
	const C_DgNodeSelection::ItemArray& items = m_pSelection->GetItems();
	C_DgNodeSelection::ItemArray::ConstIterator it = items.Begin();
	while (it != items.End())
	{
		AftkRender::C_SceneGraphNode *pNode = GetSceneGraphNode(it->pDgNode);
		if (pNode && ((pNode->GetFlags().hidden & VISIBILITY_NODE) == 0) != visible)
		{
			Btl::String tmp;
			tmp.Printf("%s,", it->pDgNode->GetId().AsString().Chars());
			objIdStr = objIdStr + tmp;
			changelist.PushBack(it->pDgNode);
		}
		it++;
	}

	if (!objIdStr.Length())
		return;
	
	objIdStr = objIdStr(0, objIdStr.Length()-1);

	Btl::String args;
	
	if(visible)
		args = objIdStr + " -s";
	else
		args = objIdStr + " -h";
	
	//save list so we can undo
	if (changelist.Size())
	{
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("visibility", args);
		EditorLib::C_Core::Instance().DirtyViews();

		if (!visible)
			m_hiddenNodes = changelist;
	}
}

void C_RenderScene::SetInverseSelectionVisibility(bool visible)
{
	assert(!m_flags.pendingSync);

	DgNodeArray changelist;

	Btl::String objIdStr;
	EditorLib::C_DgNodeIterator it = EditorLib::C_Core::Instance().GetDgManager().GetDgIterator();
	while ((EditorLib::I_DgNode*)it)
	{
		if (it->IsA(EditorLib::E_ObjType_SceneNode) && !m_pSelection->Find((EditorLib::I_DgNode*)it))
		{
			AftkRender::C_SceneGraphNode *pNode = GetSceneGraphNode(it);
			if (pNode && ((pNode->GetFlags().hidden & VISIBILITY_NODE) == 0) != visible)
			{
				Btl::String tmp;
				tmp.Printf("%s,", it->GetId().AsString().Chars());
				objIdStr = objIdStr + tmp;
				changelist.PushBack(it);
			}
		}
		it++;
	}

	if (!objIdStr.Length())
		return;

	objIdStr = objIdStr(0, objIdStr.Length()-1);

	Btl::String args;
	
	if(visible)
		args = objIdStr + " -s";
	else
		args = objIdStr + " -h";
	
	//save list so we can undo
	if (changelist.Size())
	{
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("visibility", args);
		EditorLib::C_Core::Instance().DirtyViews();

		if (!visible)
			m_hiddenNodes = changelist;
	}
}

void C_RenderScene::ShowPreviouslyHiddenNodes()
{
	assert(!m_flags.pendingSync);

	if(m_hiddenNodes.Size() == 0)
		return;

	Btl::String objIdStr;
	for (S32 i=0; i<m_hiddenNodes.Size(); ++i)
	{
		AftkRender::C_SceneGraphNode *pNode = GetSceneGraphNode(m_hiddenNodes[i]);
		if (pNode)
		{
			Btl::String tmp;
			tmp.Printf("%s,", m_hiddenNodes[i]->GetId().AsString().Chars());
			objIdStr = objIdStr + tmp;
		}
	}

	if (!objIdStr.Length())
		return;

	objIdStr = objIdStr(0, objIdStr.Length()-1);

	Btl::String args;
	args = objIdStr + " -s";
	
	if (m_hiddenNodes.Size())
	{
		EditorLib::C_Core::Instance().GetCommandProcessor().Execute("visibility", args);
		EditorLib::C_Core::Instance().DirtyViews();
	}

	m_hiddenNodes.Clear();
}

void C_RenderScene::HideUnselectableNodes()
{
	UpdateLayerVisibility(0x2, VISIBILITY_SELECTION);
}

void C_RenderScene::ShowUnselectableNodes()
{
	UpdateLayerVisibility(0xffffffff, VISIBILITY_SELECTION);
}



void C_RenderScene::OnLevelOpening(const EditorLib::I_Level& level)
{
	m_flags.pendingSync = true;
}

void C_RenderScene::OnLevelOpened(const EditorLib::I_Level& level)
{
	BIZ_ASSERT(m_flags.pendingSync);
	
	//Create render actors.
	Btl::Vector<const EditorLib::I_DgNode*> nodes;
	level.BuildActorNodeArray(nodes);
	//Populate now that the level has been opened.
	for (S32 i = 0; i < nodes.Size(); ++i)
		m_pActorScene->AddActorSceneNode(*nodes[i]);

	m_pActorScene->OnNodeDeserialised(0);
	m_pActorScene->OnNodeResolved(0);
	
	m_flags.pendingSync = false;

	//force update of active manipulator
	C_RenderCore::Instance().GetManipulatorManager().Dirty();
}

void C_RenderScene::OnLevelClosing(const EditorLib::I_Level& level)
{
	m_flags.pendingSync = true;
}

void C_RenderScene::OnLevelClosed(const EditorLib::I_Level& level)
{
	BIZ_ASSERT(m_flags.pendingSync);

	BIZ_ASSERT(m_pActorScene);

	//force update of active manipulator
	C_RenderCore::Instance().GetManipulatorManager().Dirty();

	m_flags.viewDirty = true;
	m_flags.pendingSync = false;
}

void C_RenderScene::OnSelectionChanged()
{
	assert(!m_flags.pendingSync);

	m_pSelection->Clear();

	EditorLib::C_Selection selection;
	EditorLib::C_Core::Instance().GetSelection(selection);

	const EditorLib::C_Selection::ItemArray& items = selection.GetItems();

	EditorLib::C_Selection::ItemArray::ConstIterator selectionIt = items.Begin();
	while (selectionIt != items.End())
	{
		const EditorLib::C_Selection::C_Item& item = *selectionIt;
		const EditorLib::I_DgNode* pNode = EditorLib::C_Core::Instance().GetDgManager().FindNodeById(selectionIt->id);

		if (pNode)
		{
			m_pSelection->Add(const_cast<EditorLib::I_DgNode*>(pNode), item.components);
		}
		selectionIt++;
	}

	C_RenderCore::Instance().GetRenderPipeline().GetAftkSceneRenderer().OnSelectionChanged();
	C_RenderCore::Instance().GetManipulatorManager().Dirty();

	if (!m_flags.pendingSync && m_flags.viewEnabled)
		C_RenderCore::Instance().DirtyAllViewports();
	else
		m_flags.viewDirty = true;
}

void C_RenderScene::OnDgNodeDestroy(const EditorLib::I_DgNode& dgNode)
{
	//remove from hidden node cache
	for (S32 i=0; i<m_hiddenNodes.Size(); ++i)
	{
		if (m_hiddenNodes[i] == &dgNode)
		{
			m_hiddenNodes.EraseNonOrdered(i);
			break;
		}
	}

	//remove from selection cache
	m_pActorScene->RemoveActorSceneNode(dgNode);
	m_pSelection->Remove(&dgNode);

	//resolve again so that any nodes dependent on weak ptrs are updated
	m_pActorScene->OnNodeResolved(&dgNode);

	//force update of active manipulator
	C_RenderCore::Instance().GetManipulatorManager().Dirty();
	C_RenderCore::Instance().DirtyAllViewports();
}

void C_RenderScene::OnDgNodeUpdated(const EditorLib::I_DgNode& dgNode)
{
	if (m_flags.pendingSync)
		return;

	const EditorLib::I_DgNode* pSceneNode = &dgNode;
	//Pass component updates to parent node since we don't explicity replicate components in our tree.
	if (dgNode.IsA(EditorLib::E_ObjType_DgXtCompNode))
	{
		const EditorLib::C_DgXtCompNode& compNode = (const EditorLib::C_DgXtCompNode&)dgNode;
		pSceneNode = compNode.GetParent();
	}
		
	AftkRender::C_SceneGraphNode* pSgNode = m_pActorScene->Find(pSceneNode);
	if (!pSgNode || !pSgNode->IsA(AftkRender::E_SceneGraphNode_Custom))
		return;

	m_pActorScene->OnNodeDeserialised(pSceneNode);

	C_RenderCore::Instance().DirtyAllViewports();
}

void C_RenderScene::OnDgNodeResolved(const EditorLib::I_DgNode& dgNode)
{
	if (m_flags.pendingSync)
		return;

	AftkRender::C_SceneGraphNode* pSgNode = m_pActorScene->Find(&dgNode);
	if (pSgNode)
	{
		if (!pSgNode || !pSgNode->IsA(AftkRender::E_SceneGraphNode_Custom))
			return;

		//Assumption: All custom dg nodes derived from C_DgNodeBase.
		C_DgNodeBase* pNode = (C_DgNodeBase*)pSgNode;
		m_pActorScene->OnNodeResolved(&dgNode);
		pSgNode->UpdateCache();
	}
	else
	{
		pSgNode = m_pActorScene->AddActorSceneNode(dgNode);
		if (pSgNode)
		{
			if (pSgNode->IsA(AftkRender::E_SceneGraphNode_Custom))
			{
				//Assumption: All custom dg nodes derived from C_DgNodeBase.
				C_DgNodeBase* pNode = (C_DgNodeBase*)pSgNode;
					m_pActorScene->OnNodeDeserialised(&dgNode);
					m_pActorScene->OnNodeResolved(&dgNode);
			}

			//this dgNode may have been created as a result of a type change
			//sync our selection if so...
			EditorLib::C_Selection selection;
			EditorLib::C_Core::Instance().GetSelection(selection);

			if (selection.Find(dgNode.GetId()))
			{
				m_pSelection->Add(const_cast<EditorLib::I_DgNode*>(&dgNode));
				C_RenderCore::Instance().GetManipulatorManager().Dirty();
			}
		}
	}

	C_RenderCore::Instance().DirtyAllViewports();
}

void C_RenderScene::OnAftkRefLoaded(
	const Btl::String& originalFilePath,
	const Aftk::C_Scene& aftkScene,
	const EditorLib::C_SceneNode& sceneNode,
	const EditorLib::AftkDgMap& aftkDgMap)
{
	const EditorLib::C_AftkRef *pAftkRef = EditorLib::C_Core::Instance().GetAftkRefManager().GetRef(originalFilePath);
	C_AftkRefScene* pRenderScene = new C_AftkRefScene(originalFilePath.Chars(), aftkDgMap);

	if (!pRenderScene->Load(const_cast<Aftk::C_Scene*>(&aftkScene), true))
	{
		LOGERROR(g_Log, "Failed to build scene %s", originalFilePath.Chars());
		delete pRenderScene;
		return;
	}
	else
	{
		m_aftkRefScenes.PushBack(pRenderScene);

		//dont bother building mesh, or attaching the scene to the renderer as it'll never be shown anyway
		if (pAftkRef && !pAftkRef->IsVisualisation())
		{
			pRenderScene->BuildMeshClusters();
			C_RenderCore::Instance().GetRenderPipeline().GetAftkSceneRenderer().AttachSceneGraphNode(pRenderScene->GetRootSceneGraphNodePtr());
		}
	}

	//Hide base instance/visualisation nodes
	for (AftkRender::C_SceneGraphNodeIterator it(pRenderScene->GetRootSceneGraphNodePtr()); it; ++it)
	{
		AftkRender::C_SceneGraphNode* pRenderNode = it;

		EditorLib::I_DgNode* pDgNode = pRenderScene->Find(pRenderNode);
		if (!pDgNode)
			continue;

		if (pDgNode->IsAlwaysHidden() || pAftkRef->IsVisualisation())
			pRenderScene->Hide(pRenderNode, VISIBILITY_MISC);
	}

	const EditorLib::XtVisualisationDef* pDef =
		EditorLib::C_Core::Instance().GetXtVisualisationData().FindByFilePath(
			originalFilePath.Chars());

	const char* pVisId = pDef ? pDef->id.Chars() : 0;
	if (pVisId)
	{
		//Construct from guid to extract aftk unique id.
		EditorLib::XtVisMeshInfo meshInfo(pVisId);
		const Aftk::C_SceneNode* pAftkNode = aftkScene.GetRootSceneNodePtr();

		if (pAftkNode)
		{
			Ut::UtVector< Ut::DynamicVector<const Aftk::C_MaterialResource*> > materials;
			AftkRender::C_MeshResource *pMeshResource = new AftkRender::C_MeshResource;
			AftkRender::C_MeshBuilder::Flatten(pMeshResource, pAftkNode, materials);
			AftkRender::C_MeshBuilder::Merge(pMeshResource, AftkRender::E_PrimitiveType_TriList, 0xffffffff);

			Ut::Vector<Btl::String> textures;
			textures.Reserve(materials.Size());
			for (S32 j=0; j<materials.Size(); ++j)
			{
				const Aftk::C_MaterialNode& aftkMaterial = materials[j]->GetContent();
				if (aftkMaterial.GetNumShaderImageNodes())
				{
					std::string path;
					aftkMaterial.GetShaderImageNodePtr(0)->GetFilePlug().GetValue(path);
					std::transform(path.begin(), path.end(), path.begin(), tolower);
					textures.PushBack(path.c_str());
				}
				else
					textures.PushBack(Btl::String(""));
			}

			if (pMeshResource->GetNumClusters())
			{
				PrimitiveResourcePtr pPtr = pMeshResource->DetachCluster(0);
				//This actually sets the internal type name of our mesh object
				C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().SetObjTypePrimitive(
					pVisId, 
					pPtr, 
					Maths::RGBA(0,0,128,255),
					&textures);
			}

			delete pMeshResource;
		}
	}

	//actor visualisations may have changed...force re-deserialisation & update bounds information
	if (pVisId && !m_flags.pendingSync)
		m_pActorScene->GetRootSceneGraphNodePtr()->UpdateCache();
	
	C_RenderCore::Instance().DirtyAllViewports();
}

void C_RenderScene::OnAftkRefUnloaded(const Btl::String& originalFilePath)
{
	for (S32 i=0; i<m_aftkRefScenes.Size(); ++i)
	{
		if (m_aftkRefScenes[i]->GetOriginalFilePath() == originalFilePath)
		{
			C_RenderCore::Instance().GetRenderPipeline().GetAftkSceneRenderer().DetachSceneGraphNode(m_aftkRefScenes[i]->GetRootSceneGraphNodePtr());
			SAFE_DELETE_PTR(m_aftkRefScenes[i]);
			m_aftkRefScenes.EraseByIndex(i);
		}
	}

	const EditorLib::XtVisualisationDef* pDef =
		EditorLib::C_Core::Instance().GetXtVisualisationData().FindByFilePath(
			originalFilePath.Chars());
	const char* pVisId = pDef ? pDef->id.Chars() : 0;
	if (pVisId)
		C_RenderCore::Instance().GetRenderPipeline().GetSimpleRenderer().RemoveObjTypePrimitive(pVisId);

	//actor visualisations may have changed...force re-deserialisation & update bounds information
	if (pVisId && !m_flags.pendingSync)
	{
		m_pActorScene->GetRootSceneGraphNodePtr()->UpdateCache();
	}
	AftkRender::C_Renderer::GetTextureManager().FlushUnusedTextures();
	
	C_RenderCore::Instance().DirtyAllViewports();
}

void C_RenderScene::OnCoreEnableViews(bool enable)
{
	m_flags.viewEnabled = enable;
	if (m_flags.viewDirty && m_flags.viewEnabled && !m_flags.pendingSync)
		C_RenderCore::Instance().DirtyAllViewports();
}

void C_RenderScene::OnCoreDirtyViews(EditorLib::E_DirtyEventType e)
{
	m_flags.viewDirty = true;
	C_RenderCore::Instance().DirtyAllViewports();
}

void C_RenderScene::UpdateLayerVisibility(U32 layerMask, U32 nodeMask)
{
	BIZ_ASSERT(!m_flags.pendingSync);

	for (EditorLib::LayerSetMap::ConstIterator layerSetIt = EditorLib::C_Core::Instance().GetLayerManager().LayerSetIteratorBegin(); 
		layerSetIt != EditorLib::C_Core::Instance().GetLayerManager().LayerSetIteratorEnd(); ++layerSetIt)
	{
		EditorLib::LayerList::ConstIterator layerIt = layerSetIt->GetValue()->GetLayerIterator();

		while (layerIt.Pointer())
		{
			if ((*layerIt)->GetType() == EditorLib::E_LayerType_Reference)
			{
				EditorLib::I_Layer::Flags layerFlags = (*layerIt)->GetLayerFlags();

				const EditorLib::I_Layer::DgNodeSet& dgNodeSet = (*layerIt)->GetNodes();
				for (EditorLib::I_Layer::DgNodeSet::ConstIterator dgIt = dgNodeSet.Begin(); dgIt!=dgNodeSet.End(); ++dgIt)
				{
					AftkRender::C_SceneGraphNode *pSgNode = GetSceneGraphNode(dgIt->GetValue());
					if (!pSgNode) continue;
					AftkRender::C_SceneGraphNode::Flags nodeFlags = pSgNode->GetFlags();
					nodeFlags.hidden = (layerFlags.value & layerMask) ? nodeFlags.hidden & ~nodeMask : nodeFlags.hidden | nodeMask;

					pSgNode->SetFlags(nodeFlags);
				}
			}

			++layerIt;
		}
	}
}

}