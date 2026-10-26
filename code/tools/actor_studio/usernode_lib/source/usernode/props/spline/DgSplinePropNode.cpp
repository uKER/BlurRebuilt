#include "stdafx.h"
#include <algorithm>
#include <Misc_Shared/NewCurve/Definitions.h>
#include <Misc_Shared/NewCurve/CurveHelpers.h>
#include <Misc_Shared/NewCurve/CurveDataHelpers.h>
#include <Misc_Shared/NewCurve/Reparameterise.h>
#include <Misc_Shared/NewCurve/CurveData.h>
#include <Misc_Shared/NewCurve/Curve.h>
#include <Misc_Shared/NewCurve/HermitePolicies.h>
#include <Misc_Shared/NewCurve/HermiteSpline.h>

#include <EditorLib/Collision/Collision.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Spline/DgSplinePropNode.h>

void RegisterSplinePropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgSplinePropNode)
}

namespace EditorLib
{

// From C_XtObject
C_Status C_DgSplinePropNode::XtInitialise(
	C_XtLevel* pLevel,
	LarvaType* pLarvaType,
	Xt::DomElement* pData)
{
	C_Status status = __super::XtInitialise(pLevel, pLarvaType, pData);
	if (!status) return status;

	C_CollisionScene& collisionScene = 
		C_Core::Instance().GetCollisionManager().GetGlobalCollisionScene();

	assert(!m_pCollisionInstance);
	m_pCollisionSpline = new C_CollisionSpline;
	m_pCollisionSpline->pVertices = 0;
	m_pCollisionSpline->pTangents = 0;
	m_pCollisionSpline->nVertices = 0;
	collisionScene.AddCollisionSource(m_pCollisionSpline);

	m_pCollisionInstance = new C_InternalCollisionInstance;
	{
		//Initialise instance. 
		UtMaths::SetIdentity(&m_pCollisionInstance->toWorldSpace);
		UtMaths::SetIdentity(&m_pCollisionInstance->toObjectSpace);
		m_pCollisionInstance->pSource = m_pCollisionSpline;
	}
	collisionScene.AddCollisionInstance(this, m_pCollisionInstance);

	//Initialise to default as required.
	//TODO: set defaults from schema.
	{
		LarvaSchema* pSchema = C_Core::Instance().GetXtManager().GetDb()->schema;
		Xt::DomBlock* pDataBlock = Xt::AsBlock(pData);

		if (!pDataBlock)
			return E_Status_Failure;

		//Create closed field.
		Xt::AcquireField(pDataBlock, "closed");

		//Create data array.
		if (!Xt::At(pDataBlock, "knots"))
		{
			Xt::AcquireArray(pDataBlock, "knots");
		}
	}

	return E_Status_Success;
}

// From I_DgObject
E_ObjType C_DgSplinePropNode::GetType() const
{
	return E_ObjType_DgSplinePropNode;
}

// From I_DgObject
const char* C_DgSplinePropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

// From I_DgObject
bool C_DgSplinePropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgSplinePropNode::GetType()) 
		return true;

	return false;
}

