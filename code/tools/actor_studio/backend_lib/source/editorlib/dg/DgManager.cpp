#include "stdafx.h"

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgXtCompNode.h>

#include <EditorLib/Misc/StringUtil.h>

#include <EditorLib/Core.h>
#include <EditorLib/Level/XtLevel.h>
#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

static const char* kDefaultName = "Unnamed";

C_DgManager::C_DgManager()
{
	m_state = E_State_Unknown;
}

C_DgManager::~C_DgManager()
{
	BIZ_ASSERT(m_dgById.Size() == 0);
}

void C_DgManager::Clear()
{
	BIZ_ASSERT(m_dgById.Size() == 0);

	//Delete all DG nodes.
	for (DgIdPtrMap::Iterator it = m_dgById.Begin(); it != m_dgById.End(); ++it)
	{
		I_DgNode* pNode = it->GetValue();
		delete pNode;
	}
	m_dgById.Clear();
}

I_DgNode* C_DgManager::CreateNode(I_Level* pLevel, const Btl::String& typeName)
{
	I_DgNode* pNode = C_Core::Instance().GetDgNodeFactory().Create(typeName.Chars());
	if (!pNode)
		return 0;

	pLevel->OnAttachDgNode(pNode);
	C_Core::Instance().GetGlobalSignals().dgNodeCreated(*pNode);
	pNode->SetPostCreateSignalled();
	return pNode;
}

I_DgNode* C_DgManager::CreateNode(I_Level* pLevel, E_ObjType type)
{
	I_DgNode* pNode = C_Core::Instance().GetDgNodeFactory().Create(type);
	if (!pNode)
		return 0;

	pLevel->OnAttachDgNode(pNode);
	C_Core::Instance().GetGlobalSignals().dgNodeCreated(*pNode);
	pNode->SetPostCreateSignalled();
	return pNode;
}

bool C_DgManager::DestroyNode(I_DgNode* pNode, bool deleteOverride)
{
	BIZ_ASSERT(pNode);
	//Override read only flag?
	if (!pNode->IsDeletable() && !deleteOverride)
		return false;

	C_Core::Instance().GetGlobalSignals().dgNodeDestroy(*pNode);

	I_Level* pLevel = LevelOf(pNode->GetId());
	BIZ_ASSERT(pLevel);
	pLevel->OnDetachDgNode(pNode);

	//Delete node.
	delete pNode;
	return true;
}

I_DgNode* C_DgManager::FindNodeById(C_NodeId id)
{
	DgIdPtrMap::Iterator it = m_dgById.Find(id);
	if (it == m_dgById.End())
	{
#if defined(BUILD_CONFIG_DEBUG) && defined(USER_KMACLEOD)
//		LOGDEBUG(C_Core::Instance().GetLogger(), "FindNodeById failed, this is okay so long as the object %s was deleted.\n", id.AsString().Chars());
#endif
		return 0;
	}
	return it->GetValue();
}

const I_DgNode*	C_DgManager::FindNodeById(C_NodeId id) const
{
	return (const_cast<C_DgManager*>(this))->FindNodeById(id);
}

I_Level* C_DgManager::LevelOf(const C_NodeId& id)
{
	for (S32 i = 0; i < m_levels.Size(); ++i)
		if (m_levels[i]->ContainsNode(id))
			return m_levels[i];

	return 0;
}

C_DgNodeIterator C_DgManager::GetDgIterator()
{
	return C_DgNodeIterator(&m_dgById);
}

C_DgNodeConstIterator C_DgManager::GetDgIterator() const
{
	return C_DgNodeConstIterator(&m_dgById);
}

U32	C_DgManager::GetNumDgNodes() const
{
	return (U32)m_dgById.Size();
}

void C_DgManager::CreateName(I_DgNode* pNode, Btl::String& name)
{
	if (name.Length() == 0)
		name = kDefaultName;

	//Firstly, ensure that there are no spaces in the name.
	name = name.Replace(" ", "_");
	name = name.Replace(",", "_");
}

void C_DgManager::DestroyName(I_DgNode* pNode, const Btl::String& name)
{
}

