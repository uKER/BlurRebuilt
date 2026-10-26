////////////////////////////////////////////////////////////////
// MMatrix classes

#ifndef __CORE_MMATRIX_H__
#define __CORE_MMATRIX_H__
#pragma once

#include "maths.h"

namespace UtMaths
{

/// Creates a general matrix type.
template <typename Matrix>
class MMatrix : public Matrix
{
public:
	typedef typename Matrix::DataType DataType; ///< @a
	typedef typename DataType::DataType ElementType; ///< @a
	const static int SIZE = Matrix::SIZE; ///< @a

	/// Default constructor does nothing.
	MMatrix() {}
	MMatrix(typename Matrix::type1 p) : Matrix(p) {}
	MMatrix(typename Matrix::type1 p1, typename Matrix::type2 p2) : Matrix(p1, p2) {}

	/// Copy constructor.
	template<typename Evaluator>
	MMatrix(const MMatrix<Evaluator>& e) : Matrix(e) {}

	// modification
	template<typename Evaluator>
	MMatrix& operator = (const MMatrix<Evaluator>& e) {Matrix::operator =(e); return *this;} ///< @a

	template<typename Evaluator>
	MMatrix& operator += (const MMatrix<Evaluator>& e) {Matrix::operator +=(e); return *this;} ///< @a

	template<typename  Evaluator>
	MMatrix& operator -= (const MMatrix<Evaluator>& e) {Matrix::operator -=(e); return *this;} ///< @a

	template<typename Scalar>
	MMatrix& operator *= (Scalar e) {Matrix::operator *=(e); return *this;} ///< @a

	template<typename Scalar>
	MMatrix& operator /= (Scalar e) {Matrix::operator /=(e); return *this;} ///< @a
};

/////////////////////////////////////////////////////////////////////////////

template <typename P1, typename P2, template <typename V1, typename V2> class Operator>
class BinaryOperatorMM : public BinaryOperator<P1, P2, Operator>
{
public:
	BinaryOperatorMM(P1 p1, P2 p2) : BinaryOperator<P1, P2, Operator>(p1, p2) {}

	// static functions to return new operators 
	static MMatrix<BinaryOperatorMM> Sum(P1 p1, P2 p2) {return MMatrix<BinaryOperatorMM>(p1, p2);}
	static MMatrix<BinaryOperatorMM> Difference(P1 p1, P2 p2) {return MMatrix<BinaryOperatorMM>(p1, p2);}
};

template <typename P1, typename P2, template <typename V1, typename V2> class Operator>
class BinaryOperatorMS : public BinaryOperator<P1, P2, Operator>
{
public:
	BinaryOperatorMS(P1 p1, P2 p2) : BinaryOperator<P1, P2, Operator>(p1, p2) {}

	// static functions to return new operators 
	static MMatrix<BinaryOperatorMS> Multiply(P1 p1, P2 p2) {return MMatrix<BinaryOperatorMS>(p1, p2);}
};

// In general, the destination may overlap with the source.
// The result can't be evaluated element by element in this case.
// Vector-Matrix multiplication therefore has to return a temporary vector.
template<typename Matrix>
class ProcessedMatrix;

template<typename Matrix, typename Vector, typename ResultType>
class BinaryOperatorMV
{
public:
	static ResultType VectorMatrixProduct(const MConstVector<Vector>& v, const MMatrix<Matrix>& m)
	{
		typename ProcessedMatrix<Matrix>::Result m1(m);
		typename ProcessedVector<Vector>::Result v1(MakeVector(v));

		ResultType result;
		Recurse<ResultType::SIZE>::VectorMatrixProduct(result, v1, m1);
		return result;
	}

