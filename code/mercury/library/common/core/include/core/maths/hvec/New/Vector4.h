#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR4_H
#define SOFT_MATHS_HVEC_NEW_VECTOR4_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR4_H
#define MATHS_HVEC_NEW_VECTOR4_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector4
/// @{

/// Load Vector4 from F32 ptr
template <> NOALIAS __forceinline const Vector4 Load( F32 const * const __restrict pSrc );

/// Store Vector4 to F32 ptr
NOALIAS __forceinline void Store( const Vector4 PCREF data, F32 * const __restrict pDst );


/// Load Vector4 from StorageVector4
NOALIAS __forceinline const Vector4 Load( StorageVector4 const * const __restrict pSrc );

/// Store Vector4 in StorageVector4
NOALIAS __forceinline void Store( const Vector4 PCREF data, StorageVector4 * const __restrict pDst );


/// Make Vector4 from broadcast VF32
NOALIAS __forceinline const Vector4 MakeVector4( const VF32 VECREF src );

/// Make Vector4 from Vec4F32
NOALIAS __forceinline const Vector4 MakeVector4( const Vec4F32 VECREF src );

/// Get Vec4F32 from Vector4
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Vector4 PCREF src );


/// Make Vector4 from Point3: Result W = 1.0f
NOALIAS __forceinline const Vector4 MakeVector4( const Point3 PCREF p3 );

/// Make Vector4 from Vector3: Result W = 0.0f
NOALIAS __forceinline const Vector4 MakeVector4( const Vector3 PCREF v3 );

/// Make Vector4 from Normal3: Result W = 0.0f
NOALIAS __forceinline const Vector4 MakeVector4( const Normal3 PCREF n3 );


/// Make Vector4 from 4 F32's
NOALIAS __forceinline const Vector4 MakeVector4( const F32& x, const F32& y, const F32& z, const F32& w );

/// Make Vector4 from 4 VF32's
NOALIAS __forceinline const Vector4 MakeVector4( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z, const VF32 VECREF w );

/// Make Vector4 from Vector3 & VF32: result.XYZ = xyz, result.W = w
NOALIAS __forceinline const Vector4 MakeVector4( const Vector3 PCREF xyz, const VF32 VECREF w );

/// Make Vector4 from VF32 & Vector3: result.X = x, result.YZW = yzw
NOALIAS __forceinline const Vector4 MakeVector4( const VF32 VECREF x, const Vector3 PCREF yzw );

/// Make Vector4 from a Vector5
NOALIAS __forceinline const Vector4 MakeVector4( const Vector5& v5 );


/// Vector4 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vector4 PCREF src );

/// Vector4 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vector4 PCREF src );

/// Vector4 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Vector4 PCREF src );

/// Vector4 Accessor: Get W
NOALIAS __forceinline const VF32 W( const Vector4 PCREF src );


/// Vector4 Accessor: Set X
NOALIAS __forceinline void SetX( Vector4* const __restrict pDst, const VF32 VECREF x );

/// Vector4 Accessor: Set Y
NOALIAS __forceinline void SetY( Vector4* const __restrict pDst, const VF32 VECREF y );

/// Vector4 Accessor: Set Z
NOALIAS __forceinline void SetZ( Vector4* const __restrict pDst, const VF32 VECREF z );

/// Vector4 Accessor: Set W
NOALIAS __forceinline void SetW( Vector4* const __restrict pDst, const VF32 VECREF w );


/// Vector4 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector4 PCREF v );

/// Vector4 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector4* const __restrict pDst, const VF32 VECREF ei );


/// Vector4 Write to Stream
template < typename T_Stream > void Serialise( const Vector4 PCREF src, T_Stream& stream );

/// Vector4 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Vector4* const __restrict pDst );



/// Vector4 Permute : Indices select 4 result elements [x,y,z,w] from [ 0:ax, 1:ay, 2:az, 3:aw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Vector4 Permute( const Vector4 PCREF a );

/// Vector4 Permute : Indices select 4 result elements [x,y,z,w] from [ 0:ax, 1:ay, 2:az, 3:aw, 4:bx, 5:by, 6:bz, 7:bw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Vector4 Permute( const Vector4 PCREF a, const Vector4 PCREF b );


/// Vector4 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:aw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vector3 Permute( const Vector4 PCREF a );

