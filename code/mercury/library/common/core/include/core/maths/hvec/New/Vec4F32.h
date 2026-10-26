#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC4F32_H
#define SOFT_MATHS_HVEC_NEW_VEC4F32_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC4F32_H
#define MATHS_HVEC_NEW_VEC4F32_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vec4F32
/// @{

/// Load Vec4F32 from an F32 array
template <>
NOALIAS __forceinline const Vec4F32 Load( F32 const * const __restrict pSrc );

/// Store Vec4F32 into an F32 array
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, F32* const __restrict pDst );


/// Load Vec4F32 from a Float1
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float1 const * const __restrict pSrc );

/// Store Vec4F32 into a Float1
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float1* const __restrict pDst );

/// Load Vec4F32 from a Float2
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float2 const * const __restrict pSrc );

/// Store Vec4F32 into a Float2
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float2* const __restrict pDst );

/// Load Vec4F32 from a Float3
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float3 const * const __restrict pSrc );

/// Store Vec4F32 into a Float3
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float3* const __restrict pDst );

/// Load Vec4F32 from a Float4
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float4 const * const __restrict pSrc );

/// Store Vec4F32 into a Float4
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float4* const __restrict pDst );


/// Load Vec4F32 from a Float2A16
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float2A16 const * const __restrict pSrc );

/// Store Vec4F32 into a Float2A16
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float2A16* const __restrict pDst );

/// Load Vec4F32 from a Float3A16
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float3A16 const * const __restrict pSrc );

/// Store Vec4F32 into a Float3A16
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float3A16* const __restrict pDst );

/// Load Vec4F32 from a Float4A16
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float4A16 const * const __restrict pSrc );

/// Store Vec4F32 into a Float4A16
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float4A16* const __restrict pDst );


/// StorageVec1F32: Load Vec4F32
template <>
NOALIAS __forceinline const Vec4F32 Load( StorageVec1F32 const * const __restrict pSrc );

/// StorageVec1F32: Store Vec4F32
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec1F32 * const __restrict pDst );

/// StorageVec2F32: Load Vec4F32
template <>
NOALIAS __forceinline const Vec4F32 Load( StorageVec2F32 const * const __restrict pSrc );

/// StorageVec2F32: Store Vec4F32
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec2F32 * const __restrict pDst );

/// StorageVec3F32: Load Vec4F32
template <>
NOALIAS __forceinline const Vec4F32 Load( StorageVec3F32 const * const __restrict pSrc );

/// StorageVec3F32: Store Vec4F32
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec3F32 * const __restrict pDst );

/// StorageVec4F32: Load Vec4F32
template <>
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32 const * const __restrict pSrc );

/// StorageVec4F32: Store Vec4F32
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32 * const __restrict pDst );



/// Load Vec4F32 from a Fixed3_S16
template < int T_Precision >
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Fixed3_S16<T_Precision> const * const __restrict pSrc );

/// Store Vec4F32 into a Fixed3_S16
template < int T_Precision >
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, MathsStorage::Fixed3_S16<T_Precision> * const __restrict pDst );

/// Load Vec4F32 from a Fixed4_S16
template < int T_Precision >
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Fixed4_S16<T_Precision> const * const __restrict pSrc );

/// Store Vec4F32 into a Fixed4_S16
template < int T_Precision >
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, MathsStorage::Fixed4_S16<T_Precision> * const __restrict pDst );


/// Make Vec4F32 from a Vec4S32
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Vec4S32 PCREF a );


/// Vec4F32: Load from StorageVec4F32_U8
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_U8 const * const __restrict pSrc );

/// Vec4F32: Store to StorageVec4F32_U8
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_U8 * const __restrict pDst );

/// Vec4F32: Load from StorageVec4F32_U8N
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_U8N const * const __restrict pSrc );

/// Vec4F32: Store to StorageVec4F32_U8N
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_U8N * const __restrict pDst );

/// Vec4F32: Load from StorageVec4F32_S16
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_S16 const * const __restrict pSrc );

/// Vec4F32: Store to StorageVec4F32_S16
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_S16 * const __restrict pDst );

/// Vec4F32: Load from StorageVec4F32_S16N
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_S16N const * const __restrict pSrc );