	static ResultType MatrixVectorProduct(const MMatrix<Matrix>& m, const MConstVector<Vector>& v)
	{
		typename ProcessedMatrix<Matrix>::Result m1(m);
		typename ProcessedVector<Vector>::Result v1(MakeVector(v));

		ResultType result;
		Recurse<ResultType::SIZE>::MatrixVectorProduct(result, m1, v1);
		return result;
	}
};

/////////////////////////////////////////////////////////////////////////////
// operator types for matrices - can be specialised

template<typename T, int N> class BasicMatrix;

template<typename Type1, typename Type2>
struct MatrixSumType {typedef BinaryOperatorMM<const Type1&, const Type2&, SumOperator> Result;};

template<typename Type1, typename Type2>
struct MatrixDifferenceType {typedef BinaryOperatorMM<const Type1&, const Type2&, DifferenceOperator> Result;};

template<typename Type1, typename Type2>
struct MatrixMatrixProductType 
{
	typedef typename Type1::DataType Vector1;
	typedef typename Type2::DataType Vector2;
	typedef typename ProductType<Vector2, typename Vector1::DataType>::Result Vector;
	typedef BasicMatrix<typename UT_SCR(typename ProcessedVector<typename Vector::VectorType>::Result), Type1::SIZE> Result;
	typedef MMatrix<Result> Operator;
};

template<typename Type1, typename Type2>
struct MatrixScalarProductType {typedef BinaryOperatorMS<const Type1&, Type2, MultiplyOperator> Result;};

template<typename Vector, typename Matrix>
struct VectorMatrixProductType
{
	typedef typename ProductType<typename Matrix::DataType, typename Vector::DataType>::Result Product;
	typedef typename UT_SCR(typename ProcessedVector<typename Product::VectorType>::Result) Result;
	typedef BinaryOperatorMV<Matrix, Vector, Result> Operator;
};

template<typename Matrix, typename Vector>
struct MatrixVectorProductType 
{
	typedef typename Matrix::DataType Vector1;
	typedef typename ProductType<MVector<Vector>, typename Vector1::DataType>::Result Product;
	typedef typename UT_SCR(typename ProcessedVector<typename Product::VectorType>::Result) Result;
	typedef BinaryOperatorMV<Matrix, Vector, Result> Operator;
};

/////////////////////////////////////////////////////////////////////////////
// operator overloading

/// Matrix addition
template<typename Matrix1, typename Matrix2>
inline MMatrix<typename MatrixSumType<Matrix1, Matrix2>::Result> operator + (const MMatrix<Matrix1>& p1, const MMatrix<Matrix2>& p2)
{
	return MMatrix<typename MatrixSumType<Matrix1, Matrix2>::Result>::Sum(p1, p2);
}

/// Matrix subtraction
template<typename Matrix1, typename Matrix2>
inline MMatrix<typename MatrixDifferenceType<Matrix1, Matrix2>::Result> operator - (const MMatrix<Matrix1>& p1, const MMatrix<Matrix2>& p2)
{
	return MMatrix<typename MatrixDifferenceType<Matrix1, Matrix2>::Result>::Difference(p1, p2);
}

/// Matrix multiplication
template<typename Matrix1, typename Matrix2>
inline MMatrix<typename MatrixMatrixProductType<Matrix1, Matrix2>::Result> operator * (const MMatrix<Matrix1>& p1, const MMatrix<Matrix2>& p2)
{
	BIZ_STATIC_ASSERT(Matrix1::DataType::SIZE == Matrix2::SIZE);
	return MatrixMatrixProductType<Matrix1, Matrix2>::Operator::MatrixProduct(p1, p2);
}

/// Vector * Matrix
template<typename Vector, typename Matrix>
inline typename VectorMatrixProductType<Vector, Matrix>::Result operator * (const MVector<Vector>& p1, const MMatrix<Matrix>& p2)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == Matrix::SIZE);
	return VectorMatrixProductType<Vector, Matrix>::Operator::VectorMatrixProduct(p1, p2);
}

/// Matrix * Vector
template<typename Matrix, typename Vector>
inline typename MatrixVectorProductType<Matrix, Vector>::Result operator * (const MMatrix<Matrix>& p1, const MVector<Vector>& p2)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == Matrix::SIZE);
	return MatrixVectorProductType<Matrix, Vector>::Operator::MatrixVectorProduct(p1, p2);
}

/// Vector * Matrix
template<typename Vector, typename Matrix>
inline typename VectorMatrixProductType<Vector, Matrix>::Result operator * (const MConstVector<Vector>& p1, const MMatrix<Matrix>& p2)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == Matrix::SIZE);
	return VectorMatrixProductType<Vector, Matrix>::Operator::VectorMatrixProduct(p1, p2);
}

/// Matrix * Vector
template<typename Matrix, typename Vector>
inline typename MatrixVectorProductType<Matrix, Vector>::Result operator * (const MMatrix<Matrix>& p1, const MConstVector<Vector>& p2)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == Matrix::SIZE);
	return MatrixVectorProductType<Matrix, Vector>::Operator::MatrixVectorProduct(p1, p2);
}

/// Normal * Matrix
template<typename Vector, typename Matrix>
inline typename VectorMatrixProductType<Vector, Matrix>::Result operator * (const MNormal<Vector>& p1, const MMatrix<Matrix>& p2)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == Matrix::SIZE);
	return VectorMatrixProductType<Vector, Matrix>::Operator::VectorMatrixProduct(p1, p2);
}

/// Matrix * Normal
template<typename Matrix, typename Vector>
inline typename MatrixVectorProductType<Matrix, Vector>::Result operator * (const MMatrix<Matrix>& p1, const MNormal<Vector>& p2)
{
	BIZ_STATIC_ASSERT(Vector::SIZE == Matrix::SIZE);
	return MatrixVectorProductType<Matrix, Vector>::Operator::MatrixVectorProduct(p1, p2);
}

/// Matrix * Scalar
template<typename Matrix, typename Scalar>
inline MMatrix<typename MatrixScalarProductType<Matrix, Scalar>::Result> operator * (const MMatrix<Matrix>& p1, Scalar p2)
{
	return MMatrix<typename MatrixScalarProductType<Matrix, Scalar>::Result>::Multiply(p1, p2);
}

