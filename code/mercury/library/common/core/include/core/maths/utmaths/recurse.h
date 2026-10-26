#ifndef __GNUC__
#pragma once
#endif

#ifndef _RECURSE_H_
#define _RECURSE_H_

#include "core\maths\constants.h"
#include "core\maths\MathsUtil.h"

namespace UtMaths
{

template <typename T> void Clear(T* p);
template <> inline void Clear<float>(float* p) {*p = 0.f;}
template <> inline void Clear<double>(double* p) {*p = 0.0;}

////////////////////////////////////////////////////////////////////////////////////

/// Recursion templates for unrolling loops.
template<int N>
class Recurse
{
public:
	const static int I = N - 1;

	/// @a
	template <typename Vector, typename Evaluator>
	UT_INLINE static void Set(Vector& v, const Evaluator& e)
	{
		v.template At<I>() = e.template At<I>();
		Recurse<I>::Set(v, e);
	}

	/// @a
	template <typename Vector, typename Scalar>
	UT_INLINE static void Multiply(Vector& v, Scalar e)
	{
		v.template At<I>() *= e;
		Recurse<I>::Multiply(v, e);
	}

	/// @a
	template <typename Vector, typename Scalar>
	static bool EqualityTest(Vector& v, Scalar e)
	{
		if (v.template At<I>() != e)
			return false;
		else
			return Recurse<I>::Equals(v, e);
	}

	/// @a
	template <typename Vector1, typename Vector2>
	static bool Equals(const Vector1& v1, const Vector2& v2)
	{
		if (v1.template At<I>() != v2.template At<I>())
			return false;
		else
			return Recurse<I>::Equals(v1, v2);
	}

	/// @a
	template <typename Vector, typename Scalar>
	static void Divide(Vector& v, Scalar e)
	{
		v.template At<I>() /= e;
		Recurse<I>::Divide(v, e);
	}

	/// @a
	template <typename Vector, typename Evaluator>
	UT_INLINE static void Add(Vector& v, const Evaluator& e)
	{
		v.template At<I>() += e.template At<I>();
		Recurse<I>::Add(v, e);
	}

	/// @a
	template <typename Vector, typename Evaluator>
	UT_INLINE static void Subtract(Vector& v, const Evaluator& e)
	{
		v.template At<I>() -= e.template At<I>();
		Recurse<I>::Subtract(v, e);
	}

	/// @a
	template <typename Evaluator>
	UT_INLINE static typename Evaluator::DataType Sum(const Evaluator& e)
	{
		return e.template At<I>() + Recurse<I>::Sum(e);
	}

	/// @a
	template <typename Evaluator>
	UT_INLINE static typename Evaluator::ElementType Trace(const Evaluator& e)
	{
		return e.template Element<I, I>() + Recurse<I>::Trace(e);
	}

	/// @a
	template <typename Evaluator>
	UT_INLINE static PRODUCT_DATATYPE(Evaluator, Evaluator) SumSquares(const Evaluator& e)
	{
		typename Evaluator::DataType f = e.template At<I>();
		return f * f + Recurse<I>::SumSquares(e);
	}

	/// @a
	template <typename Evaluator>
	UT_INLINE static bool IsEmpty(const Evaluator& e)
	{
		if (e.template At<I>() != Maths::Constants<typename Evaluator::DataType>::Zero()) return false;
		else return Recurse<I>::IsEmpty(e);
	}

	/// @a
	template <typename Evaluator>
	UT_INLINE static typename Evaluator::DataType SumAbs(const Evaluator& e)
	{
		return Maths::Abs(e.template At<I>()) + Recurse<I>::SumAbs(e);
	}

	/// @a
	template <typename Evaluator>
	UT_INLINE static typename Evaluator::DataType Largest(const Evaluator& e)
	{
		return Maths::Max(Maths::Abs(e.template At<I>()), Recurse<I>::Largest(e));
	}

	/// @a
	template <typename Evaluator>
	UT_INLINE static typename Evaluator::DataType Smallest(const Evaluator& e)
	{
		return Maths::Min(Maths::Abs(e.template At<I>()), Recurse<I>::Smallest(e));
	}

