#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR5_H
#define SOFT_MATHS_HVEC_NEW_VECTOR5_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR5_H
#define MATHS_HVEC_NEW_VECTOR5_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector5
/// @{

/// Load Vector5 from F32 ptr
template <> NOALIAS __forceinline const Vector5 Load( F32 const * const __restrict pSrc );

/// Store Vector5 to F32 ptr
NOALIAS __forceinline void Store( const Vector5& data, F32 * const __restrict pDst );


/// Load Vector5 from StorageVector5
NOALIAS __forceinline const Vector5 Load( StorageVector5 const * const __restrict pSrc );

/// Store Vector5 in StorageVector5
NOALIAS __forceinline void Store( const Vector5& data, StorageVector5 * const __restrict pDst );


/// Make Vector5 from broadcast VF32
NOALIAS __forceinline const Vector5 MakeVector5( const VF32 VECREF src );

/// Make Vector5 from 5 VF32's
NOALIAS __forceinline const Vector5 MakeVector5( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4 );

/// Make Vector5 from a Vector6
NOALIAS __forceinline const Vector5 MakeVector5( const Vector6& v6 );

/// Make Vector5 from a Vector4 (elements 0-3) & a VF32 (element 4)
NOALIAS __forceinline const Vector5 MakeVector5( const Vector4 PCREF e0123, const VF32 VECREF e4 );

/// Make Vector5 from a VF32 (element 0) & a Vector4 (elements 1-4)
NOALIAS __forceinline const Vector5 MakeVector5( const VF32 VECREF e0, const Vector4 PCREF e1234 );

/// Make Vector5 from a Vector3 (elements 0-2) & two VF32's (elements 3 & 4)
NOALIAS __forceinline const Vector5 MakeVector5( const Vector3 PCREF e012, const VF32 VECREF e3, const VF32 VECREF e4 );


/// Vector5 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector5& v );

/// Vector5 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector5* const __restrict pDst, const VF32 VECREF ei );


/// Vector5 Permute : Indices select 5 result elements [I0,I1,I2,I3,I4] from [ 0:v0, 1:v1, 2:v2, 3:v3, 4:v4 ]
template < unsigned int I0, unsigned int I1, unsigned int I2, unsigned int I3,
           unsigned int I4 >
NOALIAS __forceinline const Vector5 Permute( const Vector5& v );


/// Vector5 Abs of each element
NOALIAS __forceinline const Vector5 Abs( const Vector5& v );

/// Vector5 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector5 Sign( const Vector5& v );

/// Vector5 CopySign of each element : Returns a Vector5 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector5 CopySign( const Vector5& magnitude, const Vector5& sign );


/// Vector5 Negation
NOALIAS __forceinline const Vector5 operator - ( const Vector5& a );

/// Vector5 Sum
NOALIAS __forceinline const Vector5 operator + ( const Vector5& a, const Vector5& b );

/// Vector5 Difference
NOALIAS __forceinline const Vector5 operator - ( const Vector5& a, const Vector5& b );

/// Vector5 Addition Update
NOALIAS __forceinline void operator += ( Vector5& a, const Vector5& b );

/// Vector5 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector5& a, const Vector5& b );


/// Vector5 Scalar Right Multiplication
NOALIAS __forceinline const Vector5 operator * ( const Vector5& a, const VF32 VECREF b );

/// Vector5 Scalar Left Multiplication
NOALIAS __forceinline const Vector5 operator * ( const VF32 VECREF a, const Vector5& b );

/// Vector5 Scalar Right Division
NOALIAS __forceinline const Vector5 operator / ( const Vector5& a, const VF32 VECREF b );

/// Vector5 Scalar Left Division
NOALIAS __forceinline const Vector5 operator / ( const VF32 VECREF a, const Vector5& b );

/// Vector5 Scalar Right Multiplication Update
NOALIAS __forceinline void operator *= ( Vector5& a, const VF32 VECREF b );

/// Vector5 Scalar Right Division Update
NOALIAS __forceinline void operator /= ( Vector5& a, const VF32 VECREF b );


