#include "stdafx.h"

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtSemantic.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Link/DgLinkPropNode.h>

void RegisterLinkPropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgLinkPropNode)
}

namespace EditorLib
{

//From I_DgNode
bool C_DgLinkPropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgLinkPropNode::GetStaticType(), pHc);

	bool update = false;
	//If we've lost the data we're monitoring.
	if (!GetXtData())
	{
		C_DgXtCompNode* pParent = GetParent();

		Xt::DomBlock* pParentRoot = Xt::AsBlock(pParent->GetXtData());

		Btl::Vector<XtNodeHelpers::C_XtPropertyDesc> propertyDescArray;
		XtNodeHelpers::BuildXtActorPropertyList(
			propertyDescArray, pParent->GetXtType(), pParentRoot, false);
		S32 ourIndex = XtNodeHelpers::GetPropertyIndex(pParent, this);
		if (ourIndex > propertyDescArray.Size())
		{
			BIZ_ASSERT(!"Failed to find property!");
			return false;
		}

		XtNodeHelpers::C_XtPropertyDesc& propDesc = propertyDescArray[ourIndex];
		BIZ_ASSERT(propDesc.pData);
		//Reattach monitor.
		SetXtData(propDesc.pData);
		update = true;
	}
	else
		update = GetXtDirty().IsDirty(GetXtData());

	if (update)
	{
		XtId id = Xt::AsId(GetXtData());
		I_DgNode* pDgNode = id ? XtNodeHelpers::FindNodeByXtId(id) : 0;
		if (pDgNode && pDgNode->IsA(E_ObjType_SceneXtNodeBase))
		{
			C_SceneXtNodeBase* pActor = (C_SceneXtNodeBase*)pDgNode;
			m_pTargetWorldMatrixPlug->SetSourceConnection(&pActor->GetWorldMatrixPlug(), false);
		}
		else 
			m_pTargetWorldMatrixPlug->SetSourceConnection(0, false);
	}

	//We don't actually use this flag for the moment.
	m_linkNodeDirtyFlags.targetChanged = false;
	return __super::Update(&signaller.GetCollection());
}

bool C_DgLinkPropNode::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	if (setThisDirty)
		m_linkNodeDirtyFlags.data = 0xffffffff;
	return true;
}

bool C_DgLinkPropNode::IsNonXtDirty() const
{
	return __super::IsNonXtDirty();
}




const char* C_DgLinkPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_DgLinkPropNode::GetStaticTypeName()
{
	static char* p = "Link";
	return p;
}

E_ObjType C_DgLinkPropNode::GetStaticType()
{
	return E_ObjType_DgLinkPropNode;
}

void* C_DgLinkPropNode::Create()
{
	return new C_DgLinkPropNode();
}

C_DgLinkPropNode::C_DgLinkPropNode()
: C_DgXtPropNodeBase()
{
	m_pTargetWorldMatrixPlug = new C_StaticPlug<MathsStorage::Matrix44>(
		this, &C_DgLinkPropNode::OnTargetPlugChanged, E_PlugType_Input, "targetWorldMatrix");
	m_linkNodeDirtyFlags.data = 0;
}

C_DgLinkPropNode::~C_DgLinkPropNode()
{
	SAFE_DELETE_PTR(m_pTargetWorldMatrixPlug);
}

E_ObjType C_DgLinkPropNode::GetType() const
{
	return E_ObjType_DgLinkPropNode;
}

bool C_DgLinkPropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgLinkPropNode::GetType()) 
		return true;

	return false;
}

bool C_DgLinkPropNode::IsVisible() const
{
	return true;
}

bool C_DgLinkPropNode::IsDirty() const
{
	return __super::IsDirty() || m_linkNodeDirtyFlags.data != 0;
}

void C_DgLinkPropNode::ResetDirty()
{
	__super::ResetDirty();
	m_linkNodeDirtyFlags.data = 0;
}

C_Status C_DgLinkPropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	return status;
}

const I_DgNode* C_DgLinkPropNode::GetLinkedNode() const
{
	I_Plug* pTargetPlug = m_pTargetWorldMatrixPlug->GetSourceConnection();
	return pTargetPlug ? (I_DgNode*)PlugHelpers::GetTopLevelParent(*pTargetPlug) : 0;
}

C_StaticPlug<MathsStorage::Matrix44>& C_DgLinkPropNode::GetTargetWorldMatrixPlug()
{
	return *(C_StaticPlug<MathsStorage::Matrix44>*)m_pTargetWorldMatrixPlug;
}

const C_StaticPlug<MathsStorage::Matrix44>& C_DgLinkPropNode::GetTargetWorldMatrixPlug() const
{
	return *(const C_StaticPlug<MathsStorage::Matrix44>*)m_pTargetWorldMatrixPlug;
}

void C_DgLinkPropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);

	if (pEvent->code = Xt::kDomRefChanged)
		GetXtDirty().SetDirty(GetXtData());
}

void C_DgLinkPropNode::OnTargetPlugChanged(I_Plug* pPlug)
{
	BIZ_ASSERT(pPlug);
	//Set as dirty due to something interesting happening to the node we're looking at.
	I_DgNode* pParent = PlugHelpers::GetTopLevelParent(*pPlug);
	BIZ_ASSERT(pParent->IsA(E_ObjType_DgLinkPropNode));
	((C_DgLinkPropNode*)pParent)->m_linkNodeDirtyFlags.targetChanged = true;
	pParent->DirtyDependents(false);
}

}
