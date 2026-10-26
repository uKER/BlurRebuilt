#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR3_H
#define SOFT_MATHS_HVEC_NEW_VECTOR3_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR3_H
#define MATHS_HVEC_NEW_VECTOR3_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

/// New Vector Maths Library
namespace Maths
{
/// @addtogroup Vector3
/// @{

/// Load Vector3 from F32 ptr
template <> NOALIAS __forceinline const Vector3 Load( F32 const * const __restrict pSrc );

/// Store Vector3 to F32 ptr
NOALIAS __forceinline void Store( const Vector3 PCREF data, F32 * const __restrict pDst );


/// Load Vector3 from StorageVector3
NOALIAS __forceinline const Vector3 Load( StorageVector3 const * const __restrict pSrc );

/// Store Vector3 in StorageVector3
NOALIAS __forceinline void Store( const Vector3 PCREF data, StorageVector3 * const __restrict pDst );


/// Make Vector3 from splatted VF32
NOALIAS __forceinline const Vector3 MakeVector3( const VF32 VECREF src );

/// Make Vector3 from Vec4F32
NOALIAS __forceinline const Vector3 MakeVector3( const Vec4F32 VECREF src );

/// Get Vec4F32 from Vector3
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Vector3 PCREF src );


/// Make Vector3 from Vector4
NOALIAS __forceinline const Vector3 MakeVector3( const Vector4 PCREF v4 );

/// Make Vector3 from Point3
NOALIAS __forceinline const Vector3 MakeVector3( const Point3 PCREF p3 );

/// Make Vector3 from Normal3
NOALIAS __forceinline const Vector3 MakeVector3( const Normal3 PCREF n3 );


/// Make Vector3 from 3 F32's
NOALIAS __forceinline const Vector3 MakeVector3( const F32& x, const F32& y, const F32& z );

/// Make Vector3 from 3 VF32's
NOALIAS __forceinline const Vector3 MakeVector3( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );


/// Vector3 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vector3 PCREF src );

/// Vector3 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vector3 PCREF src );

/// Vector3 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Vector3 PCREF src );


/// Vector3 Accessor: Set X
NOALIAS __forceinline void SetX( Vector3* const __restrict pDst, const VF32 VECREF x );

/// Vector3 Accessor: Set Y
NOALIAS __forceinline void SetY( Vector3* const __restrict pDst, const VF32 VECREF y );

/// Vector3 Accessor: Set Z
NOALIAS __forceinline void SetZ( Vector3* const __restrict pDst, const VF32 VECREF z );


/// Vector3 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector3 PCREF v );

/// Vector3 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector3* const __restrict pDst, const VF32 VECREF ei );


/// Vector3 Write to Stream
template < typename T_Stream > void Serialise( const Vector3 PCREF src, T_Stream& stream );

/// Vector3 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Vector3* const __restrict pDst );



/// Vector3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vector3 Permute( const Vector3 PCREF a );

/// Vector3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:bx, 4:by, 5:bz ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vector3 Permute( const Vector3 PCREF a, const Vector3 PCREF b );



/// Vector3 Abs of each element
NOALIAS __forceinline const Vector3 Abs( const Vector3 PCREF v );

/// Vector3 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector3 Sign( const Vector3 PCREF v );

/// Vector3 CopySign of each element : Returns a Vector3 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector3 CopySign( const Vector3 PCREF magnitude, const Vector3 PCREF sign );



/// Vector3 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector3 Rcp( const Vector3 PCREF v );

/// Vector3 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector3 Rcp24( const Vector3 PCREF v );

/// Vector3 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector3 Div( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector3 Div24( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector3 Div( const VF32 VECREF s, const Vector3 PCREF v );

/// Vector3 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector3 Div24( const VF32 VECREF s, const Vector3 PCREF v );

/// Vector3 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vector3 Div( const Vector3 PCREF v, const VF32 VECREF s );

