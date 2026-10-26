#include <stdafx.h>
#include <Aftk/AftkEuler.h>

using namespace Aftk;

C_SceneJointNode::C_SceneJointNode(C_Scene* pScene) : C_SceneNode(pScene), C_LimitMixin()
{
	OneTimeInit();
}

C_Node* C_SceneJointNode::Clone(C_Scene* pScene) const
{
	C_SceneJointNode* pNewNode;
	if (pScene == m_pScene)
	{
		pNewNode = new C_SceneJointNode(*this);
	}
	else
	{
		//Moving between scenes.
		pNewNode = new C_SceneJointNode(pScene);
		(C_SceneJointNode&)(*pNewNode) = *this;
	}

	return pNewNode;
}

C_SceneJointNode::C_SceneJointNode(const C_SceneJointNode& rhs) 
: C_SceneNode(0)
{
	OneTimeInit();
	*this = rhs;
}

const C_SceneJointNode& C_SceneJointNode::operator=(const C_SceneJointNode& rhs)
{
	C_SceneNode::operator=(rhs);
	C_LimitMixin::operator=(rhs);
	m_segmentScaleCompensation = rhs.m_segmentScaleCompensation;

#ifndef AFTK_LEAN_AND_MEAN
	m_stiffness = rhs.m_stiffness;
	m_scaleOrientation = rhs.m_scaleOrientation;
	m_orientation = rhs.m_orientation;

	m_preferredAngle = rhs.m_preferredAngle;
	m_dof = rhs.m_dof;
	
	m_radius = rhs.m_radius;

	m_minDampRange = rhs.m_minDampRange;
	m_maxDampRange = rhs.m_maxDampRange;
	m_minDampStrength = rhs.m_minDampStrength;
	m_maxDampStrength = rhs.m_maxDampStrength;

#endif
	return *this;
}

C_SceneJointNode::~C_SceneJointNode()
{
	ClearThis();
}

E_ObjType C_SceneJointNode::GetType() const
{
	return E_ObjType_SceneJoint;
}

bool C_SceneJointNode::HasType(E_ObjType type) const
{
	if (C_SceneNode::HasType(type))
		return true;
	if (type == C_SceneJointNode::GetType()) 
		return true;

	return false;
}

#ifndef AFTK_LEAN_AND_MEAN
const Vector3F32& C_SceneJointNode::GetStiffness() const
{
	return m_stiffness;
}

const Vector3F32& C_SceneJointNode::GetPreferredAngle() const
{
	return m_preferredAngle;
}

const Matrix44& C_SceneJointNode::GetScaleOrientation() const
{
	return m_scaleOrientation;
}

const Matrix44& C_SceneJointNode::GetOrientation() const
{
	return m_orientation;
}

const Vector3Bool& C_SceneJointNode::GetDegreesOfFreedom() const
{
	return m_dof;
}
#endif

bool C_SceneJointNode::GetSegmentScaleCompensation() const
{
	return m_segmentScaleCompensation;
}

#ifndef AFTK_LEAN_AND_MEAN
F32 C_SceneJointNode::GetRadius() const
{
	return m_radius;
}

const Vector3F32& C_SceneJointNode::GetMinRotateDampRange() const
{
	return m_minDampRange;
}

const Vector3F32& C_SceneJointNode::GetMaxRotateDampRange() const
{
	return m_maxDampRange;
}

const Vector3F32& C_SceneJointNode::GetMinRotateDampStrength() const
{
	return m_minDampStrength;
}

const Vector3F32& C_SceneJointNode::GetMaxRotateDampStrength() const
{
	return m_maxDampStrength;
}

void C_SceneJointNode::SetStiffness(const Vector3F32& stiffness)
{
	m_stiffness = stiffness;
}

void C_SceneJointNode::SetScaleOrientation(const Matrix44& scaleOrientation)
{
	m_scaleOrientation = scaleOrientation;
}

void C_SceneJointNode::SetOrientation(const Matrix44& orientation)
{
	m_orientation = orientation;
}

void C_SceneJointNode::SetPreferredAngle(const Vector3F32& preferredAngle)
{
	m_preferredAngle = preferredAngle;
}

void C_SceneJointNode::SetDegreesOfFreedom(const Vector3Bool& dof)
{
	m_dof = dof;
}
#endif