/// Vector4 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:aw, 4:bx, 5:by, 6:bz, 7:bw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vector3 Permute( const Vector4 PCREF a, const Vector4 PCREF b );


/// Vector4 MergeXY : Merges the x & y elements from a & b --> [ ax, bx, ay, by ]
NOALIAS __forceinline const Vector4 MergeXY( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 MergeZW : Merges the z & w elements from a & b --> [ az, bz, aw, bw ]
NOALIAS __forceinline const Vector4 MergeZW( const Vector4 PCREF a, const Vector4 PCREF b );



/// Vector4 Abs of each element
NOALIAS __forceinline const Vector4 Abs( const Vector4 PCREF v );

/// Vector4 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector4 Sign( const Vector4 PCREF v );

/// Vector4 CopySign of each element : Returns a Vector4 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector4 CopySign( const Vector4 PCREF magnitude, const Vector4 PCREF sign );


/// Vector4 Negation : Indices select elements to negate [X,Y,Z,W]
template < bool X, bool Y, bool Z, bool W >
NOALIAS __forceinline const Vector4 Negate( const Vector4 PCREF v );


/// Vector4 Negate X
NOALIAS __forceinline const Vector4 NegateX( const Vector4 PCREF src );

/// Vector4 Negate Y
NOALIAS __forceinline const Vector4 NegateY( const Vector4 PCREF src );

/// Vector4 Negate Z
NOALIAS __forceinline const Vector4 NegateZ( const Vector4 PCREF src );

/// Vector4 Negate W
NOALIAS __forceinline const Vector4 NegateW( const Vector4 PCREF src );


/// Vector4 Negate XY
NOALIAS __forceinline const Vector4 NegateXY( const Vector4 PCREF src );

/// Vector4 Negate XZ
NOALIAS __forceinline const Vector4 NegateXZ( const Vector4 PCREF src );

/// Vector4 Negate XW
NOALIAS __forceinline const Vector4 NegateXW( const Vector4 PCREF src );

/// Vector4 Negate YZ
NOALIAS __forceinline const Vector4 NegateYZ( const Vector4 PCREF src );

/// Vector4 Negate YW
NOALIAS __forceinline const Vector4 NegateYW( const Vector4 PCREF src );

/// Vector4 Negate ZW
NOALIAS __forceinline const Vector4 NegateZW( const Vector4 PCREF src );


/// Vector4 Negate XYZ
NOALIAS __forceinline const Vector4 NegateXYZ( const Vector4 PCREF src );

/// Vector4 Negate XYW
NOALIAS __forceinline const Vector4 NegateXYW( const Vector4 PCREF src );

/// Vector4 Negate XZW
NOALIAS __forceinline const Vector4 NegateXZW( const Vector4 PCREF src );

/// Vector4 Negate YZW
NOALIAS __forceinline const Vector4 NegateYZW( const Vector4 PCREF src );



/// Vector4 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Rcp( const Vector4 PCREF v );

/// Vector4 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Rcp24( const Vector4 PCREF v );

/// Vector4 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Div( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Div24( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Div( const VF32 VECREF s, const Vector4 PCREF v );

/// Vector4 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Div24( const VF32 VECREF s, const Vector4 PCREF v );

/// Vector4 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Div( const Vector4 PCREF v, const VF32 VECREF s );

/// Vector4 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Div24( const Vector4 PCREF v, const VF32 VECREF s );

/// Vector4 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 RcpSqrt( const Vector4 PCREF v );

/// Vector4 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 RcpSqrt24( const Vector4 PCREF v );

/// Vector4 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Sqrt( const Vector4 PCREF v );

/// Vector4 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Sqrt24( const Vector4 PCREF v );


/// Vector4 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vector4 PCREF v,
                                       Vector4* const __restrict pRcp,
                                       Vector4* const __restrict pSqrt,
                                       Vector4* const __restrict pRcpSqrt );

/// Vector4 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vector4 PCREF v,
                                         Vector4* const __restrict pRcp,
                                         Vector4* const __restrict pSqrt,
                                         Vector4* const __restrict pRcpSqrt );


/// Vector4 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vector4 PCREF v,
                                           Vector4* const __restrict pSqrt,
                                           Vector4* const __restrict pRcpSqrt );

/// Vector4 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vector4 PCREF v,
                                             Vector4* const __restrict pSqrt,
                                             Vector4* const __restrict pRcpSqrt );


