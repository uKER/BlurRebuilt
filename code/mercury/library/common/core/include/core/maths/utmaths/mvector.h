////////////////////////////////////////////////////////////////
// MVector classes

#ifndef __GNUC__
#pragma once
#endif
#ifndef __MVECTOR_H__
#define __MVECTOR_H__

#include <core/utility/utility.h>
#include <core/maths/utmaths/operator.h>
#include <core/maths/utmaths/recurse.h>

#ifndef FORCE_INLINE
#define FORCE_INLINE __forceinline
#endif

/// Namespace for the vector classes.
///
/// MVector is a general-purpose vector template which makes it easy 
/// to create vector classes with overloaded arithmetic operators.
/// The operators are designed to avoid creating unnecessary temporary vectors.
/// Different MVector types can be used together in expressions so long as their elements can be.
/// It's also easy to convert between different types. @n@n
///
/// There is also a MConstVector template which makes all the modifying operators protected,
/// and an MPoint template which has only the operations that make sense for points in space.
/// (e.g. A point plus a vector gives another point, and the difference between two points is a vector.) @n@n
///
/// Any of the operators can be specialised for user-defined vector types. This makes it possible to 
/// use hardware vectors within this system. There are also classes for providing access elements 
/// when it is not possible to reference them directly. @n@n
///
/// Notes on writing MVector utility functions: @n
///
/// Use MConstVector for an input vector that is not modified, and pass by const reference.
/// MVector is derived from MConstVector so the conversion will be automatic.
/// Pass a pointer instead of a reference if the parameter is to be modified.
/// Use the recursion class rather than looping over elements. This will unroll loops.
/// A normalised vector can be represented using MNormal. Use this as a parameter
/// to require a normalised input vector, or as a return type if the returned vector is
/// guaranteed to be normalised. MPoint can be used to represent a position in space.
///
/// There are macros and templates available to find return types, e.g. PRODUCT_TYPE(Vector1, Vector2) is 
/// the scalar return type from multiplying elements of Vector1 and Vector2.
/// The Vector::DataType typedef can be used to find the type stored in a vector, and Vector::SIZE is 
/// its length.
/// The BIZ_STATIC_ASSERT() macro can be used to verify constants and types at compile-time.
/// The ProductType, QuotientType and LinearType templates can be used to find return types from scalars.
/// In general it's best to avoid returning temporary vectors, but if necessary the MStorageVector
/// class can be used for this purpose.
/// Writing functions in terms of the basic vector operations is a good idea; if those operations are 
/// specialised the function will be able to use the specialised versions.
namespace UtMaths
{

template <typename Vector> class MVector;
template <typename Vector> class GenericVector;
template<class Description> class BasicVector;
template <typename T, int N> class MStorageVector;

/// %MConstVector is a version of MVector that does not pass on modifying functions.
template <typename Vector>
class MConstVector : public Vector
{
public:
	typedef typename Vector::DataType DataType; ///< @a
	typedef Vector VectorType; ///< @a
	const static int SIZE = Vector::SIZE; ///< @a

	/// Default constructor does nothing.
	UT_INLINE MConstVector() {}
	//UT_INLINE MConstVector(typename Vector::type1 p) : Vector(p) {}

	UT_INLINE MConstVector(const MConstVector& other) : Vector((Vector&)other) {}

	/// Copy constructor.
	template <typename Evaluator>
	UT_INLINE MConstVector(const MConstVector<Evaluator>& e) : Vector(e) {}

	// Other constructors are protected.
protected:
	UT_INLINE MConstVector(DataType x, DataType y) : Vector(x, y) {} ///< @a
	UT_INLINE MConstVector(DataType x, DataType y, DataType z) : Vector(x, y, z) {} ///< @a
	UT_INLINE MConstVector(DataType x, DataType y, DataType z, DataType w) : Vector(x, y, z, w) {} ///< @a
};

/// %MVector is the main vector type. It does nothing but pass the = operators and constructors to the base class.
/// It exists for template matching purposes.
template <typename Vector>
class MVector SEALED : public MConstVector<Vector>
{
public:
	typedef typename Vector::DataType DataType; ///< @a
	typedef Vector VectorType; ///< @a

