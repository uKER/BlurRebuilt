#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtDirty.h>

#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgNodeBase.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/Hints/UpdateNameHint.h>
#include <EditorLib/Misc/GuidManager.h>

#include <EditorLib/Level/XtLevel.h>

#include <EditorLib/Core.h>

namespace EditorLib
{

//From I_DgObject
bool C_DgNodeBase::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;
	if (type == E_ObjType_DgNodeBase) 
		return true;

	return false;
}

C_DgNodeBase::~C_DgNodeBase()
{
	C_Core::Instance().GetDgManager().DestroyName(this, m_name.GetValue(false));
	if (m_id.IsValid())
	{
		U32 guid = m_id.GetXtId();
		C_Core::Instance().GetDgManager().UnregisterId(m_id);
		C_Core::Instance().GetGuidManager().DestroyGuid(guid);
	}
}

C_DgNodeBase::Flags& C_DgNodeBase::GetNodeFlags()
{
	return m_nodeFlags;
}

const C_DgNodeBase::Flags& C_DgNodeBase::GetNodeFlags() const
{
	return m_nodeFlags;
}

void C_DgNodeBase::SetName(const Btl::String& name)
{
	Btl::String tmp;
	m_name.GetValue(tmp, false);
	C_Core::Instance().GetDgManager().DestroyName(this, tmp);
	tmp = name;
	C_Core::Instance().GetDgManager().CreateName(this, tmp);
	m_name.SetValue(tmp);
}

const C_NodeId& C_DgNodeBase::GetId() const
{
	return m_id;
}

const Btl::String& C_DgNodeBase::GetName() const
{
	return m_name.GetValue();
}

void C_DgNodeBase::GetPlugs(Btl::Vector<I_Plug*>& plugs)
{
	plugs.PushBack((I_Plug*)&m_name);
}

void C_DgNodeBase::GetPlugs(Btl::Vector<const I_Plug*>& plugs) const
{
	return const_cast<C_DgNodeBase*>(this)->GetPlugs((Btl::Vector<const I_Plug*>&)plugs);
}

bool C_DgNodeBase::Update(C_UpdateHintCollection* pHc)
{
	//Do not trigger updates before signalling our creation!
	if (!m_nodeFlags.postCreateSignalled)
	{
		BIZ_ASSERT(!"Do not trigger updates before signalling our creation!");
		return false;
	}

	C_UpdateSignaller signaller(this, E_ObjType_DgNodeBase, pHc);
	if (m_nodeDirtyFlags.nameDirty)
	{
		//Load.
		Btl::String tmp(m_name.GetValue(false));
		//Remove old name from registry.
		C_Core::Instance().GetDgManager().DestroyName(this, tmp);
		//Ensure unique.
		C_Core::Instance().GetDgManager().CreateName(this, tmp);
		//Store.
		m_name.SetValue(tmp, false);
		//Add hint to indicate what's just happened.
		signaller.GetCollection().hints.PushBack(new C_UpdateNameHint());
	}

	return true;
}

bool C_DgNodeBase::DirtyDependents(bool setThisDirty)
{
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id);

	if (setThisDirty)
		m_nodeDirtyFlags.data = 0xffffffff;

	if (IsDirty())
		return false;

	return true;
}

bool C_DgNodeBase::IsNonXtDirty() const
{
	return m_nodeDirtyFlags.data != 0;
}

void C_DgNodeBase::ResetNonXtDirty()
{
	m_nodeDirtyFlags.data = 0;
}

bool C_DgNodeBase::IsReadOnly() const
{
	return m_nodeFlags.readOnly || C_Core::Instance().GetDgManager().LevelOf(m_id)->IsReadOnly();
}

bool C_DgNodeBase::IsDeletable() const
{
	return m_nodeFlags.userDeletable;
}

bool C_DgNodeBase::IsExternal() const
{
	return m_nodeFlags.external;
}

bool C_DgNodeBase::IsAlwaysHidden() const
{
	return m_nodeFlags.hidePermanently;
}

bool C_DgNodeBase::IsRenameable() const
{
	return m_nodeFlags.canRename;
}

C_XtObject* C_DgNodeBase::AsXtObject()
{
	return 0;
}

const C_XtObject* C_DgNodeBase::AsXtObject() const
{
	return 0;
}

bool C_DgNodeBase::IsDirty() const
{
	return IsNonXtDirty() || (AsXtObject() && AsXtObject()->GetXtDirty().IsDirty());
}

void C_DgNodeBase::ResetDirty()
{
	//Clear non xt dirty flags.
	ResetNonXtDirty();
	//Clear Xt dirty flags.
	if (AsXtObject())
		AsXtObject()->GetXtDirty().Clear();
}

C_DgNodeBase::C_DgNodeBase() : 
	I_DgNode(),
	m_name(this, &C_DgNodeBase::OnNodeBasePlugChanged, E_PlugType_InputOutput, "name")

{
	//Create guid.
	U32 guid;
	C_Core::Instance().GetGuidManager().CreateGuid(guid);
	m_id = C_NodeId(guid, kNoChildId);

	//Register guid with dependency graph manager.
	C_Core::Instance().GetDgManager().RegisterId(this, m_id);

	m_nodeFlags.readOnly = 0;
	m_nodeFlags.userDeletable = 1;
	m_nodeFlags.baseInstance = 0;
	m_nodeFlags.hidePermanently = 0;
	m_nodeFlags.external = 0;
	m_nodeFlags.postCreateSignalled = 0;
	m_nodeFlags.canRename = 1;
	
	m_nodeDirtyFlags.data = 0xffffffff;
}

void C_DgNodeBase::OnNodeBasePlugChanged(I_Plug* pPlug)
{
	C_DgNodeBase* pThis = (C_DgNodeBase*)PlugHelpers::GetTopLevelParent(*pPlug);
	if (pPlug == &pThis->m_name && !pThis->m_nodeDirtyFlags.nameDirty)
	{
		pThis->m_nodeDirtyFlags.nameDirty = true;
	}

	if (pThis->m_nodeDirtyFlags.data)
		pThis->DirtyDependents(false);
}

void C_DgNodeBase::SetPostCreateSignalled()
{
	m_nodeFlags.postCreateSignalled = true;
}

}
