#include "stdafx.h"

#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/Hints/UpdateNameHint.h>
#include <EditorLib/Dg/DgNodeIterator.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/DgXtNodeBase.h>
#include <EditorLib/Xt/Types/IXtType.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtTypeDatabase.h>
#include <EditorLib/Core.h>

namespace EditorLib
{

//From I_DgObject
void C_DgXtNodeBase::SetName(const Btl::String& name)
{
	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());
	assert(pComponent);

	XtNodeHelpers::XtStoreName(name, *this, AsStruct(GetXtType()), &pComponent->data);
}

bool C_DgXtNodeBase::IsA(E_ObjType type) const
{
	if (C_DgNodeBase::IsA(type))
		return true;

	if (type == E_ObjType_DgXtNodeBase) 
		return true;

	return false;
}


//From I_DgNode
bool C_DgXtNodeBase::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, E_ObjType_DgXtNodeBase, pHc);

	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());

	if (GetXtData())
	{
		//Name
		{
			Xt::DomField* pField  = Xt::AsField(FindBySemantic(
				AsStruct(GetXtType()),
				&pComponent->data,
				kXtSemanticAttr,
				kXtSemanticParam,
				kXtSemanticLabel_Name).dom);
			if (GetXtDirty().IsDirty(pField))
			{
				//Load current value.
				Btl::String tmp(m_name.GetValue(false));
				//Load new value.
				XtNodeHelpers::XtDeserialiseName(tmp, this, &pComponent->data);
				//Store.
				m_name.SetValue(tmp, false);
				//Add hint to indicate what's just happened.
				signaller.GetCollection().hints.PushBack(new C_UpdateNameHint());
				m_nodeDirtyFlags.nameDirty = false;
			}
		}

	}

	//Update superclass.
	if (!__super::Update(&signaller.GetCollection())) return false;

	return true;
}

bool C_DgXtNodeBase::IsNonXtDirty() const
{
	return __super::IsNonXtDirty() || m_dgXtNodeBaseDirtyFlags.data;
}

void C_DgXtNodeBase::ResetNonXtDirty()
{
	__super::ResetNonXtDirty();
	m_dgXtNodeBaseDirtyFlags.data = 0;
}

C_XtObject* C_DgXtNodeBase::AsXtObject()
{
	return (C_XtObject*)this;
}

const C_XtObject* C_DgXtNodeBase::AsXtObject() const
{
	return (const C_XtObject*)this;
}

//From C_SceneXtNodeBase
C_DgXtNodeBase::C_DgXtNodeBase() : 
C_DgNodeBase(), C_XtObject()
{
	m_dgXtNodeBaseDirtyFlags.data = 0xffffffff;
}

C_DgXtNodeBase::~C_DgXtNodeBase()
{
	XtNodeHelpers::ReleaseXtId(m_id, this);
}

//From C_XtObject

C_Status C_DgXtNodeBase::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	//Initialise xtid
	Xt::DomComponent* pComponent = Xt::AsComponent(pData);
	BIZ_ASSERT(pComponent);
	if (!XtNodeHelpers::AssumeXtId(pLevel, m_id, this, pComponent->id, kNoChildId, pComponent))
		return E_Status_Failure;

	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id, false);

	// If we're creating a null node
	if (!pLarvaType)
		return E_Status_Success;

	{
		Btl::String name;
		XtNodeHelpers::XtInitialiseName(name, this, AsStruct(pLarvaType), &pComponent->data);
		m_name.SetValue(name);
	}

	return E_Status_Success;
}

//From C_XtObject

void C_DgXtNodeBase::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
	C_Core::Instance().GetDgManager().AddForDeferredUpdate(m_id, typeChanged);
	DirtyDependents();

	Xt::DomComponent* pComponent = Xt::AsComponent(GetXtData());
	//Name dirty?
	{
		Xt::DomField* pField  = Xt::AsField(FindBySemantic(
			AsStruct(GetXtType()),
			&pComponent->data,
			kXtSemanticAttr,
			kXtSemanticParam,
			kXtSemanticLabel_Name).dom);
		if (pField && pEvent->from ? XtHelpers::AreRelated(pField, pEvent->from) : true)
			GetXtDirty().SetDirty(pField);
	}
}

}
