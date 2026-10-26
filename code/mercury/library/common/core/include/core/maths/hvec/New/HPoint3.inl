#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_HPOINT3_INL
#define SOFT_MATHS_HVEC_NEW_HPOINT3_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_HPOINT3_INL
#define MATHS_HVEC_NEW_HPOINT3_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup HPoint3
/// @{

/// Load HPoint3 from StorageHPoint3
NOALIAS __forceinline const HPoint3 Load( StorageHPoint3 const * const __restrict pSrc )
{
    HPoint3 result;
    result.m_V = Load< Vec4F32 >( &pSrc->m_Data );
    return result;
}

/// Store HPoint3 in StorageHPoint3
NOALIAS __forceinline void Store( const HPoint3 PCREF data, StorageHPoint3 * const __restrict pDst )
{
    Store( data.m_V, &pDst->m_Data );
}



/// Make HPoint3 from Vec4F32
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Vec4F32 VECREF src )
{
    HPoint3 result;
    result.m_V = src;
    return result;
}

/// Get Vec4F32 from HPoint3
NOALIAS __forceinline const Vec4F32 GetVec4F32( const HPoint3 PCREF src )
{
    return src.m_V;
}


/// Make HPoint3 from Vector3
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Vector3 PCREF v3 )
{
    HPoint3 result;
    result.m_V = MakeVec4F32( v3, Const<VF32>::One() );
    return result;
}

/// Make HPoint3 from Vector3 xyz & VF32 w
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Vector3 PCREF xyz, const VF32 VECREF w )
{
    HPoint3 result;
    result.m_V = MakeVec4F32( xyz, w );
    return result;
}

/// Make HPoint3 from Point3
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Point3 PCREF p3 )
{
    HPoint3 result;
    result.m_V = MakeVec4F32( p3, Const<VF32>::One() );
    return result;
}

/// Make HPoint3 from Normal3
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Normal3 PCREF n3 )
{
    HPoint3 result;
    result.m_V = MakeVec4F32( n3, Const<VF32>::One() );
    return result;
}

/// Make HPoint3 from Vector4
NOALIAS __forceinline const HPoint3 MakeHPoint3( const Vector4 PCREF v4 )
{
    HPoint3 result;
    result.m_V = v4.m_V;
    return result;
}


/// Make HPoint3 from 4 F32's
NOALIAS __forceinline const HPoint3 MakeHPoint3( const F32& x, const F32& y, const F32& z, const F32& w )
{
    HPoint3 result;
    result.m_V = MakeVec4F32( x, y, z, w );
    return result;
}

/// Make HPoint3 from 3 VF32's
NOALIAS __forceinline const HPoint3 MakeHPoint3( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z, const VF32 VECREF w )
{
    HPoint3 result;
    result.m_V = MakeVec4F32( x, y, z, w );
    return result;
}


/// HPoint3 Accessor: Get X
NOALIAS __forceinline const VF32 X( const HPoint3 PCREF src )
{
    return X( src.m_V );
}

/// HPoint3 Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const HPoint3 PCREF src )
{
    return Y( src.m_V );
}

/// HPoint3 Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const HPoint3 PCREF src )
{
    return Z( src.m_V );
}

/// HPoint3 Accessor: Get W
NOALIAS __forceinline const VF32 W( const HPoint3 PCREF src )
{
    return W( src.m_V );
}


/// HPoint3 Accessor: Set X
NOALIAS __forceinline void SetX( HPoint3* const __restrict pDst, const VF32 VECREF x )
{
    SetX( &pDst->m_V, x );
}

/// HPoint3 Accessor: Set Y
NOALIAS __forceinline void SetY( HPoint3* const __restrict pDst, const VF32 VECREF y )
{
    SetY( &pDst->m_V, y );
}

/// HPoint3 Accessor: Set Z
NOALIAS __forceinline void SetZ( HPoint3* const __restrict pDst, const VF32 VECREF z )
{
    SetZ( &pDst->m_V, z );
}

/// HPoint3 Accessor: Set W
NOALIAS __forceinline void SetW( HPoint3* const __restrict pDst, const VF32 VECREF w )
{
    SetW( &pDst->m_V, w );
}


