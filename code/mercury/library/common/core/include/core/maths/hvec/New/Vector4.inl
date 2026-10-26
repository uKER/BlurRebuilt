#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR4_INL
#define SOFT_MATHS_HVEC_NEW_VECTOR4_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR4_INL
#define MATHS_HVEC_NEW_VECTOR4_INL
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
template <> NOALIAS __forceinline const Vector4 Load( F32 const * const __restrict pSrc )
{
    Vector4 result;
    result.m_V = Load<Vec4F32>( pSrc );
    return result;
}

/// Store Vector4 to F32 ptr
NOALIAS __forceinline void Store( const Vector4 PCREF data, F32 * const __restrict pDst )
{
    Store( data.m_V, pDst );
}


/// Load Vector4 from StorageVector4
NOALIAS __forceinline const Vector4 Load( StorageVector4 const * const __restrict pSrc )
{
    Vector4 result;
    //result.m_V = Load< Vec4F32 >( &pSrc->m_Data );
    result.m_V = Load<Vec4F32>( (StorageVec4F32*)pSrc );
    return result;
}

/// Store Vector4 in StorageVector4
NOALIAS __forceinline void Store( const Vector4 PCREF data, StorageVector4 * const __restrict pDst )
{
    //Store( data.m_V, &pDst->m_Data );
    Store( data.m_V, (StorageVec4F32*)pDst );
}



/// Make Vector4 from broadcast VF32
NOALIAS __forceinline const Vector4 MakeVector4( const VF32 VECREF src )
{
    Vector4 result;
    result.m_V = MakeVec4F32( src );
    return result;
}

/// Make Vector4 from Vec4F32
NOALIAS __forceinline const Vector4 MakeVector4( const Vec4F32 VECREF src )
{
    Vector4 result;
    result.m_V = src;
    return result;
}

/// Get Vec4F32 from Vector4
NOALIAS __forceinline const Vec4F32 GetVec4F32( const Vector4 PCREF src )
{
    return src.m_V;
}


