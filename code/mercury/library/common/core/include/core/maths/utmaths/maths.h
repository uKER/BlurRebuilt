#ifndef CORE_MATHS_H
#define CORE_MATHS_H

#ifdef _MSC_VER_
#pragma once
#endif

#include <btl/utility/types.h>
#include <core/maths/utmaths/mvector.h>
#include <core/maths/constants.h>

#if defined (PLATFORM_PC) 
//	#if (defined (PLATFORM_WINDOWS) && (WORD_SIZE == 64))
//		#pragma message ("*** Warning: Do SSE intrinsics work under win64 the same as win32? ***")
//	#endif
//	#if (defined (PLATFORM_WINDOWS) && (WORD_SIZE == 64))
//		#pragma message ("*** Warning: Do SSE intrinsics work under win64 the same as win32? ***")
//	#endif
	//#pragma message ("UtMaths: Using intrinsics")
	#include <xmmintrin.h>
#else
	#define __m128	__vector4
	//#pragma message ("UtMaths: Not using intrinsics")
#endif


namespace UtMaths
{

///////////////////////////////////////////////////////////////////////////
// Vector description types

// derive from this
template <typename T, int N>
class BaseVector
{
public:
	BaseVector() {} ///< Default constructor.

	// array-style access
	UT_INLINE int Size() const {return SIZE;}
	UT_INLINE T& operator [] (int i) {assert(i > -1 && i < SIZE); return *(((T*)this) + i);}
	UT_INLINE const T& operator [] (int i) const {assert(i > -1 && i < SIZE); return *(((T*)this) + i);}

	/// Access to elements for the vector maths functions.
	/// Because I is a template parameter, the component is guaranteed to be known at compile time.
	template <int I>
	UT_INLINE T& At() {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *(((T*)this) + I);}

	template <int I>
	UT_INLINE const T& At() const {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *(((T*)this) + I);} ///< Access to element I

	typedef T DotProductType;
	typedef T DataType; ///< The type stored in the vector.
	const static int SIZE = N; ///< The number of elements in the vector.

	// Modification operators.
	// Could also use the BasicVector template to avoid defining these here.

	// Sets each element of the vector to the result provided by the evaluator.
	// The Evaluator needs to have an At<I>() function which returns a suitable type.
	template <typename Evaluator>
	UT_INLINE void operator = (const Evaluator& e) {Recurse<SIZE>::Set(*this, e);} ///< @a

	// A suitable scalar is anything that the elements can be multiplied by.
	template <typename Scalar>
	UT_INLINE void operator *= (Scalar e) {Recurse<SIZE>::Multiply(*this, e);} ///< @a

	template <typename Scalar>
	UT_INLINE void operator /= (Scalar e) {Recurse<SIZE>::Divide(*this, e);} ///< @a

	template <typename Evaluator>
	UT_INLINE void operator += (const Evaluator& e) {Recurse<SIZE>::Add(*this, e);} ///< @a

	template <typename Evaluator>
	UT_INLINE void operator -= (const Evaluator& e) {Recurse<SIZE>::Subtract(*this, e);} ///< @a

protected:
	// types for constructors
	typedef Ut::NullType type1;
	typedef Ut::NullType type2;
};

// Like BaseVector but without modification operators.
// This makes it easier to convert vectors that already have these to MVectors.
template <typename T, int N>
class ConversionVector
{
public:
	ConversionVector() {} ///< Default constructor.

	// array-style access
	int Size() const {return SIZE;}
	T& operator [] (int i) {assert(i > -1 && i < SIZE); return *(((T*)this) + i);}
	const T& operator [] (int i) const {assert(i > -1 && i < SIZE); return *(((T*)this) + i);}

	/// Access to elements for the vector maths functions.
	/// Because I is a template parameter, the component is guaranteed to be known at compile time.
	template <int I>
	T& At() {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *(((T*)this) + I);}