/// HPoint3 Accessor: Get Vector3 XYZ
NOALIAS __forceinline const Vector3 XYZ( const HPoint3 PCREF src )
{
    return MakeVector3( src.m_V );
}

/// HPoint3 Accessor: Get Vector4 XYZW
NOALIAS __forceinline const Vector4 XYZW( const HPoint3 PCREF src )
{
    return MakeVector4( src.m_V );
}


/// HPoint3 Write to Stream
template < typename T_Stream >
void Serialise( const HPoint3 PCREF src, T_Stream& stream )
{
    StorageHPoint3 storage;
    Store( src, &storage );
    stream << storage.m_Data.x;
    stream << storage.m_Data.y;
    stream << storage.m_Data.z;
    stream << storage.m_Data.w;
}

/// HPoint3 Read from Stream
template < typename T_Stream >
void Deserialise( T_Stream& stream, HPoint3* const __restrict pDst )
{
    StorageHPoint3 storage;
    stream >> storage.m_Data.x;
    stream >> storage.m_Data.y;
    stream >> storage.m_Data.z;
    stream >> storage.m_Data.w;
    *pDst = Load( &storage );
}


/// HPoint3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const HPoint3 Permute( const HPoint3 PCREF a )
{
    StaticAssert( IX < 3, IX_MUST_BE_LESS_THAN_3 );
    StaticAssert( IY < 3, IY_MUST_BE_LESS_THAN_3 );
    StaticAssert( IZ < 3, IZ_MUST_BE_LESS_THAN_3 );
    
    return MakePoint3( Permute<IX,IY,IZ>( a.m_V ) );
}

/// Point3 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:bx, 4:by, 5:bz ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const HPoint3 Permute( const HPoint3 PCREF a, const Point3 PCREF b )
{
    StaticAssert( IX < 6, IX_MUST_BE_LESS_THAN_6 );
    StaticAssert( IY < 6, IY_MUST_BE_LESS_THAN_6 );
    StaticAssert( IZ < 6, IZ_MUST_BE_LESS_THAN_6 );
    
    return MakePoint3( Permute< IX<3?IX:IX+1, IY<3?IY:IY+1, IZ<3?IZ:IZ+1 >( a.m_V, b.m_V ) );
}


/// HPoint3 Sum Operator
NOALIAS __forceinline const HPoint3 operator + ( const HPoint3 PCREF p0, const HPoint3 PCREF p1 )
{
    /// @code
    /// p0( x0, y0, z0, w0 )
    /// p1( x1, y1, z1, w1 )
    /// p0 + p1 = ( x0/w0 + x1/w1, y0/w0 + y1/w1, z0/w0 + z1/w1, 1 )
    ///         = ( x0*w1 + x1*w0, y0*w1 + y1*w0, z0*w1 + z1*w0, w0*w1 )
    /// @endcode
    
    HPoint3 result;
    Vec4F32 p1w0( p1.m_V * W( p0.m_V ) );
    SetW( &p1w0, Const<VF32>::Zero() );
    result.m_V = MulAdd( p0.m_V, W( p1.m_V ), p1w0 );
    return result;
}

/// HPoint3 Difference Operator
NOALIAS __forceinline const Vector3 operator - ( const HPoint3 PCREF p0, const HPoint3 PCREF p1 )
{
    /// @code
    /// p0( x0, y0, z0, w0 )
    /// p1( x1, y1, z1, w1 )
    /// p0 - p1 = ( x0/w0 - x1/w1, y0/w0 - y1/w1, z0/w0 - z1/w1 )
    ///         = ( x0*w1 - x1*w0, y0*w1 - y1*w0, z0*w1 - z1*w0 ) / w0*w1
    /// @endcode
    
    Vector3 result;
    Vec4F32 p1w0( p1.m_V * W( p0.m_V ) );
    const VF32 rcp_w0w1( Rcp( W( p1w0 ) ) );
    result.m_V = MulSub( p0.m_V, W( p1.m_V ), p1w0 ) * rcp_w0w1;
    return result;
}


