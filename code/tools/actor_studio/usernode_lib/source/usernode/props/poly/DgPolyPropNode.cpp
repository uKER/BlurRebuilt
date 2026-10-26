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

#include <EditorLib/Misc/Status.h>

#include <EditorLib/Xt/XtDefinitions.h>
#include <EditorLib/Xt/XtObject.h>
#include <EditorLib/Xt/XtChange.h>
#include <EditorLib/Xt/XtDirty.h>
#include <EditorLib/Xt/XtManager.h>
#include <EditorLib/Xt/XtFieldHelper.h>
#include <EditorLib/Xt/XtHelpers.h>

#include <EditorLib/Dg/IDgObject.h>
#include <EditorLib/Dg/IDgNode.h>
#include <EditorLib/Dg/SceneNode.h>
#include <EditorLib/Dg/UpdateSignaller.h>
#include <EditorLib/Dg/DgManager.h>
#include <EditorLib/Dg/DgXtCompNode.h>
#include <EditorLib/Collision/Collision.h>
#include <EditorLib/Core.h>

#include <UserNode/Props/Misc/DgPropContextHelpers.h>
#include <UserNode/Props/Poly/DgPolyPropNode.h>

void RegisterPolyPropDgNodes()
{
	REGISTER_DG_NODE(EditorLib::C_DgPolyPropNode)
}