/// Make Vector4 from Point3: Result W = 1.0f
NOALIAS __forceinline const Vector4 MakeVector4( const Point3 PCREF p3 )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_select<0,0,0,1>( p3.m_V.m_V, Const< vector float >::One() );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_select<0,0,0,1>( p3.m_V.m_V, Const< vector float >::One() );
    #elif defined( _VMX128_ )
    result.m_V.m_V = __vrlimi( p3.m_V.m_V, Const< __vector4 >::One(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_select<0,0,0,1>( p3.m_V.m_V, Const< __m128 >::One() );
    #else
    result.m_V.m_V[VX] = p3.m_V.m_V[VX];
    result.m_V.m_V[VY] = p3.m_V.m_V[VY];
    result.m_V.m_V[VZ] = p3.m_V.m_V[VZ];
    result.m_V.m_V[VW] = 1.0f;
    #endif
    return result;
}

/// Make Vector4 from Vector3: Result W = 0.0f
NOALIAS __forceinline const Vector4 MakeVector4( const Vector3 PCREF v3 )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_select<0,0,0,1>( v3.m_V.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_select<0,0,0,1>( v3.m_V.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V.m_V = __vrlimi( v3.m_V.m_V, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_select<0,0,0,1>( v3.m_V.m_V, Const< __m128 >::Zero() );
    #else
    result.m_V.m_V[VX] = v3.m_V.m_V[VX];
    result.m_V.m_V[VY] = v3.m_V.m_V[VY];
    result.m_V.m_V[VZ] = v3.m_V.m_V[VZ];
    result.m_V.m_V[VW] = 0.0f;
    #endif
    return result;
}

/// Make Vector4 from Normal3: Result W = 0.0f
NOALIAS __forceinline const Vector4 MakeVector4( const Normal3 PCREF n3 )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_select<0,0,0,1>( n3.m_V.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_select<0,0,0,1>( n3.m_V.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V.m_V = __vrlimi( n3.m_V.m_V, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_select<0,0,0,1>( n3.m_V.m_V, Const< __m128 >::Zero() );
    #else
    result.m_V.m_V[VX] = n3.m_V.m_V[VX];
    result.m_V.m_V[VY] = n3.m_V.m_V[VY];
    result.m_V.m_V[VZ] = n3.m_V.m_V[VZ];
    result.m_V.m_V[VW] = 0.0f;
    #endif
    return result;
}


/// Make Vector4 from 4 F32's
NOALIAS __forceinline const Vector4 MakeVector4( const F32& x, const F32& y, const F32& z, const F32& w )
{
    Vector4 result;
    result.m_V = MakeVec4F32( x, y, z, w );
    return result;
}

/// Make Vector4 from 4 VF32's
NOALIAS __forceinline const Vector4 MakeVector4( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z, const VF32 VECREF w )
{
    Vector4 result;
    result.m_V = MakeVec4F32( x, y, z, w );
    return result;
}

/// Make Vector4 from Vector3 & VF32: result.XYZ = xyz, result.W = w
NOALIAS __forceinline const Vector4 MakeVector4( const Vector3 PCREF xyz, const VF32 VECREF w )
{
    Vector4 result;
    #if defined( _SPU_ )
    result.m_V.m_V = spu_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #elif defined( _VMX128_ )
    result.m_V.m_V = __vrlimi( xyz.m_V.m_V, w.m_V, VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #else
    result.m_V.m_V[VX] = xyz.m_V.m_V[VX];
    result.m_V.m_V[VY] = xyz.m_V.m_V[VY];
    result.m_V.m_V[VZ] = xyz.m_V.m_V[VZ];
    result.m_V.m_V[VW] = w.m_V;
    #endif
    return result;
}

/// Make Vector4 from VF32 & Vector3: result.X = x, result.YZW = yzw
NOALIAS __forceinline const Vector4 MakeVector4( const VF32 VECREF x, const Vector3 PCREF yzw )
{
    Vector4 result;
    #if defined( _SPU_ )
    //result.m_V.m_V = spu_select<0,1,1,1>( x.m_V, spu_rlqwbyte( yzw.m_V.m_V, 3*4 ) );
    result.m_V.m_V = spu_select<0,1,1,1>( x.m_V, spu_rotate_left_qw_words<3>( yzw.m_V.m_V ) );
    #elif defined( _PPU_ )
    //result.m_V.m_V = vec_select<0,1,1,1>( x.m_V, vec_sld( yzw.m_V.m_V, yzw.m_V.m_V, 3*4 ) );
    result.m_V.m_V = vec_select<0,1,1,1>( x.m_V, vec_rotate_left_qw_words<3>( yzw.m_V.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V.m_V = __vrlimi( x.m_V, yzw.m_V.m_V, VMX_MASK_INSERT_YZW, 3 );
    #elif defined( _SSE_ )
    result.m_V.m_V = sse_select<0,1,1,1>( x.m_V, sse_rotate_left_qw_words<3>( yzw.m_V.m_V ) );
    #else
    result.m_V.m_V[VX] = x.m_V;
    result.m_V.m_V[VY] = yzw.m_V.m_V[VX];
    result.m_V.m_V[VZ] = yzw.m_V.m_V[VY];
    result.m_V.m_V[VW] = yzw.m_V.m_V[VZ];
    #endif
    return result;
}

/// Make Vector4 from a Vector5
NOALIAS __forceinline const Vector4 MakeVector4( const Vector5& v5 )
{
    Vector4 result;
    #if defined( _SIMD_ )
    result.m_V = v5.m_V.m_V0;
    #else
    result.m_V.m_V[VX] = v5.m_V[0];
    result.m_V.m_V[VY] = v5.m_V[1];
    result.m_V.m_V[VZ] = v5.m_V[2];
    result.m_V.m_V[VW] = v5.m_V[3];
    #endif
    return result;
}


/// Vector4 Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vector4 PCREF src )
{
    return X( src.m_V );
}

/// Vector4 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vector4 PCREF src )
{
    return Y( src.m_V );
}

/// Vector4 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Vector4 PCREF src )
{
    return Z( src.m_V );
}

/// Vector4 Accessor: Get W
NOALIAS __forceinline const VF32 W( const Vector4 PCREF src )
{
    return W( src.m_V );
}

/// Vector4 Accessor: Set X
NOALIAS __forceinline void SetX( Vector4* const __restrict pDst, const VF32 VECREF x )
{
    SetX( &pDst->m_V, x );
}

/// Vector4 Accessor: Set Y
NOALIAS __forceinline void SetY( Vector4* const __restrict pDst, const VF32 VECREF y )
{
    SetY( &pDst->m_V, y );
}

/// Vector4 Accessor: Set Z
NOALIAS __forceinline void SetZ( Vector4* const __restrict pDst, const VF32 VECREF z )
{
    SetZ( &pDst->m_V, z );
}

/// Vector4 Accessor: Set W
NOALIAS __forceinline void SetW( Vector4* const __restrict pDst, const VF32 VECREF w )
{
    SetW( &pDst->m_V, w );
}


/// Vector4 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vector4 PCREF v )
{
    BIZ_STATIC_ASSERT( I < 4 );
    return Element<I>( v.m_V );
}

/// Vector4 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vector4* const __restrict pDst, const VF32 VECREF ei )
{
    BIZ_STATIC_ASSERT( I < 4 );
    SetElement<I>( &pDst->m_V, ei );
}



/// Vector4 Write to Stream
template < typename T_Stream > void Serialise( const Vector4 PCREF src, T_Stream& stream )
{
    StorageVector4 storage;
    Store( src, &storage );
    stream << storage.m_Data.x;
    stream << storage.m_Data.y;
    stream << storage.m_Data.z;
    stream << storage.m_Data.w;
}

/// Vector4 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Vector4* const __restrict pDst )
{
    StorageVector4 storage;
    stream >> storage.m_Data.x;
    stream >> storage.m_Data.y;
    stream >> storage.m_Data.z;
    stream >> storage.m_Data.w;
    *pDst = Load( &storage );
}



/// Vector4 Permute : Indices select 4 result elements [x,y,z,w] from [ 0:ax, 1:ay, 2:az, 3:aw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Vector4 Permute( const Vector4 PCREF a )
{
    return MakeVector4( Permute<IX,IY,IZ,IW>( a.m_V ) );
}

/// Vector4 Permute : Indices select 4 result elements [x,y,z,w] from [ 0:ax, 1:ay, 2:az, 3:aw, 4:bx, 5:by, 6:bz, 7:bw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Vector4 Permute( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return MakeVector4( Permute<IX,IY,IZ,IW>( a.m_V, b.m_V ) );
}


/// Vector4 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:aw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vector3 Permute( const Vector4 PCREF a )
{
    return MakeVector3( Permute<IX,IY,IZ>( a.m_V ) );
}

/// Vector4 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:aw, 4:bx, 5:by, 6:bz, 7:bw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vector3 Permute( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return MakeVector3( Permute<IX,IY,IZ>( a.m_V, b.m_V ) );
}


/// Vector4 MergeXY : Merges the x & y elements from a & b --> [ ax, bx, ay, by ]
NOALIAS __forceinline const Vector4 MergeXY( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return MakeVector4( MergeXY( a.m_V, b.m_V ) );
}

/// Vector4 MergeZW : Merges the z & w elements from a & b --> [ az, bz, aw, bw ]
NOALIAS __forceinline const Vector4 MergeZW( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return MakeVector4( MergeZW( a.m_V, b.m_V ) );
}



/// Vector4 Abs of each element
NOALIAS __forceinline const Vector4 Abs( const Vector4 PCREF v )
{
    return MakeVector4( Abs( v.m_V ) );
}

/// Vector4 Sign of each element : For each e in v: returns -1.0f if e < 0.0f, +1.0f if e > 0.0f, or 0.0f if e == 0.0f 
NOALIAS __forceinline const Vector4 Sign( const Vector4 PCREF v )
{
    return MakeVector4( Sign( v.m_V ) );
}

/// Vector4 CopySign of each element : Returns a Vector4 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vector4 CopySign( const Vector4 PCREF magnitude, const Vector4 PCREF sign )
{
    return MakeVector4( CopySign( magnitude.m_V, sign.m_V ) );
}


/// Vector4 Negation : Indices select elements to negate [X,Y,Z,W]
template < bool X, bool Y, bool Z, bool W >
NOALIAS __forceinline const Vector4 Negate( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = Negate<X,Y,Z,W>( src.m_V );
    return result;
}


/// Vector4 Negate X
NOALIAS __forceinline const Vector4 NegateX( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateX( src.m_V );
    return result;
}

/// Vector4 Negate Y
NOALIAS __forceinline const Vector4 NegateY( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateY( src.m_V );
    return result;
}

/// Vector4 Negate Z
NOALIAS __forceinline const Vector4 NegateZ( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateZ( src.m_V );
    return result;
}

/// Vector4 Negate W
NOALIAS __forceinline const Vector4 NegateW( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateW( src.m_V );
    return result;
}


/// Vector4 Negate XY
NOALIAS __forceinline const Vector4 NegateXY( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateXY( src.m_V );
    return result;
}

/// Vector4 Negate XZ
NOALIAS __forceinline const Vector4 NegateXZ( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateXZ( src.m_V );
    return result;
}

/// Vector4 Negate XW
NOALIAS __forceinline const Vector4 NegateXW( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateXW( src.m_V );
    return result;
}

/// Vector4 Negate YZ
NOALIAS __forceinline const Vector4 NegateYZ( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateYZ( src.m_V );
    return result;
}

/// Vector4 Negate YW
NOALIAS __forceinline const Vector4 NegateYW( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateYW( src.m_V );
    return result;
}

/// Vector4 Negate ZW
NOALIAS __forceinline const Vector4 NegateZW( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateZW( src.m_V );
    return result;
}


/// Vector4 Negate XYZ
NOALIAS __forceinline const Vector4 NegateXYZ( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateXYZ( src.m_V );
    return result;
}

/// Vector4 Negate XYW
NOALIAS __forceinline const Vector4 NegateXYW( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateXYW( src.m_V );
    return result;
}

/// Vector4 Negate XZW
NOALIAS __forceinline const Vector4 NegateXZW( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateXZW( src.m_V );
    return result;
}

/// Vector4 Negate YZW
NOALIAS __forceinline const Vector4 NegateYZW( const Vector4 PCREF src )
{
    Vector4 result;
    result.m_V = NegateYZW( src.m_V );
    return result;
}


/// Vector4 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Div( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return MakeVector4( Div( a.m_V, b.m_V ) );
}

/// Vector4 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Div24( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return MakeVector4( Div24( a.m_V, b.m_V ) );
}


/// Vector4 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Div( const VF32 VECREF s, const Vector4 PCREF v )
{
    return MakeVector4( Div( s, v.m_V ) );
}

/// Vector4 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Div24( const VF32 VECREF s, const Vector4 PCREF v )
{
    return MakeVector4( Div24( s, v.m_V ) );
}


/// Vector4 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Div( const Vector4 PCREF v, const VF32 VECREF s )
{
    return MakeVector4( Div( v.m_V, s ) );
}

/// Vector4 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Div24( const Vector4 PCREF v, const VF32 VECREF s )
{
    return MakeVector4( Div24( v.m_V, s ) );
}


/// Vector4 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 RcpSqrt( const Vector4 PCREF v )
{
    return MakeVector4( RcpSqrt( v.m_V ) );
}

/// Vector4 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 RcpSqrt24( const Vector4 PCREF v )
{
    return MakeVector4( RcpSqrt24( v.m_V ) );
}


/// Vector4 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Sqrt( const Vector4 PCREF v )
{
    return MakeVector4( Sqrt( v.m_V ) );
}

/// Vector4 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Sqrt24( const Vector4 PCREF v )
{
    return MakeVector4( Sqrt24( v.m_V ) );
}


/// Vector4 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vector4 PCREF v,
                                       Vector4* const __restrict pRcp,
                                       Vector4* const __restrict pSqrt,
                                       Vector4* const __restrict pRcpSqrt )
{
    RcpAndSqrt( v.m_V, &pRcp->m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}

/// Vector4 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vector4 PCREF v,
                                         Vector4* const __restrict pRcp,
                                         Vector4* const __restrict pSqrt,
                                         Vector4* const __restrict pRcpSqrt )
{
    RcpAndSqrt24( v.m_V, &pRcp->m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}


/// Vector4 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vector4 PCREF v,
                                           Vector4* const __restrict pSqrt,
                                           Vector4* const __restrict pRcpSqrt )
{
    SqrtAndRcpSqrt( v.m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}

/// Vector4 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vector4 PCREF v,
                                             Vector4* const __restrict pSqrt,
                                             Vector4* const __restrict pRcpSqrt )
{
    SqrtAndRcpSqrt24( v.m_V, &pSqrt->m_V, &pRcpSqrt->m_V );
}


/// Vector4 Unary Positive Operator
NOALIAS __forceinline const Vector4 operator + ( const Vector4 PCREF x )
{
    return x;
}

/// Vector4 Unary Negation Operator
NOALIAS __forceinline const Vector4 operator - ( const Vector4 PCREF x )
{
    return MakeVector4( -x.m_V );
}


/// Vector4 Sum
NOALIAS __forceinline const Vector4 operator + ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return MakeVector4( a.m_V + b.m_V );
}

/// Vector4 Difference
NOALIAS __forceinline const Vector4 operator - ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return MakeVector4( a.m_V - b.m_V );
}

/// Vector4 Multiplication Operator
NOALIAS __forceinline const Vector4 operator * ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    Vector4 result;
    result.m_V = a.m_V * b.m_V;
    return result;
}