/// Vec4F32: Store to StorageVec4F32_S16N
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_S16N * const __restrict pDst );

/// Vec4F32: Load from StorageVec4F32_HenD3
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_HenD3 const * const __restrict pSrc );

/// Vec4F32: Store to StorageVec4F32_HenD3
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_HenD3 * const __restrict pDst );

/// Vec4F32: Load from StorageVec4F32_F16
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_F16 const * const __restrict pSrc );

/// Vec4F32: Store to StorageVec4F32_F16
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_F16 * const __restrict pDst );





/// Make Vec4F32 from an F32 with the same value broadcast into all elements
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const F32& x );

/// Make Vec4F32 from F32 XY [ Z & W are UNDEFINED! ]
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const F32& x, const F32& y );

/// Make Vec4F32 from F32 XYZ [ W is UNDEFINED! ]
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const F32& x, const F32& y, const F32& z );

/// Make Vec4F32 from F32 XYZW
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const F32& x, const F32& y, const F32& z, const F32& w );



/// Make Vec4F32 from a VF32 with the same value broadcast into all elements
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x );

/// Make Vec4F32 from VF32 XY [ Z & W are UNDEFINED! ]
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x, const VF32 VECREF y );

/// Make Vec4F32 from VF32 XYZ [ W is UNDEFINED! ]
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

/// Make Vec4F32 from VF32 XYZW
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z, const VF32 VECREF w );


/// Make Vec4F32 from Point3
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Point3 PCREF p3 );

/// Make Vec4F32 from Vector3
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Vector3 PCREF v3 );

/// Make Vec4F32 from Vector3 xyz & VF32 w
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Vector3 PCREF xyz, const VF32 VECREF w );

/// Make Vec4F32 from Point3 xyz & VF32 w
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Point3 PCREF xyz, const VF32 VECREF w );

/// Make Vec4F32 from Normal3 xyz & VF32 w
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Normal3 PCREF xyz, const VF32 VECREF w );

/// Make Vec4F32 from VF32 x & Vector3 yzw
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x, const Vector3 PCREF yzw );

/// Make Vec4F32 from Vector4
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Vector4 PCREF v4 );

/// Make Vec4F32 from Normal3
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Normal3 PCREF n3 );


/// Vec4F32 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vec4F32 VECREF v );

/// Vec4F32 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vec4F32 VECREF v );

/// Vec4F32 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Vec4F32 VECREF v );

/// Vec4F32 Accessor: Get W
NOALIAS __forceinline const VF32 W( const Vec4F32 VECREF v );

/// Vec4F32 Accessor: Set X
NOALIAS __forceinline void SetX( Vec4F32* const __restrict pDst, const VF32 VECREF x );

/// Vec4F32 Accessor: Set Y
NOALIAS __forceinline void SetY( Vec4F32* const __restrict pDst, const VF32 VECREF y );

/// Vec4F32 Accessor: Set Z
NOALIAS __forceinline void SetZ( Vec4F32* const __restrict pDst, const VF32 VECREF z );

/// Vec4F32 Accessor: Set W
NOALIAS __forceinline void SetW( Vec4F32* const __restrict pDst, const VF32 VECREF w );


/// Vec4F32 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vec4F32 VECREF v );

/// Vec4F32 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vec4F32* const __restrict pDst, const VF32 VECREF ei );



/// Vec4F32 Write to Stream
template < typename T_Stream > void Serialise( const Vec4F32 VECREF src, T_Stream& stream );

/// Vec4F32 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Vec4F32* const __restrict pDst );



/// Vec4F32 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az ], W element is preserved
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vec4F32 Permute( const Vec4F32 VECREF v );

/// Vec4F32 Permute : Indices select 4 result elements [x,y,z,w] from [ 0:ax, 1:ay, 2:az, 3:aw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Vec4F32 Permute( const Vec4F32 VECREF v );

/// Vec4F32 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:aw, 4:bx, 5:by, 6:bz, 7:bw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vec4F32 Permute( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Permute : Indices select 4 result elements [x,y,z,w] from [ 0:ax, 1:ay, 2:az, 3:aw, 4:bx, 5:by, 6:bz, 7:bw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Vec4F32 Permute( const Vec4F32 VECREF a, const Vec4F32 VECREF b );


