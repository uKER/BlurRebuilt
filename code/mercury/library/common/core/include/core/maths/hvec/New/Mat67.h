#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT67_H
#define SOFT_MATHS_HVEC_NEW_MAT67_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT67_H
#define MATHS_HVEC_NEW_MAT67_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat67
/// @{

/// Make Mat67 from 6 Vector7 rows
NOALIAS __forceinline const Mat67 MakeMat67( const Vector7& row0, const Vector7& row1, const Vector7& row2, const Vector7& row3,
                                             const Vector7& row4, const Vector7& row5 );

/// Mat67 constants
template <> struct Const< Mat67 > 
{
    NOALIAS __forceinline static const Mat67 Zero(); ///< Constant Mat67 with all elements zeroed
};


/// Mat67 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat67& src );

/// Mat67 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat67* const __restrict pDst, const VF32 VECREF e );


/// Mat67 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector7 Row( const Mat67& src );

/// Mat67 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat67* const __restrict pDst, const Vector7& row );


/// Mat67 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat67& src );


/// Mat67 Negation Operator
NOALIAS __forceinline const Mat67 operator - ( const Mat67& a );

/// Mat67 Addition Operator
NOALIAS __forceinline const Mat67 operator + ( const Mat67& a, const Mat67& b );

/// Mat67 Subtraction Operator
NOALIAS __forceinline const Mat67 operator - ( const Mat67& a, const Mat67& b );

/// Mat67 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat67& a, const Mat67& b );

/// Mat67 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat67& a, const Mat67& b );


/// Mat67 * Vector7 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat67& m, const Vector7& v );


/// Mat67 * Mat76 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat67& a, const Mat76& b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