/// Scalar * Matrix
template<typename Matrix, typename Scalar>
inline MMatrix<typename MatrixScalarProductType<Matrix, Scalar>::Result> operator * (Scalar p2, const MMatrix<Matrix>& p1)
{
	return MMatrix<typename MatrixScalarProductType<Matrix, Scalar>::Result>::Multiply(p1, p2);
}

/////////////////////////////////////////////////////////////////////////////

// If the matrix is an operator, returns a type to evaluate it. Otherwise returns a reference to the matrix type.
template<typename Matrix>
class ProcessedMatrix
{
public:
	typedef typename Ut::Select<Ut::IsConvertible<Matrix, BaseOperator>::exists,
								MMatrix<BasicMatrix<typename Matrix::DataType, Matrix::SIZE> >,
								const MMatrix<Matrix>&>::Result Result;
};

/// Class to construct a matrix from a number of vectors
/// @param T The vector type for the rows
/// @param N The number of rows.
template<typename T, int N> 
class BasicMatrix
{
public:
	typedef T DataType;
	typedef typename DataType::DataType ElementType;
	const static int SIZE = N;

	// compiler bug: needed to align vectors properly
#ifdef UT_MMATRIX_ALIGN
	__declspec(align(16)) DataType m_data[SIZE];
#else
	DataType m_data[SIZE];
#endif

	BasicMatrix() {}

	/// General constructor
	template <typename Evaluator>
	BasicMatrix(const Evaluator& e) {Recurse<SIZE>::Set(*this, e);}

	/// Returns the number of rows.
	int Size() const {return SIZE;}

	const T& AX() const {return At<0>();}
	const T& AY() const {return At<1>();}
	const T& AZ() const {return At<2>();}
	void AX(const MConstVector<typename T::VectorType>& a) {At<0>() = a;}
	void AY(const MConstVector<typename T::VectorType>& a) {At<1>() = a;}
	void AZ(const MConstVector<typename T::VectorType>& a) {At<2>() = a;}

	/// Array-style access
	UT_FORCE_INLINE T& operator [] (int i) {assert(i > -1 && i < SIZE); return *(((T*)this) + i);}
	const T& operator [] (int i) const {assert(i > -1 && i < SIZE); return *(((T*)this) + i);}

	/// Access to rows.
	template <int I>
	T& At() {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *(((T*)this) + I);}

	template <int I>
	const T& At() const {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return *(((T*)this) + I);} ///< Access to row I

	/// Access to elements.
	template <int I, int J>
	ElementType& Element() {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return At<I>().At<J>();}

	template <int I, int J>
	const ElementType& Element() const {BIZ_STATIC_ASSERT(I > -1 && I < SIZE); return At<I>().At<J>();} ///< Access to element J in row I

	// operators
	template<typename Evaluator>
	void operator = (const Evaluator& e) {Recurse<SIZE>::Set(*this, e);} ///< @a

	template<typename Scalar>
	void operator *= (Scalar e) {Recurse<SIZE>::Multiply(*this, e);}  ///< @a

	template<typename Scalar>
	void operator /= (Scalar e) {Recurse<SIZE>::Divide(*this, e);}  ///< @a

	template<typename Evaluator>
	void operator += (const Evaluator& e) {Recurse<SIZE>::Add(*this, e);}  ///< @a

	template<typename Evaluator>
	void operator -= (const Evaluator& e) {Recurse<SIZE>::Subtract(*this, e);}  ///< @a

	template<typename Matrix1, typename Matrix2>
	static MMatrix<typename MatrixMatrixProductType<Matrix1, Matrix2>::Result> MatrixProduct(const MMatrix<Matrix1>& p1, const MMatrix<Matrix2>& p2)
	{
		typename ProcessedMatrix<Matrix1>::Result m1(p1);
		typename ProcessedMatrix<Matrix2>::Result m2(p2);

		typedef MMatrix<typename MatrixMatrixProductType<Matrix1, Matrix2>::Result> ResultType;
		ResultType result;

		Recurse<ResultType::SIZE>::MatrixMultiply(result, m1, m2);
		return result;
	}

protected:
	typedef Ut::NullType type1;
	typedef Ut::NullType type2;
};

/////////////////////////////////////////////////////////////////////////////
// matrix functions

/// Set all components to zero.
template <typename Matrix>
inline void Clear(MMatrix<Matrix>* m)
{
	Recurse<Matrix::SIZE>::ClearMatrix(*m);
}

/// Identity.
template <typename Matrix>
inline void SetIdentity(MMatrix<Matrix>* m)
{
	BIZ_STATIC_ASSERT(Matrix::SIZE == Matrix::DataType::SIZE);
	Recurse<Matrix::SIZE>::SetIdentityRow(*m);
}

/// Transpose.
template <typename Matrix>
inline void Transpose(MMatrix<Matrix>* m)
{
	BIZ_STATIC_ASSERT(Matrix::SIZE == Matrix::DataType::SIZE);
	Recurse<Matrix::SIZE>::Transpose(*m);
}

/// Find the sum of the diagonal components
template <typename Matrix>
typename Matrix::ElementType Trace(const MMatrix<Matrix>& m) 
{
	BIZ_STATIC_ASSERT(Matrix::SIZE == Matrix::DataType::SIZE);
	return Recurse<Matrix::SIZE>::Trace(m);
}