	template <int I>
	const T& At() const {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *(((T*)this) + I);} ///< Access to element I

	typedef T DotProductType;
	typedef T DataType; ///< The type stored in the vector.
	const static int SIZE = N; ///< The number of elements in the vector.

	// Sets each element of the vector to the result provided by the evaluator.
	// The Evaluator needs to have an At<I>() function which returns a suitable type.
	template <typename Evaluator>
	UT_INLINE void operator = (const Evaluator& e) {Recurse<SIZE>::Set(*this, e);} ///< @a

protected:
	// types for constructors
	typedef Ut::NullType type1;
	typedef Ut::NullType type2;
};

/// 2 component vector with named elements.
template <typename T>
class VectorXY : public BaseVector<T, 2>
{
public:
	/// Named elements for convenience.
	T x, y;

	// constructors
	VectorXY() {} ///< Default constructor.
	VectorXY(T px, T py) : x(px), y(py) {}  ///< Three element constructor.

	template <typename Evaluator>
	UT_INLINE VectorXY(const Evaluator& e) {BaseVector<T, 2>::operator = (e);}

	// access
	T X() const {return x;} ///< Access x component.
	T Y() const {return y;} ///< Access y component.
};

/// 3 component vector with named elements.
template <typename T>
class VectorXYZ : public BaseVector<T, 3>
{
public:
	/// Named elements for convenience.
	T x, y, z;

	// constructors
	VectorXYZ() {} ///< Default constructor.
	VectorXYZ(T px, T py, T pz) : x(px), y(py), z(pz) {}  ///< Three element constructor.

	template <typename Evaluator>
	UT_INLINE VectorXYZ(const Evaluator& e) {BaseVector<T, 3>::operator = (e);}

	// access
	T X() const {return x;} ///< Access x component.
	T Y() const {return y;} ///< Access y component.
	T Z() const {return z;} ///< Access z component.

	void X(const T& f) {x = f;} 
	void Y(const T& f) {y = f;} 
	void Z(const T& f) {z = f;} 
};

/// 4 component vector with named elements.
template <typename T>
class VectorXYZW : public BaseVector<T, 4>
{
public:
	typedef VectorXYZ<T> VectorXYZ; 

	T x, y, z, w;

	// constructors
	VectorXYZW() {} ///< Default constructor.
	VectorXYZW(T px, T py, T pz, T pw) : x(px), y(py), z(pz), w(pw) {}  ///< Four element constructor.

	template <typename Evaluator>
	UT_INLINE VectorXYZW(const Evaluator& e) {BaseVector<T, 4>::operator = (e);}

	// access
	T X() const {return x;} ///< Access x component.
	T Y() const {return y;} ///< Access y component.
	T Z() const {return z;} ///< Access z component.
	T W() const {return w;} ///< Access w component.

	void X(const T& f) {x = f;} 
	void Y(const T& f) {y = f;} 
	void Z(const T& f) {z = f;} 
	void W(const T& f) {w = f;} 
};

//////////////////////////////////////////////////////////////////////////
// Vector utility functions

/// True if equal to null vector.
template <typename Vector>
UT_INLINE bool IsEmpty(const MConstVector<Vector>& v)
{
	return Recurse<Vector::SIZE>::IsEmpty(v);
}

// magnitude squared
template <typename Vector>
UT_INLINE typename Vector::DotProductType Magnitude2(const MConstVector<Vector>& v)
{
	return (v | v);
}

/// Size of the vector.
template <typename Vector>
UT_INLINE typename Vector::DataType Magnitude(const MConstVector<Vector>& v)
{
	return Maths::Sqrt(Magnitude2(v));
}

/// Calculate spherical coordinate wrt Y axis.
template <typename Vector>
UT_INLINE QUOTIENT_DATATYPE(Vector, Vector) Theta(const MConstVector<Vector>& v)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == 3);
	return acos(v.template At<1>() / Magnitude(v));
}