	/// @a
	template <typename Vector>
	UT_INLINE static void ClearVector(Vector& v)
	{
		::UtMaths::Clear(&v.template At<I>());
		Recurse<I>::ClearVector(v);
	}

	template <typename Matrix>
	UT_INLINE static void ClearMatrix(Matrix& v)
	{
		Recurse<Matrix::DataType::SIZE>::ClearVector(v.template At<I>());
		Recurse<I>::ClearMatrix(v);
	}

	/// @a
	template <typename Vector>
	UT_INLINE static void Negate(Vector& v)
	{
		v.template At<I>() = -v.template At<I>();
		Recurse<I>::Negate(v);
	}

	/// @a
	template <typename Vector, typename Evaluator>
	UT_INLINE static void Abs(Vector& v, const Evaluator& e)
	{
		v.template At<I>() = Maths::Abs(e.template At<I>());
		Recurse<I>::Abs(v, e);
	}

	template <typename Matrix>
	UT_INLINE static void SetIdentityRow(Matrix& m)
	{
		Recurse<Matrix::DataType::SIZE>::template SetIdentityElement<typename Matrix::DataType, I>(m.template At<I>());
		Recurse<I>::SetIdentityRow(m);
	}

	template <typename Vector, int ROW>
	UT_INLINE static void SetIdentityElement(Vector& v)
	{
		v.template At<I>() = (I == ROW) ? Maths::Constants<typename Vector::DataType>::One() : Maths::Constants<typename Vector::DataType>::Zero();
		Recurse<I>::template SetIdentityElement<Vector, ROW>(v);
	}

	template <typename Matrix, typename Vector>
	UT_INLINE static void SetDiagonalRow(Matrix& m, const Vector& v)
	{
		Recurse<Matrix::DataType::SIZE>::template SetDiagonalElement<Matrix::DataType, Vector, I>(m.template At<I>(), v);
		Recurse<I>::SetDiagonalRow(m, v);
	}

	template <typename Vector1, typename Vector2, int ROW>
	UT_INLINE static void SetDiagonalElement(Vector1& vd, const Vector2& vs)
	{
		vd.template At<I>() = (I == ROW) ?  vs.template At<I>(): Maths::Constants<typename Vector1::DataType>::Zero();
		Recurse<I>::template SetDiagonalElement<Vector1, Vector2, ROW>(vd, vs);
	}

	template <typename Matrix, typename Vector>
	UT_INLINE static void SetInverseDiagonalRow(Matrix& m, const Vector& v)
	{
		Recurse<Matrix::DataType::SIZE>::template SetInverseDiagonalElement<typename Matrix::DataType, Vector, I>(m.template At<I>(), v);
		Recurse<I>::SetInverseDiagonalRow(m, v);
	}

	template <typename Vector1, typename Vector2, int ROW>
	UT_INLINE static void SetInverseDiagonalElement(Vector1& vd, const Vector2& vs)
	{
		vd.template At<I>() = (I == ROW) ?  Maths::Constants<typename Vector1::DataType>::One() / vs.template At<I>(): Maths::Constants<typename Vector1::DataType>::Zero();
		Recurse<I>::template SetInverseDiagonalElement<Vector1, Vector2, ROW>(vd, vs);
	}

	template <typename Matrix, typename E1, typename E2>
	UT_INLINE static void MatrixMultiply(Matrix& m, const E1& e1, const E2& e2)
	{
		Recurse<E2::SIZE>::VectorMatrixProduct(m.template At<I>(), e1.template At<I>(), e2);
		Recurse<I>::MatrixMultiply(m, e1, e2);
	}

	template <typename Vector, typename E1, typename E2>
	UT_INLINE static void VectorMatrixProduct(Vector& v, const E1& e1, const E2& e2)
	{
		v = e1.template At<I>() * e2.template At<I>();
		Recurse<I>::VectorMatrixProductSub(v, e1, e2);
	}