/// Transpose the matrix.
template <typename Matrix>
inline MMatrix<Matrix> operator ~ (const MMatrix<Matrix>& m)
{
	MMatrix<Matrix> t(m);
	Transpose(&t);
	return t;
}

/// Diagonal matrix.
template <typename Matrix, typename Vector>
inline void Diagonal(MMatrix<Matrix>* m, const MConstVector<Vector>& v) 
{
	BIZ_STATIC_ASSERT(Matrix::SIZE == Matrix::DataType::SIZE && Matrix::SIZE == Vector::SIZE);
	Recurse<Matrix::SIZE>::SetDiagonalRow(*m, v);
}

/// Inverse diagonal matrix.
template <typename Matrix, typename Vector>
inline void InverseDiagonal(MMatrix<Matrix>* m, const MConstVector<Vector>& v) 
{
	BIZ_STATIC_ASSERT(Matrix::SIZE == Matrix::DataType::SIZE && Matrix::SIZE == Vector::SIZE);
	Recurse<Matrix::SIZE>::SetInverseDiagonalRow(*m, v);
}

template <typename Matrix1, typename Matrix2>
UT_INLINE bool operator == (const MMatrix<Matrix1>& m1, const MMatrix<Matrix2>& m2)
{
	BIZ_STATIC_ASSERT(Matrix1::SIZE == Matrix2::SIZE);
	return Recurse<Matrix1::SIZE>::Equals(m1, m2);
}

template <typename Matrix1, typename Matrix2>
UT_INLINE bool operator != (const MMatrix<Matrix1>& m1, const MMatrix<Matrix2>& m2)
{
	BIZ_STATIC_ASSERT(Matrix1::SIZE == Matrix2::SIZE);
	return !Recurse<Matrix1::SIZE>::Equals(m1, m2);
}

/// Make orthonormal matrix from 'nearly' orthonormal matrix
template<typename Matrix>
inline void Normalise(MMatrix<Matrix>* m) 
{
	BIZ_STATIC_ASSERT(Matrix::SIZE == 3 && Matrix::DataType::SIZE == 3);

	typedef typename Matrix::DataType Vector;
	typedef typename Vector::DotProductType Element;
	Vector& ax = m->template At<0>();
	Vector& ay = m->template At<1>();
	Vector& az = m->template At<2>();
	Element one = Maths::Constants<Element>::One();
	Element half = Maths::Constants<Element>::Half();

	Element xx = one - Magnitude2(ax);
	Element yy = one - Magnitude2(ay);
	Element xy = ax | ay;

	// normalise first two rows (approximate)
	if (Maths::Abs(xx) > Maths::Abs(yy))
		ax *= (one + xx * half);
	else
		ay *= (one + yy * half);

	// One component of ay can be expressed in terms of the two others and ax.
	// As it is approximately normalised already, one component will be bigger than 0.5.
	if (Maths::Abs(xy) > Maths::Constants<Element>::Epsilon())
	{
		if (Maths::Abs(ax.X()) > half)
			ay.x = -(xy - ax.x * ay.x) / ax.x;
		else if (Maths::Abs(ax.Z()) > half)
			ay.z = -(xy - ax.z * ay.z) / ax.z;
		else
			ay.y = -(xy - ax.y * ay.y) / ax.y;	
	}

	// third row is cross product of first two rows
	az = ax ^ ay;
}

/// Invert an orthogonal 4x4 matrix.
template<typename Matrix1, typename Matrix2>
void InvertOrthonormal(MMatrix<Matrix1>* matrixOut, const MMatrix<Matrix2>& matrixIn)
{
	BIZ_STATIC_ASSERT(Matrix1::SIZE == 4 && Matrix1::DataType::SIZE == 4);
	BIZ_STATIC_ASSERT(Matrix2::SIZE == 4 && Matrix2::DataType::SIZE == 4);

    matrixOut->template Element<0, 0>() = matrixIn.template Element<0, 0>();
    matrixOut->template Element<0, 1>() = matrixIn.template Element<1, 0>();
    matrixOut->template Element<0, 2>() = matrixIn.template Element<2, 0>();
	matrixOut->template Element<0, 3>() = Maths::Constants<typename Matrix1::ElementType>::Zero();

    matrixOut->template Element<1, 0>() = matrixIn.template Element<0, 1>();
    matrixOut->template Element<1, 1>() = matrixIn.template Element<1, 1>();
    matrixOut->template Element<1, 2>() = matrixIn.template Element<2, 1>();
    matrixOut->template Element<1, 3>() = Maths::Constants<typename Matrix1::ElementType>::Zero();

    matrixOut->template Element<2, 0>() = matrixIn.template Element<0, 2>();
    matrixOut->template Element<2, 1>() = matrixIn.template Element<1, 2>();
    matrixOut->template Element<2, 2>() = matrixIn.template Element<2, 2>();
    matrixOut->template Element<2, 3>() = Maths::Constants<typename Matrix1::ElementType>::Zero();

    matrixOut->template Element<3, 0>() = -matrixIn.template At<3>() | matrixIn.template At<0>();
    matrixOut->template Element<3, 1>() = -matrixIn.template At<3>() | matrixIn.template At<1>();
    matrixOut->template Element<3, 2>() = -matrixIn.template At<3>() | matrixIn.template At<2>();
    matrixOut->template Element<3, 3>() = Maths::Constants<typename Matrix1::ElementType>::One();
}