void C_SceneJointNode::SetSegmentScaleCompensation(bool compensate)
{
	m_segmentScaleCompensation = compensate;
}

#ifndef AFTK_LEAN_AND_MEAN
void C_SceneJointNode::SetRadius(F32 radius)
{
	m_radius = radius;
}

void C_SceneJointNode::SetMinRotateDampRange(const Vector3F32& range)
{
	m_minDampRange = range;
}

void C_SceneJointNode::SetMaxRotateDampRange(const Vector3F32& range)
{
	m_maxDampRange = range;
}

void C_SceneJointNode::SetMinRotateDampStrength(const Vector3F32& strength)
{
	m_minDampStrength = strength;
}

void C_SceneJointNode::SetMaxRotateDampStrength(const Vector3F32& strength)
{
	m_maxDampStrength = strength;
}

#endif
void C_SceneJointNode::Clear()
{
	ClearThis();
	C_SceneNode::Clear();
}

// protected members
#ifndef DOXYGEN_SHOULD_SKIP_THIS

void C_SceneJointNode::ClearThis()
{
	m_segmentScaleCompensation = true;
#ifndef AFTK_LEAN_AND_MEAN
	memset(&m_stiffness, 0, sizeof(m_stiffness));
	UtMaths::SetIdentity(&m_scaleOrientation);
	UtMaths::SetIdentity(&m_orientation);

	memset(&m_preferredAngle, 0, sizeof(m_preferredAngle));
	memset(&m_dof, 0, sizeof(m_dof));

	m_radius = 0.f;

	memset(&m_minDampRange, 0, sizeof(m_minDampRange));
	memset(&m_maxDampRange, 0, sizeof(m_maxDampRange));
	memset(&m_minDampStrength, 0, sizeof(m_minDampStrength));
	memset(&m_maxDampStrength, 0, sizeof(m_maxDampStrength));
#endif
}

void C_SceneJointNode::OneTimeInit()
{
	ClearThis();
}

void C_SceneJointNode::RecalculateStaticMatrixCache() const
{
	Vector3F32 translate;
	Vector3F32 scale;
	Vector3F32 shear;
	Vector3F32 rotate;
	Vector3F32 inverseScale(1.f,1.f,1.f);

	//Update local matrix cache
	{
		GetTranslation(translate, false);
		GetScale(scale, false);
		GetShear(shear, false);
		GetRotation(rotate, false);

		
		if (m_segmentScaleCompensation && m_pParent)
		{
			m_pParent->GetScale(inverseScale, false);
			if (fabsf(inverseScale[0]) < AFTK_EPSILON)
				inverseScale[0] = 0;
			else
				inverseScale[0] = 1.f / inverseScale[0];

			if (fabsf(inverseScale[1]) < AFTK_EPSILON)
				inverseScale[1] = 0;
			else
				inverseScale[1] = 1.f / inverseScale[1];

			if (fabsf(inverseScale[2]) < AFTK_EPSILON)
				inverseScale[2] = 0;
			else
				inverseScale[2] = 1.f / inverseScale[2];
		}

#ifdef AFTK_LEAN_AND_MEAN
		Aftk::Matrix44 m_scaleOrientation;
		UtMaths::SetIdentity(&m_scaleOrientation);
		Aftk::Matrix44 m_orientation;
		UtMaths::SetIdentity(&m_orientation);
#endif
		//Build local matrix.
		BuildJointMatrix(
			m_staticLocalMatrixCache, 
			scale,
			m_scaleOrientation,
			rotate,
			m_orientation,
			inverseScale,
			translate,
			m_rotationOrder);
	}

	//Derive other local matrices
#ifndef AFTK_LEAN_AND_MEAN
	{
		UtMaths::Invert4x4(&m_staticLocalMatrixInverseCache, m_staticLocalMatrixCache);
		Matrix44 matrixTranspose = m_staticLocalMatrixCache;
		UtMaths::Transpose(&matrixTranspose);
		UtMaths::Invert4x4(&m_staticLocalMatrixInverseTransposeCache, matrixTranspose);
	}
#endif

	//Update world matrix cache
	{
		m_staticWorldMatrixCache = m_staticLocalMatrixCache;

		//Multiply in parent transforms.
		C_SceneNode* pParent = m_pParent;
		while (pParent)
		{
			const Matrix44& parentMatrix = pParent->GetLocalMatrix(false);
			m_staticWorldMatrixCache = m_staticWorldMatrixCache * parentMatrix;
			pParent = pParent->GetParentPtr();
		}

#ifndef AFTK_LEAN_AND_MEAN
		//Derive other static world matrices.
		{
			//Calculate inverse.
			UtMaths::Invert4x4(&m_staticWorldMatrixInverseCache, m_staticWorldMatrixCache);
			//Calculate inverse transpose
			Matrix44 matrixTranspose = m_staticWorldMatrixCache;
			UtMaths::Transpose(&matrixTranspose);
			UtMaths::Invert4x4(&m_staticWorldMatrixInverseTransposeCache, matrixTranspose);
		}
#endif

		//We're no longer flagged as dirty.
		m_staticMatrixCacheDirtyFlag = false;
		//Dirty evaluation time for dynamic cache
		m_dynamicMatrixCacheDirtyFlag = true;
		
		//Dirty our bounds.
		m_worldBoundsCacheDirtyFlag	= true;

		C_SceneNode* pChild = m_pChild;
		while(pChild)
		{
			pChild->RecalculateStaticMatrixCache();
			pChild = pChild->m_pNext;
		}
	}
}