	/// Default constructor does nothing.
	UT_INLINE MVector() {}
	//UT_INLINE MVector(typename Vector::type1 p) : MConstVector<Vector>(p) {}

	UT_INLINE MVector(const MVector& other) : MConstVector<Vector>((MConstVector<Vector>&)other) {}
	UT_INLINE explicit MVector(const MConstVector<Vector>& other) : MConstVector<Vector>((MConstVector<Vector>&)other) {}

	// Constructors for different sized vectors.
	UT_INLINE MVector(DataType x, DataType y) : MConstVector<Vector>(x, y) {} ///< @a
	UT_INLINE MVector(DataType x, DataType y, DataType z) : MConstVector<Vector>(x, y, z) {} ///< @a
	UT_INLINE MVector(DataType x, DataType y, DataType z, DataType w) : MConstVector<Vector>(x, y, z, w) {} ///< @a

	/// Copy constructor.
	template<typename Evaluator>
	UT_INLINE MVector(const MVector<Evaluator>& e) : MConstVector<Vector>(e) {}

	/// Note: These templates do not stop the compiler generating a default copy constructor and = operator.
	/// This is generally a good thing because it may be quicker to do a member-wise copy than go through the access functions.
	/// If a vector needs different copy semantics to the default it should have its own functions.

	// modification
	template<typename Evaluator>
	UT_INLINE MVector& operator = (const MConstVector<Evaluator>& e) {Vector::operator =(e); return *this;} ///< @a

	template<typename Evaluator>
	UT_INLINE MVector& operator += (const MConstVector<Evaluator>& e) {Vector::operator +=(e); return *this;} ///< @a

	template<typename  Evaluator>
	UT_INLINE MVector& operator -= (const MConstVector<Evaluator>& e) {Vector::operator -=(e); return *this;} ///< @a

	template<typename Scalar>
	UT_INLINE MVector& operator *= (const Scalar& e) {Vector::operator *=(e); return *this;} ///< @a

	template<typename Scalar>
	UT_INLINE MVector& operator /= (const Scalar& e) {Vector::operator /=(e); return *this;} ///< @a
};

/// A class with only the operations that make sense for points in space.
template <typename Vector>
class MPoint SEALED : public Vector
{
public:
	typedef typename Vector::DataType DataType; ///< @a
	typedef Vector VectorType; ///< @a

	/// Default constructor does nothing.
	MPoint() {}
	//MPoint(typename Vector::type1 p) : Vector(p) {}

	UT_INLINE MPoint(const MPoint& other) : Vector((Vector&)other) {}

	// Constructors for different sized vectors.
	MPoint(DataType x, DataType y) : Vector(x, y) {} ///< @a
	MPoint(DataType x, DataType y, DataType z) : Vector(x, y, z) {} ///< @a
	MPoint(DataType x, DataType y, DataType z, DataType w) : Vector(x, y, z, w) {} ///< @a

	/// Copy constructor from MPoint.
	template<typename Evaluator>
	MPoint(const MPoint<Evaluator>& e) : Vector(e) {}

	// modification
	template<typename Evaluator>
	UT_INLINE MPoint& operator = (const MPoint<Evaluator>& e) {Vector::operator =(e); return *this;} ///< @a

	template<typename Evaluator>
	UT_INLINE MPoint& operator = (const GenericVector<Evaluator>& e) {Vector::operator =(e); return *this;} ///< @a

	template<typename Evaluator>
	UT_INLINE MPoint& operator += (const MConstVector<Evaluator>& e) {Vector::operator +=(e); return *this;} ///< @a

