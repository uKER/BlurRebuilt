#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT62_H
#define SOFT_MATHS_HVEC_NEW_MAT62_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT62_H
#define MATHS_HVEC_NEW_MAT62_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat62
/// @{

/// Make Mat62 from 6 Vector2 rows
NOALIAS __forceinline const Mat62 MakeMat62( const Vector2 PCREF row0, const Vector2 PCREF row1, const Vector2 PCREF row2, const Vector2 PCREF row3,
                                             const Vector2 PCREF row4, const Vector2 PCREF row5 );

/// Mat62 constants
template <> struct Const< Mat62 > 
{
    NOALIAS __forceinline static const Mat62 Zero(); ///< Constant Mat62 with all elements zeroed
};


/// Mat62 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat62& src );

/// Mat62 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat62* const __restrict pDst, const VF32 VECREF e );


/// Mat62 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector2 Row( const Mat62& src );

/// Mat62 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat62* const __restrict pDst, const Vector2 PCREF row );


/// Mat62 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat62& src );

/// Mat62 Accessor: Set column
template < unsigned int C >
NOALIAS __forceinline void SetCol( Mat62* const __restrict pDst, const Vector6 PCREF col );


/// Mat62 Transpose
NOALIAS __forceinline const Mat26 Transpose( const Mat62& src );


/// Mat62 Negation Operator
NOALIAS __forceinline const Mat62 operator - ( const Mat62& a );

/// Mat62 Addition Operator
NOALIAS __forceinline const Mat62 operator + ( const Mat62& a, const Mat62& b );

/// Mat62 Subtraction Operator
NOALIAS __forceinline const Mat62 operator - ( const Mat62& a, const Mat62& b );

/// Mat62 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat62& a, const Mat62& b );

/// Mat62 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat62& a, const Mat62& b );


/// Mat62 * Vector2 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat62& a, const Vector2& b );

/// Vector6 * Mat62 Multiplication Operator
NOALIAS __forceinline const Vector2 operator * ( const Vector6& a, const Mat62& b );


/// Mat62 * Mat26 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat62& a, const Mat26& b );

/// Mat26 * Mat62 Diagonal Multiply: Returns the diagonal of the resulting 2x2 matrix
NOALIAS __forceinline const Vector2 DiagonalMultiply( const Mat26& a, const Mat62& b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