/// Vector3 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vector3 Div24( const Vector3 PCREF v, const VF32 VECREF s );

/// Vector3 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector3 RcpSqrt( const Vector3 PCREF v );

/// Vector3 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector3 RcpSqrt24( const Vector3 PCREF v );

/// Vector3 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector3 Sqrt( const Vector3 PCREF v );

/// Vector3 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector3 Sqrt24( const Vector3 PCREF v );

/// Vector3 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vector3 PCREF v,
                                       Vector3* const __restrict pRcp,
                                       Vector3* const __restrict pSqrt,
                                       Vector3* const __restrict pRcpSqrt );

/// Vector3 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vector3 PCREF v,
                                         Vector3* const __restrict pRcp,
                                         Vector3* const __restrict pSqrt,
                                         Vector3* const __restrict pRcpSqrt );



/// Vector3 Unary Positive Operator
NOALIAS __forceinline const Vector3 operator + ( const Vector3 PCREF x );

/// Vector3 Unary Negation Operator
NOALIAS __forceinline const Vector3 operator - ( const Vector3 PCREF x );


/// Vector3 Negate X
NOALIAS __forceinline const Vector3 NegateX( const Vector3 PCREF src );

/// Vector3 Negate Y
NOALIAS __forceinline const Vector3 NegateY( const Vector3 PCREF src );

/// Vector3 Negate Z
NOALIAS __forceinline const Vector3 NegateZ( const Vector3 PCREF src );


/// Vector3 Negate XY
NOALIAS __forceinline const Vector3 NegateXY( const Vector3 PCREF src );

/// Vector3 Negate XZ
NOALIAS __forceinline const Vector3 NegateXZ( const Vector3 PCREF src );

/// Vector3 Negate YZ
NOALIAS __forceinline const Vector3 NegateYZ( const Vector3 PCREF src );


/// Vector3 Sum
NOALIAS __forceinline const Vector3 operator + ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Difference
NOALIAS __forceinline const Vector3 operator - ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Addition Update
NOALIAS __forceinline void operator += ( Vector3& a, const Vector3 PCREF b );

/// Vector3 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector3& a, const Vector3 PCREF b );


/// Vector3 Scalar Multiplication: Vector3 * VF32
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const VF32 VECREF s );

/// Vector3 Scalar Multiplication: VF32 * Vector3
NOALIAS __forceinline const Vector3 operator * ( const VF32 VECREF s, const Vector3 PCREF v );

/// Vector3 Scalar Division: Vector3 / VF32
NOALIAS __forceinline const Vector3 operator / ( const Vector3 PCREF v, const VF32 VECREF s );

/// Vector3 Scalar Division: VF32 / Vector3
NOALIAS __forceinline const Vector3 operator / ( const VF32 VECREF s, const Vector3 PCREF v );

/// Vector3 Scalar Multiplication Update: Vector3 *= VF32
NOALIAS __forceinline void operator *= ( Vector3& v, const VF32 VECREF s );

/// Vector3 Scalar Division Update: Vector3 /= VF32
NOALIAS __forceinline void operator /= ( Vector3& v, const VF32 VECREF s );


/// Vector3 Multiplication Operator
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Division Operator
NOALIAS __forceinline const Vector3 operator / ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector3& a, const Vector3 PCREF b );

/// Vector3 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector3& a, const Vector3 PCREF b );


/// Vector3 Addition: Vector3 + Normal3
NOALIAS __forceinline const Vector3 operator + ( const Vector3 PCREF v, const Normal3 PCREF n );

/// Vector3 Addition: Normal3 + Vector3
NOALIAS __forceinline const Vector3 operator + ( const Normal3 PCREF n, const Vector3 PCREF v );

/// Vector3 Subtraction: Vector3 - Normal3
NOALIAS __forceinline const Vector3 operator - ( const Vector3 PCREF v, const Normal3 PCREF n );

/// Vector3 Subtraction: Normal3 - Vector3
NOALIAS __forceinline const Vector3 operator - ( const Normal3 PCREF n, const Vector3 PCREF v );

