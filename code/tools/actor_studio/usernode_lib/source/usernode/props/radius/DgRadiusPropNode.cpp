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

#include <UserNode/Props/Radius/DgRadiusPropNode.h>

void RegisterRadiusPropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgRadiusPropNode)
}

namespace EditorLib
{
//from C_XtObject
C_Status C_DgRadiusPropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	//hard coded defaults
	{
		Xt::DomBlock* pRoot = Xt::AsBlock(GetXtData());
		Xt::DomArray* pRadiiArray;
		XT_HELPERS_AT_ARRAY_CHECKED(pRadiiArray, pRoot, "radii");

		if (Xt::Count(pRadiiArray) == 0)
		{
			LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;

			const F32 r[3] = { 10.f, 20.f, 30.f };
			const Maths::C_RGBA_U8 c[3] = { Maths::C_RGBA_U8(0x8000ff00), Maths::C_RGBA_U8(0x80ffff00), Maths::C_RGBA_U8(0x80ff0000) };

			for (S32 i=0; i<3; ++i)
			{
				Xt::DomComponent* pCompData = new Xt::DomComponent;
				Xt::SetType(pCompData, "AsRadius");
				SetDefaults(pSchema, AsStruct(pSchema->types["AsRadius"]), pCompData);	

				XtFieldHelper::SetScalar(Xt::AsField(Xt::At(&pCompData->data, "radius")), r[i]);
				XtFieldHelper::SetColour(Xt::AsBlock(Xt::At(&pCompData->data, "colour")), c[i]);

				Xt::Append(pRadiiArray, pCompData);
			}
		}
	}

	return status;
}

//from I_DgObject
E_ObjType C_DgRadiusPropNode::GetType() const
{
	return E_ObjType_DgRadiusPropNode;
}

const char* C_DgRadiusPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

bool C_DgRadiusPropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgRadiusPropNode::GetType()) 
		return true;

	return false;
}


//From I_DgNode
bool C_DgRadiusPropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgRadiusPropNode::GetStaticType(), pHc);

	if (GetXtDirty().IsDirty())
	{
		Xt::DomBlock* pRoot = Xt::AsBlock(GetXtData());
		Xt::DomArray* pRadiiArray;
		XT_HELPERS_AT_ARRAY_CHECKED(pRadiiArray, pRoot, "radii");
		if (pRadiiArray && GetXtDirty().IsDirty(pRadiiArray))
		{
			m_radiusPlugs.Clear();
			for (Xt::DomArray::IndexIterator it(pRadiiArray); it; ++it)
			{

				Xt::DomComponent* pXtItemComponent = Xt::AsComponent(it);
				if (!pXtItemComponent)
					continue;

				F32 radius = 0.f;
				Maths::C_RGBA_U8 colour(0);

				XtFieldHelper::GetScalar(radius, Xt::AsField(Xt::At(&pXtItemComponent->data, "radius")));
				XtFieldHelper::GetColour(colour, Xt::AsBlock(Xt::At(&pXtItemComponent->data, "colour")));
				
				RadiusPlug plugs = 
				{
					C_StaticPlug<F32>(this, &C_DgRadiusPropNode::OnPlugChanged, E_PlugType_Input, "radius"),
					C_StaticPlug<Maths::C_RGBA_U8>(this, &C_DgRadiusPropNode::OnPlugChanged, E_PlugType_Input, "colour"),
				};

				plugs.radiusPlug.SetValue(radius, false);
				plugs.colourPlug.SetValue(colour, false);

				m_radiusPlugs.PushBack(plugs);
			}
		}
	}

	return __super::Update(&signaller.GetCollection());
}


void C_DgRadiusPropNode::GetPlugs(Btl::Vector<I_Plug*>& plugs)
{
	__super::GetPlugs(plugs);
}

bool C_DgRadiusPropNode::IsVisible() const
{
	return true;
}

//From C_DgRadiusPropNode
const char* C_DgRadiusPropNode::GetStaticTypeName()
{
	static char* p = "AsRadiusArray";
	return p;
}

