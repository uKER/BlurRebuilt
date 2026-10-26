#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC4F32_INL
#define SOFT_MATHS_HVEC_NEW_VEC4F32_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC4F32_INL
#define MATHS_HVEC_NEW_VEC4F32_INL
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
NOALIAS __forceinline const Vec4F32 Load( F32 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_unaligned( pSrc );
    #elif defined( _PPU_ )
    result.m_V = vec_load_unaligned( pSrc );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_unaligned( pSrc );
    #elif defined( _SSE_ )
    result.m_V = sse_load_unaligned( pSrc );
    #else
    result.m_V[VX] = pSrc[VX];
    result.m_V[VY] = pSrc[VY];
    result.m_V[VZ] = pSrc[VZ];
    result.m_V[VW] = pSrc[VW];
    #endif
    return result;
}

/// Store Vec4F32 into an F32 array
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec4( src.m_V, pDst );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec4( src.m_V, pDst );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec4( src.m_V, pDst );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec4( src.m_V, pDst );
    #else
    pDst[VX] = src.m_V[VX];
    pDst[VY] = src.m_V[VY];
    pDst[VZ] = src.m_V[VZ];
    pDst[VW] = src.m_V[VW];
    #endif
}


/// Load Vec4F32 from a Float1
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float1 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_unaligned( pSrc->v );
    #elif defined( _PPU_ )
    result.m_V = vec_load_unaligned( pSrc->v );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_unaligned( pSrc->v );
    #elif defined( _SSE_ )
    result.m_V = sse_load_unaligned( pSrc->v );
    #else
    result.m_V[VX] = pSrc->v[VX];
    #endif
    return result;
}

/// Store Vec4F32 into a Float1
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float1* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec1( src.m_V, pDst->v );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec1( src.m_V, pDst->v );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec1( src.m_V, pDst->v );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec1( src.m_V, pDst->v );
    #else
    pDst->v[VX] = src.m_V[VX];
    #endif
}

/// Load Vec4F32 from a Float2
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float2 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_unaligned( pSrc->v );
    #elif defined( _PPU_ )
    result.m_V = vec_load_unaligned( pSrc->v );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_unaligned( pSrc->v );
    #elif defined( _SSE_ )
    result.m_V = sse_load_unaligned( pSrc->v );
    #else
    result.m_V[VX] = pSrc->v[VX];
    result.m_V[VY] = pSrc->v[VY];
    #endif
    return result;
}

/// Store Vec4F32 into a Float2
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float2* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec2( src.m_V, pDst->v );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec2( src.m_V, pDst->v );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec2( src.m_V, pDst->v );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec2( src.m_V, pDst->v );
    #else
    pDst->v[VX] = src.m_V[VX];
    pDst->v[VY] = src.m_V[VY];
    #endif
}

/// Load Vec4F32 from a Float3
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float3 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_unaligned( pSrc->v );
    #elif defined( _PPU_ )
    result.m_V = vec_load_unaligned( pSrc->v );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_unaligned( pSrc->v );
    #elif defined( _SSE_ )
    result.m_V = sse_load_unaligned( pSrc->v );
    #else
    result.m_V[VX] = pSrc->v[VX];
    result.m_V[VY] = pSrc->v[VY];
    result.m_V[VZ] = pSrc->v[VZ];
    #endif
    return result;
}

/// Store Vec4F32 into a Float3
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float3* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec3( src.m_V, pDst->v );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec3( src.m_V, pDst->v );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec3( src.m_V, pDst->v );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec3( src.m_V, pDst->v );
    #else
    pDst->v[VX] = src.m_V[VX];
    pDst->v[VY] = src.m_V[VY];
    pDst->v[VZ] = src.m_V[VZ];
    #endif
}

/// Load Vec4F32 from a Float4
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float4 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_unaligned( pSrc->v );
    #elif defined( _PPU_ )
    result.m_V = vec_load_unaligned( pSrc->v );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_unaligned( pSrc->v );
    #elif defined( _SSE_ )
    result.m_V = sse_load_unaligned( pSrc->v );
    #else
    result.m_V[VX] = pSrc->v[VX];
    result.m_V[VY] = pSrc->v[VY];
    result.m_V[VZ] = pSrc->v[VZ];
    result.m_V[VW] = pSrc->v[VW];
    #endif
    return result;
}

/// Store Vec4F32 into a Float4
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float4* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec4( src.m_V, pDst->v );
    #elif defined( _PPU_ )
    vec_store_unaligned_vec4( src.m_V, pDst->v );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec4( src.m_V, pDst->v );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec4( src.m_V, pDst->v );
    #else
    pDst->v[VX] = src.m_V[VX];
    pDst->v[VY] = src.m_V[VY];
    pDst->v[VZ] = src.m_V[VZ];
    pDst->v[VW] = src.m_V[VW];
    #endif
}



/// Load Vec4F32 from a Float2A16
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float2A16 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_aligned( pSrc->v );
    #elif defined( _PPU_ )
    result.m_V = vec_load_aligned( pSrc->v );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_aligned( pSrc->v );
    #elif defined( _SSE_ )
    result.m_V = sse_load_aligned( pSrc->v );
    #else
    result.m_V[VX] = pSrc->v[VX];
    result.m_V[VY] = pSrc->v[VY];
    #endif
    return result;
}

/// Store Vec4F32 into a Float2A16
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float2A16* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec2( src.m_V, pDst->v ); // NOTE: xxx_store_aligned_vec2 variants not worth implementing
    #elif defined( _PPU_ )
    vec_store_unaligned_vec2( src.m_V, pDst->v );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec2( src.m_V, pDst->v );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec2( src.m_V, pDst->v );
    #else
    pDst->v[VX] = src.m_V[VX];
    pDst->v[VY] = src.m_V[VY];
    #endif
}

/// Load Vec4F32 from a Float3A16
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float3A16 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_aligned( pSrc->v );
    #elif defined( _PPU_ )
    result.m_V = vec_load_aligned( pSrc->v );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_aligned( pSrc->v );
    #elif defined( _SSE_ )
    result.m_V = sse_load_aligned( pSrc->v );
    #else
    result.m_V[VX] = pSrc->v[VX];
    result.m_V[VY] = pSrc->v[VY];
    result.m_V[VZ] = pSrc->v[VZ];
    #endif
    return result;
}

/// Store Vec4F32 into a Float3A16
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float3A16* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_unaligned_vec3( src.m_V, pDst->v ); // NOTE: xxx_store_aligned_vec3 variants not worth implementing
    #elif defined( _PPU_ )
    vec_store_unaligned_vec3( src.m_V, pDst->v );
    #elif defined( _VMX128_ )
    vmx_store_unaligned_vec3( src.m_V, pDst->v );
    #elif defined( _SSE_ )
    sse_store_unaligned_vec3( src.m_V, pDst->v );
    #else
    pDst->v[VX] = src.m_V[VX];
    pDst->v[VY] = src.m_V[VY];
    pDst->v[VZ] = src.m_V[VZ];
    #endif
}

/// Load Vec4F32 from a Float4A16
template <>
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Float4A16 const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_aligned( pSrc->v );
    #elif defined( _PPU_ )
    result.m_V = vec_load_aligned( pSrc->v );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_aligned( pSrc->v );
    #elif defined( _SSE_ )
    result.m_V = sse_load_aligned( pSrc->v );
    #else
    result.m_V[VX] = pSrc->v[VX];
    result.m_V[VY] = pSrc->v[VY];
    result.m_V[VZ] = pSrc->v[VZ];
    result.m_V[VW] = pSrc->v[VW];
    #endif
    return result;
}

/// Store Vec4F32 into a Float4A16
NOALIAS __forceinline void Store( const Vec4F32 VECREF src, MathsStorage::Float4A16* const __restrict pDst )
{
    #if defined( _SPU_ )
    spu_store_aligned_vec4( src.m_V, pDst->v );
    #elif defined( _PPU_ )
    vec_store_aligned_vec4( src.m_V, pDst->v );
    #elif defined( _VMX128_ )
    vmx_store_aligned_vec4( src.m_V, pDst->v );
    #elif defined( _SSE_ )
    sse_store_aligned_vec4( src.m_V, pDst->v );
    #else
    pDst->v[VX] = src.m_V[VX];
    pDst->v[VY] = src.m_V[VY];
    pDst->v[VZ] = src.m_V[VZ];
    pDst->v[VW] = src.m_V[VW];
    #endif
}


/// StorageVec1F32: Load Vec4F32
template <>
NOALIAS __forceinline const Vec4F32 Load( StorageVec1F32 const * const __restrict pSrc )
{
    return Load< Vec4F32 >( &pSrc->m_Data );
}

/// StorageVec1F32: Store Vec4F32
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec1F32 * const __restrict pDst )
{
    Store( data, &pDst->m_Data );
}

/// StorageVec2F32: Load Vec4F32
template <>
NOALIAS __forceinline const Vec4F32 Load( StorageVec2F32 const * const __restrict pSrc )
{
    return Load< Vec4F32 >( &pSrc->m_Data );
}

/// StorageVec2F32: Store Vec4F32
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec2F32 * const __restrict pDst )
{
    Store( data, &pDst->m_Data );
}

/// StorageVec3F32: Load Vec4F32
template <>
NOALIAS __forceinline const Vec4F32 Load( StorageVec3F32 const * const __restrict pSrc )
{
    return Load< Vec4F32 >( &pSrc->m_Data );
}

/// StorageVec3F32: Store Vec4F32
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec3F32 * const __restrict pDst )
{
    Store( data, &pDst->m_Data );
}

/// StorageVec4F32: Load Vec4F32
template <>
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32 const * const __restrict pSrc )
{
    return Load< Vec4F32 >( &pSrc->m_Data );
}

/// StorageVec4F32: Store Vec4F32
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32 * const __restrict pDst )
{
    Store( data, &pDst->m_Data );
}



/// Load Vec4F32 from a Fixed3_S16
template < int T_Precision >
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Fixed3_S16<T_Precision> const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_convtf( ((vector signed int){ (signed int)pSrc->x,
                                                   (signed int)pSrc->y,
                                                   (signed int)pSrc->z,
											       (signed int)0 } ), T_Precision );
    #elif defined( _PPU_ )
    result.m_V = vec_and( vec_vcfsx( vec_vupkhsh( vec_load_unaligned( pSrc->v ) ), T_Precision ), Const< vector float >::Mask<1,1,1,0>() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( __vcfsx( __vupkhsh( vmx_load_unaligned( pSrc ) ), T_Precision ), __vzero(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    const F32 multiplier( 1.0f / (F32)( 1 << T_Precision ) ); // TODO: SSE intrinsics version!
    result = MakeVec4F32( (F32)pSrc->x * multiplier,
                          (F32)pSrc->y * multiplier,
                          (F32)pSrc->z * multiplier );
    #else
    const F32 multiplier( 1.0f / (F32)( 1 << T_Precision ) );
    result.m_V[VX] = (F32)pSrc->x * multiplier;
    result.m_V[VY] = (F32)pSrc->y * multiplier;
    result.m_V[VZ] = (F32)pSrc->z * multiplier;
    result.m_V[VW] = 0.0f;
    #endif
    return result;
}

/// Store Vec4F32 into a Fixed3_S16
template < int T_Precision >
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, MathsStorage::Fixed3_S16<T_Precision> * const __restrict pDst )
{
    #if defined( _SPU_ )
    const vector signed int temp = spu_convts( data.m_V, T_Precision );
    pDst->x = (S16)spu_extract( temp, VX );
    pDst->y = (S16)spu_extract( temp, VY );
    pDst->z = (S16)spu_extract( temp, VZ );
    #elif defined( _PPU_ )
    const vector signed short packed( vec_vpkswss( vec_vctsxs( data.m_V, T_Precision ), (vector signed int)vec_vspltisw( 0 ) ) );
    vec_stvehx( vec_vsplth( packed, VX ), 0x0, &pDst->x );
    vec_stvehx( vec_vsplth( packed, VY ), 0x0, &pDst->y );
    vec_stvehx( vec_vsplth( packed, VZ ), 0x0, &pDst->z );
    #elif defined( _VMX128_ )
    const __vector4 packed( __vpkswss( __vctsxs( data.m_V, T_Precision ), __vzero() ) );
    __stvehx( __vsplth( packed, VX ), &pDst->x, 0x0 );
    __stvehx( __vsplth( packed, VY ), &pDst->y, 0x0 );
    __stvehx( __vsplth( packed, VZ ), &pDst->z, 0x0 );
    #elif defined( _SSE_ )
    const F32 multiplier( 1 << T_Precision );
    pDst->x = (S16)( X( data ) * multiplier ); // TODO: SSE intrinsics version!
    pDst->y = (S16)( Y( data ) * multiplier );
    pDst->z = (S16)( Z( data ) * multiplier );
    #else
    const F32 multiplier( 1 << T_Precision );
    pDst->x = (S16)( data.m_V[VX] * multiplier );
    pDst->y = (S16)( data.m_V[VY] * multiplier );
    pDst->z = (S16)( data.m_V[VZ] * multiplier );
    #endif
}

/// Load Vec4F32 from a Fixed4_S16
template < int T_Precision >
NOALIAS __forceinline const Vec4F32 Load( MathsStorage::Fixed4_S16<T_Precision> const * const __restrict pSrc )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    #if 1
    result.m_V = spu_convtf( ((vector signed int){ (signed int)pSrc->x,
                                                   (signed int)pSrc->y,
                                                   (signed int)pSrc->z,
											       (signed int)pSrc->w } ), T_Precision );
    #else
    result.m_V = spu_convtf( spu_unpack_hi( *pSrc ), T_Precision );
    #endif
    #elif defined( _PPU_ )
    result.m_V = vec_vcfsx( vec_vupkhsh( vec_load_unaligned( pSrc->v ) ), T_Precision );
    #elif defined( _VMX128_ )
    result.m_V = __vcfsx( __vupkhsh( vmx_load_unaligned( pSrc ) ), T_Precision );
    #elif defined( _SSE_ )
    const F32 multiplier( 1.0f / (F32)( 1 << T_Precision ) ); // TODO: SSE intrinsics version!
    result = MakeVec4F32( (F32)pSrc->x * multiplier,
                          (F32)pSrc->y * multiplier,
                          (F32)pSrc->z * multiplier,
                          (F32)pSrc->w * multiplier );
    #else
    const F32 multiplier( 1.0f / (F32)( 1 << T_Precision ) );
    result.m_V[VX] = (F32)pSrc->x * multiplier;
    result.m_V[VY] = (F32)pSrc->y * multiplier;
    result.m_V[VZ] = (F32)pSrc->z * multiplier;
    result.m_V[VW] = (F32)pSrc->w * multiplier;
    #endif
    return result;
}

/// Store Vec4F32 into a Fixed4_S16
template < int T_Precision >
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, MathsStorage::Fixed4_S16<T_Precision> * const __restrict pDst )
{
    #if defined( _SPU_ )
    const vector signed int temp = spu_convts( data.m_V, T_Precision );
    pDst->x = (S16)spu_extract( temp, VX );
    pDst->y = (S16)spu_extract( temp, VY );
    pDst->z = (S16)spu_extract( temp, VZ );
    pDst->w = (S16)spu_extract( temp, VW );
    #elif defined( _PPU_ )
    const vector signed short packed( vec_vpkswss( vec_vctsxs( data.m_V, T_Precision ), (vector signed int)vec_vspltisw( 0 ) ) );
    vec_stvehx( vec_vsplth( packed, VX ), 0x0, &pDst->x );
    vec_stvehx( vec_vsplth( packed, VY ), 0x0, &pDst->y );
    vec_stvehx( vec_vsplth( packed, VZ ), 0x0, &pDst->z );
    vec_stvehx( vec_vsplth( packed, VW ), 0x0, &pDst->w );
    #elif defined( _VMX128_ )
    const __vector4 packed( __vpkswss( __vctsxs( data.m_V, T_Precision ), __vzero() ) );
    __stvehx( __vsplth( packed, VX ), &pDst->x, 0x0 );
    __stvehx( __vsplth( packed, VY ), &pDst->y, 0x0 );
    __stvehx( __vsplth( packed, VZ ), &pDst->z, 0x0 );
    __stvehx( __vsplth( packed, VW ), &pDst->w, 0x0 );
    #elif defined( _SSE_ )
    const F32 multiplier( 1 << T_Precision );
    pDst->x = (S16)( X( data ) * multiplier ); // TODO: SSE intrinsics version!
    pDst->y = (S16)( Y( data ) * multiplier );
    pDst->z = (S16)( Z( data ) * multiplier );
    pDst->w = (S16)( W( data ) * multiplier );
    #else
    const F32 multiplier( 1 << T_Precision );
    pDst->x = (S16)( data.m_V[VX] * multiplier );
    pDst->y = (S16)( data.m_V[VY] * multiplier );
    pDst->z = (S16)( data.m_V[VZ] * multiplier );
    pDst->w = (S16)( data.m_V[VW] * multiplier );
    #endif
}



/// Make Vec4F32 from a Vec4S32
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Vec4S32 PCREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_convtf( a.m_V, 0 );
    #elif defined( _PPU_ )
    result.m_V = vec_vcfsx( a.m_V, 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vcfsx( a.m_V, 0 );
    #elif defined( _SSE_ )
    result.m_V = _mm_cvtepi32_ps( a.m_V );
    #else
    result.m_V[0] = (F32)a.m_V[0];
    result.m_V[1] = (F32)a.m_V[1];
    result.m_V[2] = (F32)a.m_V[2];
    result.m_V[3] = (F32)a.m_V[3];
    #endif
    return result;
}


/// Vec4F32: Load from StorageVec4F32_U8
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_U8 const * const __restrict pSrc )
{
    const Vec16U8 a0( Load( &pSrc->m_Data ) );
    Vec8S16 b0, b1; Unpack( a0, &b0, &b1 );
    Vec4S32 c0, c1; Unpack( b0, &c0, &c1 );
    return MakeVec4F32( c0 );
}

/// Vec4F32: Store to StorageVec4F32_U8
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_U8 * const __restrict pDst )
{
    const Vec4S32 a( MakeVec4S32( data ) );
    const Vec8S16 b( PackSS( a, a ) );
    const Vec16U8 c( PackUS( b, b ) );
    Store( c, &pDst->m_Data );
}


/// Vec4F32: Load from StorageVec4F32_U8N
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_U8N const * const __restrict pSrc )
{
    const VF32    k( Const<VF32>::Frac<1,255>() );
    const Vec16U8 a0( Load( &pSrc->m_Data ) );
    Vec8S16 b0, b1; Unpack( a0, &b0, &b1 );
    Vec4S32 c0, c1; Unpack( b0, &c0, &c1 );
    return MakeVec4F32( c0 ) * k;
}

/// Vec4F32: Store to StorageVec4F32_U8N
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_U8N * const __restrict pDst )
{
    const VF32    k( Const<VF32>::Int<255>() );
    const Vec4S32 a( MakeVec4S32( data * k ) );
    const Vec8S16 b( PackSS( a, a ) );
    const Vec16U8 c( PackUS( b, b ) );
    Store( c, &pDst->m_Data );
}


/// Vec4F32: Load from StorageVec4F32_S16
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_S16 const * const __restrict pSrc )
{
    const Vec8S16 a0( Load( &pSrc->m_Data ) );
    Vec4S32 b0, b1; Unpack( a0, &b0, &b1 );
    return MakeVec4F32( b0 );
}

/// Vec4F32: Store to StorageVec4F32_S16
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_S16 * const __restrict pDst )
{
    const Vec4S32 a( MakeVec4S32( data ) );
    const Vec8S16 b( PackSS( a, a ) );
    Store( b, &pDst->m_Data );
}


/// Vec4F32: Load from StorageVec4F32_S16N
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_S16N const * const __restrict pSrc )
{
    const VF32    k( Const<VF32>::Frac<1,32767>() );
    const Vec8S16 a0( Load( &pSrc->m_Data ) );
    Vec4S32 b0, b1; Unpack( a0, &b0, &b1 );
    return MakeVec4F32( b0 ) * k;
}

/// Vec4F32: Store to StorageVec4F32_S16N
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_S16N * const __restrict pDst )
{
    const VF32    k( Const<VF32>::Int<32767>() );
    const Vec4S32 a( MakeVec4S32( data * k ) );
    const Vec8S16 b( PackSS( a, a ) );
    Store( b, &pDst->m_Data );
}


/// Vec4F32: Load from StorageVec4F32_HenD3
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_HenD3 const * const __restrict /*pSrc*/ )
{
    Vec4F32 result;
    assert( 0 && "Not implemented yet!\n" );
    return result;
}

/// Vec4F32: Store to StorageVec4F32_HenD3
NOALIAS __forceinline void Store( const Vec4F32 VECREF /*data*/, StorageVec4F32_HenD3 * const __restrict /*pDst*/ )
{
    assert( 0 && "Not implemented yet!\n" );
}

