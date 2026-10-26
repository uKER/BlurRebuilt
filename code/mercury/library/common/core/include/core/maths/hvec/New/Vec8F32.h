#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC8F32_H
#define SOFT_MATHS_HVEC_NEW_VEC8F32_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC8F32_H
#define MATHS_HVEC_NEW_VEC8F32_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vec8F32
/// @{

/// Load Vec8F32 from F32 ptr
template <> NOALIAS __forceinline const Vec8F32 Load( F32 const * const __restrict pSrc );

/// Store Vec8F32 into F32 ptr
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, F32 * const __restrict pDst );


/// Load Vec8F32 from Float1
//template <> NOALIAS __forceinline const Vec8F32 Load( MathsStorage::Float1 const * const __restrict pSrc );

/// Store Vec8F32 into Float1
//NOALIAS __forceinline void Store( const Vec8F32 MATREF data, MathsStorage::Float1 * const __restrict pDst );


/// Load Vec8F32 from Float2
//template <> NOALIAS __forceinline const Vec8F32 Load( MathsStorage::Float2 const * const __restrict pSrc );

/// Store Vec8F32 into Float2
//NOALIAS __forceinline void Store( const Vec8F32 MATREF data, MathsStorage::Float2 * const __restrict pDst );


/// Load Vec8F32 from Float3
//template <> NOALIAS __forceinline const Vec8F32 Load( MathsStorage::Float3 const * const __restrict pSrc );

/// Store Vec8F32 into Float3
//NOALIAS __forceinline void Store( const Vec8F32 MATREF data, MathsStorage::Float3 * const __restrict pDst );


/// Load Vec8F32 from Float4
//template <> NOALIAS __forceinline const Vec8F32 Load( MathsStorage::Float4 const * const __restrict pSrc );

/// Store Vec8F32 into Float4
//NOALIAS __forceinline void Store( const Vec8F32 MATREF data, MathsStorage::Float4 * const __restrict pDst );


/// Load Vec8F32 from Float5
template <> NOALIAS __forceinline const Vec8F32 Load( MathsStorage::Float5 const * const __restrict pSrc );

/// Store Vec8F32 into Float5
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, MathsStorage::Float5 * const __restrict pDst );


/// Load Vec8F32 from Float6
template <> NOALIAS __forceinline const Vec8F32 Load( MathsStorage::Float6 const * const __restrict pSrc );

/// Store Vec8F32 into Float6
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, MathsStorage::Float6 * const __restrict pDst );


/// Load Vec8F32 from Float7
template <> NOALIAS __forceinline const Vec8F32 Load( MathsStorage::Float7 const * const __restrict pSrc );

/// Store Vec8F32 into Float7
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, MathsStorage::Float7 * const __restrict pDst );


/// Load Vec8F32 from Float8
template <> NOALIAS __forceinline const Vec8F32 Load( MathsStorage::Float8 const * const __restrict pSrc );

/// Store Vec8F32 into Float8
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, MathsStorage::Float8 * const __restrict pDst );


/// Load Vec8F32 from StorageVec1F32
//template <> NOALIAS __forceinline const Vec8F32 Load( StorageVec1F32 const * const __restrict pSrc );

/// Store Vec8F32 into StorageVec1F32
//NOALIAS __forceinline void Store( const Vec8F32 MATREF data, StorageVec1F32 * const __restrict pDst );


/// Load Vec8F32 from StorageVec2F32
//template <> NOALIAS __forceinline const Vec8F32 Load( StorageVec2F32 const * const __restrict pSrc );

/// Store Vec8F32 into StorageVec2F32
//NOALIAS __forceinline void Store( const Vec8F32 MATREF data, StorageVec2F32 * const __restrict pDst );


/// Load Vec8F32 from StorageVec3F32
//template <> NOALIAS __forceinline const Vec8F32 Load( StorageVec3F32 const * const __restrict pSrc );

/// Store Vec8F32 into StorageVec3F32
//NOALIAS __forceinline void Store( const Vec8F32 MATREF data, StorageVec3F32 * const __restrict pDst );


/// Load Vec8F32 from StorageVec4F32
//template <> NOALIAS __forceinline const Vec8F32 Load( StorageVec4F32 const * const __restrict pSrc );

/// Store Vec8F32 into StorageVec4F32
//NOALIAS __forceinline void Store( const Vec8F32 MATREF data, StorageVec4F32 * const __restrict pDst );


