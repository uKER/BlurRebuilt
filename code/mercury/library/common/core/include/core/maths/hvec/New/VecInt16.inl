#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECINT16_INL
#define SOFT_MATHS_HVEC_NEW_VECINT16_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECINT16_INL
#define MATHS_HVEC_NEW_VECINT16_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup IntegerVector
/// @{

/// Vec8S16: Load first four elements from Short4, last 4 elements are undefined!
NOALIAS __forceinline const Vec8S16 Load( MathsStorage::Short4 const * const __restrict pSrc )
{
    Vec8S16 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_unaligned( &pSrc->x );
    #elif defined( _PPU_ )
    result.m_V = vec_load_unaligned( &pSrc->x );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_unaligned( &pSrc->x );
    #elif defined( _SSE_ )
    result.m_V = sse_load_unaligned( &pSrc->x );
    #else
    result.m_V[0] = pSrc->x;
    result.m_V[1] = pSrc->y;
    result.m_V[2] = pSrc->z;
    result.m_V[3] = pSrc->w;
    #endif
    return result;
}

/// Vec8S16: Store first four elements into Short4
NOALIAS __forceinline void Store( const Vec8S16 PCREF data, MathsStorage::Short4 * const __restrict pDst )
{
    #if defined( _SPU_ )
    pDst->x = spu_extract( data.m_V, 0 );
    pDst->y = spu_extract( data.m_V, 1 );
    pDst->z = spu_extract( data.m_V, 2 );
    pDst->w = spu_extract( data.m_V, 3 );
    #elif defined( _PPU_ )
    vec_ste( vec_splat( data.m_V, 0 ), 0x0, &pDst->x );
    vec_ste( vec_splat( data.m_V, 1 ), 0x0, &pDst->y );
    vec_ste( vec_splat( data.m_V, 2 ), 0x0, &pDst->z );
    vec_ste( vec_splat( data.m_V, 3 ), 0x0, &pDst->w );
    #elif defined( _VMX128_ )
    __stvehx( __vsplth( data.m_V, 0 ), &pDst->x, 0x0 );
    __stvehx( __vsplth( data.m_V, 1 ), &pDst->y, 0x0 );
    __stvehx( __vsplth( data.m_V, 2 ), &pDst->z, 0x0 );
    __stvehx( __vsplth( data.m_V, 3 ), &pDst->w, 0x0 );
    #elif defined( _SSE_ )
    _mm_maskmoveu_si128( data.m_V, (__m128i&)Const<__m128>::Mask<1,1,0,0>(), (char*)pDst );
    _mm_mfence();
    #else
    pDst->x = data.m_V[0];
    pDst->y = data.m_V[1];
    pDst->z = data.m_V[2];
    pDst->w = data.m_V[3];
    #endif
}

/// Vec8S16: Load first four elements from ShortN4, last 4 elements are undefined!
NOALIAS __forceinline const Vec8S16 Load( MathsStorage::ShortN4 const * const __restrict pSrc )
{
    Vec8S16 result;
    #if defined( _SPU_ )
    result.m_V = spu_load_unaligned( &pSrc->x );
    #elif defined( _PPU_ )
    result.m_V = vec_load_unaligned( &pSrc->x );
    #elif defined( _VMX128_ )
    result.m_V = vmx_load_unaligned( &pSrc->x );
    #elif defined( _SSE_ )
    result.m_V = sse_load_unaligned( &pSrc->x );
    #else
    result.m_V[0] = pSrc->x;
    result.m_V[1] = pSrc->y;
    result.m_V[2] = pSrc->z;
    result.m_V[3] = pSrc->w;
    #endif
    return result;
}

