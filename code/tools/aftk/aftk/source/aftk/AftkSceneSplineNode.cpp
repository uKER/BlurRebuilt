#include <stdafx.h>
#include "AftkEvaluateSpline.h"
#include "AftkMoveToMercury.h"

using namespace Aftk;


C_SceneSplineNode::C_SceneSplineNode(C_Scene* pScene)
:
C_SceneGeometryNode(pScene)
{
	OneTimeInit();
}

C_SceneSplineNode::C_SceneSplineNode(const C_SceneSplineNode& rhs)
:
C_SceneGeometryNode(0)
{
	OneTimeInit();

	*this = rhs;
}

C_Node* C_SceneSplineNode::Clone(C_Scene* pScene) const
{
	C_SceneSplineNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneSplineNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneSplineNode(pScene);
		(C_SceneSplineNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

const C_SceneSplineNode& C_SceneSplineNode::operator=(const C_SceneSplineNode& rhs)
{
	C_SceneGeometryNode::operator=(rhs);

	ClearThis();

	m_form = rhs.m_form;
	m_splineType = rhs.m_splineType;
	m_degree = rhs.m_degree;
	m_cvs = rhs.m_cvs;
	m_knots = rhs.m_knots;
	m_rational = rhs.m_rational;
	m_numSubDivisions = rhs.m_numSubDivisions;

	return *this;
}

C_SceneSplineNode::~C_SceneSplineNode()
{
	ClearThis();
}

void C_SceneSplineNode::SetSplineForm(E_SplineForm form)
{
	DirtyGeometryDependantCaches();
	m_form = form;
}

void C_SceneSplineNode::SetSplineType(E_SplineType splineType)
{
	DirtyGeometryDependantCaches();
	m_splineType = splineType;
}

void C_SceneSplineNode::SetDegree(U32 degree)
{
	DirtyGeometryDependantCaches();
	m_degree = degree;
}

void C_SceneSplineNode::SetCVInitialCapacity(U32 capacity)
{
	if ((U32)m_cvs.Capacity() < capacity)
	{
		m_cvs.Reserve(capacity);
	}
}

void C_SceneSplineNode::SetKnotInitialCapacity(U32 capacity)
{
	if ((U32)m_knots.Capacity() < capacity)
		m_knots.Reserve(capacity);
}

U32 C_SceneSplineNode::AddCV(const Position3& cv)
{
	DirtyGeometryDependantCaches();
	m_cvs.PushBack(cv);
	return (U32)(m_cvs.Size()-1);
}

U32 C_SceneSplineNode::AddKnot(F32 knot)
{
	DirtyGeometryDependantCaches();
	m_knots.PushBack(knot);
	return (U32)(m_knots.Size()-1);
}
void C_SceneSplineNode::SetCV(U32 instance, const Position3& cv)
{
	DirtyGeometryDependantCaches();
	m_cvs[instance] = cv;
}

void C_SceneSplineNode::SetKnot(U32 instance, F32 knot)
{
	DirtyGeometryDependantCaches();
	m_knots[instance] = knot;
}

void C_SceneSplineNode::SetIsRational(bool flag)
{
	DirtyGeometryDependantCaches();
	m_rational = flag;
}

U32 C_SceneSplineNode::GetDegree() const
{
	return m_degree;
}

E_SplineForm C_SceneSplineNode::GetSplineForm() const
{
	return m_form;
}

E_SplineType C_SceneSplineNode::GetSplineType() const
{
	return m_splineType;
}

U32 C_SceneSplineNode::GetNumCVs() const
{
	return (U32)m_cvs.Size();
}

U32 C_SceneSplineNode::GetNumKnots() const
{
	return (U32)m_knots.Size();
}

Position3& C_SceneSplineNode::GetCV(U32 instance)
{
	return m_cvs[instance];
}

const Position3& C_SceneSplineNode::GetCV(U32 instance) const
{
	return m_cvs[instance];
}

Position3Array& C_SceneSplineNode::GetCVArray()
{
	return m_cvs;
}

const Position3Array& C_SceneSplineNode::GetCVArray() const
{
	return m_cvs;
}

F32 C_SceneSplineNode::GetKnot(U32 instance) const
{
	return m_knots[instance];
}

F32Array& C_SceneSplineNode::GetKnotArray()
{
	return m_knots;
}

const F32Array& C_SceneSplineNode::GetKnotArray() const
{
	return m_knots;
}

void C_SceneSplineNode::DeleteCV(U32 instance)
{
	m_cvs.EraseByIndex(instance);
}

void C_SceneSplineNode::DeleteCV()
{
	m_cvs.PopBack();
}

bool C_SceneSplineNode::IsRational() const
{
	return m_rational;
}

void C_SceneSplineNode::GenerateLinearFromCVs()
{
	DirtyGeometryDependantCaches();
	m_rational = false;
	m_degree = 1;
	m_knots.Clear();

	m_knots.Reserve(m_cvs.Size());
	for (U32 i = 0; i < (U32)m_cvs.Size(); i++)
		m_knots.PushBack(F32(i));
}

bool C_SceneSplineNode::Evaluate(Position3& value, const F32 u) const
{
	if (!BuildEvaluationHelpers())
		return false;

	value = m_pEvaluationHelper->GetPosition(u);
	return true;
}

bool C_SceneSplineNode::EvaluateTotalLength(F32& length) const
{
	if (!BuildEvaluationHelpers())
		return false;

	length = m_pEvaluationHelper->GetTotalLength();
	return true;
}

bool C_SceneSplineNode::EvaluateLength(F32& u, const F32 length) const
{
	if (!BuildEvaluationHelpers())
		return false;

	u = m_pEvaluationHelper->GetLength(length);
	return true;
}

bool C_SceneSplineNode::GetEvaluationStepSize(F32& stepSize) const
{
	if (!BuildEvaluationHelpers())
		return false;

	stepSize = m_pEvaluationHelper->GetEvalationStepSize();
	return true;
}

void C_SceneSplineNode::ApplyMatrix(const Matrix44& matrix)
{
	DirtyGeometryDependantCaches();
	for (U32 i = 0; i < (U32)m_cvs.Size(); i++)
	{
		const Position3 *pSrc = &m_cvs[i];
		Position3 dst;

		dst[0] = ( (matrix.At<0>().X() * pSrc->X()) + (matrix.At<1>().X() * pSrc->Y()) + (matrix.At<2>().X() * pSrc->Z()) + matrix.At<3>().X()  );
		dst[1] = ( (matrix.At<0>().Y() * pSrc->X()) + (matrix.At<1>().Y() * pSrc->Y()) + (matrix.At<2>().Y() * pSrc->Z()) + matrix.At<3>().Y()  );
		dst[2] = ( (matrix.At<0>().Z() * pSrc->X()) + (matrix.At<1>().Z() * pSrc->Y()) + (matrix.At<2>().Z() * pSrc->Z()) + matrix.At<3>().Z()  );

		m_cvs[i] = dst;
	}
}

U32 C_SceneSplineNode::GetHashOfThis() const
{
	U32 hash = 1;
	hash += (U32)m_cvs.Size();
	hash += (U32)m_knots.Size();
	return hash;
}

U32 C_SceneSplineNode::GetHash(E_ObjType classToAbort) const
{
	if (classToAbort = E_ObjType_SceneSpline)
		return C_SceneSplineNode::GetHashOfThis();
	return C_SceneSplineNode::GetHashOfThis()*C_SceneGeometryNode::GetHash(classToAbort);
}

bool C_SceneSplineNode::operator== (const C_SceneSplineNode &rhs) const
{
	return IsEqualToAll(rhs);
}

bool C_SceneSplineNode::IsEqualTo(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	return IsEqualToAll(rhs, classToAbort, flags);
}

bool C_SceneSplineNode::IsEqualToAll(const I_Object &rhs, E_ObjType classToAbort, U32 flags) const
{
	if (classToAbort == E_ObjType_SceneSpline)
		return IsEqualToThis(rhs, flags);
	return (IsEqualToThis(rhs, flags) && C_SceneGeometryNode::IsEqualToAll(rhs, classToAbort, flags));
}

bool C_SceneSplineNode::IsEqualToThis(const I_Object &_rhs, U32 flags) const
{
	if (!_rhs.HasType(GetType()))
	{
		return false;
	}

	C_SceneSplineNode& rhs = (C_SceneSplineNode&) _rhs;

	if ((m_cvs.Size() != rhs.m_cvs.Size()) ||
		(m_knots.Size() != rhs.m_knots.Size()))
		return false;

	if ((m_form != rhs.m_form) ||
		(m_splineType != rhs.m_splineType) ||
		(m_degree != rhs.m_degree) ||
		(m_rational != rhs.m_rational))
		return false;

	if (m_cvs.Size() > 0)
		if (memcmp(&m_cvs[0], &rhs.m_cvs[0], sizeof(Position3)*m_cvs.Size()) != 0)
			return false;

	if (m_knots.Size() > 0)
		if (memcmp(&m_knots[0], &rhs.m_knots[0], sizeof(F32)*m_knots.Size()) != 0)
			return false;

	return true;
}

void C_SceneSplineNode::Clear()
{
	ClearThis();
	C_SceneGeometryNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneSplineNode::OneTimeInit()
{
	m_splineType = E_SplineType_Nurbs;
	m_form = E_SplineForm_Open;
	m_degree = 1;
	m_rational = false;
	m_pEvaluationHelper = 0;
	m_numSubDivisions = 100;
}

void C_SceneSplineNode::ClearThis()
{
	m_splineType = E_SplineType_Nurbs;
	m_form = E_SplineForm_Open;
	m_degree = 1;
	m_rational = false;
	m_pEvaluationHelper = 0;
	m_numSubDivisions = 100;

	m_cvs.Clear();
	m_knots.Clear();

	ClearEvaluationHelpers();
}

E_ObjType C_SceneSplineNode::GetType() const
{
	return E_ObjType_SceneSpline;
}

bool C_SceneSplineNode::HasType(E_ObjType type) const
{
	if (C_SceneGeometryNode::HasType(type))
		return true;
	if (type == C_SceneSplineNode::GetType()) 
		return true;

	return false;
}

void C_SceneSplineNode::CalculateBounds(C_Bounds& bounds, const Matrix44* pMatrix) const
{
	bounds.Clear();
	for (U32 i = 0; i < GetNumCVs(); i++)
		if (pMatrix)
			bounds.AddVertex(Aftk::ApplyMatrix(*pMatrix, m_cvs[i]));
		else
			bounds.AddVertex(m_cvs[i]);
}

void C_SceneSplineNode::DirtyGeometryDependantCaches() const
{
	ClearEvaluationHelpers();

	C_SceneGeometryNode::DirtyGeometryDependantCaches();
}

bool C_SceneSplineNode::BuildEvaluationHelpers() const
{
	if (m_pEvaluationHelper)
		return true;

	m_pEvaluationHelper = new C_EvaluateSpline;
	if (!m_pEvaluationHelper)
		return false;

	if (!m_pEvaluationHelper->Create(*this, m_numSubDivisions))
		return false;


	return true;
}

void C_SceneSplineNode::ClearEvaluationHelpers() const
{
	if (m_pEvaluationHelper)
	{
		delete m_pEvaluationHelper;
		m_pEvaluationHelper = 0;
	}
}

void C_SceneSplineNode::SetNumSubDivisions(U32 numSubDivisions)
{
	DirtyGeometryDependantCaches();

	m_numSubDivisions = numSubDivisions;
}

U32 C_SceneSplineNode::GetNumSubDivisions() const
{
	return m_numSubDivisions;
}

#endif