/// Load Vec8F32 from StorageVec5F32
template <> NOALIAS __forceinline const Vec8F32 Load( StorageVec5F32 const * const __restrict pSrc );

/// Store Vec8F32 into StorageVec5F32
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, StorageVec5F32 * const __restrict pDst );


/// Load Vec8F32 from StorageVec6F32
template <> NOALIAS __forceinline const Vec8F32 Load( StorageVec6F32 const * const __restrict pSrc );

/// Store Vec8F32 into StorageVec6F32
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, StorageVec6F32 * const __restrict pDst );


/// Load Vec8F32 from StorageVec7F32
template <> NOALIAS __forceinline const Vec8F32 Load( StorageVec7F32 const * const __restrict pSrc );

/// Store Vec8F32 into StorageVec7F32
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, StorageVec7F32 * const __restrict pDst );


/// Load Vec8F32 from StorageVec8F32
template <> NOALIAS __forceinline const Vec8F32 Load( StorageVec8F32 const * const __restrict pSrc );

/// Store Vec8F32 into StorageVec8F32
NOALIAS __forceinline void Store( const Vec8F32 MATREF data, StorageVec8F32 * const __restrict pDst );


/// Make Vec8F32 from a VF32 with the same value broadcast into all elements
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const VF32 VECREF e0 );

/// Make Vec8F32 from 2 VF32's
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const VF32 VECREF e0, const VF32 VECREF e1 );

/// Make Vec8F32 from 3 VF32's
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2 );

/// Make Vec8F32 from 4 VF32's
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3 );

/// Make Vec8F32 from 5 VF32's
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4 );

/// Make Vec8F32 from 6 VF32's
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4, const VF32 VECREF e5 );

/// Make Vec8F32 from 7 VF32's
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6 );

/// Make Vec8F32 from 8 VF32's
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const VF32 VECREF e0, const VF32 VECREF e1, const VF32 VECREF e2, const VF32 VECREF e3,
                                                 const VF32 VECREF e4, const VF32 VECREF e5, const VF32 VECREF e6, const VF32 VECREF e7 );


/// Make Vec8F32 from 2 Vec4F32's
NOALIAS __forceinline const Vec8F32 MakeVec8F32( const Vec4F32 VECREF v0123, const Vec4F32 VECREF v4567 );

/// Split Vec8F32 into 2 Vec4F32's
NOALIAS __forceinline void Split( const Vec8F32 MATREF src, Vec4F32* const __restrict pV0123, Vec4F32* const __restrict pV4567 );


/// Vec8F32 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vec8F32 MATREF v );

/// Vec8F32 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vec8F32* const __restrict pDst, const VF32 VECREF ei );


/// Vec8F32 Write to Stream
template < typename T_Stream >
void Serialise( const Vec8F32 MATREF src, T_Stream& stream );

/// Vec8F32 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, Vec8F32* const __restrict pDst );


/// Vec8F32 Permute : Indices select 8 result elements [I0,I1,I2,I3,I4,I5,I6,I7] from [ 0:v0, 1:v1, 2:v2, 3:v3, 4:v4, 5:v5, 6:v6, 7:v7 ]
template < unsigned int I0, unsigned int I1, unsigned int I2, unsigned int I3,
           unsigned int I4, unsigned int I5, unsigned int I6, unsigned int I7 >
NOALIAS __forceinline const Vec8F32 Permute( const Vec8F32 MATREF v );


/// Vec8F32 Abs of each element
NOALIAS __forceinline const Vec8F32 Abs( const Vec8F32 MATREF v );

/// Vec8F32 Sign of each element : For each x in v: returns -1.0f if x < 0.0f, +1.0f if x > 0.0f, or 0.0f if x == 0.0f 
NOALIAS __forceinline const Vec8F32 Sign( const Vec8F32 MATREF v );

/// Vec8F32 CopySign of each element : Returns a Vec8F32 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vec8F32 CopySign( const Vec8F32 MATREF magnitude, const Vec8F32 MATREF sign );

/// Vec8F32 Negation : Indices select elements to negate [I0,I1,I2,I3,I4,I5,I6,I7]
template < bool I0, bool I1, bool I2, bool I3,
           bool I4, bool I5, bool I6, bool I7 >
NOALIAS __forceinline const Vec8F32 Negate( const Vec8F32 MATREF v );


/// Vec8F32 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vec8F32 Rcp( const Vec8F32 MATREF v );