const char* C_DgRadiusPropNode::GetStaticDisplayName()
{
	static char* p = "Radius Property";
	return p;
}

E_ObjType C_DgRadiusPropNode::GetStaticType()
{
	return E_ObjType_DgRadiusPropNode;
}

void* C_DgRadiusPropNode::Create()
{
	return new C_DgRadiusPropNode();
}

U32 C_DgRadiusPropNode::GetNumRadii() const
{
	return m_radiusPlugs.Size();
}

C_StaticPlug<F32>& C_DgRadiusPropNode::GetRadiusPlug(U32 index)
{
	BIZ_ASSERT(index < (U32)m_radiusPlugs.Size());
	return m_radiusPlugs[index].radiusPlug;
}

const C_StaticPlug<F32>& C_DgRadiusPropNode::GetRadiusPlug(U32 index) const
{
	BIZ_ASSERT(index < (U32)m_radiusPlugs.Size());
	return m_radiusPlugs[index].radiusPlug;
}

C_StaticPlug<Maths::C_RGBA_U8>& C_DgRadiusPropNode::GetColourPlug(U32 index)
{
	BIZ_ASSERT(index < (U32)m_radiusPlugs.Size());
	return m_radiusPlugs[index].colourPlug;
}

const C_StaticPlug<Maths::C_RGBA_U8>& C_DgRadiusPropNode::GetColourPlug(U32 index) const
{
	BIZ_ASSERT(index < (U32)m_radiusPlugs.Size());
	return m_radiusPlugs[index].colourPlug;
}


void C_DgRadiusPropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);

	if (pEvent->code = Xt::kDomRefChanged)
		GetXtDirty().SetDirty(GetXtData());
}


C_DgRadiusPropNode::C_DgRadiusPropNode()
: C_DgXtPropNodeBase()
{
}

C_DgRadiusPropNode::~C_DgRadiusPropNode()
{
}

void C_DgRadiusPropNode::OnPlugChanged(I_Plug* pPlug)
{
	BIZ_ASSERT(pPlug);
	//Set as dirty due to something interesting happening to the node we're looking at.
	I_DgNode* pParent = PlugHelpers::GetTopLevelParent(*pPlug);
	BIZ_ASSERT(pParent->IsA(E_ObjType_DgRadiusPropNode));

	C_DgRadiusPropNode* pThis = (C_DgRadiusPropNode*)PlugHelpers::GetTopLevelParent(*pPlug);
	if (!pThis->GetXtData()) 
		return;

	Xt::DomBlock *pPropData = Xt::AsBlock(pThis->GetXtData());
	Xt::DomArray *pArrayData = Xt::AsArray(Xt::At(pPropData, "radii"));
	BIZ_ASSERT(pArrayData);

	BIZ_ASSERT(Xt::Count(pArrayData) == pThis->m_radiusPlugs.Size());

	for (S32 i=0; i<pThis->m_radiusPlugs.Size(); ++i)
	{
		if (pPlug == &pThis->m_radiusPlugs[i].radiusPlug)
		{
			Xt::DomComponent *pComponent = Xt::AsComponent(pArrayData->elements[pArrayData->index[i]]);
			if (pComponent)
			{
				C_StaticPlug<F32>* pRadiusPlug = (C_StaticPlug<F32>*)pPlug;
				XtFieldHelper::SetScalar(Xt::AsField(Xt::At(&pComponent->data, "radius")), pRadiusPlug->GetValue());
				pParent->DirtyDependents(false);
				break;
			}
		}
		else if (pPlug == &pThis->m_radiusPlugs[i].colourPlug)
		{
			Xt::DomComponent *pComponent = Xt::AsComponent(pArrayData->elements[pArrayData->index[i]]);
			if (pComponent)
			{
				C_StaticPlug<Maths::C_RGBA_U8>* pColourPlug = (C_StaticPlug<Maths::C_RGBA_U8>*)pPlug;
				XtFieldHelper::SetColour(Xt::AsBlock(Xt::At(&pComponent->data, "colour")), pColourPlug->GetValue());
				pParent->DirtyDependents(false);
				break;
			}
		}
	}

	
}

}
