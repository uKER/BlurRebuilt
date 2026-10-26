#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT63_H
#define SOFT_MATHS_HVEC_NEW_MAT63_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT63_H
#define MATHS_HVEC_NEW_MAT63_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat63
/// @{

/// Make Mat63 from 6 Vector3 rows
NOALIAS __forceinline const Mat63 MakeMat63( const Vector3 PCREF row0, const Vector3 PCREF row1, const Vector3 PCREF row2, const Vector3 PCREF row3,
                                             const Vector3 PCREF row4, const Vector3 PCREF row5 );

/// Mat63 constants
template <> struct Const< Mat63 > 
{
    NOALIAS __forceinline static const Mat63 Zero(); ///< Constant Mat63 with all elements zeroed
};


/// Mat63 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat63& src );

/// Mat63 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat63* const __restrict pDst, const VF32 VECREF e );


/// Mat63 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector3 Row( const Mat63& src );

/// Mat63 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat63* const __restrict pDst, const Vector3 PCREF row );


/// Mat63 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat63& src );


/// Mat63 Transpose
NOALIAS __forceinline const Mat36 Transpose( const Mat63& src );


/// Mat63 Negation Operator
NOALIAS __forceinline const Mat63 operator - ( const Mat63& a );

/// Mat63 Addition Operator
NOALIAS __forceinline const Mat63 operator + ( const Mat63& a, const Mat63& b );

/// Mat63 Subtraction Operator
NOALIAS __forceinline const Mat63 operator - ( const Mat63& a, const Mat63& b );

/// Mat63 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat63& a, const Mat63& b );

/// Mat63 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat63& a, const Mat63& b );


/// Mat63 * Vector3 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat63& a, const Vector3 PCREF b );

/// Vector6 * Mat63 Multiplication Operator
NOALIAS __forceinline const Vector3 operator * ( const Vector6& a, const Mat63& b );


/// Mat63 * Mat36 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat63& a, const Mat36& b );

/// Mat36 * Mat63 Diagonal Multiply: Returns the diagonal of the resulting 3x3 matrix
NOALIAS __forceinline const Vector3 DiagonalMultiply( const Mat36& a, const Mat63& b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