/// Vec4F32: Load from StorageVec4F32_F16
NOALIAS __forceinline const Vec4F32 Load( StorageVec4F32_F16 const * const __restrict pSrc )
{
    Vec4F32 result;
    
    #if defined( _SPU_ )
    
    const vector unsigned short zero_u16( Const< vector unsigned short >::Zero() );
    const vector unsigned int   zero_u32( Const< vector unsigned int >::Zero() );
    const vector unsigned int   data( (vector unsigned int)spu_merge_hi( zero_u16, spu_load_unaligned( (U16*)pSrc ) ) );
    
    const vector unsigned int f16_sign( spu_and( data, 0x8000 ) );
    const vector unsigned int f16_expt( spu_and( data, 0x7C00 ) );
    const vector unsigned int f16_mant( spu_and( data, 0x03FF ) );
    
    vector unsigned int f32_sign( spu_sl( f16_sign, 16 ) );
    vector unsigned int f32_expt( spu_sl( f16_expt, 13 ) );
    vector unsigned int f32_mant( spu_sl( f16_mant, 13 ) );
    
    f32_expt = spu_sel( spu_add( f32_expt, 112 << 23 ), zero_u32, spu_cmpeq( f32_expt, zero_u32 ) );
    
    result.m_V = (vector float)spu_or( f32_sign,
                               spu_or( f32_expt,
                                       f32_mant ) );
    
    #elif defined( _PPU_ )
    
    const vector unsigned short zero_u16( Const< vector unsigned short >::Zero() );
    const vector unsigned int   zero_u32( Const< vector unsigned int >::Zero() );
    const vector unsigned int   data( (vector unsigned int)vec_mergeh( zero_u16, vec_load_unaligned( (U16*)pSrc ) ) );
    
    const vector unsigned int f16_sign( vec_and( data, Const< vector unsigned int >::Int<0x8000>() ) );
    const vector unsigned int f16_expt( vec_and( data, Const< vector unsigned int >::Int<0x7C00>() ) );
    const vector unsigned int f16_mant( vec_and( data, Const< vector unsigned int >::Int<0x03FF>() ) );
    
    vector unsigned int f32_sign( vec_sl( f16_sign, Const< vector unsigned int >::Int<16>() ) );
    vector unsigned int f32_expt( vec_sl( f16_expt, Const< vector unsigned int >::Int<13>() ) );
    vector unsigned int f32_mant( vec_sl( f16_mant, Const< vector unsigned int >::Int<13>() ) );
    
    f32_expt = vec_sel( vec_add( f32_expt, Const< vector unsigned int >::Int< 112 << 23 >() ), zero_u32, vec_cmpeq( f32_expt, zero_u32 ) );
    
    result.m_V = (vector float)vec_or( f32_sign,
                               vec_or( f32_expt,
                                       f32_mant ) );
    
    #elif defined( _VMX128_ )
    
    result.m_V = __vupkd3d( vmx_load_unaligned_offset( &pSrc->m_Data.x, -8 ), VPACK_FLOAT16_4 );
    
    #elif defined( _SSE_ )
    
    const __m128i zero( _mm_setzero_si128() );
    const __m128i data( _mm_unpacklo_epi16( sse_load_unaligned( (U16*)pSrc ), zero ) );
    
    const __m128i f16_sign( _mm_and_si128( data, _mm_set1_epi32( 0x8000 ) ) );
    const __m128i f16_expt( _mm_and_si128( data, _mm_set1_epi32( 0x7C00 ) ) );
    const __m128i f16_mant( _mm_and_si128( data, _mm_set1_epi32( 0x03FF ) ) );
    
    __m128i f32_sign( _mm_slli_epi32( f16_sign, 16 ) );
    __m128i f32_expt( _mm_slli_epi32( f16_expt, 13 ) );
    __m128i f32_mant( _mm_slli_epi32( f16_mant, 13 ) );
    
    f32_expt = _mm_andnot_si128( _mm_cmpeq_epi32( f32_expt, zero ), _mm_add_epi32( f32_expt, _mm_set1_epi32( 112 << 23 ) ) );
    
    result.m_V = (__m128&)_mm_or_si128( f32_sign,
                          _mm_or_si128( f32_expt,
                                        f32_mant ) );
    
    #else
    
    // NOTE: This implementation is designed to emulate 'vupkd3d', NOT the non-intrinsic version in 'XMConvertHalfToFloat'
    const U32 f16_sign0( pSrc->m_Data.x.sign );
    const U32 f16_sign1( pSrc->m_Data.y.sign );
    const U32 f16_sign2( pSrc->m_Data.z.sign );
    const U32 f16_sign3( pSrc->m_Data.w.sign );
    const U32 f16_expt0( pSrc->m_Data.x.exponent );
    const U32 f16_expt1( pSrc->m_Data.y.exponent );
    const U32 f16_expt2( pSrc->m_Data.z.exponent );
    const U32 f16_expt3( pSrc->m_Data.w.exponent );
    const U32 f16_mant0( pSrc->m_Data.x.mantissa );
    const U32 f16_mant1( pSrc->m_Data.y.mantissa );
    const U32 f16_mant2( pSrc->m_Data.z.mantissa );
    const U32 f16_mant3( pSrc->m_Data.w.mantissa );
    const U32 f32_expt0( f16_expt0 == 0 ? 0 : f16_expt0 + 112 );
    const U32 f32_expt1( f16_expt1 == 0 ? 0 : f16_expt1 + 112 );
    const U32 f32_expt2( f16_expt2 == 0 ? 0 : f16_expt2 + 112 );
    const U32 f32_expt3( f16_expt3 == 0 ? 0 : f16_expt3 + 112 );
    const U32 f32_result0( ( f16_sign0 << 31 ) | ( f32_expt0 << 23 ) | ( f16_mant0 << 13 ) );
    const U32 f32_result1( ( f16_sign1 << 31 ) | ( f32_expt1 << 23 ) | ( f16_mant1 << 13 ) );
    const U32 f32_result2( ( f16_sign2 << 31 ) | ( f32_expt2 << 23 ) | ( f16_mant2 << 13 ) );
    const U32 f32_result3( ( f16_sign3 << 31 ) | ( f32_expt3 << 23 ) | ( f16_mant3 << 13 ) );
    result.m_V[0] = (F32&)f32_result0;
    result.m_V[1] = (F32&)f32_result1;
    result.m_V[2] = (F32&)f32_result2;
    result.m_V[3] = (F32&)f32_result3;
    
    #endif
    
    return result;
}

#if defined( _SSE_ )
#pragma warning( push )
#pragma warning( disable:4700 )
#endif

/// Vec4F32: Store to StorageVec4F32_F16
NOALIAS __forceinline void Store( const Vec4F32 VECREF data, StorageVec4F32_F16 * const __restrict pDst )
{
    #if defined( _SPU_ )
    
    const vector unsigned short zero_u16( Const< vector unsigned short >::Zero() );
    
    const vector unsigned int f32_data( (vector unsigned int)data.m_V );
    
    const vector unsigned int f32_sign( spu_srl<31>( f32_data ) );
    //const vector unsigned int f32_expt( spu_srl<23>( spu_and( f32_data, Const< vector unsigned int >::Int<0x7F800000>() ) ) );
    //const vector unsigned int f32_mant( spu_srl<13>( spu_and( f32_data, Const< vector unsigned int >::Int<0x007FFFFF>() ) ) );
    const vector unsigned int f32_expt( spu_srl<23>( spu_and( f32_data, 0x7F800000 ) ) );
    const vector unsigned int f32_mant( spu_srl<13>( spu_and( f32_data, 0x007FFFFF ) ) );
    
    vector unsigned short f16_sign( spu_pack_mod( f32_sign, f32_sign ) );
    vector unsigned short f16_expt( spu_pack_mod( f32_expt, f32_expt ) );
    vector unsigned short f16_mant( spu_pack_mod( f32_mant, f32_mant ) );
    
    const vector unsigned short expt_zero( spu_cmpeq( f16_expt, zero_u16 ) );
    const vector unsigned short expt_sub ( spu_cmplt( f16_expt, Const< vector unsigned short >::Int< 127 - 14 >() ) );
    const vector unsigned short expt_inf ( spu_cmpgt( f16_expt, Const< vector unsigned short >::Int< 127 + 15 >() ) );
    
    f16_expt = spu_min( spu_sub( f16_expt, Const< vector unsigned short >::Int<112>() ), Const< vector unsigned short >::Int<31>() );
    f16_expt = spu_sel( f16_expt, zero_u16, spu_or( expt_zero, expt_sub ) );
    
    f16_mant = spu_sel( f16_mant, zero_u16, expt_sub );
    f16_mant = spu_sel( f16_mant, Const< vector unsigned short >::Int<0x3FF>(), expt_inf );
    
    const vector unsigned short result( spu_or( spu_sl( f16_sign, 15 ),
                                        spu_or( spu_sl( f16_expt, 10 ),
                                                        f16_mant ) ) );
    
    spu_store_unaligned_vec4u16( result, (U16*)&pDst->m_Data.x );
    
    #elif defined( _PPU_ )
    
    const vector unsigned short zero_u16( Const< vector unsigned short >::Zero() );
    
    const vector unsigned int f32_data( (vector unsigned int)data.m_V );
    
    const vector unsigned int f32_sign( vec_vsrw( f32_data, Const< vector unsigned int >::Int<31>() ) );
    const vector unsigned int f32_expt( vec_vsrw( vec_and( f32_data, Const< vector unsigned int >::Int<0x7F800000>() ), Const< vector unsigned int >::Int<23>() ) );
    const vector unsigned int f32_mant( vec_vsrw( vec_and( f32_data, Const< vector unsigned int >::Int<0x007FFFFF>() ), Const< vector unsigned int >::Int<13>() ) );
    
    vector unsigned short f16_sign( vec_vpkuwum( f32_sign, f32_sign ) );
    vector unsigned short f16_expt( vec_vpkuwum( f32_expt, f32_expt ) );
    vector unsigned short f16_mant( vec_vpkuwum( f32_mant, f32_mant ) );
    
    const vector bool short expt_zero( vec_cmpeq( f16_expt, zero_u16 ) );
    const vector bool short expt_sub ( vec_cmplt( f16_expt, Const< vector unsigned short >::Int< 127 - 14 >() ) );
    const vector bool short expt_inf ( vec_cmpgt( f16_expt, Const< vector unsigned short >::Int< 127 + 15 >() ) );
    
    f16_expt = vec_min( vec_sub( f16_expt, Const< vector unsigned short >::Int<112>() ), Const< vector unsigned short >::Int<31>() );
    f16_expt = vec_sel( f16_expt, zero_u16, vec_or( expt_zero, expt_sub ) );
    
    f16_mant = vec_sel( f16_mant, zero_u16, expt_sub );
    f16_mant = vec_sel( f16_mant, Const< vector unsigned short >::Int<0x3FF>(), expt_inf );
    
    const vector unsigned short result( vec_or( vec_sl( f16_sign, Const< vector unsigned short >::Int<15>() ),
                                        vec_or( vec_sl( f16_expt, Const< vector unsigned short >::Int<10>() ),
                                                        f16_mant ) ) );
    
    vec_store_unaligned_vec4u16( result, (U16*)&pDst->m_Data.x );
    
    #elif defined( _VMX128_ )
    
    const __vector4 packed = __vpkd3d( packed, data.m_V, VPACK_FLOAT16_4, VPACK_64LO, 2 );
    vmx_store_unaligned_vec2( packed, &pDst->m_Data.x );
    
    #elif defined( _SSE_ )
    
    const __m128i zero( Const<__m128i>::Zero() );
    const __m128i f32_data( (__m128i&)data.m_V );
    
    const __m128i f32_sign( _mm_srai_epi32( f32_data, 31 ) );
    const __m128i f32_expt( _mm_srli_epi32( _mm_and_si128( f32_data, Const<__m128i>::ExponentMaskF32() ), 23 ) );
    const __m128i f32_mant( _mm_srli_epi32( _mm_and_si128( f32_data, Const<__m128i>::MantissaMaskF32() ), 13 ) );
    
    __m128i f16_sign( _mm_packs_epi32( f32_sign, f32_sign ) );
    __m128i f16_expt( _mm_packs_epi32( f32_expt, f32_expt ) );
    __m128i f16_mant( _mm_packs_epi32( f32_mant, f32_mant ) );
    
    const __m128i expt_zero( _mm_cmpeq_epi16( f16_expt, zero ) );
    const __m128i expt_sub ( _mm_cmplt_epi16( f16_expt, _mm_set1_epi16( 127 - 14 ) ) );
    const __m128i expt_inf ( _mm_cmpgt_epi16( f16_expt, _mm_set1_epi16( 127 + 15 ) ) );
    
    f16_expt = _mm_min_epi16( _mm_sub_epi16( f16_expt, _mm_set1_epi16( 112 ) ), _mm_set1_epi16( 31 ) );
    f16_expt = _mm_andnot_si128( _mm_or_si128( expt_zero, expt_sub ), f16_expt );
    
    f16_mant = _mm_andnot_si128( expt_sub, f16_mant );
    f16_mant = _mm_or_si128( _mm_andnot_si128( expt_inf, f16_mant ), _mm_and_si128( expt_inf, _mm_set1_epi16( 0x3FF ) ) );
    
    const __m128i result( _mm_or_si128( _mm_slli_epi16( f16_sign, 15 ),
                          _mm_or_si128( _mm_slli_epi16( f16_expt, 10 ),
                                                        f16_mant ) ) );
    
    sse_store_unaligned_vec2( (__m128&)result, (F32*)pDst );
    
    #else
    
    // NOTE: This implementation is designed to emulate 'vpkd3d', NOT the non-intrinsic version in 'XMConvertFloatToHalf'
    const U32 data0( (U32&)data.m_V[0] );
    const U32 data1( (U32&)data.m_V[1] );
    const U32 data2( (U32&)data.m_V[2] );
    const U32 data3( (U32&)data.m_V[3] );
    const U32 f32_sign0( data0 >> 31 );
    const U32 f32_sign1( data1 >> 31 );
    const U32 f32_sign2( data2 >> 31 );
    const U32 f32_sign3( data3 >> 31 );
    const U32 f32_expt0( ( data0 >> 23 ) & 0xFF );
    const U32 f32_expt1( ( data1 >> 23 ) & 0xFF );
    const U32 f32_expt2( ( data2 >> 23 ) & 0xFF );
    const U32 f32_expt3( ( data3 >> 23 ) & 0xFF );
    const U32 f32_mant0( data0 & 0x7FFFFF );
    const U32 f32_mant1( data1 & 0x7FFFFF );
    const U32 f32_mant2( data2 & 0x7FFFFF );
    const U32 f32_mant3( data3 & 0x7FFFFF );
    const bool zero0( f32_expt0 == 0 ); // zero?
    const bool zero1( f32_expt1 == 0 );
    const bool zero2( f32_expt2 == 0 );
    const bool zero3( f32_expt3 == 0 );
    const bool sub0( f32_expt0 < ( 127 - 14 ) ); // subnormal?
    const bool sub1( f32_expt1 < ( 127 - 14 ) );
    const bool sub2( f32_expt2 < ( 127 - 14 ) );
    const bool sub3( f32_expt3 < ( 127 - 14 ) );
    const bool inf0( f32_expt0 > ( 127 + 15 ) ); // infinity?
    const bool inf1( f32_expt1 > ( 127 + 15 ) );
    const bool inf2( f32_expt2 > ( 127 + 15 ) );
    const bool inf3( f32_expt3 > ( 127 + 15 ) );
    const U32 f16_expt0( zero0 || sub0 ? 0 : Min( f32_expt0 - 112, 31U ) ); // -127 + 15 = -112
    const U32 f16_expt1( zero1 || sub1 ? 0 : Min( f32_expt1 - 112, 31U ) ); // NOTE: subnormals truncated to zero!
    const U32 f16_expt2( zero2 || sub2 ? 0 : Min( f32_expt2 - 112, 31U ) );
    const U32 f16_expt3( zero3 || sub3 ? 0 : Min( f32_expt3 - 112, 31U ) );
    const U32 f16_mant0( sub0 ? 0 : inf0 ? 0x3FF : f32_mant0 >> 13 ); // NOTE: subnormal -> zero
    const U32 f16_mant1( sub1 ? 0 : inf1 ? 0x3FF : f32_mant1 >> 13 ); // NOTE: INF/IND -> NAN
    const U32 f16_mant2( sub2 ? 0 : inf2 ? 0x3FF : f32_mant2 >> 13 );
    const U32 f16_mant3( sub3 ? 0 : inf3 ? 0x3FF : f32_mant3 >> 13 );
    pDst->m_Data.x.sign = f32_sign0;
    pDst->m_Data.y.sign = f32_sign1;
    pDst->m_Data.z.sign = f32_sign2;
    pDst->m_Data.w.sign = f32_sign3;
    pDst->m_Data.x.exponent = f16_expt0;
    pDst->m_Data.y.exponent = f16_expt1;
    pDst->m_Data.z.exponent = f16_expt2;
    pDst->m_Data.w.exponent = f16_expt3;
    pDst->m_Data.x.mantissa = f16_mant0;
    pDst->m_Data.y.mantissa = f16_mant1;
    pDst->m_Data.z.mantissa = f16_mant2;
    pDst->m_Data.w.mantissa = f16_mant3;
    
    #endif
}

#if defined( _SSE_ )
#pragma warning( pop )
#endif






