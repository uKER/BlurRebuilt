#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR2_H
#define SOFT_MATHS_HVEC_NEW_VECTOR2_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR2_H
#define MATHS_HVEC_NEW_VECTOR2_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

/// New Vector Maths Library
namespace Maths
{
/// @addtogroup Vector2
/// @{

/// Load Vector2 from F32 ptr
template <> NOALIAS __forceinline const Vector2 Load( F32 const * const __restrict pSrc );

/// Store Vector2 to F32 ptr
NOALIAS __forceinline void Store( const Vector2 PCREF data, F32 * const __restrict pDst );


/// Load Vector2 from StorageVector2
NOALIAS __forceinline const Vector2 Load( StorageVector2 const * const __restrict pSrc );

/// Store Vector2 in StorageVector2
NOALIAS __forceinline void Store( const Vector2 PCREF data, StorageVector2 * const __restrict pDst );


/// Make Vector2 from broadcast VF32
NOALIAS __forceinline const Vector2 MakeVector2( const VF32 VECREF src );


/// Make Vector2 from Vec4F32
NOALIAS __forceinline const Vector2 MakeVector2( const Vec4F32 VECREF src );

/// Get Vec4F32 from Vector2
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Vector2 PCREF src );


/// Make Vector2 from Vector3
NOALIAS __forceinline const Vector2 MakeVector2( const Vector3 PCREF v3 );

/// Make Vector2 from Vector4
NOALIAS __forceinline const Vector2 MakeVector2( const Vector4 PCREF v4 );


/// Make Vector2 from 2 F32's
NOALIAS __forceinline const Vector2 MakeVector2( const F32& x, const F32& y );

/// Make Vector2 from 2 VF32's
NOALIAS __forceinline const Vector2 MakeVector2( const VF32 VECREF x, const VF32 VECREF y );


/// Vector2 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vector2 PCREF src );

/// Vector2 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vector2 PCREF src );


/// Vector2 Accessor: Set X
NOALIAS __forceinline void SetX( Vector2* const __restrict pDst, const VF32 VECREF x );

/// Vector2 Accessor: Set Y
NOALIAS __forceinline void SetY( Vector2* const __restrict pDst, const VF32 VECREF y );


/// Vector2 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector2& v );

/// Vector2 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector2* const __restrict pDst, const VF32 VECREF ei );


/// Vector2 Write to Stream
template < typename T_Stream > void Serialise( const Vector2 PCREF src, T_Stream& stream );

/// Vector2 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Vector2* const __restrict pDst );



/// Vector2 Permute : Indices select 2 result elements [x,y] from [ 0:ax, 1:ay ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vector2 Permute( const Vector2 PCREF a );

/// Vector2 Permute : Indices select 2 result elements [x,y] from [ 0:ax, 1:ay, 2:bx, 3:by ]
template < unsigned int IX, unsigned int IY >
NOALIAS __forceinline const Vector2 Permute( const Vector2 PCREF a, const Vector2 PCREF b );



/// Vector2 Abs of each element
NOALIAS __forceinline const Vector2 Abs( const Vector2 PCREF v );

/// Vector2 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector2 Sign( const Vector2 PCREF v );

/// Vector2 CopySign of each element : Returns a Vector2 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector2 CopySign( const Vector2 PCREF magnitude, const Vector2 PCREF sign );


/// Vector2 Negation : Indices select elements to negate [X,Y]
template < bool X, bool Y >
NOALIAS __forceinline const Vector2 Negate( const Vector2& v );

/// Vector2 Negate X
NOALIAS __forceinline const Vector2 NegateX( const Vector2 PCREF src );

/// Vector2 Negate Y
NOALIAS __forceinline const Vector2 NegateY( const Vector2 PCREF src );


/// Vector2 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Rcp( const Vector2 PCREF v );

/// Vector2 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Rcp24( const Vector2 PCREF v );

/// Vector2 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Div( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Div24( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Div( const VF32 VECREF s, const Vector2 PCREF v );

/// Vector2 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Div24( const VF32 VECREF s, const Vector2 PCREF v );

/// Vector2 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Div( const Vector2 PCREF v, const VF32 VECREF s );

/// Vector2 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Div24( const Vector2 PCREF v, const VF32 VECREF s );

/// Vector2 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 RcpSqrt( const Vector2 PCREF v );

/// Vector2 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 RcpSqrt24( const Vector2 PCREF v );

/// Vector2 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Sqrt( const Vector2 PCREF v );

/// Vector2 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Sqrt24( const Vector2 PCREF v );


/// Vector2 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vector2 PCREF v,
                                       Vector2* const __restrict pRcp,
                                       Vector2* const __restrict pSqrt,
                                       Vector2* const __restrict pRcpSqrt );

/// Vector2 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vector2 PCREF v,
                                         Vector2* const __restrict pRcp,
                                         Vector2* const __restrict pSqrt,
                                         Vector2* const __restrict pRcpSqrt );


/// Vector2 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vector2 PCREF v,
                                           Vector2* const __restrict pSqrt,
                                           Vector2* const __restrict pRcpSqrt );

/// Vector2 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vector2 PCREF v,
                                             Vector2* const __restrict pSqrt,
                                             Vector2* const __restrict pRcpSqrt );



/// Vector2 Unary Positive Operator
NOALIAS __forceinline const Vector2 operator + ( const Vector2 PCREF x );

/// Vector2 Unary Negation Operator
NOALIAS __forceinline const Vector2 operator - ( const Vector2 PCREF x );


/// Vector2 Sum
NOALIAS __forceinline const Vector2 operator + ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Difference
NOALIAS __forceinline const Vector2 operator - ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Multiplication Operator
NOALIAS __forceinline const Vector2 operator * ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Division Operator
NOALIAS __forceinline const Vector2 operator / ( const Vector2 PCREF a, const Vector2 PCREF b );


/// Vector2 Addition Update
NOALIAS __forceinline void operator += ( Vector2& a, const Vector2 PCREF b );

/// Vector2 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector2& a, const Vector2 PCREF b );

/// Vector2 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector2& a, const Vector2 PCREF b );

/// Vector2 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector2& a, const Vector2 PCREF b );


/// Vector2 Scalar Multiplication: Vector2 * VF32
NOALIAS __forceinline const Vector2 operator * ( const Vector2 PCREF v, const VF32 VECREF s );

/// Vector2 Scalar Multiplication: VF32 * Vector2
NOALIAS __forceinline const Vector2 operator * ( const VF32 VECREF s, const Vector2 PCREF v );

/// Vector2 Scalar Division: Vector2 / VF32
NOALIAS __forceinline const Vector2 operator / ( const Vector2 PCREF v, const VF32 VECREF s );

/// Vector2 Scalar Division: VF32 / Vector2
NOALIAS __forceinline const Vector2 operator / ( const VF32 VECREF s, const Vector2 PCREF v );


/// Vector2 Scalar Multiplication Update: Vector2 *= VF32
NOALIAS __forceinline void operator *= ( Vector2& v, const VF32 VECREF s );

/// Vector2 Scalar Division Update: Vector2 /= VF32
NOALIAS __forceinline void operator /= ( Vector2& v, const VF32 VECREF s );



/// Vector2 MulAdd : a * b + c
NOALIAS __forceinline const Vector2 MulAdd( const Vector2 PCREF a, const Vector2 PCREF b, const Vector2 PCREF c );

/// Vector2 MulSub : a * b - c
NOALIAS __forceinline const Vector2 MulSub( const Vector2 PCREF a, const Vector2 PCREF b, const Vector2 PCREF c );

/// Vector2 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector2 NegMulAdd( const Vector2 PCREF a, const Vector2 PCREF b, const Vector2 PCREF c );

/// Vector2 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector2 NegMulSub( const Vector2 PCREF a, const Vector2 PCREF b, const Vector2 PCREF c );


/// Vector2 MulAdd : a * b + c
NOALIAS __forceinline const Vector2 MulAdd( const Vector2 PCREF a, const VF32 VECREF b, const Vector2 PCREF c );

/// Vector2 MulSub : a * b - c
NOALIAS __forceinline const Vector2 MulSub( const Vector2 PCREF a, const VF32 VECREF b, const Vector2 PCREF c );