	template<typename  Evaluator>
	UT_INLINE MPoint& operator -= (const MConstVector<Evaluator>& e) {Vector::operator -=(e); return *this;} ///< @a
};

/// Normalised vector class.
template <typename Vector>
class MNormal SEALED : public MConstVector<Vector>
{
public:
	typedef typename Vector::DataType DataType; ///< @a
	typedef Vector VectorType; ///< @a

	/// Default constructor does nothing.
	MNormal() {}
	//MNormal(typename Vector::type1 p) : MConstVector<Vector>(p) {}

	UT_INLINE MNormal(const MNormal& other) : MConstVector<Vector>((MConstVector<Vector>&)other) {}

	/// Copy constructor from MNormal.
	template<typename Evaluator>
	MNormal(const MNormal<Evaluator>& e) : MConstVector<Vector>(e) {}

	/// Constructor from MVector.
	//template<typename Evaluator>
	//DEPRECATED UT_INLINE explicit MNormal(const MConstVector<Evaluator>& e) : MConstVector<Vector>(e / Magnitude(e)) {}

	enum _UNIT_LENGTH_VECTOR {UNIT_LENGTH_VECTOR};

	template<typename Evaluator>
	UT_INLINE MNormal(const MConstVector<Evaluator>& e, const _UNIT_LENGTH_VECTOR unit_length_vector) : MConstVector<Vector>(e) {}

	template<typename Evaluator>
	UT_INLINE MNormal& operator = (const MNormal<Evaluator>& e) {Vector::operator =(e); return *this;} ///< @a

	FRIEND_CLASS Maths::Constants<MNormal>;

private:
	// These should be used only by functions that return normalised vectors.
	MNormal(DataType x, DataType y) : MConstVector<Vector>(x, y) {} ///< @a
	MNormal(DataType x, DataType y, DataType z) : MConstVector<Vector>(x, y, z) {} ///< @a
	MNormal(DataType x, DataType y, DataType z, DataType w) : MConstVector<Vector>(x, y, z, w) {} ///< @a
};

// A generic vector will convert to any other sort of vector.
template <typename Vector>
class GenericVector : public Vector
{
public:
	typedef typename Vector::DataType DataType;
	typedef Vector VectorType;

	UT_INLINE GenericVector() {}
	UT_INLINE GenericVector(typename Vector::type1 p) : Vector(p) {}
	UT_INLINE GenericVector(typename Vector::type1 p1, typename Vector::type2 p2) : Vector(p1, p2) {}

	UT_INLINE GenericVector(DataType x, DataType y) : Vector(x, y) {} ///< @a
	UT_INLINE GenericVector(DataType x, DataType y, DataType z) : Vector(x, y, z) {} ///< @a
	UT_INLINE GenericVector(DataType x, DataType y, DataType z, DataType w) : Vector(x, y, z, w) {} ///< @a

	UT_INLINE operator MVector<Vector>& () {return *(MVector<Vector>*) this;}
	UT_INLINE operator MConstVector<Vector>& () {return *(MConstVector<Vector>*) this;}
	UT_INLINE operator MPoint<Vector>& () {return *(MPoint<Vector>*) this;}
	UT_INLINE operator MNormal<Vector>& () {return *(MNormal<Vector>*) this;}

	// modification
	template<typename Evaluator>
	UT_INLINE GenericVector& operator = (const MConstVector<Evaluator>& e) {Vector::operator =(e); return *this;} ///< @a
};

// Note: although these templates match anything, they will only work with
// types that have a typedef 'VectorType', which gives some type safety.
template<typename Vector>
UT_INLINE GenericVector<typename Vector::VectorType>& MakeGeneric(Vector& vector)
{
	return (GenericVector<typename Vector::VectorType>&) vector;
}

template<typename Vector>
UT_INLINE MVector<typename Vector::VectorType>& MakeVector(Vector& vector)
{
	return (MVector<typename Vector::VectorType>&) vector;
}

template<typename Vector>
UT_INLINE MPoint<typename Vector::VectorType>& MakePoint(Vector& vector)
{
	return (MPoint<typename Vector::VectorType>&) vector;
}

template<typename Vector>
UT_INLINE MNormal<typename Vector::VectorType>& MakeNormal(Vector& vector)
{
	return (MNormal<typename Vector::VectorType>&) vector;
}

//////////////////////////////////////////////////////////////////////////

/// N component vector which does element-by-element processing.
/// Data held in class Storage.
/// Use with the MVector template, i.e. MVector< BasicVector<...> >.
/// This provides many common functions required for vectors, making it easier to create new vector types.
template<class Description>
class BasicVector : public Description
{
public:
	// information about the vector
	typedef typename Description::DotProductType DotProductType;
	typedef typename Description::DataType DataType;
	const static int SIZE = Description::SIZE;