	template <typename Vector, typename E1, typename E2>
	UT_INLINE static void VectorMatrixProductSub(Vector& v, const E1& e1, const E2& e2)
	{
		v += e1.template At<I>() * e2.template At<I>();
		Recurse<I>::VectorMatrixProductSub(v, e1, e2);
	}

	template <typename Vector, typename E1, typename E2>
	UT_INLINE static void MatrixVectorProduct(Vector& v, const E1& e1, const E2& e2)
	{
		v.template At<I>() = e1.template At<I>() | e2;
		Recurse<I>::MatrixVectorProduct(v, e1, e2);
	}

	template <typename Matrix>
	UT_INLINE static void Transpose(Matrix& m)
	{
		Recurse<I>::template TransposeSub<Matrix, I>(m);
		Recurse<I>::Transpose(m);
	}

	template <typename Matrix, int J>
	UT_INLINE static void TransposeSub(Matrix& m)
	{
		BIZ_STATIC_ASSERT(J > I);
		Ut::Swap(m.template Element<I, J>(), m.template Element<J, I>());
		Recurse<I>::template TransposeSub<Matrix, J>(m);
	}

	template <typename Result, typename Vector1, typename Vector2>
	UT_INLINE static void ComponentMultiply(Result& result, const Vector1& v1, const Vector2& v2)
	{
		result.template At<I>() = v1.template At<I>() * v2.template At<I>();
		Recurse<I>::ComponentMultiply(result, v1, v2);
	}
};

template<>
class Recurse<1>
{
	const static int I = 0;
public:
	template <typename Vector, typename Evaluator>
	UT_INLINE static void Set(Vector& v, const Evaluator& e)
	{
		v.template At<I>() = e.template At<I>();
	}

	template <typename Vector, typename Scalar>
	UT_INLINE static void Multiply(Vector& v, Scalar e)
	{
		v.template At<I>() *= e;
	}

	template <typename Vector1, typename Vector2>
	static bool Equals(const Vector1& v1, const Vector2& v2)
	{
		return (v1.template At<I>() == v2.template At<I>());
	}

	template <typename Vector, typename Scalar>
	static bool EqualityTest(Vector& v, Scalar e)
	{
		return (v.template At<I>() == e);
	}

	template <typename Vector, typename Scalar>
	static void Divide(Vector& v, Scalar e)
	{
		v.template At<I>() /= e;
	}

	template <typename Vector, typename Evaluator>
	UT_INLINE static void Add(Vector& v, const Evaluator& e)
	{
		v.template At<I>() += e.template At<I>();
	}

	template <typename Vector, typename Evaluator>
	UT_INLINE static void Subtract(Vector& v, const Evaluator& e)
	{
		v.template At<I>() -= e.template At<I>();
	}

	template <typename Evaluator>
	UT_INLINE static typename Evaluator::DataType Sum(const Evaluator& e)
	{
		return e.template At<I>();
	}

	template <typename Evaluator>
	UT_INLINE static typename Evaluator::ElementType Trace(const Evaluator& e)
	{
		return e.template Element<I, I>();
	}

	template <typename Evaluator>
	UT_INLINE static PRODUCT_DATATYPE(Evaluator, Evaluator) SumSquares(const Evaluator& e)
	{
		typename Evaluator::DataType f = e.template At<I>();
		return f * f;
	}

	template <typename Evaluator>
	UT_INLINE static bool IsEmpty(const Evaluator& e)
	{
		if (e.template At<I>() != Maths::Constants<typename Evaluator::DataType>::Zero()) return false;
		else return true;
	}

	template <typename Evaluator>
	UT_INLINE static typename Evaluator::DataType SumAbs(const Evaluator& e)
	{
		return Maths::Abs(e.template At<I>());
	}

	template <typename Evaluator>
	UT_INLINE static typename Evaluator::DataType Largest(const Evaluator& e)
	{
		return Maths::Abs(e.template At<I>());
	}

