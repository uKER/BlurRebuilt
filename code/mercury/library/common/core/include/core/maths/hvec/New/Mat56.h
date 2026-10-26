#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT56_H
#define SOFT_MATHS_HVEC_NEW_MAT56_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT56_H
#define MATHS_HVEC_NEW_MAT56_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat56
/// @{

/// Make Mat56 from 5 Vector6 rows
NOALIAS __forceinline const Mat56 MakeMat56( const Vector6& row0, const Vector6& row1, const Vector6& row2, const Vector6& row3,
                                             const Vector6& row4 );

/// Mat56 constants
template <> struct Const< Mat56 > 
{
    NOALIAS __forceinline static const Mat56 Zero(); ///< Constant Mat56 with all elements zeroed
};


/// Mat56 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat56& src );

/// Mat56 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat56* const __restrict pDst, const VF32 VECREF e );


/// Mat56 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat56& src );

/// Mat56 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat56* const __restrict pDst, const Vector6& row );


/// Mat56 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector5 Col( const Mat56& src );


/// Mat56 Negation Operator
NOALIAS __forceinline const Mat56 operator - ( const Mat56& a );

/// Mat56 Addition Operator
NOALIAS __forceinline const Mat56 operator + ( const Mat56& a, const Mat56& b );

/// Mat56 Subtraction Operator
NOALIAS __forceinline const Mat56 operator - ( const Mat56& a, const Mat56& b );

/// Mat56 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat56& a, const Mat56& b );

/// Mat56 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat56& a, const Mat56& b );


/// Mat56 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector5 operator * ( const Mat56& m, const Vector6& v );


/// Mat56 * Mat65 Diagonal Multiply: Returns the diagonal of the resulting 5x5 matrix
NOALIAS __forceinline const Vector5 DiagonalMultiply( const Mat56& a, const Mat65& b );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