void C_DgManager::AddForDeferredUpdate(const C_NodeId& nodeId, bool typeChange)
{
	C_PendingDeferredUpdates* pDeferredUpdates;
	if (m_state == E_State_Updating)
		pDeferredUpdates = &deferredUpdatesSecondary;
	else
		pDeferredUpdates = &deferredUpdatesPrimary;

	if (!nodeId.IsValid())
		return;
	UpdateItemMap::InsertionReturnType r = pDeferredUpdates->deferredUpdateMap.Insert(nodeId);
	if(r.inserted)
	{
		pDeferredUpdates->deferredUpdates.PushBack(new C_UpdateItem(nodeId, typeChange));
		r.iterator->GetValue() = pDeferredUpdates->deferredUpdates.Back();
	}
	else
	{
		r.iterator->GetValue()->typeChange |= typeChange;
	}
}

void C_DgManager::RemoveForDeferredUpdate(const C_NodeId& nodeId)
{
	C_PendingDeferredUpdates* pDeferredUpdates;
	if (m_state == E_State_Updating)
		pDeferredUpdates = &deferredUpdatesSecondary;
	else
		pDeferredUpdates = &deferredUpdatesPrimary;

	UpdateItemMap::Iterator it = pDeferredUpdates->deferredUpdateMap.Find(nodeId);
	if (it == pDeferredUpdates->deferredUpdateMap.End())
	{
		BIZ_ASSERT(!"RemoveForDeferredUpdate failed!");
		return;
	}

	for (S32 i = 0; i < pDeferredUpdates->deferredUpdates.Size(); ++i)
	{
		if (pDeferredUpdates->deferredUpdates[i]->nodeId == nodeId)
		{
			SAFE_DELETE_PTR(pDeferredUpdates->deferredUpdates[i]);
			pDeferredUpdates->deferredUpdates.EraseByIndex(i);
		}
	}
	pDeferredUpdates->deferredUpdateMap.Erase(it);
}

bool C_DgManager::IsDeferredUpdatePending() const
{
	return deferredUpdatesPrimary.deferredUpdates.Size() != 0;
}

S32 GetDepth(I_DgNode* pDgNode)
{
	if (!pDgNode)
		return 0;

	C_SceneNode* pSceneNode = NULL;
	if (pDgNode->IsA(EditorLib::E_ObjType_SceneNode))
	{
		pSceneNode = (C_SceneNode*)pDgNode;
	}
	else if(pDgNode->IsA(EditorLib::E_ObjType_DgXtCompNode))
	{
		C_DgXtCompNode* pCompNode = (C_DgXtCompNode*)pDgNode;
		if (pCompNode->GetParent() && pCompNode->GetParent()->IsA(EditorLib::E_ObjType_SceneNode))
			pSceneNode = (C_SceneNode*)pCompNode->GetParent();
		else
			return 0;
	}
	else if (pDgNode->IsA(EditorLib::E_ObjType_DgXtPropNodeBase))
	{
		C_DgXtPropNodeBase* pPropNode = (C_DgXtPropNodeBase*)pDgNode;
		if (!pPropNode->GetParent())
			return 0;

		C_DgXtCompNode* pCompNode = (C_DgXtCompNode*)pPropNode->GetParent();
		if (pCompNode->GetParent() && pCompNode->GetParent()->IsA(EditorLib::E_ObjType_SceneNode))
			pSceneNode = (C_SceneNode*)pCompNode->GetParent();
		else
			return 0;
	}
	else
		return 0;

	S32 depth = 0;
	C_SceneNode* pCurrent = pSceneNode->GetParent();
	while(pCurrent)
	{
		++depth;
		pCurrent = pCurrent->GetParent();
	}
	return depth;
}