/// Vec8F32 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vec8F32 Rcp24( const Vec8F32 MATREF v );

/// Vec8F32 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec8F32 Div( const Vec8F32 MATREF x, const Vec8F32 MATREF y );

/// Vec8F32 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec8F32 Div24( const Vec8F32 MATREF x, const Vec8F32 MATREF y );

/// Vec8F32 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec8F32 Div( const VF32 VECREF x, const Vec8F32 MATREF y );

/// Vec8F32 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec8F32 Div24( const VF32 VECREF x, const Vec8F32 MATREF y );

/// Vec8F32 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vec8F32 Div( const Vec8F32 MATREF x, const VF32 VECREF y );

/// Vec8F32 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vec8F32 Div24( const Vec8F32 MATREF x, const VF32 VECREF y );

/// Vec8F32 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec8F32 RcpSqrt( const Vec8F32 MATREF v );

/// Vec8F32 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec8F32 RcpSqrt24( const Vec8F32 MATREF v );

/// Vec8F32 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec8F32 Sqrt( const Vec8F32 MATREF v );

/// Vec8F32 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec8F32 Sqrt24( const Vec8F32 MATREF v );

/// Vec8F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vec8F32 MATREF v,
                                       Vec8F32* const __restrict pRcp,
                                       Vec8F32* const __restrict pSqrt,
                                       Vec8F32* const __restrict pRcpSqrt );

/// Vec8F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vec8F32 MATREF v,
                                         Vec8F32* const __restrict pRcp,
                                         Vec8F32* const __restrict pSqrt,
                                         Vec8F32* const __restrict pRcpSqrt );

/// Vec8F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vec8F32 MATREF v,
                                           Vec8F32* const __restrict pSqrt,
                                           Vec8F32* const __restrict pRcpSqrt );

/// Vec8F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vec8F32 MATREF v,
                                             Vec8F32* const __restrict pSqrt,
                                             Vec8F32* const __restrict pRcpSqrt );


/// Vec8F32 Unary Positive Operator
NOALIAS __forceinline const Vec8F32 operator + ( const Vec8F32 MATREF x );

/// Vec8F32 Unary Negation Operator
NOALIAS __forceinline const Vec8F32 operator - ( const Vec8F32 MATREF x );


/// Vec8F32 Addition Operator
NOALIAS __forceinline const Vec8F32 operator + ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Subtraction Operator
NOALIAS __forceinline const Vec8F32 operator - ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Multiplication Operator
NOALIAS __forceinline const Vec8F32 operator * ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Division Operator
NOALIAS __forceinline const Vec8F32 operator / ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );


/// Vec8F32 Addition Update Operator
NOALIAS __forceinline void operator += ( Vec8F32& a, const Vec8F32 MATREF b );

/// Vec8F32 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Vec8F32& a, const Vec8F32 MATREF b );

/// Vec8F32 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vec8F32& a, const Vec8F32 MATREF b );

/// Vec8F32 Division Update Operator
NOALIAS __forceinline void operator /= ( Vec8F32& a, const Vec8F32 MATREF b );


/// Vec8F32 Scalar Multiplication: Vec8F32 * VF32
NOALIAS __forceinline const Vec8F32 operator * ( const Vec8F32 MATREF v, const VF32 VECREF s );

/// Vec8F32 Scalar Multiplication: VF32 * Vec8F32
NOALIAS __forceinline const Vec8F32 operator * ( const VF32 VECREF s, const Vec8F32 MATREF v );

/// Vec8F32 Scalar Division: Vec8F32 / VF32
NOALIAS __forceinline const Vec8F32 operator / ( const Vec8F32 MATREF v, const VF32 VECREF s );

/// Vec8F32 Scalar Division: VF32 / Vec8F32
NOALIAS __forceinline const Vec8F32 operator / ( const VF32 VECREF s, const Vec8F32 MATREF v );


/// Vec8F32 Scalar Multiplication Update: Vec8F32 *= VF32
NOALIAS __forceinline void operator *= ( Vec8F32& v, const VF32 VECREF s );

/// Vec8F32 Scalar Division Update: Vec8F32 /= VF32
NOALIAS __forceinline void operator /= ( Vec8F32& v, const VF32 VECREF s );


/// Vec8F32 MulAdd : a * b + c
NOALIAS __forceinline const Vec8F32 MulAdd( const Vec8F32 MATREF a, const Vec8F32 MATREF b, const Vec8F32 MATREF c );

