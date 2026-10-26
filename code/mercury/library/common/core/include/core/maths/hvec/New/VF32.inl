#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VF32_INL
#define SOFT_MATHS_HVEC_NEW_VF32_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VF32_INL
#define MATHS_HVEC_NEW_VF32_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup VF32
/// @{

/// Load VF32 from an F32
template <> NOALIAS __forceinline const VF32 Load( F32 const * const __restrict pSrc )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( *pSrc );
    #elif defined( _PPU_ )
    //result.m_V = vec_splats( *pSrc );
    result.m_V = vec_splat( vec_lvlx( 0x0, pSrc ), 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( pSrc, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( *pSrc );
    #else
    result.m_V = *pSrc;
    #endif
    return result;
}

/// Store VF32 into an F32
NOALIAS __forceinline void Store( const VF32 VECREF src, F32* const __restrict pDst )
{
    #if defined( _SPU_ )
    *pDst = spu_extract( src.m_V, VX );
    #elif defined( _PPU_ )
    vec_ste( src.m_V, 0x0, pDst );
    #elif defined( _VMX128_ )
    __stvewx( src.m_V, pDst, 0x0 );
    #elif defined( _SSE_ )
    _mm_store_ss( pDst, src.m_V );
    #else
    *pDst = src.m_V;
    #endif
}

/// StorageVF32: Load VF32
NOALIAS __forceinline const VF32 Load( StorageVF32 const * const __restrict pSrc )
{
    return Load< VF32 >( &pSrc->m_Data );
}

/// StorageVF32: Store VF32
NOALIAS __forceinline void Store( const VF32 VECREF src, StorageVF32 * const __restrict pDst )
{
    Store( src, &pDst->m_Data );
}



/// Make VF32 from F32
NOALIAS __forceinline const VF32 MakeVF32( const F32& x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( x );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( vec_load_left_aligned( &x ), VX );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( &x, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( x );
    #else
    result.m_V = x;
    #endif
    return result;
}

/// Make VF32 from F64
NOALIAS __forceinline const VF32 MakeVF32( const F64& x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( (F32)x );
    #elif defined( _PPU_ )
    const F32 temp = (F32)x;
    result.m_V = vec_splat( vec_load_left_aligned( &temp ), VX );
    #elif defined( _VMX128_ )
    const F32 temp = (F32)x;
    result.m_V = __vspltw( __lvlx( &temp, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( (F32)x );
    #else
    result.m_V = (F32)x;
    #endif
    return result;
}

/// Make VF32 from S32
NOALIAS __forceinline const VF32 MakeVF32( const S32& x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_convtf( spu_splats( x ), 0 );
    #elif defined( _PPU_ )
    result.m_V = vec_vcfsx( vec_splat( vec_load_left_aligned( &x ), VX ), 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vcfsx( __vspltw( __lvlx( &x, 0x0 ), VX ), 0 );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( (F32)x );
    #else
    result.m_V = (F32)x;
    #endif
    return result;
}

/// Make VF32 from U32
NOALIAS __forceinline const VF32 MakeVF32( const U32& x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_convtf( spu_splats( x ), 0 );
    #elif defined( _PPU_ )
    result.m_V = vec_vcfux( vec_splat( vec_load_left_aligned( &x ), VX ), 0 );
    #elif defined( _VMX128_ )
    result.m_V = __vcfux( __vspltw( __lvlx( &x, 0x0 ), VX ), 0 );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( (F32)x );
    #else
    result.m_V = (F32)x;
    #endif
    return result;
}

/// Make VF32 from F32 pointer
NOALIAS __forceinline const VF32 MakeVF32( F32 const * const __restrict ptr )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( *ptr );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( vec_load_left_aligned( ptr ), VX );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( ptr, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_ps( *ptr );
    #else
    result.m_V = *ptr;
    #endif
    return result;
}

/// Convert VF32 to F32
NOALIAS __forceinline const F32 GetF32( const VF32 VECREF x )
{
    F32 F32_from_VF32;
    #if defined( _SPU_ )
    F32_from_VF32 = spu_extract( x.m_V, VX ); // extract any element, they're all the same
    #elif defined( _PPU_ )
    vec_stvewx( x.m_V, 0x0, &F32_from_VF32 );
    #elif defined( _VMX128_ )
    __stvewx( x.m_V, &F32_from_VF32, 0x0 );
    #elif defined( _SSE_ )
    _mm_store_ss( &F32_from_VF32, x.m_V );
    #else
    F32_from_VF32 = x.m_V;
    #endif
    return F32_from_VF32;
}

/// Convert VF32 to int S32
NOALIAS __forceinline const S32 GetS32( const VF32 VECREF x )
{
    S32 S32_from_Real32;
    #if defined( _SPU_ )
    S32_from_Real32 = spu_extract( spu_convts( x.m_V, 0 ), VX );
    #elif defined( _PPU_ )
    vec_stvewx( vec_vctsxs( x.m_V, 0 ), 0x0, &S32_from_Real32 );
    #elif defined( _VMX128_ )
    __stvewx( __vctsxs( x.m_V, 0 ), &S32_from_Real32, 0x0 );
    #elif defined( _SSE_ )
    S32_from_Real32 = _mm_cvttss_si32( x.m_V );
    #else
    S32_from_Real32 = (S32)x.m_V;
    #endif
    return S32_from_Real32;
}



/// VF32 Write to Stream
template < typename T_Stream > void Serialise( const VF32 VECREF src, T_Stream& stream )
{
    stream << GetF32( src );
}

/// VF32 Read from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, VF32* const __restrict pDst )
{
    F32 x;
    stream >> x;
    *pDst = MakeVF32( x );
}



/// VF32 Abs
NOALIAS __forceinline const VF32 Abs( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_abs( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_abs( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, Const< __vector4 >::AbsMask() );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_ps( x.m_V, Const< __m128 >::AbsMask() );
    #else
    result.m_V = Maths::Abs( x.m_V );
    #endif
    return result;
}

/// VF32 Sign : Returns -1.0f if scalar < 0.0f, +1.0f if scalar > 0.0f, or 0.0f if scalar == 0.0f 
NOALIAS __forceinline const VF32 Sign( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SPU_ )
    const vector float zero( Const< vector float >::Zero() );
    const vector float one( Const< vector float >::One() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    result.m_V = spu_sel( (vector float)spu_andc( one, (vector float)spu_cmpeq( x.m_V, zero ) ), x.m_V, sign_mask );
    #elif defined( _PPU_ )
    const vector float zero( Const< vector float >::Zero() );
    const vector float one( Const< vector float >::One() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    result.m_V = vec_sel( (vector float)vec_andc( one, (vector float)vec_vcmpeqfp( x.m_V, zero ) ), x.m_V, sign_mask );
    #elif defined( _VMX128_ )
    const __vector4 zero( Const< __vector4 >::Zero() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 sign_mask( Const< __vector4 >::SignMask() );
    result.m_V = __vsel( __vandc( one, __vcmpeqfp( x.m_V, zero ) ), x.m_V, sign_mask );
    #elif defined( _SSE_ )
    const __m128 zero( _mm_setzero_ps() );
    const __m128 one( Const< __m128 >::One() );
    const __m128 sign_mask( Const< __m128 >::SignMask() );
    const __m128 signed_one( _mm_or_ps( _mm_and_ps( x.m_V, sign_mask ), one ) );
    //const __m128 not_zero( _mm_cmpneq_ps( _mm_andnot_ps( sign_mask, v.m_V ), zero ) );
    const __m128 not_zero( _mm_cmpneq_ps( x.m_V, zero ) );
    result.m_V = _mm_and_ps( signed_one, not_zero );
    #else
    result.m_V = ( x.m_V < 0.0f ? -1.0f : ( x.m_V > 0.0f ? 1.0f : 0.0f ) );
    #endif
    return result;
}

/// VF32 CopySign : Returns a VF32 with it's absolute value taken from 'magnitude' and it's sign bit from 'sign'
NOALIAS __forceinline const VF32 CopySign( const VF32 VECREF magnitude, const VF32 VECREF sign )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( magnitude.m_V, sign.m_V, Const< vector unsigned int >::SignMask() );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( magnitude.m_V, sign.m_V, Const< vector unsigned int >::SignMask() );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( magnitude.m_V, sign.m_V, Const<__vector4>::SignMask() );
    #elif defined( _SSE_ )
    //result.m_V = _mm_or_ps( _mm_and_ps( Const<__m128>::AbsMask(), magnitude.m_V ),
    //                        _mm_and_ps( Const<__m128>::SignMask(), sign.m_V ) );
    result.m_V = sse_select( magnitude.m_V, sign.m_V, Const<__m128>::SignMask() );
    #else
    result.m_V = (F32)_copysign( magnitude.m_V, sign.m_V );
    #endif
    return result;
}

/// VF32 XorSign : Returns a VF32 with it's absolute value taken from 'magnitude' and it's sign bit from the XOR of the sign bits of 'sign' and 'magnitude'
NOALIAS __forceinline const VF32 XorSign( const VF32 VECREF magnitude, const VF32 VECREF sign )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( magnitude.m_V, spu_xor( magnitude.m_V, sign.m_V ), Const< vector unsigned int >::SignMask() );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( magnitude.m_V, vec_xor( magnitude.m_V, sign.m_V ), Const< vector unsigned int >::SignMask() );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( magnitude.m_V, __vxor( magnitude.m_V, sign.m_V ), Const<__vector4>::SignMask() );
    #elif defined( _SSE_ )
    //result.m_V = _mm_or_ps( _mm_and_ps( Const<__m128>::AbsMask(), magnitude.m_V ),
    //                        _mm_and_ps( Const<__m128>::SignMask(), _mm_xor_ps( magnitude.m_V, sign.m_V ) ) );
    result.m_V = sse_select( magnitude.m_V, _mm_xor_ps( magnitude.m_V, sign.m_V ), Const<__m128>::SignMask() );
    #else
    result.m_V = (F32)_copysign( magnitude.m_V, magnitude.m_V * sign.m_V );
    #endif
    return result;
}


/// VF32 Reciprocal (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Rcp( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Rcp<22>( x.m_V );
    #else
    result.m_V = Maths::Reciprocal( x.m_V );
    #endif
    return result;
}

/// VF32 Reciprocal (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Rcp24( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Rcp<24>( x.m_V );
    #else
    result.m_V = Maths::Reciprocal( x.m_V );
    #endif
    return result;
}

/// VF32 Inverse (Reciprocal) (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Inverse( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Rcp<22>( x.m_V );
    #else
    result.m_V = Maths::Reciprocal( x.m_V );
    #endif
    return result;
}

/// VF32 Inverse (Reciprocal) (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Inverse24( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Rcp<24>( x.m_V );
    #else
    result.m_V = Maths::Reciprocal( x.m_V );
    #endif
    return result;
}

/// VF32 Division (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Div( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Div<22>( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V * Maths::Reciprocal( y.m_V );
    #endif
    return result;
}

/// VF32 Division (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Div24( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Div<24>( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V * Maths::Reciprocal( y.m_V );
    #endif
    return result;
}