// Solve an equation in the form Ax = 0.
// Destroys matrix, returns result in x.
// Returns false if the matrix was singular.
// Requires array access to vector and matrix.
template <typename Matrix, typename Vector>
bool GESolve(MMatrix<Matrix>& a, MVector<Vector>* x)
{
	typedef typename Vector::DataType DataType;
	DataType epsilon = Maths::Constants<DataType>::Epsilon();
	int nRows = Matrix::SIZE;

	int i, j, k, maxrow;
	DataType tmp;

	for (i = 0; i < nRows; i++) 
	{
		// Find the row with the largest first value.
		maxrow = i;
		for (j = i + 1; j < nRows; j++) 
		{
			if (Maths::Abs(a[j][i]) > Maths::Abs(a[maxrow][i]))
				maxrow = j;
		}

		// Swap the maxrow and ith row.
		Ut::Swap(a[i], a[maxrow]);
	
		// Singular matrix?
		if (Maths::Abs(a[i][i]) < epsilon)
			return false;

		// Eliminate the ith element of the jth row.
		for (j = i + 1; j < nRows; j++) 
			for (k = nRows; k >= i; k--) 
				a[j][k] -= a[i][k] * a[j][i] / a[i][i];
   }

   // Do the back substitution.
   for (j = nRows - 1; j >= 0; j--) 
   {
		tmp = 0;
		for (k = j + 1; k < nRows; k++)
			tmp += a[j][k] * (*x)[k];

		(*x)[j] = (a[j][nRows] - tmp) / a[j][j];
   }

   return true;
}

// Get determinant of an Matrix33.
template<typename Matrix>
inline typename Matrix::ElementType GetDeterminant3x3(const MMatrix<Matrix>& m)
{
	BIZ_STATIC_ASSERT(Matrix::SIZE == 3 && Matrix::DataType::SIZE == 3);

	return  m[0][0]*m[1][1]*m[2][2] -
			m[0][0]*m[1][2]*m[2][1] -
			m[0][1]*m[1][0]*m[2][2] +
			m[0][1]*m[1][2]*m[2][0] +
			m[0][2]*m[1][0]*m[2][1] -
			m[0][2]*m[1][1]*m[2][0];
}

// Get determinant of an Matrix44.
template<typename Matrix>
inline typename Matrix::ElementType GetDeterminant4x4(const MMatrix<Matrix>& m)
{
	BIZ_STATIC_ASSERT(Matrix::SIZE == 4 && Matrix::DataType::SIZE == 4);

	return  m[0][0]*m[1][1]*m[2][2]*m[3][3] -
			m[0][0]*m[1][1]*m[2][3]*m[3][2] -
			m[0][0]*m[1][2]*m[2][1]*m[3][3] +
			m[0][0]*m[1][2]*m[2][3]*m[3][1] +
			m[0][0]*m[1][3]*m[2][1]*m[3][2] -
			m[0][0]*m[1][3]*m[2][2]*m[3][1] -
			m[0][1]*m[1][0]*m[2][2]*m[3][3] +
			m[0][1]*m[1][0]*m[2][3]*m[3][2] +
			m[0][1]*m[1][2]*m[2][0]*m[3][3] -
			m[0][1]*m[1][2]*m[2][3]*m[3][0] -
			m[0][1]*m[1][3]*m[2][0]*m[3][2] +
			m[0][1]*m[1][3]*m[2][2]*m[3][0] +
			m[0][2]*m[1][0]*m[2][1]*m[3][3] -
			m[0][2]*m[1][0]*m[2][3]*m[3][1] -
			m[0][2]*m[1][1]*m[2][0]*m[3][3] +
			m[0][2]*m[1][1]*m[2][3]*m[3][0] +
			m[0][2]*m[1][3]*m[2][0]*m[3][1] -
			m[0][2]*m[1][3]*m[2][1]*m[3][0] -
			m[0][3]*m[1][0]*m[2][1]*m[3][2] +
			m[0][3]*m[1][0]*m[2][2]*m[3][1] +
			m[0][3]*m[1][1]*m[2][0]*m[3][2] -
			m[0][3]*m[1][1]*m[2][2]*m[3][0] -
			m[0][3]*m[1][2]*m[2][0]*m[3][1] +
			m[0][3]*m[1][2]*m[2][1]*m[3][0];
}