//From I_DgNode
bool C_DgSplinePropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgSplinePropNode::GetStaticType(), pHc);
	if (!GetXtDirty().IsDirty()) return true;

	Xt::DomBlock* pData = Xt::AsBlock(GetXtData());

	//Movement resolution...
	{
		Xt::DomBlock* pData = Xt::AsBlock(GetXtData());
		Xt::DomField* pField = Xt::AsField(Xt::At(pData, "reparamRes"));
		Curve::E_ReparamRes res = Curve::E_ReparamRes_Default;
		if (pField)
		{
			for (U32 i = 0; i < Curve::E_ReparamRes_Count; ++i)
				if (!stricmp(pField->value.Text(), Curve::GetReparamString(i)))
				{
					res = (Curve::E_ReparamRes)i;
					break;
				}
		}
		switch(res)
		{
		case Curve::E_ReparamRes_None:
			m_reparamRes = 0;
			break;
		case Curve::E_ReparamRes_Low:
			m_reparamRes = Curve::kReparamResLow;
			break;
		case Curve::E_ReparamRes_Medium:
			m_reparamRes = Curve::kReparamResMedium;
			break;
		case Curve::E_ReparamRes_High:
			m_reparamRes = Curve::kReparamResHigh;
			break;
		case Curve::E_ReparamRes_Default:
		case Curve::E_ReparamRes_Unknown:
		default:
			m_reparamRes = Curve::kReparamResDefault;
			break;
		}
	}

	Xt::DomField* pClosedField = Xt::AsField(Xt::At(pData, "closed"));
	if (pClosedField && GetXtDirty().IsDirty(pClosedField))
		XtFieldHelper::GetBool(m_isSplineClosed, pClosedField, false);

	Xt::DomArray* pKnotsArray;
	XT_HELPERS_AT_ARRAY_CHECKED(pKnotsArray, pData, "knots");
	if (pKnotsArray && (GetXtDirty().IsDirty(pKnotsArray) || GetXtDirty().IsDirty(pClosedField)))
	{
		Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = m_pCurveData->GetKnots();
		knots.Clear();

		U32 nKnots = m_isSplineClosed ? Xt::Count(pKnotsArray)+1 : Xt::Count(pKnotsArray);
		knots.Resize(nKnots);

		for (U32 i=0; i<nKnots; ++i)
		{
			Xt::DomArray::IndexIterator it(pKnotsArray);
			it.index = i % Xt::Count(pKnotsArray);

			Xt::DomComponent* pXtItemComponent = Xt::AsComponent(it);
			if (!pXtItemComponent)
				continue;

			Xt::DomBlock* pXtItemBlock = &pXtItemComponent->data;
			if (!pXtItemBlock)
				continue;

			{
				Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pXtItemBlock, "position"));
				Vector3 fieldVector(0,0,0);
				if (pBlock)	XtFieldHelper::GetVector(fieldVector, pBlock);
		
				Maths::HVec3& position = knots[i].value;
				position.X(fieldVector.X());
				position.Y(fieldVector.Y());
				position.Z(fieldVector.Z());
			}

			{
				Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pXtItemBlock, "tangentIn"));
				Vector3 fieldVector(0,0,0);
				if (pBlock)	XtFieldHelper::GetVector(fieldVector, pBlock);
		
				Maths::HVec3& position = knots[i].tangentIn;
				position.X(fieldVector.X());
				position.Y(fieldVector.Y());
				position.Z(fieldVector.Z());
			}

			{
				Xt::DomBlock* pBlock = Xt::AsBlock(Xt::At(pXtItemBlock, "tangentOut"));
				Vector3 fieldVector(0,0,0);
				if (pBlock)	XtFieldHelper::GetVector(fieldVector, pBlock);
		
				Maths::HVec3& position = knots[i].tangentOut;
				position.X(fieldVector.X());
				position.Y(fieldVector.Y());
				position.Z(fieldVector.Z());
			}

			Xt::DomField* pField = Xt::AsField(Xt::At(pXtItemBlock, "tangentScale"));
			F32 tangentScale(1.f);
			if (pField)	XtFieldHelper::GetScalar(tangentScale, pField);
			knots[i].tangentScale = tangentScale;
		}
	}

	m_pCurve->Process();
	OnSplineChanged();
	return __super::Update(&signaller.GetCollection());
}

// From C_DgXtPropNodeBase
bool C_DgSplinePropNode::IsVisible() const
{
	return true;
}

// From C_DgSplinePropNode
const char* C_DgSplinePropNode::GetStaticTypeName()
{
	//Must match name in Xt schema exactly!
	static char* p = "SplineVec3";
	return p;
}

const char* C_DgSplinePropNode::GetStaticDisplayName()
{
	static char* p = "Spline Property";
	return p;
}

// From C_DgSplinePropNode
E_ObjType C_DgSplinePropNode::GetStaticType()
{
	return E_ObjType_DgSplinePropNode;
}

