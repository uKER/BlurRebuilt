#include "stdafx.h"

#include <EditorLib/Misc/Status.h>
#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Dg/XtNodeHelpers.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/Attr/IPlug.h>
#include <EditorLib/Dg/Attr/NullPlug.h>
#include <EditorLib/Dg/Attr/StaticPlug.h>
#include <EditorLib/Dg/Attr/PlugHelpers.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/CoverNode/DgCoverNodePropNode.h>

void RegisterCoverNodePropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgCoverNodePropNode)
}

namespace EditorLib
{

//From I_DgNode
bool C_DgCoverNodePropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgCoverNodePropNode::GetStaticType(), pHc);

	{
		C_DgXtCompNode* pParent = GetParent();
		Xt::DomBlock* pParentRoot = Xt::AsBlock(pParent->GetXtData());
		Xt::DomRef* pRef = Xt::AsRef(Xt::At(pParentRoot, "LinkedCover"));
		if (pRef)
		{
			XtId id = Xt::AsId(pRef);
			I_DgNode* pDgNode = id ? XtNodeHelpers::FindNodeByXtId(id) : 0;
			if (pDgNode && pDgNode->IsA(E_ObjType_SceneXtActorNode))
			{
				C_SceneXtActorNode* pActor = (C_SceneXtActorNode*)pDgNode;
				m_pTargetWorldMatrixPlug->SetSourceConnection(&pActor->GetWorldMatrixPlug(), false);
			}
			else 
				m_pTargetWorldMatrixPlug->SetSourceConnection(0, false);
		}
	}

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
		Xt::DomField* pField = Xt::AsField(GetXtData());
		if (pField)
		{
			Btl::String coverNodeType = Xt::AsText(pField);
			coverNodeType = coverNodeType.Lower();

			if (coverNodeType == "highwall")
				m_coverNodeType = E_CoverNodeType_HighWall;
			else if (coverNodeType == "lowwall")
				m_coverNodeType = E_CoverNodeType_LowWall;
			else if (coverNodeType == "lowstepoutleft")
				m_coverNodeType = E_CoverNodeType_LowStepOutLeft;
			else if (coverNodeType == "lowstepoutright")
				m_coverNodeType = E_CoverNodeType_LowStepOutRight;
			else if (coverNodeType == "highstepoutleft")
				m_coverNodeType = E_CoverNodeType_HighStepOutLeft;
			else if (coverNodeType == "highstepoutright")
				m_coverNodeType = E_CoverNodeType_HighStepOutRight;
		}
	}

	//We don't actually use this flag for the moment.
	m_coverNodeDirtyFlags.data = 0;
	return __super::Update(&signaller.GetCollection());
}

bool C_DgCoverNodePropNode::DirtyDependents(bool setThisDirty)
{
	if (!__super::DirtyDependents(setThisDirty))
		return false;

	if (setThisDirty)
		m_coverNodeDirtyFlags.data = 0xffffffff;
	return true;
}

bool C_DgCoverNodePropNode::IsNonXtDirty() const
{
	return __super::IsNonXtDirty();
}


const char* C_DgCoverNodePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

const char* C_DgCoverNodePropNode::GetStaticTypeName()
{
	//Must match name in Xt schema exactly!
	static char* p = "CoverType";
	return p;
}

E_ObjType C_DgCoverNodePropNode::GetStaticType()
{
	return E_ObjType_DgCoverNodePropNode;
}

void* C_DgCoverNodePropNode::Create()
{
	return new C_DgCoverNodePropNode();
}

C_DgCoverNodePropNode::C_DgCoverNodePropNode()
:
C_DgXtPropNodeBase()
{
	m_pTargetWorldMatrixPlug = new C_StaticPlug<MathsStorage::Matrix44>(
		this, &C_DgCoverNodePropNode::OnTargetPlugChanged, E_PlugType_Input, "targetWorldMatrix");
	m_coverNodeDirtyFlags.data = 0;
}

C_DgCoverNodePropNode::~C_DgCoverNodePropNode()
{
	SAFE_DELETE_PTR(m_pTargetWorldMatrixPlug);
}

E_ObjType C_DgCoverNodePropNode::GetType() const
{
	return E_ObjType_DgCoverNodePropNode;
}

bool C_DgCoverNodePropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgCoverNodePropNode::GetType()) 
		return true;

	return false;
}

bool C_DgCoverNodePropNode::IsVisible() const
{
	return true;
}

C_Status C_DgCoverNodePropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	m_coverNodeType = E_CoverNodeType_LowStepOutLeft;
	return status;
}

void C_DgCoverNodePropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);

	//We're only interested in a field change event.
	if (pEvent->code = Xt::kDomFieldChanged)
		GetXtDirty().SetDirty(GetXtData());
}

C_DgCoverNodePropNode::E_CoverNodeType C_DgCoverNodePropNode::GetCoverNodeType() const
{
	return m_coverNodeType;
}

const I_DgNode* C_DgCoverNodePropNode::GetLinkedNode() const
{
	I_Plug* pTargetPlug = m_pTargetWorldMatrixPlug->GetSourceConnection();
	return pTargetPlug ? (I_DgNode*)PlugHelpers::GetTopLevelParent(*pTargetPlug) : 0;
}

void C_DgCoverNodePropNode::OnTargetPlugChanged(I_Plug* pPlug)
{
	BIZ_ASSERT(pPlug);
	//Set as dirty due to something interesting happening to the node we're looking at.
	I_DgNode* pParent = PlugHelpers::GetTopLevelParent(*pPlug);
	BIZ_ASSERT(pParent->IsA(E_ObjType_DgCoverNodePropNode));
	((C_DgCoverNodePropNode*)pParent)->m_coverNodeDirtyFlags.targetChanged = true;
	pParent->DirtyDependents(false);
}

}

