#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT64_H
#define SOFT_MATHS_HVEC_NEW_MAT64_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT64_H
#define MATHS_HVEC_NEW_MAT64_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat64
/// @{

/// Make Mat64 from 6 Vector4 rows
NOALIAS __forceinline const Mat64 MakeMat64( const Vector4 PCREF row0, const Vector4 PCREF row1, const Vector4 PCREF row2, const Vector4 PCREF row3,
                                             const Vector4 PCREF row4, const Vector4 PCREF row5 );

/// Mat64 constants
template <> struct Const< Mat64 > 
{
    NOALIAS __forceinline static const Mat64 Zero(); ///< Constant Mat64 with all elements zeroed
};


/// Mat64 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat64& src );

/// Mat64 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat64* const __restrict pDst, const VF32 VECREF e );


/// Mat64 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector4 Row( const Mat64& src );

/// Mat64 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat64* const __restrict pDst, const Vector4 PCREF row );


/// Mat64 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat64& src );


/// Mat64 Transpose
NOALIAS __forceinline const Mat46 Transpose( const Mat64& src );


/// Mat64 Negation Operator
NOALIAS __forceinline const Mat64 operator - ( const Mat64& a );

/// Mat64 Addition Operator
NOALIAS __forceinline const Mat64 operator + ( const Mat64& a, const Mat64& b );

/// Mat64 Subtraction Operator
NOALIAS __forceinline const Mat64 operator - ( const Mat64& a, const Mat64& b );

/// Mat64 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat64& a, const Mat64& b );

/// Mat64 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat64& a, const Mat64& b );


/// Mat64 * Vector4 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat64& a, const Vector4 PCREF b );

/// Vector6 * Mat64 Multiplication Operator
NOALIAS __forceinline const Vector4 operator * ( const Vector6& a, const Mat64& b );


/// Mat64 * Mat46 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat64& a, const Mat46& b );

/// Mat46 * Mat64 Diagonal Multiply: Returns the diagonal of the resulting 4x4 matrix
NOALIAS __forceinline const Vector4 DiagonalMultiply( const Mat46& a, const Mat64& b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