/// Vec8S16: Store first four elements into ShortN4
NOALIAS __forceinline void Store( const Vec8S16 PCREF data, MathsStorage::ShortN4 * const __restrict pDst )
{
    #if defined( _SPU_ )
    pDst->x = spu_extract( data.m_V, 0 );
    pDst->y = spu_extract( data.m_V, 1 );
    pDst->z = spu_extract( data.m_V, 2 );
    pDst->w = spu_extract( data.m_V, 3 );
    #elif defined( _PPU_ )
    vec_ste( vec_splat( data.m_V, 0 ), 0x0, &pDst->x );
    vec_ste( vec_splat( data.m_V, 1 ), 0x0, &pDst->y );
    vec_ste( vec_splat( data.m_V, 2 ), 0x0, &pDst->z );
    vec_ste( vec_splat( data.m_V, 3 ), 0x0, &pDst->w );
    #elif defined( _VMX128_ )
    __stvehx( __vsplth( data.m_V, 0 ), &pDst->x, 0x0 );
    __stvehx( __vsplth( data.m_V, 1 ), &pDst->y, 0x0 );
    __stvehx( __vsplth( data.m_V, 2 ), &pDst->z, 0x0 );
    __stvehx( __vsplth( data.m_V, 3 ), &pDst->w, 0x0 );
    #elif defined( _SSE_ )
    _mm_maskmoveu_si128( data.m_V, (__m128i&)Const<__m128>::Mask<1,1,0,0>(), (char*)pDst );
    _mm_mfence();
    #else
    pDst->x = data.m_V[0];
    pDst->y = data.m_V[1];
    pDst->z = data.m_V[2];
    pDst->w = data.m_V[3];
    #endif
}


