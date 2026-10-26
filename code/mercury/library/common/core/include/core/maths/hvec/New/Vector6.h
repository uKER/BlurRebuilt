#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR6_H
#define SOFT_MATHS_HVEC_NEW_VECTOR6_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR6_H
#define MATHS_HVEC_NEW_VECTOR6_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector6
/// @{

/// Load Vector6 from F32 ptr
template <> NOALIAS __forceinline const Vector6 Load( F32 const * const __restrict pSrc );

/// Store Vector6 to F32 ptr
NOALIAS __forceinline void Store( const Vector6& data, F32 * const __restrict pDst );


/// Load Vector6 from StorageVector6
NOALIAS __forceinline const Vector6 Load( StorageVector6 const * const __restrict pSrc );

/// Store Vector6 in StorageVector6
NOALIAS __forceinline void Store( const Vector6& data, StorageVector6 * const __restrict pDst );


/// Make Vector6 from broadcast VF32
NOALIAS __forceinline const Vector6 MakeVector6( const VF32 VECREF src );

/// Make Vector6 from 6 VF32's
NOALIAS __forceinline const Vector6 MakeVector6( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4, const VF32 VECREF e5 );

/// Make Vector6 from concatenation of two Vector3's
NOALIAS __forceinline const Vector6 MakeVector6( const Vector3 PCREF v012, const Vector3 PCREF v345 );

/// Make Vector6 from concatenation of a Vector3 and a Normal3
NOALIAS __forceinline const Vector6 MakeVector6( const Vector3 PCREF v012, const Normal3 PCREF n345 );

/// Make Vector6 from concatenation of a Normal3 and a Vector3
NOALIAS __forceinline const Vector6 MakeVector6( const Normal3 PCREF n012, const Vector3 PCREF v345 );

/// Make Vector6 from concatenation of two Normal3's
NOALIAS __forceinline const Vector6 MakeVector6( const Normal3 PCREF n012, const Normal3 PCREF n345 );

/// Make Vector6 from a Vector5 (elements 0-4) & a VF32 (element 5)
NOALIAS __forceinline const Vector6 MakeVector6( const Vector5& e01234, const VF32 VECREF e5 );

/// Make Vector6 from a VF32 (element 0) & a Vector5 (elements 1-5)
NOALIAS __forceinline const Vector6 MakeVector6( const VF32 VECREF e0, const Vector5& e12345 );


/// Split Vector6 into two Vector3's
NOALIAS __forceinline void Split( const Vector6& v, Vector3* const __restrict pV012, Vector3* const __restrict pV345 );

/// Vector6 Extract low Vector3 (elements 0, 1 & 2)
NOALIAS __forceinline const Vector3 ExtractLoVector3( const Vector6& v );

/// Vector6 Extract high Vector3 (elements 3, 4 & 5)
NOALIAS __forceinline const Vector3 ExtractHiVector3( const Vector6& v );


/// Vector6 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector6& v );

/// Vector6 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector6* const __restrict pDst, const VF32 VECREF ei );


/// Vector6 Permute : Indices select 6 result elements [I0,I1,I2,I3,I4,I5] from [ 0:v0, 1:v1, 2:v2, 3:v3, 4:v4, 5:v5 ]
template < unsigned int I0, unsigned int I1, unsigned int I2, unsigned int I3,
           unsigned int I4, unsigned int I5 >
NOALIAS __forceinline const Vector6 Permute( const Vector6& v );


/// Vector6 Abs of each element
NOALIAS __forceinline const Vector6 Abs( const Vector6& v );

/// Vector6 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector6 Sign( const Vector6& v );

/// Vector6 CopySign of each element : Returns a Vector6 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector6 CopySign( const Vector6& magnitude, const Vector6& sign );


/// Vector6 Negation
NOALIAS __forceinline const Vector6 operator - ( const Vector6& a );

/// Vector6 Sum
NOALIAS __forceinline const Vector6 operator + ( const Vector6& a, const Vector6& b );

/// Vector6 Difference
NOALIAS __forceinline const Vector6 operator - ( const Vector6& a, const Vector6& b );

/// Vector6 Addition Update
NOALIAS __forceinline void operator += ( Vector6& a, const Vector6& b );

/// Vector6 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector6& a, const Vector6& b );


/// Vector6 Scalar Right Multiplication
NOALIAS __forceinline const Vector6 operator * ( const Vector6& a, const VF32 VECREF b );

/// Vector6 Scalar Left Multiplication
NOALIAS __forceinline const Vector6 operator * ( const VF32 VECREF a, const Vector6& b );

/// Vector6 Scalar Right Division
NOALIAS __forceinline const Vector6 operator / ( const Vector6& a, const VF32 VECREF b );

/// Vector6 Scalar Left Division
NOALIAS __forceinline const Vector6 operator / ( const VF32 VECREF a, const Vector6& b );

/// Vector6 Scalar Right Multiplication Update
NOALIAS __forceinline void operator *= ( Vector6& a, const VF32 VECREF b );

/// Vector6 Scalar Right Division Update
NOALIAS __forceinline void operator /= ( Vector6& a, const VF32 VECREF b );


/// Vector6 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector6& a, const Vector6& b );

/// Vector6 Division Operator
NOALIAS __forceinline const Vector6 operator / ( const Vector6& a, const Vector6& b );

/// Vector6 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector6& a, const Vector6& b );

/// Vector6 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector6& a, const Vector6& b );


/// Vector6 MulAdd: a * b + c
NOALIAS __forceinline const Vector6 MulAdd( const Vector6& a, const Vector6& b, const Vector6& c );