void C_DgManager::FlushDeferredUpdates(C_PendingDeferredUpdates& pendingDeferredUpdates)
{
	//Type changes may in turn generate further deferred deserialisation tasks (eg type changing to a box trigger).
	//Iterators are therefore not guaranteed to be valid after a node deserialisation
	for (S32 i = 0; i < pendingDeferredUpdates.deferredUpdates.Size(); ++i)
	{
		const C_NodeId& id = pendingDeferredUpdates.deferredUpdates[i]->nodeId;
		I_DgNode* pNode = FindNodeById(id);
		if (!pNode)
			continue;

		S32 depth = GetDepth((C_SceneNode*)pNode);
		if (m_sortedUpdates.Size() <= depth)
		{
			m_sortedUpdates.Reserve((depth+1)*2);
			m_sortedUpdates.Resize(depth+1);
		}
		
		m_sortedUpdates[depth].PushBack(pendingDeferredUpdates.deferredUpdates[i]);
	}

	for (S32 iDepth = 0; iDepth < m_sortedUpdates.Size(); ++iDepth)
	{
		for (S32 j = 0; j < m_sortedUpdates[iDepth].Size(); ++j)
		{
			C_UpdateItem* pUpdateItem = m_sortedUpdates[iDepth][j];
			const C_NodeId& id = pUpdateItem->nodeId;
			//Check we've not processed this node yet.
			UpdateItemMap::Iterator it = pendingDeferredUpdates.deferredUpdateMap.Find(id);
			if (it == pendingDeferredUpdates.deferredUpdateMap.End())
				//Already done.
				continue;

			//Remove from set to process.
			pendingDeferredUpdates.deferredUpdateMap.Erase(it);
			//Find node ptr.
			I_DgNode* pNode = FindNodeById(id);
			//Note, node may have been erased.
			if (pNode)
			{
//				LOGDEBUG(C_Core::Instance().GetLogger(), "Update %s", id.AsString().Chars());
				//Node type changing.
				if (pUpdateItem->typeChange)
				{
					I_Level* pLevel = LevelOf(id);
					if (pLevel)
					{
						BIZ_ASSERT(pLevel->GetType() == E_LevelType_XtLevel);
						((C_XtLevel*)pLevel)->OnNodeChangeXtType(id);
					}
				}
				else
					pNode->Update();
			}
		}
	}
	m_sortedUpdates.Clear();
}

void C_DgManager::FlushDeferredUpdates()
{
	if (deferredUpdatesPrimary.deferredUpdates.Size() == 0)
		return;

	m_state = E_State_Updating;
//	LOGDEBUG(C_Core::Instance().GetLogger(),"Begin update");

	FlushDeferredUpdates(deferredUpdatesPrimary);
	FlushDeferredUpdates(deferredUpdatesSecondary);
	ClearDeferredUpdates();

//	LOGDEBUG(C_Core::Instance().GetLogger(), "End update");
	m_state = E_State_Unknown;
}

void C_DgManager::ClearDeferredUpdates()
{
	deferredUpdatesPrimary.Clear();
	deferredUpdatesSecondary.Clear();
}

bool C_DgManager::RegisterId(I_DgNode* pNode, C_NodeId& id)
{
	BIZ_ASSERT(id.IsValid());
#ifdef BUILD_CONFIG_DEBUG
	BIZ_ASSERT(m_dgById.Find(id) == m_dgById.End());
//	LOGDEBUG(C_Core::Instance().GetLogger(), "RegisterId %s", id.AsString().Chars());
#endif

	m_dgById.Insert(DgIdPtrMap::DataType(id, pNode));
#ifdef BUILD_CONFIG_DEBUG
	BIZ_ASSERT(m_dgById.Find(id) != m_dgById.End());
#endif

	return true;
}

void C_DgManager::UnregisterId(C_NodeId id)
{
#ifdef BUILD_CONFIG_DEBUG
///	LOGDEBUG(C_Core::Instance().GetLogger(), "UnregisterId %s", id.AsString().Chars());
#endif

	DgIdPtrMap::Iterator it = m_dgById.Find(id);
	if (it != m_dgById.End())
	{
		m_dgById.Erase(it);
		return;
	}

	BIZ_ASSERT(!"Could not find id to destroy!");
}

void C_DgManager::AttachLevel(I_Level* pLevel)
{
	m_levels.PushBack(pLevel);
}

void C_DgManager::DetachLevel(I_Level* pLevel)
{
	for (S32 i = 0; i < m_levels.Size(); ++i)
	{
		if (m_levels[i] == pLevel)
		{
			m_levels.EraseByIndex(i);
			return;
		}
	}

	BIZ_ASSERT(!"Could not detach level!");
}

S32	C_DgManager::FindLevelIdx(const I_Level* pLevel) const
{
	for (S32 i = 0; i < m_levels.Size(); ++i)
	{
		if (m_levels[i] == pLevel)
			return (U32)i;
	}

	return -1;
}

}
