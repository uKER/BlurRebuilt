#include "stdafx.h"

#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Box/DgBoxPropNode.h>

void RegisterBoxPropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgBoxPropNode)
}

namespace EditorLib
{

//From I_XtObject
C_Status C_DgBoxPropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* _pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, _pData);
	if (!status) return status;

	Xt::DomBlock* pData = Xt::AsBlock(_pData);

	{
		Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pData, "Orientation"));
		if (XtFieldHelper::IsNullMatrix(pBlock))
		{
			pBlock = Xt::AcquireBlock(pData, "Orientation");
			Matrix44 m;
			UtMaths::SetIdentity(&m);
			XtFieldHelper::SetMatrix(pBlock, m);
			PlugHelpers::SetRotationMatrix(
				m,
				(C_NullPlug&)*m_staticPlugs[E_StaticPlug_Rotate],
				E_RotationOrder_XYZ,
				false);
		}
	}	

	{
		Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pData, "Size"));
		if (XtFieldHelper::IsNullVector(pBlock))
		{
			pBlock = Xt::AcquireBlock(pData, "Size");
			Vector3 size(1,1,1);
			XtFieldHelper::SetVector(pBlock, size);
			PlugHelpers::SetCompoundValue(GetSizePlug(), size, false);
		}
	}

	return E_Status_Success;
}


//From I_XtObject
E_ObjType C_DgBoxPropNode::GetType() const
{
	return E_ObjType_DgBoxPropNode;
}

const char* C_DgBoxPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

bool C_DgBoxPropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgBoxPropNode::GetType()) 
		return true;

	return false;
}

//From I_DgNode
bool C_DgBoxPropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgBoxPropNode::GetStaticType(), pHc);

	Xt::DomBlock* pBlockData = Xt::AsBlock(GetXtData());
	if (!pBlockData) return false;

	{
		Xt::DomBlock* pBlock;
		XT_HELPERS_AT_BLOCK_CHECKED(pBlock, pBlockData, "Orientation");
		if (pBlock && GetXtDirty().IsDirty(pBlock))
		{
			XtFieldHelper::GetMatrix(m_localMatrix, pBlock);
			PlugHelpers::SetRotationMatrixComponents(
				(C_NullPlug&)*m_staticPlugs[E_StaticPlug_Rotate],
				E_RotationOrder_XYZ,
				m_localMatrix,
				false); //Don't trigger update.
		}
	}
	{
		Xt::DomBlock* pBlock;
		XT_HELPERS_AT_BLOCK_CHECKED(pBlock, pBlockData, "Size");
		if (pBlock && GetXtDirty().IsDirty(pBlock))
		{
			Vector3 size;
			XtFieldHelper::GetVector(size, pBlock);
			PlugHelpers::SetCompoundValue(GetSizePlug(), size, false);  //Don't trigger update.
		}
	}

	return __super::Update(&signaller.GetCollection());
}

void C_DgBoxPropNode::GetPlugs(Btl::Vector<I_Plug*>& plugs)
{
	__super::GetPlugs(plugs);
	for (U32 i = 0; i < E_StaticPlug_Count; ++i)
		plugs.PushBack(m_staticPlugs[i]);
}


//From C_DgXtPropNodeBase
bool C_DgBoxPropNode::IsVisible() const
{
	return true;
}


//From C_DgBoxPropNode
const char* C_DgBoxPropNode::GetStaticTypeName()
{
	//Must match name in Xt schema exactly!
	static char* p = "BoundingBox";
	return p;
}

const char* C_DgBoxPropNode::GetStaticDisplayName()
{
	static char* p = "Box Property";
	return p;
}

E_ObjType C_DgBoxPropNode::GetStaticType()
{
	return E_ObjType_DgBoxPropNode;
}

void* C_DgBoxPropNode::Create()
{
	return new C_DgBoxPropNode();
}

const Matrix44& C_DgBoxPropNode::GetLocalMatrix() const
{
	return m_localMatrix;
}

C_NullPlug& C_DgBoxPropNode::GetSizePlug()
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Size];
}

const C_NullPlug& C_DgBoxPropNode::GetSizePlug() const
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Size];
}

C_NullPlug& C_DgBoxPropNode::GetRotatePlug()
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Rotate];
}

const C_NullPlug& C_DgBoxPropNode::GetRotatePlug() const
{
	return (C_NullPlug&)*m_staticPlugs[E_StaticPlug_Rotate];
}


//From C_XtObject
void C_DgBoxPropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
	Xt::DomBlock* pBlockData = Xt::AsBlock(GetXtData());
	BIZ_ASSERT(pBlockData);

	{
		Xt::DomBlock* pBlock;
		XT_HELPERS_AT_BLOCK_CHECKED(pBlock, pBlockData, "Orientation");
		if (XtHelpers::AreRelated(pBlock, pEvent->from))
			GetXtDirty().SetDirty(pBlock);
	}
	{
		Xt::DomBlock* pBlock;
		XT_HELPERS_AT_BLOCK_CHECKED(pBlock, pBlockData, "Size");
		if (XtHelpers::AreRelated(pBlock, pEvent->from))
			GetXtDirty().SetDirty(pBlock);
	}
}


//From C_DgBoxPropNode
C_DgBoxPropNode::C_DgBoxPropNode()
:
C_DgXtPropNodeBase()
{
	m_staticPlugs[E_StaticPlug_Rotate]	= PlugHelpers::CreateCompoundVec3(
		this, &C_DgBoxPropNode::OnPlugChanged, E_PlugType_Input, "rotate");
	m_staticPlugs[E_StaticPlug_Size]	= PlugHelpers::CreateCompoundVec3(
		this, &C_DgBoxPropNode::OnPlugChanged, E_PlugType_Input, "size");
}

C_DgBoxPropNode::~C_DgBoxPropNode()
{
	for (S32 i = 0; i < E_StaticPlug_Count; i++)
		delete m_staticPlugs[i];
}

//From C_DgBoxPropNode
void C_DgBoxPropNode::OnPlugChanged(I_Plug* pPlug)
{
	C_DgBoxPropNode* pThis = (C_DgBoxPropNode*)PlugHelpers::GetTopLevelParent(*pPlug);
	if (!pThis->GetXtData()) 
		return;

	Xt::DomBlock* pBlockData = Xt::AsBlock(pThis->GetXtData());
	BIZ_ASSERT(pBlockData);
	
	if (pPlug->GetParentPlug() == pThis->m_staticPlugs[E_StaticPlug_Rotate])
	{
		Xt::DomBlock* pBlock = Xt::AcquireBlock(pBlockData, "Orientation");

		Matrix44 m;
		PlugHelpers::SetRotationMatrix(
			m,
			*(C_NullPlug*)pPlug->GetParentPlug(),
			E_RotationOrder_XYZ);
		XtFieldHelper::SetMatrix(pBlock, m);
		pThis->DirtyDependents(false);
	}
	else if (pPlug->GetParentPlug() == pThis->m_staticPlugs[E_StaticPlug_Size])
	{
		Xt::DomBlock* pBlock = Xt::AcquireBlock(pBlockData, "Size");

		Vector3 size;
		EditorLib::PlugHelpers::GetCompoundValue(*(C_NullPlug*)pPlug->GetParentPlug(), size);
		XtFieldHelper::SetVector(pBlock, size);
		pThis->DirtyDependents(false);
	}
}

}