	template <typename Evaluator>
	UT_INLINE static typename Evaluator::DataType Smallest(const Evaluator& e)
	{
		return Maths::Abs(e.template At<I>());
	}

	template <typename Vector>
	UT_INLINE static void ClearVector(Vector& v)
	{
		::UtMaths::Clear(&v.template At<I>());
	}

	template <typename Matrix>
	UT_INLINE static void ClearMatrix(Matrix& v)
	{
		Recurse<Matrix::DataType::SIZE>::ClearVector(v.template At<I>());
	}

	template <typename Vector>
	UT_INLINE static void Negate(Vector& v)
	{
		v.template At<I>() = -v.template At<I>();
	}

	template <typename Vector, typename Evaluator>
	UT_INLINE static void Abs(Vector& v, const Evaluator& e)
	{
		v.template At<I>() = Maths::Abs(e.template At<I>());
	}

	template <typename Matrix>
	UT_INLINE static void SetIdentityRow(Matrix& m)
	{
		Recurse<Matrix::DataType::SIZE>::template SetIdentityElement<typename Matrix::DataType, I>(m.template At<I>());
	}

	template <typename Vector, int ROW>
	UT_INLINE static void SetIdentityElement(Vector& v)
	{
		v.template At<I>() = (I == ROW) ? Maths::Constants<typename Vector::DataType>::One() : Maths::Constants<typename Vector::DataType>::Zero();
	}

	template <typename Matrix, typename Vector>
	UT_INLINE static void SetDiagonalRow(Matrix& m, const Vector& v)
	{
		Recurse<Matrix::DataType::SIZE>::template SetDiagonalElement<typename Matrix::DataType, Vector, I>(m.template At<I>(), v);
	}

	template <typename Vector1, typename Vector2, int ROW>
	UT_INLINE static void SetDiagonalElement(Vector1& vd, const Vector2& vs)
	{
		vd.template At<I>() = (I == ROW) ?  vs.template At<I>(): Maths::Constants<typename Vector1::DataType>::Zero();
	}

	template <typename Matrix, typename Vector>
	UT_INLINE static void SetInverseDiagonalRow(Matrix& m, const Vector& v)
	{
		Recurse<Matrix::DataType::SIZE>::template SetInverseDiagonalElement<typename Matrix::DataType, Vector, I>(m.template At<I>(), v);
	}

	template <typename Vector1, typename Vector2, int ROW>
	UT_INLINE static void SetInverseDiagonalElement(Vector1& vd, const Vector2& vs)
	{
		vd.template At<I>() = (I == ROW) ?  Maths::Constants<typename Vector1::DataType>::One() / vs.template At<I>(): Maths::Constants<typename Vector1::DataType>::Zero();
	}

	template <typename Matrix, typename E1, typename E2>
	UT_INLINE static void MatrixMultiply(Matrix& m, const E1& e1, const E2& e2)
	{
		Recurse<E2::SIZE>::VectorMatrixProduct(m.template At<I>(), e1.template At<I>(), e2);
	}

	template <typename Vector, typename E1, typename E2>
	UT_INLINE static void VectorMatrixProductSub(Vector& v, const E1& e1, const E2& e2)
	{
		v += e1.template At<I>() * e2.template At<I>();
	}

	template <typename Vector, typename E1, typename E2>
	UT_INLINE static void MatrixVectorProduct(Vector& v, const E1& e1, const E2& e2)
	{
		v.template At<I>() = e1.template At<I>() | e2;
	}

	template <typename Matrix>
	UT_INLINE static void Transpose(Matrix& m)
	{
	}

	template <typename Matrix, int J>
	UT_INLINE static void TransposeSub(Matrix& m)
	{
		BIZ_STATIC_ASSERT(J > I);
		Ut::Swap(m.template Element<I, J>(), m.template Element<J, I>());
	}

	template <typename Result, typename Vector1, typename Vector2>
	UT_INLINE static void ComponentMultiply(Result& result, const Vector1& v1, const Vector2& v2)
	{
		result.template At<I>() = v1.template At<I>() * v2.template At<I>();
	}
};

}

#endif