	// Derived class decides which constructors to expose.
protected:
	// constructors
	BasicVector() {} ///< @a
	BasicVector(DataType x, DataType y) : Description(x, y) {}  ///< @a
	BasicVector(DataType x, DataType y, DataType z) : Description(x, y, z) {}  ///< @a
	BasicVector(DataType x, DataType y, DataType z, DataType w) : Description(x, y, z, w) {}  ///< @a
	
	/// Copy constructor.
	template<typename Evaluator>
	UT_INLINE BasicVector(const Evaluator& e) {operator = (e);}

	// Sets each element of the vector to the result provided by the evaluator.
	// The Evaluator needs to have an At<I>() function which returns a suitable type.
	template<typename Evaluator>
	UT_INLINE void operator = (const Evaluator& e) {Recurse<SIZE>::Set(*this, e);}  ///< @a

	// A suitable scalar is anything that the elements can be multiplied by.
	template<typename Scalar>
	UT_INLINE void operator *= (const Scalar& e) {Recurse<SIZE>::Multiply(*this, e);}  ///< @a

	template<typename Scalar>
	UT_INLINE void operator /= (const Scalar& e) {Recurse<SIZE>::Divide(*this, e);}  ///< @a

	template<typename Evaluator>
	UT_INLINE void operator += (const Evaluator& e) {Recurse<SIZE>::Add(*this, e);}  ///< @a

	template<typename Evaluator>
	UT_INLINE void operator -= (const Evaluator& e) {Recurse<SIZE>::Subtract(*this, e);}  ///< @a

	// types for constructors
	typedef Ut::NullType type1;
	typedef Ut::NullType type2;
};

/////////////////////////////////////////////////////////////////////////////
// Binary operator.
// Just takes two types, stores references and returns a result when asked.
// The nature of the result depends on the operator template.

template <typename Type, template <typename V> class Operator>
class UnaryOperatorV : public UnaryOperator<Type, Operator>
{
public:
	UnaryOperatorV(Type p) : UnaryOperator<Type, Operator>(p) {}

	// operators
	UT_INLINE static GenericVector<UnaryOperatorV> Negate(Type p1) {return GenericVector<UnaryOperatorV>(p1);};
};

template <typename P1, typename P2, template <typename V1, typename V2> class Operator>
class BinaryOperatorVV : public BinaryOperator<P1, P2, Operator>
{
public:
	BinaryOperatorVV(P1 p1, P2 p2) : BinaryOperator<P1, P2, Operator>(p1, p2) {}

	typedef typename BinaryOperator<P1, P2, Operator>::DataType DotProductType;

	// static functions to return new operators 
	UT_INLINE static GenericVector<BinaryOperatorVV> Sum(P1 p1, P2 p2) {return GenericVector<BinaryOperatorVV>(p1, p2);}
	UT_INLINE static GenericVector<BinaryOperatorVV> Difference(P1 p1, P2 p2) {return GenericVector<BinaryOperatorVV>(p1, p2);}
	UT_INLINE static DotProductType DotProduct(P1 p1, P2 p2) {return Recurse<UT_SCR(P1)::SIZE>::Sum(BinaryOperatorVV(p1, p2));}
};

template <typename P1, typename P2, template <typename V1, typename V2> class Operator>
class BinaryOperatorVS : public BinaryOperator<P1, P2, Operator>
{
public:
	BinaryOperatorVS(P1 p1, P2 p2) : BinaryOperator<P1, P2, Operator>(p1, p2) {}

