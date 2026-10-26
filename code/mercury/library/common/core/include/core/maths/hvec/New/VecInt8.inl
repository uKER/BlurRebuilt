#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECINT8_INL
#define SOFT_MATHS_HVEC_NEW_VECINT8_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECINT8_INL
#define MATHS_HVEC_NEW_VECINT8_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup IntegerVector
/// @{

/// Vec16U8: Load first four elements from _UByte4, last 12 elements are undefined!
NOALIAS __forceinline const Vec16U8 Load( MathsStorage::_UByte4 const * const __restrict pSrc )
{
    Vec16U8 result;
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

/// Vec16U8: Store first four elements into _UByte4
NOALIAS __forceinline void Store( const Vec16U8 PCREF data, MathsStorage::_UByte4 * const __restrict pDst )
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
    __stvebx( __vspltb( data.m_V, 0 ), &pDst->x, 0x0 );
    __stvebx( __vspltb( data.m_V, 1 ), &pDst->y, 0x0 );
    __stvebx( __vspltb( data.m_V, 2 ), &pDst->z, 0x0 );
    __stvebx( __vspltb( data.m_V, 3 ), &pDst->w, 0x0 );
    #elif defined( _SSE_ )
    _mm_maskmoveu_si128( data.m_V, (__m128i&)Const<__m128>::Mask<1,0,0,0>(), (char*)pDst );
    _mm_mfence();
    #else
    pDst->x = data.m_V[0];
    pDst->y = data.m_V[1];
    pDst->z = data.m_V[2];
    pDst->w = data.m_V[3];
    #endif
}


/// Vec16U8: Load first four elements from _UByteN4, last 12 elements are undefined!
NOALIAS __forceinline const Vec16U8 Load( MathsStorage::_UByteN4 const * const __restrict pSrc )
{
    Vec16U8 result;
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

/// Vec16U8: Store first four elements into _UByteN4
NOALIAS __forceinline void Store( const Vec16U8 PCREF data, MathsStorage::_UByteN4 * const __restrict pDst )
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
    __stvebx( __vspltb( data.m_V, 0 ), &pDst->x, 0x0 );
    __stvebx( __vspltb( data.m_V, 1 ), &pDst->y, 0x0 );
    __stvebx( __vspltb( data.m_V, 2 ), &pDst->z, 0x0 );
    __stvebx( __vspltb( data.m_V, 3 ), &pDst->w, 0x0 );
    #elif defined( _SSE_ )
    _mm_maskmoveu_si128( data.m_V, (__m128i&)Const<__m128>::Mask<1,0,0,0>(), (char*)pDst );
    _mm_mfence();
    #else
    pDst->x = data.m_V[0];
    pDst->y = data.m_V[1];
    pDst->z = data.m_V[2];
    pDst->w = data.m_V[3];
    #endif
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


