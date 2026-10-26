#ifndef __PHYSICS_SPHERE_H__
#define __PHYSICS_SPHERE_H__

#include "Impact\Shape.h"

namespace Impact
{

/// sphere shape
class Sphere : public BaseShape
{
public:
	/// constructor
			Sphere(float fRadius, const Maths::HVec3 &vPos);

	/// create an instance of this Shape, to attach to a body
	virtual	Shape	*Instance();

private:
	float	m_fRadius;
	Maths::HVec3	m_vPos;	// position of the centre of the sphere relative to the origin
};

/// instance of a sphere containing instance specific data
class SphereInstance : public Shape
{
public:
	/// constructor
						SphereInstance();

	/// return the AABB of this sphere, given the body's position
	Maths::C_AxisBox	GetAABB(const Maths::HVec3 &vPos);

	/// return the AABB of this sphere, given the body's position
	Maths::C_AxisBox	GetAABB(const Body *pBody);

	/// apply the body's transform to the sphere position
	void				ApplyTransform();

	float				m_fRadius;
	Maths::HVec3		m_vLocalPos;	// position of the centre of the sphere relative to the origin
	Maths::HVec3		m_vWorldPos;	// world-space position
};

};

#endif