// From C_DgSplinePropNode
void* C_DgSplinePropNode::Create()
{
	return new C_DgSplinePropNode();
}

// From C_DgSplinePropNode
C_Status C_DgSplinePropNode::XtSave()
{
	Xt::DomBlock* pBlock = Xt::AsBlock(GetXtData());
	if (!pBlock)
		return E_Status_Failure;

	Xt::DomArray* pXtKnots = Xt::AcquireArray(pBlock, "knots");
	if (!pXtKnots)
		return E_Status_Failure;

	const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = m_pCurveData->GetKnots();
	
	S32 nKnots = Xt::Count(pXtKnots);
	assert(nKnots == (m_isSplineClosed ? knots.Size()-1 : knots.Size()));

	for (S32 i = 0; i < knots.Size(); i++)
	{
		Xt::DomComponent* pXtItemComponent = Xt::AcquireComponent(pXtKnots,i);
		if (!pXtItemComponent)
			continue;

		Xt::DomBlock* pXtItemBlock = &pXtItemComponent->data;

		Xt::DomBlock* pBlock = Xt::AcquireBlock(pXtItemBlock, "position");
		if (!pBlock) return E_Status_Failure;
		XtFieldHelper::SetVector(pBlock, knots[i].value);

		pBlock = Xt::AcquireBlock(pXtItemBlock, "tangentIn");
		if (!pBlock) return E_Status_Failure;
		XtFieldHelper::SetVector(pBlock, knots[i].tangentIn);

		pBlock = Xt::AcquireBlock(pXtItemBlock, "tangentOut");
		if (!pBlock) return E_Status_Failure;
		XtFieldHelper::SetVector(pBlock, knots[i].tangentOut);

		Xt::DomField* pField = Xt::AcquireField(pXtItemBlock, "tangentScale");
		if (!pField)return E_Status_Failure;
		XtFieldHelper::SetScalar(pField, knots[i].tangentScale);
	}

	return E_Status_Success;
}

// From C_DgSplinePropNode
C_Status C_DgSplinePropNode::Apply()
{
	return XtSave();
}

// From C_DgSplinePropNode
C_Status C_DgSplinePropNode::AddPoint(const Maths::HVec3& pos, F32 tangentScale)
{
	C_XtObject::C_Unbinder unbinder(this);
	GetXtDirty().SetDirty(GetXtData());

	Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = m_pCurveData->GetKnots();

	knots.PushBack();
	knots.Back().value = pos;
	knots.Back().tangentScale = 1.f;
	S32 idx = knots.Size()-1;

	if (knots.Size() == 1)
	{
		knots.Back().tangentIn = Maths::HVec3(0,0,0);
		knots.Back().tangentOut = Maths::HVec3(0,0,0);
	}
	else
	{
		//A bit hacky, but fudge our tangent lengths to make creation more intuitive.
		m_pCurve->CalculateKnotTangents(idx-1);
		m_pCurve->CalculateKnotTangents(idx);
		F32 kMagicFudge = GetDefaultTangentScale();
		knots[idx-1].tangentIn = knots[idx-1].tangentIn * kMagicFudge;
		knots[idx-1].tangentOut = knots[idx-1].tangentOut * kMagicFudge;
		knots[idx].tangentIn = knots[idx].tangentIn * kMagicFudge;
		knots[idx].tangentOut = knots[idx].tangentOut * kMagicFudge;
	}

	m_pCurve->Process();
	OnSplineChanged();

	// Acquire array
	Xt::DomBlock* pData = Xt::AsBlock(GetXtData());
	Xt::DomArray* pXtKnots = Xt::AcquireArray(pData, "knots");
	
	//fix up previous knot tangents
	if (knots.Size() > 1)
	{
		Xt::DomComponent *pXtItemComponent = Xt::AsComponent(Xt::At(pXtKnots, idx-1));
		if (pXtItemComponent)
		{
			Xt::DomBlock *pXtItemBlock = &pXtItemComponent->data;
			if (pXtItemComponent)
			{
				Xt::DomBlock *pBlock = 0;
				pBlock = Xt::AcquireBlock(pXtItemBlock, "tangentIn");
				if (pBlock) XtFieldHelper::SetVector(pBlock, knots[idx-1].tangentIn);

				pBlock = Xt::AcquireBlock(pXtItemBlock, "tangentOut");
				if (pBlock) XtFieldHelper::SetVector(pBlock, knots[idx-1].tangentIn);

				Xt::DomField* pField = Xt::AcquireField(pXtItemBlock, "tangentScale");
				XtFieldHelper::SetScalar(pField, knots[idx-1].tangentScale);
			}
		}
	}

	//append new knot data
	{
		// Create block
		Xt::DomComponent* pXtItemComponent = new Xt::DomComponent;
		Xt::DomBlock* pXtItemBlock = &pXtItemComponent->data;
			
		// Set data
		Xt::DomBlock* pBlock = Xt::AcquireBlock(pXtItemBlock, "position");

		XtFieldHelper::SetVector(pBlock, knots.Back().value);

		pBlock = Xt::AcquireBlock(pXtItemBlock, "tangentIn");
		XtFieldHelper::SetVector(pBlock, knots.Back().tangentIn);

		pBlock = Xt::AcquireBlock(pXtItemBlock, "tangentOut");
		XtFieldHelper::SetVector(pBlock, knots.Back().tangentOut);

		Xt::DomField* pField = Xt::AcquireField(pXtItemBlock, "tangentScale");
		XtFieldHelper::SetScalar(pField, knots.Back().tangentScale);

		// Append array data
		Xt::Append(pXtKnots, pXtItemComponent);
	}
	return E_Status_Success;
}