/// Vector5 Multiplication Operator
NOALIAS __forceinline const Vector5 operator * ( const Vector5& a, const Vector5& b );

/// Vector5 Division Operator
NOALIAS __forceinline const Vector5 operator / ( const Vector5& a, const Vector5& b );

/// Vector5 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector5& a, const Vector5& b );

/// Vector5 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector5& a, const Vector5& b );


/// Vector5 Comparison a == b : Returns VBool representing results of a[0] == b[0] && ... && a[4] == b[4]
NOALIAS __forceinline const VBool operator == ( const Vector5& a, const Vector5& b );

/// Vector5 Comparison a != b : Returns VBool representing results of a[0] != b[0] || ... || a[4] != b[4]
NOALIAS __forceinline const VBool operator != ( const Vector5& a, const Vector5& b );

/// Vector5 Comparison a <= b : Returns Vec8Bool representing results of a[i] <= b[i], for each element i [0,4]
NOALIAS __forceinline const Vec8Bool operator <= ( const Vector5& a, const Vector5& b );

/// Vector5 Comparison a >= b : Returns Vec8Bool representing results of a[i] >= b[i], for each element i [0,4]
NOALIAS __forceinline const Vec8Bool operator >= ( const Vector5& a, const Vector5& b );

/// Vector5 Comparison a < b : Returns Vec8Bool representing results of a[i] < b[i], for each element i [0,4]
NOALIAS __forceinline const Vec8Bool operator < ( const Vector5& a, const Vector5& b );

/// Vector5 Comparison a > b : Returns Vec8Bool representing results of a[i] > b[i], for each element i [0,4]
NOALIAS __forceinline const Vec8Bool operator > ( const Vector5& a, const Vector5& b );

/// Vector5 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector5& a, const Vector5& b );

/// Vector5 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector5& a, const Vector5& b );


/// Vector5 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vector5& a );

/// Vector5 Finite Test : true if a contains no elements that are NaN's and for each element i [0,4], abs(x[i]) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vector5& a );


/// Vector5 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector5 Select( const VBool PCREF condition, const Vector5& result_if_true, const Vector5& result_if_false );

/// Vector5 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector5 Select( const Vec8Bool PCREF condition, const Vector5& result_if_true, const Vector5& result_if_false );


/// Vector5 Bitwise AND: a & b : for each element i, returns ( b ? a[i] : zero )
NOALIAS __forceinline const Vector5 AND( const Vector5& a, const VBool PCREF b );

/// Vector5 Bitwise ANDC: a & ~b : for each element i, returns ( b ? zero : a[i] )
NOALIAS __forceinline const Vector5 ANDC( const Vector5& a, const VBool PCREF b );


/// Vector5 Bitwise AND: a & b : for each element i, returns ( b[i] ? a[i] : zero )
NOALIAS __forceinline const Vector5 AND( const Vector5& a, const Vec8Bool PCREF b );

/// Vector5 Bitwise ANDC: a & ~b : for each element i, returns ( b[i] ? zero : a[i] )
NOALIAS __forceinline const Vector5 ANDC( const Vector5& a, const Vec8Bool PCREF b );


/// Vector5 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector5 Rcp( const Vector5& v );

/// Vector5 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector5 Rcp24( const Vector5& v );


/// Vector5 Dot Product
NOALIAS __forceinline const VF32 Dot( const Vector5& a, const Vector5& b );

/// Vector5 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Vector5& a, const Vector5& b );

/// Vector5 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector5& v );

/// Vector5 Length
NOALIAS __forceinline const VF32 Length( const Vector5& v );

/// Vector5 Inverse: Returns A^-1, parallel to A and Dot( A, A^-1 ) == 1
NOALIAS __forceinline const Vector5 Inverse( const Vector5& v );


/// Vector Space Tensor Product of a 5x1 column vector and a 1x5 row vector -> Mat55
NOALIAS __forceinline const Mat55 TensorProduct( const Vector5& a, const Vector5& b );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