/// Calculate spherical coordinate wrt Y axis.
template <typename Vector> 
UT_INLINE QUOTIENT_DATATYPE(Vector, Vector) Phi(const MConstVector<Vector>& v)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == 3);
	return atan2(v.template At<2>(), v.template At<0>());
}

/// Faster than Magnitude() but less accurate.
template <typename Vector>
UT_INLINE typename Vector::DataType SumAbs(const MConstVector<Vector>& v)
{
	return Recurse<Vector::SIZE>::SumAbs(v);
}

/// Return largest component.
template <typename Vector>
UT_INLINE typename Vector::DataType Largest(const MConstVector<Vector>& v)
{
	return Recurse<Vector::SIZE>::Largest(v);
}

/// Return smallest component.
template <typename Vector>
UT_INLINE typename Vector::DataType Smallest(const MConstVector<Vector>& v)
{
	return Recurse<Vector::SIZE>::Smallest(v);
}

/// Return normalised vector.
template <typename Vector>
UT_INLINE MNormal<typename VectorDivideType<Vector, typename Vector::DataType>::Result> Normalised(const MVector<Vector>& v)
{
	return MNormal<typename VectorDivideType<Vector, typename Vector::DataType>::Result>::Divide(v, Magnitude(v));
}

/// Find a point between two other points.
template <typename Point1, typename Point2>
UT_INLINE MPoint<BasicVector<MStorageVector<typename Point1::DataType, Point1::SIZE> > >
	Between(const MPoint<Point1>& p1, const MPoint<Point2>& p2, QUOTIENT_DATATYPE(Point1, Point2) f1, QUOTIENT_DATATYPE(Point1, Point2) f2)
{
	BIZ_STATIC_ASSERT(Point1::SIZE == Point1::SIZE);
	BIZ_STATIC_ASSERT((Btl::SameType<typename Point1::DataType, typename Point2::DataType>::result));

	MPoint<BasicVector<MStorageVector<typename Point1::DataType, Point1::SIZE> > > p;
	QUOTIENT_DATATYPE(Point1, Point2) r = f2 / (f1 + f2);
	Blend(&p, p1, p2, r);
	return p;
}

/// Reflect in a plane.
template <typename Point1, typename Point2, typename Normal>
UT_INLINE MPoint<BasicVector<MStorageVector<typename Point1::DataType, Point1::SIZE> > > 
	Reflected(const MPoint<Point1>& p, const MPoint<Point1>& planePosition, const MNormal<Normal>& planeNormal)
{
	return p - planeNormal * (((p - planePosition) | planeNormal) * 2);
}

template <typename Point>
UT_INLINE MPoint<BasicVector<MStorageVector<typename Point::DataType, Point::SIZE> > > 
	Scaled(const MPoint<Point>& point, QUOTIENT_DATATYPE(Point, Point) f)
{
	return MakePoint(MakeVector(point) * f);
}

/// Rotate around normal n.
template <typename Vector1, typename Vector2, typename Scalar>
UT_INLINE typename UT_SCR(typename ProcessedVector<Vector1>::Result) Rotated(const MVector<Vector1>& v, const MNormal<Vector2>& n, const Scalar& angle)
{
	Scalar cos, sin;
	Maths::SinCos(&sin, &cos, -angle);
	typename ProcessedVector<Vector1>::Result vp(v);
	return vp * cos + ((n | vp) * (Maths::Constants<Scalar>::One() - cos)) * n + (vp ^ n) * sin;
}

/// Rotate around normal n.
template <typename Vector1, typename Vector2, typename Scalar>
UT_INLINE MNormal<Vector1> Rotated(const MNormal<Vector1>& v, const MNormal<Vector2>& n, const Scalar& angle)
{
	Scalar cos, sin;
	Maths::SinCos(&sin, &cos, -angle);
	return MakeNormal(v * cos + ((n | v) * (Maths::Constants<Scalar>::One() - cos)) * n + (v ^ n) * sin);
}

