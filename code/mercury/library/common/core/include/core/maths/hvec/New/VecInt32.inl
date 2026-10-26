#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECINT32_INL
#define SOFT_MATHS_HVEC_NEW_VECINT32_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECINT32_INL
#define MATHS_HVEC_NEW_VECINT32_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup IntegerVector
/// @{

/// Make Vec4S32 from Vec4F32
/// WARNING: results are undefined for input F32's outside the range of an S32!
NOALIAS __forceinline const Vec4S32 MakeVec4S32( const Vec4F32 VECREF src )
{
    Vec4S32 result;
    #if defined( _SPU_ )
    result.m_V = spu_convts( src.m_V, 0 );
    #elif defined( _PPU_ )
    result.m_V = vec_vctsxs( src.m_V, 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vctsxs( src.m_V, 0 );
    #elif defined( _SSE_ )
    result.m_V = _mm_cvttps_epi32( src.m_V );
    #else
    result.m_V[0] = (S32)src.m_V[0];
    result.m_V[1] = (S32)src.m_V[1];
    result.m_V[2] = (S32)src.m_V[2];
    result.m_V[3] = (S32)src.m_V[3];
    #endif
    return result;
}


/// Pack two Vec4S32's into one Vec8S16 with Signed Saturation to [ -2^15, +2^15-1 ]
NOALIAS __forceinline const Vec8S16 PackSS( const Vec4S32 PCREF a, const Vec4S32 PCREF b )
{
    Vec8S16 result;
    #if defined( _SPU_ )
    result.m_V = spu_pack_ss( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vpkswss( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vpkswss( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_packs_epi32( a.m_V, b.m_V );
    #else
    result.m_V[0] = (S16)a.m_V[0];
    result.m_V[1] = (S16)a.m_V[1];
    result.m_V[2] = (S16)a.m_V[2];
    result.m_V[3] = (S16)a.m_V[3];
    result.m_V[4] = (S16)b.m_V[0];
    result.m_V[5] = (S16)b.m_V[1];
    result.m_V[6] = (S16)b.m_V[2];
    result.m_V[7] = (S16)b.m_V[3];
    #endif
    return result;
}

/// Pack two Vec4U32's into one Vec8U16 with Unsigned Saturation to [ 0, +2^16-1 ]
//NOALIAS __forceinline const Vec8U16 PackUS( const Vec4U32 PCREF a, const Vec4U32 PCREF b )
//{
//    Vec8U16 result;
//    #if defined( _SPU_ )
//    assert( 0 && "Not implemented yet!\n" );
//    #elif defined( _PPU_ )
//    result.m_V = vec_vpkuwus( a.m_V, b.m_V );
//    #elif defined( _VMX128_ )
//    result.m_V = __vpkuwus( a.m_V, b.m_V );
//    #elif defined( _SSE_ )
//    assert( 0 && "Not implemented yet!\n" );
//    #else
//    assert( 0 && "Not implemented yet!\n" );
//    #endif
//    return result;
//}

/// Unpack Vec8S16 into two Vec4S32's
NOALIAS __forceinline void Unpack( const Vec8S16 PCREF a, Vec4S32* const __restrict pB, Vec4S32* const __restrict pC )
{
    #if defined( _SPU_ )
    pB->m_V = spu_unpack_hi( a.m_V );
    pC->m_V = spu_unpack_lo( a.m_V );
    #elif defined( _PPU_ )
    pB->m_V = vec_vupkhsh( a.m_V );
    pC->m_V = vec_vupklsh( a.m_V );
    #elif defined( _VMX128_ )
    pB->m_V = __vupkhsh( a.m_V );
    pC->m_V = __vupklsh( a.m_V );
    #elif defined( _SSE_ )
    pB->m_V = _mm_srai_epi32( _mm_unpacklo_epi16( a.m_V, a.m_V ), 16 );
    pC->m_V = _mm_srai_epi32( _mm_unpackhi_epi16( a.m_V, a.m_V ), 16 );
    #else
    pB->m_V[0] = (S32)a.m_V[0];
    pB->m_V[1] = (S32)a.m_V[1];
    pB->m_V[2] = (S32)a.m_V[2];
    pB->m_V[3] = (S32)a.m_V[3];
    pC->m_V[0] = (S32)a.m_V[4];
    pC->m_V[1] = (S32)a.m_V[5];
    pC->m_V[2] = (S32)a.m_V[6];
    pC->m_V[3] = (S32)a.m_V[7];
    #endif
}

/// Unpack Vec8U16 into two Vec4S32's
//NOALIAS __forceinline void Unpack( const Vec8U16 PCREF a, Vec4S32* const __restrict pB, Vec4S32* const __restrict pC )
//{
//    #if defined( _SPU_ )
//    assert( 0 && "Not implemented yet!\n" );
//    #elif defined( _PPU_ )
//    const vector unsigned short zero( Const< vector unsigned short >::Zero() );
//    pB->m_V = __vmrghh( zero, a.m_V );
//    pC->m_V = __vmrglh( zero, a.m_V );
//    #elif defined( _VMX128_ )
//    const __vector4 zero( Const<__vector4>::Zero() );
//    pB->m_V = __vmrghh( zero, a.m_V );
//    pC->m_V = __vmrglh( zero, a.m_V );
//    #elif defined( _SSE_ )
//    assert( 0 && "Not implemented yet!\n" );
//    #else
//    assert( 0 && "Not implemented yet!\n" );
//    #endif
//}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