/// Vector4 Division Operator
NOALIAS __forceinline const Vector4 operator / ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    Vector4 result;
    result.m_V = a.m_V / b.m_V;
    return result;
}


/// Vector4 Addition Update
NOALIAS __forceinline void operator += ( Vector4& a, const Vector4 PCREF b )
{
    a.m_V += b.m_V;
}

/// Vector4 Subtraction Update
NOALIAS __forceinline void operator -= ( Vector4& a, const Vector4 PCREF b )
{
    a.m_V -= b.m_V;
}

/// Vector4 Multiplication Update Operator
NOALIAS __forceinline void operator *= ( Vector4& a, const Vector4 PCREF b )
{
    a.m_V *= b.m_V;
}

/// Vector4 Division Update Operator
NOALIAS __forceinline void operator /= ( Vector4& a, const Vector4 PCREF b )
{
    a.m_V /= b.m_V;
}


/// Vector4 Scalar Multiplication: Vector4 * VF32
NOALIAS __forceinline const Vector4 operator * ( const Vector4 PCREF v, const VF32 VECREF s )
{
    return MakeVector4( v.m_V * s );
}

/// Vector4 Scalar Multiplication: VF32 * Vector4
NOALIAS __forceinline const Vector4 operator * ( const VF32 VECREF s, const Vector4 PCREF v )
{
    return MakeVector4( s * v.m_V );
}

