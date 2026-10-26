#ifndef RP_BOUNDING_BOX_H
#define RP_BOUNDING_BOX_H
#ifdef _MSC_VER
#pragma once
#endif

#include "racingphysics/physics/geometry.h"

namespace RacingPhysics
{

// Optimised for Xenon
// Use for bounding box tests against triangles, edges or vertices.
class BoundingBoxCollision
{
public:

	typedef FloatVR ScalarType;
	typedef BoolVR BooleanType;

	// Initialise
	BoundingBoxCollision() {}
	FORCE_INLINE BoundingBoxCollision(const Point3R& v0, const Point3R& v1);
	FORCE_INLINE void SetMinMax(const Point3R& min, const Point3R& max);

	// info
	const Point3R& GetMin() const {return m_min;}
	const Point3R& GetMax() const {return m_max;}
	const Point3R& GetCentre() const {return m_centre;}
	const Vector3R& GetSize() const {return m_size;}

	// Bounding box tests.
	FORCE_INLINE BooleanType TestBounds(const Point3R& v0, const Point3R& v1, const Point3R& v2) const;
	FORCE_INLINE BooleanType TestBounds(const Point3R& v0, const Point3R& v1) const;
	FORCE_INLINE BooleanType TestBoundsXZ(const Point3R& v0, const Point3R& v1) const;
	FORCE_INLINE BooleanType TestBounds(const Point3R& v) const;

	// Full collision against triangle.
	BooleanType CollideTriangle(const Point3R& v0, const Point3R& v1, const Point3R& v2) const;

	// Just tests the triangle edges in XZ against the bounding box.
	// (this plus the bounds test is good for rejecting ground triangles)
	BooleanType TestTriangleEdgesXZ(const Point3R& v0, const Point3R& v1, const Point3R& v2) const;

	// Just test the line normal in XZ against the bounding box.
	BooleanType TestLineNormalXZ(const Point3R& v0, const Point3R& v1) const;