	// static functions to return new operators
	UT_INLINE static GenericVector<BinaryOperatorVS> Multiply(P1 p1, P2 p2) {return GenericVector<BinaryOperatorVS>(p1, p2);}
	UT_INLINE static GenericVector<BinaryOperatorVS> Divide(P1 p1, P2 p2) {return GenericVector<BinaryOperatorVS>(p1, p2);}
};

/////////////////////////////////////////////////////////////////////////////
// Operator types for vectors. Can be specialised.
// Specialise these rather than the operator functions because
// multiple operator functions may use the same operator type.

/// Gives the type to expect when adding two vectors.
/// The default is a binary operator template.
/// This can be specialised to override the behavior of the + operator.
/// The result type must have a static function Sum(Type1&, Type2&).
template<typename Type1, typename Type2>
struct VectorSumType {typedef BinaryOperatorVV<const Type1&, const Type2&, SumOperator> Result;};

/// Gives the type to expect when subtracting two vectors.
/// The default is a binary operator template.
/// This can be specialised to override the behavior of the - operator.
/// The result type must have a static function Difference(Type1&, Type2&).
template<typename Type1, typename Type2>
struct VectorDifferenceType {typedef BinaryOperatorVV<const Type1&, const Type2&, DifferenceOperator> Result;};

/// Gives the type to expect when multiplying a vector by a scalar.
/// The default is a binary operator template.
/// This can be specialised to override the behavior of the * operator.
/// The result type must have a static function Multiply(Type1&, Type2&).
template<typename Type1, typename Type2>
struct VectorMultiplyType {typedef BinaryOperatorVS<const Type1&, Type2, MultiplyOperator> Result;};

/// Gives the type to expect when dividing a vector by a scalar.
/// The default is a binary operator template.
/// This can be specialised to override the behavior of the / operator.
/// The result type must have a static function Divide(Type1&, Type2&).
template<typename Type1, typename Type2>
struct VectorDivideType {typedef BinaryOperatorVS<const Type1&, Type2, DivideOperator> Result;};

/// Gives the type to expect when negating a vector.
/// The default is a unary operator template.
/// This can be specialised to override the behavior of the - operator.
/// The result type must have a static function Negate(Type1&, Type2&).
template<typename Vector>
struct VectorNegateType {typedef UnaryOperatorV<const Vector&, NegateOperator> Result;};

/// Gives the type to expect from a dot product.
/// The default is a binary operator template.
/// This can be specialised to override the behavior of the | operator.
/// The result type must have a static function DotProduct(Type1&, Type2&).
template<typename Type1, typename Type2>
struct DotProductType {typedef BinaryOperatorVV<const Type1&, const Type2&, DotProductOperator> Result;};

/// Gives the type to expect from a cross product.
/// The default is a binary operator template.
/// This can be specialised to override the behavior of the ^ operator.
/// The result type must have a static function CrossProduct(Type1&, Type2&).
template<typename Type1, typename Type2>
struct CrossProductType {typedef BasicVector<MStorageVector<typename ProductType<typename Type1::DataType, typename Type2::DataType>::Result, 3> > Result;};

//////////////////////////////////////////////////////////////////////////////

// If the vector is an operator, returns a type to evaluate it. Otherwise returns a reference to the vector type.
template<typename Vector>
class ProcessedVector
{
public:
	typedef typename Ut::Select<Ut::IsConvertible<Vector, BaseOperator>::exists,
								MVector<BasicVector<MStorageVector<typename Vector::DataType, Vector::SIZE> > >,
								const MVector<Vector>&>::Result Result;
};

// VectorBase for specialisation
template <typename VectorBase, typename DataType = typename VectorBase::DataType, int SIZE = VectorBase::SIZE>
struct TemporaryVector
{
	typedef BasicVector<MStorageVector<DataType, SIZE> > Result;
};

#define TEMP_POINT_TYPE MPoint<typename TemporaryVector<Point>::Result>
#define TEMP_VECTOR_TYPE MVector<typename TemporaryVector<Vector>::Result>

// Minimal class for storing temporary vectors.
template <typename T, int N>
class MStorageVector
{
public:
	typedef T DotProductType;
	typedef T DataType;
	const static int SIZE = N;
	typedef Ut::NullType type1;