/// VF32 Reciprocal Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 RcpSqrt( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_RcpSqrt<22>( x.m_V );
    #else
    result.m_V = Maths::ReciprocalSqrt( x.m_V );
    #endif
    return result;
}

/// VF32 Reciprocal Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 RcpSqrt24( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_RcpSqrt<24>( x.m_V );
    #else
    result.m_V = Maths::ReciprocalSqrt( x.m_V );
    #endif
    return result;
}

/// VF32 Square Root (faster but only 22 bit precision)
NOALIAS __forceinline const VF32 Sqrt( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Sqrt<22>( x.m_V );
    #else
    result.m_V = Maths::Sqrt( x.m_V );
    #endif
    return result;
}

/// VF32 Square Root (slower but full 24 bit precision)
NOALIAS __forceinline const VF32 Sqrt24( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Sqrt<24>( x.m_V );
    #else
    result.m_V = Maths::Sqrt( x.m_V );
    #endif
    return result;
}

/// VF32 Square Root Estimate (very fast but only 11 bit precision)
NOALIAS __forceinline const VF32 SqrtEst( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Sqrt<11>( x.m_V );
    #else
    result.m_V = Maths::Sqrt( x.m_V );
    #endif
    return result;
}

/// VF32 Reciprocals And Square Root : Returns 1/x, sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void RcpAndSqrt( const VF32 VECREF x,
                                       VF32* const __restrict pRcp,
                                       VF32* const __restrict pSqrt,
                                       VF32* const __restrict pRcpSqrt )
{
    #if defined( _SIMD_ )
    const V128 rcp_x( SIMD_Rcp<22>( x.m_V ) );
    const V128 rcp_sqrt_x( SIMD_RcpSqrt<22>( x.m_V ) );
    pRcp->m_V     = rcp_x;
    pRcpSqrt->m_V = rcp_sqrt_x;
    #if defined( _SPU_ )
    pSqrt->m_V = spu_mul( x.m_V, rcp_sqrt_x );
    #elif defined( _PPU_ )
    pSqrt->m_V = vec_mul( x.m_V, rcp_sqrt_x );
    #elif defined( _VMX128_ )
    pSqrt->m_V = __vmulfp( x.m_V, rcp_sqrt_x );
    #elif defined( _SSE_ )
    pSqrt->m_V = _mm_mul_ps( x.m_V, rcp_sqrt_x );
    #endif
    #else
    pRcp->m_V     = Maths::Reciprocal( x.m_V );
    pSqrt->m_V    = Maths::Sqrt( x.m_V );
    pRcpSqrt->m_V = Maths::ReciprocalSqrt( x.m_V );
    #endif
}