/// Make Vec4F32 from an F32 with the same value broadcast into all elements
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const F32& x )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( x );
    #elif defined( _PPU_ )
    //result.m_V = vec_splats( x );
    result.m_V = vec_splat( vec_lvlx( 0x0, &x ), 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( &x, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( x );
    #else
    result.m_V[VX] =
    result.m_V[VY] =
    result.m_V[VZ] =
    result.m_V[VW] = x;
    #endif
    return result;
}

/// Make Vec4F32 from F32 XY [ Z & W are UNDEFINED! ]
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const F32& x, const F32& y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    //const vector float zero( Const< vector float >::Zero() );
    //result.m_V = spu_insert( x, spu_insert( y, zero, VY ), VX );
    result.m_V = spu_insert( x, spu_promote( y, VY ), VX );
    #elif defined( _PPU_ )
    const vector float vx( vec_lvlx( 0x0, &x ) );
    const vector float vy( vec_lvlx( 0x0, &y ) );
    const vector float zero( Const< vector float >::Zero() );
    result.m_V = vec_merge_xy( vec_merge_xy( vx, zero ),
                               vec_merge_xy( vy, zero ) );
    #elif defined( _VMX128_ )
    const __vector4 vx( __lvlx( &x, 0x0 ) );
    const __vector4 vy( __lvlx( &y, 0x0 ) );
    result.m_V = vmx_merge_xy( vmx_merge_xy( vx, __vzero() ),
                               vmx_merge_xy( vy, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_setr_ps( x, y, 0.0f, 0.0f );
    #else
    result.m_V[VX] = x;
    result.m_V[VY] = y;
    //result.m_V[VZ] = 
    //result.m_V[VW] = 0.0f;
    #endif
    return result;
}

/// Make Vec4F32 from F32 XYZ [ W is UNDEFINED! ]
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const F32& x, const F32& y, const F32& z )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    //const vector float zero( Const< vector float >::Zero() );
    //result.m_V = spu_insert( x, spu_insert( y, spu_insert( z, zero, VZ ), VY ), VX );
    result.m_V = spu_insert( x, spu_insert( y, spu_promote( z, VZ ), VY ), VX );
    #elif defined( _PPU_ )
    const vector float vx( vec_lvlx( 0x0, &x ) );
    const vector float vy( vec_lvlx( 0x0, &y ) );
    const vector float vz( vec_lvlx( 0x0, &z ) );
    const vector float zero( Const< vector float >::Zero() );
    result.m_V = vec_merge_xy( vec_merge_xy( vx, vz ),
                               vec_merge_xy( vy, zero ) );
    #elif defined( _VMX128_ )
    const __vector4 vx( __lvlx( &x, 0x0 ) );
    const __vector4 vy( __lvlx( &y, 0x0 ) );
    const __vector4 vz( __lvlx( &z, 0x0 ) );
    result.m_V = vmx_merge_xy( vmx_merge_xy( vx, vz ),
                               vmx_merge_xy( vy, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_setr_ps( x, y, z, 0.0f );
    #else
    result.m_V[VX] = x;
    result.m_V[VY] = y;
    result.m_V[VZ] = z;
    //result.m_V[VW] = 0.0f;
    #endif
    return result;
}

/// Make Vec4F32 from F32 XYZW
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const F32& x, const F32& y, const F32& z, const F32& w )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_insert( x, spu_insert( y, spu_insert( z, spu_promote( w, VW ), VZ ), VY ), VX );
    #elif defined( _PPU_ )
    const vector float vx( vec_lvlx( 0x0, &x ) );
    const vector float vy( vec_lvlx( 0x0, &y ) );
    const vector float vz( vec_lvlx( 0x0, &z ) );
    const vector float vw( vec_lvlx( 0x0, &w ) );
    result.m_V = vec_merge_xy( vec_merge_xy( vx, vz ),
                               vec_merge_xy( vy, vw ) );
    #elif defined( _VMX128_ )
    const __vector4 vx( __lvlx( &x, 0x0 ) );
    const __vector4 vy( __lvlx( &y, 0x0 ) );
    const __vector4 vz( __lvlx( &z, 0x0 ) );
    const __vector4 vw( __lvlx( &w, 0x0 ) );
    result.m_V = vmx_merge_xy( vmx_merge_xy( vx, vz ), 
                               vmx_merge_xy( vy, vw ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_setr_ps( x, y, z, w );
    #else
    result.m_V[VX] = x;
    result.m_V[VY] = y;
    result.m_V[VZ] = z;
    result.m_V[VW] = w;
    #endif
    return result;
}


/// Make Vec4F32 from a VF32 with the same value broadcast into all elements
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = x.m_V;
    #else
    result.m_V[VX] =
    result.m_V[VY] =
    result.m_V[VZ] =
    result.m_V[VW] = x.m_V;
    #endif
    return result;
}

/// Make Vec4F32 from VF32 XY [ Z & W are UNDEFINED! ]
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x, const VF32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    //result.m_V = spu_insert( spu_extract( x.m_V, VX ),
    //             spu_insert( spu_extract( y.m_V, VY ),
    //             Const< vector float >::Zero(), VY ), VX );
    //result.m_V = spu_insert( spu_extract( x.m_V, VX ), spu_promote( spu_extract( y.m_V, VY ), VY ), VX );
    result.m_V = spu_insert( spu_extract( x.m_V, VX ), y.m_V, VX );
    #elif defined( _PPU_ )
    const vector float zero( Const< vector float >::Zero() );
    result.m_V = vec_merge_xy( vec_merge_xy( x.m_V, zero ), 
                               vec_merge_xy( y.m_V, zero ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_merge_xy( vmx_merge_xy( x.m_V, __vzero() ),
                               vmx_merge_xy( y.m_V, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = sse_merge_xy( sse_merge_xy( x.m_V, _mm_setzero_ps() ),
                               sse_merge_xy( y.m_V, _mm_setzero_ps() ) );
    #else
    result.m_V[VX] = x.m_V;
    result.m_V[VY] = y.m_V;
    //result.m_V[VZ] = 
    //result.m_V[VW] = 0.0f;
    #endif
    return result;
}

/// Make Vec4F32 from VF32 XYZ [ W is UNDEFINED! ]
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    //result.m_V = spu_insert( spu_extract( x.m_V, VX ),
    //             spu_insert( spu_extract( y.m_V, VY ),
    //             spu_insert( spu_extract( z.m_V, VZ ),
    //             Const< vector float >::Zero(), VZ ), VY ), VX );
    result.m_V = spu_insert( spu_extract( x.m_V, VX ), spu_insert( spu_extract( y.m_V, VY ), z.m_V, VY ), VX );
    #elif defined( _PPU_ )
    const vector float zero( Const< vector float >::Zero() );
    result.m_V = vec_merge_xy( vec_merge_xy( x.m_V, z.m_V ),
                               vec_merge_xy( y.m_V, zero ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_merge_xy( vmx_merge_xy( x.m_V, z.m_V ), 
                               vmx_merge_xy( y.m_V, __vzero() ) );
    #elif defined( _SSE_ )
    result.m_V = sse_merge_xy( sse_merge_xy( x.m_V, z.m_V ),
                               sse_merge_xy( y.m_V, _mm_setzero_ps() ) );
    #else
    result.m_V[VX] = x.m_V;
    result.m_V[VY] = y.m_V;
    result.m_V[VZ] = z.m_V;
    //result.m_V[VW] = 0.0f;
    #endif
    return result;
}

/// Make Vec4F32 from VF32 XYZW
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z, const VF32 VECREF w )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_insert( spu_extract( x.m_V, VX ),
                 spu_insert( spu_extract( y.m_V, VY ),
                 spu_insert( spu_extract( z.m_V, VZ ),
                 w.m_V, VZ ), VY ), VX );
    #elif defined( _PPU_ )
    result.m_V = vec_merge_xy( vec_merge_xy( x.m_V, z.m_V ),
                               vec_merge_xy( y.m_V, w.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_merge_xy( vmx_merge_xy( x.m_V, z.m_V ), 
                               vmx_merge_xy( y.m_V, w.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = sse_merge_xy( sse_merge_xy( x.m_V, z.m_V ),
                               sse_merge_xy( y.m_V, w.m_V ) );
    #else
    result.m_V[VX] = x.m_V;
    result.m_V[VY] = y.m_V;
    result.m_V[VZ] = z.m_V;
    result.m_V[VW] = w.m_V;
    #endif
    return result;
}


/// Make Vec4F32 from Point3
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Point3 PCREF p3 )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_select<0,0,0,1>( p3.m_V.m_V, Const< vector float >::One() );
    #elif defined( _PPU_ )
    result.m_V = vec_select<0,0,0,1>( p3.m_V.m_V, Const< vector float >::One() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( p3.m_V.m_V, Const< __vector4 >::One(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V = sse_select<0,0,0,1>( p3.m_V.m_V, Const< __m128 >::One() );
    #else
    result.m_V[VX] = p3.m_V.m_V[VX];
    result.m_V[VY] = p3.m_V.m_V[VY];
    result.m_V[VZ] = p3.m_V.m_V[VZ];
    result.m_V[VW] = 1.0f;
    #endif
    return result;
}

/// Make Vec4F32 from Vector3
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Vector3 PCREF v3 )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_select<0,0,0,1>( v3.m_V.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V = vec_select<0,0,0,1>( v3.m_V.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( v3.m_V.m_V, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V = sse_select<0,0,0,1>( v3.m_V.m_V, Const< __m128 >::Zero() );
    #else
    result.m_V[VX] = v3.m_V.m_V[VX];
    result.m_V[VY] = v3.m_V.m_V[VY];
    result.m_V[VZ] = v3.m_V.m_V[VZ];
    result.m_V[VW] = 0.0f;
    #endif
    return result;
}

/// Make Vec4F32 from Vector3 xyz & VF32 w
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Vector3 PCREF xyz, const VF32 VECREF w )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( xyz.m_V.m_V, w.m_V, VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V = sse_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #else
    result.m_V[VX] = xyz.m_V.m_V[VX];
    result.m_V[VY] = xyz.m_V.m_V[VY];
    result.m_V[VZ] = xyz.m_V.m_V[VZ];
    result.m_V[VW] = w.m_V;
    #endif
    return result;
}

/// Make Vec4F32 from Point3 xyz & VF32 w
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Point3 PCREF xyz, const VF32 VECREF w )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( xyz.m_V.m_V, w.m_V, VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V = sse_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #else
    result.m_V[VX] = xyz.m_V.m_V[VX];
    result.m_V[VY] = xyz.m_V.m_V[VY];
    result.m_V[VZ] = xyz.m_V.m_V[VZ];
    result.m_V[VW] = w.m_V;
    #endif
    return result;
}

/// Make Vec4F32 from Normal3 xyz & VF32 w
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Normal3 PCREF xyz, const VF32 VECREF w )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( xyz.m_V.m_V, w.m_V, VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V = sse_select<0,0,0,1>( xyz.m_V.m_V, w.m_V );
    #else
    result.m_V[VX] = xyz.m_V.m_V[VX];
    result.m_V[VY] = xyz.m_V.m_V[VY];
    result.m_V[VZ] = xyz.m_V.m_V[VZ];
    result.m_V[VW] = w.m_V;
    #endif
    return result;
}

/// Make Vec4F32 from VF32 x & Vector3 yzw
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const VF32 VECREF x, const Vector3 PCREF yzw )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_select<0,1,1,1>( x.m_V, spu_rotate_left_qw_words<3>( yzw.m_V.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_select<0,1,1,1>( x.m_V, vec_rotate_left_qw_words<3>( yzw.m_V.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( x.m_V, yzw.m_V.m_V, VMX_MASK_INSERT_YZW, 3 );
    #elif defined( _SSE_ )
    result.m_V = sse_select<0,1,1,1>( x.m_V, sse_rotate_left_qw_words<3>( yzw.m_V.m_V ) );
    #else
    result.m_V[VX] = x.m_V;
    result.m_V[VY] = yzw.m_V.m_V[VX];
    result.m_V[VZ] = yzw.m_V.m_V[VY];
    result.m_V[VW] = yzw.m_V.m_V[VZ];
    #endif
    return result;
}

/// Make Vec4F32 from Vector4
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Vector4 PCREF v4 )
{
    return v4.m_V;
}

/// Make Vec4F32 from Normal3
NOALIAS __forceinline const Vec4F32 MakeVec4F32( const Normal3 PCREF n3 )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_select<0,0,0,1>( n3.m_V.m_V, Const< vector float >::Zero() );
    #elif defined( _PPU_ )
    result.m_V = vec_select<0,0,0,1>( n3.m_V.m_V, Const< vector float >::Zero() );
    #elif defined( _VMX128_ )
    result.m_V = __vrlimi( n3.m_V.m_V, Const< __vector4 >::Zero(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    result.m_V = sse_select<0,0,0,1>( n3.m_V.m_V, Const< __m128 >::Zero() );
    #else
    result.m_V[VX] = n3.m_V.m_V[VX];
    result.m_V[VY] = n3.m_V.m_V[VY];
    result.m_V[VZ] = n3.m_V.m_V[VZ];
    result.m_V[VW] = 0.0f;
    #endif
    return result;
}



/// Accessor: Get X
NOALIAS __forceinline const VF32 X( const Vec4F32 VECREF v )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<VX>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, VX );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( v.m_V, VX );
    #elif defined( _SSE_ )
    //result.m_V = _mm_shuffle_ps( v.m_V, v.m_V, _MM_SHUFFLE( VX, VX, VX, VX );
    result.m_V = sse_splat<VX>( v.m_V );
    #else
    result.m_V = v.m_V[VX];
    #endif
    return result;
}

/// Accessor: Get Y
NOALIAS __forceinline const VF32 Y( const Vec4F32 VECREF v )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<VY>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, VY );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( v.m_V, VY );
    #elif defined( _SSE_ )
    //result.m_V = _mm_shuffle_ps( v.m_V, v.m_V, _MM_SHUFFLE( VY, VY, VY, VY );
    result.m_V = sse_splat<VY>( v.m_V );
    #else
    result.m_V = v.m_V[VY];
    #endif
    return result;
}

/// Accessor: Get Z
NOALIAS __forceinline const VF32 Z( const Vec4F32 VECREF v )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<VZ>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, VZ );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( v.m_V, VZ );
    #elif defined( _SSE_ )
    //result.m_V = _mm_shuffle_ps( v.m_V, v.m_V, _MM_SHUFFLE( VZ, VZ, VZ, VZ );
    result.m_V = sse_splat<VZ>( v.m_V );
    #else
    result.m_V = v.m_V[VZ];
    #endif
    return result;
}

/// Accessor: Get W
NOALIAS __forceinline const VF32 W( const Vec4F32 VECREF v )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<VW>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, VW );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( v.m_V, VW );
    #elif defined( _SSE_ )
    //result.m_V = _mm_shuffle_ps( v.m_V, v.m_V, _MM_SHUFFLE( VW, VW, VW, VW );
    result.m_V = sse_splat<VW>( v.m_V );
    #else
    result.m_V = v.m_V[VW];
    #endif
    return result;
}

/// Accessor: Set X
NOALIAS __forceinline void SetX( Vec4F32* const __restrict pDst, const VF32 VECREF x )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_select<1,0,0,0>( pDst->m_V, x.m_V );
    #elif defined( _PPU_ )
    pDst->m_V = vec_select<1,0,0,0>( pDst->m_V, x.m_V );
    #elif defined( _VMX128_ )
    pDst->m_V = __vrlimi( pDst->m_V, x.m_V, VMX_MASK_INSERT_X, 0 );
    #elif defined( _SSE_ )
    //const __m128 mask( Const< __m128 >::Mask<0,1,1,1>() );
    //pDst->m_V = _mm_or_ps( _mm_and_ps( mask, pDst->m_V ), _mm_andnot_ps( mask, x.m_V ) );
    pDst->m_V = sse_select<1,0,0,0>( pDst->m_V, x.m_V );
    #else
    pDst->m_V[VX] = x.m_V;
    #endif
}

/// Accessor: Set Y
NOALIAS __forceinline void SetY( Vec4F32* const __restrict pDst, const VF32 VECREF y )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_select<0,1,0,0>( pDst->m_V, y.m_V );
    #elif defined( _PPU_ )
    pDst->m_V = vec_select<0,1,0,0>( pDst->m_V, y.m_V );
    #elif defined( _VMX128_ )
    pDst->m_V = __vrlimi( pDst->m_V, y.m_V, VMX_MASK_INSERT_Y, 0 );
    #elif defined( _SSE_ )
    //const __m128 mask( Const< __m128 >::Mask<1,0,1,1>() );
    //pDst->m_V = _mm_or_ps( _mm_and_ps( mask, pDst->m_V ), _mm_andnot_ps( mask, y.m_V ) );
    pDst->m_V = sse_select<0,1,0,0>( pDst->m_V, y.m_V );
    #else
    pDst->m_V[VY] = y.m_V;
    #endif
}

/// Accessor: Set Z
NOALIAS __forceinline void SetZ( Vec4F32* const __restrict pDst, const VF32 VECREF z )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_select<0,0,1,0>( pDst->m_V, z.m_V );
    #elif defined( _PPU_ )
    pDst->m_V = vec_select<0,0,1,0>( pDst->m_V, z.m_V );
    #elif defined( _VMX128_ )
    pDst->m_V = __vrlimi( pDst->m_V, z.m_V, VMX_MASK_INSERT_Z, 0 );
    #elif defined( _SSE_ )
    //const __m128 mask( Const< __m128 >::Mask<1,1,0,1>() );
    //pDst->m_V = _mm_or_ps( _mm_and_ps( mask, pDst->m_V ), _mm_andnot_ps( mask, z.m_V ) );
    pDst->m_V = sse_select<0,0,1,0>( pDst->m_V, z.m_V );
    #else
    pDst->m_V[VZ] = z.m_V;
    #endif
}

/// Accessor: Set W
NOALIAS __forceinline void SetW( Vec4F32* const __restrict pDst, const VF32 VECREF w )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_select<0,0,0,1>( pDst->m_V, w.m_V );
    #elif defined( _PPU_ )
    pDst->m_V = vec_select<0,0,0,1>( pDst->m_V, w.m_V );
    #elif defined( _VMX128_ )
    pDst->m_V = __vrlimi( pDst->m_V, w.m_V, VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    //const __m128 mask( Const< __m128 >::Mask<1,1,1,0>() );
    //pDst->m_V = _mm_or_ps( _mm_and_ps( mask, pDst->m_V ), _mm_andnot_ps( mask, w.m_V ) );
    pDst->m_V = sse_select<0,0,0,1>( pDst->m_V, w.m_V );
    #else
    pDst->m_V[VW] = w.m_V;
    #endif
}


/// Vec4F32 Accessor: Get element
template < unsigned int I >
NOALIAS __forceinline const VF32 Element( const Vec4F32 VECREF v )
{
    BIZ_STATIC_ASSERT( I < 4 );
    switch ( I )
    {
        case 0  : return X( v );
        case 1  : return Y( v );
        case 2  : return Z( v );
        case 3  : return W( v );
        default : return Const<VF32>::Zero();
    }
}

/// Vec4F32 Accessor: Set element
template < unsigned int I >
NOALIAS __forceinline void SetElement( Vec4F32* const __restrict pDst, const VF32 VECREF ei )
{
    BIZ_STATIC_ASSERT( I < 4 );
    switch ( I )
    {
        case 0  : SetX( pDst, ei ); break;
        case 1  : SetY( pDst, ei ); break;
        case 2  : SetZ( pDst, ei ); break;
        case 3  : SetW( pDst, ei ); break;
        default : break;
    }
}



/// Vec4F32 Write to Stream
template < typename T_Stream > void Serialise( const Vec4F32 VECREF src, T_Stream& stream )
{
    StorageVec4F32 storage;
    Store( src, &storage );
    stream << storage.m_Data.x;
    stream << storage.m_Data.y;
    stream << storage.m_Data.z;
    stream << storage.m_Data.w;
}

/// Vec4F32 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, Vec4F32* const __restrict pDst )
{
    StorageVec4F32 storage;
    stream >> storage.m_Data.x;
    stream >> storage.m_Data.y;
    stream >> storage.m_Data.z;
    stream >> storage.m_Data.w;
    *pDst = Load<Vec4F32>( &storage );
}




/// Vec4F32 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az ], W element is preserved
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vec4F32 Permute( const Vec4F32 VECREF v )
{
    StaticAssert( IX < 3, IX_MUST_BE_LESS_THAN_3 );
    StaticAssert( IY < 3, IY_MUST_BE_LESS_THAN_3 );
    StaticAssert( IZ < 3, IZ_MUST_BE_LESS_THAN_3 );
    
    if ( ( IX == 0 ) && ( IY == 1 ) && ( IZ == 2 ) )
    {
        return v;
    }
    else
    {
        Vec4F32 result;
        #if defined( _SPU_ )
        result.m_V = spu_permute<IX,IY,IZ,VW>( v.m_V );
        #elif defined( _PPU_ )
        result.m_V = vec_permute<IX,IY,IZ,VW>( v.m_V );
        #elif defined( _VMX128_ )
        result.m_V = vmx_permute<IX,IY,IZ,VW>( v.m_V );
        #elif defined( _SSE_ )
        result.m_V = sse_permute<IX,IY,IZ,VW>( v.m_V );
        #else
        result.m_V[VX] = v.m_V[IX];
        result.m_V[VY] = v.m_V[IY];
        result.m_V[VZ] = v.m_V[IZ];
        result.m_V[VW] = v.m_V[VW];
        #endif
        return result;
    }
}

/// Vec4F32 Permute : Indices select 4 result elements [x,y,z,w] from [ 0:ax, 1:ay, 2:az, 3:aw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Vec4F32 Permute( const Vec4F32 VECREF v )
{
    StaticAssert( IX < 4, IX_MUST_BE_LESS_THAN_4 );
    StaticAssert( IY < 4, IY_MUST_BE_LESS_THAN_4 );
    StaticAssert( IZ < 4, IZ_MUST_BE_LESS_THAN_4 );
    StaticAssert( IW < 4, IW_MUST_BE_LESS_THAN_4 );
    
    if ( ( IX == 0 ) && ( IY == 1 ) && ( IZ == 2 ) && ( IW == 3 ) )
    {
        return v;
    }
    else
    {
        Vec4F32 result;
        #if defined( _SPU_ )
        result.m_V = spu_permute<IX,IY,IZ,IW>( v.m_V );
        #elif defined( _PPU_ )
        result.m_V = vec_permute<IX,IY,IZ,IW>( v.m_V );
        #elif defined( _VMX128_ )
        result.m_V = vmx_permute<IX,IY,IZ,IW>( v.m_V );
        #elif defined( _SSE_ )
        result.m_V = sse_permute<IX,IY,IZ,IW>( v.m_V );
        #else
        result.m_V[VX] = v.m_V[IX];
        result.m_V[VY] = v.m_V[IY];
        result.m_V[VZ] = v.m_V[IZ];
        result.m_V[VW] = v.m_V[IW];
        #endif
        return result;
    }
}