/// Vector4 Scalar Division: Vector4 / VF32
NOALIAS __forceinline const Vector4 operator / ( const Vector4 PCREF v, const VF32 VECREF s )
{
    return MakeVector4( v.m_V / s );
}

/// Vector4 Scalar Division: VF32 / Vector4
NOALIAS __forceinline const Vector4 operator / ( const VF32 VECREF s, const Vector4 PCREF v )
{
    return MakeVector4( s / v.m_V );
}

/// Vector4 Scalar Multiplication Update: Vector4 *= VF32
NOALIAS __forceinline void operator *= ( Vector4& v, const VF32 VECREF s )
{
    v.m_V *= s;
}

/// Vector4 Scalar Division Update: Vector4 /= VF32
NOALIAS __forceinline void operator /= ( Vector4& v, const VF32 VECREF s )
{
    v.m_V /= s;
}



/// Vector4 Matrix Multiplication: Vector4 * Mat43
NOALIAS __forceinline const Vector4 operator * ( const Vector4 PCREF v4, const Mat43 MATREF m43 )
{
    //Vector4 result;
    //#if defined( _SPU_ )
    //result.m_V.m_V = spu_multiply_vector4_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    //#elif defined( _PPU_ )
    //result.m_V.m_V = vec_multiply_vector4_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    //#elif defined( _VMX128_ )
    //result.m_V.m_V = vmx_multiply_vector4_matrix43( v.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    //#elif defined( _SSE_ )
    //result.m_V.m_V = sse_multiply_vector4_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    //#else
    //for ( S32 c=0; c<3; c++ )
    //{
    //    result.m_V.m_V[c] = v.m_V.m_V[0] * m.m[0][c] +
    //                        v.m_V.m_V[1] * m.m[1][c] +
    //                        v.m_V.m_V[2] * m.m[2][c] +
    //                        v.m_V.m_V[3] * m.m[3][c];
    //}
    //result.m_V.m_V[3] = v.m_V.m_V[3];
    //#endif
    //return result;
    
    Vector4 result;
    result.m_V = v4.m_V * m43;
    return result;
}