/// VF32 Reciprocals And Square Root : Returns 1/x, sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void RcpAndSqrt24( const VF32 VECREF x,
                                         VF32* const __restrict pRcp,
                                         VF32* const __restrict pSqrt,
                                         VF32* const __restrict pRcpSqrt )
{
    #if defined( _SIMD_ )
    const V128 rcp_x( SIMD_Rcp<24>( x.m_V ) );
    const V128 rcp_sqrt_x( SIMD_RcpSqrt<24>( x.m_V ) );
    pRcp->m_V     = rcp_x;
    pRcpSqrt->m_V = rcp_sqrt_x;
    #if defined( _SPU_ )
    pSqrt->m_V = spu_mul( x.m_V, rcp_sqrt_x );
    #elif defined( _PPU_ )
    pSqrt->m_V = vec_mul( x.m_V, rcp_sqrt_x );
    #elif defined( _VMX128_ )
    pSqrt->m_V = __vmulfp( x.m_V, rcp_sqrt_x );
    #elif defined( _SSE_ )
    pSqrt->m_V = _mm_mul_ps( x.m_V, rcp_sqrt_x );
    #endif
    #else
    pRcp->m_V     = Maths::Reciprocal( x.m_V );
    pSqrt->m_V    = Maths::Sqrt( x.m_V );
    pRcpSqrt->m_V = Maths::ReciprocalSqrt( x.m_V );
    #endif
}


/// VF32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (faster but only 22 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt( const VF32 VECREF x,
                                           VF32* const __restrict pSqrt,
                                           VF32* const __restrict pRcpSqrt )
{
    #if defined( _SIMD_ )
    const V128 rcp_sqrt_x( SIMD_RcpSqrt<22>( x.m_V ) );
    pRcpSqrt->m_V = rcp_sqrt_x;
    #if defined( _SPU_ )
    pSqrt->m_V = spu_mul( x.m_V, rcp_sqrt_x );
    #elif defined( _PPU_ )
    pSqrt->m_V = vec_mul( x.m_V, rcp_sqrt_x );
    #elif defined( _VMX128_ )
    pSqrt->m_V = __vmulfp( x.m_V, rcp_sqrt_x );
    #elif defined( _SSE_ )
    pSqrt->m_V = _mm_mul_ps( x.m_V, rcp_sqrt_x );
    #endif
    #else
    pSqrt->m_V    = Maths::Sqrt( x.m_V );
    pRcpSqrt->m_V = Maths::ReciprocalSqrt( x.m_V );
    #endif
}