namespace EditorLib
{

// From C_XtObject
C_Status C_DgPolyPropNode::XtInitialise(
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
	{
		Xt::DomBlock* pDataBlock = Xt::AsBlock(pData);

		if (!pDataBlock)
			return E_Status_Failure;

		//Create data array.
		if (!Xt::At(pDataBlock, "Points"))
		{
			Xt::AcquireArray(pDataBlock, "Points");
		}

		// Create height field
		if (!Xt::At(pDataBlock, "Height"))
		{
			Xt::AcquireField(pDataBlock, "Height");
		}
	}

	return E_Status_Success;
}

// From I_DgObject
E_ObjType C_DgPolyPropNode::GetType() const
{
	return E_ObjType_DgPolyPropNode;
}

// From I_DgObject
const char* C_DgPolyPropNode::GetTypeName() const
{
	return GetStaticTypeName();
}

// From I_DgObject
bool C_DgPolyPropNode::IsA(E_ObjType type) const
{
	if (__super::IsA(type))
		return true;

	if (type == C_DgPolyPropNode::GetType()) 
		return true;

	return false;
}

//From I_DgNode
bool C_DgPolyPropNode::Update(C_UpdateHintCollection* pHc)
{
	C_UpdateSignaller signaller(this, C_DgPolyPropNode::GetStaticType(), pHc);

	if (GetXtDirty().IsDirty())
	{
		Xt::DomBlock* pData = Xt::AsBlock(GetXtData());

		Xt::DomArray* pPointsArray;
		XT_HELPERS_AT_ARRAY_CHECKED(pPointsArray, pData, "Points");
		if (pPointsArray && GetXtDirty().IsDirty(pPointsArray))
		{
			m_polyPoints.Clear();

			for (U32 i=0; i<Xt::Count(pPointsArray); ++i)
			{
				Xt::DomComponent* pComponent = Xt::AsComponent(Xt::At(pPointsArray, i));
				if (!pComponent)
					continue;

				Xt::DomBlock* pBlock = &pComponent->data;
				if (!pBlock)
					continue;

				{
					Vector3 fieldVector(0,0,0);
					if (pBlock)	XtFieldHelper::GetVector(fieldVector, pBlock);
			
					m_polyPoints.PushBack();
					Maths::HVec3& position = m_polyPoints.Back();
					position.X(fieldVector.X());
					position.Y(fieldVector.Y());
					position.Z(fieldVector.Z());
				}
			}
		}

		Xt::DomField* pHeightField;
		XT_HELPERS_AT_FIELD_CHECKED(pHeightField, pData, "Height");
		if (pHeightField && GetXtDirty().IsDirty())
		{
			XtFieldHelper::GetScalar(m_height, pHeightField);
		}

		OnPolyChanged();
	}

	return __super::Update(&signaller.GetCollection());
}

// From I_DgXtActorPropNode
bool C_DgPolyPropNode::IsVisible() const
{
	return true;
}

// From C_DgPolyPropNode
const char* C_DgPolyPropNode::GetStaticTypeName()
{
	//Must match name in Xt schema exactly!
	static char* p = "BoundingPoly";
	return p;
}

const char* C_DgPolyPropNode::GetStaticDisplayName()
{
	static char* p = "Polygon Property";
	return p;
}

// From C_DgPolyPropNode
E_ObjType C_DgPolyPropNode::GetStaticType()
{
	return E_ObjType_DgPolyPropNode;
}

// From C_DgPolyPropNode
void* C_DgPolyPropNode::Create()
{
	return new C_DgPolyPropNode();
}

// From C_DgPolyPropNode
C_Status C_DgPolyPropNode::XtSave()
{
	Xt::DomBlock* pBlock = Xt::AsBlock(GetXtData());
	if (!pBlock)
		return E_Status_Failure;

	Xt::DomArray* pXtPoints = Xt::AcquireArray(pBlock, "Points");
	if (!pXtPoints)
		return E_Status_Failure;

	S32 nPoints = Xt::Count(pXtPoints);
	assert(nPoints == m_polyPoints.Size());

	for (S32 i = 0; i < m_polyPoints.Size(); i++)
	{
		Xt::DomComponent* pComponent = Xt::AcquireComponent(pXtPoints,i);
		if (!pComponent)
			continue;

		Xt::DomBlock* pBlock = &pComponent->data;
		if (!pBlock)
			continue;

		XtFieldHelper::SetVector(pBlock, m_polyPoints[i]);
	}

	return E_Status_Success;
}

// From C_DgPolyPropNode
C_Status C_DgPolyPropNode::Apply()
{
	return XtSave();
}

// From C_DgPolyPropNode
C_Status C_DgPolyPropNode::AddPoint(const Maths::HVec3& pos)
{
	C_XtObject::C_Unbinder unbinder(this);

	m_polyPoints.PushBack();
	m_polyPoints.Back() = pos;

	OnPolyChanged();

	// Acquire array
	Xt::DomBlock* pData = Xt::AsBlock(GetXtData());
	Xt::DomArray* pXtPoints = Xt::AcquireArray(pData, "Points");
	
	//append new knot data
	{
		Xt::DomComponent* pComponent = new Xt::DomComponent();
		if (!pComponent)
			return E_Status_Failure;

		GenerateIdForComponent(pComponent, false);

		XtFieldHelper::SetVector(&pComponent->data, pos);

		Xt::Append(pXtPoints, pComponent);
	}
	
	return E_Status_Success;
}

// From C_DgPolyPropNode
C_Status C_DgPolyPropNode::DeletePoints(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds)
{
	Xt::DomBlock* pData = Xt::AsBlock(GetXtData());
	Xt::DomArray* pXtPoints = Xt::AcquireArray(pData, "Points");
	if (!pXtPoints || componentIds.Size() == 0)
		return E_Status_Failure;

	Btl::Set<U32> sortedKnotIndices;
	for (S32 i = 0; i < componentIds.Size(); ++i)
	{
		sortedKnotIndices.Insert(componentIds[i]);
	}
	{
		C_XtObject::C_Unbinder unbinder(this);
		GetXtDirty().SetDirty(GetXtData());
		for (Btl::Set<U32>::ReverseIterator it = sortedKnotIndices.ReverseBegin(); 
			it != sortedKnotIndices.ReverseEnd(); ++it)
		{
			U32 knotIndex = *it;
			Xt::Remove(pXtPoints,knotIndex);
		}
	}
	
	return E_Status_Success;
}

// From C_DgPolyPropNode
Ut::Vector<U32> C_DgPolyPropNode::InsertPoints(const Ut::UtVector< Ut::DynamicVector<U32> >& componentIds)
{
	C_ViewDisabler viewDisabler;
	Ut::Vector<U32> addedComponents;

	int insertedPoints = 0;

	Ut::UtVector< Ut::DynamicVector<U32> > sortedComponents = componentIds;
	std::sort(sortedComponents.Begin(), sortedComponents.End());

	// Acquire array
	Xt::DomBlock* pData = Xt::AsBlock(GetXtData());
	Xt::DomArray* pXtPoints = Xt::AcquireArray(pData, "Points");

	for (S32 i=0; i<sortedComponents.Size(); i++) 
	{
		if (insertedPoints && sortedComponents.Size()==2)
			break;

		//Find adjacent knots.
		S32 point0 = sortedComponents[(i+sortedComponents.Size()-1)%sortedComponents.Size()] + insertedPoints;
		S32 point1 = sortedComponents[i] + insertedPoints;
		
		if (point0 > point1)
		{
			S32 temp = point0;
			point0 = point1;
			point1 = temp;
		}

		if (abs((S32)(point1-point0)) == 1 || point0 == insertedPoints && point1 == m_polyPoints.Size()-1)
		{
			Maths::HVec3 newPoint = (m_polyPoints[point0]+m_polyPoints[point1])/2;

			S32 newIdx;
			
			if (point0 == insertedPoints && point1 == m_polyPoints.Size()-1)
			{
				newIdx = 0;
				newPoint = (m_polyPoints[0]+m_polyPoints[point1])/2;
			}
			else
				newIdx = point1;

			{
				Btl::Vector<Maths::HVec3>::Iterator it = m_polyPoints.Begin();
				it = it + newIdx;
				m_polyPoints.InsertBefore(newPoint, it);
			}

			// Create block
			Xt::DomComponent* pXtItemComponent = new Xt::DomComponent;
			GenerateIdForComponent(pXtItemComponent, false);
	
			// Append array data
			Xt::Insert(pXtPoints, newIdx, pXtItemComponent);	

			addedComponents.PushBack(newIdx);
			insertedPoints++;
		}
	}

	if (addedComponents.Size() > 0)
		XtSave();

	return addedComponents;
}

// From C_DgPolyPropNode
U32 C_DgPolyPropNode::GetNumComponents() const
{
	return m_polyPoints.Size();
}

// From C_DgPolyPropNode
Maths::HVec3 C_DgPolyPropNode::GetComponent(U32 index) const
{
	return m_polyPoints[index];
}

// From C_DgPolyPropNode
void C_DgPolyPropNode::SetComponent(U32 index, const Maths::HVec3& pos)
{
	m_polyPoints[index] = pos;
}

// From C_DgPolyPropNode
float C_DgPolyPropNode::GetHeight() const
{
	return m_height;
}

// From C_XtObject
void C_DgPolyPropNode::OnXtDomMonitorEvent(
	Xt::DomMonitor* pMonitor, 
	Xt::DomEvent* pEvent,
	bool typeChanged)
{
	__super::OnXtDomMonitorEvent(pMonitor, pEvent, typeChanged);

	Xt::DomBlock* pBlockData = Xt::AsBlock(GetXtData());
	BIZ_ASSERT(pBlockData);

	// Point array
	{
		Xt::DomArray* pKnotsArray;
		XT_HELPERS_AT_ARRAY_CHECKED(pKnotsArray, pBlockData, "Points");
		if (XtHelpers::AreRelated(pKnotsArray, pEvent->from))
		{
			GetXtDirty().SetDirty(pKnotsArray);
		}
	}

	// Height field
	{
		Xt::DomField* pHeightField;
		XT_HELPERS_AT_FIELD_CHECKED(pHeightField, pBlockData, "Height");
		if (XtHelpers::AreRelated(pHeightField, pEvent->from))
		{
			GetXtDirty().SetDirty(pHeightField);
		}
	}
		
}

// From C_DgPolyPropNode
C_DgPolyPropNode::C_DgPolyPropNode()
:
C_DgXtPropNodeBase()
{
	m_pCollisionInstance = 0;
}

// From C_DgPolyPropNode
C_DgPolyPropNode::~C_DgPolyPropNode()
{
	if (m_pCollisionSpline)
	{
		assert(m_pCollisionInstance);
		C_CollisionScene& collisionScene = 
			C_Core::Instance().GetCollisionManager().GetGlobalCollisionScene();

		collisionScene.RemoveCollisionInstance(this);
		collisionScene.RemoveCollisionSource(m_pCollisionSpline);
	}
}

// From C_DgPolyPropNode
void C_DgPolyPropNode::OnPolyChanged()
{
	UpdateCollisionSource();
}

// From C_DgPolyPropNode
void C_DgPolyPropNode::UpdateCollisionSource()
{
	// Todo: Add collision stuff
}

// From C_DgPolyPropNode
void C_DgPolyPropNode::UpdateCollisionInstance()
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