template <typename Vector, int N>
UT_INLINE MVector<BasicVector<MStorageVector<typename Vector::DataType, N> > > GetComponents(const MConstVector<Vector>& p, Ut::StaticInteger<N> dummy)
{
	MVector<BasicVector<MStorageVector<typename Vector::DataType, N> > > newVector;
	Recurse<N>::Set(newVector, p);
	return newVector;
}

// Intersect two line segments
template <typename Point2>
inline bool Intersect(	const MPoint<Point2>& vec1, 
						const MPoint<Point2>& vec2,
						const MPoint<Point2>& Bvec1,
						const MPoint<Point2>& Bvec2)
{
	typedef typename Point2::DataType T;

	// (Bx-Ax)(Dy-Cy)-(By-Ay)(Dx-Cx)
	T q = (vec2.x-vec1.x) * (Bvec2.y-Bvec1.y) - (vec2.y-vec1.y) * (Bvec2.x-Bvec1.x);

	if (q == 0) return false; // parallel line segments

	// ( (Ay-Cy)(Dx-Cx)-(Ax-Cx)(Dy-Cy) ) / q
	T r = ( (vec1.y-Bvec1.y) * (Bvec2.x-Bvec1.x) - (vec1.x-Bvec1.x) * (Bvec2.y-Bvec1.y) ) / q;

	if (r < 0.0f || r > 1.0f) return false;

	// ( (Ay-Cy)(Bx-Ax)-(Ax-Cx)(By-Ay) ) / q
	T s = ( (vec1.y-Bvec1.y) * (vec2.x-vec1.x) - (vec1.x-Bvec1.x) * (vec2.y-vec1.y) ) / q;

	if (s < 0.0f || s > 1.0f) return false;
	return true;
}

template <typename Vector1, typename Vector2>
UT_INLINE bool operator == (const MVector<Vector1>& v1, const MConstVector<Vector2>& v2)
{
	BIZ_STATIC_ASSERT(Vector1::SIZE == Vector2::SIZE);
	return Recurse<Vector1::SIZE>::Equals(v1, v2);
}

template <typename Vector1, typename Vector2>
UT_INLINE bool operator != (const MVector<Vector1>& v1, const MConstVector<Vector2>& v2)
{
	BIZ_STATIC_ASSERT(Vector1::SIZE == Vector2::SIZE);
	return !Recurse<Vector1::SIZE>::Equals(v1, v2);
}

template <typename Point1, typename Point2>
UT_INLINE bool operator == (const MPoint<Point1>& p1, const MPoint<Point2>& p2)
{
	BIZ_STATIC_ASSERT(Point1::SIZE == Point2::SIZE);
	return Recurse<Point1::SIZE>::Equals(p1, p2);
}

template <typename Point1, typename Point2>
UT_INLINE bool operator != (const MPoint<Point1>& p1, const MPoint<Point2>& p2)
{
	BIZ_STATIC_ASSERT(Point1::SIZE == Point2::SIZE);
	return !Recurse<Point1::SIZE>::Equals(p1, p2);
}

template <typename Vector1, typename Vector2>
UT_INLINE MVector<typename TemporaryVector<Vector1, PRODUCT_DATATYPE(Vector1, Vector2), Vector1::SIZE>::Result>
ComponentMultiply(const MConstVector<Vector1>& v1, const MConstVector<Vector2>& v2)
{
	BIZ_STATIC_ASSERT(Vector1::SIZE == Vector2::SIZE);
	MVector<BasicVector<MStorageVector<PRODUCT_DATATYPE(Vector1, Vector2), Vector1::SIZE> > > result;
	Recurse<Vector1::SIZE>::ComponentMultiply(result, v1, v2);
	return result;
}

template <typename Vector>
UT_INLINE TEMP_VECTOR_TYPE
Abs(const MConstVector<Vector>& v)
{
	TEMP_VECTOR_TYPE result;
	Recurse<Vector::SIZE>::Abs(result, v);
	return result;
}