/// Vec8F32 MulSub : a * b - c
NOALIAS __forceinline const Vec8F32 MulSub( const Vec8F32 MATREF a, const Vec8F32 MATREF b, const Vec8F32 MATREF c );

/// Vec8F32 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vec8F32 NegMulAdd( const Vec8F32 MATREF a, const Vec8F32 MATREF b, const Vec8F32 MATREF c );

/// Vec8F32 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vec8F32 NegMulSub( const Vec8F32 MATREF a, const Vec8F32 MATREF b, const Vec8F32 MATREF c );


/// Vec8F32 MulAdd : a * b + c : [ vector * scalar + vector ]
NOALIAS __forceinline const Vec8F32 MulAdd( const Vec8F32 MATREF a, const VF32 VECREF b, const Vec8F32 MATREF c );

/// Vec8F32 MulSub : a * b - c : [ vector * scalar - vector ]
NOALIAS __forceinline const Vec8F32 MulSub( const Vec8F32 MATREF a, const VF32 VECREF b, const Vec8F32 MATREF c );

/// Vec8F32 NegMulAdd : -( a * b + c ) : [ - ( vector * scalar + vector ) ]
NOALIAS __forceinline const Vec8F32 NegMulAdd( const Vec8F32 MATREF a, const VF32 VECREF b, const Vec8F32 MATREF c );

/// Vec8F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( vector * scalar - vector ) ]
NOALIAS __forceinline const Vec8F32 NegMulSub( const Vec8F32 MATREF a, const VF32 VECREF b, const Vec8F32 MATREF c );


/// Vec8F32 MulAdd : a * b + c : [ scalar * vector + vector ]
NOALIAS __forceinline const Vec8F32 MulAdd( const VF32 VECREF a, const Vec8F32 MATREF b, const Vec8F32 MATREF c );

/// Vec8F32 MulSub : a * b - c : [ scalar * vector - vector ]
NOALIAS __forceinline const Vec8F32 MulSub( const VF32 VECREF a, const Vec8F32 MATREF b, const Vec8F32 MATREF c );

/// Vec8F32 NegMulAdd : -( a * b + c ) : [ - ( scalar * vector + vector ) ]
NOALIAS __forceinline const Vec8F32 NegMulAdd( const VF32 VECREF a, const Vec8F32 MATREF b, const Vec8F32 MATREF c );

/// Vec8F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( scalar * vector - vector ) ]
NOALIAS __forceinline const Vec8F32 NegMulSub( const VF32 VECREF a, const Vec8F32 MATREF b, const Vec8F32 MATREF c );


/// Vec8F32 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vec8F32 Lerp( const Vec8F32 MATREF src, const Vec8F32 MATREF dst, const VF32 VECREF t );


/// Vec8F32 Min
NOALIAS __forceinline const Vec8F32 Min( const Vec8F32 MATREF x, const Vec8F32 MATREF y );

/// Vec8F32 Max
NOALIAS __forceinline const Vec8F32 Max( const Vec8F32 MATREF x, const Vec8F32 MATREF y );

/// Vec8F32 Min
NOALIAS __forceinline const Vec8F32 Min( const Vec8F32 MATREF x, const VF32 VECREF y );

/// Vec8F32 Max
NOALIAS __forceinline const Vec8F32 Max( const Vec8F32 MATREF x, const VF32 VECREF y );

/// Vec8F32 Min
NOALIAS __forceinline void Min( Vec8F32* const __restrict pX, const Vec8F32 MATREF y );

/// Vec8F32 Max
NOALIAS __forceinline void Max( Vec8F32* const __restrict pX, const Vec8F32 MATREF y );


/// Vec8F32 Clamp
NOALIAS __forceinline const Vec8F32 Clamp( const Vec8F32 MATREF x, const Vec8F32 MATREF lo, const Vec8F32 MATREF hi );

/// Vec8F32 Clamp
NOALIAS __forceinline void Clamp( Vec8F32* const __restrict pX, const Vec8F32 MATREF lo, const Vec8F32 MATREF hi );


