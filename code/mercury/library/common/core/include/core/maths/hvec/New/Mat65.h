#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT65_H
#define SOFT_MATHS_HVEC_NEW_MAT65_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT65_H
#define MATHS_HVEC_NEW_MAT65_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat65
/// @{

/// Make Mat65 from 6 Vector5 rows
NOALIAS __forceinline const Mat65 MakeMat65( const Vector5& row0, const Vector5& row1, const Vector5& row2, const Vector5& row3,
                                             const Vector5& row4, const Vector5& row5 );

/// Mat65 constants
template <> struct Const< Mat65 > 
{
    NOALIAS __forceinline static const Mat65 Zero(); ///< Constant Mat65 with all elements zeroed
};


/// Mat65 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat65& src );

/// Mat65 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat65* const __restrict pDst, const VF32 VECREF e );


/// Mat65 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector5 Row( const Mat65& src );

/// Mat65 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat65* const __restrict pDst, const Vector5& row );


/// Mat65 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat65& src );


/// Mat65 Negation Operator
NOALIAS __forceinline const Mat65 operator - ( const Mat65& a );

/// Mat65 Addition Operator
NOALIAS __forceinline const Mat65 operator + ( const Mat65& a, const Mat65& b );

/// Mat65 Subtraction Operator
NOALIAS __forceinline const Mat65 operator - ( const Mat65& a, const Mat65& b );

/// Mat65 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat65& a, const Mat65& b );

/// Mat65 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat65& a, const Mat65& b );


/// Mat65 * Vector5 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat65& m, const Vector5& v );


/// Mat65 * Mat56 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat65& a, const Mat56& b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