template <typename Point, typename Scalar>
__forceinline MPoint<Point> Lerp(const MPoint<Point>& p1, const MPoint<Point>& p2, const Scalar& s)
{
	return p1 + (p2 - p1) * s;
}

template <typename Vector, typename Scalar>
__forceinline MVector<Vector> Lerp(const MVector<Vector>& v1, const MVector<Vector>& v2, const Scalar& s)
{
	return v1 + (v2 - v1) * s;
}

///////////////////////////////////////////////////////////
// Modification functions

/// Set three components of a vector
template <typename Vector>
UT_INLINE void Set(MVector<Vector>* v, typename Vector::DataType ix, 
									typename Vector::DataType iy, 
									typename Vector::DataType iz)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == 3);
	v->template At<0>() = ix; v->template At<1>() = iy; v->template At<2>() = iz;
}

/// Set three components of a point.
template <typename Point>
UT_INLINE void Set(MPoint<Point>* p,	typename Point::DataType ix, 
									typename Point::DataType iy, 
									typename Point::DataType iz)
{
	BIZ_STATIC_ASSERT(Point::SIZE == 3);
	p->template At<0>() = ix; p->template At<1>() = iy; p->template At<2>() = iz;
}

/// Set four components of a vector
template <typename Vector>
UT_INLINE void Set(MVector<Vector>* v,	typename Vector::DataType ix, 
										typename Vector::DataType iy, 
										typename Vector::DataType iz,
										typename Vector::DataType iw)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == 4);
	v->template At<0>() = ix; v->template At<1>() = iy; v->template At<2>() = iz; v->template At<3>() = iw;
}

/// Set four components of a point.
template <typename Point>
UT_INLINE void Set(MPoint<Point>* p,	typename Point::DataType ix, 
										typename Point::DataType iy, 
										typename Point::DataType iz,
										typename Point::DataType iw)
{
	BIZ_STATIC_ASSERT(Point::SIZE == 4);
	p->template At<0>() = ix; p->template At<1>() = iy; p->template At<2>() = iz; p->template At<3>() = iw;
}

/// Set two components of a vector
template <typename Vector>
UT_INLINE void Set(MVector<Vector>* v, typename Vector::DataType ix, 
									typename Vector::DataType iy)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == 2);
	v->template At<0>() = ix; v->template At<1>() = iy;
}

/// Set two components of a point.
template <typename Point>
UT_INLINE void Set(MPoint<Point>* p,	typename Point::DataType ix, 
									typename Point::DataType iy)
{
	BIZ_STATIC_ASSERT(Point::SIZE == 2);
	p->template At<0>() = ix; p->template At<1>() = iy;;
}

// Set four component vector from three components and scalar
template <typename Vector>
UT_INLINE void Set(MVector<Vector>* v, MConstVector<typename Vector::VectorXYZ> xyz, typename Vector::DataType w)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == 4);
	v->template At<0>() = xyz.template At<0>();
	v->template At<1>() = xyz.template At<1>();
	v->template At<2>() = xyz.template At<2>();
	v->template At<3>() = w;
}

/// Set all components to zero.
template <typename Vector>
UT_INLINE void Clear(MVector<Vector>* v)
{
	Recurse<Vector::SIZE>::ClearVector(*v);
}

/// Set all components to zero.
template <typename Vector>
UT_INLINE void Clear(MPoint<Vector>* v)
{
	Recurse<Vector::SIZE>::ClearVector(*v);
}

/// Set all components to the maximum.
template <typename Vector>
UT_INLINE void SetMax(MPoint<Vector>* v)
{
	for (int i = 0; i < Vector::SIZE; i++)
		(*v)[i] = Maths::Constants<typename Vector::DataType>::Max();
}

/// Set all components to minimum.
template <typename Vector>
UT_INLINE void SetMin(MPoint<Vector>* v)
{
	for (int i = 0; i < Vector::SIZE; i++)
		(*v)[i] = Maths::Constants<typename Vector::DataType>::Min();
}