/// Vec4F32 MergeXY : Merges the x & y elements from a & b --> [ ax, bx, ay, by ]
NOALIAS __forceinline const Vec4F32 MergeXY( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 MergeZW : Merges the z & w elements from a & b --> [ az, bz, aw, bw ]
NOALIAS __forceinline const Vec4F32 MergeZW( const Vec4F32 VECREF a, const Vec4F32 VECREF b );


/// Vec4F32 Abs of each element
NOALIAS __forceinline const Vec4F32 Abs( const Vec4F32 VECREF v );

/// Vec4F32 Sign of each element : For each x in v: returns -1.0f if x < 0.0f, +1.0f if x > 0.0f, or 0.0f if x == 0.0f 
NOALIAS __forceinline const Vec4F32 Sign( const Vec4F32 VECREF v );

/// Vec4F32 CopySign of each element : Returns a Vec4F32 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vec4F32 CopySign( const Vec4F32 VECREF magnitude, const Vec4F32 VECREF sign );


/// Vec4F32 Negation : Indices select elements to negate [X,Y,Z,W]
template < bool X, bool Y, bool Z, bool W >
NOALIAS __forceinline const Vec4F32 Negate( const Vec4F32 VECREF v );


/// Vec4F32 Negate X
NOALIAS __forceinline const Vec4F32 NegateX( const Vec4F32 VECREF src );

/// Vec4F32 Negate Y
NOALIAS __forceinline const Vec4F32 NegateY( const Vec4F32 VECREF src );

/// Vec4F32 Negate Z
NOALIAS __forceinline const Vec4F32 NegateZ( const Vec4F32 VECREF src );

/// Vec4F32 Negate W
NOALIAS __forceinline const Vec4F32 NegateW( const Vec4F32 VECREF src );


/// Vec4F32 Negate XY
NOALIAS __forceinline const Vec4F32 NegateXY( const Vec4F32 VECREF src );

/// Vec4F32 Negate XZ
NOALIAS __forceinline const Vec4F32 NegateXZ( const Vec4F32 VECREF src );

/// Vec4F32 Negate XW
NOALIAS __forceinline const Vec4F32 NegateXW( const Vec4F32 VECREF src );

/// Vec4F32 Negate YZ
NOALIAS __forceinline const Vec4F32 NegateYZ( const Vec4F32 VECREF src );

/// Vec4F32 Negate YW
NOALIAS __forceinline const Vec4F32 NegateYW( const Vec4F32 VECREF src );

/// Vec4F32 Negate ZW
NOALIAS __forceinline const Vec4F32 NegateZW( const Vec4F32 VECREF src );


/// Vec4F32 Negate XYZ
NOALIAS __forceinline const Vec4F32 NegateXYZ( const Vec4F32 VECREF src );

/// Vec4F32 Negate XYW
NOALIAS __forceinline const Vec4F32 NegateXYW( const Vec4F32 VECREF src );

/// Vec4F32 Negate XZW
NOALIAS __forceinline const Vec4F32 NegateXZW( const Vec4F32 VECREF src );

/// Vec4F32 Negate YZW
NOALIAS __forceinline const Vec4F32 NegateYZW( const Vec4F32 VECREF src );


/// Vec4F32 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Rcp( const Vec4F32 VECREF v );

/// Vec4F32 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Rcp24( const Vec4F32 VECREF v );

/// Vec4F32 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Div( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Div24( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Div( const VF32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Div24( const VF32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Div( const Vec4F32 VECREF x, const VF32 VECREF y );

/// Vec4F32 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Div24( const Vec4F32 VECREF x, const VF32 VECREF y );

/// Vec4F32 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 RcpSqrt( const Vec4F32 VECREF v );

/// Vec4F32 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 RcpSqrt24( const Vec4F32 VECREF v );

/// Vec4F32 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Sqrt( const Vec4F32 VECREF v );

/// Vec4F32 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Sqrt24( const Vec4F32 VECREF v );

/// Vec4F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vec4F32 VECREF v,
                                       Vec4F32* const __restrict pRcp,
                                       Vec4F32* const __restrict pSqrt,
                                       Vec4F32* const __restrict pRcpSqrt );