// From C_DgSplinePropNode
C_Status C_DgSplinePropNode::DeletePoints(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds)
{
	Xt::DomBlock* pData = Xt::AsBlock(GetXtData());
	Xt::DomArray* pXtKnots = Xt::AcquireArray(pData, "knots");
	if (!pXtKnots || componentIds.Size() == 0)
		return E_Status_Failure;

	Btl::Set<U32> sortedKnotIndices;
	for (S32 i = 0; i < componentIds.Size(); ++i)
	{
		U32 knotIndex = GetComponentKnotIndex(componentIds[i]);
		sortedKnotIndices.Insert(knotIndex);
	}
	{
		C_XtObject::C_Unbinder unbinder(this);
		GetXtDirty().SetDirty(GetXtData());
		for (Btl::Set<U32>::ReverseIterator it = sortedKnotIndices.ReverseBegin(); 
			it != sortedKnotIndices.ReverseEnd(); ++it)
		{
			U32 knotIndex = *it;
			Xt::Remove(pXtKnots,knotIndex);
		}
	}

	return E_Status_Success;
}

// From C_DgSplinePropNode
Ut::Vector<U32> C_DgSplinePropNode::InsertPoints(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds)
{
	C_ViewDisabler viewDisabler;
	Ut::Vector<U32> addedComponents;

	Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = m_pCurveData->GetKnots();

	int insertedPoints = 0;

	Ut::UtVector< Ut::DynamicVector<U32> > sortedComponents = componentIds;
	std::sort(sortedComponents.Begin(), sortedComponents.End());

	// Acquire array
	Xt::DomBlock* pData = Xt::AsBlock(GetXtData());
	Xt::DomArray* pXtKnots = Xt::AcquireArray(pData, "knots");

	for (S32 i=1; i<sortedComponents.Size(); i++) 
	{
		//Find adjacent knots.
		S32 knot0 = GetComponentKnotIndex(sortedComponents[i-1]);
		S32 knot1 = GetComponentKnotIndex(sortedComponents[i]);
		if (abs((S32)(knot1-knot0)) == 1)
		{
			Curve::C_HermiteSplineKnot<Maths::HVec3> newKnot;

			S32 newIdx = knot1+insertedPoints;
			F32 uA = knots[knot0+insertedPoints].GetU(knot0+insertedPoints, knots.Size());
			F32 uB = knots[newIdx].GetU(newIdx, knots.Size());
			F32 newKnotU = (uA+uB)/2.f;
			newKnot.value = m_pCurve->Evaluate(newKnotU);
			newKnot.tangentScale = 1.f;
			newKnot.tangentIn = newKnot.tangentOut = m_pCurve->EvaluateTangent(newKnotU);
			newKnot.tangentIn.Normalise();
			newKnot.tangentOut.Normalise();
			F32 scale = (knots[knot0].value - knots[knot1].value).Length() / 3.f;
			newKnot.tangentOut = newKnot.tangentOut * scale;
			newKnot.tangentIn = newKnot.tangentIn * scale;

			{
				Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >::Iterator it = knots.Begin();
				it = it + newIdx;
				knots.InsertBefore(newKnot, it);
			}

			// Create block
			Xt::DomComponent* pXtItemComponent = new Xt::DomComponent;
	
			// Append array data
			Xt::Insert(pXtKnots, newIdx, pXtItemComponent);	

			// First tangent index.
			U32 firstComponentIndex = GetKnotComponentIndex(newIdx);

			// Adjust to knot index.
			addedComponents.PushBack(firstComponentIndex+1);
			insertedPoints++;
		}
	}
	if (addedComponents.Size() > 0)
		XtSave();
	return addedComponents;
}