	// Test against another bounding box.
	FORCE_INLINE BooleanType TestOverlap(const Point3R& min, const Point3R& max) const;
	FORCE_INLINE BooleanType TestOverlap(const Point3R& min, const Point3R& max, const FloatVR& tolerance) const;
	FORCE_INLINE BooleanType TestOverlapXZ(const Point3R& min, const Point3R& max) const;
	FORCE_INLINE BooleanType Contains(const Point3R& min, const Point3R& max, const FloatVR& tolerance) const;
	FORCE_INLINE BooleanType Contains(const Point3R& min, const Point3R& max) const;

private:
	Point3R m_max;
	Point3R m_min;
	Vector3R m_size;
	Point3R m_centre;
};

//////////////////////////////////////////////////////////////////////////////

FORCE_INLINE BoundingBoxCollision::BoundingBoxCollision(const Point3R& v0, const Point3R& v1)
{
	m_max = Max(v0, v1);
	m_min = Min(v0, v1);

	m_size = (m_max - m_min) * Maths::Constants<ScalarType>::Half();
	m_centre = MakePoint((MakeVector(m_max) + MakeVector(m_min)) * Maths::Constants<ScalarType>::Half());
}

FORCE_INLINE void BoundingBoxCollision::SetMinMax(const Point3R& min, const Point3R& max) 
{
	m_max = max;
	m_min = min;

	m_size = (m_max - m_min) * Maths::Constants<ScalarType>::Half();
	m_centre = MakePoint((MakeVector(m_max) + MakeVector(m_min)) * Maths::Constants<ScalarType>::Half());
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestBounds(const Point3R& v0, const Point3R& v1, const Point3R& v2) const
{
	Point3R min, max;

	min = Min(v0, v1);
	min = Min(min, v2);

	max = Max(v0, v1);
	max = Max(max, v2);

	return TestOverlap(min, max);
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestBounds(const Point3R& v0, const Point3R& v1) const
{
	Point3R min = Min(v0, v1);
	Point3R max = Max(v0, v1);

	return TestOverlap(min, max);
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestBoundsXZ(const Point3R& v0, const Point3R& v1) const
{
	Point3R min = Min(v0, v1);
	Point3R max = Max(v0, v1);

	return TestOverlapXZ(min, max);
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestBounds(const Point3R& v) const
{
	return TestOverlap(v, v);
}

//#ifdef RP_USE_SIMD

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestOverlap(const Point3R& min, const Point3R& max) const
{
	Maths::HVec3 bmin, bmax, pmin, pmax;
	Maths::C_Vector xyz = Maths::Constants<Maths::C_Vector>::XYZ();
	Maths::C_Vector null = Maths::Constants<Maths::C_Vector>::Null();
	Maths::HF32 zero = Maths::Constants<Maths::HF32>::Zero();

	bmin = ConvertHVec(m_min);
	bmax = ConvertHVec(m_max);
	pmin = ConvertHVec(min);
	pmax = ConvertHVec(max);

	Maths::Vector3_Bool b = (bmax < pmin) | (bmin > pmax);

	//XMVECTOR b = XMVectorOrInt(XMVectorLess((XMVECTOR&) m_max, (XMVECTOR&) min), XMVectorGreater((XMVECTOR&) m_min, (XMVECTOR&) max));
	//XMVECTOR v = XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), b);
	//XMVECTOR result = XMVectorEqual(XMVector4Dot(v, Maths::XMVECTOR_CONSTANT_XYZ), XMVectorZero());
	
	Maths::C_Vector v = Maths::Select(b, xyz, null);
	Maths::Scalar_Bool result = Maths::Scalar_Bool((v | v) == zero);
	return ConvertHVec(result);
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestOverlap(const Point3R& min, const Point3R& max, const FloatVR& tolerance) const
{
	Maths::HVec3 bmin, bmax, pmin, pmax;
	Maths::C_Vector xyz = Maths::Constants<Maths::C_Vector>::XYZ();
	Maths::C_Vector null = Maths::Constants<Maths::C_Vector>::Null();
	Maths::HF32 zero = Maths::Constants<Maths::HF32>::Zero();
	Maths::HVec3 vt(ConvertHVec(tolerance));

	bmin = ConvertHVec(m_min);
	bmax = ConvertHVec(m_max);
	pmin = ConvertHVec(min);
	pmax = ConvertHVec(max);

	Maths::Vector3_Bool b = (bmax + vt < pmin) | bmin - vt > pmax;

	//XMVECTOR test = XMVectorOrInt(XMVectorLess((XMVECTOR&) m_max + (XMVECTOR&) tolerance, (XMVECTOR&) min),
	//								XMVectorGreater((XMVECTOR&) m_min - (XMVECTOR&) tolerance, (XMVECTOR&) max));

	//XMVECTOR result = XMVectorEqual(XMVector4Dot(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), test), Maths::XMVECTOR_CONSTANT_XYZ), XMVectorZero());
	//return (BooleanType&) result;

	Maths::C_Vector v = Maths::Select(b, xyz, null);
	Maths::Scalar_Bool result = Maths::Scalar_Bool((v | v) == zero);
	return ConvertHVec(result);
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestOverlapXZ(const Point3R& min, const Point3R& max) const
{
	Maths::HVec3 bmin, bmax, pmin, pmax;
	Maths::C_Vector xz = Maths::Constants<Maths::C_Vector>::XZ();
	Maths::C_Vector null = Maths::Constants<Maths::C_Vector>::Null();
	Maths::HF32 zero = Maths::Constants<Maths::HF32>::Zero();

	bmin = ConvertHVec(m_min);
	bmax = ConvertHVec(m_max);
	pmin = ConvertHVec(min);
	pmax = ConvertHVec(max);

	Maths::Vector3_Bool b = (bmax < pmin) | (bmin > pmax);

	//XMVECTOR test = XMVectorOrInt(XMVectorLess((XMVECTOR&) m_max, (XMVECTOR&) min),
	//								XMVectorGreater((XMVECTOR&) m_min, (XMVECTOR&) max));

	//XMVECTOR result = XMVectorEqual(XMVector4Dot(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), test), Maths::XMVECTOR_CONSTANT_XYZ), XMVectorZero());
	//return (BooleanType&) result;

	Maths::C_Vector v = Maths::Select(b, xz, null);
	Maths::Scalar_Bool result = Maths::Scalar_Bool((v | v) == zero);
	return ConvertHVec(result);
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::Contains(const Point3R& min, const Point3R& max) const
{
	Maths::HVec3 bmin, bmax, pmin, pmax;
	Maths::C_Vector xyz = Maths::Constants<Maths::C_Vector>::XYZ();
	Maths::C_Vector null = Maths::Constants<Maths::C_Vector>::Null();
	Maths::HF32 zero = Maths::Constants<Maths::HF32>::Zero();

	bmin = ConvertHVec(m_min);
	bmax = ConvertHVec(m_max);
	pmin = ConvertHVec(min);
	pmax = ConvertHVec(max);

	Maths::Vector3_Bool b = (bmin > pmin) | (bmax < pmax);

	//XMVECTOR test = XMVectorOrInt(XMVectorGreater((XMVECTOR&) m_min, (XMVECTOR&) min),
	//								XMVectorLess((XMVECTOR&) m_max, (XMVECTOR&) max));

	//XMVECTOR result = XMVectorEqual(XMVector4Dot(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), test), Maths::XMVECTOR_CONSTANT_XYZ), XMVectorZero());
	//return (BooleanType&) result;

	Maths::C_Vector v = Maths::Select(b, xyz, null);
	Maths::Scalar_Bool result = Maths::Scalar_Bool((v | v) == zero);
	return ConvertHVec(result);
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::Contains(const Point3R& min, const Point3R& max, const FloatVR& tolerance) const
{
	Maths::HVec3 bmin, bmax, pmin, pmax;
	Maths::C_Vector xyz = Maths::Constants<Maths::C_Vector>::XYZ();
	Maths::C_Vector null = Maths::Constants<Maths::C_Vector>::Null();
	Maths::HF32 zero = Maths::Constants<Maths::HF32>::Zero();
	Maths::HVec3 vt(ConvertHVec(tolerance));

	bmin = ConvertHVec(m_min);
	bmax = ConvertHVec(m_max);
	pmin = ConvertHVec(min);
	pmax = ConvertHVec(max);

	Maths::Vector3_Bool b = (bmin - vt > pmin) | (bmax + vt < pmax);

	//XMVECTOR test = XMVectorOrInt(XMVectorGreater((XMVECTOR&) m_min - (XMVECTOR&) tolerance, (XMVECTOR&) min),
	//								XMVectorLess((XMVECTOR&) m_max + (XMVECTOR&) tolerance, (XMVECTOR&) max));

	//XMVECTOR result = XMVectorEqual(XMVector4Dot(XMVectorSelect(XMVectorZero(), XMVectorSplatOne(), test), Maths::XMVECTOR_CONSTANT_XYZ), XMVectorZero());
	//return (BooleanType&) result;

	Maths::C_Vector v = Maths::Select(b, xyz, null);
	Maths::Scalar_Bool result = Maths::Scalar_Bool((v | v) == zero);
	return ConvertHVec(result);
}

//#else

/*FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestOverlap(const Point3R& min, const Point3R& max) const
{
	if (min.X() <= m_max.X() && max.X() >= m_min.X()
		&& min.Y() <= m_max.Y() && max.Y() >= m_min.Y()
		&& min.Z() <= m_max.Z() && max.Z() >= m_min.Z())
		return true;
	else
		return false;
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestOverlap(const Point3R& min, const Point3R& max, const FloatVR& tolerance) const
{
	if (min.X() <= m_max.X() + tolerance && max.X() >= m_min.X() - tolerance
		&& min.Y() <= m_max.Y() + tolerance && max.Y() >= m_min.Y() - tolerance
		&& min.Z() <= m_max.Z() + tolerance && max.Z() >= m_min.Z() - tolerance)
		return true;
	else
		return false;
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::TestOverlapXZ(const Point3R& min, const Point3R& max) const
{
	if (min.X() <= m_max.X() && max.X() >= m_min.X()
		&& min.Z() <= m_max.Z() && max.Z() >= m_min.Z())
		return true;
	else
		return false;
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::Contains(const Point3R& min, const Point3R& max) const
{
	if (min.X() >= m_min.X() && max.X() <= m_max.X()
		&& min.Y() >= m_min.Y() && max.Y() <= m_max.Y()
		&& min.Z() >= m_min.Z() && max.Z() <= m_max.Z())
		return true;
	else
		return false;
}

FORCE_INLINE BoundingBoxCollision::BooleanType BoundingBoxCollision::Contains(const Point3R& min, const Point3R& max, const FloatVR& tolerance) const
{
	if (min.X() >= m_min.X() - tolerance && max.X() <= m_max.X() + tolerance
		&& min.Y() >= m_min.Y() - tolerance && max.Y() <= m_max.Y() + tolerance
		&& min.Z() >= m_min.Z() - tolerance && max.Z() <= m_max.Z() + tolerance)
		return true;
	else
		return false;
}*/

//#endif

}

#endif