	// constructors
	MStorageVector() {}
	MStorageVector(T px, T py, T pz) {BIZ_STATIC_ASSERT(N == 3); data[0] = px; data[1] = py; data[2] = pz;} 
	MStorageVector(T px, T py, T pz, T pw) {BIZ_STATIC_ASSERT(N == 4); data[0] = px; data[1] = py; data[2] = pz; data[3] = pw;} 

	// access
	template <int I>
	UT_INLINE T& At() {BIZ_STATIC_ASSERT(I < N); return data[I];}

	template <int I>
	UT_INLINE const T& At() const {BIZ_STATIC_ASSERT(I < N); return data[I];}

	UT_INLINE T& operator [] (int i) {assert(i < N); return data[i];}
	UT_INLINE const T& operator [] (int i) const {assert(i < N); return data[i];}

	template<typename Vector1, typename Vector2>
	UT_INLINE static MVector<typename CrossProductType<Vector1, Vector2>::Result> CrossProduct(const MConstVector<Vector1>& p1, const MConstVector<Vector2>& p2)
	{
		typename ProcessedVector<Vector1>::Result v1(MakeVector(p1));
		typename ProcessedVector<Vector2>::Result v2(MakeVector(p2));

		return MVector<typename CrossProductType<Vector1, Vector2>::Result>
		(
			v1.At<1>() * v2.At<2>() - v1.At<2>() * v2.At<1>(),
			-v1.At<0>() * v2.At<2>() + v1.At<2>() * v2.At<0>(),
			v1.At<0>() * v2.At<1>() - v1.At<1>() * v2.At<0>()
		);
	}

private:
	T data[N];
};

/////////////////////////////////////////////////////////////////////////////
// return types for operators

template <typename Vector1, typename Vector2> 
struct LinearType<MVector<Vector1>, MVector<Vector2> >
{typedef MVector<typename VectorSumType<Vector1, Vector2>::Result> Result;};

template <typename Vector, typename Scalar> 
struct ProductType<MVector<Vector>, Scalar>
{typedef MVector<typename VectorMultiplyType<Vector, Scalar>::Result> Result;};

/////////////////////////////////////////////////////////////////////////////
// operator overloading
// We only match MVector types. Otherwise the templates would go berserk and start matching anything in sight.

/// Vector addition
template<typename Vector1, typename Vector2>
UT_INLINE MVector<typename VectorSumType<Vector1, Vector2>::Result> operator + (const MConstVector<Vector1>& p1, const MConstVector<Vector2>& p2)
{
	return MVector<typename VectorSumType<Vector1, Vector2>::Result>::Sum(p1, p2);
}

/// Vector subtraction
template<typename Vector1, typename Vector2>
UT_INLINE MVector<typename VectorDifferenceType<Vector1, Vector2>::Result> operator - (const MConstVector<Vector1>& p1, const MConstVector<Vector2>& p2)
{
	return MVector<typename VectorDifferenceType<Vector1, Vector2>::Result>::Difference(p1, p2);
}

/// Vector * scalar
template<typename Vector, typename Scalar>
UT_INLINE MVector<typename VectorMultiplyType<Vector, Scalar>::Result> operator * (const MConstVector<Vector>& p1, const Scalar& p2) 
{
	return MVector<typename VectorMultiplyType<Vector, Scalar>::Result>::Multiply(p1, p2);
}

/// scalar * Vector
template<typename Vector, typename Scalar>
UT_INLINE MVector<typename VectorMultiplyType<Vector, Scalar>::Result> operator * (const Scalar& p1, const MConstVector<Vector>& p2)
{
	return MVector<typename VectorMultiplyType<Vector, Scalar>::Result>::Multiply(p2, p1);
}

/// Vector / scalar
template<typename Vector, typename Scalar>
UT_INLINE MVector<typename VectorDivideType<Vector, Scalar>::Result> operator / (const MConstVector<Vector>& p1, const Scalar& p2)
{
	return MVector<typename VectorDivideType<Vector, Scalar>::Result>::Divide(p1, p2);
}

/// Dot product
template<typename Vector1, typename Vector2>
UT_INLINE typename DotProductType<Vector1, Vector2>::Result::DotProductType operator | (const MConstVector<Vector1>& p1, const MConstVector<Vector2>& p2)
{
	return DotProductType<Vector1, Vector2>::Result::DotProduct(p1, p2);
}

/// Negate
template <typename Vector>
UT_INLINE MVector<typename VectorNegateType<Vector>::Result> operator - (const MConstVector<Vector>& v)
{
	return MVector<typename VectorNegateType<Vector>::Result>::Negate(v);
}

// Cross product.
template<typename Vector1, typename Vector2>
UT_INLINE MVector<typename CrossProductType<Vector1, Vector2>::Result> operator ^ (const MConstVector<Vector1>& p1, const MConstVector<Vector2>& p2)
{
	return MVector<typename CrossProductType<Vector1, Vector2>::Result>::CrossProduct(p1, p2);
}

///////////////////////////////////////////////////////////////////////////
// MPoint operators

/// Addition.
template<typename Vector1, typename Vector2>
UT_INLINE MPoint<typename VectorSumType<Vector1, Vector2>::Result> operator + (const MConstVector<Vector1>& p1, const MPoint<Vector2>& p2)
{
	return MPoint<typename VectorSumType<Vector1, Vector2>::Result>::Sum(p1, p2);
}

/// Addition.
template<typename Vector1, typename Vector2>
UT_INLINE MPoint<typename VectorSumType<Vector1, Vector2>::Result> operator + (const MPoint<Vector1>& p1, const MConstVector<Vector2>& p2)
{
	return MPoint<typename VectorSumType<Vector1, Vector2>::Result>::Sum(p1, p2);
}

// Subtraction.
template<typename Vector1, typename Vector2>
UT_INLINE MPoint<typename VectorDifferenceType<Vector1, Vector2>::Result> operator - (const MPoint<Vector1>& p1, const MConstVector<Vector2>& p2)
{
	return MPoint<typename VectorDifferenceType<Vector1, Vector2>::Result>::Difference(p1, p2);
}

/// Subtraction.
template<typename Vector1, typename Vector2>
UT_INLINE MVector<typename VectorDifferenceType<Vector1, Vector2>::Result> operator - (const MPoint<Vector1>& p1, const MPoint<Vector2>& p2)
{
	return MVector<typename VectorDifferenceType<Vector1, Vector2>::Result>::Difference(p1, p2);
}

/////////////////////////////////////////////////////////////////////////////
// MNormal operators

/// Negate
template <typename Vector>
UT_INLINE MNormal<typename VectorNegateType<Vector>::Result> operator - (const MNormal<Vector>& v)
{
	return MNormal<typename VectorNegateType<Vector>::Result>::Negate(v);
}

///////////////////////////////////////////////////////////////////////////
// Accessor

/// Const version of Accessor.
template <typename Vector, int I>
class ConstAccessor
{
public:
	typedef typename Vector::DataType DataType; ///< @a