// From C_DgSplinePropNode
void C_DgSplinePropNode::Reverse()
{
	Curve::Helpers::ReverseKnots(*m_pCurveData);
	XtSave();
}

// From C_DgSplinePropNode
const C_DgSplinePropNode::CurveType* C_DgSplinePropNode::GetCurve() const
{
	return m_pCurve;
}

// From C_DgSplinePropNode
C_DgSplinePropNode::CurveType* C_DgSplinePropNode::GetCurve()
{
	return m_pCurve;
}

// From C_DgSplinePropNode
const C_DgSplinePropNode::CurveDataType* C_DgSplinePropNode::GetCurveData() const
{
	return m_pCurveData;
}

// From C_DgSplinePropNode
C_DgSplinePropNode::CurveDataType* C_DgSplinePropNode::GetCurveData()
{
	return m_pCurveData;
}

// From C_DgSplinePropNode
const Curve::C_Reparameterise<Maths::HVec3>* C_DgSplinePropNode::GetReparam() const
{
	return m_pReparam;
}

// From C_DgSplinePropNode
U32 C_DgSplinePropNode::GetNumComponents() const
{
	const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = m_pCurveData->GetKnots();
	//3 vectors per knot, apart from missing tangents at the end knots.
	//don't bother with the last component if spline is closed
	return m_isSplineClosed ? (knots.Size()-1)*3 : knots.Size()*3 ;
}

// From C_DgSplinePropNode
U32 C_DgSplinePropNode::GetComponentKnotIndex(U32 componentIndex) const
{
	return componentIndex / 3;
}

// From C_DgSplinePropNode
U32 C_DgSplinePropNode::GetKnotComponentIndex(U32 knotIndex) const
{
	return knotIndex * 3;
}

// From C_DgSplinePropNode
Maths::HVec3 C_DgSplinePropNode::GetComponent(U32 index, E_SplineCompType* pType) const
{
	E_SplineCompType componentType = (E_SplineCompType)(index%E_SplineCompType_Count);
	if (pType)
		*pType = componentType;

	const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = m_pCurveData->GetKnots();

	U32 knotIndex = GetComponentKnotIndex(index);
	const Curve::C_HermiteSplineKnot<Maths::HVec3>& knot = knots[knotIndex];

	Maths::HVec3 position;
	switch(componentType)
	{
		case E_SplineCompType_InTangent:
		{
			position = Curve::Helpers::InTangentToControlPoint(
				knot.value, knot.tangentIn, knot.tangentScale);
			break;
		}
		case E_SplineCompType_Value:
		{
			position = knot.value;
			break;
		}
		case E_SplineCompType_OutTangent:
		{
			position = Curve::Helpers::OutTangentToControlPoint(
				knot.value, knot.tangentOut, knot.tangentScale);
			break;
		}
		default:
			assert(!"Invalid component type!");
	};

	return position;
}