/// Vec4F32 Permute : Indices select 3 result elements [x,y,z] from [ 0:ax, 1:ay, 2:az, 3:aw, 4:bx, 5:by, 6:bz, 7:bw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ >
NOALIAS __forceinline const Vec4F32 Permute( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    StaticAssert( IX < 8, IX_MUST_BE_LESS_THAN_8 );
    StaticAssert( IY < 8, IY_MUST_BE_LESS_THAN_8 );
    StaticAssert( IZ < 8, IZ_MUST_BE_LESS_THAN_8 );
    
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_permute<IX,IY,IZ,IZ>( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_permute<IX,IY,IZ,IZ>( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_permute<IX,IY,IZ,IZ>( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_permute<IX,IY,IZ,IZ>( a.m_V, b.m_V );
    #else
    //result.m_V[VX] = IX < 4 ? a.m_V[IX] : b.m_V[IX-4];
    //result.m_V[VY] = IX < 4 ? a.m_V[IY] : b.m_V[IY-4];
    //result.m_V[VZ] = IX < 4 ? a.m_V[IZ] : b.m_V[IZ-4];
    //result.m_V[VW] = IX < 4 ? a.m_V[IZ] : b.m_V[IZ-4];
    result.m_V[VX] = IX < 4 ? a.m_V[IX] : b.m_V[IX%4];
    result.m_V[VY] = IX < 4 ? a.m_V[IY] : b.m_V[IY%4];
    result.m_V[VZ] = IX < 4 ? a.m_V[IZ] : b.m_V[IZ%4];
    result.m_V[VW] = IX < 4 ? a.m_V[IZ] : b.m_V[IZ%4];
    #endif
    return result;
}

/// Vec4F32 Permute : Indices select 4 result elements [x,y,z,w] from [ 0:ax, 1:ay, 2:az, 3:aw, 4:bx, 5:by, 6:bz, 7:bw ]
template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const Vec4F32 Permute( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    StaticAssert( IX < 8, IX_MUST_BE_LESS_THAN_8 );
    StaticAssert( IY < 8, IY_MUST_BE_LESS_THAN_8 );
    StaticAssert( IZ < 8, IZ_MUST_BE_LESS_THAN_8 );
    StaticAssert( IW < 8, IW_MUST_BE_LESS_THAN_8 );
    
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_permute<IX,IY,IZ,IW>( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_permute<IX,IY,IZ,IW>( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_permute<IX,IY,IZ,IW>( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_permute<IX,IY,IZ,IW>( a.m_V, b.m_V );
    #else
    //result.m_V[VX] = IX < 4 ? a.m_V[IX] : b.m_V[IX-4];
    //result.m_V[VY] = IX < 4 ? a.m_V[IY] : b.m_V[IY-4];
    //result.m_V[VZ] = IX < 4 ? a.m_V[IZ] : b.m_V[IZ-4];
    //result.m_V[VW] = IX < 4 ? a.m_V[IW] : b.m_V[IW-4];
    result.m_V[VX] = IX < 4 ? a.m_V[IX] : b.m_V[IX%4];
    result.m_V[VY] = IX < 4 ? a.m_V[IY] : b.m_V[IY%4];
    result.m_V[VZ] = IX < 4 ? a.m_V[IZ] : b.m_V[IZ%4];
    result.m_V[VW] = IX < 4 ? a.m_V[IW] : b.m_V[IW%4];
    #endif
    return result;
}


/// Vec4F32 MergeXY : Merges the x & y elements from a & b --> [ ax, bx, ay, by ]
NOALIAS __forceinline const Vec4F32 MergeXY( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_merge_xy( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_merge_xy( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_merge_xy( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_merge_xy( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX];
    result.m_V[VY] = b.m_V[VX];
    result.m_V[VZ] = a.m_V[VY];
    result.m_V[VW] = b.m_V[VY];
    #endif
    return result;
}

/// Vec4F32 MergeZW : Merges the z & w elements from a & b --> [ az, bz, aw, bw ]
NOALIAS __forceinline const Vec4F32 MergeZW( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_merge_xy( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_merge_xy( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_merge_xy( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_merge_xy( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VZ];
    result.m_V[VY] = b.m_V[VZ];
    result.m_V[VZ] = a.m_V[VW];
    result.m_V[VW] = b.m_V[VW];
    #endif
    return result;
}


/// Vec4F32 Abs of each element
NOALIAS __forceinline const Vec4F32 Abs( const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_abs( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_abs( v.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vand( v.m_V, Const< __vector4 >::AbsMask() );
    result.m_V = vmx_abs( v.m_V );
    #elif defined( _SSE_ )
    //result.m_V = _mm_and_ps( v.m_V, Const< __m128 >::AbsMask() );
    result.m_V = sse_abs( v.m_V );
    #else
    result.m_V[VX] = Maths::Abs( v.m_V[VX] );
    result.m_V[VY] = Maths::Abs( v.m_V[VY] );
    result.m_V[VZ] = Maths::Abs( v.m_V[VZ] );
    result.m_V[VW] = Maths::Abs( v.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Sign of each element : For each x in v: returns -1.0f if x < 0.0f, +1.0f if x > 0.0f, or 0.0f if x == 0.0f 
NOALIAS __forceinline const Vec4F32 Sign( const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    const vector float zero( Const< vector float >::Zero() );
    const vector float one( Const< vector float >::One() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    result.m_V = spu_sel( (vector float)spu_andc( one, (vector float)spu_cmpeq( v.m_V, zero ) ), v.m_V, sign_mask );
    #elif defined( _PPU_ )
    const vector float zero( Const< vector float >::Zero() );
    const vector float one( Const< vector float >::One() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    result.m_V = vec_sel( (vector float)vec_andc( one, (vector float)vec_vcmpeqfp( v.m_V, zero ) ), v.m_V, sign_mask );
    #elif defined( _VMX128_ )
    const __vector4 zero( Const< __vector4 >::Zero() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 sign_mask( Const< __vector4 >::SignMask() );
    result.m_V = __vsel( __vandc( one, __vcmpeqfp( v.m_V, zero ) ), v.m_V, sign_mask );
    #elif defined( _SSE_ )
    const __m128 zero( _mm_setzero_ps() );
    const __m128 one( Const< __m128 >::One() );
    const __m128 sign_mask( Const< __m128 >::SignMask() );
    const __m128 signed_one( _mm_or_ps( _mm_and_ps( sign_mask, v.m_V ), one ) );
    //const __m128 not_zero( _mm_cmpneq_ps( _mm_andnot_ps( sign_mask, v.m_V ), zero ) );
    const __m128 not_zero( _mm_cmpneq_ps( v.m_V, zero ) );
    result.m_V = _mm_and_ps( signed_one, not_zero );
    #else
    result.m_V[VX] = ( v.m_V[VX] < 0.0f ? -1.0f : ( v.m_V[VX] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[VY] = ( v.m_V[VY] < 0.0f ? -1.0f : ( v.m_V[VY] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[VZ] = ( v.m_V[VZ] < 0.0f ? -1.0f : ( v.m_V[VZ] > 0.0f ? 1.0f : 0.0f ) );
    result.m_V[VW] = ( v.m_V[VW] < 0.0f ? -1.0f : ( v.m_V[VW] > 0.0f ? 1.0f : 0.0f ) );
    #endif
    return result;
}

/// Vec4F32 CopySign of each element : Returns a Vec4F32 with each element having it's abs value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const Vec4F32 CopySign( const Vec4F32 VECREF magnitude, const Vec4F32 VECREF sign )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( magnitude.m_V, sign.m_V, Const< vector unsigned int >::SignMask() );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( magnitude.m_V, sign.m_V, Const< vector unsigned int >::SignMask() );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( magnitude.m_V, sign.m_V, Const< __vector4 >::SignMask() );
    #elif defined( _SSE_ )
    const __m128 mask( Const< __m128 >::SignMask() );
    result.m_V = _mm_or_ps( _mm_and_ps( mask, sign.m_V ), _mm_andnot_ps( mask, magnitude.m_V ) );
    #else
    result.m_V[VX] = (F32)_copysign( magnitude.m_V[VX], sign.m_V[VX] );
    result.m_V[VY] = (F32)_copysign( magnitude.m_V[VY], sign.m_V[VY] );
    result.m_V[VZ] = (F32)_copysign( magnitude.m_V[VZ], sign.m_V[VZ] );
    result.m_V[VW] = (F32)_copysign( magnitude.m_V[VW], sign.m_V[VW] );
    #endif
    return result;
}


/// Vec4F32 Negation : Indices select elements to negate [X,Y,Z,W]
template < bool X, bool Y, bool Z, bool W >
NOALIAS __forceinline const Vec4F32 Negate( const Vec4F32 VECREF v )
{
    if ( X || Y || Z || W )
        return XOR( v, Const<Vec4F32>::XorNegate<X,Y,Z,W>() );
    else
        return v;
}


/// Vec4F32 Negate X
NOALIAS __forceinline const Vec4F32 NegateX( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateX() );
}

/// Vec4F32 Negate Y
NOALIAS __forceinline const Vec4F32 NegateY( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateY() );
}

/// Vec4F32 Negate Z
NOALIAS __forceinline const Vec4F32 NegateZ( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateZ() );
}

/// Vec4F32 Negate W
NOALIAS __forceinline const Vec4F32 NegateW( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateW() );
}


/// Vec4F32 Negate XY
NOALIAS __forceinline const Vec4F32 NegateXY( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateXY() );
}

/// Vec4F32 Negate XZ
NOALIAS __forceinline const Vec4F32 NegateXZ( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateXZ() );
}

/// Vec4F32 Negate XW
NOALIAS __forceinline const Vec4F32 NegateXW( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateXW() );
}

/// Vec4F32 Negate YZ
NOALIAS __forceinline const Vec4F32 NegateYZ( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateYZ() );
}

/// Vec4F32 Negate YW
NOALIAS __forceinline const Vec4F32 NegateYW( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateYW() );
}

/// Vec4F32 Negate ZW
NOALIAS __forceinline const Vec4F32 NegateZW( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateZW() );
}


/// Vec4F32 Negate XYZ
NOALIAS __forceinline const Vec4F32 NegateXYZ( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateXYZ() );
}

/// Vec4F32 Negate XYW
NOALIAS __forceinline const Vec4F32 NegateXYW( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateXYW() );
}

/// Vec4F32 Negate XZW
NOALIAS __forceinline const Vec4F32 NegateXZW( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateXZW() );
}

/// Vec4F32 Negate YZW
NOALIAS __forceinline const Vec4F32 NegateYZW( const Vec4F32 VECREF src )
{
    return XOR( src, Const<Vec4F32>::XorNegateYZW() );
}



/// Vec4F32 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Rcp( const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Rcp<22>( v.m_V );
    #else
    result.m_V[VX] = Maths::Reciprocal( v.m_V[VX] );
    result.m_V[VY] = Maths::Reciprocal( v.m_V[VY] );
    result.m_V[VZ] = Maths::Reciprocal( v.m_V[VZ] );
    result.m_V[VW] = Maths::Reciprocal( v.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Rcp24( const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Rcp<24>( v.m_V );
    #else
    result.m_V[VX] = Maths::Reciprocal( v.m_V[VX] );
    result.m_V[VY] = Maths::Reciprocal( v.m_V[VY] );
    result.m_V[VZ] = Maths::Reciprocal( v.m_V[VZ] );
    result.m_V[VW] = Maths::Reciprocal( v.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Division : vector / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Div( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Div<22>( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] * Maths::Reciprocal( y.m_V[VX] );
    result.m_V[VY] = x.m_V[VY] * Maths::Reciprocal( y.m_V[VY] );
    result.m_V[VZ] = x.m_V[VZ] * Maths::Reciprocal( y.m_V[VZ] );
    result.m_V[VW] = x.m_V[VW] * Maths::Reciprocal( y.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Division : vector / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Div24( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Div<24>( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] * Maths::Reciprocal( y.m_V[VX] );
    result.m_V[VY] = x.m_V[VY] * Maths::Reciprocal( y.m_V[VY] );
    result.m_V[VZ] = x.m_V[VZ] * Maths::Reciprocal( y.m_V[VZ] );
    result.m_V[VW] = x.m_V[VW] * Maths::Reciprocal( y.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Division : scalar / vector (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Div( const VF32 VECREF x, const Vec4F32 VECREF y )
{
    return Div( MakeVec4F32( x ), y );
}

/// Vec4F32 Division : scalar / vector (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Div24( const VF32 VECREF x, const Vec4F32 VECREF y )
{
    return Div24( MakeVec4F32( x ), y );
}

/// Vec4F32 Division : vector / scalar (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Div( const Vec4F32 VECREF x, const VF32 VECREF y )
{
    return Div( x, MakeVec4F32( y ) );
}

/// Vec4F32 Division : vector / scalar (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Div24( const Vec4F32 VECREF x, const VF32 VECREF y )
{
    return Div24( x, MakeVec4F32( y ) );
}

/// Vec4F32 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 RcpSqrt( const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_RcpSqrt<22>( v.m_V );
    #else
    result.m_V[VX] = Maths::ReciprocalSqrt( v.m_V[VX] );
    result.m_V[VY] = Maths::ReciprocalSqrt( v.m_V[VY] );
    result.m_V[VZ] = Maths::ReciprocalSqrt( v.m_V[VZ] );
    result.m_V[VW] = Maths::ReciprocalSqrt( v.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 RcpSqrt24( const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_RcpSqrt<24>( v.m_V );
    #else
    result.m_V[VX] = Maths::ReciprocalSqrt( v.m_V[VX] );
    result.m_V[VY] = Maths::ReciprocalSqrt( v.m_V[VY] );
    result.m_V[VZ] = Maths::ReciprocalSqrt( v.m_V[VZ] );
    result.m_V[VW] = Maths::ReciprocalSqrt( v.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Sqrt( const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Sqrt<22>( v.m_V );
    #else
    result.m_V[VX] = Maths::Sqrt( v.m_V[VX] );
    result.m_V[VY] = Maths::Sqrt( v.m_V[VY] );
    result.m_V[VZ] = Maths::Sqrt( v.m_V[VZ] );
    result.m_V[VW] = Maths::Sqrt( v.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Sqrt24( const Vec4F32 VECREF v )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Sqrt<24>( v.m_V );
    #else
    result.m_V[VX] = Maths::Sqrt( v.m_V[VX] );
    result.m_V[VY] = Maths::Sqrt( v.m_V[VY] );
    result.m_V[VZ] = Maths::Sqrt( v.m_V[VZ] );
    result.m_V[VW] = Maths::Sqrt( v.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const Vec4F32 VECREF v,
                                       Vec4F32* const __restrict pRcp,
                                       Vec4F32* const __restrict pSqrt,
                                       Vec4F32* const __restrict pRcpSqrt )
{
    #if defined( _SIMD_ )
    const V128 rcp_v( SIMD_Rcp<22>( v.m_V ) );
    const V128 rcp_sqrt_v( SIMD_RcpSqrt<22>( v.m_V ) );
    pRcp->m_V     = rcp_v;
    pRcpSqrt->m_V = rcp_sqrt_v;
    #if defined( _SPU_ )
    pSqrt->m_V = spu_mul( v.m_V, rcp_sqrt_v );
    #elif defined( _PPU_ )
    pSqrt->m_V = vec_mul( v.m_V, rcp_sqrt_v );
    #elif defined( _VMX128_ )
    pSqrt->m_V = __vmulfp( v.m_V, rcp_sqrt_v );
    #elif defined( _SSE_ )
    pSqrt->m_V = _mm_mul_ps( v.m_V, rcp_sqrt_v );
    #endif
    #else
    pRcp->m_V[VX]     = Maths::Reciprocal( v.m_V[VX] );
    pRcp->m_V[VY]     = Maths::Reciprocal( v.m_V[VY] );
    pRcp->m_V[VZ]     = Maths::Reciprocal( v.m_V[VZ] );
    pRcp->m_V[VW]     = Maths::Reciprocal( v.m_V[VW] );
    pRcpSqrt->m_V[VX] = Maths::ReciprocalSqrt( v.m_V[VX] );
    pRcpSqrt->m_V[VY] = Maths::ReciprocalSqrt( v.m_V[VY] );
    pRcpSqrt->m_V[VZ] = Maths::ReciprocalSqrt( v.m_V[VZ] );
    pRcpSqrt->m_V[VW] = Maths::ReciprocalSqrt( v.m_V[VW] );
    pSqrt->m_V[VX]    = v.m_V[VX] * pRcpSqrt->m_V[VX];
    pSqrt->m_V[VY]    = v.m_V[VY] * pRcpSqrt->m_V[VY];
    pSqrt->m_V[VZ]    = v.m_V[VZ] * pRcpSqrt->m_V[VZ];
    pSqrt->m_V[VW]    = v.m_V[VW] * pRcpSqrt->m_V[VW];
    #endif
}

/// Vec4F32 Reciprocals And Square Root : For each element x in v, returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const Vec4F32 VECREF v,
                                         Vec4F32* const __restrict pRcp,
                                         Vec4F32* const __restrict pSqrt,
                                         Vec4F32* const __restrict pRcpSqrt )
{
    #if defined( _SIMD_ )
    const V128 rcp_v( SIMD_Rcp<24>( v.m_V ) );
    const V128 rcp_sqrt_v( SIMD_RcpSqrt<24>( v.m_V ) );
    pRcp->m_V     = rcp_v;
    pRcpSqrt->m_V = rcp_sqrt_v;
    #if defined( _SPU_ )
    pSqrt->m_V = spu_mul( v.m_V, rcp_sqrt_v );
    #elif defined( _PPU_ )
    pSqrt->m_V = vec_mul( v.m_V, rcp_sqrt_v );
    #elif defined( _VMX128_ )
    pSqrt->m_V = __vmulfp( v.m_V, rcp_sqrt_v );
    #elif defined( _SSE_ )
    pSqrt->m_V = _mm_mul_ps( v.m_V, rcp_sqrt_v );
    #endif
    #else
    pRcp->m_V[VX]     = Maths::Reciprocal( v.m_V[VX] );
    pRcp->m_V[VY]     = Maths::Reciprocal( v.m_V[VY] );
    pRcp->m_V[VZ]     = Maths::Reciprocal( v.m_V[VZ] );
    pRcp->m_V[VW]     = Maths::Reciprocal( v.m_V[VW] );
    pRcpSqrt->m_V[VX] = Maths::ReciprocalSqrt( v.m_V[VX] );
    pRcpSqrt->m_V[VY] = Maths::ReciprocalSqrt( v.m_V[VY] );
    pRcpSqrt->m_V[VZ] = Maths::ReciprocalSqrt( v.m_V[VZ] );
    pRcpSqrt->m_V[VW] = Maths::ReciprocalSqrt( v.m_V[VW] );
    pSqrt->m_V[VX]    = v.m_V[VX] * pRcpSqrt->m_V[VX];
    pSqrt->m_V[VY]    = v.m_V[VY] * pRcpSqrt->m_V[VY];
    pSqrt->m_V[VZ]    = v.m_V[VZ] * pRcpSqrt->m_V[VZ];
    pSqrt->m_V[VW]    = v.m_V[VW] * pRcpSqrt->m_V[VW];
    #endif
}


/// Vec4F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const Vec4F32 VECREF v,
                                           Vec4F32* const __restrict pSqrt,
                                           Vec4F32* const __restrict pRcpSqrt )
{
    #if defined( _SIMD_ )
    const V128 rcp_sqrt_v( SIMD_RcpSqrt<22>( v.m_V ) );
    pRcpSqrt->m_V = rcp_sqrt_v;
    #if defined( _SPU_ )
    pSqrt->m_V = spu_mul( v.m_V, rcp_sqrt_v );
    #elif defined( _PPU_ )
    pSqrt->m_V = vec_mul( v.m_V, rcp_sqrt_v );
    #elif defined( _VMX128_ )
    pSqrt->m_V = __vmulfp( v.m_V, rcp_sqrt_v );
    #elif defined( _SSE_ )
    pSqrt->m_V = _mm_mul_ps( v.m_V, rcp_sqrt_v );
    #endif
    #else
    pRcpSqrt->m_V[VX] = Maths::ReciprocalSqrt( v.m_V[VX] );
    pRcpSqrt->m_V[VY] = Maths::ReciprocalSqrt( v.m_V[VY] );
    pRcpSqrt->m_V[VZ] = Maths::ReciprocalSqrt( v.m_V[VZ] );
    pRcpSqrt->m_V[VW] = Maths::ReciprocalSqrt( v.m_V[VW] );
    pSqrt->m_V[VX]    = v.m_V[VX] * pRcpSqrt->m_V[VX];
    pSqrt->m_V[VY]    = v.m_V[VY] * pRcpSqrt->m_V[VY];
    pSqrt->m_V[VZ]    = v.m_V[VZ] * pRcpSqrt->m_V[VZ];
    pSqrt->m_V[VW]    = v.m_V[VW] * pRcpSqrt->m_V[VW];
    #endif
}

/// Vec4F32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const Vec4F32 VECREF v,
                                             Vec4F32* const __restrict pSqrt,
                                             Vec4F32* const __restrict pRcpSqrt )
{
    #if defined( _SIMD_ )
    const V128 rcp_sqrt_v( SIMD_RcpSqrt<24>( v.m_V ) );
    pRcpSqrt->m_V = rcp_sqrt_v;
    #if defined( _SPU_ )
    pSqrt->m_V = spu_mul( v.m_V, rcp_sqrt_v );
    #elif defined( _PPU_ )
    pSqrt->m_V = vec_mul( v.m_V, rcp_sqrt_v );
    #elif defined( _VMX128_ )
    pSqrt->m_V = __vmulfp( v.m_V, rcp_sqrt_v );
    #elif defined( _SSE_ )
    pSqrt->m_V = _mm_mul_ps( v.m_V, rcp_sqrt_v );
    #endif
    #else
    pRcpSqrt->m_V[VX] = Maths::ReciprocalSqrt( v.m_V[VX] );
    pRcpSqrt->m_V[VY] = Maths::ReciprocalSqrt( v.m_V[VY] );
    pRcpSqrt->m_V[VZ] = Maths::ReciprocalSqrt( v.m_V[VZ] );
    pRcpSqrt->m_V[VW] = Maths::ReciprocalSqrt( v.m_V[VW] );
    pSqrt->m_V[VX]    = v.m_V[VX] * pRcpSqrt->m_V[VX];
    pSqrt->m_V[VY]    = v.m_V[VY] * pRcpSqrt->m_V[VY];
    pSqrt->m_V[VZ]    = v.m_V[VZ] * pRcpSqrt->m_V[VZ];
    pSqrt->m_V[VW]    = v.m_V[VW] * pRcpSqrt->m_V[VW];
    #endif
}


/// Vec4F32 Unary Positive Operator
NOALIAS __forceinline const Vec4F32 operator + ( const Vec4F32 VECREF x )
{
    return x;
}

/// Vec4F32 Unary Negation Operator
NOALIAS __forceinline const Vec4F32 operator - ( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_negate( x.m_V );
    #elif defined( _PPU_ )
    //result.m_V = vec_xor( x.m_V, Const< vector float >::SignMask() );
    result.m_V = vec_negate( x.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vxor( x.m_V, Const< __vector4 >::SignMask() );
    result.m_V = vmx_negate( x.m_V );
    #elif defined( _SSE_ )
    //result.m_V = _mm_xor_ps( x.m_V, Const< __m128 >::SignMask() );
    result.m_V = sse_negate( x.m_V );
    #else
    result.m_V[VX] = -x.m_V[VX];
    result.m_V[VY] = -x.m_V[VY];
    result.m_V[VZ] = -x.m_V[VZ];
    result.m_V[VW] = -x.m_V[VW];
    #endif
    return result;
}




/// Vec4F32 Binary Addition Operator
NOALIAS __forceinline const Vec4F32 operator + ( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_add( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_add( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vaddfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] + y.m_V[VX];
    result.m_V[VY] = x.m_V[VY] + y.m_V[VY];
    result.m_V[VZ] = x.m_V[VZ] + y.m_V[VZ];
    result.m_V[VW] = x.m_V[VW] + y.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Binary Subtraction Operator
NOALIAS __forceinline const Vec4F32 operator - ( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sub( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sub( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsubfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] - y.m_V[VX];
    result.m_V[VY] = x.m_V[VY] - y.m_V[VY];
    result.m_V[VZ] = x.m_V[VZ] - y.m_V[VZ];
    result.m_V[VW] = x.m_V[VW] - y.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Binary Multiplication Operator
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] * y.m_V[VX];
    result.m_V[VY] = x.m_V[VY] * y.m_V[VY];
    result.m_V[VZ] = x.m_V[VZ] * y.m_V[VZ];
    result.m_V[VW] = x.m_V[VW] * y.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Binary Division Operator
NOALIAS __forceinline const Vec4F32 operator / ( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Div<22>( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] * Maths::Reciprocal( y.m_V[VX] );
    result.m_V[VY] = x.m_V[VY] * Maths::Reciprocal( y.m_V[VY] );
    result.m_V[VZ] = x.m_V[VZ] * Maths::Reciprocal( y.m_V[VZ] );
    result.m_V[VW] = x.m_V[VW] * Maths::Reciprocal( y.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Add Assign Operator
NOALIAS __forceinline const Vec4F32& operator += ( Vec4F32& x, const Vec4F32 VECREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_add( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_add( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vaddfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_add_ps( x.m_V, y.m_V );
    #else
    x.m_V[VX] += y.m_V[VX];
    x.m_V[VY] += y.m_V[VY];
    x.m_V[VZ] += y.m_V[VZ];
    x.m_V[VW] += y.m_V[VW];
    #endif
    return x;
}

/// Vec4F32 Subtract Assign Operator
NOALIAS __forceinline const Vec4F32& operator -= ( Vec4F32& x, const Vec4F32 VECREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_sub( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_sub( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vsubfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_sub_ps( x.m_V, y.m_V );
    #else
    x.m_V[VX] -= y.m_V[VX];
    x.m_V[VY] -= y.m_V[VY];
    x.m_V[VZ] -= y.m_V[VZ];
    x.m_V[VW] -= y.m_V[VW];
    #endif
    return x;
}

/// Vec4F32 Multiply Assign Operator
NOALIAS __forceinline const Vec4F32& operator *= ( Vec4F32& x, const Vec4F32 VECREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_mul( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_mul( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vmulfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_mul_ps( x.m_V, y.m_V );
    #else
    x.m_V[VX] *= y.m_V[VX];
    x.m_V[VY] *= y.m_V[VY];
    x.m_V[VZ] *= y.m_V[VZ];
    x.m_V[VW] *= y.m_V[VW];
    #endif
    return x;
}

/// Vec4F32 Divide Assign Operator
NOALIAS __forceinline const Vec4F32& operator /= ( Vec4F32& x, const Vec4F32 VECREF y )
{
    #if defined( _SIMD_ )
    x.m_V = SIMD_Div<22>( x.m_V, y.m_V );
    #else
    x.m_V[VX] *= Maths::Reciprocal( y.m_V[VX] );
    x.m_V[VY] *= Maths::Reciprocal( y.m_V[VY] );
    x.m_V[VZ] *= Maths::Reciprocal( y.m_V[VZ] );
    x.m_V[VW] *= Maths::Reciprocal( y.m_V[VW] );
    #endif
    return x;
}

/// Vec4F32 Binary Multiplication Operator : vector * scalar
NOALIAS __forceinline const Vec4F32 operator * ( const Vec4F32 VECREF x, const VF32 VECREF y )
{
    return x * MakeVec4F32( y );
}

/// Vec4F32 Binary Multiplication Operator : scalar * vector
NOALIAS __forceinline const Vec4F32 operator * ( const VF32 VECREF x, const Vec4F32 VECREF y )
{
    return MakeVec4F32( x ) * y;
}

/// Vec4F32 Binary Division Operator : vector * scalar
NOALIAS __forceinline const Vec4F32 operator / ( const Vec4F32 VECREF x, const VF32 VECREF y )
{
    return x / MakeVec4F32( y );
}

/// Vec4F32 Binary Division Operator : scalar * vector
NOALIAS __forceinline const Vec4F32 operator / ( const VF32 VECREF x, const Vec4F32 VECREF y )
{
    return MakeVec4F32( x ) / y;
}

/// Vec4F32 Multiply Assign Operator : vector *= scalar
NOALIAS __forceinline const Vec4F32& operator *= ( Vec4F32& x, const VF32 VECREF y )
{
    x *= MakeVec4F32( y );
    return x;
}

/// Vec4F32 Divide Assign Operator : vector / scalar
NOALIAS __forceinline const Vec4F32& operator /= ( Vec4F32& x, const VF32 VECREF y )
{
    x /= MakeVec4F32( y );
    return x;
}


/// Vec4F32 MulAdd : a * b + c
NOALIAS __forceinline const Vec4F32 MulAdd( const Vec4F32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_madd( a.m_V, b.m_V, c.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_madd( a.m_V, b.m_V, c.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmaddfp( a.m_V, b.m_V, c.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_ps( _mm_mul_ps( a.m_V, b.m_V ), c.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] * b.m_V[VX] + c.m_V[VX];
    result.m_V[VY] = a.m_V[VY] * b.m_V[VY] + c.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] * b.m_V[VZ] + c.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] * b.m_V[VW] + c.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 MulSub : a * b - c
NOALIAS __forceinline const Vec4F32 MulSub( const Vec4F32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_msub( a.m_V, b.m_V, c.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_madd( a.m_V, b.m_V, vec_xor( c.m_V, Const< vector float >::SignMask() ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmaddfp( a.m_V, b.m_V, __vxor( c.m_V, Const<__vector4>::SignMask() ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_ps( _mm_mul_ps( a.m_V, b.m_V ), c.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] * b.m_V[VX] - c.m_V[VX];
    result.m_V[VY] = a.m_V[VY] * b.m_V[VY] - c.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] * b.m_V[VZ] - c.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] * b.m_V[VW] - c.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const Vec4F32 NegMulAdd( const Vec4F32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( spu_madd( a.m_V, b.m_V, c.m_V ), Const< vector float >::SignMask() );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( vec_madd( a.m_V, b.m_V, c.m_V ), Const< vector float >::SignMask() );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( __vmaddfp( a.m_V, b.m_V, c.m_V ), Const<__vector4>::SignMask() );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_ps( _mm_xor_ps( c.m_V, Const<__m128>::SignMask() ), _mm_mul_ps( a.m_V, b.m_V ) );
    #else
    result.m_V[VX] = -a.m_V[VX] * b.m_V[VX] - c.m_V[VX];
    result.m_V[VY] = -a.m_V[VY] * b.m_V[VY] - c.m_V[VY];
    result.m_V[VZ] = -a.m_V[VZ] * b.m_V[VZ] - c.m_V[VZ];
    result.m_V[VW] = -a.m_V[VW] * b.m_V[VW] - c.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const Vec4F32 NegMulSub( const Vec4F32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_nmsub( a.m_V, b.m_V, c.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_nmsub( a.m_V, b.m_V, c.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vnmsubfp( a.m_V, b.m_V, c.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_ps( c.m_V, _mm_mul_ps( a.m_V, b.m_V ) );
    #else
    result.m_V[VX] = c.m_V[VX] - ( a.m_V[VX] * b.m_V[VX] );
    result.m_V[VY] = c.m_V[VY] - ( a.m_V[VY] * b.m_V[VY] );
    result.m_V[VZ] = c.m_V[VZ] - ( a.m_V[VZ] * b.m_V[VZ] );
    result.m_V[VW] = c.m_V[VW] - ( a.m_V[VW] * b.m_V[VW] );
    #endif
    return result;
}


/// Vec4F32 MulAdd : a * b + c : [ vector * scalar + vector ]
NOALIAS __forceinline const Vec4F32 MulAdd( const Vec4F32 VECREF a, const VF32 VECREF b, const Vec4F32 VECREF c )
{
    return MulAdd( a, MakeVec4F32( b ), c );
}

/// Vec4F32 MulSub : a * b - c : [ vector * scalar - vector ]
NOALIAS __forceinline const Vec4F32 MulSub( const Vec4F32 VECREF a, const VF32 VECREF b, const Vec4F32 VECREF c )
{
    return MulSub( a, MakeVec4F32( b ), c );
}

/// Vec4F32 NegMulAdd : -( a * b + c ) : [ - ( vector * scalar + vector ) ]
NOALIAS __forceinline const Vec4F32 NegMulAdd( const Vec4F32 VECREF a, const VF32 VECREF b, const Vec4F32 VECREF c )
{
    return NegMulAdd( a, MakeVec4F32( b ), c );
}

/// Vec4F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( vector * scalar - vector ) ]
NOALIAS __forceinline const Vec4F32 NegMulSub( const Vec4F32 VECREF a, const VF32 VECREF b, const Vec4F32 VECREF c )
{
    return NegMulSub( a, MakeVec4F32( b ), c );
}


/// Vec4F32 MulAdd : a * b + c : [ scalar * vector + vector ]
NOALIAS __forceinline const Vec4F32 MulAdd( const VF32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c )
{
    return MulAdd( MakeVec4F32( a ), b, c );
}

/// Vec4F32 MulSub : a * b - c : [ scalar * vector - vector ]
NOALIAS __forceinline const Vec4F32 MulSub( const VF32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c )
{
    return MulSub( MakeVec4F32( a ), b, c );
}

/// Vec4F32 NegMulAdd : -( a * b + c ) : [ - ( scalar * vector + vector ) ]
NOALIAS __forceinline const Vec4F32 NegMulAdd( const VF32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c )
{
    return NegMulAdd( MakeVec4F32( a ), b, c );
}

/// Vec4F32 NegMulSub : -( a * b - c ) == c - a * b : [ - ( scalar * vector - vector ) ]
NOALIAS __forceinline const Vec4F32 NegMulSub( const VF32 VECREF a, const Vec4F32 VECREF b, const Vec4F32 VECREF c )
{
    return NegMulSub( MakeVec4F32( a ), b, c );
}


/// Vec4F32 Lerp : src + ( ( dst - src ) * t )
NOALIAS __forceinline const Vec4F32 Lerp( const Vec4F32 VECREF src, const Vec4F32 VECREF dst, const VF32 VECREF t )
{
    return MulAdd( dst - src, t, src );
}



/// Vec4F32 Min
NOALIAS __forceinline const Vec4F32 Min( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_min( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_min( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vminfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_min_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = Maths::Min( x.m_V[VX], y.m_V[VX] );
    result.m_V[VY] = Maths::Min( x.m_V[VY], y.m_V[VY] );
    result.m_V[VZ] = Maths::Min( x.m_V[VZ], y.m_V[VZ] );
    result.m_V[VW] = Maths::Min( x.m_V[VW], y.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Max
NOALIAS __forceinline const Vec4F32 Max( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_max( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_max( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmaxfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_max_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = Maths::Max( x.m_V[VX], y.m_V[VX] );
    result.m_V[VY] = Maths::Max( x.m_V[VY], y.m_V[VY] );
    result.m_V[VZ] = Maths::Max( x.m_V[VZ], y.m_V[VZ] );
    result.m_V[VW] = Maths::Max( x.m_V[VW], y.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Min
NOALIAS __forceinline const Vec4F32 Min( const Vec4F32 VECREF x, const VF32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_min( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_min( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vminfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_min_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = Maths::Min( x.m_V[VX], y.m_V );
    result.m_V[VY] = Maths::Min( x.m_V[VY], y.m_V );
    result.m_V[VZ] = Maths::Min( x.m_V[VZ], y.m_V );
    result.m_V[VW] = Maths::Min( x.m_V[VW], y.m_V );
    #endif
    return result;
}

/// Vec4F32 Max
NOALIAS __forceinline const Vec4F32 Max( const Vec4F32 VECREF x, const VF32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_max( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_max( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmaxfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_max_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = Maths::Max( x.m_V[VX], y.m_V );
    result.m_V[VY] = Maths::Max( x.m_V[VY], y.m_V );
    result.m_V[VZ] = Maths::Max( x.m_V[VZ], y.m_V );
    result.m_V[VW] = Maths::Max( x.m_V[VW], y.m_V );
    #endif
    return result;
}

/// Vec4F32 Min Update
DEPRECATED NOALIAS __forceinline const Vec4F32& MinEq( Vec4F32* const __restrict pX, const Vec4F32 VECREF y )
{
    #if defined( _SPU_ )
    pX->m_V = spu_min( pX->m_V, y.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_min( pX->m_V, y.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vminfp( pX->m_V, y.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_min_ps( pX->m_V, y.m_V );
    #else
    pX->m_V[VX] = Maths::Min( pX->m_V[VX], y.m_V[VX] );
    pX->m_V[VY] = Maths::Min( pX->m_V[VY], y.m_V[VY] );
    pX->m_V[VZ] = Maths::Min( pX->m_V[VZ], y.m_V[VZ] );
    pX->m_V[VW] = Maths::Min( pX->m_V[VW], y.m_V[VW] );
    #endif
    return *pX;
}

/// Vec4F32 Max Update
DEPRECATED NOALIAS __forceinline const Vec4F32& MaxEq( Vec4F32* const __restrict pX, const Vec4F32 VECREF y )
{
    #if defined( _SPU_ )
    pX->m_V = spu_max( pX->m_V, y.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_max( pX->m_V, y.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vmaxfp( pX->m_V, y.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_max_ps( pX->m_V, y.m_V );
    #else
    pX->m_V[VX] = Maths::Max( pX->m_V[VX], y.m_V[VX] );
    pX->m_V[VY] = Maths::Max( pX->m_V[VY], y.m_V[VY] );
    pX->m_V[VZ] = Maths::Max( pX->m_V[VZ], y.m_V[VZ] );
    pX->m_V[VW] = Maths::Max( pX->m_V[VW], y.m_V[VW] );
    #endif
    return *pX;
}


/// Vec4F32 Min
NOALIAS __forceinline void Min( Vec4F32* const __restrict pX, const Vec4F32 VECREF y )
{
    #if defined( _SPU_ )
    pX->m_V = spu_min( pX->m_V, y.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_min( pX->m_V, y.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vminfp( pX->m_V, y.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_min_ps( pX->m_V, y.m_V );
    #else
    pX->m_V[VX] = Maths::Min( pX->m_V[VX], y.m_V[VX] );
    pX->m_V[VY] = Maths::Min( pX->m_V[VY], y.m_V[VY] );
    pX->m_V[VZ] = Maths::Min( pX->m_V[VZ], y.m_V[VZ] );
    pX->m_V[VW] = Maths::Min( pX->m_V[VW], y.m_V[VW] );
    #endif
}

/// Vec4F32 Max
NOALIAS __forceinline void Max( Vec4F32* const __restrict pX, const Vec4F32 VECREF y )
{
    #if defined( _SPU_ )
    pX->m_V = spu_max( pX->m_V, y.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_max( pX->m_V, y.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vmaxfp( pX->m_V, y.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_max_ps( pX->m_V, y.m_V );
    #else
    pX->m_V[VX] = Maths::Max( pX->m_V[VX], y.m_V[VX] );
    pX->m_V[VY] = Maths::Max( pX->m_V[VY], y.m_V[VY] );
    pX->m_V[VZ] = Maths::Max( pX->m_V[VZ], y.m_V[VZ] );
    pX->m_V[VW] = Maths::Max( pX->m_V[VW], y.m_V[VW] );
    #endif
}

/// Vec4F32 Min
NOALIAS __forceinline void Min( Vec4F32* const __restrict pX, const VF32 VECREF y )
{
    #if defined( _SPU_ )
    pX->m_V = spu_min( pX->m_V, y.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_min( pX->m_V, y.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vminfp( pX->m_V, y.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_min_ps( pX->m_V, y.m_V );
    #else
    pX->m_V[VX] = Maths::Min( pX->m_V[VX], y.m_V );
    pX->m_V[VY] = Maths::Min( pX->m_V[VY], y.m_V );
    pX->m_V[VZ] = Maths::Min( pX->m_V[VZ], y.m_V );
    pX->m_V[VW] = Maths::Min( pX->m_V[VW], y.m_V );
    #endif
}

/// Vec4F32 Max
NOALIAS __forceinline void Max( Vec4F32* const __restrict pX, const VF32 VECREF y )
{
    #if defined( _SPU_ )
    pX->m_V = spu_max( pX->m_V, y.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_max( pX->m_V, y.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vmaxfp( pX->m_V, y.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_max_ps( pX->m_V, y.m_V );
    #else
    pX->m_V[VX] = Maths::Max( pX->m_V[VX], y.m_V );
    pX->m_V[VY] = Maths::Max( pX->m_V[VY], y.m_V );
    pX->m_V[VZ] = Maths::Max( pX->m_V[VZ], y.m_V );
    pX->m_V[VW] = Maths::Max( pX->m_V[VW], y.m_V );
    #endif
}


/// Vec4F32 Clamp
NOALIAS __forceinline const Vec4F32 Clamp( const Vec4F32 VECREF x, const Vec4F32 VECREF lo, const Vec4F32 VECREF hi )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_min( spu_max( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_min( vec_vmaxfp( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vminfp( __vmaxfp( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_min_ps( _mm_max_ps( x.m_V, lo.m_V ), hi.m_V );
    #else
    result.m_V[VX] = Maths::Min( Maths::Max( x.m_V[VX], lo.m_V[VX] ), hi.m_V[VX] );
    result.m_V[VY] = Maths::Min( Maths::Max( x.m_V[VY], lo.m_V[VY] ), hi.m_V[VY] );
    result.m_V[VZ] = Maths::Min( Maths::Max( x.m_V[VZ], lo.m_V[VZ] ), hi.m_V[VZ] );
    result.m_V[VW] = Maths::Min( Maths::Max( x.m_V[VW], lo.m_V[VW] ), hi.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Clamp
NOALIAS __forceinline const Vec4F32 Clamp( const Vec4F32 VECREF x, const VF32 VECREF lo, const VF32 VECREF hi )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_min( spu_max( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_min( vec_vmaxfp( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vminfp( __vmaxfp( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_min_ps( _mm_max_ps( x.m_V, lo.m_V ), hi.m_V );
    #else
    result.m_V[VX] = Maths::Min( Maths::Max( x.m_V[VX], lo.m_V ), hi.m_V );
    result.m_V[VY] = Maths::Min( Maths::Max( x.m_V[VY], lo.m_V ), hi.m_V );
    result.m_V[VZ] = Maths::Min( Maths::Max( x.m_V[VZ], lo.m_V ), hi.m_V );
    result.m_V[VW] = Maths::Min( Maths::Max( x.m_V[VW], lo.m_V ), hi.m_V );
    #endif
    return result;
}

/// Vec4F32 Clamp Update
DEPRECATED NOALIAS __forceinline const Vec4F32& ClampEq( Vec4F32* const __restrict pX, const Vec4F32 VECREF lo, const Vec4F32 VECREF hi )
{
    #if defined( _SPU_ )
    pX->m_V = spu_min( spu_max( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_min( vec_vmaxfp( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vminfp( __vmaxfp( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_min_ps( _mm_max_ps( pX->m_V, lo.m_V ), hi.m_V );
    #else
    pX->m_V[VX] = Maths::Min( Maths::Max( pX->m_V[VX], lo.m_V[VX] ), hi.m_V[VX] );
    pX->m_V[VY] = Maths::Min( Maths::Max( pX->m_V[VY], lo.m_V[VY] ), hi.m_V[VY] );
    pX->m_V[VZ] = Maths::Min( Maths::Max( pX->m_V[VZ], lo.m_V[VZ] ), hi.m_V[VZ] );
    pX->m_V[VW] = Maths::Min( Maths::Max( pX->m_V[VW], lo.m_V[VW] ), hi.m_V[VW] );
    #endif
    return *pX;
}

/// Vec4F32 Clamp
NOALIAS __forceinline void Clamp( Vec4F32* const __restrict pX, const Vec4F32 VECREF lo, const Vec4F32 VECREF hi )
{
    #if defined( _SPU_ )
    pX->m_V = spu_min( spu_max( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_min( vec_vmaxfp( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vminfp( __vmaxfp( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_min_ps( _mm_max_ps( pX->m_V, lo.m_V ), hi.m_V );
    #else
    pX->m_V[VX] = Maths::Min( Maths::Max( pX->m_V[VX], lo.m_V[VX] ), hi.m_V[VX] );
    pX->m_V[VY] = Maths::Min( Maths::Max( pX->m_V[VY], lo.m_V[VY] ), hi.m_V[VY] );
    pX->m_V[VZ] = Maths::Min( Maths::Max( pX->m_V[VZ], lo.m_V[VZ] ), hi.m_V[VZ] );
    pX->m_V[VW] = Maths::Min( Maths::Max( pX->m_V[VW], lo.m_V[VW] ), hi.m_V[VW] );
    #endif
}

/// Vec4F32 Clamp
NOALIAS __forceinline void Clamp( Vec4F32* const __restrict pX, const VF32 VECREF lo, const VF32 VECREF hi )
{
    #if defined( _SPU_ )
    pX->m_V = spu_min( spu_max( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _PPU_ )
    pX->m_V = vec_min( vec_vmaxfp( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _VMX128_ )
    pX->m_V = __vminfp( __vmaxfp( pX->m_V, lo.m_V ), hi.m_V );
    #elif defined( _SSE_ )
    pX->m_V = _mm_min_ps( _mm_max_ps( pX->m_V, lo.m_V ), hi.m_V );
    #else
    pX->m_V[VX] = Maths::Min( Maths::Max( pX->m_V[VX], lo.m_V ), hi.m_V );
    pX->m_V[VY] = Maths::Min( Maths::Max( pX->m_V[VY], lo.m_V ), hi.m_V );
    pX->m_V[VZ] = Maths::Min( Maths::Max( pX->m_V[VZ], lo.m_V ), hi.m_V );
    pX->m_V[VW] = Maths::Min( Maths::Max( pX->m_V[VW], lo.m_V ), hi.m_V );
    #endif
}



/// Vec4F32 Round to integer towards minus infinity
NOALIAS __forceinline const Vec4F32 Floor( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rfim( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vrfim( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrfim( x.m_V );
    #elif defined( _SSE_ )
    result.m_V = SSE_RoundToIntMinusInf( x.m_V );
    #else
    result.m_V[VX] = floorf( x.m_V[VX] );
    result.m_V[VY] = floorf( x.m_V[VY] );
    result.m_V[VZ] = floorf( x.m_V[VZ] );
    result.m_V[VW] = floorf( x.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Round to integer towards plus infinity
NOALIAS __forceinline const Vec4F32 Ceil ( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rfip( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vrfip( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrfip( x.m_V );
    #elif defined( _SSE_ )
    result.m_V = SSE_RoundToIntPlusInf( x.m_V );
    #else
    result.m_V[VX] = ceilf( x.m_V[VX] );
    result.m_V[VY] = ceilf( x.m_V[VY] );
    result.m_V[VZ] = ceilf( x.m_V[VZ] );
    result.m_V[VW] = ceilf( x.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Round to integer towards nearest
NOALIAS __forceinline const Vec4F32 Round( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rfin( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vrfin( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrfin( x.m_V );
    #elif defined( _SSE_ )
    result.m_V = SSE_RoundToIntNearest( x.m_V );
    #else
    result.m_V[VX] = (F32)_copysign( floorf( fabsf( x.m_V[VX] ) + 0.5f ), x.m_V[VX] );
    result.m_V[VY] = (F32)_copysign( floorf( fabsf( x.m_V[VY] ) + 0.5f ), x.m_V[VY] );
    result.m_V[VZ] = (F32)_copysign( floorf( fabsf( x.m_V[VZ] ) + 0.5f ), x.m_V[VZ] );
    result.m_V[VW] = (F32)_copysign( floorf( fabsf( x.m_V[VW] ) + 0.5f ), x.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Round to integer towards zero
NOALIAS __forceinline const Vec4F32 Trunc( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rfiz( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vrfiz( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrfiz( x.m_V );
    #elif defined( _SSE_ )
    result.m_V = SSE_RoundToIntZero( x.m_V );
    #else
    result.m_V[VX] = (F32)(S64)x.m_V[VX];
    result.m_V[VY] = (F32)(S64)x.m_V[VY];
    result.m_V[VZ] = (F32)(S64)x.m_V[VZ];
    result.m_V[VW] = (F32)(S64)x.m_V[VW];
    #endif
    return result;
}


/// Vec4F32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const Vec4F32 Mod( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    return NegMulSub( Trunc( x / y ), y, x ); // x - Trunc( x / y ) * y
}

/// Vec4F32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const Vec4F32 Mod( const Vec4F32 VECREF x, const Vec4F32 VECREF y, const Vec4F32 VECREF rcp_y )
{
    return NegMulSub( Trunc( x * rcp_y ), y, x );
}

/// Vec4F32 Fractional Part : Returns fractional part of x, f = x - i [ x = i + f ], where i is an integer, i & f have the same sign as x
NOALIAS __forceinline const Vec4F32 Frac( const Vec4F32 VECREF x )
{
    return x - Trunc( x );
}

/// Vec4F32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const Vec4F32 WrapToInterval( const Vec4F32 VECREF x, const Vec4F32 VECREF p )
{
    return NegMulSub( Floor( x / p ), p, x ); // x - ( Floor( x / p ) * p )
}

/// Vec4F32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const Vec4F32 WrapToInterval( const Vec4F32 VECREF x, const Vec4F32 VECREF p, const Vec4F32 VECREF rcp_p )
{
    return NegMulSub( Floor( x * rcp_p ), p, x ); // x - ( Floor( x * rcp_p ) * p )
}

/// Vec4F32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const Vec4F32 WrapToIntervalOffset( const Vec4F32 VECREF x, const Vec4F32 VECREF p, const Vec4F32 VECREF o )
{
    return NegMulSub( Floor( ( x + o ) / p ), p, x ); // x - ( Floor( ( x + o ) / p ) * p )
}

/// Vec4F32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const Vec4F32 WrapToIntervalOffset( const Vec4F32 VECREF x, const Vec4F32 VECREF p, const Vec4F32 VECREF rcp_p, const Vec4F32 VECREF o )
{
    return NegMulSub( Floor( ( x + o ) * rcp_p ), p, x ); // x - ( Floor( ( x + o ) * rcp_p ) * p )
}


/// Vec4F32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const Vec4F32 Mod( const Vec4F32 VECREF x, const VF32 VECREF y )
{
    const Vec4F32 vy( MakeVec4F32( y ) );
    return NegMulSub( Trunc( x / vy ), vy, x ); // x - Trunc( x / y ) * y
}

/// Vec4F32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
/// NOTE: You MUST ensure that y * rcp_y = 1!
NOALIAS __forceinline const Vec4F32 Mod( const Vec4F32 VECREF x, const VF32 VECREF y, const VF32 VECREF rcp_y )
{
    return NegMulSub( Trunc( x * MakeVec4F32( rcp_y ) ), MakeVec4F32( y ), x );
}

/// Vec4F32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const Vec4F32 WrapToInterval( const Vec4F32 VECREF x, const VF32 VECREF p )
{
    const Vec4F32 vp( MakeVec4F32( p ) );
    return NegMulSub( Floor( x / vp ), vp, x ); // x - ( Floor( x / p ) * p )
}

/// Vec4F32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const Vec4F32 WrapToInterval( const Vec4F32 VECREF x, const VF32 VECREF p, const VF32 VECREF rcp_p )
{
    return NegMulSub( Floor( x * MakeVec4F32( rcp_p ) ), MakeVec4F32( p ), x ); // x - ( Floor( x * rcp_p ) * p )
}

/// Vec4F32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const Vec4F32 WrapToIntervalOffset( const Vec4F32 VECREF x, const VF32 VECREF p, const VF32 VECREF o )
{
    const Vec4F32 vp( MakeVec4F32( p ) );
    return NegMulSub( Floor( ( x + MakeVec4F32( o ) ) / vp ), vp, x ); // x - ( Floor( ( x + o ) / p ) * p )
}

/// Vec4F32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const Vec4F32 WrapToIntervalOffset( const Vec4F32 VECREF x, const VF32 VECREF p, const VF32 VECREF rcp_p, const VF32 VECREF o )
{
    return NegMulSub( Floor( ( x + MakeVec4F32( o ) ) * MakeVec4F32( rcp_p ) ), MakeVec4F32( p ), x ); // x - ( Floor( ( x + o ) * rcp_p ) * p )
}




/// Vec4F32 Comparison a == b : Returns VBool representing results of a.x == b.x && a.y == b.y && a.z == b.z && a.w == b.w
NOALIAS __forceinline const VBool operator == ( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    VBool result;
    #if defined( _SPU_ )
    //const vector unsigned int eq( spu_cmpeq( a.m_V, b.m_V ) );
    //result.m_V = spu_and( spu_and( spu_splat<VX>( eq ), spu_splat<VY>( eq ) ),
    //                      spu_and( spu_splat<VZ>( eq ), spu_splat<VW>( eq ) ) );
    result.m_V = spu_horizontal_and( spu_cmpeq( a.m_V, b.m_V ) );
    #elif defined( _PPU_ )
    //const vector bool int eq( vec_cmpeq( a.m_V, b.m_V ) );
    //result.m_V = vec_and( vec_and( vec_splat( eq, VX ), vec_splat( eq, VY ) ),
    //                      vec_and( vec_splat( eq, VZ ), vec_splat( eq, VW ) ) );
    result.m_V = vec_horizontal_and( vec_cmpeq( a.m_V, b.m_V ) );
    #elif defined( _VMX128_ )
    //const __vector4 eq( __vcmpeqfp( a.m_V, b.m_V ) );
    //result.m_V = __vand( __vand( __vspltw( eq, VX ), __vspltw( eq, VY ) ),
    //                     __vand( __vspltw( eq, VZ ), __vspltw( eq, VW ) ) );
    result.m_V = vmx_horizontal_and_int32( __vcmpeqfp( a.m_V, b.m_V ) );
    #elif defined( _SSE_ )
    //const __m128 eq( _mm_cmpeq_ps( a.m_V, b.m_V ) );
    //result.m_V = _mm_and_ps( _mm_and_ps( _mm_shuffle_ps( eq, _MM_SHUFFLE( VX, VX, VX, VX ) ),
    //                                     _mm_shuffle_ps( eq, _MM_SHUFFLE( VY, VY, VY, VY ) ) ),
    //                         _mm_and_ps( _mm_shuffle_ps( eq, _MM_SHUFFLE( VZ, VZ, VZ, VZ ) ),
    //                                     _mm_shuffle_ps( eq, _MM_SHUFFLE( VW, VW, VW, VW ) ) ) );
    result.m_V = sse_horizontal_and_int32( _mm_cmpeq_ps( a.m_V, b.m_V ) );
    #else
    result.m_V = a.m_V[VX] == b.m_V[VX] &&
                 a.m_V[VY] == b.m_V[VY] &&
                 a.m_V[VZ] == b.m_V[VZ] &&
                 a.m_V[VW] == b.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Comparison a != b : Returns VBool representing results of a.x != b.x || a.y != b.y || a.z != b.z || a.w != b.w
NOALIAS __forceinline const VBool operator != ( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    VBool result;
    #if defined( _SPU_ )
    //const vector unsigned int ne( spu_not( spu_cmpeq( a.m_V, b.m_V ) ) );
    //result.m_V = spu_or( spu_or( spu_splat<VX>( ne ), spu_splat<VY>( ne ) ),
    //                     spu_or( spu_splat<VZ>( ne ), spu_splat<VW>( ne ) ) );
    result.m_V = spu_horizontal_or( spu_not( spu_cmpeq( a.m_V, b.m_V ) ) );
    #elif defined( _PPU_ )
    //const vector bool int ne( vec_not( vec_cmpeq( a.m_V, b.m_V ) ) );
    //result.m_V = vec_or( vec_or( vec_splat( ne, VX ), vec_splat( ne, VY ) ),
    //                     vec_or( vec_splat( ne, VZ ), vec_splat( ne, VW ) ) );
    result.m_V = vec_horizontal_or( vec_not( vec_cmpeq( a.m_V, b.m_V ) ) );
    #elif defined( _VMX128_ )
    const __vector4 eq( __vcmpeqfp( a.m_V, b.m_V ) );
    //const __vector4 ne( __vnor( eq, eq ) );
    //result.m_V = __vor( __vor( __vspltw( ne, VX ), __vspltw( ne, VY ) ),
    //                    __vor( __vspltw( ne, VZ ), __vspltw( ne, VW ) ) );
    result.m_V = vmx_horizontal_or_int32( __vnor( eq, eq ) );
    #elif defined( _SSE_ )
    //const __m128 ne( _mm_cmpneq_ps( a.m_V, b.m_V ) );
    //result.m_V = _mm_or_ps( _mm_or_ps( _mm_shuffle_ps( ne, _MM_SHUFFLE( VX, VX, VX, VX ) ),
    //                                   _mm_shuffle_ps( ne, _MM_SHUFFLE( VY, VY, VY, VY ) ) ),
    //                        _mm_or_ps( _mm_shuffle_ps( ne, _MM_SHUFFLE( VZ, VZ, VZ, VZ ) ),
    //                                   _mm_shuffle_ps( ne, _MM_SHUFFLE( VW, VW, VW, VW ) ) ) );
    result.m_V = sse_horizontal_or_int32( _mm_cmpneq_ps( a.m_V, b.m_V ) );
    #else
    result.m_V = a.m_V[VX] != b.m_V[VX] ||
                 a.m_V[VY] != b.m_V[VY] ||
                 a.m_V[VZ] != b.m_V[VZ] ||
                 a.m_V[VW] != b.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Comparison a <= b : Returns Vec4Bool representing results of a.i <= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator <= ( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpge( b.m_V, a.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpge( b.m_V, a.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpgefp( b.m_V, a.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmple_ps( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] <= b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] <= b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] <= b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] <= b.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Comparison a >= b : Returns Vec4Bool representing results of a.i >= b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator >= ( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpge( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpge( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpgefp( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpge_ps( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] >= b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] >= b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] >= b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] >= b.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Comparison a < b : Returns Vec4Bool representing results of a.i < b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator < ( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpgt( b.m_V, a.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpgt( b.m_V, a.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpgtfp( b.m_V, a.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmplt_ps( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] < b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] < b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] < b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] < b.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Comparison a > b : Returns Vec4Bool representing results of a.i > b.i, for each element i [x,y,z,w]
NOALIAS __forceinline const Vec4Bool operator > ( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpgt( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpgt( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpgtfp( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpgt_ps( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] > b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] > b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] > b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] > b.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 Ordered Test : true if neither a nor b contains any elements that are NaN's
NOALIAS __forceinline const VBool Ordered( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::True(); // SPU doesn't support vector float NaN's
    #elif defined( _PPU_ )
    const vector bool int ord( vec_or( vec_cmpge( a.m_V, b.m_V ),
                                       vec_cmpge( b.m_V, a.m_V ) ) );
    //result.m_V = vec_and( vec_and( vec_splat( ord, VX ), vec_splat( ord, VY ) ),
    //                      vec_and( vec_splat( ord, VZ ), vec_splat( ord, VW ) ) );
    result.m_V = vec_horizontal_and( ord );
    #elif defined( _VMX128_ )
    const __vector4 ord( __vor( __vcmpgefp( a.m_V, b.m_V ),
                                __vcmpgefp( b.m_V, a.m_V ) ) );
    //result.m_V = __vand( __vand( __vspltw( ord, VX ), __vspltw( ord, VY ) ),
    //                     __vand( __vspltw( ord, VZ ), __vspltw( ord, VW ) ) );
    result.m_V = vmx_horizontal_and_int32( ord );
    #elif defined( _SSE_ )
    //const __m128 ord( _mm_cmpord_ps( a.m_V, b.m_V ) );
    //result.m_V = _mm_and_ps( _mm_and_ps( sse_splat<VX>( ord ), sse_splat<VY>( ord ) ),
    //                         _mm_and_ps( sse_splat<VZ>( ord ), sse_splat<VW>( ord ) ) );
    result.m_V = sse_horizontal_and_int32( _mm_cmpord_ps( a.m_V, b.m_V ) );
    #else
    result.m_V = !_isnan( a.m_V[VX] ) && !_isnan( b.m_V[VX] ) &&
                 !_isnan( a.m_V[VY] ) && !_isnan( b.m_V[VY] ) &&
                 !_isnan( a.m_V[VZ] ) && !_isnan( b.m_V[VZ] ) &&
                 !_isnan( a.m_V[VW] ) && !_isnan( b.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Unordered Test : true if a or b contains any elements that are NaN's
NOALIAS __forceinline const VBool Unordered( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::False(); // SPU doesn't support vector float NaN's
    #elif defined( _PPU_ )
    const vector bool int unord( vec_nor( vec_cmpge( a.m_V, b.m_V ),
                                          vec_cmpge( b.m_V, a.m_V ) ) );
    //result.m_V = vec_or( vec_or( vec_splat( unord, VX ), vec_splat( unord, VY ) ),
    //                     vec_or( vec_splat( unord, VZ ), vec_splat( unord, VW ) ) );
    result.m_V = vec_horizontal_or( unord );
    #elif defined( _VMX128_ )
    const __vector4 unord( __vnor( __vcmpgefp( a.m_V, b.m_V ),
                                   __vcmpgefp( b.m_V, a.m_V ) ) );
    //result.m_V = __vor( __vor( __vspltw( unord, VX ), __vspltw( unord, VY ) ),
    //                    __vor( __vspltw( unord, VZ ), __vspltw( unord, VW ) ) );
    result.m_V = vmx_horizontal_or_int32( unord );
    #elif defined( _SSE_ )
    //const __m128 unord( _mm_cmpunord_ps( a.m_V, b.m_V ) );
    //result.m_V = _mm_or_ps( _mm_or_ps( sse_splat<VX>( unord ), sse_splat<VY>( unord ) ),
    //                        _mm_or_ps( sse_splat<VZ>( unord ), sse_splat<VW>( unord ) ) );
    result.m_V = sse_horizontal_or_int32( _mm_cmpunord_ps( a.m_V, b.m_V ) );
    #else
    result.m_V = _isnan( a.m_V[VX] ) || _isnan( b.m_V[VX] ) ||
                 _isnan( a.m_V[VY] ) || _isnan( b.m_V[VY] ) ||
                 _isnan( a.m_V[VZ] ) || _isnan( b.m_V[VZ] ) ||
                 _isnan( a.m_V[VW] ) || _isnan( b.m_V[VW] );
    #endif
    return result;
}


/// Vec4F32 NaN Test : true if a contains any element that is a NaN
NOALIAS __forceinline const VBool IsNan( const Vec4F32 VECREF a )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::False(); // SPU doesn't support vector float NaN's
    #elif defined( _PPU_ )
    //const vector bool int isnan( vec_not( vec_cmpeq( a.m_V, a.m_V ) ) );
    //result.m_V = vec_or( vec_or( vec_splat( isnan, VX ), vec_splat( isnan, VY ) ),
    //                     vec_or( vec_splat( isnan, VZ ), vec_splat( isnan, VW ) ) );
    result.m_V = vec_horizontal_or( vec_not( vec_cmpeq( a.m_V, a.m_V ) ) );
    #elif defined( _VMX128_ )
    const __vector4 eq( __vcmpeqfp( a.m_V, a.m_V ) );
    //const __vector4 isnan( __vnor( eq, eq ) );
    //result.m_V = __vor( __vor( __vspltw( isnan, VX ), __vspltw( isnan, VY ) ),
    //                    __vor( __vspltw( isnan, VZ ), __vspltw( isnan, VW ) ) );
    result.m_V = vmx_horizontal_or_int32( __vnor( eq, eq ) );
    #elif defined( _SSE_ )
    //const __m128 isnan( _mm_cmpunord_ps( a.m_V, a.m_V ) );
    //result.m_V = _mm_or_ps( _mm_or_ps( sse_splat<VX>( isnan ), sse_splat<VY>( isnan ) ),
    //                        _mm_or_ps( sse_splat<VZ>( isnan ), sse_splat<VW>( isnan ) ) );
    result.m_V = sse_horizontal_or_int32( _mm_cmpunord_ps( a.m_V, a.m_V ) );
    #else
    result.m_V = _isnan( a.m_V[VX] ) ||
                 _isnan( a.m_V[VY] ) ||
                 _isnan( a.m_V[VZ] ) ||
                 _isnan( a.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Finite Test : true if a contains no elements that are NaN's and for each element i [x,y,z,w], abs(x.i) <= FLT_MAX
NOALIAS __forceinline const VBool Finite( const Vec4F32 VECREF a )
{
    VBool result;
    #if defined( _SPU_ )
    const vector unsigned int finite( spu_not( spu_cmpabsgt( a.m_V, Const< vector float >::FltMax() ) ) ); // SPU doesn't support vector float NaN's
    //result.m_V = spu_and( spu_and( spu_splat<VX>( finite ), spu_splat<VY>( finite ) ),
    //                      spu_and( spu_splat<VZ>( finite ), spu_splat<VW>( finite ) ) );
    result.m_V = spu_horizontal_and( finite );
    #elif defined( _PPU_ )
    const vector bool int finite( vec_and( vec_cmpeq( a.m_V, a.m_V ),
                                           vec_cmpge( Const< vector float >::FltMax(), vec_abs( a.m_V ) ) ) );
    //result.m_V = vec_and( vec_and( vec_splat( finite, VX ), vec_splat( finite, VY ) ),
    //                      vec_and( vec_splat( finite, VZ ), vec_splat( finite, VW ) ) );
    result.m_V = vec_horizontal_and( finite );
    #elif defined( _VMX128_ )
    const __vector4 finite( __vand( __vcmpeqfp( a.m_V, a.m_V ),
                                    __vcmpgefp( Const< __vector4 >::FltMax(), __vand( a.m_V, Const< __vector4 >::AbsMask() ) ) ) );
    //result.m_V = __vand( __vand( __vspltw( finite, VX ), __vspltw( finite, VY ) ),
    //                     __vand( __vspltw( finite, VZ ), __vspltw( finite, VW ) ) );
    result.m_V = vmx_horizontal_and_int32( finite );
    #elif defined( _SSE_ )
    const __m128 finite( _mm_and_ps( _mm_cmpord_ps( a.m_V, a.m_V ),
                                     _mm_cmpge_ps( Const< __m128 >::FltMax(), sse_abs( a.m_V ) ) ) );
    //result.m_V = _mm_and_ps( _mm_and_ps( sse_splat<VX>( finite ), sse_splat<VY>( finite ) ),
    //                         _mm_and_ps( sse_splat<VZ>( finite ), sse_splat<VW>( finite ) ) );
    result.m_V = sse_horizontal_and_int32( finite );
    #else
    result.m_V = !_isnan( a.m_V[VX] ) && ( Maths::Abs( a.m_V[VX] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[VY] ) && ( Maths::Abs( a.m_V[VY] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[VZ] ) && ( Maths::Abs( a.m_V[VZ] ) <= FLT_MAX ) &&
                 !_isnan( a.m_V[VW] ) && ( Maths::Abs( a.m_V[VW] ) <= FLT_MAX );
    #endif
    return result;
}



/// Vec4F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec4F32 Select( const VBool PCREF condition, const Vec4F32 VECREF result_if_true, const Vec4F32 VECREF result_if_false )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_ps( _mm_and_ps( condition.m_V, result_if_true.m_V ), _mm_andnot_ps( condition.m_V, result_if_false.m_V ) );
    #else
    result.m_V[VX] = ( condition.m_V ? result_if_true.m_V[VX] : result_if_false.m_V[VX] );
    result.m_V[VY] = ( condition.m_V ? result_if_true.m_V[VY] : result_if_false.m_V[VY] );
    result.m_V[VZ] = ( condition.m_V ? result_if_true.m_V[VZ] : result_if_false.m_V[VZ] );
    result.m_V[VW] = ( condition.m_V ? result_if_true.m_V[VW] : result_if_false.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const Vec4F32 Select( const Vec4Bool PCREF condition, const Vec4F32 VECREF result_if_true, const Vec4F32 VECREF result_if_false )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_ps( _mm_and_ps( condition.m_V, result_if_true.m_V ), _mm_andnot_ps( condition.m_V, result_if_false.m_V ) );
    #else
    result.m_V[VX] = ( condition.m_V[VX] ? result_if_true.m_V[VX] : result_if_false.m_V[VX] );
    result.m_V[VY] = ( condition.m_V[VY] ? result_if_true.m_V[VY] : result_if_false.m_V[VY] );
    result.m_V[VZ] = ( condition.m_V[VZ] ? result_if_true.m_V[VZ] : result_if_false.m_V[VZ] );
    result.m_V[VW] = ( condition.m_V[VW] ? result_if_true.m_V[VW] : result_if_false.m_V[VW] );
    #endif
    return result;
}


/// Vec4F32 Bitwise NOT: ~a
NOALIAS __forceinline const Vec4F32 NOT( const Vec4F32 VECREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_not( a.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_not( a.m_V );
    #elif defined( _VMX128_ )                    
    //result.m_V = __vnor( a.m_V, a.m_V );
    result.m_V = vmx_not( a.m_V );
    #elif defined( _SSE_ )                      
    //result.m_V = _mm_xor_ps( a.m_V, Const< __m128 >::Mask<1,1,1,1>() );
    result.m_V = sse_not( a.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( ~TypePun_FloatToInt( a.m_V[VX] ) );
    result.m_V[VY] = TypePun_IntToFloat( ~TypePun_FloatToInt( a.m_V[VY] ) );
    result.m_V[VZ] = TypePun_IntToFloat( ~TypePun_FloatToInt( a.m_V[VZ] ) );
    result.m_V[VW] = TypePun_IntToFloat( ~TypePun_FloatToInt( a.m_V[VW] ) );
    #endif
    return result;
}

/// Vec4F32 Bitwise OR: a | b
NOALIAS __forceinline const Vec4F32 OR( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_or( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_or( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_or( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_or( a.m_V, b.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VX] ) | TypePun_FloatToInt( b.m_V[VX] ) );
    result.m_V[VY] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VY] ) | TypePun_FloatToInt( b.m_V[VY] ) );
    result.m_V[VZ] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VZ] ) | TypePun_FloatToInt( b.m_V[VZ] ) );
    result.m_V[VW] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VW] ) | TypePun_FloatToInt( b.m_V[VW] ) );
    #endif
    return result;
}

/// Vec4F32 Bitwise AND: a & b
NOALIAS __forceinline const Vec4F32 AND( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_and( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_and( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_and( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_and( a.m_V, b.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VX] ) & TypePun_FloatToInt( b.m_V[VX] ) );
    result.m_V[VY] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VY] ) & TypePun_FloatToInt( b.m_V[VY] ) );
    result.m_V[VZ] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VZ] ) & TypePun_FloatToInt( b.m_V[VZ] ) );
    result.m_V[VW] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VW] ) & TypePun_FloatToInt( b.m_V[VW] ) );
    #endif
    return result;
}

/// Vec4F32 Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const Vec4F32 XOR( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_xor( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_xor( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_xor( a.m_V, b.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VX] ) ^ TypePun_FloatToInt( b.m_V[VX] ) );
    result.m_V[VY] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VY] ) ^ TypePun_FloatToInt( b.m_V[VY] ) );
    result.m_V[VZ] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VZ] ) ^ TypePun_FloatToInt( b.m_V[VZ] ) );
    result.m_V[VW] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VW] ) ^ TypePun_FloatToInt( b.m_V[VW] ) );
    #endif
    return result;
}

/// Vec4F32 Bitwise NOR: ~(a | b)
NOALIAS __forceinline const Vec4F32 NOR( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_nor( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_nor( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_nor( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_nor( a.m_V, b.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VX] ) | TypePun_FloatToInt( b.m_V[VX] ) ) );
    result.m_V[VY] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VY] ) | TypePun_FloatToInt( b.m_V[VY] ) ) );
    result.m_V[VZ] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VZ] ) | TypePun_FloatToInt( b.m_V[VZ] ) ) );
    result.m_V[VW] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VW] ) | TypePun_FloatToInt( b.m_V[VW] ) ) );
    #endif
    return result;
}

/// Vec4F32 Bitwise NAND: ~(a & b)
NOALIAS __forceinline const Vec4F32 NAND( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_nand( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_nand( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_nand( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_nand( a.m_V, b.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VX] ) & TypePun_FloatToInt( b.m_V[VX] ) ) );
    result.m_V[VY] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VY] ) & TypePun_FloatToInt( b.m_V[VY] ) ) );
    result.m_V[VZ] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VZ] ) & TypePun_FloatToInt( b.m_V[VZ] ) ) );
    result.m_V[VW] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VW] ) & TypePun_FloatToInt( b.m_V[VW] ) ) );
    #endif
    return result;
}

/// Vec4F32 Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const Vec4F32 EQV( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_eqv( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_eqv( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_eqv( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_eqv( a.m_V, b.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VX] ) ^ TypePun_FloatToInt( b.m_V[VX] ) ) );
    result.m_V[VY] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VY] ) ^ TypePun_FloatToInt( b.m_V[VY] ) ) );
    result.m_V[VZ] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VZ] ) ^ TypePun_FloatToInt( b.m_V[VZ] ) ) );
    result.m_V[VW] = TypePun_IntToFloat( ~( TypePun_FloatToInt( a.m_V[VW] ) ^ TypePun_FloatToInt( b.m_V[VW] ) ) );
    #endif
    return result;
}

/// Vec4F32 Bitwise ORC: a | ~b
NOALIAS __forceinline const Vec4F32 ORC( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_orc( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_orc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_orc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_orc( a.m_V, b.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VX] ) | ~TypePun_FloatToInt( b.m_V[VX] ) );
    result.m_V[VY] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VY] ) | ~TypePun_FloatToInt( b.m_V[VY] ) );
    result.m_V[VZ] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VZ] ) | ~TypePun_FloatToInt( b.m_V[VZ] ) );
    result.m_V[VW] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VW] ) | ~TypePun_FloatToInt( b.m_V[VW] ) );
    #endif
    return result;
}

/// Vec4F32 Bitwise ANDC: a & ~b
NOALIAS __forceinline const Vec4F32 ANDC( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_andc( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_andc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_andc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_andc( a.m_V, b.m_V );
    #else
    result.m_V[VX] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VX] ) & ~TypePun_FloatToInt( b.m_V[VX] ) );
    result.m_V[VY] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VY] ) & ~TypePun_FloatToInt( b.m_V[VY] ) );
    result.m_V[VZ] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VZ] ) & ~TypePun_FloatToInt( b.m_V[VZ] ) );
    result.m_V[VW] = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V[VW] ) & ~TypePun_FloatToInt( b.m_V[VW] ) );
    #endif
    return result;
}


/// Vec4F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec4F32 AND( const Vec4F32 VECREF a, const VBool PCREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_and( a.m_V, (vector float)b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_and( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_and( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_and( a.m_V, b.m_V );
    #else
    result.m_V[VX] = b.m_V ? a.m_V[VX] : 0.0f;
    result.m_V[VY] = b.m_V ? a.m_V[VY] : 0.0f;
    result.m_V[VZ] = b.m_V ? a.m_V[VZ] : 0.0f;
    result.m_V[VW] = b.m_V ? a.m_V[VW] : 0.0f;
    #endif
    return result;
}

/// Vec4F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec4F32 ANDC( const Vec4F32 VECREF a, const VBool PCREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_andc( a.m_V, (vector float)b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_andc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_andc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_andc( a.m_V, b.m_V );
    #else
    result.m_V[VX] = !b.m_V ? a.m_V[VX] : 0.0f;
    result.m_V[VY] = !b.m_V ? a.m_V[VY] : 0.0f;
    result.m_V[VZ] = !b.m_V ? a.m_V[VZ] : 0.0f;
    result.m_V[VW] = !b.m_V ? a.m_V[VW] : 0.0f;
    #endif
    return result;
}


/// Vec4F32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const Vec4F32 AND( const Vec4F32 VECREF a, const Vec4Bool PCREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_and( a.m_V, (vector float)b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_and( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_and( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_and( a.m_V, b.m_V );
    #else
    result.m_V[VX] = b.m_V[VX] ? a.m_V[VX] : 0.0f;
    result.m_V[VY] = b.m_V[VY] ? a.m_V[VY] : 0.0f;
    result.m_V[VZ] = b.m_V[VZ] ? a.m_V[VZ] : 0.0f;
    result.m_V[VW] = b.m_V[VW] ? a.m_V[VW] : 0.0f;
    #endif
    return result;
}

/// Vec4F32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const Vec4F32 ANDC( const Vec4F32 VECREF a, const Vec4Bool PCREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_andc( a.m_V, (vector float)b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_andc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_andc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_andc( a.m_V, b.m_V );
    #else
    result.m_V[VX] = !b.m_V[VX] ? a.m_V[VX] : 0.0f;
    result.m_V[VY] = !b.m_V[VY] ? a.m_V[VY] : 0.0f;
    result.m_V[VZ] = !b.m_V[VZ] ? a.m_V[VZ] : 0.0f;
    result.m_V[VW] = !b.m_V[VW] ? a.m_V[VW] : 0.0f;
    #endif
    return result;
}


/// Vec4F32 2 element dot product
NOALIAS __forceinline const VF32 Dot2( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_dot_product2( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_dot_product2( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vmsum3fp( __vrlimi( a.m_V, __vzero(), VMX_MASK_INSERT_ZW, 0 ),
    //                         __vrlimi( b.m_V, __vzero(), VMX_MASK_INSERT_ZW, 0 ) );
    result.m_V = vmx_dot_product2( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_dot_product2( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V[VX] * b.m_V[VX] +
                 a.m_V[VY] * b.m_V[VY];
    #endif
    return result;
}

/// Vec4F32 3 element dot product
NOALIAS __forceinline const VF32 Dot3( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_dot_product3( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_dot_product3( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vmsum3fp( a.m_V, b.m_V );
    result.m_V = vmx_dot_product3( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_dot_product3( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V[VX] * b.m_V[VX] +
                 a.m_V[VY] * b.m_V[VY] +
                 a.m_V[VZ] * b.m_V[VZ];
    #endif
    return result;
}

/// Vec4F32 4 element dot product
NOALIAS __forceinline const VF32 Dot4( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_dot_product4( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_dot_product4( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vmsum4fp( a.m_V, b.m_V );
    result.m_V = vmx_dot_product4( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_dot_product4( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V[VX] * b.m_V[VX] +
                 a.m_V[VY] * b.m_V[VY] +
                 a.m_V[VZ] * b.m_V[VZ] +
                 a.m_V[VW] * b.m_V[VW];
    #endif
    return result;
}

/// Vec4F32 2 element cross product
NOALIAS __forceinline const VF32 Cross2( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_cross_product2( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cross_product2( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vnmsubfp( __vspltw( a.m_V, VY ), __vspltw( b.m_V, VX ),
    //               __vmulfp( __vspltw( a.m_V, VX ), __vspltw( b.m_V, VY ) ) );
    result.m_V = vmx_cross_product2( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_cross_product2( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V[VX] * b.m_V[VY] - a.m_V[VY] * b.m_V[VX];
    #endif
    return result;
}

/// Vec4F32 3 element cross product. W element of result is UNDEFINED!
NOALIAS __forceinline const Vec4F32 Cross3( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_cross_product3( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cross_product3( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vnmsubfp( __vpermwi( a.m_V, VPERMWI_CONST(VZ,VX,VY,VW) ), __vpermwi( b.m_V, VPERMWI_CONST(VY,VZ,VX,VW) ),
    //               __vmulfp( __vpermwi( a.m_V, VPERMWI_CONST(VY,VZ,VX,VW) ), __vpermwi( b.m_V, VPERMWI_CONST(VZ,VX,VY,VW) ) ) );
    result.m_V = vmx_cross_product3( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_cross_product3( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VY] * b.m_V[VZ] - a.m_V[VZ] * b.m_V[VY];
    result.m_V[VY] = a.m_V[VZ] * b.m_V[VX] - a.m_V[VX] * b.m_V[VZ];
    result.m_V[VZ] = a.m_V[VX] * b.m_V[VY] - a.m_V[VY] * b.m_V[VX];
    //result.m_V[VW] = 0.0f;
    #endif
    return result;
}

/// Vec4F32 3 element dot product operator [ More commonly used than Dot4 and matches the Cross3 operator ]
NOALIAS __forceinline const VF32 operator | ( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    /// @par WARNING: Operator precedence of '|' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_dot_product3( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_dot_product3( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vmsum3fp( a.m_V, b.m_V );
    result.m_V = vmx_dot_product3( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_dot_product3( a.m_V, b.m_V );
    #else
    result.m_V = a.m_V[VX] * b.m_V[VX] +
                 a.m_V[VY] * b.m_V[VY] +
                 a.m_V[VZ] * b.m_V[VZ];
    #endif
    return result;
}

/// Vec4F32 3 element cross product operator. W element of result is UNDEFINED!
NOALIAS __forceinline const Vec4F32 operator ^ ( const Vec4F32 VECREF a, const Vec4F32 VECREF b )
{
    /// @par WARNING: Operator precedence of '^' is lower than the precedence of '+' & '-'!
    /// @par NOTE:    Operator precedence of cross product '^' is higher than that of dot product '|'
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_cross_product3( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cross_product3( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    //result.m_V = __vnmsubfp( __vpermwi( a.m_V, VPERMWI_CONST(VZ,VX,VY,VW) ), __vpermwi( b.m_V, VPERMWI_CONST(VY,VZ,VX,VW) ),
    //               __vmulfp( __vpermwi( a.m_V, VPERMWI_CONST(VY,VZ,VX,VW) ), __vpermwi( b.m_V, VPERMWI_CONST(VZ,VX,VY,VW) ) ) );
    result.m_V = vmx_cross_product3( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_cross_product3( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VY] * b.m_V[VZ] - a.m_V[VZ] * b.m_V[VY];
    result.m_V[VY] = a.m_V[VZ] * b.m_V[VX] - a.m_V[VX] * b.m_V[VZ];
    result.m_V[VZ] = a.m_V[VX] * b.m_V[VY] - a.m_V[VY] * b.m_V[VX];
    //result.m_V[VW] = 0.0f;
    #endif
    return result;
}


/// Vec4F32 Vector Length Squared (2d)
NOALIAS __forceinline const VF32 LengthSquared2( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_dot_product2( a.m_V, a.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_dot_product2( a.m_V, a.m_V );
    #elif defined( _VMX128_ )
    result.m_V = vmx_dot_product2( a.m_V, a.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_dot_product2( a.m_V, a.m_V );
    #else
    result.m_V = a.m_V[VX] * a.m_V[VX] +
                 a.m_V[VY] * a.m_V[VY];
    #endif
    return result;
}

/// Vec4F32 Vector Length Squared (3d)
NOALIAS __forceinline const VF32 LengthSquared3( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_dot_product3( a.m_V, a.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_dot_product3( a.m_V, a.m_V );
    #elif defined( _VMX128_ )
    result.m_V = vmx_dot_product3( a.m_V, a.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_dot_product3( a.m_V, a.m_V );
    #else
    result.m_V = a.m_V[VX] * a.m_V[VX] +
                 a.m_V[VY] * a.m_V[VY] +
                 a.m_V[VZ] * a.m_V[VZ];
    #endif
    return result;
}

/// Vec4F32 Vector Length Squared (4d)
NOALIAS __forceinline const VF32 LengthSquared4( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_dot_product4( a.m_V, a.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_dot_product4( a.m_V, a.m_V );
    #elif defined( _VMX128_ )
    result.m_V = vmx_dot_product4( a.m_V, a.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_dot_product4( a.m_V, a.m_V );
    #else
    result.m_V = a.m_V[VX] * a.m_V[VX] +
                 a.m_V[VY] * a.m_V[VY] +
                 a.m_V[VZ] * a.m_V[VZ] +
                 a.m_V[VW] * a.m_V[VW];
    #endif
    return result;
}


/// Vec4F32 Vector Length (2d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length2( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = SIMD_Sqrt<22>( spu_dot_product2( a.m_V, a.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = SIMD_Sqrt<22>( vec_dot_product2( a.m_V, a.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = SIMD_Sqrt<22>( vmx_dot_product2( a.m_V, a.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = SIMD_Sqrt<22>( sse_dot_product2( a.m_V, a.m_V ) );
    #else
    result.m_V = sqrtf( a.m_V[VX] * a.m_V[VX] +
                        a.m_V[VY] * a.m_V[VY] );
    #endif
    return result;
}

/// Vec4F32 Vector Length (3d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length3( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = SIMD_Sqrt<22>( spu_dot_product3( a.m_V, a.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = SIMD_Sqrt<22>( vec_dot_product3( a.m_V, a.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = SIMD_Sqrt<22>( vmx_dot_product3( a.m_V, a.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = SIMD_Sqrt<22>( sse_dot_product3( a.m_V, a.m_V ) );
    #else
    result.m_V = sqrtf( a.m_V[VX] * a.m_V[VX] +
                        a.m_V[VY] * a.m_V[VY] +
                        a.m_V[VZ] * a.m_V[VZ] );
    #endif
    return result;
}

/// Vec4F32 Vector Length (4d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Length4( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = SIMD_Sqrt<22>( spu_dot_product4( a.m_V, a.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = SIMD_Sqrt<22>( vec_dot_product4( a.m_V, a.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = SIMD_Sqrt<22>( vmx_dot_product4( a.m_V, a.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = SIMD_Sqrt<22>( sse_dot_product4( a.m_V, a.m_V ) );
    #else
    result.m_V = sqrtf( a.m_V[VX] * a.m_V[VX] +
                        a.m_V[VY] * a.m_V[VY] +
                        a.m_V[VZ] * a.m_V[VZ] +
                        a.m_V[VW] * a.m_V[VW] );
    #endif
    return result;
}



/// Vec4F32 Vector Length (2d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length2_24( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = SIMD_Sqrt<24>( spu_dot_product2( a.m_V, a.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = SIMD_Sqrt<24>( vec_dot_product2( a.m_V, a.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = SIMD_Sqrt<24>( vmx_dot_product2( a.m_V, a.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = SIMD_Sqrt<24>( sse_dot_product2( a.m_V, a.m_V ) );
    #else
    result.m_V = sqrtf( a.m_V[VX] * a.m_V[VX] +
                        a.m_V[VY] * a.m_V[VY] );
    #endif
    return result;
}

/// Vec4F32 Vector Length (3d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length3_24( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = SIMD_Sqrt<24>( spu_dot_product3( a.m_V, a.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = SIMD_Sqrt<24>( vec_dot_product3( a.m_V, a.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = SIMD_Sqrt<24>( vmx_dot_product3( a.m_V, a.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = SIMD_Sqrt<24>( sse_dot_product3( a.m_V, a.m_V ) );
    #else
    result.m_V = sqrtf( a.m_V[VX] * a.m_V[VX] +
                        a.m_V[VY] * a.m_V[VY] +
                        a.m_V[VZ] * a.m_V[VZ] );
    #endif
    return result;
}

/// Vec4F32 Vector Length (4d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Length4_24( const Vec4F32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = SIMD_Sqrt<24>( spu_dot_product4( a.m_V, a.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = SIMD_Sqrt<24>( vec_dot_product4( a.m_V, a.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = SIMD_Sqrt<24>( vmx_dot_product4( a.m_V, a.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = SIMD_Sqrt<24>( sse_dot_product4( a.m_V, a.m_V ) );
    #else
    result.m_V = sqrtf( a.m_V[VX] * a.m_V[VX] +
                        a.m_V[VY] * a.m_V[VY] +
                        a.m_V[VZ] * a.m_V[VZ] +
                        a.m_V[VW] * a.m_V[VW] );
    #endif
    return result;
}



/// Vec4F32 Vector Reciprocal Length (2d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpLength2( const Vec4F32 VECREF a )
{
    return RcpSqrt( LengthSquared2( a ) );
}

/// Vec4F32 Vector Reciprocal Length (3d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpLength3( const Vec4F32 VECREF a )
{
    return RcpSqrt( LengthSquared3( a ) );
}

/// Vec4F32 Vector Reciprocal Length (4d) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpLength4( const Vec4F32 VECREF a )
{
    return RcpSqrt( LengthSquared4( a ) );
}



/// Vec4F32 Vector Reciprocal Length (2d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpLength2_24( const Vec4F32 VECREF a )
{
    return RcpSqrt24( LengthSquared2( a ) );
}

/// Vec4F32 Vector Reciprocal Length (3d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpLength3_24( const Vec4F32 VECREF a )
{
    return RcpSqrt24( LengthSquared3( a ) );
}

/// Vec4F32 Vector Reciprocal Length (4d) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpLength4_24( const Vec4F32 VECREF a )
{
    return RcpSqrt24( LengthSquared4( a ) );
}



/// Vec4F32 Normalise (2d) (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise2( const Vec4F32 VECREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<22>( spu_dot_product2( a.m_V, a.m_V ) ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<22>( vec_dot_product2( a.m_V, a.m_V ) ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<22>( vmx_dot_product2( a.m_V, a.m_V ) ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<22>( sse_dot_product2( a.m_V, a.m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( a.m_V[VX] * a.m_V[VX] +
    //                                    a.m_V[VY] * a.m_V[VY] ) );
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    const F32 rcp_length( valid ? 1.0f / sqrtf( length_squared ) : 0.0f );
    result.m_V[VX] = a.m_V[VX] * rcp_length;
    result.m_V[VY] = a.m_V[VY] * rcp_length;
    #endif
    return result;
}

/// Vec4F32 Normalise (3d) (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise3( const Vec4F32 VECREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<22>( spu_dot_product3( a.m_V, a.m_V ) ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<22>( vec_dot_product3( a.m_V, a.m_V ) ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<22>( vmx_dot_product3( a.m_V, a.m_V ) ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<22>( sse_dot_product3( a.m_V, a.m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( a.m_V[VX] * a.m_V[VX] +
    //                                    a.m_V[VY] * a.m_V[VY] +
    //                                    a.m_V[VZ] * a.m_V[VZ] ) );
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] +
                              a.m_V[VZ] * a.m_V[VZ] );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    const F32 rcp_length( valid ? 1.0f / sqrtf( length_squared ) : 0.0f );
    result.m_V[VX] = a.m_V[VX] * rcp_length;
    result.m_V[VY] = a.m_V[VY] * rcp_length;
    result.m_V[VZ] = a.m_V[VZ] * rcp_length;
    #endif
    return result;
}

/// Vec4F32 Normalise (4d) (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise4( const Vec4F32 VECREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<22>( spu_dot_product4( a.m_V, a.m_V ) ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<22>( vec_dot_product4( a.m_V, a.m_V ) ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<22>( vmx_dot_product4( a.m_V, a.m_V ) ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<22>( sse_dot_product4( a.m_V, a.m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( a.m_V[VX] * a.m_V[VX] +
    //                                    a.m_V[VY] * a.m_V[VY] +
    //                                    a.m_V[VZ] * a.m_V[VZ] +
    //                                    a.m_V[VW] * a.m_V[VW] ) );
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] +
                              a.m_V[VZ] * a.m_V[VZ] +
                              a.m_V[VW] * a.m_V[VW] );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    const F32 rcp_length( valid ? 1.0f / sqrtf( length_squared ) : 0.0f );
    result.m_V[VX] = a.m_V[VX] * rcp_length;
    result.m_V[VY] = a.m_V[VY] * rcp_length;
    result.m_V[VZ] = a.m_V[VZ] * rcp_length;
    result.m_V[VW] = a.m_V[VW] * rcp_length;
    #endif
    return result;
}



/// Vec4F32 Normalise (2d) (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise2_24( const Vec4F32 VECREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<24>( spu_dot_product2( a.m_V, a.m_V ) ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<24>( vec_dot_product2( a.m_V, a.m_V ) ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<24>( vmx_dot_product2( a.m_V, a.m_V ) ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<24>( sse_dot_product2( a.m_V, a.m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( a.m_V[VX] * a.m_V[VX] +
    //                                    a.m_V[VY] * a.m_V[VY] ) );
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    const F32 rcp_length( valid ? 1.0f / sqrtf( length_squared ) : 0.0f );
    result.m_V[VX] = a.m_V[VX] * rcp_length;
    result.m_V[VY] = a.m_V[VY] * rcp_length;
    #endif
    return result;
}

/// Vec4F32 Normalise (3d) (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise3_24( const Vec4F32 VECREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<24>( spu_dot_product3( a.m_V, a.m_V ) ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<24>( vec_dot_product3( a.m_V, a.m_V ) ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<24>( vmx_dot_product3( a.m_V, a.m_V ) ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<24>( sse_dot_product3( a.m_V, a.m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( a.m_V[VX] * a.m_V[VX] +
    //                                    a.m_V[VY] * a.m_V[VY] +
    //                                    a.m_V[VZ] * a.m_V[VZ] ) );
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] +
                              a.m_V[VZ] * a.m_V[VZ] );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    const F32 rcp_length( valid ? 1.0f / sqrtf( length_squared ) : 0.0f );
    result.m_V[VX] = a.m_V[VX] * rcp_length;
    result.m_V[VY] = a.m_V[VY] * rcp_length;
    result.m_V[VZ] = a.m_V[VZ] * rcp_length;
    #endif
    return result;
}

/// Vec4F32 Normalise (4d) (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise4_24( const Vec4F32 VECREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<24>( spu_dot_product4( a.m_V, a.m_V ) ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<24>( vec_dot_product4( a.m_V, a.m_V ) ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<24>( vmx_dot_product4( a.m_V, a.m_V ) ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<24>( sse_dot_product4( a.m_V, a.m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( a.m_V[VX] * a.m_V[VX] +
    //                                    a.m_V[VY] * a.m_V[VY] +
    //                                    a.m_V[VZ] * a.m_V[VZ] +
    //                                    a.m_V[VW] * a.m_V[VW] ) );
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] +
                              a.m_V[VZ] * a.m_V[VZ] +
                              a.m_V[VW] * a.m_V[VW] );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    const F32 rcp_length( valid ? 1.0f / sqrtf( length_squared ) : 0.0f );
    result.m_V[VX] = a.m_V[VX] * rcp_length;
    result.m_V[VY] = a.m_V[VY] * rcp_length;
    result.m_V[VZ] = a.m_V[VZ] * rcp_length;
    result.m_V[VW] = a.m_V[VW] * rcp_length;
    #endif
    return result;
}



/// Vec4F32 Normalise (2d). Returns zeroLengthResult if length of a is zero. (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise2( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<22>( spu_dot_product2( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<22>( vec_dot_product2( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<22>( vmx_dot_product2( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<22>( sse_dot_product2( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #else
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    const F32 rcp_length( 1.0f / sqrtf( length_squared ) );
    result.m_V[VX] = valid ? a.m_V[VX] * rcp_length : zeroLengthResult.m_V[VX];
    result.m_V[VY] = valid ? a.m_V[VY] * rcp_length : zeroLengthResult.m_V[VY];
    #endif
    return result;
}

/// Vec4F32 Normalise (3d). Returns zeroLengthResult if length of a is zero. (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise3( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<22>( spu_dot_product3( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<22>( vec_dot_product3( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<22>( vmx_dot_product3( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<22>( sse_dot_product3( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #else
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] +
                              a.m_V[VZ] * a.m_V[VZ] );
    const F32 rcp_length( 1.0f / sqrtf( length_squared ) );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    result.m_V[VX] = valid ? a.m_V[VX] * rcp_length : zeroLengthResult.m_V[VX];
    result.m_V[VY] = valid ? a.m_V[VY] * rcp_length : zeroLengthResult.m_V[VY];
    result.m_V[VZ] = valid ? a.m_V[VZ] * rcp_length : zeroLengthResult.m_V[VZ];
    #endif
    return result;
}

/// Vec4F32 Normalise (4d). Returns zeroLengthResult if length of a is zero. (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise4( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<22>( spu_dot_product4( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<22>( vec_dot_product4( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<22>( vmx_dot_product4( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<22>( sse_dot_product4( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #else
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] +
                              a.m_V[VZ] * a.m_V[VZ] +
                              a.m_V[VW] * a.m_V[VW] );
    const F32 rcp_length( 1.0f / sqrtf( length_squared ) );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    result.m_V[VX] = valid ? a.m_V[VX] * rcp_length : zeroLengthResult.m_V[VX];
    result.m_V[VY] = valid ? a.m_V[VY] * rcp_length : zeroLengthResult.m_V[VY];
    result.m_V[VZ] = valid ? a.m_V[VZ] * rcp_length : zeroLengthResult.m_V[VZ];
    result.m_V[VW] = valid ? a.m_V[VW] * rcp_length : zeroLengthResult.m_V[VW];
    #endif
    return result;
}



/// Vec4F32 Normalise (2d). Returns zeroLengthResult if length of a is zero. (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise2_24( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<24>( spu_dot_product2( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<24>( vec_dot_product2( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<24>( vmx_dot_product2( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<24>( sse_dot_product2( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #else
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] );
    const F32 rcp_length( 1.0f / sqrtf( length_squared ) );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    result.m_V[VX] = valid ? a.m_V[VX] * rcp_length : zeroLengthResult.m_V[VX];
    result.m_V[VY] = valid ? a.m_V[VY] * rcp_length : zeroLengthResult.m_V[VY];
    #endif
    return result;
}

/// Vec4F32 Normalise (3d). Returns zeroLengthResult if length of a is zero. (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise3_24( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<24>( spu_dot_product3( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<24>( vec_dot_product3( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<24>( vmx_dot_product3( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<24>( sse_dot_product3( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #else
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] +
                              a.m_V[VZ] * a.m_V[VZ] );
    const F32 rcp_length( 1.0f / sqrtf( length_squared ) );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    result.m_V[VX] = valid ? a.m_V[VX] * rcp_length : zeroLengthResult.m_V[VX];
    result.m_V[VY] = valid ? a.m_V[VY] * rcp_length : zeroLengthResult.m_V[VY];
    result.m_V[VZ] = valid ? a.m_V[VZ] * rcp_length : zeroLengthResult.m_V[VZ];
    #endif
    return result;
}

/// Vec4F32 Normalise (4d). Returns zeroLengthResult if length of a is zero. (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Normalise4_24( const Vec4F32 VECREF a, const Vec4F32 VECREF zeroLengthResult )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, SIMD_RcpSqrt<24>( spu_dot_product4( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, SIMD_RcpSqrt<24>( vec_dot_product4( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, SIMD_RcpSqrt<24>( vmx_dot_product4( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, SIMD_RcpSqrt<24>( sse_dot_product4( a.m_V, a.m_V ), zeroLengthResult.m_V ) );
    #else
    const F32 length_squared( a.m_V[VX] * a.m_V[VX] +
                              a.m_V[VY] * a.m_V[VY] +
                              a.m_V[VZ] * a.m_V[VZ] +
                              a.m_V[VW] * a.m_V[VW] );
    const F32 rcp_length( 1.0f / sqrtf( length_squared ) );
    const bool valid( ( length_squared > FLT_MIN ) && ( length_squared < FLT_MAX ) );
    result.m_V[VX] = valid ? a.m_V[VX] * rcp_length : zeroLengthResult.m_V[VX];
    result.m_V[VY] = valid ? a.m_V[VY] * rcp_length : zeroLengthResult.m_V[VY];
    result.m_V[VZ] = valid ? a.m_V[VZ] * rcp_length : zeroLengthResult.m_V[VZ];
    result.m_V[VW] = valid ? a.m_V[VW] * rcp_length : zeroLengthResult.m_V[VW];
    #endif
    return result;
}



/// Vec4F32 Normalise (2d) (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise2( Vec4F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<22>( spu_dot_product2( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<22>( vec_dot_product2( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<22>( vmx_dot_product2( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<22>( sse_dot_product2( pDst->m_V, pDst->m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    *pDst = Normalise2( *pDst );
    #endif
}

/// Vec4F32 Normalise (3d) (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise3( Vec4F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<22>( spu_dot_product3( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<22>( vec_dot_product3( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<22>( vmx_dot_product3( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<22>( sse_dot_product3( pDst->m_V, pDst->m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] +
    //                                    pDst->m_V[VZ] * pDst->m_V[VZ] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    //pDst->m_V[VZ] = pDst->m_V[VZ] * rcp_length;
    *pDst = Normalise3( *pDst );
    #endif
}

/// Vec4F32 Normalise (4d) (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise4( Vec4F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<22>( spu_dot_product4( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<22>( vec_dot_product4( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<22>( vmx_dot_product4( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<22>( sse_dot_product4( pDst->m_V, pDst->m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] +
    //                                    pDst->m_V[VZ] * pDst->m_V[VZ] +
    //                                    pDst->m_V[VW] * pDst->m_V[VW] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    //pDst->m_V[VZ] = pDst->m_V[VZ] * rcp_length;
    //pDst->m_V[VW] = pDst->m_V[VW] * rcp_length;
    *pDst = Normalise4( *pDst );
    #endif
}


/// Vec4F32 Normalise (2d) (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise2_24( Vec4F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<24>( spu_dot_product2( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<24>( vec_dot_product2( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<24>( vmx_dot_product2( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<24>( sse_dot_product2( pDst->m_V, pDst->m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    *pDst = Normalise2( *pDst );
    #endif
}

/// Vec4F32 Normalise (3d) (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise3_24( Vec4F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<24>( spu_dot_product3( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<24>( vec_dot_product3( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<24>( vmx_dot_product3( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<24>( sse_dot_product3( pDst->m_V, pDst->m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] +
    //                                    pDst->m_V[VZ] * pDst->m_V[VZ] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    //pDst->m_V[VZ] = pDst->m_V[VZ] * rcp_length;
    *pDst = Normalise3( *pDst );
    #endif
}

/// Vec4F32 Normalise (4d) (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise4_24( Vec4F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<24>( spu_dot_product4( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<24>( vec_dot_product4( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<24>( vmx_dot_product4( pDst->m_V, pDst->m_V ) ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<24>( sse_dot_product4( pDst->m_V, pDst->m_V ) ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] +
    //                                    pDst->m_V[VZ] * pDst->m_V[VZ] +
    //                                    pDst->m_V[VW] * pDst->m_V[VW] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    //pDst->m_V[VZ] = pDst->m_V[VZ] * rcp_length;
    //pDst->m_V[VW] = pDst->m_V[VW] * rcp_length;
    *pDst = Normalise4( *pDst );
    #endif
}


/// Vec4F32 Normalise (2d). Sets zeroLengthResult if length of *pDst is zero. (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise2( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<22>( spu_dot_product2( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<22>( vec_dot_product2( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<22>( vmx_dot_product2( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<22>( sse_dot_product2( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    *pDst = Normalise2( *pDst );
    #endif
}

/// Vec4F32 Normalise (3d). Sets zeroLengthResult if length of *pDst is zero. (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise3( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<22>( spu_dot_product3( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<22>( vec_dot_product3( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<22>( vmx_dot_product3( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<22>( sse_dot_product3( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] +
    //                                    pDst->m_V[VZ] * pDst->m_V[VZ] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    //pDst->m_V[VZ] = pDst->m_V[VZ] * rcp_length;
    *pDst = Normalise3( *pDst );
    #endif
}

/// Vec4F32 Normalise (4d). Sets zeroLengthResult if length of *pDst is zero. (faster but only 22 bit precision)
NOALIAS __forceinline void Normalise4( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<22>( spu_dot_product4( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<22>( vec_dot_product4( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<22>( vmx_dot_product4( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<22>( sse_dot_product4( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] +
    //                                    pDst->m_V[VZ] * pDst->m_V[VZ] +
    //                                    pDst->m_V[VW] * pDst->m_V[VW] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    //pDst->m_V[VZ] = pDst->m_V[VZ] * rcp_length;
    //pDst->m_V[VW] = pDst->m_V[VW] * rcp_length;
    *pDst = Normalise4( *pDst );
    #endif
}


/// Vec4F32 Normalise (2d). Sets zeroLengthResult if length of *pDst is zero. (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise2_24( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<24>( spu_dot_product2( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<24>( vec_dot_product2( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<24>( vmx_dot_product2( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<24>( sse_dot_product2( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    *pDst = Normalise2( *pDst );
    #endif
}

/// Vec4F32 Normalise (3d). Sets zeroLengthResult if length of *pDst is zero. (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise3_24( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<24>( spu_dot_product3( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<24>( vec_dot_product3( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<24>( vmx_dot_product3( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<24>( sse_dot_product3( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] +
    //                                    pDst->m_V[VZ] * pDst->m_V[VZ] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    //pDst->m_V[VZ] = pDst->m_V[VZ] * rcp_length;
    *pDst = Normalise3( *pDst );
    #endif
}

/// Vec4F32 Normalise (4d). Sets zeroLengthResult if length of *pDst is zero. (slower but full 24 bit precision)
NOALIAS __forceinline void Normalise4_24( Vec4F32* const __restrict pDst, const Vec4F32 VECREF zeroLengthResult )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_mul( pDst->m_V, SIMD_RcpSqrt<24>( spu_dot_product4( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _PPU_ )
    pDst->m_V = vec_mul( pDst->m_V, SIMD_RcpSqrt<24>( vec_dot_product4( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _VMX128_ )
    pDst->m_V = __vmulfp( pDst->m_V, SIMD_RcpSqrt<24>( vmx_dot_product4( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #elif defined( _SSE_ )
    pDst->m_V = _mm_mul_ps( pDst->m_V, SIMD_RcpSqrt<24>( sse_dot_product4( pDst->m_V, pDst->m_V ), zeroLengthResult.m_V ) );
    #else
    //const F32 rcp_length( 1.0f / sqrtf( pDst->m_V[VX] * pDst->m_V[VX] +
    //                                    pDst->m_V[VY] * pDst->m_V[VY] +
    //                                    pDst->m_V[VZ] * pDst->m_V[VZ] +
    //                                    pDst->m_V[VW] * pDst->m_V[VW] ) );
    //pDst->m_V[VX] = pDst->m_V[VX] * rcp_length;
    //pDst->m_V[VY] = pDst->m_V[VY] * rcp_length;
    //pDst->m_V[VZ] = pDst->m_V[VZ] * rcp_length;
    //pDst->m_V[VW] = pDst->m_V[VW] * rcp_length;
    *pDst = Normalise4( *pDst );
    #endif
}


/// Vec4F32 Vector Set Length (2d) (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength2( Vec4F32* const __restrict pV, const VF32 VECREF length )
{
    const Vec4F32 v( *pV );
    *pV = v * length * RcpSqrt( LengthSquared2( v ) );
}

/// Vec4F32 Vector Set Length (3d) (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength3( Vec4F32* const __restrict pV, const VF32 VECREF length )
{
    const Vec4F32 v( *pV );
    *pV = v * length * RcpSqrt( LengthSquared3( v ) );
}

/// Vec4F32 Vector Set Length (4d) (faster but only 22 bit precision)
NOALIAS __forceinline void SetLength4( Vec4F32* const __restrict pV, const VF32 VECREF length )
{
    const Vec4F32 v( *pV );
    *pV = v * length * RcpSqrt( LengthSquared4( v ) );
}


/// Vec4F32 Vector Set Length (2d) (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength2_24( Vec4F32* const __restrict pV, const VF32 VECREF length )
{
    const Vec4F32 v( *pV );
    *pV = v * length * RcpSqrt24( LengthSquared2( v ) );
}

/// Vec4F32 Vector Set Length (3d) (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength3_24( Vec4F32* const __restrict pV, const VF32 VECREF length )
{
    const Vec4F32 v( *pV );
    *pV = v * length * RcpSqrt24( LengthSquared3( v ) );
}

/// Vec4F32 Vector Set Length (4d) (slower but full 24 bit precision)
NOALIAS __forceinline void SetLength4_24( Vec4F32* const __restrict pV, const VF32 VECREF length )
{
    const Vec4F32 v( *pV );
    *pV = v * length * RcpSqrt24( LengthSquared4( v ) );
}


/// Vec4F32 Limit Length (2d) (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength2( Vec4F32* const __restrict pV, const VF32 VECREF limit )
{
    const Vec4F32 v( *pV );
    const VF32 lengthSquared( LengthSquared2( v ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt( lengthSquared, &length, &rcpLength );
    *pV = Select( length <= limit, v, v * rcpLength * limit );
}

/// Vec4F32 Limit Length (3d) (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength3( Vec4F32* const __restrict pV, const VF32 VECREF limit )
{
    const Vec4F32 v( *pV );
    const VF32 lengthSquared( LengthSquared3( v ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt( lengthSquared, &length, &rcpLength );
    *pV = Select( length <= limit, v, v * rcpLength * limit );
}

/// Vec4F32 Limit Length (4d) (faster but only 22 bit precision)
NOALIAS __forceinline void LimitLength4( Vec4F32* const __restrict pV, const VF32 VECREF limit )
{
    const Vec4F32 v( *pV );
    const VF32 lengthSquared( LengthSquared4( v ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt( lengthSquared, &length, &rcpLength );
    *pV = Select( length <= limit, v, v * rcpLength * limit );
}


/// Vec4F32 Limit Length (2d) (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength2_24( Vec4F32* const __restrict pV, const VF32 VECREF limit )
{
    const Vec4F32 v( *pV );
    const VF32 lengthSquared( LengthSquared2( v ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt24( lengthSquared, &length, &rcpLength );
    *pV = Select( length <= limit, v, v * rcpLength * limit );
}

/// Vec4F32 Limit Length (3d) (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength3_24( Vec4F32* const __restrict pV, const VF32 VECREF limit )
{
    const Vec4F32 v( *pV );
    const VF32 lengthSquared( LengthSquared3( v ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt24( lengthSquared, &length, &rcpLength );
    *pV = Select( length <= limit, v, v * rcpLength * limit );
}

/// Vec4F32 Limit Length (4d) (slower but full 24 bit precision)
NOALIAS __forceinline void LimitLength4_24( Vec4F32* const __restrict pV, const VF32 VECREF limit )
{
    const Vec4F32 v( *pV );
    const VF32 lengthSquared( LengthSquared4( v ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt24( lengthSquared, &length, &rcpLength );
    *pV = Select( length <= limit, v, v * rcpLength * limit );
}


/// Vec4F32 Inverse (2d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse2( const Vec4F32 VECREF v )
{
    return v * Rcp( LengthSquared2( v ) );
}

/// Vec4F32 Inverse (3d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse3( const Vec4F32 VECREF v )
{
    return v * Rcp( LengthSquared3( v ) );
}

/// Vec4F32 Inverse (4d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (faster but only 22 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse4( const Vec4F32 VECREF v )
{
    return v * Rcp( LengthSquared4( v ) );
}


/// Vec4F32 Inverse (2d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse2_24( const Vec4F32 VECREF v )
{
    return v * Rcp24( LengthSquared2( v ) );
}

/// Vec4F32 Inverse (3d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse3_24( const Vec4F32 VECREF v )
{
    return v * Rcp24( LengthSquared3( v ) );
}

/// Vec4F32 Inverse (4d): Returns v^-1, parallel to v and Dot( v, v^-1 ) == 1 (slower but full 24 bit precision)
NOALIAS __forceinline const Vec4F32 Inverse4_24( const Vec4F32 VECREF v )
{
    return v * Rcp24( LengthSquared4( v ) );
}


/// Vec4F32 Flush To Zero (2d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec4F32 FlushToZero2( const Vec4F32 VECREF v, const VF32 VECREF tolerance )
{
    return AND( v, LengthSquared2( v ) > ( tolerance * tolerance ) );
}

/// Vec4F32 Flush To Zero (3d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec4F32 FlushToZero3( const Vec4F32 VECREF v, const VF32 VECREF tolerance )
{
    return AND( v, LengthSquared3( v ) > ( tolerance * tolerance ) );
}

/// Vec4F32 Flush To Zero (4d): flushes vector to zero if length less than or equal to tolerance
NOALIAS __forceinline const Vec4F32 FlushToZero4( const Vec4F32 VECREF v, const VF32 VECREF tolerance )
{
    return AND( v, LengthSquared4( v ) > ( tolerance * tolerance ) );
}


/// Vec4F32 Flush Elements To Zero: flushes elements to zero if less than or equal to tolerance
NOALIAS __forceinline const Vec4F32 FlushElementsToZero( const Vec4F32 VECREF value, const VF32 VECREF tolerance )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_and( value.m_V, (vector float)spu_cmpabsgt( value.m_V, tolerance.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_and( value.m_V, vec_cmpgt( vec_abs( value.m_V ), tolerance.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vand( value.m_V, __vcmpgtfp( vmx_abs( value.m_V ), tolerance.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_ps( value.m_V, _mm_cmpgt_ps( sse_abs( value.m_V ), tolerance.m_V ) );
    #else
    result.m_V[0] = Abs( value.m_V[0] ) > tolerance.m_V ? value.m_V[0] : 0.0f;
    result.m_V[1] = Abs( value.m_V[1] ) > tolerance.m_V ? value.m_V[1] : 0.0f;
    result.m_V[2] = Abs( value.m_V[2] ) > tolerance.m_V ? value.m_V[2] : 0.0f;
    result.m_V[3] = Abs( value.m_V[3] ) > tolerance.m_V ? value.m_V[3] : 0.0f;
    #endif
    return result;
}


/// Vector Space Tensor Product of a 3x1 column vector and a 1x3 row vector -> Mat33
NOALIAS __forceinline const Mat33 TensorProduct33( const Vec4F32 VECREF lhs, const Vec4F32 VECREF rhs )
{
    Mat33 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_mul( spu_splat<VX>( lhs.m_V ), rhs.m_V );
    result.m_RowY = spu_mul( spu_splat<VY>( lhs.m_V ), rhs.m_V );
    result.m_RowZ = spu_mul( spu_splat<VZ>( lhs.m_V ), rhs.m_V );
    #elif defined( _PPU_ )
    result.m_RowX = vec_mul( vec_splat( lhs.m_V, VX ), rhs.m_V );
    result.m_RowY = vec_mul( vec_splat( lhs.m_V, VY ), rhs.m_V );
    result.m_RowZ = vec_mul( vec_splat( lhs.m_V, VZ ), rhs.m_V );
    #elif defined( _VMX128_ )
    result.m_ColX = __vmulfp( lhs.m_V, __vspltw( rhs.m_V, VX ) );
    result.m_ColY = __vmulfp( lhs.m_V, __vspltw( rhs.m_V, VY ) );
    result.m_ColZ = __vmulfp( lhs.m_V, __vspltw( rhs.m_V, VZ ) );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_mul_ps( sse_splat<VX>( lhs.m_V ), rhs.m_V );
    result.m_RowY = _mm_mul_ps( sse_splat<VY>( lhs.m_V ), rhs.m_V );
    result.m_RowZ = _mm_mul_ps( sse_splat<VZ>( lhs.m_V ), rhs.m_V );
    #else
    result.m[0][0] = lhs.m_V[0] * rhs.m_V[0]; result.m[0][1] = lhs.m_V[0] * rhs.m_V[1]; result.m[0][2] = lhs.m_V[0] * rhs.m_V[2];
    result.m[1][0] = lhs.m_V[1] * rhs.m_V[0]; result.m[1][1] = lhs.m_V[1] * rhs.m_V[1]; result.m[1][2] = lhs.m_V[1] * rhs.m_V[2];
    result.m[2][0] = lhs.m_V[2] * rhs.m_V[0]; result.m[2][1] = lhs.m_V[2] * rhs.m_V[1]; result.m[2][2] = lhs.m_V[2] * rhs.m_V[2];
    #endif
    return result;
}

/// Vector Space Tensor Product of a 4x1 column vector and a 1x4 row vector -> Mat44
NOALIAS __forceinline const Mat44 TensorProduct44( const Vec4F32 VECREF lhs, const Vec4F32 VECREF rhs )
{
    Mat44 result;
    #if defined( _SPU_ )
    result.m_RowX = spu_mul( spu_splat<VX>( lhs.m_V ), rhs.m_V );
    result.m_RowY = spu_mul( spu_splat<VY>( lhs.m_V ), rhs.m_V );
    result.m_RowZ = spu_mul( spu_splat<VZ>( lhs.m_V ), rhs.m_V );
    result.m_RowW = spu_mul( spu_splat<VW>( lhs.m_V ), rhs.m_V );
    #elif defined( _PPU_ )
    result.m_RowX = vec_mul( vec_splat( lhs.m_V, VX ), rhs.m_V );
    result.m_RowY = vec_mul( vec_splat( lhs.m_V, VY ), rhs.m_V );
    result.m_RowZ = vec_mul( vec_splat( lhs.m_V, VZ ), rhs.m_V );
    result.m_RowW = vec_mul( vec_splat( lhs.m_V, VW ), rhs.m_V );
    #elif defined( _VMX128_ )
    result.m_ColX = __vmulfp( lhs.m_V, __vspltw( rhs.m_V, VX ) );
    result.m_ColY = __vmulfp( lhs.m_V, __vspltw( rhs.m_V, VY ) );
    result.m_ColZ = __vmulfp( lhs.m_V, __vspltw( rhs.m_V, VZ ) );
    result.m_ColW = __vmulfp( lhs.m_V, __vspltw( rhs.m_V, VW ) );
    #elif defined( _SSE_ )
    result.m_RowX = _mm_mul_ps( sse_splat<VX>( lhs.m_V ), rhs.m_V );
    result.m_RowY = _mm_mul_ps( sse_splat<VY>( lhs.m_V ), rhs.m_V );
    result.m_RowZ = _mm_mul_ps( sse_splat<VZ>( lhs.m_V ), rhs.m_V );
    result.m_RowW = _mm_mul_ps( sse_splat<VW>( lhs.m_V ), rhs.m_V );
    #else
    result.m[0][0] = lhs.m_V[0] * rhs.m_V[0]; result.m[0][1] = lhs.m_V[0] * rhs.m_V[1]; result.m[0][2] = lhs.m_V[0] * rhs.m_V[2]; result.m[0][3] = lhs.m_V[0] * rhs.m_V[3];
    result.m[1][0] = lhs.m_V[1] * rhs.m_V[0]; result.m[1][1] = lhs.m_V[1] * rhs.m_V[1]; result.m[1][2] = lhs.m_V[1] * rhs.m_V[2]; result.m[1][3] = lhs.m_V[1] * rhs.m_V[3];
    result.m[2][0] = lhs.m_V[2] * rhs.m_V[0]; result.m[2][1] = lhs.m_V[2] * rhs.m_V[1]; result.m[2][2] = lhs.m_V[2] * rhs.m_V[2]; result.m[2][3] = lhs.m_V[2] * rhs.m_V[3];
    result.m[3][0] = lhs.m_V[3] * rhs.m_V[0]; result.m[3][1] = lhs.m_V[3] * rhs.m_V[1]; result.m[3][2] = lhs.m_V[3] * rhs.m_V[2]; result.m[3][3] = lhs.m_V[3] * rhs.m_V[3];
    #endif
    return result;
}


/// Vec4F32 Rotate Elements Left
template < int N >
NOALIAS __forceinline const Vec4F32 RotateLeft( const Vec4F32 VECREF a )
{
    Vec4F32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rlqwbyte( a.m_V, N * 4 );
    #elif defined( _PPU_ )
    result.m_V = vec_sld( a.m_V, a.m_V, N * 4 );
    #elif defined( _VMX128_ )
    result.m_V = __vsldoi( a.m_V, a.m_V, N * 4 );
    #elif defined( _SSE_ )
    result.m_V = sse_rotate_left_qw_words<1>( a.m_V );
    #else
    result.m_V[VX] = a.m_V[VY];
    result.m_V[VY] = a.m_V[VZ];
    result.m_V[VZ] = a.m_V[VW];
    result.m_V[VW] = a.m_V[VX];
    #endif
    return result;
}

/// Vec4F32 Rotate Elements Right
template < int N >
NOALIAS __forceinline const Vec4F32 RotateRight( const Vec4F32 VECREF a )
{
    return RotateLeft< -N & 3 >( a );
}





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )




