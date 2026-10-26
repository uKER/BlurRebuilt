#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT46_H
#define SOFT_MATHS_HVEC_NEW_MAT46_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT46_H
#define MATHS_HVEC_NEW_MAT46_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat46
/// @{

/// Make Mat46 from 4 Vector6 rows
NOALIAS __forceinline const Mat46 MakeMat46( const Vector6& row0, const Vector6& row1, const Vector6& row2, const Vector6& row3 );

/// Mat46 constants
template <> struct Const< Mat46 > 
{
    NOALIAS __forceinline static const Mat46 Zero(); ///< Constant Mat46 with all elements zeroed
};


/// Mat46 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat46& src );

/// Mat46 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat46* const __restrict pDst, const VF32 VECREF e );


/// Mat46 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat46& src );

/// Mat46 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat46* const __restrict pDst, const Vector6& row );


/// Mat46 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector4 Col( const Mat46& src );


/// Mat46 Transpose
NOALIAS __forceinline const Mat64 Transpose( const Mat46& src );


/// Mat46 Negation Operator
NOALIAS __forceinline const Mat46 operator - ( const Mat46& a );

/// Mat46 Addition Operator
NOALIAS __forceinline const Mat46 operator + ( const Mat46& a, const Mat46& b );

/// Mat46 Subtraction Operator
NOALIAS __forceinline const Mat46 operator - ( const Mat46& a, const Mat46& b );

/// Mat46 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat46& a, const Mat46& b );

/// Mat46 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat46& a, const Mat46& b );


/// Mat46 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector4 operator * ( const Mat46& a, const Vector6& b );

/// Vector4 * Mat46 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector4 PCREF a, const Mat46& b );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