/// VF32 Square Root And Reciprocal Square Root: Returns sqrt(x) & 1/sqrt(x) (slower but full 24 bit precision)
NOALIAS __forceinline void SqrtAndRcpSqrt24( const VF32 VECREF x,
                                             VF32* const __restrict pSqrt,
                                             VF32* const __restrict pRcpSqrt )
{
    #if defined( _SIMD_ )
    const V128 rcp_sqrt_x( SIMD_RcpSqrt<24>( x.m_V ) );
    pRcpSqrt->m_V = rcp_sqrt_x;
    #if defined( _SPU_ )
    pSqrt->m_V = spu_mul( x.m_V, rcp_sqrt_x );
    #elif defined( _PPU_ )
    pSqrt->m_V = vec_mul( x.m_V, rcp_sqrt_x );
    #elif defined( _VMX128_ )
    pSqrt->m_V = __vmulfp( x.m_V, rcp_sqrt_x );
    #elif defined( _SSE_ )
    pSqrt->m_V = _mm_mul_ps( x.m_V, rcp_sqrt_x );
    #endif
    #else
    pSqrt->m_V    = Maths::Sqrt( x.m_V );
    pRcpSqrt->m_V = Maths::ReciprocalSqrt( x.m_V );
    #endif
}


/// VF32 Quadratic Equation Solver
NOALIAS __forceinline const VBool SolveQuadratic( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, VF32* const __restrict pRoot1, VF32* const __restrict pRoot2 )
{
    // http://mathworld.wolfram.com/QuadraticFormula.html
    
    const VF32  zero( Const<VF32>::Zero() );
    const VF32  four( Const<VF32>::Four() );
    const VF32  epsilon( Const<VF32>::Epsilon() );
    const VF32  error( -Const<VF32>::Max() );
    const VF32  square( ( b * b ) - ( four * a * c ) );
    const VBool valid( ( square >= zero ) & Abs( a ) > epsilon );
    const VF32  root( Sqrt( square ) );
    const VF32  rcp2a( Rcp( a + a ) );
    const VF32  t0( (  root - b ) * rcp2a );
    const VF32  t1( ( -root - b ) * rcp2a );
    
    *pRoot1 = Select( valid, t0, error );
    *pRoot2 = Select( valid, t1, error );
    
    return valid;
}

/// VF32 Quadratic Equation Solver
NOALIAS __forceinline const QuadraticResult SolveQuadratic( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c )
{
    // http://mathworld.wolfram.com/QuadraticFormula.html
    
    const VF32  zero( Const<VF32>::Zero() );
    const VF32  four( Const<VF32>::Four() );
    const VF32  epsilon( Const<VF32>::Epsilon() );
    const VF32  error( -Const<VF32>::Max() );
    const VF32  square( ( b * b ) - ( four * a * c ) );
    const VBool valid( ( square >= zero ) & Abs( a ) > epsilon );
    const VF32  root( Sqrt( square ) );
    const VF32  rcp2a( Rcp( a + a ) );
    const VF32  t0( (  root - b ) * rcp2a );
    const VF32  t1( ( -root - b ) * rcp2a );
    
    QuadraticResult result;
    result.m_X1 = Select( valid, t0, error );
    result.m_X2 = Select( valid, t1, error );
    return result;
}



/// VF32 Unary Positive Operator
NOALIAS __forceinline const VF32 operator + ( const VF32 VECREF x )
{
    return x;
}

/// VF32 Unary Negation Operator
NOALIAS __forceinline const VF32 operator - ( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_negate( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, Const< vector float >::SignMask() );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, Const< __vector4 >::SignMask() );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_ps( x.m_V, Const< __m128 >::SignMask() );
    #else
    result.m_V = -x.m_V;
    #endif
    return result;
}