	ConstAccessor(const Vector& v) : vector(const_cast<Vector&>(v)) {} ///< @a

	operator DataType () {return vector.template Get<I>();} ///< @a
	typename Vector::RawDataType GetRaw() {return vector.template GetRaw<I>();} ///< @a

protected:
	Vector& vector;
};

/// Encapsulates access to vector elements.
/// An accessor provides a wrapper to act as a reference to an element of a vector,
/// when a real reference cannot exist because the elements are not stored
/// directly as their working type.
/// @param Vector The vector type.
/// @param I The index of the element to be accessed.

template <typename Vector, int I>
class Accessor : public ConstAccessor<Vector, I>
{
public:
	typedef typename Vector::DataType DataType; ///< @a
	typedef ConstAccessor<Vector, I> Base;

	Accessor(Vector& v) : ConstAccessor<Vector, I>(v) {} ///< @a

	UT_INLINE Accessor& operator = (const DataType& e) {Base::vector.Set<I>(e); return *this;} ///< @a
	UT_INLINE Accessor& operator += (const DataType& e) {Base::vector.Add<I>(e); return *this;} ///< @a
	UT_INLINE Accessor& operator -= (const DataType& e) {Base::vector.Subtract<I>(e); return *this;} ///< @a
	UT_INLINE Accessor& operator *= (const DataType& e) {Base::vector.Multiply<I>(e); return *this;} ///< @a
	UT_INLINE Accessor& operator /= (const DataType& e) {Base::vector.Divide<I>(e); return *this;} ///< @a