/// Vector4 Unary Positive Operator
NOALIAS __forceinline const Vector4 operator + ( const Vector4 PCREF x );

/// Vector4 Unary Negation Operator
NOALIAS __forceinline const Vector4 operator - ( const Vector4 PCREF x );


/// Vector4 Sum
NOALIAS __forceinline const Vector4 operator + ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Difference
NOALIAS __forceinline const Vector4 operator - ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Multiplication Operator
NOALIAS __forceinline const Vector4 operator * ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Division Operator
NOALIAS __forceinline const Vector4 operator / ( const Vector4 PCREF a, const Vector4 PCREF b );


/// Vector4 Addition Update
NOALIAS __forceinline void operator += ( Vector4& a, const Vector4 PCREF b );

/// Vector4 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector4& a, const Vector4 PCREF b );

/// Vector4 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector4& a, const Vector4 PCREF b );

/// Vector4 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector4& a, const Vector4 PCREF b );


/// Vector4 Scalar Multiplication: Vector4 * VF32
NOALIAS __forceinline const Vector4 operator * ( const Vector4 PCREF v, const VF32 VECREF s );

/// Vector4 Scalar Multiplication: VF32 * Vector4
NOALIAS __forceinline const Vector4 operator * ( const VF32 VECREF s, const Vector4 PCREF v );

/// Vector4 Scalar Division: Vector4 / VF32
NOALIAS __forceinline const Vector4 operator / ( const Vector4 PCREF v, const VF32 VECREF s );

/// Vector4 Scalar Division: VF32 / Vector4
NOALIAS __forceinline const Vector4 operator / ( const VF32 VECREF s, const Vector4 PCREF v );


/// Vector4 Scalar Multiplication Update: Vector4 *= VF32
NOALIAS __forceinline void operator *= ( Vector4& v, const VF32 VECREF s );

/// Vector4 Scalar Division Update: Vector4 /= VF32
NOALIAS __forceinline void operator /= ( Vector4& v, const VF32 VECREF s );


/// Vector4 Matrix Multiplication: Vector4 * Mat43
NOALIAS __forceinline const Vector4 operator * ( const Vector4 PCREF v4, const Mat43 MATREF m43 );


/// Vector4 * Mat44 Multiplication operator : input = 4 element row vector, output = 4 element row vector
NOALIAS __forceinline const Vector4 operator * ( const Vector4 PCREF v4, const Mat44 MATREF m44 );

/// Mat44 * Vector4 Multiplication operator : input = 4 element col vector, output = 4 element col vector
NOALIAS __forceinline const Vector4 operator * ( const Mat44 MATREF m44, const Vector4 PCREF v4 );


/// Vector4 Matrix Multiplication Update: Vector4 *= Mat43
NOALIAS __forceinline void operator *= ( Vector4& v, const Mat43 MATREF m );

/// Vector4 Matrix Multiplication Update: Vector4 *= Mat44
NOALIAS __forceinline void operator *= ( Vector4& v, const Mat44 MATREF m );


/// Vector4 MulAdd : a * b + c
NOALIAS __forceinline const Vector4 MulAdd( const Vector4 PCREF a, const Vector4 PCREF b, const Vector4 PCREF c );

/// Vector4 MulSub : a * b - c
NOALIAS __forceinline const Vector4 MulSub( const Vector4 PCREF a, const Vector4 PCREF b, const Vector4 PCREF c );

/// Vector4 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector4 NegMulAdd( const Vector4 PCREF a, const Vector4 PCREF b, const Vector4 PCREF c );

/// Vector4 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector4 NegMulSub( const Vector4 PCREF a, const Vector4 PCREF b, const Vector4 PCREF c );


/// Vector4 MulAdd : a * b + c
NOALIAS __forceinline const Vector4 MulAdd( const Vector4 PCREF a, const VF32 VECREF b, const Vector4 PCREF c );

/// Vector4 MulSub : a * b - c
NOALIAS __forceinline const Vector4 MulSub( const Vector4 PCREF a, const VF32 VECREF b, const Vector4 PCREF c );

/// Vector4 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector4 NegMulAdd( const Vector4 PCREF a, const VF32 VECREF b, const Vector4 PCREF c );

/// Vector4 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector4 NegMulSub( const Vector4 PCREF a, const VF32 VECREF b, const Vector4 PCREF c );