// From C_DgSplinePropNode
void C_DgSplinePropNode::SetComponent(U32 index, const Maths::HVec3& pos, bool lockTangents)
{
	E_SplineCompType componentType = (E_SplineCompType)(index%E_SplineCompType_Count);
	U32 knotIndex = index / 3;

	Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = m_pCurveData->GetKnots();
	Curve::C_HermiteSplineKnot<Maths::HVec3>& knot = knots[knotIndex];

	Maths::HVec3 position;
	switch(componentType)
	{
		case E_SplineCompType_InTangent:
		{
			knot.tangentIn = Curve::Helpers::ControlPointToInTangent(
				knot.value, pos, knot.tangentScale);
			knot.tangentOut = knot.tangentIn;
			break;
		}
		case E_SplineCompType_Value:
		{
			knot.value = pos;
			break;
		}
		case E_SplineCompType_OutTangent:
		{
			knot.tangentOut = Curve::Helpers::ControlPointToOutTangent(
				knot.value, pos, knot.tangentScale);
			knot.tangentIn = knot.tangentOut;
			break;
		}
		default:
			assert(!"Invalid component type!");
	};
}

// From C_DgSplinePropNode
float C_DgSplinePropNode::GetDefaultTangentScale() const
{
	return 0.3f;
}

// From C_DgSplinePropNode
bool C_DgSplinePropNode::IsSplineClosed() const
{
	return m_isSplineClosed;
}

// From C_XtObject
void C_DgSplinePropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);
	Xt::DomBlock* pBlockData = Xt::AsBlock(GetXtData());
	BIZ_ASSERT(pBlockData);

	//Movement resolution...
	{
		Xt::DomField* pField;
		XT_HELPERS_AT_FIELD_CHECKED(pField, pBlockData, "reparamRes");
		if (XtHelpers::AreRelated(pField, pEvent->from))
		{
			GetXtDirty().SetDirty(pField);
		}
	}

	// Closed field
	{
		Xt::DomField* pField;
		XT_HELPERS_AT_FIELD_CHECKED(pField, pBlockData, "closed");
		if (XtHelpers::AreRelated(pField, pEvent->from))
		{
			GetXtDirty().SetDirty(pField);
		}
	}

	//Data array.
	{
		Xt::DomArray* pKnotsArray;
		XT_HELPERS_AT_ARRAY_CHECKED(pKnotsArray, pBlockData, "knots");
		if (XtHelpers::AreRelated(pKnotsArray, pEvent->from))
		{
			GetXtDirty().SetDirty(pKnotsArray);
		}
	}
}

// From C_DgSplinePropNode
C_DgSplinePropNode::C_DgSplinePropNode()
:
C_DgXtPropNodeBase()
{
	Curve::C_HermiteSplineFlags flags;
	flags.calcKnotU = Curve::E_ProcessU_None;
	flags.calcTangents = false;

	m_pReparam = new Curve::C_Reparameterise<Maths::HVec3>();
	m_pCurveData = new CurveDataType();
	m_pCurve = new CurveType(flags);
	m_pCurve->SetData(m_pCurveData);

	m_pCollisionInstance = 0;
	m_isSplineClosed = false;
	m_reparamRes = Curve::kReparamResDefault;
}

// From C_DgSplinePropNode
C_DgSplinePropNode::~C_DgSplinePropNode()
{
	if (m_pCollisionSpline)
	{
		assert(m_pCollisionInstance);
		C_CollisionScene& collisionScene = 
			C_Core::Instance().GetCollisionManager().GetGlobalCollisionScene();

		collisionScene.RemoveCollisionInstance(this);
		collisionScene.RemoveCollisionSource(m_pCollisionSpline);
	}
	SAFE_DELETE_PTR(m_pReparam);
	SAFE_DELETE_PTR(m_pCurveData);
	SAFE_DELETE_PTR(m_pCurve);
}