	UT_INLINE Accessor& operator += (const ConstAccessor<Vector, I>& e) {Base::vector.Add<I>(e); return *this;} ///< @a
	UT_INLINE Accessor& operator -= (const ConstAccessor<Vector, I>& e) {Base::vector.Subtract<I>(e); return *this;} ///< @a
};

} // End Ut

///////////////////////////////////////////////////////////////////
// constants

namespace Maths
{

// Note: These can be specialised further for different vector types
// - so any changes here may need to be implemented for those types as well.

template<typename Vector> struct Constants<UtMaths::MVector<Vector> >
{
	typedef UtMaths::MVector<Vector> VectorType;

	static const VectorType Zero() {VectorType v; Clear(&v); return v;}
	static const VectorType Null() {VectorType v; Clear(&v); return v;}
	static const VectorType X() {return VectorType(Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::Zero());}
	static const VectorType Y() {return VectorType(Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero());}
	static const VectorType Z() {return VectorType(Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::One());}
	static const VectorType NegativeX() {return VectorType(-Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::Zero());}
	static const VectorType NegativeY() {return VectorType(Constants<typename Vector::DataType>::Zero(), -Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero());}
	static const VectorType NegativeZ() {return VectorType(Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::Zero(), -Constants<typename Vector::DataType>::One());}
	static const VectorType XY() {return VectorType(Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero());}
	static const VectorType XZ() {return VectorType(Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::One());}
	static const VectorType YZ() {return VectorType(Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::One());}
	static const VectorType XYZ() {return VectorType(Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::One());}
};

template<typename Vector> struct Constants<UtMaths::MNormal<Vector> >
{
	typedef UtMaths::MNormal<Vector> NormalType;

	static const NormalType X() {return NormalType(Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::Zero());}
	static const NormalType Y() {return NormalType(Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero());}
	static const NormalType Z() {return NormalType(Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::One());}
	static const NormalType NegativeX() {return NormalType(-Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::Zero());}
	static const NormalType NegativeY() {return NormalType(Constants<typename Vector::DataType>::Zero(), -Constants<typename Vector::DataType>::One(), Constants<typename Vector::DataType>::Zero());}
	static const NormalType NegativeZ() {return NormalType(Constants<typename Vector::DataType>::Zero(), Constants<typename Vector::DataType>::Zero(), -Constants<typename Vector::DataType>::One());}
};

template<typename Vector> struct Constants<UtMaths::MPoint<Vector> >
{
	typedef UtMaths::MPoint<Vector> PointType;

	static const PointType Origin() {PointType v; Clear(&v); return v;}
	static const PointType Min() {PointType v; SetMin(&v); return v;}
	static const PointType Max() {PointType v; SetMax(&v); return v;}
	static const PointType NegativeMax() {PointType v; SetMax(&v); UtMaths::MVector<Vector> m = -MakeVector(v); return MakePoint(m);}
};

}

#endif
