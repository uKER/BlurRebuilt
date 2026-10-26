#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT36_H
#define SOFT_MATHS_HVEC_NEW_MAT36_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT36_H
#define MATHS_HVEC_NEW_MAT36_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat36
/// @{

/// Make Mat36 from 3 Vector6 rows
NOALIAS __forceinline const Mat36 MakeMat36( const Vector6& row0, const Vector6& row1, const Vector6& row2 );

/// Make Mat36 from 2 concatenated Mat33's
NOALIAS __forceinline const Mat36 MakeMat36( const Mat33 MATREF a, const Mat33 MATREF b );


/// Mat36 constants
template <> struct Const< Mat36 > 
{
    NOALIAS __forceinline static const Mat36 Zero(); ///< Constant Mat36 with all elements zeroed
};


/// Mat36 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat36& src );

/// Mat36 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat36* const __restrict pDst, const VF32 VECREF e );


/// Mat36 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat36& src );

/// Mat36 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat36* const __restrict pDst, const Vector6& row );


/// Mat36 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector3 Col( const Mat36& src );


/// Mat36 Transpose
NOALIAS __forceinline const Mat63 Transpose( const Mat36& src );


/// Mat36 Negation Operator
NOALIAS __forceinline const Mat36 operator - ( const Mat36& a );

/// Mat36 Addition Operator
NOALIAS __forceinline const Mat36 operator + ( const Mat36& a, const Mat36& b );

/// Mat36 Subtraction Operator
NOALIAS __forceinline const Mat36 operator - ( const Mat36& a, const Mat36& b );

/// Mat36 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat36& a, const Mat36& b );

/// Mat36 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat36& a, const Mat36& b );


/// Mat36 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector3 operator * ( const Mat36& a, const Vector6& b );

/// Vector3 * Mat36 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector3 PCREF a, const Mat36& b );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