/// Vector6 MulAdd: a * b + c
NOALIAS __forceinline const Vector6 MulAdd( const Vector6& a, const VF32 VECREF b, const Vector6& c );

/// Vector6 MulAdd: a * b + c
NOALIAS __forceinline const Vector6 MulAdd( const VF32 VECREF a, const Vector6& b, const Vector6& c );


/// Vector6 MulSub: a * b - c
NOALIAS __forceinline const Vector6 MulSub( const Vector6& a, const Vector6& b, const Vector6& c );

/// Vector6 MulSub: a * b - c
NOALIAS __forceinline const Vector6 MulSub( const Vector6& a, const VF32 VECREF b, const Vector6& c );

/// Vector6 MulSub: a * b - c
NOALIAS __forceinline const Vector6 MulSub( const VF32 VECREF a, const Vector6& b, const Vector6& c );


/// Vector6 NegMulAdd: -( a * b + c )
NOALIAS __forceinline const Vector6 NegMulAdd( const Vector6& a, const Vector6& b, const Vector6& c );

/// Vector6 NegMulAdd: -( a * b + c )
NOALIAS __forceinline const Vector6 NegMulAdd( const Vector6& a, const VF32 VECREF b, const Vector6& c );

/// Vector6 NegMulAdd: -( a * b + c )
NOALIAS __forceinline const Vector6 NegMulAdd( const VF32 VECREF a, const Vector6& b, const Vector6& c );


/// Vector6 NegMulSub: -( a * b - c ) = c - a * b
NOALIAS __forceinline const Vector6 NegMulSub( const Vector6& a, const Vector6& b, const Vector6& c );

/// Vector6 NegMulSub: -( a * b - c ) = c - a * b
NOALIAS __forceinline const Vector6 NegMulSub( const Vector6& a, const VF32 VECREF b, const Vector6& c );

/// Vector6 NegMulSub: -( a * b - c ) = c - a * b
NOALIAS __forceinline const Vector6 NegMulSub( const VF32 VECREF a, const Vector6& b, const Vector6& c );


/// Vector6 Comparison a == b : Returns VBool representing results of a[0] == b[0] && ... && a[5] == b[5]
NOALIAS __forceinline const VBool operator == ( const Vector6& a, const Vector6& b );

/// Vector6 Comparison a != b : Returns VBool representing results of a[0] != b[0] || ... || a[5] != b[5]
NOALIAS __forceinline const VBool operator != ( const Vector6& a, const Vector6& b );

/// Vector6 Comparison a <= b : Returns Vec8Bool representing results of a[i] <= b[i], for each element i [0,5]
NOALIAS __forceinline const Vec8Bool operator <= ( const Vector6& a, const Vector6& b );

/// Vector6 Comparison a >= b : Returns Vec8Bool representing results of a[i] >= b[i], for each element i [0,5]
NOALIAS __forceinline const Vec8Bool operator >= ( const Vector6& a, const Vector6& b );

/// Vector6 Comparison a < b : Returns Vec8Bool representing results of a[i] < b[i], for each element i [0,5]
NOALIAS __forceinline const Vec8Bool operator < ( const Vector6& a, const Vector6& b );

/// Vector6 Comparison a > b : Returns Vec8Bool representing results of a[i] > b[i], for each element i [0,5]
NOALIAS __forceinline const Vec8Bool operator > ( const Vector6& a, const Vector6& b );

/// Vector6 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector6& a, const Vector6& b );

/// Vector6 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector6& a, const Vector6& b );


/// Vector6 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vector6& a );

/// Vector6 Finite Test : true if a contains no elements that are NaN's and for each element i [0,5], abs(x[i]) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vector6& a );


/// Vector6 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector6 Select( const VBool PCREF condition, const Vector6& result_if_true, const Vector6& result_if_false );

/// Vector6 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector6 Select( const Vec8Bool PCREF condition, const Vector6& result_if_true, const Vector6& result_if_false );


/// Vector6 Bitwise AND: a & b : for each element i, returns ( b ? a[i] : zero )
NOALIAS __forceinline const Vector6 AND( const Vector6& a, const VBool PCREF b );

/// Vector6 Bitwise ANDC: a & ~b : for each element i, returns ( b ? zero : a[i] )
NOALIAS __forceinline const Vector6 ANDC( const Vector6& a, const VBool PCREF b );


/// Vector6 Bitwise AND: a & b : for each element i, returns ( b[i] ? a[i] : zero )
NOALIAS __forceinline const Vector6 AND( const Vector6& a, const Vec8Bool PCREF b );

/// Vector6 Bitwise ANDC: a & ~b : for each element i, returns ( b[i] ? zero : a[i] )
NOALIAS __forceinline const Vector6 ANDC( const Vector6& a, const Vec8Bool PCREF b );


/// Vector6 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector6 Rcp( const Vector6& v );

/// Vector6 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector6 Rcp24( const Vector6& v );


/// Vector6 Dot Product
NOALIAS __forceinline const VF32 Dot( const Vector6& a, const Vector6& b );

/// Vector6 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector6& v );

/// Vector6 Length
NOALIAS __forceinline const VF32 Length( const Vector6& v );

/// Vector6 Inverse: Returns A^-1, parallel to A and Dot( A, A^-1 ) == 1
NOALIAS __forceinline const Vector6 Inverse( const Vector6& v );


/// Vector Space Tensor Product of a 6x1 column vector and a 1x6 row vector -> Mat66
NOALIAS __forceinline const Mat66 TensorProduct( const Vector6& a, const Vector6& b );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