/// Vec8F32 Round to integer towards minus infinity
NOALIAS __forceinline const Vec8F32 Floor( const Vec8F32 MATREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec8F32 Round to integer towards plus infinity
NOALIAS __forceinline const Vec8F32 Ceil ( const Vec8F32 MATREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec8F32 Round to integer towards nearest
NOALIAS __forceinline const Vec8F32 Round( const Vec8F32 MATREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec8F32 Round to integer towards zero
NOALIAS __forceinline const Vec8F32 Trunc( const Vec8F32 MATREF x ) __attribute__(( /* pure, ? */ always_inline ));


/// Vec8F32 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w
NOALIAS __forceinline const VBool operator == ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w
NOALIAS __forceinline const VBool operator != ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Comparison a <= b : Returns Vec8Bool representing results of a.i <= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec8Bool operator <= ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Comparison a >= b : Returns Vec8Bool representing results of a.i >= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec8Bool operator >= ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Comparison a < b : Returns Vec8Bool representing results of a.i < b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec8Bool operator < ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Comparison a > b : Returns Vec8Bool representing results of a.i > b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec8Bool operator > ( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vec8F32 MATREF a, const Vec8F32 MATREF b );


/// Vec8F32 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vec8F32 MATREF a );

/// Vec8F32 Finite Test : true if a contains no elements that are NaN's and for each element i [x,y,z,w], abs(x.i) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vec8F32 MATREF a );


/// Vec8F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec8F32 Select( const VBool PCREF condition, const Vec8F32 MATREF result_if_true, const Vec8F32 MATREF result_if_false );

/// Vec8F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec8F32 Select( const Vec8Bool PCREF condition, const Vec8F32 MATREF result_if_true, const Vec8F32 MATREF result_if_false );


/// Vec8F32 Bitwise NOT: ~a
NOALIAS __forceinline const Vec8F32 NOT( const Vec8F32 MATREF a );

/// Vec8F32 Bitwise OR: a | b
NOALIAS __forceinline const Vec8F32 OR( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Bitwise AND: a & b
NOALIAS __forceinline const Vec8F32 AND( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const Vec8F32 XOR( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Bitwise NOR: ~(a | b)
NOALIAS __forceinline const Vec8F32 NOR( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Bitwise NAND: ~(a & b)
NOALIAS __forceinline const Vec8F32 NAND( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const Vec8F32 EQV( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Bitwise ORC: a | ~b
NOALIAS __forceinline const Vec8F32 ORC( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Bitwise ANDC: a & ~b
NOALIAS __forceinline const Vec8F32 ANDC( const Vec8F32 MATREF a, const Vec8F32 MATREF b );


/// Vec8F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec8F32 AND( const Vec8F32 MATREF a, const VBool PCREF b );

/// Vec8F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec8F32 ANDC( const Vec8F32 MATREF a, const VBool PCREF b );


/// Vec8F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec8F32 AND( const Vec8F32 MATREF a, const Vec8Bool PCREF b );

/// Vec8F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec8F32 ANDC( const Vec8F32 MATREF a, const Vec8Bool PCREF b );



/// Vec8F32 Dot Product 2d
//NOALIAS __forceinline const VF32 Dot2( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Dot Product 3d
//NOALIAS __forceinline const VF32 Dot3( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Dot Product 4d
//NOALIAS __forceinline const VF32 Dot4( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Dot Product 5d
NOALIAS __forceinline const VF32 Dot5( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Dot Product 6d
NOALIAS __forceinline const VF32 Dot6( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Dot Product 7d
NOALIAS __forceinline const VF32 Dot7( const Vec8F32 MATREF a, const Vec8F32 MATREF b );

/// Vec8F32 Dot Product 8d
NOALIAS __forceinline const VF32 Dot8( const Vec8F32 MATREF a, const Vec8F32 MATREF b );


/// Vec8F32 Flush To Zero (5d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec8F32 FlushToZero5( const Vec8F32 MATREF v, const VF32 VECREF tolerance );

/// Vec8F32 Flush To Zero (6d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec8F32 FlushToZero6( const Vec8F32 MATREF v, const VF32 VECREF tolerance );

/// Vec8F32 Flush To Zero (7d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec8F32 FlushToZero7( const Vec8F32 MATREF v, const VF32 VECREF tolerance );

/// Vec8F32 Flush To Zero (8d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec8F32 FlushToZero8( const Vec8F32 MATREF v, const VF32 VECREF tolerance );


/// Vec8F32 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vec8F32 FlushElementsToZero( const Vec8F32 MATREF value, const VF32 VECREF tolerance );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