/// Vector4 * Mat44 Multiplication operator : input = 4 element row vector, output = 4 element row vector
NOALIAS __forceinline const Vector4 operator * ( const Vector4 PCREF v4, const Mat44 MATREF m44 )
{
    Vector4 result;
    result.m_V = v4.m_V * m44;
    return result;
}

/// Mat44 * Vector4 Multiplication operator : input = 4 element col vector, output = 4 element col vector
NOALIAS __forceinline const Vector4 operator * ( const Mat44 MATREF m44, const Vector4 PCREF v4 )
{
    Vector4 result;
    result.m_V = m44 * v4.m_V;
    return result;
}


/// Vector4 Matrix Multiplication Update: Vector4 *= Mat43
NOALIAS __forceinline void operator *= ( Vector4& v, const Mat43 MATREF m )
{
    #if defined( _SPU_ )
    v.m_V.m_V = spu_multiply_vector4_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _PPU_ )
    v.m_V.m_V = vec_multiply_vector4_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _VMX128_ )
    v.m_V.m_V = vmx_multiply_vector4_matrix43( v.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ );
    #elif defined( _SSE_ )
    v.m_V.m_V = sse_multiply_vector4_matrix43( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #else
    Vector3 result;
    for ( S32 c=0; c<3; c++ )
    {
        result.m_V.m_V[c] = v.m_V.m_V[0] * m.m[0][c] +
                            v.m_V.m_V[1] * m.m[1][c] +
                            v.m_V.m_V[2] * m.m[2][c] +
                            v.m_V.m_V[3] * m.m[3][c];
    }
    //result.m_V.m_V[3] = v.m_V.m_V[3];
    //v = result;
    v.m_V.m_V[0] = result.m_V.m_V[0];
    v.m_V.m_V[1] = result.m_V.m_V[1];
    v.m_V.m_V[2] = result.m_V.m_V[2];
    #endif
}