/// VF32 Binary Addition Operator
NOALIAS __forceinline const VF32 operator + ( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_add( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_add( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vaddfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V + y.m_V;
    #endif
    return result;
}

/// VF32 Binary Subtraction Operator
NOALIAS __forceinline const VF32 operator - ( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sub( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sub( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsubfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V - y.m_V;
    #endif
    return result;
}

/// VF32 Binary Multiplication Operator
NOALIAS __forceinline const VF32 operator * ( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V * y.m_V;
    #endif
    return result;
}

/// VF32 Binary Division Operator
NOALIAS __forceinline const VF32 operator / ( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_Div<22>( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V * Maths::Reciprocal( y.m_V );
    #endif
    return result;
}

/// VF32 Add Assign Operator
NOALIAS __forceinline const VF32& operator += ( VF32& x, const VF32 VECREF y )
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
    x.m_V += y.m_V;
    #endif
    return x;
}

/// VF32 Subtract Assign Operator
NOALIAS __forceinline const VF32& operator -= ( VF32& x, const VF32 VECREF y )
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
    x.m_V -= y.m_V;
    #endif
    return x;
}

/// VF32 Multiply Assign Operator
NOALIAS __forceinline const VF32& operator *= ( VF32& x, const VF32 VECREF y )
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
    x.m_V *= y.m_V;
    #endif
    return x;
}

/// VF32 Divide Assign Operator
NOALIAS __forceinline const VF32& operator /= ( VF32& x, const VF32 VECREF y )
{
    #if defined( _SIMD_ )
    x.m_V = SIMD_Div<22>( x.m_V, y.m_V );
    #else
    x.m_V *= Maths::Reciprocal( y.m_V );
    #endif
    return x;
}


/// VF32 Square
NOALIAS __forceinline const VF32 Square( const VF32 VECREF a )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_mul( a.m_V, a.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_mul( a.m_V, a.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmulfp( a.m_V, a.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_mul_ps( a.m_V, a.m_V );
    #else
    result.m_V = a.m_V * a.m_V;
    #endif
    return result;
}


/// VF32 MulAdd : a * b + c
NOALIAS __forceinline const VF32 MulAdd( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_madd( a.m_V, b.m_V, c.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_madd( a.m_V, b.m_V, c.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmaddfp( a.m_V, b.m_V, c.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_ps( _mm_mul_ps( a.m_V, b.m_V ), c.m_V );
    #else
    result.m_V = a.m_V * b.m_V + c.m_V;
    #endif
    return result;
}

/// VF32 MulSub : a * b - c
NOALIAS __forceinline const VF32 MulSub( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_msub( a.m_V, b.m_V, c.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_madd( a.m_V, b.m_V, vec_xor( c.m_V, Const< vector float >::SignMask() ) );
    #elif defined( _VMX128_ )
    result.m_V = __vmaddfp( a.m_V, b.m_V, __vxor( c.m_V, Const<__vector4>::SignMask() ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_ps( _mm_mul_ps( a.m_V, b.m_V ), c.m_V );
    #else
    result.m_V = a.m_V * b.m_V - c.m_V;
    #endif
    return result;
}

/// VF32 NegMulAdd : -( a * b + c )
NOALIAS __forceinline const VF32 NegMulAdd( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( spu_madd( a.m_V, b.m_V, c.m_V ), Const< vector float >::SignMask() );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( vec_madd( a.m_V, b.m_V, c.m_V ), Const< vector float >::SignMask() );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( __vmaddfp( a.m_V, b.m_V, c.m_V ), Const<__vector4>::SignMask() );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_ps( _mm_xor_ps( c.m_V, Const<__m128>::SignMask() ), _mm_mul_ps( a.m_V, b.m_V ) );
    #else
    //result.m_V = -( a.m_V * b.m_V + c.m_V );
    result.m_V = -a.m_V * b.m_V - c.m_V;
    #endif
    return result;
}

/// VF32 NegMulSub : -( a * b - c ) == c - a * b
NOALIAS __forceinline const VF32 NegMulSub( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_nmsub( a.m_V, b.m_V, c.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_nmsub( a.m_V, b.m_V, c.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vnmsubfp( a.m_V, b.m_V, c.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_ps( c.m_V, _mm_mul_ps( a.m_V, b.m_V ) );
    #else
    result.m_V = c.m_V - ( a.m_V * b.m_V );
    #endif
    return result;
}



/// VF32 Min
NOALIAS __forceinline const VF32 Min( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_min( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_min( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vminfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_min_ps( x.m_V, y.m_V );
    #else
    result.m_V = Maths::Min( x.m_V, y.m_V );
    #endif
    return result;
}

/// VF32 Max
NOALIAS __forceinline const VF32 Max( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_max( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_max( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vmaxfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_max_ps( x.m_V, y.m_V );
    #else
    result.m_V = Maths::Max( x.m_V, y.m_V );
    #endif
    return result;
}

/// VF32 Min Assign
DEPRECATED NOALIAS __forceinline const VF32& MinEq( VF32* const __restrict pX, const VF32 VECREF y )
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
    pX->m_V = Maths::Min( pX->m_V, y.m_V );
    #endif
    return *pX;
}

/// VF32 Max Assign
DEPRECATED NOALIAS __forceinline const VF32& MaxEq( VF32* const __restrict pX, const VF32 VECREF y )
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
    pX->m_V = Maths::Max( pX->m_V, y.m_V );
    #endif
    return *pX;
}

/// VF32 Min
NOALIAS __forceinline void Min( VF32* const __restrict pX, const VF32 VECREF y )
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
    pX->m_V = Maths::Min( pX->m_V, y.m_V );
    #endif
}

/// VF32 Max
NOALIAS __forceinline void Max( VF32* const __restrict pX, const VF32 VECREF y )
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
    pX->m_V = Maths::Max( pX->m_V, y.m_V );
    #endif
}


/// VF32 Clamp
NOALIAS __forceinline const VF32 Clamp( const VF32 VECREF x, const VF32 VECREF lo, const VF32 VECREF hi )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_min( spu_max( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_min( vec_vmaxfp( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vminfp( __vmaxfp( x.m_V, lo.m_V ), hi.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_min_ps( _mm_max_ps( x.m_V, lo.m_V ), hi.m_V );
    #else
    result.m_V = Maths::Min( Maths::Max( x.m_V, lo.m_V ), hi.m_V );
    #endif
    return result;
}

/// VF32 Clamp Assign
DEPRECATED NOALIAS __forceinline const VF32& ClampEq( VF32* const __restrict pX, const VF32 VECREF lo, const VF32 VECREF hi )
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
    pX->m_V = Maths::Min( Maths::Max( pX->m_V, lo.m_V ), hi.m_V );
    #endif
    return *pX;
}

/// VF32 Clamp
NOALIAS __forceinline void Clamp( VF32* const __restrict pX, const VF32 VECREF lo, const VF32 VECREF hi )
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
    pX->m_V = Maths::Min( Maths::Max( pX->m_V, lo.m_V ), hi.m_V );
    #endif
}



/// VF32 Round to integer towards minus infinity
NOALIAS __forceinline const VF32 Floor( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rfim( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vrfim( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrfim( x.m_V );
    #elif defined( _SSE_ )
    result.m_V = SSE_RoundToIntMinusInf( x.m_V );
    #else
    result.m_V = floorf( x.m_V );
    #endif
    return result;
}

/// VF32 Round to integer towards plus infinity
NOALIAS __forceinline const VF32 Ceil( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rfip( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vrfip( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrfip( x.m_V );
    #elif defined( _SSE_ )
    result.m_V = SSE_RoundToIntPlusInf( x.m_V );
    #else
    result.m_V = ceilf( x.m_V );
    #endif
    return result;
}

/// VF32 Round to integer towards nearest
NOALIAS __forceinline const VF32 Round( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rfin( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vrfin( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrfin( x.m_V );
    #elif defined( _SSE_ )
    result.m_V = SSE_RoundToIntNearest( x.m_V );
    #else
    result.m_V = (F32)_copysign( floorf( fabsf( x.m_V ) + 0.5f ), x.m_V );
    #endif
    return result;
}

/// VF32 Round to integer towards zero
NOALIAS __forceinline const VF32 Trunc( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_rfiz( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vrfiz( x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vrfiz( x.m_V );
    #elif defined( _SSE_ )
    result.m_V = SSE_RoundToIntZero( x.m_V );
    #else
    //result.m_V = (F32)(S32)x.m_V;
    result.m_V = (F32)(S64)x.m_V;
    #endif
    return result;
}


/// VF32 Flush to Zero: flushes value to zero if less than or equal to tolerance
NOALIAS __forceinline const VF32 FlushToZero( const VF32 VECREF value, const VF32 VECREF tolerance )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_and( value.m_V, (vector float)spu_cmpabsgt( value.m_V, tolerance.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_and( value.m_V, vec_cmpgt( vec_abs( value.m_V ), tolerance.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vand( value.m_V, __vcmpgtfp( vmx_abs( value.m_V ), tolerance.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_ps( value.m_V, _mm_cmpgt_ps( sse_abs( value.m_V ), tolerance.m_V ) );
    #else
    result.m_V = Abs( value.m_V ) > tolerance.m_V ? value.m_V : 0.0f;
    #endif
    return result;
}




/// VF32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x and |r| < |y|
NOALIAS __forceinline const VF32 Mod( const VF32 VECREF x, const VF32 VECREF y )
{
    return NegMulSub( Trunc( x / y ), y, x ); // x - Trunc( x / y ) * y
}

/// VF32 Modulus : Returns remainder of x/y, r = x - i.y [ x = i.y + r ], where i is an integer, r has same sign as x, |r| < |y| and y * rcp_y = 1
NOALIAS __forceinline const VF32 Mod( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF rcp_y )
{
    return NegMulSub( Trunc( x * rcp_y ), y, x );
}

/// VF32 Fractional Part : Returns fractional part of x, f = x - i [ x = i + f ], where i is an integer, i & f have the same sign as x
NOALIAS __forceinline const VF32 Frac( const VF32 VECREF x )
{
    return x - Trunc( x );
}

/// VF32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ] e.g. [0,2pi]
NOALIAS __forceinline const VF32 WrapToInterval( const VF32 VECREF x, const VF32 VECREF p )
{
    return NegMulSub( Floor( x / p ), p, x ); // x - ( Floor( x / p ) * p )
}

/// VF32 Wrap to Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ 0, p ], where p * rcp_p = 1 : e.g. [0,2pi] : 
NOALIAS __forceinline const VF32 WrapToInterval( const VF32 VECREF x, const VF32 VECREF p, const VF32 VECREF rcp_p )
{
    return NegMulSub( Floor( x * rcp_p ), p, x ); // x - ( Floor( x * rcp_p ) * p )
}

/// VF32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ] e.g. [-pi,+pi]
NOALIAS __forceinline const VF32 WrapToIntervalOffset( const VF32 VECREF x, const VF32 VECREF p, const VF32 VECREF o )
{
    return NegMulSub( Floor( ( x + o ) / p ), p, x ); // x - ( Floor( ( x + o ) / p ) * p )
}

/// VF32 Wrap to Offset Periodic Interval : For periodic functions f(x), wraps x to the smallest interval [ -o, p - o ], where p * rcp_p = 1 : e.g. [-pi,+pi]
NOALIAS __forceinline const VF32 WrapToIntervalOffset( const VF32 VECREF x, const VF32 VECREF p, const VF32 VECREF rcp_p, const VF32 VECREF o )
{
    return NegMulSub( Floor( ( x + o ) * rcp_p ), p, x ); // x - ( Floor( ( x + o ) * rcp_p ) * p )
}



/// VF32 Mean
NOALIAS __forceinline const VF32 Mean( const VF32 VECREF a, const VF32 VECREF b )
{
    return ( a + b ) * Const<VF32>::Half();
}

/// VF32 Mean
NOALIAS __forceinline const VF32 Mean( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c )
{
    return ( a + b + c ) * Const<VF32>::Third();
}

/// VF32 Mean
NOALIAS __forceinline const VF32 Mean( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, const VF32 VECREF d )
{
    return ( a + b + c + d ) * Const<VF32>::Quarter();
}

/// VF32 Mean
NOALIAS __forceinline const VF32 Mean( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, const VF32 VECREF d,
                                       const VF32 VECREF e, const VF32 VECREF f, const VF32 VECREF g, const VF32 VECREF h )
{
    return ( a + b + c + d + e + f + g + h ) * Const<VF32>::Frac<1,8>();
}


/// VF32 RMS: Root Mean Square
NOALIAS __forceinline const VF32 RMS( const VF32 VECREF a, const VF32 VECREF b )
{
    return Sqrt( Mean( Square(a), Square(b) ) );
}

/// VF32 RMS: Root Mean Square
NOALIAS __forceinline const VF32 RMS( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c )
{
    return Sqrt( Mean( Square(a), Square(b), Square(c) ) );
}

/// VF32 RMS: Root Mean Square
NOALIAS __forceinline const VF32 RMS( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, const VF32 VECREF d )
{
    return Sqrt( Mean( Square(a), Square(b), Square(c), Square(d) ) );
}

/// VF32 RMS: Root Mean Square
NOALIAS __forceinline const VF32 RMS( const VF32 VECREF a, const VF32 VECREF b, const VF32 VECREF c, const VF32 VECREF d,
                                      const VF32 VECREF e, const VF32 VECREF f, const VF32 VECREF g, const VF32 VECREF h )
{
    return Sqrt( Mean( Square(a), Square(b), Square(c), Square(d), Square(e), Square(f), Square(g), Square(h) ) );
}


/// VF32 Comparison x == y : Returns VBool representing results of x == y
NOALIAS __forceinline const VBool operator == ( const VF32 VECREF x, const VF32 VECREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpeq( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpeq( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpeqfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpeq_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V == y.m_V;
    #endif
    return result;
}

/// VF32 Comparison x != y : Returns VBool representing results of x != y
NOALIAS __forceinline const VBool operator != ( const VF32 VECREF x, const VF32 VECREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_not( spu_cmpeq( x.m_V, y.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_not( vec_cmpeq( x.m_V, y.m_V ) );
    #elif defined( _VMX128_ )
    const __vector4 eq( __vcmpeqfp( x.m_V, y.m_V ) );
    result.m_V = __vnor( eq, eq );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpneq_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V != y.m_V;
    #endif
    return result;
}

/// VF32 Comparison x <= y : Returns VBool representing results of x <= y
NOALIAS __forceinline const VBool operator <= ( const VF32 VECREF x, const VF32 VECREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpge( y.m_V, x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpge( y.m_V, x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpgefp( y.m_V, x.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmple_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V <= y.m_V;
    #endif
    return result;
}

/// VF32 Comparison x >= y : Returns VBool representing results of x >= y
NOALIAS __forceinline const VBool operator >= ( const VF32 VECREF x, const VF32 VECREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpge( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpge( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpgefp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpge_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V >= y.m_V;
    #endif
    return result;
}

/// VF32 Comparison x < y : Returns VBool representing results of x < y
NOALIAS __forceinline const VBool operator < ( const VF32 VECREF x, const VF32 VECREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpgt( y.m_V, x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpgt( y.m_V, x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpgtfp( y.m_V, x.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmplt_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V < y.m_V;
    #endif
    return result;
}

/// VF32 Comparison x > y : Returns VBool representing results of x > y
NOALIAS __forceinline const VBool operator > ( const VF32 VECREF x, const VF32 VECREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_cmpgt( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_cmpgt( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vcmpgtfp( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpgt_ps( x.m_V, y.m_V );
    #else
    result.m_V = x.m_V > y.m_V;
    #endif
    return result;
}

/// VF32 Ordered Test ( true if neither x nor y is a NaN )
NOALIAS __forceinline const VBool Ordered( const VF32 VECREF x, const VF32 VECREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::True(); // SPU doesn't support vector float NaN's
    #elif defined( _PPU_ )
    result.m_V = vec_or( vec_cmpge( x.m_V, y.m_V ), vec_cmpge( y.m_V, x.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vor( __vcmpgefp( x.m_V, y.m_V ), __vcmpgefp( y.m_V, x.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpord_ps( x.m_V, y.m_V );
    #else
    result.m_V = !_isnan( x.m_V ) && !_isnan( y.m_V );
    #endif
    return result;
}

/// VF32 Unordered Test ( true if x or y is a NaN )
NOALIAS __forceinline const VBool Unordered( const VF32 VECREF x, const VF32 VECREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::False(); // SPU doesn't support vector float NaN's
    #elif defined( _PPU_ )
    result.m_V = vec_nor( vec_cmpge( x.m_V, y.m_V ), vec_cmpge( y.m_V, x.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vnor( __vcmpgefp( x.m_V, y.m_V ), __vcmpgefp( y.m_V, x.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpunord_ps( x.m_V, y.m_V );
    #else
    result.m_V = _isnan( x.m_V ) || _isnan( y.m_V );
    #endif
    return result;
}

/// VF32 NaN Test
NOALIAS __forceinline const VBool IsNan( const VF32 VECREF x )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::False(); // SPU doesn't support vector float NaN's
    #elif defined( _PPU_ )
    result.m_V = vec_not( vec_cmpeq( x.m_V, x.m_V ) );
    #elif defined( _VMX128_ )
    const __vector4 eq( __vcmpeqfp( x.m_V, x.m_V ) );
    result.m_V = __vnor( eq, eq );
    #elif defined( _SSE_ )
    result.m_V = _mm_cmpunord_ps( x.m_V, x.m_V );
    #else
    result.m_V = _isnan( x.m_V ) != 0;
    #endif
    return result;
}

/// VF32 Finite Test ( true if x is not a NaN and abs(x) <= FLT_MAX )
NOALIAS __forceinline const VBool Finite( const VF32 VECREF x )
{
    VBool result;
    #if defined( _SPU_ )
    //result.m_V = spu_cmplt( spu_abs( x.m_V ), Const< vector float >::FltMax() ); // SPU doesn't support vector float NaN's
    result.m_V = spu_not( spu_cmpabsgt( x.m_V, Const< vector float >::FltMax() ) ); // SPU doesn't support vector float NaN's
    #elif defined( _PPU_ )
    result.m_V = vec_and( vec_cmpeq( x.m_V, x.m_V ), vec_cmpge( Const< vector float >::FltMax(), vec_and( x.m_V, Const< vector float >::AbsMask() ) ) );
    #elif defined( _VMX128_ )
    result.m_V = __vand( __vcmpeqfp( x.m_V, x.m_V ), __vcmpgefp( Const<__vector4>::FltMax(), __vand( x.m_V, Const<__vector4>::AbsMask() ) ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_ps( _mm_cmpord_ps( x.m_V, x.m_V ), _mm_cmpge_ps( Const<__m128>::FltMax(), _mm_and_ps( x.m_V, Const<__m128>::AbsMask() ) ) );
    #else
    result.m_V = _finite( x.m_V ) != 0;
    #endif
    return result;
}

/// VF32 Select : returns ( condition ? result_if_true : result_if_false )
NOALIAS __forceinline const VF32 Select( const VBool PCREF condition, const VF32 VECREF result_if_true, const VF32 VECREF result_if_false )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( result_if_false.m_V, result_if_true.m_V, condition.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_ps( _mm_and_ps( condition.m_V, result_if_true.m_V ), _mm_andnot_ps( condition.m_V, result_if_false.m_V ) );
    #else
    result.m_V = ( condition.m_V ? result_if_true.m_V : result_if_false.m_V );
    #endif
    return result;
}


/// VF32 Branchless Conditional Store: stores src into pDst if condition is true
/// NOTE: This performs a read from pDst first which will load the data into the L1 cache rather than just writing through to L2 (if not already in L1)
NOALIAS __forceinline void StoreConditional( const VBool PCREF condition, const VF32 VECREF src, F32* const __restrict pDst )
{
    Store( Select( condition, src, MakeVF32( pDst ) ), pDst );
}


/// VF32 Bitwise AND: a & b : returns ( b ? a : zero )
NOALIAS __forceinline const VF32 AND( const VF32 VECREF a, const VBool PCREF b )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_and( a.m_V, (vector float)b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_and( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_and( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_and( a.m_V, b.m_V );
    #else
    result.m_V = b.m_V ? a.m_V : 0.0f;
    #endif
    return result;
}

/// VF32 Bitwise ANDC: a & ~b : returns ( b ? zero : a )
NOALIAS __forceinline const VF32 ANDC( const VF32 VECREF a, const VBool PCREF b )
{
    VF32 result;
    #if defined( _SPU_ )
    result.m_V = spu_andc( a.m_V, (vector float)b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_andc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_andc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_andc( a.m_V, b.m_V );
    #else
    //result.m_V = TypePun_IntToFloat( TypePun_FloatToInt( a.m_V ) & ~b.m_V );
    result.m_V = !b.m_V ? a.m_V : 0.0f;
    #endif
    return result;
}












//
// Promote ( F32, VF32 ) binary ops to ( VF32, VF32 )
//

NOALIAS __forceinline const VF32 operator + ( const F32& x, const VF32 VECREF y ) { return MakeVF32( x ) + y; }
NOALIAS __forceinline const VF32 operator - ( const F32& x, const VF32 VECREF y ) { return MakeVF32( x ) - y; }
NOALIAS __forceinline const VF32 operator * ( const F32& x, const VF32 VECREF y ) { return MakeVF32( x ) * y; }
NOALIAS __forceinline const VF32 operator / ( const F32& x, const VF32 VECREF y ) { return MakeVF32( x ) / y; }

//
// Promote ( VF32, F32 ) binary ops to ( VF32, VF32 )
//

NOALIAS __forceinline const VF32 operator + ( const VF32 VECREF x, const F32& y ) { return x + MakeVF32( y ); }
NOALIAS __forceinline const VF32 operator - ( const VF32 VECREF x, const F32& y ) { return x - MakeVF32( y ); }
NOALIAS __forceinline const VF32 operator * ( const VF32 VECREF x, const F32& y ) { return x * MakeVF32( y ); }
NOALIAS __forceinline const VF32 operator / ( const VF32 VECREF x, const F32& y ) { return x / MakeVF32( y ); }

NOALIAS __forceinline const VF32& operator += ( VF32& x, const F32& y ) { return x += MakeVF32( y ); }
NOALIAS __forceinline const VF32& operator -= ( VF32& x, const F32& y ) { return x -= MakeVF32( y ); }
NOALIAS __forceinline const VF32& operator *= ( VF32& x, const F32& y ) { return x *= MakeVF32( y ); }
NOALIAS __forceinline const VF32& operator /= ( VF32& x, const F32& y ) { return x /= MakeVF32( y ); }




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