// From C_DgSplinePropNode
void C_DgSplinePropNode::OnSplineChanged()
{
	U32 nSubDivisions = (U32)m_pCurve->GetArcLengthLinear() * m_reparamRes;
	m_pReparam->BuildTable(m_pCurve, nSubDivisions);
	UpdateCollisionSource();
}

// From C_DgSplinePropNode
void C_DgSplinePropNode::UpdateCollisionSource()
{
	C_Status status;

	C_CollisionSpline* pSpline = (C_CollisionSpline*)m_pCollisionInstance->pSource;

	//Resize collision data?
	if (pSpline->nVertices != m_pCurveData->GetNumKnots())
	{
		delete[] pSpline->pVertices;
		delete[] pSpline->pTangents;

		pSpline->nVertices = m_pCurveData->GetNumKnots();
		pSpline->pVertices = new Vector3[pSpline->nVertices];
		pSpline->pTangents = new C_CollisionSpline::C_Tangents[pSpline->nVertices];
	}

	const Vector3* pVertex = pSpline->pVertices;
	const C_CollisionSpline::C_Tangents* pTangent = pSpline->pTangents;

	const Btl::Vector<Curve::C_HermiteSplineKnot<Maths::HVec3> >& knots = m_pCurveData->GetKnots();
	for (U32 i = 0; i < pSpline->nVertices; i++, pVertex++, pTangent++)
	{
		Maths::Store(knots[i].value, (MathsStorage::Float3*)pVertex);
	}

	Vector3 aabbMin(FLT_MAX,FLT_MAX,FLT_MAX);
	Vector3 aabbMax(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	F32 t = 0.0f;
	U32 numSteps = 100;
	for(U32 i = 0; i < numSteps-1; ++i, t += 1.0f/numSteps)
	{
		Maths::HVec3 p;
		if (knots.Size() < 2)
			p = Maths::HVec3(0,0,0);
		else
			p = m_pCurve->Evaluate(t);

		//AABB calculation.
		if (p.X() < aabbMin.x)
			aabbMin.x = p.X();
		if (p.Y() < aabbMin.y)
			aabbMin.y = p.Y();
		if (p.Z() < aabbMin.z)
			aabbMin.z = p.Z();
		if (p.X() > aabbMax.x)
			aabbMax.x = p.X();
		if (p.Y() > aabbMax.y)
			aabbMax.y = p.Y();
		if (p.Z() > aabbMax.z)
			aabbMax.z = p.Z();
	}

	//Calc bounding box centre.
	Vector3 centre = (aabbMax - aabbMin) * 0.5f + aabbMin;

	//OBB calculation.
	{
		pSpline->obb.transform.SetIdentity();
		pSpline->obb.transform.AW(Maths::HVec3(centre.x, centre.y, centre.z));
		Vector3 extents = (aabbMax - aabbMin)*0.5f;
		if (extents[0] * extents[1] * extents[2] < 0.01f)
		{
			if (extents[0] < 0.01f)
				extents[0] = 0.01f;
			if (extents[1] < 0.01f)
				extents[1] = 0.01f;
			if (extents[2] < 0.01f)
				extents[2] = 0.01f;
		}
		pSpline->obb.extents = Maths::HVec3(extents[0], extents[1], extents[2]);
	}
}

// From C_DgSplinePropNode
void C_DgSplinePropNode::UpdateCollisionInstance()
{
	C_DgXtCompNode* pComponent = GetParent();
	assert(pComponent);
	I_DgNode* pTopLevelParent = pComponent->GetParent();
	if (pTopLevelParent && pTopLevelParent->IsA(E_ObjType_SceneNode))
	{
		I_CollisionInstance* pInstance = 
			C_Core::Instance().GetCollisionManager().FindCollisionInstance(this);
		if (pInstance)
		{
			//Found our node. Update its transform and inverse.
			pInstance->toWorldSpace = 
				((C_SceneNode*)pTopLevelParent)->GetWorldMatrix();
			UtMaths::Invert4x4(&pInstance->toObjectSpace, pInstance->toWorldSpace);
		}
	}
}

}