/// Vector4 Matrix Multiplication Update: Vector4 *= Mat44
NOALIAS __forceinline void operator *= ( Vector4& v, const Mat44 MATREF m )
{
    #if defined( _SPU_ )
    v.m_V.m_V = spu_multiply_vector4_matrix44( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _PPU_ )
    v.m_V.m_V = vec_multiply_vector4_matrix44( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #elif defined( _VMX128_ )
    v.m_V.m_V = vmx_multiply_vector4_matrix44( v.m_V.m_V, m.m_ColX, m.m_ColY, m.m_ColZ, m.m_ColW );
    #elif defined( _SSE_ )
    v.m_V.m_V = sse_multiply_vector4_matrix44( v.m_V.m_V, m.m_RowX, m.m_RowY, m.m_RowZ, m.m_RowW );
    #else
    Vector4 result;
    for ( S32 c=0; c<4; c++ )
    {
        result.m_V.m_V[c] = v.m_V.m_V[0] * m.m[0][c] +
                            v.m_V.m_V[1] * m.m[1][c] +
                            v.m_V.m_V[2] * m.m[2][c] +
                            v.m_V.m_V[3] * m.m[3][c];
    }
    v = result;
    #endif
}




/// Vector4 MulAdd : a * b + c
NOALIAS __forceinline const Vector4 MulAdd( const Vector4 PCREF a, const Vector4 PCREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = MulAdd( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector4 MulSub : a * b - c
NOALIAS __forceinline const Vector4 MulSub( const Vector4 PCREF a, const Vector4 PCREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = MulSub( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector4 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector4 NegMulAdd( const Vector4 PCREF a, const Vector4 PCREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = NegMulAdd( a.m_V, b.m_V, c.m_V );
    return result;
}

/// Vector4 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector4 NegMulSub( const Vector4 PCREF a, const Vector4 PCREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = NegMulSub( a.m_V, b.m_V, c.m_V );
    return result;
}


/// Vector4 MulAdd : a * b + c
NOALIAS __forceinline const Vector4 MulAdd( const Vector4 PCREF a, const VF32 VECREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = MulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Vector4 MulSub : a * b - c
NOALIAS __forceinline const Vector4 MulSub( const Vector4 PCREF a, const VF32 VECREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = MulSub( a.m_V, b, c.m_V );
    return result;
}

/// Vector4 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vector4 NegMulAdd( const Vector4 PCREF a, const VF32 VECREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = NegMulAdd( a.m_V, b, c.m_V );
    return result;
}

/// Vector4 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vector4 NegMulSub( const Vector4 PCREF a, const VF32 VECREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = NegMulSub( a.m_V, b, c.m_V );
    return result;
}


/// Vector4 MulAdd : a * b + c : [ scalar * vector + vector ]
NOALIAS __forceinline const Vector4 MulAdd( const VF32 VECREF a, const Vector4 PCREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = MulAdd( a, b.m_V, c.m_V );
    return result;
}

/// Vector4 MulSub : a * b - c : [ scalar * vector - vector ]
NOALIAS __forceinline const Vector4 MulSub( const VF32 VECREF a, const Vector4 PCREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = MulSub( a, b.m_V, c.m_V );
    return result;
}

/// Vector4 NegMulAdd : -( a * b + c ) : [ - ( scalar * vector + vector ) ]
NOALIAS __forceinline const Vector4 NegMulAdd( const VF32 VECREF a, const Vector4 PCREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = NegMulAdd( a, b.m_V, c.m_V );
    return result;
}

/// Vector4 NegMulSub : -( a * b - c ) == c - a * b : [ - ( scalar * vector - vector ) ]
NOALIAS __forceinline const Vector4 NegMulSub( const VF32 VECREF a, const Vector4 PCREF b, const Vector4 PCREF c )
{
    Vector4 result;
    result.m_V = NegMulSub( a, b.m_V, c.m_V );
    return result;
}


/// Vector4 Lerp
NOALIAS __forceinline const Vector4 Lerp( const Vector4 PCREF src, const Vector4 PCREF dst, const VF32 VECREF t )
{
    Vector4 result;
    result.m_V = Lerp( src.m_V, dst.m_V, t );
    return result;
}


/// Vector4 Min
NOALIAS __forceinline const Vector4 Min( const Vector4 PCREF a, const Vector4 PCREF b )
{
    Vector4 result;
    result.m_V = Min( a.m_V, b.m_V );
    return result;
}

/// Vector4 Max
NOALIAS __forceinline const Vector4 Max( const Vector4 PCREF a, const Vector4 PCREF b )
{
    Vector4 result;
    result.m_V = Max( a.m_V, b.m_V );
    return result;
}

/// Vector4 Min
NOALIAS __forceinline const Vector4 Min( const Vector4 PCREF a, const VF32 VECREF b )
{
    Vector4 result;
    result.m_V = Min( a.m_V, b );
    return result;
}

/// Vector4 Max
NOALIAS __forceinline const Vector4 Max( const Vector4 PCREF a, const VF32 VECREF b )
{
    Vector4 result;
    result.m_V = Max( a.m_V, b );
    return result;
}

/// Vector4 Min Update
NOALIAS __forceinline void Min( Vector4* const __restrict pA, const Vector4 PCREF b )
{
    Min( &pA->m_V, b.m_V );
}

/// Vector4 Max Update
NOALIAS __forceinline void Max( Vector4* const __restrict pA, const Vector4 PCREF b )
{
    Max( &pA->m_V, b.m_V );
}

/// Vector4 Min Update
NOALIAS __forceinline void Min( Vector4* const __restrict pA, const VF32 VECREF b )
{
    Min( &pA->m_V, b );
}

/// Vector4 Max Update
NOALIAS __forceinline void Max( Vector4* const __restrict pA, const VF32 VECREF b )
{
    Max( &pA->m_V, b );
}


/// Vector4 Clamp
NOALIAS __forceinline const Vector4 Clamp( const Vector4 PCREF a, const Vector4 PCREF lo, const Vector4 PCREF hi )
{
    Vector4 result;
    result.m_V = Clamp( a.m_V, lo.m_V, hi.m_V );
    return result;
}

/// Vector4 Clamp
NOALIAS __forceinline const Vector4 Clamp( const Vector4 PCREF a, const VF32 VECREF lo, const VF32 VECREF hi )
{
    Vector4 result;
    result.m_V = Clamp( a.m_V, lo, hi );
    return result;
}

/// Vector4 Clamp Update
NOALIAS __forceinline void Clamp( Vector4* const __restrict pA, const Vector4 PCREF lo, const Vector4 PCREF hi )
{
    Clamp( &pA->m_V, lo.m_V, hi.m_V );
}

/// Vector4 Clamp Update
NOALIAS __forceinline void Clamp( Vector4* const __restrict pA, const VF32 VECREF lo, const VF32 VECREF hi )
{
    Clamp( &pA->m_V, lo, hi );
}



/// Vector4 Round to integer towards minus infinity
NOALIAS __forceinline const Vector4 Floor( const Vector4 PCREF a )
{
    Vector4 result;
    result.m_V = Floor( a.m_V );
    return result;
}

/// Vector4 Round to integer towards plus infinity
NOALIAS __forceinline const Vector4 Ceil( const Vector4 PCREF a )
{
    Vector4 result;
    result.m_V = Ceil( a.m_V );
    return result;
}

/// Vector4 Round to integer towards nearest
NOALIAS __forceinline const Vector4 Round( const Vector4 PCREF a )
{
    Vector4 result;
    result.m_V = Round( a.m_V );
    return result;
}

/// Vector4 Round to integer towards zero
NOALIAS __forceinline const Vector4 Trunc( const Vector4 PCREF a )
{
    Vector4 result;
    result.m_V = Trunc( a.m_V );
    return result;
}


/// Vector4 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const Vector4 Mod( const Vector4 PCREF x, const Vector4 PCREF y )
{
    Vector4 result;
    result.m_V = Mod( x.m_V, y.m_V );
    return result;
}

/// Vector4 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const Vector4 Mod( const Vector4 PCREF x, const Vector4 PCREF y, const Vector4 PCREF rcp_y )
{
    Vector4 result;
    result.m_V = Mod( x.m_V, y.m_V, rcp_y.m_V );
    return result;
}

/// Vector4 Fractional Part : Returns fractional part of x, f = x - i [ x = i + f ], where i is an integer, i & f have the same sign as x
NOALIAS __forceinline const Vector4 Frac( const Vector4 PCREF x )
{
    Vector4 result;
    result.m_V = Frac( x.m_V );
    return result;
}

/// Vector4 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const Vector4 WrapToInterval( const Vector4 PCREF x, const Vector4 PCREF p )
{
    Vector4 result;
    result.m_V = WrapToInterval( x.m_V, p.m_V );
    return result;
}

/// Vector4 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const Vector4 WrapToInterval( const Vector4 PCREF x, const Vector4 PCREF p, const Vector4 PCREF rcp_p )
{
    Vector4 result;
    result.m_V = WrapToInterval( x.m_V, p.m_V, rcp_p.m_V );
    return result;
}

/// Vector4 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const Vector4 WrapToIntervalOffset( const Vector4 PCREF x, const Vector4 PCREF p, const Vector4 PCREF o )
{
    Vector4 result;
    result.m_V = WrapToIntervalOffset( x.m_V, p.m_V, o.m_V );
    return result;
}

/// Vector4 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const Vector4 WrapToIntervalOffset( const Vector4 PCREF x, const Vector4 PCREF p, const Vector4 PCREF rcp_p, const Vector4 PCREF o )
{
    Vector4 result;
    result.m_V = WrapToIntervalOffset( x.m_V, p.m_V, rcp_p.m_V, o.m_V );
    return result;
}


/// Vector4 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const Vector4 Mod( const Vector4 PCREF x, const VF32 VECREF y )
{
    Vector4 result;
    result.m_V = Mod( x.m_V, y );
    return result;
}

/// Vector4 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const Vector4 Mod( const Vector4 PCREF x, const VF32 VECREF y, const VF32 VECREF rcp_y )
{
    Vector4 result;
    result.m_V = Mod( x.m_V, y, rcp_y );
    return result;
}

/// Vector4 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const Vector4 WrapToInterval( const Vector4 PCREF x, const VF32 VECREF p )
{
    Vector4 result;
    result.m_V = WrapToInterval( x.m_V, p );
    return result;
}

/// Vector4 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const Vector4 WrapToInterval( const Vector4 PCREF x, const VF32 VECREF p, const VF32 VECREF rcp_p )
{
    Vector4 result;
    result.m_V = WrapToInterval( x.m_V, p, rcp_p );
    return result;
}

/// Vector4 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const Vector4 WrapToIntervalOffset( const Vector4 PCREF x, const VF32 VECREF p, const VF32 VECREF o )
{
    Vector4 result;
    result.m_V = WrapToIntervalOffset( x.m_V, p, o );
    return result;
}

/// Vector4 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const Vector4 WrapToIntervalOffset( const Vector4 PCREF x, const VF32 VECREF p, const VF32 VECREF rcp_p, const VF32 VECREF o )
{
    Vector4 result;
    result.m_V = WrapToIntervalOffset( x.m_V, p, rcp_p, o );
    return result;
}



/// Vector4 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w
NOALIAS __forceinline const VBool operator == ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return Permute<VX,VY,VZ,VZ>( a.m_V ) == Permute<VX,VY,VZ,VZ>( b.m_V );
}

/// Vector4 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w
NOALIAS __forceinline const VBool operator != ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return Permute<VX,VY,VZ,VZ>( a.m_V ) != Permute<VX,VY,VZ,VZ>( b.m_V );
}

/// Vector4 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator <= ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return a.m_V <= b.m_V;
}