/// Invert a 4x4 matrix.
template<typename Matrix1, typename Matrix2>
void Invert4x4(MMatrix<Matrix1>* outInverse, const MMatrix<Matrix2>& m)
{
	BIZ_STATIC_ASSERT(Matrix1::SIZE == 4 && Matrix1::DataType::SIZE == 4);
	BIZ_STATIC_ASSERT(Matrix2::SIZE == 4 && Matrix2::DataType::SIZE == 4);

	float det = GetDeterminant4x4(m);

#ifdef BUILD_CONFIG_DEBUG
	assert(det != 0.f);
#endif
	float invDet = 1.f;
	if (det != 0.f)
	{
		invDet = 1.f / det;
	}

	// First column.
	(*outInverse)[0][0] = m[1][1]*m[2][2]*m[3][3] -
								m[1][1]*m[2][3]*m[3][2] -
								m[1][2]*m[2][1]*m[3][3] +
								m[1][2]*m[2][3]*m[3][1] +
								m[1][3]*m[2][1]*m[3][2] -
								m[1][3]*m[2][2]*m[3][1];

	(*outInverse)[1][0] = -m[1][0]*m[2][2]*m[3][3] +
								m[1][0]*m[2][3]*m[3][2] +
								m[1][2]*m[2][0]*m[3][3] -
								m[1][2]*m[2][3]*m[3][0] -
								m[1][3]*m[2][0]*m[3][2] +
								m[1][3]*m[2][2]*m[3][0];

	(*outInverse)[2][0] = m[1][0]*m[2][1]*m[3][3] -
								m[1][0]*m[2][3]*m[3][1] -
								m[1][1]*m[2][0]*m[3][3] +
								m[1][1]*m[2][3]*m[3][0] +
								m[1][3]*m[2][0]*m[3][1] -
								m[1][3]*m[2][1]*m[3][0];

	(*outInverse)[3][0] = -m[1][0]*m[2][1]*m[3][2] +
								m[1][0]*m[2][2]*m[3][1] +
								m[1][1]*m[2][0]*m[3][2] -
								m[1][1]*m[2][2]*m[3][0] -
								m[1][2]*m[2][0]*m[3][1] +
								m[1][2]*m[2][1]*m[3][0];

	// Second column.
	(*outInverse)[0][1] = -m[0][1]*m[2][2]*m[3][3] +
								m[0][1]*m[2][3]*m[3][2] +
								m[0][2]*m[2][1]*m[3][3] -
								m[0][2]*m[2][3]*m[3][1] -
								m[0][3]*m[2][1]*m[3][2] +
								m[0][3]*m[2][2]*m[3][1];
	
	(*outInverse)[1][1] = m[0][0]*m[2][2]*m[3][3] -
								m[0][0]*m[2][3]*m[3][2] -
								m[0][2]*m[2][0]*m[3][3] +
								m[0][2]*m[2][3]*m[3][0] +
								m[0][3]*m[2][0]*m[3][2] -
								m[0][3]*m[2][2]*m[3][0];

	(*outInverse)[2][1] = -m[0][0]*m[2][1]*m[3][3] +
								m[0][0]*m[2][3]*m[3][1] +
								m[0][1]*m[2][0]*m[3][3] -
								m[0][1]*m[2][3]*m[3][0] -
								m[0][3]*m[2][0]*m[3][1] +
								m[0][3]*m[2][1]*m[3][0];

	(*outInverse)[3][1] = m[0][0]*m[2][1]*m[3][2] -
								m[0][0]*m[2][2]*m[3][1] -
								m[0][1]*m[2][0]*m[3][2] +
								m[0][1]*m[2][2]*m[3][0] +
								m[0][2]*m[2][0]*m[3][1] -
								m[0][2]*m[2][1]*m[3][0];

	// Third column.
	(*outInverse)[0][2] = m[0][1]*m[1][2]*m[3][3] -
								m[0][1]*m[1][3]*m[3][2] -
								m[0][2]*m[1][1]*m[3][3] +
								m[0][2]*m[1][3]*m[3][1] +
								m[0][3]*m[1][1]*m[3][2] -
								m[0][3]*m[1][2]*m[3][1];

	(*outInverse)[1][2] = -m[0][0]*m[1][2]*m[3][3] +
								m[0][0]*m[1][3]*m[3][2] +
								m[0][2]*m[1][0]*m[3][3] -
								m[0][2]*m[1][3]*m[3][0] -
								m[0][3]*m[1][0]*m[3][2] +
								m[0][3]*m[1][2]*m[3][0];

	(*outInverse)[2][2] = m[0][0]*m[1][1]*m[3][3] -
								m[0][0]*m[1][3]*m[3][1] -
								m[0][1]*m[1][0]*m[3][3] +
								m[0][1]*m[1][3]*m[3][0] +
								m[0][3]*m[1][0]*m[3][1] -
								m[0][3]*m[1][1]*m[3][0];

	(*outInverse)[3][2] = -m[0][0]*m[1][1]*m[3][2] +
								m[0][0]*m[1][2]*m[3][1] +
								m[0][1]*m[1][0]*m[3][2] -
								m[0][1]*m[1][2]*m[3][0] -
								m[0][2]*m[1][0]*m[3][1] +
								m[0][2]*m[1][1]*m[3][0];

	// Fourth column.
	(*outInverse)[0][3] = -m[0][1]*m[1][2]*m[2][3] +
								m[0][1]*m[1][3]*m[2][2] +
								m[0][2]*m[1][1]*m[2][3] -
								m[0][2]*m[1][3]*m[2][1] -
								m[0][3]*m[1][1]*m[2][2] +
								m[0][3]*m[1][2]*m[2][1];

	(*outInverse)[1][3] = m[0][0]*m[1][2]*m[2][3] -
								m[0][0]*m[1][3]*m[2][2] -
								m[0][2]*m[1][0]*m[2][3] +
								m[0][2]*m[1][3]*m[2][0] +
								m[0][3]*m[1][0]*m[2][2] -
								m[0][3]*m[1][2]*m[2][0];

	(*outInverse)[2][3] = -m[0][0]*m[1][1]*m[2][3] +
								m[0][0]*m[1][3]*m[2][1] +
								m[0][1]*m[1][0]*m[2][3] -
								m[0][1]*m[1][3]*m[2][0] -
								m[0][3]*m[1][0]*m[2][1] +
								m[0][3]*m[1][1]*m[2][0];

	(*outInverse)[3][3] = m[0][0]*m[1][1]*m[2][2] -
								m[0][0]*m[1][2]*m[2][1] -
								m[0][1]*m[1][0]*m[2][2] +
								m[0][1]*m[1][2]*m[2][0] +
								m[0][2]*m[1][0]*m[2][1] -
								m[0][2]*m[1][1]*m[2][0];

	// Scale by 1/determinant.
	(*outInverse) *= invDet;
}