/// Vector3 Addition Update: Vector3 += Normal3
NOALIAS __forceinline void operator += ( Vector3& v, const Normal3 PCREF n );

/// Vector3 Subtraction Update: Vector3 -= Normal3
NOALIAS __forceinline void operator -= ( Vector3& v, const Normal3 PCREF n );


/// Vector3 MulAdd : a * b + c
NOALIAS __forceinline const Vector3 MulAdd( const Vector3 PCREF a, const Vector3 PCREF b, const Vector3 PCREF c );

/// Vector3 MulSub : a * b - c
NOALIAS __forceinline const Vector3 MulSub( const Vector3 PCREF a, const Vector3 PCREF b, const Vector3 PCREF c );

/// Vector3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector3 NegMulAdd( const Vector3 PCREF a, const Vector3 PCREF b, const Vector3 PCREF c );

/// Vector3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector3 NegMulSub( const Vector3 PCREF a, const Vector3 PCREF b, const Vector3 PCREF c );


/// Vector3 MulAdd : a * b + c
NOALIAS __forceinline const Vector3 MulAdd( const VF32 VECREF a, const Vector3 PCREF b, const Vector3 PCREF c );

/// Vector3 MulSub : a * b - c
NOALIAS __forceinline const Vector3 MulSub( const VF32 VECREF a, const Vector3 PCREF b, const Vector3 PCREF c );

/// Vector3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector3 NegMulAdd( const VF32 VECREF a, const Vector3 PCREF b, const Vector3 PCREF c );

/// Vector3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector3 NegMulSub( const VF32 VECREF a, const Vector3 PCREF b, const Vector3 PCREF c );


/// Vector3 MulAdd : a * b + c
NOALIAS __forceinline const Vector3 MulAdd( const Vector3 PCREF a, const VF32 VECREF b, const Vector3 PCREF c );

/// Vector3 MulSub : a * b - c
NOALIAS __forceinline const Vector3 MulSub( const Vector3 PCREF a, const VF32 VECREF b, const Vector3 PCREF c );

/// Vector3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector3 NegMulAdd( const Vector3 PCREF a, const VF32 VECREF b, const Vector3 PCREF c );

/// Vector3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector3 NegMulSub( const Vector3 PCREF a, const VF32 VECREF b, const Vector3 PCREF c );


/// Vector3 MulAdd : a * b + c
NOALIAS __forceinline const Vector3 MulAdd( const Normal3 PCREF a, const VF32 VECREF b, const Vector3 PCREF c );

/// Vector3 MulSub : a * b - c
NOALIAS __forceinline const Vector3 MulSub( const Normal3 PCREF a, const VF32 VECREF b, const Vector3 PCREF c );

/// Vector3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector3 NegMulAdd( const Normal3 PCREF a, const VF32 VECREF b, const Vector3 PCREF c );

/// Vector3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector3 NegMulSub( const Normal3 PCREF a, const VF32 VECREF b, const Vector3 PCREF c );


/// Vector3 MulAdd : a * b + c
NOALIAS __forceinline const Vector3 MulAdd( const Vector3 PCREF a, const VF32 VECREF b, const Normal3 PCREF c );

/// Vector3 MulSub : a * b - c
NOALIAS __forceinline const Vector3 MulSub( const Vector3 PCREF a, const VF32 VECREF b, const Normal3 PCREF c );

/// Vector3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector3 NegMulAdd( const Vector3 PCREF a, const VF32 VECREF b, const Normal3 PCREF c );

/// Vector3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector3 NegMulSub( const Vector3 PCREF a, const VF32 VECREF b, const Normal3 PCREF c );


/// Vector3 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vector3 Lerp( const Vector3 PCREF src, const Vector3 PCREF dst, const VF32 VECREF t );

/// Vector3 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vector3 Lerp( const Normal3 PCREF src, const Vector3 PCREF dst, const VF32 VECREF t );