/// Vector4 Comparison a >= b : Returns VBool representing results of a.i >= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator >= ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return a.m_V >= b.m_V;
}

/// Vector4 Comparison a < b : Returns VBool representing results of a.i < b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator < ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return a.m_V < b.m_V;
}

/// Vector4 Comparison a > b : Returns VBool representing results of a.i > b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator > ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return a.m_V > b.m_V;
}

/// Vector4 Ordered Test : true if neither a nor b contains elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return Ordered( Permute<VX,VY,VZ,VZ>( a.m_V ), Permute<VX,VY,VZ,VZ>( b.m_V ) );
}

/// Vector4 Unordered Test : true if a or b contains elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return Unordered( Permute<VX,VY,VZ,VZ>( a.m_V ), Permute<VX,VY,VZ,VZ>( b.m_V ) );
}

/// Vector4 Finite test
NOALIAS __forceinline const VBool Finite( const Vector4 PCREF src )
{
    return Finite( src.m_V );
}



/// Vector4 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector4 Select( const VBool PCREF condition, const Vector4 PCREF result_if_true, const Vector4 PCREF result_if_false )
{
    return MakeVector4( Select( condition, result_if_true.m_V, result_if_false.m_V ) );
}

/// Vector4 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vector4 Select( const Vec4Bool PCREF condition, const Vector4 PCREF result_if_true, const Vector4 PCREF result_if_false )
{
    return MakeVector4( Select( condition, result_if_true.m_V, result_if_false.m_V ) );
}


