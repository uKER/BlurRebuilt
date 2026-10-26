#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC12F32_H
#define SOFT_MATHS_HVEC_NEW_VEC12F32_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC12F32_H
#define MATHS_HVEC_NEW_VEC12F32_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vec12F32
/// @{

/// Load Vec12F32 from F32 ptr
template <> NOALIAS __forceinline const Vec12F32 Load( F32 const * const __restrict pSrc );

/// Store Vec12F32 into F32 ptr
NOALIAS __forceinline void Store( const Vec12F32& data, F32 * const __restrict pDst );


/// Load Vec12F32 from Float9
template <> NOALIAS __forceinline const Vec12F32 Load( MathsStorage::Float9 const * const __restrict pSrc );

/// Store Vec12F32 into Float9
NOALIAS __forceinline void Store( const Vec12F32& data, MathsStorage::Float9 * const __restrict pDst );


/// Load Vec12F32 from Float10
template <> NOALIAS __forceinline const Vec12F32 Load( MathsStorage::Float10 const * const __restrict pSrc );

/// Store Vec12F32 into Float10
NOALIAS __forceinline void Store( const Vec12F32& data, MathsStorage::Float10 * const __restrict pDst );


/// Load Vec12F32 from Float11
template <> NOALIAS __forceinline const Vec12F32 Load( MathsStorage::Float11 const * const __restrict pSrc );

/// Store Vec12F32 into Float11
NOALIAS __forceinline void Store( const Vec12F32& data, MathsStorage::Float11 * const __restrict pDst );


/// Load Vec12F32 from Float12
template <> NOALIAS __forceinline const Vec12F32 Load( MathsStorage::Float12 const * const __restrict pSrc );

/// Store Vec12F32 into Float12
NOALIAS __forceinline void Store( const Vec12F32& data, MathsStorage::Float12 * const __restrict pDst );


/// Load Vec12F32 from StorageVec9F32
template <> NOALIAS __forceinline const Vec12F32 Load( StorageVec9F32 const * const __restrict pSrc );

/// Store Vec12F32 into StorageVec9F32
NOALIAS __forceinline void Store( const Vec12F32& data, StorageVec9F32 * const __restrict pDst );


/// Load Vec12F32 from StorageVec10F32
template <> NOALIAS __forceinline const Vec12F32 Load( StorageVec10F32 const * const __restrict pSrc );

/// Store Vec12F32 into StorageVec10F32
NOALIAS __forceinline void Store( const Vec12F32& data, StorageVec10F32 * const __restrict pDst );


/// Load Vec12F32 from StorageVec11F32
template <> NOALIAS __forceinline const Vec12F32 Load( StorageVec11F32 const * const __restrict pSrc );

/// Store Vec12F32 into StorageVec11F32
NOALIAS __forceinline void Store( const Vec12F32& data, StorageVec11F32 * const __restrict pDst );


/// Load Vec12F32 from StorageVec12F32
template <> NOALIAS __forceinline const Vec12F32 Load( StorageVec12F32 const * const __restrict pSrc );

/// Store Vec12F32 into StorageVec12F32
NOALIAS __forceinline void Store( const Vec12F32& data, StorageVec12F32 * const __restrict pDst );


/// Make Vec12F32 from a VF32 with the same value broadcast into all elements
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0 );

/// Make Vec12F32 from 9 VF32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                   const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6, const VF32 VECREF e7,
                                                   const VF32 VECREF e8 );

/// Make Vec12F32 from 10 VF32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                   const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6, const VF32 VECREF e7,
                                                   const VF32 VECREF e8, const VF32 VECREF e9 );

/// Make Vec12F32 from 11 VF32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                   const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6, const VF32 VECREF e7,
                                                   const VF32 VECREF e8, const VF32 VECREF e9, const VF32 VECREF e10 );

/// Make Vec12F32 from 12 VF32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2,  const VF32 VECREF e3,
                                                   const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6,  const VF32 VECREF e7,
                                                   const VF32 VECREF e8, const VF32 VECREF e9, const VF32 VECREF e10, const VF32 VECREF e11 );


/// Make Vec12F32 from 3 Vec4F32's
NOALIAS __forceinline const Vec12F32 MakeVec12F32( const Vec4F32 VECREF v0123, const Vec4F32 VECREF v4567, const Vec4F32 VECREF v89AB );

/// Split Vec12F32 into 3 Vec4F32's
NOALIAS __forceinline void Split( const Vec12F32& src, Vec4F32* const __restrict pV0123, Vec4F32* const __restrict pV4567, Vec4F32* const __restrict pV89AB );


/// Vec12F32 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vec12F32& v );