/// Change sign.
template <typename Vector>
UT_INLINE void Negate(MVector<Vector>* v)
{
	Recurse<Vector::SIZE>::Negate(*v);
}

/// Change sign.
template <typename Normal>
UT_INLINE void Negate(MNormal<Normal>* v)
{
	Recurse<Normal::SIZE>::Negate(*v);
}

/// Normalise to size x.
template <typename Vector>
UT_INLINE void Normalise(MVector<Vector>* v, typename Vector::DataType x)
{
	*v /= Magnitude(v) / x;
}

/// Normalise to size 1.
template <typename Vector>
UT_INLINE void Normalise(MVector<Vector>* v)
{
	*v /= Magnitude(*v);
}

template <typename Vector>
UT_INLINE void SafeNormalise(MVector<Vector>* v)
{
	float m = Magnitude(*v);
	if (m == 0)
	{
		Clear(v);
	}
	else
		*v /= m;
}

template <typename Vector1, typename Vector2>
UT_INLINE void Reflect(MVector<Vector1>* v, const MNormal<Vector2>& d)
{
	v -= d * (v | d) * 2;
}

/// Rotate around normal n.
template <typename Vector1, typename Vector2, typename Scalar>
UT_INLINE void Rotate(MVector<Vector1>* v, const MNormal<Vector2>& n, const Scalar& angle)
{
	Scalar cos, sin;
	Maths::SinCos(&sin, &cos, -angle);
	*v = *v * cos + ((n | *v) * (Maths::Constants<Scalar>::One() - cos)) * n + (*v ^ n) * sin;
}

/// Rotate around normal n.
template <typename Vector1, typename Vector2, typename Scalar>
UT_INLINE void Rotate(MNormal<Vector1>* v, const MNormal<Vector2>& n, const Scalar& angle)
{
	Scalar cos, sin;
	Maths::SinCos(&sin, &cos, -angle);
	MNormal<Vector1> t = MakeNormal(*v * cos + ((n | *v) * (Maths::Constants<Scalar>::One() - cos)) * n + (*v ^ n) * sin);
	*v = t;
}

/// Find a point between two other points.
template <typename Point, typename Point1, typename Point2>
UT_INLINE void Blend(MPoint<Point>* p, const MPoint<Point1>& p1, const MPoint<Point2>& p2, QUOTIENT_DATATYPE(Point, Point) f)
{
	(MVector<Point>&) *p = (MVector<Point1>&) p1 * f + (MVector<Point2>&) p2 * (Maths::Constants<QUOTIENT_DATATYPE(Point, Point)>::One() - f);
}

// update 3D minimum and maximum extents using input position
template <typename Point1, typename Point2>
UT_INLINE void UpdateBounds(MPoint<Point1>* min, MPoint<Point1>* max, const MPoint<Point2>& p)
{
	assert(_finite(p.template At<0>()));
	assert(_finite(p.template At<1>()));
	assert(_finite(p.template At<2>()));

	BIZ_STATIC_ASSERT(Point1::SIZE == 3 && Point2::SIZE == 3);
	min->template At<0>() = Maths::Min(min->template At<0>(), p.template At<0>());
	min->template At<1>() = Maths::Min(min->template At<1>(), p.template At<1>());
	min->template At<2>() = Maths::Min(min->template At<2>(), p.template At<2>());
	max->template At<0>() = Maths::Max(max->template At<0>(), p.template At<0>());
	max->template At<1>() = Maths::Max(max->template At<1>(), p.template At<1>());
	max->template At<2>() = Maths::Max(max->template At<2>(), p.template At<2>());
}

