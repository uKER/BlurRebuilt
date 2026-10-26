#include "stdafx.h"

#include <EditorLib/Xt/XtObject.h>

#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/DgXtPropNodeBase.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Level/ILevel.h>
#include <EditorLib/Core.h>

namespace EditorLib
{

//From I_DgNode
bool C_DgXtPropNodeBase::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, E_ObjType_DgXtPropNodeBase, pHc);
	m_propNodeDirtyFlags.data = 0;

	if (m_pParent)
	{
		m_propNodeDirtyFlags.hierarchyDirty = false;
	}
	else
	{
		BIZ_ASSERT(!"Parent must be set before initial update!");
	}
	
	return __super::Update(pHc);
}

bool C_DgXtPropNodeBase::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	if (setThisDirty)
		m_propNodeDirtyFlags.data = 0xffffffff;
	return true;
}

bool C_DgXtPropNodeBase::IsNonXtDirty() const
{
	return m_propNodeDirtyFlags.data || __super::IsNonXtDirty();
}

void C_DgXtPropNodeBase::ResetNonXtDirty()
{
	__super::ResetNonXtDirty();
	m_propNodeDirtyFlags.data = 0;
}

//From C_DgXtPropNodeBase
bool C_DgXtPropNodeBase::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == E_ObjType_DgXtPropNodeBase) 
		return true;

	return false;
}

C_DgXtCompNode* C_DgXtPropNodeBase::GetParent()
{
	return m_pParent;
}

const C_DgXtCompNode* C_DgXtPropNodeBase::GetParent() const
{
	return m_pParent;
}

//From C_DgXtPropNodeBase

C_DgXtPropNodeBase::C_DgXtPropNodeBase() : C_DgNodeBase(), C_XtObject()
{
	m_pParent = 0;
	m_propNodeDirtyFlags.data = 0xffffffff;
}

C_DgXtPropNodeBase::~C_DgXtPropNodeBase()
{
	XtNodeHelpers::ReleaseXtId(m_id, this);
}

void C_DgXtPropNodeBase::SetParent(C_DgXtCompNode* pParent, U32 childId)
{
	//Id within acceptable range.
	assert(childId < 0xffff);
	//Parent id does not have prop id.
	assert((pParent->GetId().GetChildId() & 0x0000ffff) == 0xffff);
	//Cannot reparent component.
	if (m_pParent)
	{
		assert(m_pParent == pParent);
		return;
	}
	m_pParent = pParent;
	I_Level* pLevel = C_Core::Instance().GetDgManager().LevelOf(m_pParent->GetId());
	BIZ_ASSERT(pLevel);
	BIZ_ASSERT(pLevel->GetType() == E_LevelType_XtLevel);

	bool result = XtNodeHelpers::AssumeXtId(
		(C_XtLevel*)pLevel,
		m_id, 
		this, 
		pParent->GetId().GetXtId(), 
		(pParent->GetId().GetChildId() & 0xffff0000) | childId);
	BIZ_ASSERT(result);

	DirtyDependents();
}

bool C_DgXtPropNodeBase::IsDirty() const
{
	return __super::IsDirty() || m_propNodeDirtyFlags.data != 0;
}

void C_DgXtPropNodeBase::ResetDirty()
{
	__super::ResetDirty();
	m_propNodeDirtyFlags.data = 0;
}

C_XtObject* C_DgXtPropNodeBase::AsXtObject()
{
	return (C_XtObject*)this;
}

const C_XtObject* C_DgXtPropNodeBase::AsXtObject() const
{
	return (const C_XtObject*)this;
}

C_Status C_DgXtPropNodeBase::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id, false);
	return status;
}

void C_DgXtPropNodeBase::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id, typeChanged);
	DirtyDependents();
}

}
