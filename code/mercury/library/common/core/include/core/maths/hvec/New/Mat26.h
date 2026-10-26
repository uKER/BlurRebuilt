#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT26_H
#define SOFT_MATHS_HVEC_NEW_MAT26_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT26_H
#define MATHS_HVEC_NEW_MAT26_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat26
/// @{

/// Make Mat26 from 3 Vector6 rows
NOALIAS __forceinline const Mat26 MakeMat26( const Vector6& row0, const Vector6& row1 );


/// Mat26 constants
template <> struct Const< Mat26 > 
{
    NOALIAS __forceinline static const Mat26 Zero(); ///< Constant Mat26 with all elements zeroed
};


/// Mat26 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat26& src );

/// Mat26 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat26* const __restrict pDst, const VF32 VECREF e );


/// Mat26 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat26& src );

/// Mat26 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat26* const __restrict pDst, const Vector6& row );


/// Mat26 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector2 Col( const Mat26& src );

/// Mat26 Accessor: Set column
template < unsigned int C >
NOALIAS __forceinline void SetCol( Mat26* const __restrict pDst, const Vector2 PCREF col );


/// Mat26 Transpose
NOALIAS __forceinline const Mat62 Transpose( const Mat26& src );


/// Mat26 Negation Operator
NOALIAS __forceinline const Mat26 operator - ( const Mat26& a );

/// Mat26 Addition Operator
NOALIAS __forceinline const Mat26 operator + ( const Mat26& a, const Mat26& b );

/// Mat26 Subtraction Operator
NOALIAS __forceinline const Mat26 operator - ( const Mat26& a, const Mat26& b );

/// Mat26 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat26& a, const Mat26& b );

/// Mat26 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat26& a, const Mat26& b );


/// Mat26 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector2 operator * ( const Mat26& a, const Vector6& b );

/// Vector2 * Mat26 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector2 PCREF a, const Mat26& b );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