template <typename Point1, typename Point2, typename Point3>
UT_INLINE void SetMin(MPoint<Point1>* out, const MPoint<Point2>& p1, const MPoint<Point3>& p2)
{
	BIZ_STATIC_ASSERT(Point1::SIZE == 3 && Point2::SIZE == 3 && Point3::SIZE == 3);
	out->template At<0>() = Maths::Min(p1.template At<0>(), p2.template At<0>());
	out->template At<1>() = Maths::Min(p1.template At<1>(), p2.template At<1>());
	out->template At<2>() = Maths::Min(p1.template At<2>(), p2.template At<2>());
}

template <typename Point1, typename Point2, typename Point3>
UT_INLINE void SetMax(MPoint<Point1>* out, const MPoint<Point2>& p1, const MPoint<Point3>& p2)
{
	BIZ_STATIC_ASSERT(Point1::SIZE == 3 && Point2::SIZE == 3 && Point3::SIZE == 3);
	out->template At<0>() = Maths::Max(p1.template At<0>(), p2.template At<0>());
	out->template At<1>() = Maths::Max(p1.template At<1>(), p2.template At<1>());
	out->template At<2>() = Maths::Max(p1.template At<2>(), p2.template At<2>());
}

/// Scale a point.
template <typename Point>
UT_INLINE void Scale(MPoint<Point>* p, QUOTIENT_DATATYPE(Point, Point) f)
{
	MakeVector(*p) *= f;
}

// Get a normal from three clockwise vertices of a triangle
template <typename Normal, typename Point>
void GetNormal(MNormal<Normal>* n, const MPoint<Point> v0, const MPoint<Point> v1, const MPoint<Point> v2)
{
	n->x = n->y = n->z = 0.f;

	n->x += (v0.y - v1.y) * (v1.z + v0.z);
	n->y += (v0.z - v1.z) * (v1.x + v0.x);
	n->z += (v0.x - v1.x) * (v1.y + v0.y);

	n->x += (v1.y - v2.y) * (v2.z + v1.z);
	n->y += (v1.z - v2.z) * (v2.x + v1.x);
	n->z += (v1.x - v2.x) * (v2.y + v1.y);

	n->x += (v2.y - v0.y) * (v0.z + v2.z);
	n->y += (v2.z - v0.z) * (v0.x + v2.x);
	n->z += (v2.x - v0.x) * (v0.y + v2.y);

	MakeVector(*n) /= Magnitude(MakeVector(*n));  
}

template <typename Vector>
UT_INLINE MVector<BasicVector<MStorageVector<typename Vector::DataType, 4> > > MakeHomogeneous(const MPoint<Vector>& p)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == 3);
	return MVector<BasicVector<MStorageVector<typename Vector::DataType, 4> > >(p.x, p.y, p.z, 1.f);
}

template <typename Vector>
UT_INLINE MVector<BasicVector<MStorageVector<typename Vector::DataType, 4> > > MakeHomogeneous(const MVector<Vector>& v)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == 3);
	return MVector<BasicVector<MStorageVector<typename Vector::DataType, 4> > >(v.x, v.y, v.z, 0.f);
}

template <typename Point>
UT_INLINE TEMP_POINT_TYPE Min(const MPoint<Point>& p1, const MPoint<Point>& p2)
{
	TEMP_POINT_TYPE p;
	SetMin(&p, p1, p2);
	return p;
}

template <typename Point>
UT_INLINE TEMP_POINT_TYPE Max(const MPoint<Point>& p1, const MPoint<Point>& p2)
{
	TEMP_POINT_TYPE p;
	SetMax(&p, p1, p2);
	return p;
}

////////////////////////////////////////////////////////////////

// some typedefs for convenience

typedef MVector<VectorXY<float> > MVector2F;
typedef MVector<VectorXYZ<float> > MVector3F;
typedef MVector<VectorXYZW<float> > MVector4F;

typedef MPoint<VectorXYZ<float> > MPoint3F;
typedef MPoint<VectorXYZW<float> > MPoint4F;

typedef MNormal<VectorXYZ<float> > MNormal3F;

} // End Ut

#endif // __MATHS_H__



