#include "stdafx.h"

#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Rectangle/DgRectanglePropNode.h>

void RegisterRectanglePropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgRectanglePropNode)
}

namespace EditorLib
{

// From C_XtObject
C_Status C_DgRectanglePropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* _pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, _pData);
	if (!status) return status;

	Xt::DomBlock* pData = Xt::AsBlock(_pData);

	Xt::DomField* pField = Xt::AsField(Xt::At(pData, "Width"));
	if (XtFieldHelper::IsNullScalar(pField))
	{
		pField = Xt::AcquireField(pData, "Width");
		F32 width(1.f);
		XtFieldHelper::SetScalar(pField, width);
		GetWidthPlug().SetValue(width);
	}

	pField = Xt::AsField(Xt::At(pData, "Height"));
	if (XtFieldHelper::IsNullScalar(pField))
	{
		pField = Xt::AcquireField(pData, "Height");
		F32 height(1.f);
		XtFieldHelper::SetScalar(pField, height);
		GetHeightPlug().SetValue(height);
	}

	return E_Status_Success;
}

// From I_DgObject
E_ObjType C_DgRectanglePropNode::GetType() const
{
	return E_ObjType_DgRectanglePropNode;
}

// From I_DgObject
const char* C_DgRectanglePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

// From I_DgObject
bool C_DgRectanglePropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgRectanglePropNode::GetType()) 
		return true;

	return false;
}

//From I_DgNode
bool C_DgRectanglePropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgRectanglePropNode::GetStaticType(), pHc);

	if (GetXtDirty().IsDirty())
	{
		Xt::DomBlock* pRoot = Xt::AsBlock(GetXtData());

		{
			Xt::DomField* pField;
			XT_HELPERS_AT_FIELD_CHECKED(pField, pRoot, "Width");
			if (pField && GetXtDirty().IsDirty(pField))
			{
				F32 width;
				XtFieldHelper::GetScalar(width, pField);
				GetWidthPlug().SetValue(width);
			}
		}

		{
			Xt::DomField* pField;
			XT_HELPERS_AT_FIELD_CHECKED(pField, pRoot, "Height");
			if (pField && GetXtDirty().IsDirty(pField))
			{
				F32 height;
				XtFieldHelper::GetScalar(height, pField);
				GetHeightPlug().SetValue(height);
			}
		}
	}

	return __super::Update(&signaller.GetCollection());
}

// From I_DgNode
void C_DgRectanglePropNode::GetPlugs(Btl::Vector<I_Plug*>& plugs)
{
	__super::GetPlugs(plugs);
	for (U32 i = 0; i < E_StaticPlug_Count; ++i)
		plugs.PushBack(m_staticPlugs[i]);
}

// From C_DgXtPropNodeBase
bool C_DgRectanglePropNode::IsVisible() const
{
	return true;
}

// From C_DgRectanglePropNode
const char* C_DgRectanglePropNode::GetStaticTypeName()
{
	//Must match name in Xt schema exactly!
	static char* p = "Rectangle";
	return p;
}

const char* C_DgRectanglePropNode::GetStaticDisplayName()
{
	static char* p = "Rectangle Property";
	return p;
}

// From C_DgRectanglePropNode
E_ObjType C_DgRectanglePropNode::GetStaticType()
{
	return E_ObjType_DgRectanglePropNode;
}

// From C_DgRectanglePropNode
void* C_DgRectanglePropNode::Create()
{
	return new C_DgRectanglePropNode();
}

// From C_DgRectanglePropNode
const Matrix44& C_DgRectanglePropNode::GetLocalMatrix() const
{
	return m_localMatrix;
}

// From C_DgRectanglePropNode
C_StaticPlug<F32>& C_DgRectanglePropNode::GetWidthPlug()
{
	return (C_StaticPlug<F32>&)*m_staticPlugs[E_StaticPlug_Width];
}

// From C_DgRectanglePropNode
const C_StaticPlug<F32>& C_DgRectanglePropNode::GetWidthPlug() const
{
	return (const C_StaticPlug<F32>&)*m_staticPlugs[E_StaticPlug_Width];
}

// From C_DgRectanglePropNode
C_StaticPlug<F32>& C_DgRectanglePropNode::GetHeightPlug()
{
	return (C_StaticPlug<F32>&)*m_staticPlugs[E_StaticPlug_Height];
}

// From C_DgRectanglePropNode
const C_StaticPlug<F32>& C_DgRectanglePropNode::GetHeightPlug() const
{
	return (const C_StaticPlug<F32>&)*m_staticPlugs[E_StaticPlug_Height];
}

// From C_XtObject
void C_DgRectanglePropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);

	Xt::DomBlock* pBlockData = Xt::AsBlock(GetXtData());
	BIZ_ASSERT(pBlockData);

	{
		Xt::DomField* pField;
		XT_HELPERS_AT_FIELD_CHECKED(pField, pBlockData, "Width");
		if (XtHelpers::AreRelated(pField, pEvent->from))
		{
			GetXtDirty().SetDirty(pField);
		}
	}

	{
		Xt::DomField* pField;
		XT_HELPERS_AT_FIELD_CHECKED(pField, pBlockData, "Height");
		if (XtHelpers::AreRelated(pField, pEvent->from))
		{
			GetXtDirty().SetDirty(pField);
		}
	}
}

// From C_DgRectanglePropNode
C_DgRectanglePropNode::C_DgRectanglePropNode()
:
C_DgXtPropNodeBase()
{
	m_staticPlugs[E_StaticPlug_Width]	= PlugHelpers::CreateScalar(this, C_DgRectanglePropNode::OnPlugChanged, E_PlugType_Input, "width", 1.f);
	m_staticPlugs[E_StaticPlug_Height]	= PlugHelpers::CreateScalar(this, C_DgRectanglePropNode::OnPlugChanged, E_PlugType_Input, "height", 1.f);
}

// From C_DgRectanglePropNode
C_DgRectanglePropNode::~C_DgRectanglePropNode()
{
	for (S32 i = 0; i < E_StaticPlug_Count; i++)
		delete m_staticPlugs[i];
}

// From C_DgRectanglePropNode
void C_DgRectanglePropNode::OnPlugChanged(I_Plug* pPlug)
{
	BIZ_ASSERT(pPlug);

	C_DgRectanglePropNode* pThis = (C_DgRectanglePropNode*)PlugHelpers::GetTopLevelParent(*pPlug);
	if (!pThis->GetXtData()) 
		return;

	Xt::DomBlock* pData = Xt::AsBlock(pThis->GetXtData());
	BIZ_ASSERT(pData);

	if (pPlug == pThis->m_staticPlugs[E_StaticPlug_Width])
	{
		Xt::DomField* pField = Xt::AcquireField(pData, "Width");
		F32 width;
		((C_StaticPlug<F32>*)pPlug)->GetValue(width);
		XtFieldHelper::SetScalar(pField, width);
		pThis->DirtyDependents();
	} 
	else if (pPlug == pThis->m_staticPlugs[E_StaticPlug_Height])
	{
		Xt::DomField* pField = Xt::AcquireField(pData, "Height");
		F32 height;
		((C_StaticPlug<F32>*)pPlug)->GetValue(height);
		XtFieldHelper::SetScalar(pField, height);
		pThis->DirtyDependents();
	}
}

}