/// Pack two Vec8S16's into one Vec16S8 with Signed Saturation to [ -2^7, +2^7-1 ]
NOALIAS __forceinline const Vec16S8 PackSS( const Vec8S16 PCREF a, const Vec8S16 PCREF b )
{
    Vec16S8 result;
    #if defined( _SPU_ )
    result.m_V = spu_pack_ss( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vpkshss( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vpkshss( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_pack_sat_s16_to_s8( a.m_V, b.m_V );
    #else
    const S16 limit_lo( -128 );
    const S16 limit_hi( 127 );
    result.m_V[0x0] = (S8)Clamp( a.m_V[0], limit_lo, limit_hi );
    result.m_V[0x1] = (S8)Clamp( a.m_V[1], limit_lo, limit_hi );
    result.m_V[0x2] = (S8)Clamp( a.m_V[2], limit_lo, limit_hi );
    result.m_V[0x3] = (S8)Clamp( a.m_V[3], limit_lo, limit_hi );
    result.m_V[0x4] = (S8)Clamp( a.m_V[4], limit_lo, limit_hi );
    result.m_V[0x5] = (S8)Clamp( a.m_V[5], limit_lo, limit_hi );
    result.m_V[0x6] = (S8)Clamp( a.m_V[6], limit_lo, limit_hi );
    result.m_V[0x7] = (S8)Clamp( a.m_V[7], limit_lo, limit_hi );
    result.m_V[0x8] = (S8)Clamp( b.m_V[0], limit_lo, limit_hi );
    result.m_V[0x9] = (S8)Clamp( b.m_V[1], limit_lo, limit_hi );
    result.m_V[0xA] = (S8)Clamp( b.m_V[2], limit_lo, limit_hi );
    result.m_V[0xB] = (S8)Clamp( b.m_V[3], limit_lo, limit_hi );
    result.m_V[0xC] = (S8)Clamp( b.m_V[4], limit_lo, limit_hi );
    result.m_V[0xD] = (S8)Clamp( b.m_V[5], limit_lo, limit_hi );
    result.m_V[0xE] = (S8)Clamp( b.m_V[6], limit_lo, limit_hi );
    result.m_V[0xF] = (S8)Clamp( b.m_V[7], limit_lo, limit_hi );
    #endif
    return result;
}

/// Pack two Vec8S16's into one Vec16U8 with Unsigned Saturation to [ 0, +2^8-1 ]
NOALIAS __forceinline const Vec16U8 PackUS( const Vec8S16 PCREF a, const Vec8S16 PCREF b )
{
    Vec16U8 result;
    #if defined( _SPU_ )
    result.m_V = spu_pack_us( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vpkshus( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vpkshus( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_pack_sat_s16_to_u8( a.m_V, b.m_V );
    #else
    const S16 limit_lo( 0 );
    const S16 limit_hi( 255 );
    result.m_V[0x0] = (U8)Clamp( a.m_V[0], limit_lo, limit_hi );
    result.m_V[0x1] = (U8)Clamp( a.m_V[1], limit_lo, limit_hi );
    result.m_V[0x2] = (U8)Clamp( a.m_V[2], limit_lo, limit_hi );
    result.m_V[0x3] = (U8)Clamp( a.m_V[3], limit_lo, limit_hi );
    result.m_V[0x4] = (U8)Clamp( a.m_V[4], limit_lo, limit_hi );
    result.m_V[0x5] = (U8)Clamp( a.m_V[5], limit_lo, limit_hi );
    result.m_V[0x6] = (U8)Clamp( a.m_V[6], limit_lo, limit_hi );
    result.m_V[0x7] = (U8)Clamp( a.m_V[7], limit_lo, limit_hi );
    result.m_V[0x8] = (U8)Clamp( b.m_V[0], limit_lo, limit_hi );
    result.m_V[0x9] = (U8)Clamp( b.m_V[1], limit_lo, limit_hi );
    result.m_V[0xA] = (U8)Clamp( b.m_V[2], limit_lo, limit_hi );
    result.m_V[0xB] = (U8)Clamp( b.m_V[3], limit_lo, limit_hi );
    result.m_V[0xC] = (U8)Clamp( b.m_V[4], limit_lo, limit_hi );
    result.m_V[0xD] = (U8)Clamp( b.m_V[5], limit_lo, limit_hi );
    result.m_V[0xE] = (U8)Clamp( b.m_V[6], limit_lo, limit_hi );
    result.m_V[0xF] = (U8)Clamp( b.m_V[7], limit_lo, limit_hi );
    #endif
    return result;
}

/// Unpack Vec16U8 into two Vec8S16's
NOALIAS __forceinline void Unpack( const Vec16U8 PCREF a, Vec8S16* const __restrict pB, Vec8S16* const __restrict pC )
{
    #if defined( _SPU_ )
    const vector unsigned char zero( Const< vector unsigned char >::Zero() );
    pB->m_V = (vector signed short)spu_merge_hi( zero, a.m_V );
    pC->m_V = (vector signed short)spu_merge_lo( zero, a.m_V );
    #elif defined( _PPU_ )
    const vector unsigned char zero( Const< vector unsigned char >::Zero() );
    pB->m_V = (vector signed short)vec_mergeh( zero, a.m_V );
    pC->m_V = (vector signed short)vec_mergel( zero, a.m_V );
    #elif defined( _VMX128_ )
    const __vector4 zero( Const<__vector4>::Zero() );
    pB->m_V = __vmrghb( zero, a.m_V );
    pC->m_V = __vmrglb( zero, a.m_V );
    #elif defined( _SSE_ )
    const __m128i zero( Const<__m128i>::Zero() );
    pB->m_V = _mm_unpacklo_epi8( zero, a.m_V );
    pC->m_V = _mm_unpackhi_epi8( zero, a.m_V );
    #else
    pB->m_V[0x0] = a.m_V[0x0];
    pB->m_V[0x1] = a.m_V[0x1];
    pB->m_V[0x2] = a.m_V[0x2];
    pB->m_V[0x3] = a.m_V[0x3];
    pB->m_V[0x4] = a.m_V[0x4];
    pB->m_V[0x5] = a.m_V[0x5];
    pB->m_V[0x6] = a.m_V[0x6];
    pB->m_V[0x7] = a.m_V[0x7];
    pC->m_V[0x0] = a.m_V[0x8];
    pC->m_V[0x1] = a.m_V[0x9];
    pC->m_V[0x2] = a.m_V[0xA];
    pC->m_V[0x3] = a.m_V[0xB];
    pC->m_V[0x4] = a.m_V[0xC];
    pC->m_V[0x5] = a.m_V[0xD];
    pC->m_V[0x6] = a.m_V[0xE];
    pC->m_V[0x7] = a.m_V[0xF];
    #endif
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


