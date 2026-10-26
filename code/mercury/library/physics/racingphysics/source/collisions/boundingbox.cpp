#include "stdafx.h"
#include "core/maths.h"
#include "racingphysics/collisions/boundingbox.h"

//using namespace Maths;

//#pragma optimize("", off)

////////////////////////////////////////////////

namespace RacingPhysics
{

FORCE_INLINE BoundingBoxCollision::BooleanType RejectAxis(const Normal3R& e, // axis
														 const Vector3R& f, // triangle edge
														 const Vector3R& v0, const Vector3R& v1, // two triangle points, one on the edge
														 const Vector3R& size // box size
														 )
{
	Normal3R axis = Normalised(e ^ f);
	Vector3R b = Abs(ComponentMultiply(axis, size));
	BoundingBoxCollision::ScalarType s = b | Maths::Constants<Vector3R>::XYZ();
	BoundingBoxCollision::ScalarType t0 = axis | v0;
	BoundingBoxCollision::ScalarType t1 = axis | v1;

	return ((Min(t0, t1) > s) | (Max(t0, t1) < -s));
}

FORCE_INLINE BoundingBoxCollision::BooleanType RejectAxis(const Normal3R& axis, // axis
															const Vector3R& v0, // single point on the line
															const Vector3R& size // box size
														 )
{
	Vector3R b = Abs(ComponentMultiply(axis, size));
	BoundingBoxCollision::ScalarType s = b | Maths::Constants<Vector3R>::XYZ();
	BoundingBoxCollision::ScalarType d = v0 | axis;
	return (Abs(d) > s);
}

// Full collision against triangle.
BoundingBoxCollision::BooleanType BoundingBoxCollision::CollideTriangle(const Point3R& v0, const Point3R& v1, const Point3R& v2) const
{
	// test bounding box first
	if (!TestBounds(v0, v1, v2))
		return Maths::Constants<BooleanType>::False();

	Normal3R triangleNormal = Normalised((v0 - v1) ^ (v2 - v1));
	Vector3R l0 = v0 - m_centre;

	// test triangle normal
	if (RejectAxis(triangleNormal, l0, m_size))
		return Maths::Constants<BooleanType>::False();

	Vector3R l1 = v1 - m_centre;
	Vector3R l2 = v2 - m_centre;

	Normal3R x = Maths::Constants<Normal3R>::X();
	Normal3R y = Maths::Constants<Normal3R>::Y();
	Normal3R z = Maths::Constants<Normal3R>::Z();

	// test edges
	if (RejectAxis(x, l1 - l0, l0, l2, m_size)
		| RejectAxis(x, l2 - l1, l1, l0, m_size)
		| RejectAxis(x, l0 - l2, l2, l1, m_size)
		| RejectAxis(y, l1 - l0, l0, l2, m_size)
		| RejectAxis(y, l2 - l1, l1, l0, m_size)
		| RejectAxis(y, l0 - l2, l2, l1, m_size)
		| RejectAxis(z, l1 - l0, l0, l2, m_size)
		| RejectAxis(z, l2 - l1, l1, l0, m_size)
		| RejectAxis(z, l0 - l2, l2, l1, m_size))
		return Maths::Constants<BooleanType>::False();

	return Maths::Constants<BooleanType>::True();
}

// Test triangle edges in XZ plane (good for rough test against ground triangles) 
BoundingBoxCollision::BooleanType BoundingBoxCollision::TestTriangleEdgesXZ(const Point3R& v0, const Point3R& v1, const Point3R& v2) const
{
	Vector3R l0 = v0 - m_centre;
	Vector3R l1 = v1 - m_centre;
	Vector3R l2 = v2 - m_centre;

	Normal3R y = Maths::Constants<Normal3R>::Y();

	if (RejectAxis(y, l1 - l0, l0, l2, m_size)
		|| RejectAxis(y, l2 - l1, l1, l0, m_size)
		|| RejectAxis(y, l0 - l2, l2, l1, m_size))
		return Maths::Constants<BooleanType>::False();

	return Maths::Constants<BooleanType>::True();
}

// Test line normal in XZ plane (good for rough test against ground triangles) 
BoundingBoxCollision::BooleanType BoundingBoxCollision::TestLineNormalXZ(const Point3R& v0, const Point3R& v1) const
{
	return !RejectAxis(Normalised(Maths::Constants<Normal3R>::Y() ^ (v0 - v1)), v0 - m_centre, m_size);
}

}