void C_SceneJointNode::RecalculateDynamicMatrixCache() const
{
	//Build dynamic local matrix.
	{
		Vector3F32 translate;
		Vector3F32 scale;
		Vector3F32 shear;
		Vector3F32 euler;
		Matrix44 rotate;

		GetTranslation(translate, true);
		GetScale(scale, true);
		GetShear(shear, true);
		GetRotation(euler, true);

		if (IsLimited(E_LimitType_TranslateMinX) && translate[0] < GetLimit(E_LimitType_TranslateMinX))
			translate[0] = GetLimit(E_LimitType_TranslateMinX);
		if (IsLimited(E_LimitType_TranslateMinY) && translate[1] < GetLimit(E_LimitType_TranslateMinY))
			translate[1] = GetLimit(E_LimitType_TranslateMinY);
		if (IsLimited(E_LimitType_TranslateMinZ) && translate[2] < GetLimit(E_LimitType_TranslateMinZ))
			translate[2] = GetLimit(E_LimitType_TranslateMinZ);

		if (IsLimited(E_LimitType_TranslateMaxX) && translate[0] > GetLimit(E_LimitType_TranslateMaxX))
			translate[0] = GetLimit(E_LimitType_TranslateMaxX);
		if (IsLimited(E_LimitType_TranslateMaxY) && translate[1] > GetLimit(E_LimitType_TranslateMaxY))
			translate[1] = GetLimit(E_LimitType_TranslateMaxY);
		if (IsLimited(E_LimitType_TranslateMaxZ) && translate[2] > GetLimit(E_LimitType_TranslateMaxZ))
			translate[2] = GetLimit(E_LimitType_TranslateMaxZ);

		if (IsLimited(E_LimitType_RotateMinX) && euler[0] < GetLimit(E_LimitType_RotateMinX))
			euler[0] = GetLimit(E_LimitType_RotateMinX);
		if (IsLimited(E_LimitType_RotateMinY) && euler[1] < GetLimit(E_LimitType_RotateMinY))
			euler[1] = GetLimit(E_LimitType_RotateMinY);
		if (IsLimited(E_LimitType_RotateMinZ) && euler[2] < GetLimit(E_LimitType_RotateMinZ))
			euler[2] = GetLimit(E_LimitType_RotateMinZ);

		if (IsLimited(E_LimitType_RotateMaxX) && euler[0] > GetLimit(E_LimitType_RotateMaxX))
			euler[0] = GetLimit(E_LimitType_RotateMaxX);
		if (IsLimited(E_LimitType_RotateMaxY) && euler[1] > GetLimit(E_LimitType_RotateMaxY))
			euler[1] = GetLimit(E_LimitType_RotateMaxY);
		if (IsLimited(E_LimitType_RotateMaxZ) && euler[2] > GetLimit(E_LimitType_RotateMaxZ))
			euler[2] = GetLimit(E_LimitType_RotateMaxZ);

		if (IsLimited(E_LimitType_ScaleMinX) && scale[0] < GetLimit(E_LimitType_ScaleMinX))
			scale[0] = GetLimit(E_LimitType_ScaleMinX);
		if (IsLimited(E_LimitType_ScaleMinY) && scale[1] < GetLimit(E_LimitType_ScaleMinY))
			scale[1] = GetLimit(E_LimitType_ScaleMinY);
		if (IsLimited(E_LimitType_ScaleMinZ) && scale[2] < GetLimit(E_LimitType_ScaleMinZ))
			scale[2] = GetLimit(E_LimitType_ScaleMinZ);

		if (IsLimited(E_LimitType_ScaleMaxX) && scale[0] > GetLimit(E_LimitType_ScaleMaxX))
			scale[0] = GetLimit(E_LimitType_ScaleMaxX);
		if (IsLimited(E_LimitType_ScaleMaxY) && scale[1] > GetLimit(E_LimitType_ScaleMaxY))
			scale[1] = GetLimit(E_LimitType_ScaleMaxY);
		if (IsLimited(E_LimitType_ScaleMaxZ) && scale[2] > GetLimit(E_LimitType_ScaleMaxZ))
			scale[2] = GetLimit(E_LimitType_ScaleMaxZ);

		EulerToMatrix(rotate, euler, m_rotationOrder);

		Vector3F32 inverseScale(1.f,1.f,1.f);
		if (m_segmentScaleCompensation && m_pParent)
		{
			m_pParent->GetScale(inverseScale, true);
			if (fabsf(inverseScale[0]) < AFTK_EPSILON)
				inverseScale[0] = 0;
			else
				inverseScale[0] = 1.f / inverseScale[0];

			if (fabsf(inverseScale[1]) < AFTK_EPSILON)
				inverseScale[1] = 0;
			else
				inverseScale[1] = 1.f / inverseScale[1];

			if (fabsf(inverseScale[2]) < AFTK_EPSILON)
				inverseScale[2] = 0;
			else
				inverseScale[2] = 1.f / inverseScale[2];
		}

#ifdef AFTK_LEAN_AND_MEAN
		Aftk::Matrix44 m_scaleOrientation;
		UtMaths::SetIdentity(&m_scaleOrientation);
		Aftk::Matrix44 m_orientation;
		UtMaths::SetIdentity(&m_orientation);
#endif
		//Build local matrix.
		BuildJointMatrix(
			m_dynamicLocalMatrixCache, 
			scale,
			m_scaleOrientation,
			rotate,
			m_orientation,
			inverseScale,
			translate);
	}

#ifndef AFTK_LEAN_AND_MEAN
	//Derive other dynamic local matrices.
	{
		//Calculate inverse.
		UtMaths::Invert4x4(&m_dynamicLocalMatrixInverseCache, m_dynamicLocalMatrixCache);
		//Calculate inverse transpose
		Matrix44 matrixTranspose = m_dynamicLocalMatrixCache;
		UtMaths::Transpose(&matrixTranspose);
		UtMaths::Invert4x4(&m_dynamicLocalMatrixInverseTransposeCache, matrixTranspose);
	}
#endif

	if (m_pParent)
	{
		//Apply parental transform - this will potentially trigger re-evaluation of the parent's
		//world matrix.
		m_dynamicWorldMatrixCache = 
			m_dynamicLocalMatrixCache * m_pParent->EvaluateWorldMatrix();
	}
	else
	{
		m_dynamicWorldMatrixCache = m_dynamicLocalMatrixCache;
	}

#ifndef AFTK_LEAN_AND_MEAN
	//Derive other dynamic world matrices
	{
		//Calculate inverse
		UtMaths::Invert4x4(&m_dynamicWorldMatrixInverseCache, m_dynamicWorldMatrixCache);
		//Calculate inverse transpose
		Matrix44 matrixTranspose = m_dynamicWorldMatrixCache;
		UtMaths::Transpose(&matrixTranspose);
		UtMaths::Invert4x4(&m_dynamicWorldMatrixInverseTransposeCache, matrixTranspose);
	}
#endif

	m_dynamicMatrixCacheDirtyFlag = false;
}

#endif
