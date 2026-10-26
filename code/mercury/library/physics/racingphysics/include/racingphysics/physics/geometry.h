#ifndef RP_GEOMETRY_H
#define RP_GEOMETRY_H
#pragma once

namespace RacingPhysics {void PhysicsError();}

#if !CFG_SPEED
#define RP_ASSERT(expr) do { if (!(expr)) {BIZ_ASSERT_FAIL(#expr);  PhysicsError();} } while(0)
#else
#define RP_ASSERT(expr)
#endif

//#define RP_USE_UTMATHS
#ifdef RP_USE_UTMATHS

#include "core/maths/utmaths/maths.h"
#include "core/maths/utmaths/mmatrix.h"


#else

#include "hvecsupport.h"

#endif

#ifndef RP_USE_UTMATHS

namespace RacingPhysics
{
	typedef Maths::C_Vertex Point3R;
	typedef Maths::C_Vector Vector3R;
	typedef Maths::C_Normal Normal3R;
	typedef Maths::HVec4 Vector4R;
	typedef Maths::HVec4 VectorD4;

	typedef Maths::HMat33 Matrix33R;
	typedef Maths::HMat44 Matrix44R;

	typedef Maths::HF32 FloatVR;
	typedef Maths::Scalar_Bool BoolVR;

	//typedef UtMaths::MPoint<UtMaths::VectorXYZ<float> > Point3M;
	//typedef UtMaths::MVector<UtMaths::VectorXYZ<float> > Vector3M;
	//typedef UtMaths::MNormal<UtMaths::VectorXYZ<float> > Normal3M;

	// convert does nothing
	FORCE_INLINE const Vector3R& ConvertHVec(const Maths::C_Vector& src) {return src;}
	FORCE_INLINE const Normal3R& ConvertHVec(const Maths::C_Normal& src) {return src;}
	FORCE_INLINE const Point3R& ConvertHVec(const Maths::C_Vertex& src) {return src;}
	FORCE_INLINE const Vector3R& ConvertHVec(const Maths::HVec3& src) {return (Vector3R&) src;}
	FORCE_INLINE const BoolVR& ConvertHVec(const Maths::Scalar_Bool& src) {return src;}
	FORCE_INLINE const FloatVR& ConvertHVec(const Maths::HF32& src) {return src;}
	FORCE_INLINE const Maths::HMat33& ConvertHVec(const Matrix33R& src) {return src;}
}

#else

namespace RacingPhysics
{

#define RP_USE_SIMD

#ifdef RP_USE_SIMD
	typedef UtMaths::XMVector_Wrapper<3> Vector3Base;
	typedef UtMaths::XMVector_Wrapper<4> Vector4Base;
#else
	typedef UtMaths::VectorXYZ<double> Vector3Base; // software
	typedef UtMaths::VectorXYZW<double> Vector4Base;
#endif

	// Basic vector types.
	// 3 = 3 component
	// R = Register
	typedef UtMaths::MPoint<Vector3Base> Point3R;
	typedef UtMaths::MVector<Vector3Base> Vector3R;
	typedef UtMaths::MNormal<Vector3Base> Normal3R;
	typedef UtMaths::MVector<Vector4Base> Vector4R;

	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector3R, 3> > Matrix33R;
	typedef UtMaths::MMatrix<UtMaths::BasicMatrix<Vector4R, 4> > Matrix44R;
	
#ifdef RP_USE_SIMD

	// scalar value in vector register
	typedef UtMaths::XMFloat_Wrapper FloatVR;
	typedef UtMaths::XMBoolean_Wrapper BoolVR;

	// Vector3R is exactly the same data structure as HVec
	FORCE_INLINE const Vector3R& ConvertHVec(const Maths::C_Vector& src) {return (Vector3R&) src;}
	FORCE_INLINE const Normal3R& ConvertHVec(const Maths::C_Normal& src) {return (Normal3R&) src;}
	FORCE_INLINE const Point3R& ConvertHVec(const Maths::C_Vertex& src) {return (Point3R&) src;}
	FORCE_INLINE const Vector3R& ConvertHVec(const Maths::HVec3& src) {return (Vector3R&) src;}
	FORCE_INLINE const BoolVR& ConvertHVec(const Maths::Scalar_Bool& src) {return (BoolVR&) src;}
	FORCE_INLINE const FloatVR& ConvertHVec(const Maths::HF32& src) {return (FloatVR&) src;}

	FORCE_INLINE const Maths::C_Vector& ConvertHVec(const Vector3R& src) {return (Maths::C_Vector&) src;}
	FORCE_INLINE const Maths::C_Normal& ConvertHVec(const Normal3R& src) {return (Maths::C_Normal&) src;}
	FORCE_INLINE const Maths::C_Vertex& ConvertHVec(const Point3R& src) {return (Maths::C_Vertex&) src;}
	FORCE_INLINE const Maths::HF32& ConvertHVec(const FloatVR& src) {return (Maths::HF32&) src;}

#else

	typedef double FloatVR;
	typedef bool BoolVR;

	FORCE_INLINE const Vector3R ConvertHVec(const Maths::C_Vector& src) {return Vector3R(src.X(), src.Y(), src.Z());}
	FORCE_INLINE const Normal3R ConvertHVec(const Maths::C_Normal& src) {return (Normal3R&)Vector3R(src.X(), src.Y(), src.Z());}
	FORCE_INLINE const Point3R ConvertHVec(const Maths::C_Vertex& src) {return Point3R(src.X(), src.Y(), src.Z());}
	FORCE_INLINE const Vector3R ConvertHVec(const Maths::HVec3& src) {return Vector3R(src.X(), src.Y(), src.Z());}
	FORCE_INLINE const BoolVR ConvertHVec(const Maths::Scalar_Bool& src) {return (BoolVR) src;}
	FORCE_INLINE const FloatVR ConvertHVec(const Maths::HF32& src) {return (FloatVR) src;}

	FORCE_INLINE const Maths::C_Vector ConvertHVec(const Vector3R& src) {return Maths::C_Vector(src.X(), src.Y(), src.Z());}
	FORCE_INLINE const Maths::C_Normal ConvertHVec(const Normal3R& src) {return Maths::C_Normal(src.X(), src.Y(), src.Z());}
	FORCE_INLINE const Maths::C_Vertex ConvertHVec(const Point3R& src) {return Maths::C_Vertex(src.X(), src.Y(), src.Z());}
	FORCE_INLINE const Maths::HF32 ConvertHVec(const FloatVR& src) {return (Maths::HF32) src;}

#endif

	FORCE_INLINE Maths::HMat33 ConvertHVec(const Matrix33R& src)
	{
		Maths::HMat33 dst;
		dst.AX(ConvertHVec(src.AX()));
		dst.AY(ConvertHVec(src.AY()));
		dst.AZ(ConvertHVec(src.AZ()));
		return dst;
	}
}

#endif
#endif
