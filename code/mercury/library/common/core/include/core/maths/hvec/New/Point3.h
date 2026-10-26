#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_POINT3_H
#define SOFT_MATHS_HVEC_NEW_POINT3_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_POINT3_H
#define MATHS_HVEC_NEW_POINT3_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Point3
/// @{

/// Load Point3 from StoragePoint3
NOALIAS __forceinline const Point3 Load( StoragePoint3 const * const __restrict pSrc );

/// Store Point3 in StoragePoint3
NOALIAS __forceinline void Store( const Point3 PCREF data, StoragePoint3 * const __restrict pDst );



/// Make Point3 from Vec4F32
NOALIAS __forceinline const Point3 MakePoint3( const Vec4F32 VECREF src );

/// Get Vec4F32 from Point3
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Point3 PCREF src );


/// Make Point3 from Vector3
NOALIAS __forceinline const Point3 MakePoint3( const Vector3 PCREF v3 );

/// Make Point3 from Normal3
NOALIAS __forceinline const Point3 MakePoint3( const Normal3 PCREF n3 );

/// Make Point3 from Vector4: returns ( x/w, y/w, z/w )
//NOALIAS __forceinline const Point3 MakePoint3( const Vector4 PCREF v4 );


/// Make Point3 from 3 F32's
NOALIAS __forceinline const Point3 MakePoint3( const F32& x, const F32& y, const F32& z );

/// Make Point3 from 3 VF32's
NOALIAS __forceinline const Point3 MakePoint3( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );



/// Point3 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Point3 PCREF src );

/// Point3 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Point3 PCREF src );

/// Point3 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Point3 PCREF src );

/// Point3 Accessor: Set X
NOALIAS __forceinline void SetX( Point3* const __restrict pDst, const VF32 VECREF x );

/// Point3 Accessor: Set Y
NOALIAS __forceinline void SetY( Point3* const __restrict pDst, const VF32 VECREF y );

/// Point3 Accessor: Set Z
NOALIAS __forceinline void SetZ( Point3* const __restrict pDst, const VF32 VECREF z );



/// Point3 Write to Stream
template < typename T_Stream >
void Serialise( const Point3 PCREF src, T_Stream& stream );

/// Point3 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, Point3* const __restrict pDst );



/// Point3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Point3 Permute( const Point3 PCREF a );

/// Point3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:bx, 4:by, 5:bz ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Point3 Permute( const Point3 PCREF a, const Point3 PCREF b );



/// Point3 Sum
NOALIAS __forceinline const Point3 operator + ( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Difference
NOALIAS __forceinline const Vector3 operator - ( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Addition Update
NOALIAS __forceinline void operator += ( Point3& a, const Point3 PCREF b );


/// Point3 Scalar Multiplication: Point3 * VF32
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF p, const VF32 VECREF s );

/// Point3 Scalar Multiplication: VF32 * Point3
NOALIAS __forceinline const Point3 operator * ( const VF32 VECREF s, const Point3 PCREF p );

/// Point3 Scalar Division: Point3 / VF32
NOALIAS __forceinline const Point3 operator / ( const Point3 PCREF p, const VF32 VECREF s );

/// Point3 Scalar Multiplication Update: Point3 *= VF32
NOALIAS __forceinline void operator *= ( Point3& p, const VF32 VECREF s );

/// Point3 Scalar Division Update: Point3 /= VF32
NOALIAS __forceinline void operator /= ( Point3& p, const VF32 VECREF s );


/// Point3 Unary Positive Operator
NOALIAS __forceinline const Point3 operator + ( const Point3 PCREF x );

/// Point3 Unary Negation Operator
NOALIAS __forceinline const Point3 operator - ( const Point3 PCREF x );

/// Point3 Addition: Point3 + Vector3
NOALIAS __forceinline const Point3 operator + ( const Point3 PCREF p, const Vector3 PCREF v );

/// Point3 Subtraction: Point3 - Vector3
NOALIAS __forceinline const Point3 operator - ( const Point3 PCREF p, const Vector3 PCREF v );

/// Point3 Addition Update: Point3 += Vector3
NOALIAS __forceinline void operator += ( Point3& p, const Vector3 PCREF v );

/// Point3 Subtraction Update: Point3 -= Vector3
NOALIAS __forceinline void operator -= ( Point3& p, const Vector3 PCREF v );


/// Point3 Addition: Point3 + Normal3
NOALIAS __forceinline const Point3 operator + ( const Point3 PCREF p, const Normal3 PCREF n );

/// Point3 Subtraction: Point3 - Normal3
NOALIAS __forceinline const Point3 operator - ( const Point3 PCREF p, const Normal3 PCREF n );

/// Point3 Addition Update: Point3 += Normal3
NOALIAS __forceinline void operator += ( Point3& p, const Normal3 PCREF n );

/// Point3 Subtraction Update: Point3 -= Normal3
NOALIAS __forceinline void operator -= ( Point3& p, const Normal3 PCREF n );


/// Point3 MulAdd : a * b + c
NOALIAS __forceinline const Point3 MulAdd( const Vector3 PCREF a, const VF32 VECREF b, const Point3 PCREF c );

/// Point3 MulSub : a * b - c
NOALIAS __forceinline const Point3 MulSub( const Vector3 PCREF a, const VF32 VECREF b, const Point3 PCREF c );

/// Point3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Point3 NegMulAdd( const Vector3 PCREF a, const VF32 VECREF b, const Point3 PCREF c );

/// Point3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Point3 NegMulSub( const Vector3 PCREF a, const VF32 VECREF b, const Point3 PCREF c );


/// Point3 MulAdd : a * b + c
NOALIAS __forceinline const Point3 MulAdd( const Normal3 PCREF a, const VF32 VECREF b, const Point3 PCREF c );

/// Point3 MulSub : a * b - c
NOALIAS __forceinline const Point3 MulSub( const Normal3 PCREF a, const VF32 VECREF b, const Point3 PCREF c );

/// Point3 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Point3 NegMulAdd( const Normal3 PCREF a, const VF32 VECREF b, const Point3 PCREF c );

/// Point3 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Point3 NegMulSub( const Normal3 PCREF a, const VF32 VECREF b, const Point3 PCREF c );


/// Point3 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z
NOALIAS __forceinline const VBool operator == ( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z
NOALIAS __forceinline const VBool operator != ( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator <= ( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Comparison a >= b : Returns VBool representing results of a.i >= b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator >= ( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Comparison a < b : Returns VBool representing results of a.i < b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator < ( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Comparison a > b : Returns VBool representing results of a.i > b.i, for each element i [x,y,z] (W is undefined)
NOALIAS __forceinline const Vec4Bool operator > ( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Ordered Test : true if neither a nor b contains elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Point3 PCREF a, const Point3 PCREF b );

/// Point3 Unordered Test : true if a or b contains elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Point3 PCREF a, const Point3 PCREF b );



/// Point3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Point3 Select( const VBool PCREF condition, const Point3 PCREF result_if_true, const Point3 PCREF result_if_false );

/// Point3 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Point3 Select( const Vec4Bool PCREF condition, const Point3 PCREF result_if_true, const Point3 PCREF result_if_false );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