/// Vector4 MulAdd : a * b + c : [ scalar * vector + vector ]
NOALIAS __forceinline const Vector4 MulAdd( const VF32 VECREF a, const Vector4 PCREF b, const Vector4 PCREF c );

/// Vector4 MulSub : a * b - c : [ scalar * vector - vector ]
NOALIAS __forceinline const Vector4 MulSub( const VF32 VECREF a, const Vector4 PCREF b, const Vector4 PCREF c );

/// Vector4 NegMulAdd : -( a * b + c ) : [ - ( scalar * vector + vector ) ]
NOALIAS __forceinline const Vector4 NegMulAdd( const VF32 VECREF a, const Vector4 PCREF b, const Vector4 PCREF c );

/// Vector4 NegMulSub : -( a * b - c ) == c - a * b : [ - ( scalar * vector - vector ) ]
NOALIAS __forceinline const Vector4 NegMulSub( const VF32 VECREF a, const Vector4 PCREF b, const Vector4 PCREF c );


/// Vector4 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vector4 Lerp( const Vector4 PCREF src, const Vector4 PCREF dst, const VF32 VECREF t );


/// Vector4 Min
NOALIAS __forceinline const Vector4 Min( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Max
NOALIAS __forceinline const Vector4 Max( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Min
NOALIAS __forceinline const Vector4 Min( const Vector4 PCREF a, const VF32 VECREF b );

/// Vector4 Max
NOALIAS __forceinline const Vector4 Max( const Vector4 PCREF a, const VF32 VECREF b );

/// Vector4 Min Update
NOALIAS __forceinline void Min( Vector4* const __restrict pA, const Vector4 PCREF b );

/// Vector4 Max Update
NOALIAS __forceinline void Max( Vector4* const __restrict pA, const Vector4 PCREF b );

/// Vector4 Min Update
NOALIAS __forceinline void Min( Vector4* const __restrict pA, const VF32 VECREF b );

/// Vector4 Max Update
NOALIAS __forceinline void Max( Vector4* const __restrict pA, const VF32 VECREF b );


/// Vector4 Clamp
NOALIAS __forceinline const Vector4 Clamp( const Vector4 PCREF a, const Vector4 PCREF lo, const Vector4 PCREF hi );

/// Vector4 Clamp
NOALIAS __forceinline const Vector4 Clamp( const Vector4 PCREF a, const VF32 VECREF lo, const VF32 VECREF hi );

/// Vector4 Clamp Update
NOALIAS __forceinline void Clamp( Vector4* const __restrict pA, const Vector4 PCREF lo, const Vector4 PCREF hi );

/// Vector4 Clamp Update
NOALIAS __forceinline void Clamp( Vector4* const __restrict pA, const VF32 VECREF lo, const VF32 VECREF hi );


/// Vector4 Round to integer towards minus infinity
NOALIAS __forceinline const Vector4 Floor( const Vector4 PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// Vector4 Round to integer towards plus infinity
NOALIAS __forceinline const Vector4 Ceil ( const Vector4 PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// Vector4 Round to integer towards nearest
NOALIAS __forceinline const Vector4 Round( const Vector4 PCREF a ) __attribute__(( /* pure, ? */ always_inline ));

/// Vector4 Round to integer towards zero
NOALIAS __forceinline const Vector4 Trunc( const Vector4 PCREF a ) __attribute__(( /* pure, ? */ always_inline ));


/// Vector4 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const Vector4 Mod( const Vector4 PCREF x, const Vector4 PCREF y );

/// Vector4 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const Vector4 Mod( const Vector4 PCREF x, const Vector4 PCREF y, const Vector4 PCREF rcp_y );

/// Vector4 Fractional Part : Returns fractional part of x, f = x - i [ x = i + f ], where i is an integer, i & f have the same sign as x
NOALIAS __forceinline const Vector4 Frac( const Vector4 PCREF x );

/// Vector4 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const Vector4 WrapToInterval( const Vector4 PCREF x, const Vector4 PCREF p );

/// Vector4 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const Vector4 WrapToInterval( const Vector4 PCREF x, const Vector4 PCREF p, const Vector4 PCREF rcp_p );

/// Vector4 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const Vector4 WrapToIntervalOffset( const Vector4 PCREF x, const Vector4 PCREF p, const Vector4 PCREF o );

/// Vector4 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const Vector4 WrapToIntervalOffset( const Vector4 PCREF x, const Vector4 PCREF p, const Vector4 PCREF rcp_p, const Vector4 PCREF o );


/// Vector4 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const Vector4 Mod( const Vector4 PCREF x, const VF32 VECREF y );

/// Vector4 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const Vector4 Mod( const Vector4 PCREF x, const VF32 VECREF y, const VF32 VECREF rcp_y );

/// Vector4 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const Vector4 WrapToInterval( const Vector4 PCREF x, const VF32 VECREF p );

/// Vector4 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const Vector4 WrapToInterval( const Vector4 PCREF x, const VF32 VECREF p, const VF32 VECREF rcp_p );

/// Vector4 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const Vector4 WrapToIntervalOffset( const Vector4 PCREF x, const VF32 VECREF p, const VF32 VECREF o );

/// Vector4 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const Vector4 WrapToIntervalOffset( const Vector4 PCREF x, const VF32 VECREF p, const VF32 VECREF rcp_p, const VF32 VECREF o );



/// Vector4 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w
NOALIAS __forceinline const VBool operator == ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w
NOALIAS __forceinline const VBool operator != ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator <= ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Comparison a >= b : Returns VBool representing results of a.i >= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator >= ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Comparison a < b : Returns VBool representing results of a.i < b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator < ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Comparison a > b : Returns VBool representing results of a.i > b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator > ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Ordered Test : true if neither a nor b contains elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Unordered Test : true if a or b contains elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Finite test
NOALIAS __forceinline const VBool Finite( const Vector4 PCREF src );


/// Vector4 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector4 Select( const VBool PCREF condition, const Vector4 PCREF result_if_true, const Vector4 PCREF result_if_false );

/// Vector4 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector4 Select( const Vec4Bool PCREF condition, const Vector4 PCREF result_if_true, const Vector4 PCREF result_if_false );


/// Vector4 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vector4 AND( const Vector4 PCREF a, const VBool PCREF b );

/// Vector4 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vector4 ANDC( const Vector4 PCREF a, const VBool PCREF b );


/// Vector4 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vector4 AND( const Vector4 PCREF a, const Vec4Bool PCREF b );

/// Vector4 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vector4 ANDC( const Vector4 PCREF a, const Vec4Bool PCREF b );


/// Vector4 dot product
NOALIAS __forceinline const VF32 Dot( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Vector4 PCREF a, const Vector4 PCREF b );

/// Vector4 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector4 PCREF v );


/// Vector4 Length (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length( const Vector4 PCREF v );

/// Vector4 Length (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length24( const Vector4 PCREF v );


/// Vector4 Set Length (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength( Vector4* const __restrict pDst, const VF32 VECREF length );

/// Vector4 Set Length (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength24( Vector4* const __restrict pDst, const VF32 VECREF length );


/// Vector4 Limit Length (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength( Vector4* const __restrict pV, const VF32 VECREF limit );

/// Vector4 Limit Length (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength24( Vector4* const __restrict pV, const VF32 VECREF limit );


/// Vector4 Normalise (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Normalise( const Vector4 PCREF v );

/// Vector4 Normalise (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Normalise24( const Vector4 PCREF v );


/// Vector4 Normalise (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Vector4* const __restrict pDst );

/// Vector4 Normalise (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Vector4* const __restrict pDst );


/// Vector4 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Inverse( const Vector4 PCREF v );

/// Vector4 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Inverse24( const Vector4 PCREF v );


/// Vector4 Flush To Zero: flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vector4 FlushToZero( const Vector4 PCREF v, const VF32 VECREF tolerance );

/// Vector4 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vector4 FlushElementsToZero( const Vector4 PCREF v, const VF32 VECREF tolerance );


/// Vector Space Tensor Product of a 4x1 column vector and a 1x4 row vector -> Mat44
NOALIAS __forceinline const Mat44 TensorProduct( const Vector4 PCREF lhs, const Vector4 PCREF rhs );


/// Vector4 Rotate Elements Left
template < int N >
NOALIAS __forceinline const Vector4 RotateLeft( const Vector4 PCREF a );

/// Vector4 Rotate Elements Right
template < int N >
NOALIAS __forceinline const Vector4 RotateRight( const Vector4 PCREF a );





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