/// Vector4 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vector4 AND( const Vector4 PCREF a, const VBool PCREF b )
{
    Vector4 result;
    result.m_V = AND( a.m_V, b );
    return result;
}

/// Vector4 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vector4 ANDC( const Vector4 PCREF a, const VBool PCREF b )
{
    Vector4 result;
    result.m_V = ANDC( a.m_V, b );
    return result;
}


/// Vector4 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vector4 AND( const Vector4 PCREF a, const Vec4Bool PCREF b )
{
    Vector4 result;
    result.m_V = AND( a.m_V, b );
    return result;
}

/// Vector4 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vector4 ANDC( const Vector4 PCREF a, const Vec4Bool PCREF b )
{
    Vector4 result;
    result.m_V = ANDC( a.m_V, b );
    return result;
}


/// Vector4 dot product
NOALIAS __forceinline const VF32 Dot( const Vector4 PCREF a, const Vector4 PCREF b )
{
    return Dot4( a.m_V, b.m_V );
}

/// Vector4 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Rcp( const Vector4 PCREF v )
{
    return MakeVector4( Rcp( v.m_V ) );
}

/// Vector4 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Rcp24( const Vector4 PCREF v )
{
    return MakeVector4( Rcp24( v.m_V ) );
}

/// Vector4 dot product operator
NOALIAS __forceinline const VF32 operator | ( const Vector4 PCREF a, const Vector4 PCREF b )
{
    /// @par WARNING: Operator precedence of '|' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    return Dot4( a.m_V, b.m_V );
}

/// Vector4 Length Squared
NOALIAS __forceinline const VF32 LengthSquared( const Vector4 PCREF v )
{
    return LengthSquared4( v.m_V );
}


/// Vector4 Length (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length( const Vector4 PCREF v )
{
    return Length4( v.m_V );
}

/// Vector4 Length (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length24( const Vector4 PCREF v )
{
    return Length4_24( v.m_V );
}


/// Vector4 Set Length (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength( Vector4* const __restrict pDst, const VF32 VECREF length )
{
    SetLength4( &pDst->m_V, length );
}

/// Vector4 Set Length (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength24( Vector4* const __restrict pDst, const VF32 VECREF length )
{
    SetLength4_24( &pDst->m_V, length );
}


/// Vector4 Limit Length (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength( Vector4* const __restrict pV, const VF32 VECREF limit )
{
    LimitLength4( &pV->m_V, limit );
}

/// Vector4 Limit Length (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength24( Vector4* const __restrict pV, const VF32 VECREF limit )
{
    LimitLength4_24( &pV->m_V, limit );
}


/// Vector4 Normalise (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Normalise( const Vector4 PCREF v )
{
    Vector4 result;
    result.m_V = Normalise4( v.m_V );
    return result;
}

/// Vector4 Normalise (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Normalise24( const Vector4 PCREF v )
{
    Vector4 result;
    result.m_V = Normalise4_24( v.m_V );
    return result;
}


/// Vector4 Normalise (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise( Vector4* const __restrict pDst )
{
    Normalise4( &pDst->m_V );
}

/// Vector4 Normalise (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise24( Vector4* const __restrict pDst )
{
    Normalise4_24( &pDst->m_V );
}


/// Vector4 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vector4 Inverse( const Vector4 PCREF v )
{
    return v * Rcp( LengthSquared( v ) );
}

/// Vector4 Inverse: Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vector4 Inverse24( const Vector4 PCREF v )
{
    return v * Rcp24( LengthSquared( v ) );
}



/// Vector4 Flush To Zero: flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vector4 FlushToZero( const Vector4 PCREF v, const VF32 VECREF tolerance )
{
    Vector4 result;
    result.m_V = FlushToZero4( v.m_V, tolerance );
    return result;
}

/// Vector4 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vector4 FlushElementsToZero( const Vector4 PCREF v, const VF32 VECREF tolerance )
{
    Vector4 result;
    result.m_V = FlushElementsToZero( v.m_V, tolerance );
    return result;
}


/// Vector Space Tensor Product of a 4x1 column vector and a 1x4 row vector -> Mat44
NOALIAS __forceinline const Mat44 TensorProduct( const Vector4 PCREF lhs, const Vector4 PCREF rhs )
{
    return TensorProduct44( lhs.m_V, rhs.m_V );
}


/// Vector4 Rotate Elements Left
template < int N >
NOALIAS __forceinline const Vector4 RotateLeft( const Vector4 PCREF a )
{
    Vector4 result;
    result.m_V = RotateLeft<N>( a.m_V );
    return result;
}

/// Vector4 Rotate Elements Right
template < int N >
NOALIAS __forceinline const Vector4 RotateRight( const Vector4 PCREF a )
{
    Vector4 result;
    result.m_V = RotateRight<N>( a.m_V );
    return result;
}








/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