/// Vec4F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vec4F32 VECREF v,
                                         Vec4F32* const __restrict pRcp,
                                         Vec4F32* const __restrict pSqrt,
                                         Vec4F32* const __restrict pRcpSqrt );


/// Vec4F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vec4F32 VECREF v,
                                           Vec4F32* const __restrict pSqrt,
                                           Vec4F32* const __restrict pRcpSqrt );

/// Vec4F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vec4F32 VECREF v,
                                             Vec4F32* const __restrict pSqrt,
                                             Vec4F32* const __restrict pRcpSqrt );


/// Vec4F32 Unary Positive Operator
NOALIAS __forceinline const Vec4F32 operator + ( const Vec4F32 VECREF x );

/// Vec4F32 Unary Negation Operator
NOALIAS __forceinline const Vec4F32 operator - ( const Vec4F32 VECREF x );


/// Vec4F32 Binary Addition Operator
NOALIAS __forceinline const Vec4F32 operator + ( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Binary Subtraction Operator
NOALIAS __forceinline const Vec4F32 operator - ( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Binary Multiplication Operator
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Binary Division Operator
NOALIAS __forceinline const Vec4F32 operator / ( const Vec4F32 VECREF x, const Vec4F32 VECREF y );


/// Vec4F32 Add Assign Operator
NOALIAS __forceinline const Vec4F32& operator += ( Vec4F32& x, const Vec4F32 VECREF y );

/// Vec4F32 Subtract Assign Operator
NOALIAS __forceinline const Vec4F32& operator -= ( Vec4F32& x, const Vec4F32 VECREF y );

/// Vec4F32 Multiply Assign Operator
NOALIAS __forceinline const Vec4F32& operator *= ( Vec4F32& x, const Vec4F32 VECREF y );

/// Vec4F32 Divide Assign Operator
NOALIAS __forceinline const Vec4F32& operator /= ( Vec4F32& x, const Vec4F32 VECREF y );


/// Vec4F32 Binary Multiplication Operator : vector * scalar
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF x, const VF32 VECREF y );

/// Vec4F32 Binary Multiplication Operator : scalar * vector
NOALIAS __forceinline const Vec4F32 operator * ( const VF32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Binary Division Operator : vector * scalar
NOALIAS __forceinline const Vec4F32 operator / ( const Vec4F32 VECREF x, const VF32 VECREF y );

/// Vec4F32 Binary Division Operator : scalar * vector
NOALIAS __forceinline const Vec4F32 operator / ( const VF32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Multiply Assign Operator : vector *= scalar
NOALIAS __forceinline const Vec4F32& operator *= ( Vec4F32& x, const VF32 VECREF y );

/// Vec4F32 Divide Assign Operator : vector / scalar
NOALIAS __forceinline const Vec4F32& operator /= ( Vec4F32& x, const VF32 VECREF y );


/// Vec4F32 MulAdd : a * b + c
NOALIAS __forceinline const Vec4F32 MulAdd( const Vec4F32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 MulSub : a * b - c
NOALIAS __forceinline const Vec4F32 MulSub( const Vec4F32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vec4F32 NegMulAdd( const Vec4F32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vec4F32 NegMulSub( const Vec4F32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c );


/// Vec4F32 MulAdd : a * b + c : [ vector * scalar + vector ]
NOALIAS __forceinline const Vec4F32 MulAdd( const Vec4F32 VECREF a, const VF32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 MulSub : a * b - c : [ vector * scalar - vector ]
NOALIAS __forceinline const Vec4F32 MulSub( const Vec4F32 VECREF a, const VF32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 NegMulAdd : -( a * b + c ) : [ - ( vector * scalar + vector ) ]
NOALIAS __forceinline const Vec4F32 NegMulAdd( const Vec4F32 VECREF a, const VF32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( vector * scalar - vector ) ]
NOALIAS __forceinline const Vec4F32 NegMulSub( const Vec4F32 VECREF a, const VF32 VECREF b, const Vec4F32 VECREF c );


/// Vec4F32 MulAdd : a * b + c : [ scalar * vector + vector ]
NOALIAS __forceinline const Vec4F32 MulAdd( const VF32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 MulSub : a * b - c : [ scalar * vector - vector ]
NOALIAS __forceinline const Vec4F32 MulSub( const VF32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 NegMulAdd : -( a * b + c ) : [ - ( scalar * vector + vector ) ]
NOALIAS __forceinline const Vec4F32 NegMulAdd( const VF32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c );

/// Vec4F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( scalar * vector - vector ) ]
NOALIAS __forceinline const Vec4F32 NegMulSub( const VF32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c );


/// Vec4F32 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vec4F32 Lerp( const Vec4F32 VECREF src, const Vec4F32 VECREF dst, const VF32 VECREF t );



/// Vec4F32 Min
NOALIAS __forceinline const Vec4F32 Min( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Max
NOALIAS __forceinline const Vec4F32 Max( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Min
NOALIAS __forceinline const Vec4F32 Min( const Vec4F32 VECREF x, const VF32 VECREF y );

/// Vec4F32 Max
NOALIAS __forceinline const Vec4F32 Max( const Vec4F32 VECREF x, const VF32 VECREF y );

/// Vec4F32 Min Update
DEPRECATED NOALIAS __forceinline const Vec4F32& MinEq( Vec4F32* const __restrict pX, const Vec4F32 VECREF y );

/// Vec4F32 Max Update
DEPRECATED NOALIAS __forceinline const Vec4F32& MaxEq( Vec4F32* const __restrict pX, const Vec4F32 VECREF y );

/// Vec4F32 Min
NOALIAS __forceinline void Min( Vec4F32* const __restrict pX, const Vec4F32 VECREF y );

/// Vec4F32 Max
NOALIAS __forceinline void Max( Vec4F32* const __restrict pX, const Vec4F32 VECREF y );

/// Vec4F32 Min
NOALIAS __forceinline void Min( Vec4F32* const __restrict pX, const VF32 VECREF y );

/// Vec4F32 Max
NOALIAS __forceinline void Max( Vec4F32* const __restrict pX, const VF32 VECREF y );


/// Vec4F32 Clamp
NOALIAS __forceinline const Vec4F32 Clamp( const Vec4F32 VECREF x, const Vec4F32 VECREF lo, const Vec4F32 VECREF hi );

/// Vec4F32 Clamp
NOALIAS __forceinline const Vec4F32 Clamp( const Vec4F32 VECREF x, const VF32 VECREF lo, const VF32 VECREF hi );

/// Vec4F32 Clamp Update
DEPRECATED NOALIAS __forceinline const Vec4F32& ClampEq( Vec4F32* const __restrict pX, const Vec4F32 VECREF lo, const Vec4F32 VECREF hi );

/// Vec4F32 Clamp
NOALIAS __forceinline void Clamp( Vec4F32* const __restrict pX, const Vec4F32 VECREF lo, const Vec4F32 VECREF hi );

/// Vec4F32 Clamp
NOALIAS __forceinline void Clamp( Vec4F32* const __restrict pX, const VF32 VECREF lo, const VF32 VECREF hi );


/// Vec4F32 Round to integer towards minus infinity
NOALIAS __forceinline const Vec4F32 Floor( const Vec4F32 VECREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec4F32 Round to integer towards plus infinity
NOALIAS __forceinline const Vec4F32 Ceil ( const Vec4F32 VECREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec4F32 Round to integer towards nearest
NOALIAS __forceinline const Vec4F32 Round( const Vec4F32 VECREF x ) __attribute__(( /* pure, ? */ always_inline ));

/// Vec4F32 Round to integer towards zero
NOALIAS __forceinline const Vec4F32 Trunc( const Vec4F32 VECREF x ) __attribute__(( /* pure, ? */ always_inline ));


/// Vec4F32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const Vec4F32 Mod( const Vec4F32 VECREF x, const Vec4F32 VECREF y );

/// Vec4F32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const Vec4F32 Mod( const Vec4F32 VECREF x, const Vec4F32 VECREF y, const Vec4F32 VECREF rcp_y );

/// Vec4F32 Fractional Part : Returns fractional part of x, f = x - i [ x = i + f ], where i is an integer, i & f have the same sign as x
NOALIAS __forceinline const Vec4F32 Frac( const Vec4F32 VECREF x );

/// Vec4F32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const Vec4F32 WrapToInterval( const Vec4F32 VECREF x, const Vec4F32 VECREF p );

/// Vec4F32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const Vec4F32 WrapToInterval( const Vec4F32 VECREF x, const Vec4F32 VECREF p, const Vec4F32 VECREF rcp_p );

/// Vec4F32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const Vec4F32 WrapToIntervalOffset( const Vec4F32 VECREF x, const Vec4F32 VECREF p, const Vec4F32 VECREF o );

/// Vec4F32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const Vec4F32 WrapToIntervalOffset( const Vec4F32 VECREF x, const Vec4F32 VECREF p, const Vec4F32 VECREF rcp_p, const Vec4F32 VECREF o );


/// Vec4F32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const Vec4F32 Mod( const Vec4F32 VECREF x, const VF32 VECREF y );

/// Vec4F32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const Vec4F32 Mod( const Vec4F32 VECREF x, const VF32 VECREF y, const VF32 VECREF rcp_y );

/// Vec4F32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const Vec4F32 WrapToInterval( const Vec4F32 VECREF x, const VF32 VECREF p );

/// Vec4F32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const Vec4F32 WrapToInterval( const Vec4F32 VECREF x, const VF32 VECREF p, const VF32 VECREF rcp_p );

/// Vec4F32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const Vec4F32 WrapToIntervalOffset( const Vec4F32 VECREF x, const VF32 VECREF p, const VF32 VECREF o );

/// Vec4F32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const Vec4F32 WrapToIntervalOffset( const Vec4F32 VECREF x, const VF32 VECREF p, const VF32 VECREF rcp_p, const VF32 VECREF o );




/// Vec4F32 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w
NOALIAS __forceinline const VBool operator == ( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w
NOALIAS __forceinline const VBool operator != ( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator <= ( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Comparison a >= b : Returns Vec4Bool representing results of a.i >= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator >= ( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Comparison a < b : Returns Vec4Bool representing results of a.i < b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator < ( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Comparison a > b : Returns Vec4Bool representing results of a.i > b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator > ( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vec4F32 VECREF a, const Vec4F32 VECREF b );


/// Vec4F32 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vec4F32 VECREF a );

/// Vec4F32 Finite Test : true if a contains no elements that are NaN's and for each element i [x,y,z,w], abs(x.i) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vec4F32 VECREF a );


/// Vec4F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec4F32 Select( const VBool PCREF condition, const Vec4F32 VECREF result_if_true, const Vec4F32 VECREF result_if_false );

/// Vec4F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec4F32 Select( const Vec4Bool PCREF condition, const Vec4F32 VECREF result_if_true, const Vec4F32 VECREF result_if_false );


/// Vec4F32 Bitwise NOT: ~a
NOALIAS __forceinline const Vec4F32 NOT( const Vec4F32 VECREF a );

/// Vec4F32 Bitwise OR: a | b
NOALIAS __forceinline const Vec4F32 OR( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Bitwise AND: a & b
NOALIAS __forceinline const Vec4F32 AND( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const Vec4F32 XOR( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Bitwise NOR: ~(a | b)
NOALIAS __forceinline const Vec4F32 NOR( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Bitwise NAND: ~(a & b)
NOALIAS __forceinline const Vec4F32 NAND( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const Vec4F32 EQV( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Bitwise ORC: a | ~b
NOALIAS __forceinline const Vec4F32 ORC( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 Bitwise ANDC: a & ~b
NOALIAS __forceinline const Vec4F32 ANDC( const Vec4F32 VECREF a, const Vec4F32 VECREF b );


/// Vec4F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec4F32 AND( const Vec4F32 VECREF a, const VBool PCREF b );

/// Vec4F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec4F32 ANDC( const Vec4F32 VECREF a, const VBool PCREF b );


/// Vec4F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec4F32 AND( const Vec4F32 VECREF a, const Vec4Bool PCREF b );

/// Vec4F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec4F32 ANDC( const Vec4F32 VECREF a, const Vec4Bool PCREF b );


/// Vec4F32 2 element dot product
NOALIAS __forceinline const VF32 Dot2( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 3 element dot product
NOALIAS __forceinline const VF32 Dot3( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 4 element dot product
NOALIAS __forceinline const VF32 Dot4( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 2 element cross product
NOALIAS __forceinline const VF32 Cross2( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 3 element cross product. W element of result is UNDEFINED!
NOALIAS __forceinline const Vec4F32 Cross3( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 3 element dot product operator [ More commonly used than Dot4 and matches the Cross3 operator ]
NOALIAS __forceinline const VF32 operator | ( const Vec4F32 VECREF a, const Vec4F32 VECREF b );

/// Vec4F32 3 element cross product operator. W element of result is UNDEFINED!
NOALIAS __forceinline const Vec4F32 operator ^ ( const Vec4F32 VECREF a, const Vec4F32 VECREF b );


/// Vec4F32 Vector Length Squared (2d)
NOALIAS __forceinline const VF32 LengthSquared2( const Vec4F32 VECREF a );

/// Vec4F32 Vector Length Squared (3d)
NOALIAS __forceinline const VF32 LengthSquared3( const Vec4F32 VECREF a );

/// Vec4F32 Vector Length Squared (4d)
NOALIAS __forceinline const VF32 LengthSquared4( const Vec4F32 VECREF a );


/// Vec4F32 Vector Length (2d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length2( const Vec4F32 VECREF a );

/// Vec4F32 Vector Length (3d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length3( const Vec4F32 VECREF a );

/// Vec4F32 Vector Length (4d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length4( const Vec4F32 VECREF a );


/// Vec4F32 Vector Length (2d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length2_24( const Vec4F32 VECREF a );

/// Vec4F32 Vector Length (3d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length3_24( const Vec4F32 VECREF a );

/// Vec4F32 Vector Length (4d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length4_24( const Vec4F32 VECREF a );


/// Vec4F32 Vector Reciprocal Length (2d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpLength2( const Vec4F32 VECREF a );

/// Vec4F32 Vector Reciprocal Length (3d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpLength3( const Vec4F32 VECREF a );

/// Vec4F32 Vector Reciprocal Length (4d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpLength4( const Vec4F32 VECREF a );


/// Vec4F32 Vector Reciprocal Length (2d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpLength2_24( const Vec4F32 VECREF a );

/// Vec4F32 Vector Reciprocal Length (3d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpLength3_24( const Vec4F32 VECREF a );

/// Vec4F32 Vector Reciprocal Length (4d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpLength4_24( const Vec4F32 VECREF a );


/// Vec4F32 Normalise (2d) (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise2( const Vec4F32 VECREF a );

/// Vec4F32 Normalise (3d) (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise3( const Vec4F32 VECREF a );

/// Vec4F32 Normalise (4d) (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise4( const Vec4F32 VECREF a );


/// Vec4F32 Normalise (2d) (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise2_24( const Vec4F32 VECREF a );

/// Vec4F32 Normalise (3d) (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise3_24( const Vec4F32 VECREF a );

/// Vec4F32 Normalise (4d) (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise4_24( const Vec4F32 VECREF a );


/// Vec4F32 Normalise (2d). Returns zeroLengthResult if length of a is zero. (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise2( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult );

/// Vec4F32 Normalise (3d). Returns zeroLengthResult if length of a is zero. (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise3( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult );

/// Vec4F32 Normalise (4d). Returns zeroLengthResult if length of a is zero. (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise4( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult );


/// Vec4F32 Normalise (2d). Returns zeroLengthResult if length of a is zero. (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise2_24( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult );

/// Vec4F32 Normalise (3d). Returns zeroLengthResult if length of a is zero. (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise3_24( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult );

/// Vec4F32 Normalise (4d). Returns zeroLengthResult if length of a is zero. (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise4_24( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult );


/// Vec4F32 Normalise (2d) (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise2( Vec4F32* const __restrict pDst );

/// Vec4F32 Normalise (3d) (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise3( Vec4F32* const __restrict pDst );

/// Vec4F32 Normalise (4d) (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise4( Vec4F32* const __restrict pDst );


/// Vec4F32 Normalise (2d) (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise2_24( Vec4F32* const __restrict pDst );

/// Vec4F32 Normalise (3d) (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise3_24( Vec4F32* const __restrict pDst );

/// Vec4F32 Normalise (4d) (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise4_24( Vec4F32* const __restrict pDst );


/// Vec4F32 Normalise (2d). Sets zeroLengthResult if length of *pDst is zero. (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise2( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult );

/// Vec4F32 Normalise (3d). Sets zeroLengthResult if length of *pDst is zero. (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise3( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult );

/// Vec4F32 Normalise (4d). Sets zeroLengthResult if length of *pDst is zero. (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise4( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult );


/// Vec4F32 Normalise (2d). Sets zeroLengthResult if length of *pDst is zero. (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise2_24( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult );

/// Vec4F32 Normalise (3d). Sets zeroLengthResult if length of *pDst is zero. (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise3_24( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult );

/// Vec4F32 Normalise (4d). Sets zeroLengthResult if length of *pDst is zero. (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise4_24( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult );


/// Vec4F32 Vector Set Length (2d) (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength2( Vec4F32* const __restrict pV, const VF32 VECREF length );

/// Vec4F32 Vector Set Length (3d) (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength3( Vec4F32* const __restrict pV, const VF32 VECREF length );

/// Vec4F32 Vector Set Length (4d) (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength4( Vec4F32* const __restrict pV, const VF32 VECREF length );


/// Vec4F32 Vector Set Length (2d) (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength2_24( Vec4F32* const __restrict pV, const VF32 VECREF length );

/// Vec4F32 Vector Set Length (3d) (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength3_24( Vec4F32* const __restrict pV, const VF32 VECREF length );

/// Vec4F32 Vector Set Length (4d) (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength4_24( Vec4F32* const __restrict pV, const VF32 VECREF length );


/// Vec4F32 Limit Length (2d) (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength2( Vec4F32* const __restrict pV, const VF32 VECREF limit );

/// Vec4F32 Limit Length (3d) (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength3( Vec4F32* const __restrict pV, const VF32 VECREF limit );

/// Vec4F32 Limit Length (4d) (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength4( Vec4F32* const __restrict pV, const VF32 VECREF limit );


/// Vec4F32 Limit Length (2d) (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength2_24( Vec4F32* const __restrict pV, const VF32 VECREF limit );

/// Vec4F32 Limit Length (3d) (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength3_24( Vec4F32* const __restrict pV, const VF32 VECREF limit );

/// Vec4F32 Limit Length (4d) (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength4_24( Vec4F32* const __restrict pV, const VF32 VECREF limit );


/// Vec4F32 Inverse (2d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse2( const Vec4F32 VECREF v );

/// Vec4F32 Inverse (3d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse3( const Vec4F32 VECREF v );

/// Vec4F32 Inverse (4d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse4( const Vec4F32 VECREF v );


/// Vec4F32 Inverse (2d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse2_24( const Vec4F32 VECREF v );

/// Vec4F32 Inverse (3d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse3_24( const Vec4F32 VECREF v );

/// Vec4F32 Inverse (4d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse4_24( const Vec4F32 VECREF v );


/// Vec4F32 Flush To Zero (2d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec4F32 FlushToZero2( const Vec4F32 VECREF v, const VF32 VECREF tolerance );

/// Vec4F32 Flush To Zero (3d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec4F32 FlushToZero3( const Vec4F32 VECREF v, const VF32 VECREF tolerance );

/// Vec4F32 Flush To Zero (4d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec4F32 FlushToZero4( const Vec4F32 VECREF v, const VF32 VECREF tolerance );


/// Vec4F32 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vec4F32 FlushElementsToZero( const Vec4F32 VECREF value, const VF32 VECREF tolerance );


/// Vector Space Tensor Product of a 3x1 column vector and a 1x3 row vector -> Mat33
NOALIAS __forceinline const Mat33 TensorProduct33( const Vec4F32 VECREF lhs, const Vec4F32 VECREF rhs );

/// Vector Space Tensor Product of a 4x1 column vector and a 1x4 row vector -> Mat44
NOALIAS __forceinline const Mat44 TensorProduct44( const Vec4F32 VECREF lhs, const Vec4F32 VECREF rhs );


/// Vec4F32 Rotate Elements Left
template < int N >
NOALIAS __forceinline const Vec4F32 RotateLeft( const Vec4F32 VECREF a );

/// Vec4F32 Rotate Elements Right
template < int N >
NOALIAS __forceinline const Vec4F32 RotateRight( const Vec4F32 VECREF a );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


