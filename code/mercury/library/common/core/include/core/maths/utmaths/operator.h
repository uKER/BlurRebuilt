#ifndef __GNUC__
#pragma once
#endif

#ifndef __OPERATOR_H__
#define __OPERATOR_H__

namespace UtMaths
{

////////////////////////////////////////////////////////////////////////////
// We need to know what type to return when combining other types.

/// A template that gives the type to expect when multiplying two scalars.
/// It can specialised for user-defined types.
template <typename T1, typename T2>
struct ProductType {};

/// A template that gives the type to expect when dividing two scalars.
/// It can specialised for user-defined types.
template <typename T1, typename T2>
struct QuotientType {};

/// A template that gives the type to expect when adding or subtracting two scalars.
/// It can specialised for user-defined types.
template <typename T1, typename T2>
struct LinearType {};

// Specialisations for built in types
template <> struct ProductType<float, float> {typedef float Result;};
template <> struct QuotientType<float, float> {typedef float Result;};
template <> struct LinearType<float, float> {typedef float Result;};

template <> struct ProductType<int, int> {typedef int Result;};
template <> struct QuotientType<int, int> {typedef int Result;};
template <> struct LinearType<int, int> {typedef int Result;};

template <> struct ProductType<double, double> {typedef double Result;};
template <> struct QuotientType<double, double> {typedef double Result;};
template <> struct LinearType<double, double> {typedef double Result;};

template <> struct ProductType<int, float> {typedef float Result;};
template <> struct QuotientType<int, float> {typedef float Result;};
template <> struct LinearType<int, float> {typedef float Result;};
template <> struct ProductType<float, int> {typedef float Result;};
template <> struct QuotientType<float, int> {typedef float Result;};
template <> struct LinearType<float, int> {typedef float Result;};

template <> struct ProductType<int, double> {typedef double Result;};
template <> struct QuotientType<int, double> {typedef double Result;};
template <> struct LinearType<int, double> {typedef double Result;};
template <> struct ProductType<double, int> {typedef double Result;};
template <> struct QuotientType<double, int> {typedef double Result;};
template <> struct LinearType<double, int> {typedef double Result;};

template <> struct ProductType<float, double> {typedef double Result;};
template <> struct QuotientType<float, double> {typedef double Result;};
template <> struct LinearType<float, double> {typedef double Result;};
template <> struct ProductType<double, float> {typedef double Result;};
template <> struct QuotientType<double, float> {typedef double Result;};
template <> struct LinearType<double, float> {typedef double Result;};

//////////////////////////////////////////////////////////////////////////
// Some #defines to make these types easier to write for vectors

#define PRODUCT_DATATYPE(Vector1, Vector2) typename ProductType<typename Vector1::DataType, typename Vector2::DataType>::Result ///<@
#define QUOTIENT_DATATYPE(Vector1, Vector2) typename QuotientType<typename Vector1::DataType, typename Vector2::DataType>::Result ///<@
#define LINEAR_DATATYPE(Vector1, Vector2) typename LinearType<typename Vector1::DataType, typename Vector2::DataType>::Result ///<@

// get the type referred to
#define UT_SCR(X) Ut::StripConst< typename Ut::StripReference< X >::Result >::Result

/////////////////////////////////////////////////////////////////////////////
// Operators.
// These do the real work and are responsible for checking that types and sizes are correct.

// This is just so we know something is an operator
class BaseOperator
{
};

// Something to derive from.
template <typename Vector1, typename Vector2>
class LinearOperator : public BaseOperator
{
public:
	typedef typename LinearType<typename UT_SCR(Vector1)::DataType, typename UT_SCR(Vector2)::DataType>::Result DataType;
	const static int SIZE = UT_SCR(Vector1)::SIZE;

private:
	BIZ_STATIC_ASSERT2(UT_SCR(Vector1)::SIZE == UT_SCR(Vector2)::SIZE, MustBeSameSize);
};

// Addition
template <typename Vector1, typename Vector2>
class SumOperator : public LinearOperator<Vector1, Vector2>
{
public:
	template <int I>
	static typename LinearOperator<Vector1,Vector2>::DataType At(Vector1 p1, Vector2 p2) {return p1.At<I>() + p2.At<I>();}
};

// Subtraction
template <typename Vector1, typename Vector2>
class DifferenceOperator : public LinearOperator<Vector1, Vector2>
{
public:
	template <int I>
	static typename LinearOperator<Vector1,Vector2>::DataType At(Vector1 p1, Vector2 p2) {return p1.At<I>() - p2.At<I>();}
};

// Compare
template <typename Vector1, typename Vector2>
class CompareOperator : public LinearOperator<Vector1, Vector2>
{
public:
	template <int I>
	static typename LinearOperator<Vector1,Vector2>::DataType At(Vector1 p1, Vector2 p2) {return p1.At<I>() == p2.At<I>();}
};

// Multiply
template <typename Vector, typename Scalar>
class MultiplyOperator : public BaseOperator
{
public:
	typedef typename ProductType<typename UT_SCR(Vector)::DataType, Scalar>::Result DataType;
	const static int SIZE = UT_SCR(Vector)::SIZE; // return a vector of the same size