//////////////////////////////////////////////////////////////////////////////////

// some typedefs for convenience

/// A 3x3 matrix type.
typedef MMatrix<BasicMatrix<MVector3F, 3> > MMatrix33;

/// A 4x4 matrix type.
typedef MMatrix<BasicMatrix<MVector4F, 4> > MMatrix44;

/// A 4x3 matrix type.
typedef MMatrix<BasicMatrix<MVector4F, 3> > MMatrix43;

///////////////////////////////////////////////////////////////////////////////////

inline MMatrix44 NormalisedZY(const MMatrix44& m)
{
	MVector3F az(m[2][0], m[2][1], m[2][2]);
	UtMaths::SafeNormalise(&az);
    
	MVector3F tmp(m[1][0], m[1][1], m[1][2]);
	MVector3F ax(tmp ^ az);
	UtMaths::SafeNormalise(&ax);

	MVector3F ay(az ^ ax);

	MMatrix44 out;
	out[0][0] = ax[0];
	out[0][1] = ax[1];
	out[0][2] = ax[2];
	
	out[1][0] = ay[0];
	out[1][1] = ay[1];
	out[1][2] = ay[2];
	
	out[2][0] = az[0];
	out[2][1] = az[1];
	out[2][2] = az[2];

	out[0][3] = 0.f;
	out[1][3] = 0.f;
	out[2][3] = 0.f;
	out[3][3] = 1.f;
	return out;
}
        
/// Conversion to Euler angles - Roll-Pitch-Heading
template <typename Matrix>
inline void GetRotationRPH(F32& r, F32& p, F32& h, const MMatrix<Matrix>& m)
{
    const F32 sinPitch(-m[2][1]);
    p = asin(sinPitch);

    
	if ( fabs( sinPitch ) < 0.999f )
    {
        // m[X][Y] / m[Y][Y} = sin(r)cos(p) / cos(r)cos(p)
        //                   = sin(r) / cos(r)
        //                   = tan(r)
        // -->             r = atan( m[X][Y] / m[Y][Y] )
        
        // m[Z][X] / m[Z][Z] = cos(p)sin(h) / cos(p)cos(h)
        //                   = sin(h) / cos(h)
        //                   = tan(h)
        // -->             h = atan( m[Z][X] / m[Z][Z] )
        
        const F32 tanRollSin( m[0][1] );
        const F32 tanRollCos( m[1][1] );
        const F32 tanHeadingSin( m[2][0] );
        const F32 tanHeadingCos( m[2][2] );
        
        h = atan2( tanHeadingSin, tanHeadingCos );
        r = atan2( tanRollSin, tanRollCos );
    }
    else
    {
        // sin(p) ~= 1 therefore cos(p) ~= 0
        // m[Y][X] / m[Y][Z] = -sin(r)cos(h) + cos(r)sin(p)sin(h) / sin(r)sin(h) + cos(r)sin(p)cos(h)
        //                   = sin(h)cos(r) - cos(h)sin(r)        / cos(h)cos(r) + sin(h)sin(r)
        // using    sin(x-y) = sin(x)cos(y) - cos(x)sin(y)
        // and      cos(x-y) = cos(x)cos(y) + sin(x)sin(y)
        //                   = sin(h-r)                           / cos(h-r)
        // [ at poles h equivalent to r therefore assume r = 0 ]
        //                   = sin(h) / cos(h)
        //                 h = atan( sin(h) / cos(h) )
        //                 h = atan( m[Y][X] / m[Y][Z]
        
        const F32 tanHeadingSin( m[1][0] );
        const F32 tanHeadingCos( m[1][2] );
        
        h = atan2( tanHeadingSin, tanHeadingCos );
        r = 0.0f;
    }
} 

