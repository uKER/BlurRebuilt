////////////////////////////////////////////////////////////////////////////////
// GmQuaternion
////////////////////////////////////////////////////////////////////////////////

namespace Gm {

/// Quaternion.
class GmQuaternion
{
public:
	typedef UtMaths::VectorXYZ<float> VectorType;

private:
	float w;
	UtMaths::MVector<VectorType> a;

public:
	// constructors
	GmQuaternion() {} 
	GmQuaternion(float s, const UtMaths::MVector<VectorType>& v) : a(v), w(s) {}
	explicit GmQuaternion(const GmRotationMatrix& m);
	explicit GmQuaternion(const UtMaths::MVector<VectorType>& v) : a(v) {float f = v | v; w = (f < 1.f) ? sqrt(1.f - (v | v)) : 0.f;}
	GmQuaternion(const UtMaths::MVector<VectorType>& vFrom, const UtMaths::MVector<VectorType>& vTo);
	
	// initialisation
	void SetIdentity() {w = 1.f; a = UtMaths::MVector<VectorType>(0, 0, 0);} ///<
	void SetRotation(const GmRotationMatrix& rotationMatrix); ///< Set rotation from matrix.
	void SetRotation(const GmNormal& axis, float angle); ///< Set from axis and angle.
	void SetComponents(float inX, float inY, float inZ, float inW)	{ a.x = inX; a.y = inY; a.z = inZ; w = inW; } ///< Directly set components


	GmQuaternion& operator = (const GmQuaternion& q) {a = q.a; w = q.w; return *this;}

	// access
	float W() const {return w;} ///< Scalar component.
	const UtMaths::MVector<VectorType>& A() const {return a;} ///< Vector component.

	// information
	GmQuaternion operator +(const GmQuaternion& q) const;
	GmQuaternion operator -(const GmQuaternion& q) const;
	GmQuaternion Conjugate() const;
	float Norm() const; ///< Square root of sum of components squared
	float		GetRotationAngle() const;	///< Get rotation angle.
	GmNormal	GetRotationAxis() const;	///< Get rotation axis.

	// modification
	void Rotate(const UtMaths::MVector<VectorType>& v); ///< Rotate using vector.
	void Rotate(const GmNormal& v, float a); ///< Rotate using vector and angle.
	void operator +=(const GmQuaternion& q);
	void operator -=(const GmQuaternion& q);
	void operator *=(const GmQuaternion& q);
	void operator *=(float f);
	void operator /=(float f);
	void Normalise(); ///< Divide by Norm.
	void BlendFast(const GmQuaternion& q1, const GmQuaternion& q2, float f); ///< Blend between two quaternions that are quite close.
	void Negate(); ///< Negate components giving same rotation on different hemisphere.

	friend GmQuaternion operator *(const GmQuaternion& p1, const GmQuaternion& p2);
	template <typename VectorType> 
	friend GmQuaternion operator *(const GmQuaternion& q, const UtMaths::MVector<VectorType>& v);

	friend GmQuaternion operator *(const GmQuaternion& q, float f);

	friend float operator |(const GmQuaternion& p1, const GmQuaternion& p2);

	UtMaths::MVector<VectorType>	GetRotatedVector(const UtMaths::MVector<VectorType>& v) const;	///< Get a vector rotated by this quaternion.
	UtMaths::MPoint<VectorType>		GetRotatedPoint(const UtMaths::MPoint<VectorType>& p) const;	///< Get a point rotated by this quaternion.
};

inline GmQuaternion GmQuaternion::operator +(const GmQuaternion& q) const
{
	return GmQuaternion(w + q.w, a + q.a);
}

inline GmQuaternion GmQuaternion::operator -(const GmQuaternion& q) const
{
	return GmQuaternion(w - q.w, a - q.a);
}

inline float GmQuaternion::Norm() const
{
	return sqrt(w * w + (a | a));
}

inline void GmQuaternion::Normalise()
{
	*this /= Norm();
}

inline void GmQuaternion::operator +=(const GmQuaternion& q)
{
	w += q.w;
	a += q.a;
}

inline void GmQuaternion::operator -=(const GmQuaternion& q)
{
	w -= q.w;
	a -= q.a;
}

inline void GmQuaternion::operator /=(float f)
{
	w /= f;
	a /= f;
}

inline void GmQuaternion::operator *=(float f)
{
	w *= f;
	a *= f;
}

inline void GmQuaternion::operator *=(const GmQuaternion& q)
{
	*this = *this * q;
}

inline void GmQuaternion::SetRotation(const GmNormal& axis, float angle)
{
	const float sinHalfAngle = sinf(angle * 0.5f);
	a.x = axis.x * sinHalfAngle;
	a.y = axis.y * sinHalfAngle;
	a.z = axis.z * sinHalfAngle;
	w   = cosf(angle * 0.5f);
}

inline GmQuaternion operator *(const GmQuaternion& p1, const GmQuaternion& p2)
{
	GmQuaternion q2;

	q2.w = p1.w * p2.w - (p1.a | p2.a);
	q2.a = p1.a * p2.w + p2.a * p1.w + (p1.a ^ p2.a);

	return q2;
}

template <typename VectorType>
inline GmQuaternion operator *(const GmQuaternion& q, const UtMaths::MVector<VectorType>& v)
{
	GmQuaternion q2;

	q2.w = -(q.a | v);
	q2.a = v * q.w + (q.a ^ v);

	return q2;
}

inline float operator |(const GmQuaternion& p1, const GmQuaternion& p2)
{
	return p1.w * p2.w + (p1.a | p2.a);
}

inline GmQuaternion operator *(const GmQuaternion& q, float f)
{
	GmQuaternion q2;

	q2.w = q.w * f;
	q2.a = q.a * f;

	return q2;
}

inline UtMaths::MVector<GmQuaternion::VectorType> GmQuaternion::GetRotatedVector(const UtMaths::MVector<VectorType>& v) const
{
    return (*this * v * Conjugate()).A();
}

inline UtMaths::MPoint<GmQuaternion::VectorType> GmQuaternion::GetRotatedPoint(const UtMaths::MPoint<VectorType>& p) const
{
	return MakePoint((*this * MakeVector(p) * Conjugate()).A());
}

} // End Gm