/// HPoint3 + Vector3 Addition Operator
NOALIAS __forceinline const HPoint3 operator + ( const HPoint3 PCREF p, const Vector3 PCREF v )
{
    /// @code
    /// p( px, py, pz, pw )
    /// v( vx, vy, vz )
    /// p + v = ( px + vx*pw, py + vy*pw, pz + vz*pw, pw )
    /// @endcode
    
    HPoint3 result;
    const Vector4 vxyz0( MakeVector4(v) ); // NOTE: W(vxyz0) = 0.0f
    result.m_V = MulAdd( vxyz0.m_V, W(p), p.m_V );
    return result;
}

/// HPoint3 - Vector3 Subtraction Operator
NOALIAS __forceinline const HPoint3 operator - ( const HPoint3 PCREF p, const Vector3 PCREF v )
{
    /// @code
    /// p( px, py, pz, pw )
    /// v( vx, vy, vz )
    /// p - v = ( px - vx*pw, py - vy*pw, pz - vz*pw, pw )
    /// @endcode
    
    HPoint3 result;
    const Vector4 vxyz0( MakeVector4(v) ); // NOTE: W(vxyz0) = 0.0f
    result.m_V = NegMulSub( vxyz0.m_V, W(p), p.m_V );
    return result;
}


/// HPoint3 * VF32 Multiplication Operator
NOALIAS __forceinline const HPoint3 operator * ( const HPoint3 PCREF a, const VF32 VECREF b )
{
    /// @code
    /// a( x, y, z, w )
    /// a * b = ( x*b, y*b, z*b, w )
    /// @endcode
    
    return MakeHPoint3( XYZ(a) * b, W(a) );
}

/// VF32 * HPoint3 Multiplication Operator
NOALIAS __forceinline const HPoint3 operator * ( const VF32 VECREF a, const HPoint3 PCREF b )
{
    /// @code
    /// b( x, y, z, w )
    /// a * b = ( a*x, a*y, a*z, w )
    /// @endcode
    
    return MakeHPoint3( a * XYZ(b), W(b) );
}


/// HPoint3 / VF32 Division Operator
NOALIAS __forceinline const HPoint3 operator / ( const HPoint3 PCREF a, const VF32 VECREF b )
{
    /// @code
    /// a( x, y, z, w )
    /// a * b = ( x/b, y/b, z/b, w )
    ///       = ( x, y, z, w * b )
    /// @endcode
    
    return MakeHPoint3( XYZ(a), W(a) * b );
}


/// HPoint3 Normalise: returns Point3( x/w, y/w, z/w )
NOALIAS __forceinline const Point3 Normalise( const HPoint3 PCREF a )
{
    /// @code
    /// a( x, y, z, w )
    /// Normalise(a) = ( x/w, y/w, z/w )
    /// @endcode
    
    return MakePoint3( XYZ(a) * Rcp(W(a)) );
}

/// HPoint3 Normalise: returns Point3( x/w, y/w, z/w )
NOALIAS __forceinline const Point3 Normalise24( const HPoint3 PCREF a )
{
    /// @code
    /// a( x, y, z, w )
    /// Normalise(a) = ( x/w, y/w, z/w )
    /// @endcode
    
    return MakePoint3( XYZ(a) * Rcp24(W(a)) );
}


/// HPoint3 Normalise: ( x, y, z, w ) -> ( x/w, y/w, z/w, 1 )
NOALIAS __forceinline void Normalise( HPoint3* const __restrict pA )
{
    /// @code
    /// a( x, y, z, w )
    /// a = a/w = ( x/w, y/w, z/w, 1 )
    /// @endcode
    
    const HPoint3 a( *pA );
    *pA = MakeHPoint3( XYZ(a) * Rcp(W(a)), Const<VF32>::One() );
}

/// HPoint3 Normalise: ( x, y, z, w ) -> ( x/w, y/w, z/w, 1 )
NOALIAS __forceinline void Normalise24( HPoint3* const __restrict pA )
{
    /// @code
    /// a( x, y, z, w )
    /// a = a/w = ( x/w, y/w, z/w, 1 )
    /// @endcode
    
    const HPoint3 a( *pA );
    *pA = MakeHPoint3( XYZ(a) * Rcp24(W(a)), Const<VF32>::One() );
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

