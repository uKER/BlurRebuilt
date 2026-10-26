#include "stdafx.h"

#include "Impact\ContactUID.h"
#include "Impact\CollisionUtil.h"
#include "Impact\Sphere.h"
#include "Impact\Body.h"

using namespace Maths;

namespace Impact
{

/// constructor
Sphere::Sphere(float fRadius, const Maths::HVec3 &vPos)
{
	m_fRadius = fRadius;
	m_vPos = vPos;
}

/// create an instance of this Shape, to attach to a body
Shape *Sphere::Instance()
{
	SphereInstance *p = new SphereInstance();
	p->m_fRadius = m_fRadius;
	p->m_vLocalPos = m_vPos;
	return p;
}

/// constructor
SphereInstance::SphereInstance()
{
	m_type = E_ShapeType_Sphere;
}

C_AxisBox SphereInstance::GetAABB(const Maths::HVec3 &vPos)
{
	return C_AxisBox( vPos-HVec3(m_fRadius), vPos+HVec3(m_fRadius) );
}

/// return the AABB of this sphere, given the body's position
C_AxisBox SphereInstance::GetAABB(const Body *pBody)
{
	C_AxisBox box = GetAABB(pBody->m_vPos + m_vLocalPos * pBody->m_rotation);
	return box;
}

/// apply the body's transform to the sphere position
void SphereInstance::ApplyTransform()
{
	if (m_bDirty)
	{
		m_vWorldPos = m_vPos + m_vLocalPos * m_qRotation;
		m_bDirty = false;
	}
}


}; // namespace Impact