/// Vector2 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector2 NegMulAdd( const Vector2 PCREF a, const VF32 VECREF b, const Vector2 PCREF c );

/// Vector2 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector2 NegMulSub( const Vector2 PCREF a, const VF32 VECREF b, const Vector2 PCREF c );


/// Vector2 MulAdd : a * b + c
NOALIAS __forceinline const Vector2 MulAdd( const VF32 VECREF a, const Vector2 PCREF b, const Vector2 PCREF c );

/// Vector2 MulSub : a * b - c
NOALIAS __forceinline const Vector2 MulSub( const VF32 VECREF a, const Vector2 PCREF b, const Vector2 PCREF c );

/// Vector2 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector2 NegMulAdd( const VF32 VECREF a, const Vector2 PCREF b, const Vector2 PCREF c );

/// Vector2 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector2 NegMulSub( const VF32 VECREF a, const Vector2 PCREF b, const Vector2 PCREF c );


/// Vector2 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vector2 Lerp( const Vector2 PCREF src, const Vector2 PCREF dst, const VF32 VECREF t );


/// Vector2 Min
NOALIAS __forceinline const Vector2 Min( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Max
NOALIAS __forceinline const Vector2 Max( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Min
NOALIAS __forceinline const Vector2 Min( const Vector2 PCREF a, const VF32 VECREF b );

/// Vector2 Max
NOALIAS __forceinline const Vector2 Max( const Vector2 PCREF a, const VF32 VECREF b );

/// Vector2 Min Update
NOALIAS __forceinline void Min( Vector2* const __restrict pA, const Vector2 PCREF b );

/// Vector2 Max Update
NOALIAS __forceinline void Max( Vector2* const __restrict pA, const Vector2 PCREF b );

/// Vector2 Min Update
NOALIAS __forceinline void Min( Vector2* const __restrict pA, const VF32 VECREF b );

/// Vector2 Max Update
NOALIAS __forceinline void Max( Vector2* const __restrict pA, const VF32 VECREF b );


/// Vector2 Clamp
NOALIAS __forceinline const Vector2 Clamp( const Vector2 PCREF a, const Vector2 PCREF lo, const Vector2 PCREF hi );

/// Vector2 Clamp
NOALIAS __forceinline const Vector2 Clamp( const Vector2 PCREF a, const VF32 VECREF lo, const VF32 VECREF hi );

/// Vector2 Clamp Update
NOALIAS __forceinline void Clamp( Vector2* const __restrict pA, const Vector2 PCREF lo, const Vector2 PCREF hi );

/// Vector2 Clamp Update
NOALIAS __forceinline void Clamp( Vector2* const __restrict pA, const VF32 VECREF lo, const VF32 VECREF hi );


/// Vector2 Round to integer towards minus infinity
NOALIAS __forceinline const Vector2 Floor( const Vector2 PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// Vector2 Round to integer towards plus infinity
NOALIAS __forceinline const Vector2 Ceil ( const Vector2 PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// Vector2 Round to integer towards nearest
NOALIAS __forceinline const Vector2 Round( const Vector2 PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// Vector2 Round to integer towards zero
NOALIAS __forceinline const Vector2 Trunc( const Vector2 PCREF a ) __attribute__(( /* pure, ? */ always_inline ));


/// Vector2 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y
NOALIAS __forceinline const VBool operator == ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y
NOALIAS __forceinline const VBool operator != ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y] (Z&W are undefined)
NOALIAS __forceinline const Vec4Bool operator <= ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Comparison a >= b : Returns VBool representing results of a.i >= b.i, for each element i [x,y] (Z&W are undefined)
NOALIAS __forceinline const Vec4Bool operator >= ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Comparison a < b : Returns VBool representing results of a.i < b.i, for each element i [x,y] (Z&W are undefined)
NOALIAS __forceinline const Vec4Bool operator < ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Comparison a > b : Returns VBool representing results of a.i > b.i, for each element i [x,y] (Z&W are undefined)
NOALIAS __forceinline const Vec4Bool operator > ( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Ordered Test : true if neither a nor b contains elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Unordered Test : true if a or b contains elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector2 PCREF a, const Vector2 PCREF b );

/// Vector2 Finite test
NOALIAS __forceinline const VBool Finite( const Vector2 PCREF src );


/// Vector2 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector2 Select( const VBool PCREF condition, const Vector2 PCREF result_if_true, const Vector2 PCREF result_if_false );

/// Vector2 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector2 Select( const Vec4Bool PCREF condition, const Vector2 PCREF result_if_true, const Vector2 PCREF result_if_false );


/// Vector2 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vector2 AND( const Vector2 PCREF a, const VBool PCREF b );

/// Vector2 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vector2 ANDC( const Vector2 PCREF a, const VBool PCREF b );


/// Vector2 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vector2 AND( const Vector2 PCREF a, const Vec4Bool PCREF b );

/// Vector2 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vector2 ANDC( const Vector2 PCREF a, const Vec4Bool PCREF b );


/// Vector2 dot product
NOALIAS __forceinline const VF32 Dot( const Vector2 PCREF a, const Vector2 PCREF b );


/// Vector2 cross product
NOALIAS __forceinline const VF32 Cross( const Vector2 PCREF a, const Vector2 PCREF b );


/// Vector2 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Vector2 PCREF a, const Vector2 PCREF b );


/// Vector2 cross product operator
NOALIAS __forceinline const VF32 operator ^ ( const Vector2 PCREF a, const Vector2 PCREF b );


/// Vector2 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector2 PCREF v );


/// Vector2 Length (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length( const Vector2 PCREF v );

/// Vector2 Length (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length24( const Vector2 PCREF v );


/// Vector2 Set Length (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength( Vector2* const __restrict pDst, const VF32 VECREF length );

/// Vector2 Set Length (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength24( Vector2* const __restrict pDst, const VF32 VECREF length );


/// Vector2 Limit Length (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength( Vector2* const __restrict pV, const VF32 VECREF limit );

/// Vector2 Limit Length (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength24( Vector2* const __restrict pV, const VF32 VECREF limit );


/// Vector2 Normalise (faster but only 22 bit precision)
//NOALIAS __forceinline const Normal2 Normalise( const Vector2 PCREF v );
NOALIAS __forceinline const Vector2 Normalise( const Vector2 PCREF v );

/// Vector2 Normalise (slower but full 24 bit precision)
//NOALIAS __forceinline const Normal2 Normalise24( const Vector2 PCREF v );
NOALIAS __forceinline const Vector2 Normalise24( const Vector2 PCREF v );


/// Vector2 Normalise (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Vector2* const __restrict pDst );

/// Vector2 Normalise (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Vector2* const __restrict pDst );


/// Vector2 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vector2 Inverse( const Vector2 PCREF v );

/// Vector2 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vector2 Inverse24( const Vector2 PCREF v );


/// Vector2 Flush To Zero: flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vector2 FlushToZero( const Vector2 PCREF v, const VF32 VECREF tolerance );

/// Vector2 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vector2 FlushElementsToZero( const Vector2 PCREF v, const VF32 VECREF tolerance );


/// Vector2 maximum absolute value of all elements
NOALIAS __forceinline const VF32 MaxAbsValue( const Vector2 PCREF src );


/// Vector2 Project Onto Axis: returns the component of vector 'v' parallel to axis 'a'
NOALIAS __forceinline const Vector2 Project( const Vector2 PCREF v, const Vector2 PCREF a );

/// Vector2 Reject From Axis: returns the component of vector 'v' perpendicular to axis 'a'
NOALIAS __forceinline const Vector2 Reject( const Vector2 PCREF v, const Vector2 PCREF a );


/// Vector2 ZeroX: sets the X component to zero
NOALIAS __forceinline void ZeroX( Vector2* const __restrict pV );

/// Vector2 ZeroY: sets the Y component to zero
NOALIAS __forceinline void ZeroY( Vector2* const __restrict pV );


/// Vector2 ZeroX: returns copy of v with the X component set to zero
NOALIAS __forceinline const Vector2 ZeroX( const Vector2 PCREF v );

/// Vector2 ZeroY: returns copy of v with the Y component set to zero
NOALIAS __forceinline const Vector2 ZeroY( const Vector2 PCREF v );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