	template <int I>
	static DataType At(Vector p1, Scalar p2) {return p1.At<I>() * p2;}
};

// Divide
template <typename Vector, typename Scalar>
class DivideOperator : public BaseOperator
{
public:
	typedef typename QuotientType<typename UT_SCR(Vector)::DataType, Scalar>::Result DataType;
	const static int SIZE = UT_SCR(Vector)::SIZE; // return a vector of the same size

	template <int I>
	static DataType At(Vector p1, Scalar p2) {return p1.At<I>() / p2;}
};

// Dot product
template <typename Vector1, typename Vector2>
class DotProductOperator : public BaseOperator
{
public:
	typedef typename ProductType<typename UT_SCR(Vector1)::DataType, typename UT_SCR(Vector2)::DataType>::Result DataType;
	const static int SIZE = 1;

	template <int I>
	static DataType At(Vector1 p1, Vector2 p2) {return p1.At<I>() * p2.At<I>();}

private:
	BIZ_STATIC_ASSERT2(UT_SCR(Vector1)::SIZE == UT_SCR(Vector2)::SIZE, MustBeSameSize);
};

// Multiply
template <typename Vector>
class NegateOperator : public BaseOperator
{
public:
	typedef typename UT_SCR(Vector)::DataType DataType;
	const static int SIZE = UT_SCR(Vector)::SIZE; // return a vector of the same size

	template <int I>
	static DataType At(Vector p) {return -p.At<I>();}
};

//////////////////////////////////////////////////////////////////

template <typename P1, typename P2, template <typename V1, typename V2> class Operator>
class BinaryOperator : public BaseOperator
{
public:
	typedef P1 type1;
	typedef P2 type2;

	typedef typename Operator<P1, P2>::DataType DataType;
	const static int SIZE = Operator<P1, P2>::SIZE;

	BinaryOperator(P1 p1, P2 p2) : m_p1(p1), m_p2(p2) {}

	// evaluate element I
	template <int I>
	DataType At() const {
		return Operator<P1, P2>::template At<I>(m_p1, m_p2);}

private:
	BinaryOperator& operator = (const BinaryOperator& other);

	P1 m_p1;
	P2 m_p2;
};

template <typename Type, template <typename V> class Operator>
class UnaryOperator : public BaseOperator
{
public:
	typedef Type type1;
	typedef Ut::NullType type2;

	UnaryOperator() {}
	UnaryOperator(Type p) : m_p(p) {}

	typedef typename Operator<Type>::DataType DataType;
	const static int SIZE = Operator<Type>::SIZE;

	// evaluate element I
	template <int I>
	DataType At() const {return Operator<Type>::template At<I>(m_p);}

private:
	Type m_p;
};

///////////////////////////////////////////////////////////////////

}

#endif
