#include "stdafx.h"

#include <EditorLib/Aftk/AftkRef.h>
#include <EditorLib/Aftk/AftkRefManager.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Level/Layer.h>
#include <EditorLib/Level/LevelBase.h>
#include <EditorLib/Misc/UndoManager.h>
#include <EditorLib/Core.h>
#include <EditorLib/GlobalSignals.h>

namespace EditorLib
{

C_LevelBase::C_LevelBase()
{
	m_flags.isOpen = false;
	m_flags.pendingSync = false;

	C_Core::Instance().GetDgManager().AttachLevel(this);
}

C_LevelBase::~C_LevelBase()
{
	if (m_flags.isOpen)
		Unload();

	C_Core::Instance().GetDgManager().DetachLevel(this);
}

E_LevelType C_LevelBase::GetType() const
{
	return E_LevelType_LevelBase;
}

C_Status C_LevelBase::Load()
{
	if (m_flags.isOpen)
		return E_Status_Failure;

	m_flags.isOpen = true;
	m_flags.pendingSync = true;

	C_Status status = Initialise();
	if (!status) return status;

	C_Core::Instance().GetGlobalSignals().levelOpening(*this);

	LoadData();

	//Perform one-off immediate initialisation.
	if (m_pRootSceneNode)
	{
		m_pRootSceneNode->PostInitialiseImmediatePropagate();
	}

	C_Core::Instance().GetGlobalSignals().levelOpened(*this);
	
	m_flags.pendingSync = false;
	m_flags.isHidden = false;

	return status;
}

void C_LevelBase::Unload()
{
	if (!m_flags.isOpen)
		return;

	m_flags.pendingSync = true;

	C_Core::Instance().GetGlobalSignals().levelClosing(*this);
	
	//Unload and remove any references that we own.
	ClearAftkRefs();
	
	UnloadData();

	//Erase the Dg nodes that belong to this level.
	{
		for (DgIdPtrMap::Iterator it = m_dgNodes.Begin(); it != m_dgNodes.End(); it = m_dgNodes.Begin())
		{
			C_Core::Instance().GetDgManager().DestroyNode(it->GetValue(), true);
		}
	}

	// Clear all nodes so we can re-load this level at a later date
	m_dgNodes.Clear();

	m_pRootSceneNode = 0;
	m_flags.isOpen = false;
	m_flags.pendingSync = false;

	C_Core::Instance().GetGlobalSignals().levelClosed(*this);
}

C_Status C_LevelBase::Initialise()
{
	//Initialise scene root node.
	{
		m_pRootSceneNode = 
			(C_SceneNode*)C_Core::Instance().GetDgManager().CreateNode(this, E_ObjType_SceneNode);
		m_pRootSceneNode->SetName("Root");
		m_pRootSceneNode->GetNodeFlags().readOnly = 1;
		m_pRootSceneNode->GetSceneNodeFlags().canReparent = 0;
		m_pRootSceneNode->GetSceneNodeFlags().canAddChildren = 1;
		m_pRootSceneNode->GetNodeFlags().userDeletable = 0;
	}

	return E_Status_Success;
}

C_Status C_LevelBase::LoadData()
{
	return E_Status_Success;
}

C_Status C_LevelBase::UnloadData()
{
	return E_Status_Success;
}

const C_AftkLevelRef* C_LevelBase::AddAftkRef(const Btl::String& _filePath)
{
	C_CheckpointDisabler cpDisable;
	Btl::String filePath = Misc::FormatUnixFileString(_filePath.Lower());
	const C_AftkLevelRef* p = GetAftkRef(filePath);
	if (p) return p;

	C_AftkLevelRef* pAftkLevelRef = new C_AftkLevelRef();
	
	m_aftkRefs.PushBack(pAftkLevelRef);

	return pAftkLevelRef;
}

C_Status C_LevelBase::RemoveAftkRef(const Btl::String& filepath)
{
	C_CheckpointDisabler cpDisable;
	Btl::String filePath = Misc::FormatUnixFileString(filePath.Lower());

	AftkLevelRefList::Iterator toEraseIt = m_aftkRefs.End();
	for (AftkLevelRefList::Iterator it = m_aftkRefs.Begin(); 
		it != m_aftkRefs.End(); 
		++it)
	{
		const C_AftkRef* pScene = (*it)->pAftkRef;
		if (pScene->GetOriginalFilePath() == filePath)
		{
			toEraseIt = it;
			break;
		}
	}

	if (toEraseIt == m_aftkRefs.End())
		return E_Status_Failure;

	//Destroy ref.
	C_Status status = EditorLib::C_Core::Instance().GetAftkRefManager().DestroyRef(filePath);
	if (!status) return status;

	//Erase local data.
	SAFE_DELETE_PTR(*toEraseIt);
	m_aftkRefs.Erase(toEraseIt);

	return E_Status_Success;
}

const C_AftkLevelRef* C_LevelBase::GetAftkRef(const Btl::String& _filePath) const
{
	Btl::String filePath = Misc::FormatUnixFileString(_filePath.Lower());

	for (AftkLevelRefList::ConstIterator it = m_aftkRefs.Begin(); 
		it != m_aftkRefs.End(); 
		++it)
	{
		const C_AftkRef* pScene = (*it)->pAftkRef;
		if (pScene->GetOriginalFilePath() == filePath)
			return *it;
	}

	return 0;
}

const AftkLevelRefList& C_LevelBase::GetAftkRefs() const
{
	return m_aftkRefs;
}

C_SceneNode* C_LevelBase::GetRootSceneNode()
{
	return m_pRootSceneNode;
}

const C_SceneNode* C_LevelBase::GetRootSceneNode() const
{
	return m_pRootSceneNode;
}

void C_LevelBase::SetName(const Btl::String& name)
{
	m_Name = name;

	C_Core::Instance().GetGlobalSignals().levelNameChanged(*this);
}

const Btl::String& C_LevelBase::GetName() const
{
	return m_Name;
}

void C_LevelBase::BuildActorNodeArray(Btl::Vector<const I_DgNode*>& nodes) const
{
	LOGERROR(C_Core::Instance().GetLogger(), "BuildActorNodeArray is not implemented in C_LevelBase");
}

bool C_LevelBase::ContainsNode(const C_NodeId& id) const
{
	DgIdPtrMap::Iterator it = m_dgNodes.Find(id);
	return it != m_dgNodes.End();
}

const I_Level::C_Flags& C_LevelBase::GetFlags() const
{
	return m_flags;
}

void C_LevelBase::OnAttachDgNode(I_DgNode* pNode)
{
	BIZ_ASSERT(pNode);
	DgIdPtrMap::InsertionReturnType r = m_dgNodes.Insert(
		DgIdPtrMap::DataType(pNode->GetId(), 0));
	//Check was inserted.
	BIZ_ASSERT(r.second);
	r.first->GetValue() = pNode;
}

void C_LevelBase::OnDetachDgNode(I_DgNode* pNode)
{
	DgIdPtrMap::Iterator it = m_dgNodes.Find(pNode->GetId());
	if (it == m_dgNodes.End())
	{
		BIZ_ASSERT(!"Could not find node to detach!");
		return;
	}

	//Remove entry in dg map.
	m_dgNodes.Erase(it);
}

bool C_LevelBase::IsHidden() const
{
	return m_flags.isHidden;
}

void C_LevelBase::SetHidden(bool hidden)
{
	m_flags.isHidden = hidden;
}

bool C_LevelBase::IsReadOnly() const
{
	return m_flags.isReadOnly;
}

void C_LevelBase::SetReadOnly(bool readOnly)
{
	m_flags.isReadOnly = readOnly;

	C_Core::Instance().GetGlobalSignals().levelEvent(this, E_LevelEvent_ReadOnlyFlagChanged);
}

void C_LevelBase::ClearAftkRefs()
{
	for (AftkLevelRefList::Iterator it = m_aftkRefs.Begin(); 
		it != m_aftkRefs.End(); 
		++it)
	{
		const C_AftkRef* pScene = (*it)->pAftkRef;
		const Btl::String path = pScene->GetOriginalFilePath();
		C_Status status = C_Core::Instance().GetAftkRefManager().DestroyRef(pScene->GetOriginalFilePath());
		BIZ_ASSERT(status);

		SAFE_DELETE_PTR(*it);
	}

	m_aftkRefs.Clear();
}

}
