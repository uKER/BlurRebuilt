#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT76_H
#define SOFT_MATHS_HVEC_NEW_MAT76_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT76_H
#define MATHS_HVEC_NEW_MAT76_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat76
/// @{

/// Make Mat76 from 7 Vector6 rows
NOALIAS __forceinline const Mat76 MakeMat76( const Vector6& row0, const Vector6& row1, const Vector6& row2, const Vector6& row3,
                                             const Vector6& row4, const Vector6& row5, const Vector6& row6 );

/// Mat76 constants
template <> struct Const< Mat76 > 
{
    NOALIAS __forceinline static const Mat76 Zero(); ///< Constant Mat76 with all elements zeroed
};


/// Mat76 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat76& src );

/// Mat76 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat76* const __restrict pDst, const VF32 VECREF e );


/// Mat76 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat76& src );

/// Mat76 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat76* const __restrict pDst, const Vector6& row );


/// Mat76 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector7 Col( const Mat76& src );


/// Mat76 Negation Operator
NOALIAS __forceinline const Mat76 operator - ( const Mat76& a );

/// Mat76 Addition Operator
NOALIAS __forceinline const Mat76 operator + ( const Mat76& a, const Mat76& b );

/// Mat76 Subtraction Operator
NOALIAS __forceinline const Mat76 operator - ( const Mat76& a, const Mat76& b );

/// Mat76 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat76& a, const Mat76& b );

/// Mat76 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat76& a, const Mat76& b );


/// Mat76 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector7 operator * ( const Mat76& m, const Vector6& v );


/// Mat76 * Mat67 Diagonal Multiply: Returns the diagonal of the resulting 7x7 matrix
NOALIAS __forceinline const Vector7 DiagonalMultiply( const Mat76& a, const Mat67& b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