/// Vec12F32 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vec12F32* const __restrict pDst, const VF32 VECREF ei );


/// Vec12F32 Write to Stream
template < typename T_Stream >
void Serialise( const Vec12F32& src, T_Stream& stream );

/// Vec12F32 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, Vec12F32* const __restrict pDst );


/// Vec12F32 Permute : Indices select 12 result elements [I0,I11] from [ 0:v0, 11:v11 ]
//template < unsigned int I0, unsigned int I1, unsigned int I2,  unsigned int I3,
//           unsigned int I4, unsigned int I5, unsigned int I6,  unsigned int I7,
//           unsigned int I8, unsigned int I9, unsigned int I10, unsigned int I11 >
//NOALIAS __forceinline const Vec12F32 Permute( const Vec12F32& v );


/// Vec12F32 Abs of each element
NOALIAS __forceinline const Vec12F32 Abs( const Vec12F32& v );

/// Vec12F32 Sign of each element : For each x in v: returns -1.0f if x < 0.0f, +1.0f if x > 0.0f, or 0.0f if x == 0.0f 
NOALIAS __forceinline const Vec12F32 Sign( const Vec12F32& v );

/// Vec12F32 CopySign of each element : Returns a Vec12F32 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vec12F32 CopySign( const Vec12F32& magnitude, const Vec12F32& sign );

/// Vec12F32 Negation : Indices select elements to negate [I0,I11]
template < bool I0, bool I1, bool I2,  bool I3,
           bool I4, bool I5, bool I6,  bool I7,
           bool I8, bool I9, bool I10, bool I11 >
NOALIAS __forceinline const Vec12F32 Negate( const Vec12F32& v );


/// Vec12F32 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Rcp( const Vec12F32& v );

/// Vec12F32 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Rcp24( const Vec12F32& v );

/// Vec12F32 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Div( const Vec12F32& x, const Vec12F32& y );

/// Vec12F32 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Div24( const Vec12F32& x, const Vec12F32& y );

/// Vec12F32 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Div( const VF32 VECREF x, const Vec12F32& y );

/// Vec12F32 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Div24( const VF32 VECREF x, const Vec12F32& y );

/// Vec12F32 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Div( const Vec12F32& x, const VF32 VECREF y );

/// Vec12F32 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Div24( const Vec12F32& x, const VF32 VECREF y );

/// Vec12F32 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 RcpSqrt( const Vec12F32& v );

/// Vec12F32 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 RcpSqrt24( const Vec12F32& v );

/// Vec12F32 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec12F32 Sqrt( const Vec12F32& v );

/// Vec12F32 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec12F32 Sqrt24( const Vec12F32& v );

/// Vec12F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vec12F32& v,
                                       Vec12F32* const __restrict pRcp,
                                       Vec12F32* const __restrict pSqrt,
                                       Vec12F32* const __restrict pRcpSqrt );

/// Vec12F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vec12F32& v,
                                         Vec12F32* const __restrict pRcp,
                                         Vec12F32* const __restrict pSqrt,
                                         Vec12F32* const __restrict pRcpSqrt );

/// Vec12F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vec12F32& v,
                                           Vec12F32* const __restrict pSqrt,
                                           Vec12F32* const __restrict pRcpSqrt );

/// Vec12F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vec12F32& v,
                                             Vec12F32* const __restrict pSqrt,
                                             Vec12F32* const __restrict pRcpSqrt );


/// Vec12F32 Unary Positive Operator
NOALIAS __forceinline const Vec12F32 operator + ( const Vec12F32& x );

/// Vec12F32 Unary Negation Operator
NOALIAS __forceinline const Vec12F32 operator - ( const Vec12F32& x );


/// Vec12F32 Addition Operator
NOALIAS __forceinline const Vec12F32 operator + ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Subtraction Operator
NOALIAS __forceinline const Vec12F32 operator - ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Multiplication Operator
NOALIAS __forceinline const Vec12F32 operator * ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Division Operator
NOALIAS __forceinline const Vec12F32 operator / ( const Vec12F32& a, const Vec12F32& b );


