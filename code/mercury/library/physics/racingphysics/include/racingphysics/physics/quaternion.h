////////////////////////////////////////////////////////////////////////////////
// Quaternion
////////////////////////////////////////////////////////////////////////////////

#ifndef RP_QUATERNION_H
#define RP_QUATERNION_H
#ifdef _MSC_VER
#pragma once
#endif

#include "geometry.h"

namespace RacingPhysics 
{

class RotationMatrix;

/// Quaternion.
class Quaternion
{
private:
	float w;
	Vector3R a;

public:
	// constructors
	Quaternion() {} 
	Quaternion(float s, const Vector3R& v) : a(v), w(s) {}
	explicit Quaternion(const RotationMatrix& m);
	explicit Quaternion(const Vector3R& v) : a(v) {float f = v | v; w = (f < 1.f) ? sqrt(1.f - (v | v)) : 0.f;}
	Quaternion(const Vector3R& vFrom, const Vector3R& vTo);
	
	// initialisation
	void SetIdentity() {w = 1.f; a = Vector3R(0, 0, 0);} ///<
	void SetRotation(const RotationMatrix& rotationMatrix); ///< Set rotation from matrix.
	void SetRotation(const Normal3R& axis, float angle); ///< Set from axis and angle.
	void SetComponents(float inX, float inY, float inZ, float inW)	{ a.X(inX); a.Y(inY); a.Z(inZ); w = inW; } ///< Directly set components


	Quaternion& operator = (const Quaternion& q) {a = q.a; w = q.w; return *this;}

	// access
	float W() const {return w;} ///< Scalar component.
	const Vector3R& A() const {return a;} ///< Vector component.

	// information
	Quaternion operator +(const Quaternion& q) const;
	Quaternion operator -(const Quaternion& q) const;
	Quaternion Conjugate() const;
	float Norm() const; ///< Square root of sum of components squared
	float		GetRotationAngle() const;	///< Get rotation angle.
	Normal3R	GetRotationAxis() const;	///< Get rotation axis.

	// modification
	void Rotate(const Vector3R& v); ///< Rotate using vector.
	void Rotate(const Normal3R& v, float a); ///< Rotate using vector and angle.
	void operator +=(const Quaternion& q);
	void operator -=(const Quaternion& q);
	void operator *=(const Quaternion& q);
	void operator *=(float f);
	void operator /=(float f);
	void Normalise(); ///< Divide by Norm.
	void BlendFast(const Quaternion& q1, const Quaternion& q2, float f); ///< Blend between two quaternions that are quite close.
	void Negate(); ///< Negate components giving same rotation on different hemisphere.

	friend Quaternion operator *(const Quaternion& p1, const Quaternion& p2);
	friend Quaternion operator *(const Quaternion& q, const Vector3R& v);
	friend Quaternion operator *(const Quaternion& q, float f);

	friend float operator |(const Quaternion& p1, const Quaternion& p2);

	Vector3R GetRotatedVector(const Vector3R& v) const;	///< Get a vector rotated by this quaternion.
	Point3R GetRotatedPoint(const Point3R & p) const;	///< Get a point rotated by this quaternion.
};

inline Quaternion Quaternion::operator +(const Quaternion& q) const
{
	return Quaternion(w + q.w, a + q.a);
}

inline Quaternion Quaternion::operator -(const Quaternion& q) const
{
	return Quaternion(w - q.w, a - q.a);
}

inline float Quaternion::Norm() const
{
	return sqrt(w * w + (a | a));
}

inline void Quaternion::Normalise()
{
	*this /= Norm();
}

inline void Quaternion::operator +=(const Quaternion& q)
{
	w += q.w;
	a += q.a;
}

inline void Quaternion::operator -=(const Quaternion& q)
{
	w -= q.w;
	a -= q.a;
}

inline void Quaternion::operator /=(float f)
{
	w /= f;
	a /= f;
}

inline void Quaternion::operator *=(float f)
{
	w *= f;
	a *= f;
}

inline void Quaternion::operator *=(const Quaternion& q)
{
	*this = *this * q;
}

inline void Quaternion::SetRotation(const Normal3R& axis, float angle)
{
	const float sinHalfAngle = sinf(angle * 0.5f);
	a.X(axis.X() * sinHalfAngle);
	a.Y(axis.Y() * sinHalfAngle);
	a.Z(axis.Z() * sinHalfAngle);
	w   = cosf(angle * 0.5f);
}

inline Quaternion operator *(const Quaternion& p1, const Quaternion& p2)
{
	Quaternion q2;

	q2.w = p1.w * p2.w - (p1.a | p2.a);
	q2.a = p1.a * p2.w + p2.a * p1.w + (p1.a ^ p2.a);

	return q2;
}

inline Quaternion operator *(const Quaternion& q, const Vector3R& v)
{
	Quaternion q2;

	q2.w = -(q.a | v);
	q2.a = v * q.w + (q.a ^ v);

	return q2;
}

inline float operator |(const Quaternion& p1, const Quaternion& p2)
{
	return p1.w * p2.w + (p1.a | p2.a);
}

inline Quaternion operator *(const Quaternion& q, float f)
{
	Quaternion q2;

	q2.w = q.w * f;
	q2.a = q.a * f;

	return q2;
}

inline Vector3R Quaternion::GetRotatedVector(const Vector3R& v) const
{
    return (*this * v * Conjugate()).A();
}

inline Point3R Quaternion::GetRotatedPoint(const Point3R& p) const
{
	return MakePoint((*this * MakeVector(p) * Conjugate()).A());
}

inline Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float f)
{
	Quaternion q;

	if ((q1 | q2) > 0.f)
		q = q2 * f + q1 * (1.f - f);
	else
		q = q2 * f - q1 * (1.f - f);

	q.Normalise();
	return q;
}

}

#endif