/// Constructor - Rotation matrix from euler angles - Roll-Pitch-Heading
inline MMatrix44 MatrixFromRotationRPH(const F32 r, const F32 p, const F32 h)
{
    // Matrix RPH =
    //            [col][X]                               [col][Y]        [col][Z]
    // [X][row](  cos(r)*cos(h) + sin(r)*sin(p)*sin(h),  sin(r)*cos(p), -cos(r)*sin(h) + sin(r)*sin(p)*cos(h) );
    // [Y][row]( -sin(r)*cos(h) + cos(r)*sin(p)*sin(h),  cos(r)*cos(p),  sin(r)*sin(h) + cos(r)*sin(p)*cos(h) );
    // [Z][row](  cos(p)*sin(h),                        -sin(p),         cos(p)*cos(h) );
    
   
    const F32 cr( cos( r ) );
    const F32 sr( sin( r ) );
    const F32 cp( cos( p ) );
    const F32 sp( sin( p ) );
    const F32 ch( cos( h ) );
    const F32 sh( sin( h ) );
    
    const MVector3F ax(  cr*ch + sr*sp*sh,  sr*cp, -cr*sh + sr*sp*ch );
    const MVector3F ay( -sr*ch + cr*sp*sh,  cr*cp,  sr*sh + cr*sp*ch );
    const MVector3F az(  cp*sh,            -sp,     cp*ch );
    
	MMatrix44 out;
	out[0][0] = ax[0];
	out[0][1] = ax[1];
	out[0][2] = ax[2];
	
	out[1][0] = ay[0];
	out[1][1] = ay[1];
	out[1][2] = ay[2];
	
	out[2][0] = az[0];
	out[2][1] = az[1];
	out[2][2] = az[2];

	out[0][3] = 0.f;
	out[1][3] = 0.f;
	out[2][3] = 0.f;
	out[3][3] = 1.f;
	return out;
}

inline void GetSRT(
	F32& sx,
	F32& sy,
	F32& sz,
	F32& rr,
	F32& rp,
	F32& rh,
	F32& tx,
	F32& ty,
	F32& tz,
	const MMatrix44 &m)
{
	tx = m[3][0];
	ty = m[3][1];
	tz = m[3][2];
	
	MMatrix44 scaleRotate(m);
	scaleRotate[3][0] = 0;
	scaleRotate[3][1] = 0;
	scaleRotate[3][2] = 0;
	scaleRotate[3][3] = 1.f;
	{
		MMatrix44 tmp = NormalisedZY(scaleRotate);
		GetRotationRPH(rr, rp, rh, tmp);
	}

	MMatrix44 rotate = MatrixFromRotationRPH(rr, rp, rh);

	MMatrix44 rotateT(rotate);
	UtMaths::Transpose(&rotateT);
	MMatrix44 scale(scaleRotate * rotateT);

	sx = UtMaths::Magnitude(scale[0]);
	sy = UtMaths::Magnitude(scale[1]);
	sz = UtMaths::Magnitude(scale[2]);
}

inline void GetSRTS(
	F32& sx,
	F32& sy,
	F32& sz,
	const MMatrix44 &m)
{
	F32 rr;
	F32 rp;
	F32 rh;
	MMatrix44 scaleRotate(m);
	scaleRotate[3][0] = 0;
	scaleRotate[3][1] = 0;
	scaleRotate[3][2] = 0;
	scaleRotate[3][3] = 1.f;
	{
		MMatrix44 tmp = NormalisedZY(scaleRotate);
		GetRotationRPH(rr, rp, rh, tmp);
	}

	MMatrix44 rotate = MatrixFromRotationRPH(rr, rp, rh);

	MMatrix44 rotateT(rotate);
	UtMaths::Transpose(&rotateT);
	MMatrix44 scale(scaleRotate * rotateT);

	sx = UtMaths::Magnitude(scale[0]);
	sy = UtMaths::Magnitude(scale[1]);
	sz = UtMaths::Magnitude(scale[2]);
}

inline void GetSRTR(
	F32& rr,
	F32& rp,
	F32& rh,
	const MMatrix44 &m)
{
	MMatrix44 scaleRotate(m);
	scaleRotate[3][0] = 0;
	scaleRotate[3][1] = 0;
	scaleRotate[3][2] = 0;
	scaleRotate[3][3] = 1.f;
	{
		MMatrix44 tmp = NormalisedZY(scaleRotate);
		GetRotationRPH(rr, rp, rh, tmp);
	}
}

inline void GetSRTT(
	F32& tx,
	F32& ty,
	F32& tz,
	const MMatrix44 &m)
{
	tx = m[3][0];
	ty = m[3][1];
	tz = m[3][2];
}

}

#endif