/// Vec12F32 Addition Update Operator
NOALIAS __forceinline void operator += ( Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Division Update Operator
NOALIAS __forceinline void operator /= ( Vec12F32& a, const Vec12F32& b );


/// Vec12F32 Scalar Multiplication: Vec12F32 * VF32
NOALIAS __forceinline const Vec12F32 operator * ( const Vec12F32& v, const VF32 VECREF s );

/// Vec12F32 Scalar Multiplication: VF32 * Vec12F32
NOALIAS __forceinline const Vec12F32 operator * ( const VF32 VECREF s, const Vec12F32& v );

/// Vec12F32 Scalar Division: Vec12F32 / VF32
NOALIAS __forceinline const Vec12F32 operator / ( const Vec12F32& v, const VF32 VECREF s );

/// Vec12F32 Scalar Division: VF32 / Vec12F32
NOALIAS __forceinline const Vec12F32 operator / ( const VF32 VECREF s, const Vec12F32& v );


/// Vec12F32 Scalar Multiplication Update: Vec12F32 *= VF32
NOALIAS __forceinline void operator *= ( Vec12F32& v, const VF32 VECREF s );

/// Vec12F32 Scalar Division Update: Vec12F32 /= VF32
NOALIAS __forceinline void operator /= ( Vec12F32& v, const VF32 VECREF s );


/// Vec12F32 MulAdd : a * b + c
NOALIAS __forceinline const Vec12F32 MulAdd( const Vec12F32& a, const Vec12F32& b, const Vec12F32& c );

/// Vec12F32 MulSub : a * b - c
NOALIAS __forceinline const Vec12F32 MulSub( const Vec12F32& a, const Vec12F32& b, const Vec12F32& c );

/// Vec12F32 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vec12F32 NegMulAdd( const Vec12F32& a, const Vec12F32& b, const Vec12F32& c );

/// Vec12F32 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vec12F32 NegMulSub( const Vec12F32& a, const Vec12F32& b, const Vec12F32& c );


/// Vec12F32 MulAdd : a * b + c : [ vector * scalar + vector ]
NOALIAS __forceinline const Vec12F32 MulAdd( const Vec12F32& a, const VF32 VECREF b, const Vec12F32& c );

/// Vec12F32 MulSub : a * b - c : [ vector * scalar - vector ]
NOALIAS __forceinline const Vec12F32 MulSub( const Vec12F32& a, const VF32 VECREF b, const Vec12F32& c );

/// Vec12F32 NegMulAdd : -( a * b + c ) : [ - ( vector * scalar + vector ) ]
NOALIAS __forceinline const Vec12F32 NegMulAdd( const Vec12F32& a, const VF32 VECREF b, const Vec12F32& c );

/// Vec12F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( vector * scalar - vector ) ]
NOALIAS __forceinline const Vec12F32 NegMulSub( const Vec12F32& a, const VF32 VECREF b, const Vec12F32& c );


/// Vec12F32 MulAdd : a * b + c : [ scalar * vector + vector ]
NOALIAS __forceinline const Vec12F32 MulAdd( const VF32 VECREF a, const Vec12F32& b, const Vec12F32& c );

/// Vec12F32 MulSub : a * b - c : [ scalar * vector - vector ]
NOALIAS __forceinline const Vec12F32 MulSub( const VF32 VECREF a, const Vec12F32& b, const Vec12F32& c );

/// Vec12F32 NegMulAdd : -( a * b + c ) : [ - ( scalar * vector + vector ) ]
NOALIAS __forceinline const Vec12F32 NegMulAdd( const VF32 VECREF a, const Vec12F32& b, const Vec12F32& c );

/// Vec12F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( scalar * vector - vector ) ]
NOALIAS __forceinline const Vec12F32 NegMulSub( const VF32 VECREF a, const Vec12F32& b, const Vec12F32& c );


/// Vec12F32 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vec12F32 Lerp( const Vec12F32& src, const Vec12F32& dst, const VF32 VECREF t );


/// Vec12F32 Min
NOALIAS __forceinline const Vec12F32 Min( const Vec12F32& x, const Vec12F32& y );

/// Vec12F32 Max
NOALIAS __forceinline const Vec12F32 Max( const Vec12F32& x, const Vec12F32& y );

/// Vec12F32 Min
NOALIAS __forceinline const Vec12F32 Min( const Vec12F32& x, const VF32 VECREF y );

/// Vec12F32 Max
NOALIAS __forceinline const Vec12F32 Max( const Vec12F32& x, const VF32 VECREF y );

/// Vec12F32 Min Update
DEPRECATED NOALIAS __forceinline const Vec12F32& MinEq( Vec12F32* const __restrict pX, const Vec12F32& y );

/// Vec12F32 Max Update
DEPRECATED NOALIAS __forceinline const Vec12F32& MaxEq( Vec12F32* const __restrict pX, const Vec12F32& y );

/// Vec12F32 Min
NOALIAS __forceinline void Min( Vec12F32* const __restrict pX, const Vec12F32& y );

/// Vec12F32 Max
NOALIAS __forceinline void Max( Vec12F32* const __restrict pX, const Vec12F32& y );


/// Vec12F32 Clamp
NOALIAS __forceinline const Vec12F32 Clamp( const Vec12F32& x, const Vec12F32& lo, const Vec12F32& hi );

/// Vec12F32 Clamp Update
DEPRECATED NOALIAS __forceinline const Vec12F32& ClampEq( Vec12F32* const __restrict pX, const Vec12F32& lo, const Vec12F32& hi );

/// Vec12F32 Clamp
NOALIAS __forceinline void Clamp( Vec12F32* const __restrict pX, const Vec12F32& lo, const Vec12F32& hi );


/// Vec12F32 Round to integer towards minus infinity
NOALIAS __forceinline const Vec12F32 Floor( const Vec12F32& x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec12F32 Round to integer towards plus infinity
NOALIAS __forceinline const Vec12F32 Ceil ( const Vec12F32& x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec12F32 Round to integer towards nearest
NOALIAS __forceinline const Vec12F32 Round( const Vec12F32& x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec12F32 Round to integer towards zero
NOALIAS __forceinline const Vec12F32 Trunc( const Vec12F32& x ) __attribute__(( /* pure, ? */ always_inline ));


/// Vec12F32 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w
NOALIAS __forceinline const VBool operator == ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w
NOALIAS __forceinline const VBool operator != ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Comparison a <= b : Returns Vec16Bool representing results of a.i <= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec16Bool operator <= ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Comparison a >= b : Returns Vec16Bool representing results of a.i >= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec16Bool operator >= ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Comparison a < b : Returns Vec16Bool representing results of a.i < b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec16Bool operator < ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Comparison a > b : Returns Vec16Bool representing results of a.i > b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec16Bool operator > ( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vec12F32& a, const Vec12F32& b );


/// Vec12F32 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vec12F32& a );

/// Vec12F32 Finite Test : true if a contains no elements that are NaN's and for each element i [x,y,z,w], abs(x.i) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vec12F32& a );


/// Vec12F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec12F32 Select( const VBool PCREF condition, const Vec12F32& result_if_true, const Vec12F32& result_if_false );

/// Vec12F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec12F32 Select( const Vec16Bool PCREF condition, const Vec12F32& result_if_true, const Vec12F32& result_if_false );


/// Vec12F32 Bitwise NOT: ~a
NOALIAS __forceinline const Vec12F32 NOT( const Vec12F32& a );

/// Vec12F32 Bitwise OR: a | b
NOALIAS __forceinline const Vec12F32 OR( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Bitwise AND: a & b
NOALIAS __forceinline const Vec12F32 AND( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const Vec12F32 XOR( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Bitwise NOR: ~(a | b)
NOALIAS __forceinline const Vec12F32 NOR( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Bitwise NAND: ~(a & b)
NOALIAS __forceinline const Vec12F32 NAND( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const Vec12F32 EQV( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Bitwise ORC: a | ~b
NOALIAS __forceinline const Vec12F32 ORC( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Bitwise ANDC: a & ~b
NOALIAS __forceinline const Vec12F32 ANDC( const Vec12F32& a, const Vec12F32& b );


/// Vec12F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec12F32 AND( const Vec12F32& a, const VBool PCREF b );

/// Vec12F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec12F32 ANDC( const Vec12F32& a, const VBool PCREF b );


/// Vec12F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec12F32 AND( const Vec12F32& a, const Vec16Bool PCREF b );

/// Vec12F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec12F32 ANDC( const Vec12F32& a, const Vec16Bool PCREF b );



/// Vec12F32 Dot Product 9d
NOALIAS __forceinline const VF32 Dot9( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Dot Product 10d
NOALIAS __forceinline const VF32 Dot10( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Dot Product 11d
NOALIAS __forceinline const VF32 Dot11( const Vec12F32& a, const Vec12F32& b );

/// Vec12F32 Dot Product 12d
NOALIAS __forceinline const VF32 Dot12( const Vec12F32& a, const Vec12F32& b );


/// Vec12F32 Flush To Zero (9d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushToZero9( const Vec12F32& v, const VF32 VECREF tolerance );

/// Vec12F32 Flush To Zero (10d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushToZero10( const Vec12F32& v, const VF32 VECREF tolerance );

/// Vec12F32 Flush To Zero (11d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushToZero11( const Vec12F32& v, const VF32 VECREF tolerance );

/// Vec12F32 Flush To Zero (12d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushToZero12( const Vec12F32& v, const VF32 VECREF tolerance );


/// Vec12F32 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vec12F32 FlushElementsToZero( const Vec12F32& value, const VF32 VECREF tolerance );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