/// Vector3 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vector3 Lerp( const Vector3 PCREF src, const Normal3 PCREF dst, const VF32 VECREF t );


/// Vector3 dot product
NOALIAS __forceinline const VF32 Dot( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 dot product
NOALIAS __forceinline const VF32 Dot( const Vector3 PCREF a, const Normal3 PCREF b );

/// Vector3 dot product
NOALIAS __forceinline const VF32 Dot( const Normal3 PCREF a, const Vector3 PCREF b );


/// Vector3 cross product
NOALIAS __forceinline const Vector3 Cross( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 cross product
NOALIAS __forceinline const Vector3 Cross( const Vector3 PCREF a, const Normal3 PCREF b );

/// Vector3 cross product
NOALIAS __forceinline const Vector3 Cross( const Normal3 PCREF a, const Vector3 PCREF b );


/// Vector3 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 | Normal3 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Vector3 PCREF a, const Normal3 PCREF b );

/// Normal3 | Vector3 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Normal3 PCREF a, const Vector3 PCREF b );


/// Vector3 cross product operator
NOALIAS __forceinline const Vector3 operator ^ ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Normal3 ^ Vector3 cross product operator
NOALIAS __forceinline const Vector3 operator ^ ( const Normal3 PCREF a, const Vector3 PCREF b );

/// Vector3 ^ Normal3 cross product operator
NOALIAS __forceinline const Vector3 operator ^ ( const Vector3 PCREF a, const Normal3 PCREF b );


/// Vector3 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector3 PCREF v );


/// Vector3 Length (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length( const Vector3 PCREF v );

/// Vector3 Length (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length24( const Vector3 PCREF v );


/// Vector3 Reciprocal Length (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpLength( const Vector3 PCREF v );

/// Vector3 Reciprocal Length (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpLength24( const Vector3 PCREF v );


/// Vector3 Set Length (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength( Vector3* const __restrict pDst, const VF32 VECREF length );

/// Vector3 Set Length (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength24( Vector3* const __restrict pDst, const VF32 VECREF length );


/// Vector3 Normalise (faster but only 22 bit precision)
NOALIAS __forceinline const Normal3 Normalise( const Vector3 PCREF v );

/// Vector3 Normalise (slower but full 24 bit precision)
NOALIAS __forceinline const Normal3 Normalise24( const Vector3 PCREF v );


/// Vector3 Normalise (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Vector3* const __restrict pDst );

/// Vector3 Normalise (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Vector3* const __restrict pDst );


/// Vector3 Normalise, returns zeroLengthResult if length of v is zero. (faster but only 22 bit precision)
NOALIAS __forceinline const Normal3 Normalise( const Vector3 PCREF v, const Normal3 PCREF zeroLengthResult );

/// Vector3 Normalise, returns zeroLengthResult if length of v is zero. (slower but full 24 bit precision)
NOALIAS __forceinline const Normal3 Normalise24( const Vector3 PCREF v, const Normal3 PCREF zeroLengthResult );


/// Vector3 Normalise, sets zeroLengthResult if length of *pDst is zero. (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Vector3* const __restrict pDst, const Vector3 PCREF zeroLengthResult );

/// Vector3 Normalise, sets zeroLengthResult if length of *pDst is zero. (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Vector3* const __restrict pDst, const Vector3 PCREF zeroLengthResult );


/// Vector3 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vector3 Inverse( const Vector3 PCREF v );

/// Vector3 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vector3 Inverse24( const Vector3 PCREF v );


/// Vector3 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z
NOALIAS __forceinline const VBool operator == ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z
NOALIAS __forceinline const VBool operator != ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator <= ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Comparison a >= b : Returns VBool representing results of a.i >= b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator >= ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Comparison a < b : Returns VBool representing results of a.i < b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator < ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Comparison a > b : Returns VBool representing results of a.i > b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator > ( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Ordered Test : true if neither a nor b contains elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Unordered Test : true if a or b contains elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Finite test
NOALIAS __forceinline const VBool Finite( const Vector3 PCREF src );



/// Vector3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector3 Select( const VBool PCREF condition, const Vector3 PCREF result_if_true, const Vector3 PCREF result_if_false );

/// Vector3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector3 Select( const Vec4Bool PCREF condition, const Vector3 PCREF result_if_true, const Vector3 PCREF result_if_false );


/// Vector3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector3 Select( const VBool PCREF condition, const Vector3 PCREF result_if_true, const Normal3 PCREF result_if_false );

/// Vector3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector3 Select( const Vec4Bool PCREF condition, const Vector3 PCREF result_if_true, const Normal3 PCREF result_if_false );

/// Vector3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector3 Select( const VBool PCREF condition, const Normal3 PCREF result_if_true, const Vector3 PCREF result_if_false );

/// Vector3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector3 Select( const Vec4Bool PCREF condition, const Normal3 PCREF result_if_true, const Vector3 PCREF result_if_false );


/// Vector3 maximum absolute value of all elements
NOALIAS __forceinline const VF32 MaxAbsValue( const Vector3 PCREF src );


/// Vector3 Project Onto Axis: returns the component of vector 'v' parallel to axis 'a'
NOALIAS __forceinline const Vector3 Project( const Vector3 PCREF v, const Normal3 PCREF a );

/// Vector3 Project Onto Axis: returns the component of vector 'v' parallel to axis 'a'. Note: normalises axis and then calls the Normal3 variant of Project!
NOALIAS __forceinline const Vector3 Project( const Vector3 PCREF v, const Vector3 PCREF a );

/// Vector3 Reject From Axis: returns the component of vector 'v' perpendicular to axis 'a'
NOALIAS __forceinline const Vector3 Reject( const Vector3 PCREF v, const Normal3 PCREF a );

/// Vector3 Reject From Axis: returns the component of vector 'v' perpendicular to axis 'a'. Note: normalises axis and then calls the Normal3 variant of Reject!
NOALIAS __forceinline const Vector3 Reject( const Vector3 PCREF v, const Vector3 PCREF a );


/// Vector3 ZeroX: sets the X component to zero
NOALIAS __forceinline void ZeroX( Vector3* const __restrict pV );

/// Vector3 ZeroY: sets the Y component to zero
NOALIAS __forceinline void ZeroY( Vector3* const __restrict pV );

/// Vector3 ZeroZ: sets the Z component to zero
NOALIAS __forceinline void ZeroZ( Vector3* const __restrict pV );


/// Vector3 ZeroX: returns copy of v with the X component set to zero
NOALIAS __forceinline const Vector3 ZeroX( const Vector3 PCREF v );

/// Vector3 ZeroY: returns copy of v with the Y component set to zero
NOALIAS __forceinline const Vector3 ZeroY( const Vector3 PCREF v );

/// Vector3 ZeroZ: returns copy of v with the Z component set to zero
NOALIAS __forceinline const Vector3 ZeroZ( const Vector3 PCREF v );


/// Vector3 Flush To Zero: flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vector3 FlushToZero( const Vector3 PCREF v, const VF32 VECREF tolerance );

/// Vector3 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vector3 FlushElementsToZero( const Vector3 PCREF v, const VF32 VECREF tolerance );


/// Vector3 Min
NOALIAS __forceinline const Vector3 Min( const Vector3 PCREF a, const Vector3 PCREF b );

/// Vector3 Max
NOALIAS __forceinline const Vector3 Max( const Vector3 PCREF a, const Vector3 PCREF b );


/// Vector3: Calculate Perpendicular Axes
/// Output normals obey same handedness as matrices, with input as axis z
NOALIAS __forceinline void PerpendicularAxes( const Vector3 PCREF vectorZ,
                                              Normal3* const __restrict pAxisX,
                                              Normal3* const __restrict pAxisY,
                                              Normal3* const __restrict pAxisZ );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

