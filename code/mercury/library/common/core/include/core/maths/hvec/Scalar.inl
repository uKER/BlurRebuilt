#ifndef MATHS_HVEC_SCALAR_INL
#define MATHS_HVEC_SCALAR_INL

#ifdef _MSC_VER
#pragma once
#endif

//#if defined( _VMX128_ )
//#include <ppcintrinsics.h>
//#endif

namespace Maths
{

///
/// Hardware Scalar Float Class
/// SSE: Single float stored in X component of hardware 4 element vector register. Y, Z & W components are set to zero [todo:undefined].
/// VMX: Float is replicated into each component of hardware 4 element vector register
///

#if defined( MATHS_DONT_INITIALISE_TO_ZERO )
/// Default Constructor
NOALIAS __forceinline HF32::HF32()
{
}
#else
NOALIAS __forceinline HF32::HF32()
    #if defined( _SPU_ )
    : m_F( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_F( Const< vector float >::Zero() )
    #elif defined( _VMX128_ )
    : m_F( __vzero() )
    //#elif defined( _SSE4_1_ )
    //: m_F( _mm_insert_ps( m_F, m_F, 0x0F ) ) // INSERTPS m_F, m_F, 00b::00b::1111b : Src=X, Dst=X, ZeroMask=WZYX
    #elif defined( _SSE_ )
    : m_F( _mm_setzero_ps() )
    #elif defined( _SIMD_ )
    : m_F( SIMD_Zero() )
    #else
    : m_F( 0.0f )
    #endif
{
}
#endif

/// Zero Init Constructor
NOALIAS __forceinline HF32::HF32( const _ZERO_INIT /*zeroinit*/ )
    #if defined( _SPU_ )
    : m_F( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_F( Const< vector float >::Zero() )
    #elif defined( _VMX128_ )
    : m_F( __vzero() )
    //#elif defined( _SSE4_1_ )
    //: m_F( _mm_insert_ps( m_F, m_F, 0x0F ) ) // INSERTPS m_F, m_F, 00b::00b::1111b : Src=X, Dst=X, ZeroMask=WZYX
    #elif defined( _SSE_ )
    : m_F( _mm_setzero_ps() )
    #elif defined( _SIMD_ )
    : m_F( SIMD_Zero() )
    #else
    : m_F( 0.0f )
    #endif
{
}

/// Undefined Constructor
NOALIAS __forceinline HF32::HF32( const _UNDEFINED /*undefined*/ )
{
}

#if !defined( PASSINREG_ENABLED )
/// Copy Constructor
NOALIAS __forceinline HF32::HF32( const HF32& copy ) __restrict
    : m_F( copy.m_F )
{
}
#endif

/// F32 Constructor
NOALIAS __forceinline HF32::HF32( const F32& f )
    #if defined( _SPU_ )
    : m_F( spu_splats( f ) )
    #elif defined( _PPU_ )
    //: m_F( __builtin_constant_p( f ) ? (vector float){ f, f, f, f } : vec_splat( vec_load_left_aligned( &f ), VX ) )
    : m_F( vec_splat( vec_load_left_aligned( &f ), VX ) )
    #elif defined( _VMX128_ )
    : m_F( __vspltw( __lvlx( &f, 0x0 ), VX ) )
    //#elif defined( _SSE4_1_ )
    //: m_F( _mm_insert_ps( m_F, (__m128&)f, 0x0E ) ) // 0b00::0b00::0b1110 : Src=X, Dst=X, ZeroMask=WZY_
    #elif defined( _SSE_ )
    //: m_F( SIMD_Scalar( f ) )
    //: m_F( _mm_set_ss( f ) )   // _mm_set_ss for old scalar implementation (scalar element[x] = f; elements y,z,w = 0.0f)
    : m_F( _mm_set1_ps( f ) )  // _mm_set1_ps for new scalar implementation where all vector elements set to same value
    #else
    : m_F( f )
    #endif
{
}

/// F32 Constructor
NOALIAS __forceinline HF32::HF32( const F64& d )
    #if defined( _SPU_ )
    : m_F( spu_splats( (float)d ) )
    #elif defined( _VMX128_ ) || defined( _SSE_ )
    : m_F( SIMD_Scalar( (F32)d ) )
    #elif !defined( _SIMD_ )
    : m_F( (F32)d )
    #endif
{
    #if defined( _PPU_ )
    const F32 f = (F32)d;
    //const vector float vf = vec_lvx( 0x0, &f );
    //const vector unsigned char vp = vec_lvsl( 0x0, &f );
    //m_F = vec_splat( vec_perm( vf, vf, vp ), VX );
    m_F = vec_splat( vec_load_left_aligned( &f ), VX );
    #endif
}


/// S32 Constructor
NOALIAS __forceinline HF32::HF32( const S32& i )
    #if defined( _SPU_ )
    : m_F( spu_convtf( spu_splats( i ), 0 ) )
    #elif defined( _PPU_ )
    : m_F( vec_vcfsx( vec_splat( vec_load_left_aligned( &i ), VX ), 0 ) )
    #elif defined( _VMX128_ ) || defined( _SSE_ )
    : m_F( SIMD_Scalar( i ) )
    #else
    : m_F( (F32)i )
    #endif
{
}

/// U32 Constructor
NOALIAS __forceinline HF32::HF32( const U32& i )
    #if defined( _SPU_ )
    : m_F( spu_convtf( spu_splats( i ), 0 ) )
    #elif defined( _PPU_ )
    : m_F( vec_vcfux( vec_splat( vec_load_left_aligned( &i ), VX ), 0 ) )
    #elif defined( _VMX128_ ) || defined( _SSE_ )
    : m_F( SIMD_Scalar( i ) )
    #else
    : m_F( (F32)i )
    #endif
{
}

/// F32 Ptr Constructor
NOALIAS __forceinline HF32::HF32( F32 const * const __restrict ptr )
    #if defined( _SPU_ )
    : m_F( spu_splats( *ptr ) )
    #elif defined( _PPU_ )
    : m_F( vec_splat( vec_load_left_aligned( ptr ), VX ) )
    #elif defined( _VMX128_ )
    : m_F( __vspltw( __lvlx( ptr, 0x0 ), VX ) )
    #elif defined( _SSE_ )
    : m_F( SIMD_Scalar( ptr ) )
    #elif !defined( _SIMD_ )
    : m_F( *ptr )
    #endif
{                                                
}

/// Selector Constructor: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
SYS_DEPRECATED NOALIAS __forceinline HF32::HF32( const Scalar_Bool PCREF selector, const HF32 PCREF a, const HF32 PCREF b )
    #if defined( _SPU_ )
    : m_F( spu_sel( b.m_F, a.m_F, selector.m_V ) )
    #elif defined( _PPU_ )
    : m_F( vec_sel( b.m_F, a.m_F, selector.m_V ) )
    #elif defined( _SIMD_ )
    : m_F( SIMD_Select( selector.m_V, a.m_F, b.m_F ) )
    #endif
{
    #if !defined( _SIMD_ )
    const U32 result = ( selector.m_F & (U32&)a.m_F ) | ( ~selector.m_F & (U32&)b.m_F );
    m_F = (F32&)result;
    #endif
}

/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HF32::HF32( const V128 PCREF src )
    #if defined( _SIMD_ )
    : m_F( src )
    #else
    : m_F( src.m_X )
    #endif
{
}

/// HVecX Constructor
NOALIAS __forceinline HF32::HF32( const HVecX PCREF x )
    #if defined( _SPU_ )
    : m_F( spu_splat<VX>( x.m_X ) )
    #elif defined( _PPU_ )
    : m_F( vec_splat( x.m_X, VX ) )
    #elif defined( _VMX128_ )
    : m_F( __vspltw( x.m_X, VX ) )
    #elif defined( _SSE_ )
    : m_F( SIMD_Scalar_VectorX( x.m_X ) )
    #else
    : m_F( x.m_X )
    #endif
{
}

/// HVecY Constructor
NOALIAS __forceinline HF32::HF32( const HVecY PCREF y )
    #if defined( _SPU_ )
    : m_F( spu_splat<VY>( y.m_Y ) )
    #elif defined( _PPU_ )
    : m_F( vec_splat( y.m_Y, VY ) )
    #elif defined( _VMX128_ )
    : m_F( __vspltw( y.m_Y, VY ) )
    #elif defined( _SSE_ )
    : m_F( SIMD_Scalar_VectorY( y.m_Y ) )
    #else
    : m_F( y.m_Y )
    #endif
{
}

/// HVecZ Constructor
NOALIAS __forceinline HF32::HF32( const HVecZ PCREF z )
    #if defined( _SPU_ )
    : m_F( spu_splat<VZ>( z.m_Z ) )
    #elif defined( _PPU_ )
    : m_F( vec_splat( z.m_Z, VZ ) )
    #elif defined( _VMX128_ )
    : m_F( __vspltw( z.m_Z, VZ ) )
    #elif defined( _SSE_ )
    : m_F( SIMD_Scalar_VectorZ( z.m_Z ) )
    #else
    : m_F( z.m_Z )
    #endif
{
}

/// HVecW Constructor
NOALIAS __forceinline HF32::HF32( const HVecW PCREF w )
    #if defined( _SPU_ )
    : m_F( spu_splat<VW>( w.m_W ) )
    #elif defined( _PPU_ )
    : m_F( vec_splat( w.m_W, VW ) )
    #elif defined( _VMX128_ )
    : m_F( __vspltw( w.m_W, VW ) )
    #elif defined( _SSE_ )
    : m_F( SIMD_Scalar_VectorW( w.m_W ) )
    #else
    : m_F( w.m_W )
    #endif
{
}

/// F32 Cast
NOALIAS __forceinline HF32::operator const F32() const __restrict
{
    #if defined( _SPU_ )
    return spu_extract( m_F, VX ); // extract any component, they're all the same
    #elif defined( _PPU_ )
    F32 F32_from_HF32;
    vec_ste( m_F, 0x0, &F32_from_HF32 );
    return F32_from_HF32;
    #elif defined( _VMX128_ )
    F32 F32_from_HF32;
    __stvewx( m_F, &F32_from_HF32, 0x0 );
    return F32_from_HF32;
    #elif defined( _SIMD_ )
    return SIMD_ScalarToF32( m_F );
    #else
    return m_F;
    #endif
}

/// Copy Assignment
NOALIAS __forceinline const HF32& HF32::operator = ( const HF32& copy )
{
    m_F = copy.m_F;
    return copy;
}

/// F32 Assignment
NOALIAS __forceinline const HF32& HF32::operator = ( const F32& f ) __restrict
{
    #if defined( _SPU_ )
    m_F = spu_splats( f );
    #elif defined( _PPU_ )
    //m_F = __builtin_constant_p( f ) ? (vector float){ f, f, f, f } : vec_splat( vec_load_left_aligned( &f ), VX );
    m_F = vec_splat( vec_load_left_aligned( &f ), VX );
    #elif defined( _VMX128_ )
    m_F = __vspltw( __lvlx( &f, 0x0 ), VX );
    #elif defined( _SSE_ )
    m_F = SIMD_Scalar( f );
    #else
    m_F = f;
    #endif
    return *this;
}

/// S32 Assignment
NOALIAS __forceinline const HF32& HF32::operator = ( const S32& i ) __restrict
{
    #if defined( _SPU_ )
    m_F = spu_convtf( spu_splats( i ), 0 );
    #elif defined( _PPU_ )
    m_F = vec_vcfsx( vec_splat( vec_load_left_aligned( &i ), VX ), 0 );
    #elif defined( _SIMD_ )
    m_F = SIMD_Scalar( i );
    #else
    m_F = (F32)i;
    #endif
    return *this;
}

/// U32 Assignment
NOALIAS __forceinline const HF32& HF32::operator = ( const U32& i ) __restrict
{
    #if defined( _SPU_ )
    m_F = spu_convtf( spu_splats( i ), 0 );
    #elif defined( _PPU_ )
    m_F = vec_vcfux( vec_splat( vec_load_left_aligned( &i ), VX ), 0 );
    #elif defined( _SIMD_ )
    m_F = SIMD_Scalar( i );
    #else
    m_F = (F32)i;
    #endif
    return *this;
}

/// S32 Conversion
NOALIAS __forceinline const S32 HF32::F2I() const __restrict
{
    #if defined( _SPU_ )
    return spu_extract( spu_convts( m_F, 0 ), VX );
    #elif defined( _PPU_ )
    S32 i;
    vec_ste( vec_vctsxs( m_F, 0 ), 0x0, &i );
    return i;
    #elif defined( _SSE_ )
    return _mm_cvttss_si32( m_F );
    #elif defined( _SIMD_ )
    return SIMD_ScalarToS32( m_F );
    #else
    return (S32)m_F;
    #endif
}

/// Abs
NOALIAS __forceinline const HF32 HF32::Abs() const __restrict
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_abs( m_F );
    #elif defined( _PPU_ )
    result.m_F = vec_abs( m_F );
    #elif defined( _VMX128_ )
    const __vector4 absMask( c_VectorAbsMask );
    result.m_F = __vand( m_F, absMask );
    #elif defined( _SIMD_ )
    //result.m_F = SIMD_ScalarAbs( m_F );
    result.m_F = _mm_and_ps( m_F, (__m128&)c_VectorAbsMask );
    #else
    result.m_F = Maths::Abs( m_F );
    #endif
    return result;
}

/// Sign : Returns -1.0f if scalar < 0.0f, +1.0f if scalar > 0.0f, or 0.0f if scalar == 0.0f 
NOALIAS __forceinline const HF32 HF32::Sign() const __restrict
{
    HF32 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float zero( Const< vector float >::Zero() );
    //const vector float one( spu_splats( 1.0f ) );
    const vector float one( Const< vector float >::One() );
    //const vector unsigned int sign_mask( spu_splats( 0x80000000 ) );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    const vector float sign( spu_sel( (vector float)spu_andc( one, (vector float)spu_cmpeq( m_F, zero ) ), m_F, sign_mask ) );
    result.m_F = sign;
    
    #elif defined( _PPU_ )
    
    //const vector float zero( c_ScalarZero );
    //const vector float one( c_ScalarOne );
    //const vector unsigned int signMask( c_ScalarSignMask );
    const vector float zero( Const< vector float >::Zero() );
    const vector float one( Const< vector float >::One() );
    const vector unsigned int signMask( Const< vector unsigned int >::SignMask() );
    const vector float sign( vec_sel( (vector float)vec_andc( one, (vector float)vec_vcmpeqfp( m_F, zero ) ), m_F, signMask ) );
    result.m_F = sign;
    
    #elif defined( _VMX128_ )
    
    const __vector4 zero( __vzero() );
    const __vector4 one( c_ScalarOne );
    const __vector4 signMask( c_ScalarSignMask );
    const __vector4 sign( __vsel( __vandc( one, __vcmpeqfp( m_F, zero ) ), m_F, signMask ) );
    result.m_F = sign;
    
    #elif defined( _SSE_ )
    
    //const HF32 zero( Constants<HF32>::Zero() );
    //const HF32 result( SIMD_OR( SIMD_AND( m_F, (__m128&)c_ScalarSignMask ), c_ScalarOne ) ); // c_ScalarOne positive so doesn't need sign bit masking out!
    //result = Select( Abs().NE( zero ), result, zero );
    const __m128 zero( _mm_setzero_ps() );
    //const __m128 one( c_ScalarOne );
    //const __m128 signMask( (__m128&)c_ScalarSignMask );
    const __m128 one( c_VectorOne );
    const __m128 absMask( (__m128&)c_VectorAbsMask );
    const __m128 signMask( (__m128&)c_VectorSignMask );
    const __m128 sign( _mm_or_ps( _mm_and_ps( m_F, signMask ), one ) );
    const __m128 notZero( _mm_cmpneq_ps( _mm_and_ps( m_F, absMask ), zero ) );
    result.m_F = _mm_and_ps( sign, notZero );
    
    #else
	
    result.m_F = ( m_F < 0.0f ? -1.0f : ( m_F > 0.0f ? 1.0f : 0.0f ) );
    
    #endif
    
    return result;
}

/// Square Root
NOALIAS __forceinline const HF32 HF32::Sqrt() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = m_F;
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = spu_madd( B, rcp_sqrt_B, zero );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = m_F;
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_madd( B, rcp_sqrt_B, zero );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( c_VectorHalf );
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 fltMax( c_VectorFltMax );
    const __vector4 B( m_F );
    const __vector4 B_GT_fltMin( __vcmpgtfp( B, fltMin ) );
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );
    const __vector4 valid( __vand( B_GT_fltMin, B_LT_fltMax ) );
    const __vector4 y0( __vrsqrtefp( B ) );
    const __vector4 t0( __vmulfp( y0, y0 ) );
    const __vector4 t1( __vmulfp( y0, half ) );
    const __vector4 t2( __vnmsubfp( B, t0, one ) );
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
    const __vector4 rcp_sqrt_B( __vsel( __vzero(), y1, valid ) );
    HF32 result( UNDEFINED );
    result.m_F = __vmulfp( B, rcp_sqrt_B );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarSqrt( m_F ) );
    #else
    return HF32( Maths::Sqrt( m_F ) );
    #endif
}

/// Reciprocal
NOALIAS __forceinline const HF32 HF32::Rcp() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_B;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 B( m_F );
    const __vector4 abs_B( __vand( B, c_VectorAbsMask ) );
    const __vector4 valid( __vcmpgtfp( abs_B, fltMin ) );
    const __vector4 y0( __vrefp( B ) );
    const __vector4 t0( __vnmsubfp( y0, B, one ) );
    const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
    
    HF32 result( UNDEFINED );
    result.m_F = __vsel( __vzero(), y1, valid );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarReciprocal( m_F ) );
    #else
    //return HF32( 1.0f / m_F );
    return HF32( Maths::Reciprocal( m_F ) );
    #endif
}

/// Reciprocal
NOALIAS __forceinline const HF32 HF32::Reciprocal() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_B;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 B( m_F );
    const __vector4 abs_B( __vand( B, c_VectorAbsMask ) );
    const __vector4 valid( __vcmpgtfp( abs_B, fltMin ) );
    const __vector4 y0( __vrefp( B ) );
    const __vector4 t0( __vnmsubfp( y0, B, one ) );
    const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
    HF32 result( UNDEFINED );
    result.m_F = __vsel( __vzero(), y1, valid );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarReciprocal( m_F ) );
    #else
    //return HF32( 1.0f / m_F );
    return HF32( Maths::Reciprocal( m_F ) );
    #endif
}

/// Reciprocal Square Root
NOALIAS __forceinline const HF32 HF32::RcpSqrt() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = m_F;
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_sqrt_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = m_F;
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_sqrt_B;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( c_VectorHalf );
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 fltMax( c_VectorFltMax );
    const __vector4 B( m_F );
    const __vector4 B_GT_fltMin( __vcmpgtfp( B, fltMin ) );
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );
    const __vector4 valid( __vand( B_GT_fltMin, B_LT_fltMax ) );
    const __vector4 y0( __vrsqrtefp( B ) );
    const __vector4 t0( __vmulfp( y0, y0 ) );
    const __vector4 t1( __vmulfp( y0, half ) );
    const __vector4 t2( __vnmsubfp( B, t0, one ) );
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
    HF32 result( UNDEFINED );
    result.m_F = __vsel( __vzero(), y1, valid );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarReciprocalSqrt( m_F ) );
    #else
    return HF32( Maths::ReciprocalSqrt( m_F ) );
    #endif
}

/// Reciprocal Square Root
NOALIAS __forceinline const HF32 HF32::ReciprocalSqrt() const __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = m_F;
    vector float        rcp_sqrt_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_sqrt_B;
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = m_F;
    vector float       rcp_sqrt_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = rcp_sqrt_B;
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( c_VectorHalf );
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 fltMax( c_VectorFltMax );
    const __vector4 B( m_F );
    const __vector4 B_GT_fltMin( __vcmpgtfp( B, fltMin ) );
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );
    const __vector4 valid( __vand( B_GT_fltMin, B_LT_fltMax ) );
    const __vector4 y0( __vrsqrtefp( B ) );
    const __vector4 t0( __vmulfp( y0, y0 ) );
    const __vector4 t1( __vmulfp( y0, half ) );
    const __vector4 t2( __vnmsubfp( B, t0, one ) );
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
    HF32 result( UNDEFINED );
    result.m_F = __vsel( __vzero(), y1, valid );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarReciprocalSqrt( m_F ) );
    #else
    //return HF32( 1.0f / Maths::Sqrt( m_F ) );
    return HF32( Maths::ReciprocalSqrt( m_F ) );
    #endif
}


/// Unary Positive Operator ( yes, this is needed, otherwise the (xenon) compiler will do very stupid things... )
NOALIAS __forceinline const HF32 HF32::operator + () const __restrict
{
    return *this;
}

/// Unary Negation Operator
NOALIAS __forceinline const HF32 HF32::operator - () const __restrict
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_negate( m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float signMask( Const< vector float >::SignMask() );
    HF32 result( UNDEFINED );
    result.m_F = vec_xor( m_F, signMask );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 signMask( c_Vector4_SignMask );
    HF32 result( UNDEFINED );
    result.m_F = __vxor( m_F, signMask );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarNegate( m_F ) );
    #else
    return HF32( -m_F );
    #endif
}

/// Binary Addition Operator
NOALIAS __forceinline const HF32 HF32::operator + ( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_add( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_add( m_F, src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vaddfp( m_F, src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarAdd( m_F, src.m_F ) );
    #else
    return HF32( m_F + src.m_F );
    #endif
}

/// Binary Subtraction Operator
NOALIAS __forceinline const HF32 HF32::operator - ( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_sub( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_sub( m_F, src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vsubfp( m_F, src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarSubtract( m_F, src.m_F ) );
    #else
    return HF32( m_F - src.m_F );
    #endif
}

/// Binary Multiplication Operator
NOALIAS __forceinline const HF32 HF32::operator * ( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_mul( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_mul( m_F, src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vmulfp( m_F, src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarMultiply( m_F, src.m_F ) );
    #else
    return HF32( m_F * src.m_F );
    #endif
}

/// Binary Division Operator
NOALIAS __forceinline const HF32 HF32::operator / ( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_F;
    const vector float  B = src.m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = spu_mul( A, rcp_B );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_F;
    const vector float B = src.m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    HF32 result( UNDEFINED );
    result.m_F = vec_mul( A, rcp_B );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 absMask( c_VectorAbsMask );
    const __vector4 A( m_F );
    const __vector4 B( src.m_F );
    const __vector4 abs_B( __vand( B, absMask ) );
    const __vector4 valid( __vcmpgtfp( abs_B, fltMin ) );
    const __vector4 y0( __vrefp( B ) );
    const __vector4 t0( __vnmsubfp( y0, B, one ) );
    const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
    const __vector4 rcp_B( __vsel( __vzero(), y1, valid ) );
    HF32 result( UNDEFINED );
    result.m_F = __vmulfp( A, rcp_B );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarDivide( m_F, src.m_F ) );
    #else
    //return HF32( m_F / src.m_F );
    return HF32( m_F * Maths::Reciprocal( src.m_F ) );
    #endif
}

/// Add Update Operator
NOALIAS __forceinline const HF32& HF32::operator += ( const HF32 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_F = spu_add( m_F, src.m_F );
    #elif defined( _PPU_ )
    m_F = vec_add( m_F, src.m_F );
    #elif defined( _VMX128_ )
    m_F = __vaddfp( m_F, src.m_F );
    #elif defined( _SIMD_ )
    m_F = SIMD_ScalarAdd( m_F, src.m_F );
    #else
    m_F = m_F + src.m_F;
    #endif
    return *this;
}

/// Subtract Update Operator
NOALIAS __forceinline const HF32& HF32::operator -= ( const HF32 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_F = spu_sub( m_F, src.m_F );
    #elif defined( _PPU_ )
    m_F = vec_sub( m_F, src.m_F );
    #elif defined( _VMX128_ )
    m_F = __vsubfp( m_F, src.m_F );
    #elif defined( _SIMD_ )
    m_F = SIMD_ScalarSubtract( m_F, src.m_F );
    #else
    m_F = m_F - src.m_F;
    #endif
    return *this;
}

/// Multiply Update Operator
NOALIAS __forceinline const HF32& HF32::operator *= ( const HF32 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_F = spu_mul( m_F, src.m_F );
    #elif defined( _PPU_ )
    m_F = vec_mul( m_F, src.m_F );
    #elif defined( _VMX128_ )
    m_F = __vmulfp( m_F, src.m_F );
    #elif defined( _SIMD_ )
    m_F = SIMD_ScalarMultiply( m_F, src.m_F );
    #else
    m_F = m_F * src.m_F;
    #endif
    return *this;
}

/// Divide Update Operator
NOALIAS __forceinline const HF32& HF32::operator /= ( const HF32 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  A = m_F;
    const vector float  B = src.m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    m_F = spu_mul( A, rcp_B );
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float A = m_F;
    const vector float B = src.m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    m_F = vec_mul( A, rcp_B );
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 absMask( c_VectorAbsMask );
    const __vector4 A( m_F );
    const __vector4 B( src.m_F );
    const __vector4 abs_B( __vand( B, absMask ) );
    const __vector4 valid( __vcmpgtfp( abs_B, fltMin ) );
    const __vector4 y0( __vrefp( B ) );
    const __vector4 t0( __vnmsubfp( y0, B, one ) );
    const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
    const __vector4 rcp_B( __vsel( __vzero(), y1, valid ) );
    m_F = __vmulfp( A, rcp_B );
    
    #elif defined( _SIMD_ )
    m_F = SIMD_ScalarDivide( m_F, src.m_F );
    #else
    //m_F = m_F / src.m_F;
    m_F = m_F * Maths::Reciprocal( src.m_F );
    #endif
    return *this;
}

/// Min
NOALIAS __forceinline const HF32 HF32::Min( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_min( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_min( m_F, src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vminfp( m_F, src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarMin( m_F, src.m_F ) );
    #else
    return HF32( Maths::Min( m_F, src.m_F ) );
    #endif
}

/// Max
NOALIAS __forceinline const HF32 HF32::Max( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_max( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_max( m_F, src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vmaxfp( m_F, src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_ScalarMax( m_F, src.m_F ) );
    #else
    return HF32( Maths::Max( m_F, src.m_F ) );
    #endif
}

/// Clamp to range
NOALIAS __forceinline const HF32 HF32::Clamp( const HF32 PCREF lo, const HF32 PCREF hi ) const __restrict
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_min( spu_max( m_F, lo.m_F ), hi.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_min( vec_vmaxfp( m_F, lo.m_F ), hi.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vminfp( __vmaxfp( m_F, lo.m_F ), hi.m_F );
    return result;
    
    #else
    return Max( lo ).Min( hi );
    #endif
}

/// Min Update
NOALIAS __forceinline const HF32& HF32::MinEq( const HF32 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_F = spu_min( m_F, src.m_F );
    #elif defined( _PPU_ )
    m_F = vec_min( m_F, src.m_F );
    #elif defined( _VMX128_ )
    m_F = __vminfp( m_F, src.m_F );
    #elif defined( _SIMD_ )
    m_F = SIMD_ScalarMin( m_F, src.m_F );
    #else
    m_F = Maths::Min( m_F, src.m_F );
    #endif
    return *this;
}

/// Max Update
NOALIAS __forceinline const HF32& HF32::MaxEq( const HF32 PCREF src ) __restrict
{
    #if defined( _SPU_ )
    m_F = spu_max( m_F, src.m_F );
    #elif defined( _PPU_ )
    m_F = vec_max( m_F, src.m_F );
    #elif defined( _VMX128_ )
    m_F = __vmaxfp( m_F, src.m_F );
    #elif defined( _SIMD_ )
    m_F = SIMD_ScalarMax( m_F, src.m_F );
    #else
    m_F = Maths::Max( m_F, src.m_F );
    #endif
    return *this;
}

NOALIAS __forceinline const Scalar_Bool HF32::operator == ( const HF32 PCREF src ) const __restrict {return EQ(src);}
NOALIAS __forceinline const Scalar_Bool HF32::operator != ( const HF32 PCREF src ) const __restrict {return NE(src);}
NOALIAS __forceinline const Scalar_Bool HF32::operator <= ( const HF32 PCREF src ) const __restrict {return LE(src);}
NOALIAS __forceinline const Scalar_Bool HF32::operator < ( const HF32 PCREF src ) const __restrict {return LT(src);}
NOALIAS __forceinline const Scalar_Bool HF32::operator >= ( const HF32 PCREF src ) const __restrict {return GE(src);}
NOALIAS __forceinline const Scalar_Bool HF32::operator > ( const HF32 PCREF src ) const __restrict {return GT(src);}

/// Generate 4 element bit mask for this == src
NOALIAS __forceinline const Scalar_Bool HF32::EQ( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_cmpeq( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_cmpeq( m_F, src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vcmpeqfp( m_F, src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return SIMD_ScalarSelectorEQ( m_F, src.m_F );
    #else
    return Scalar_Bool( m_F == src.m_F ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Generate 4 element bit mask for this != src
NOALIAS __forceinline const Scalar_Bool HF32::NE( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_cmpne( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector bool int eq( vec_cmpeq( m_F, src.m_F ) );
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_vnor( eq, eq );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 eq( __vcmpeqfp( m_F, src.m_F ) );
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vnor( eq, eq );
    return result;
    
    #elif defined( _SIMD_ )
    return SIMD_ScalarSelectorNE( m_F, src.m_F );
    #else
    return Scalar_Bool( m_F != src.m_F ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Generate 4 element bit mask for this <= src
NOALIAS __forceinline const Scalar_Bool HF32::LE( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_cmpge( src.m_F, m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_cmpge( src.m_F, m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vcmpgefp( src.m_F, m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return SIMD_ScalarSelectorLE( m_F, src.m_F );
    #else
    return Scalar_Bool( m_F <= src.m_F ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Generate 4 element bit mask for this < src
NOALIAS __forceinline const Scalar_Bool HF32::LT( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_cmpgt( src.m_F, m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_cmpgt( src.m_F, m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vcmpgtfp( src.m_F, m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return SIMD_ScalarSelectorLT( m_F, src.m_F );
    #else
    return Scalar_Bool( m_F < src.m_F ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Generate 4 element bit mask for this >= src
NOALIAS __forceinline const Scalar_Bool HF32::GE( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_cmpge( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_cmpge( m_F, src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vcmpgefp( m_F, src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return SIMD_ScalarSelectorGE( m_F, src.m_F );
    #else
    return Scalar_Bool( m_F >= src.m_F ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Generate 4 element bit mask for this > src
NOALIAS __forceinline const Scalar_Bool HF32::GT( const HF32 PCREF src ) const __restrict
{
    #if defined( _SPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = spu_cmpgt( m_F, src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_cmpgt( m_F, src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vcmpgtfp( m_F, src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return SIMD_ScalarSelectorGT( m_F, src.m_F );
    #else
    return Scalar_Bool( m_F > src.m_F ? 0xFFFFFFFF : 0x00000000 );
    #endif
}

/// Finite Test
NOALIAS __forceinline const Scalar_Bool HF32::Finite() const __restrict
{
    #if defined( _SPU_ )
    
    Scalar_Bool result( UNDEFINED );
    //result.m_V = spu_and( spu_cmplt( spu_abs( m_F ), Const< vector float >::FltMax() ), spu_cmpeq( m_F, m_F ) ); /// NOTE: No NaN's on SPU!
    result.m_V = spu_cmplt( spu_abs( m_F ), Const< vector float >::FltMax() );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float flt_max = Const< vector float >::FltMax();
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = vec_and( vec_cmpeq( vec_cmpb( m_F, flt_max ), Const< vector int >::Zero() ), vec_cmpeq( m_F, m_F ) );
    return result;
    
    #elif defined( _VMX128_ )
    
    Scalar_Bool result( UNDEFINED );
    result.m_V = __vand( __vcmpequw( __vcmpbfp( m_F, c_VectorFltMax ), __vzero() ), __vcmpeqfp( m_F, m_F ) );
    return result;
    
    #elif defined( _SSE_ )
    #pragma warning(push)
    #pragma warning(disable:4800)
    return Scalar_Bool( 1 ^ ( _mm_comilt_ss( m_F, HF32( -FLT_MAX ).m_F ) |
                              _mm_comigt_ss( m_F, HF32( +FLT_MAX ).m_F ) ) );
    #pragma warning(pop)
    #else
    
    return Scalar_Bool( _finite( m_F ) );
    
    #endif
}


/// Debug validation
NOALIAS __forceinline void HF32::DebugValidate() const __restrict
{
    #if defined( MATHS_DEBUG_MAX )
    #if defined( _VMX128_ )
    U32 CR1, CR2, CR3;
    __vcmpeqfpR( __vspltw( m_F, VX ), __vspltw( m_F, VY ), &CR1 );
    __vcmpeqfpR( __vspltw( m_F, VX ), __vspltw( m_F, VZ ), &CR2 );
    __vcmpeqfpR( __vspltw( m_F, VX ), __vspltw( m_F, VW ), &CR3 );
    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
    {
        DebugBreak();
    }
    #elif defined( _SSE_ )
    if ( ( m_F.m128_f32[VY] != 0.0f ) ||
         ( m_F.m128_f32[VZ] != 0.0f ) ||
         ( m_F.m128_f32[VW] != 0.0f ) )
    {
        DebugBreak();
    }
    #endif
    #endif
}


/// Read from Stream
template < typename T_Stream >
NOALIAS __forceinline void HF32::Deserialise( T_Stream& stream )
{
    #if defined( _SPU_ )
    F32 x;
    stream >> x;
    m_F = spu_splats( x );
    #elif defined( _PPU_ )
    F32 x;
    stream >> x;
    *this = HF32( x );
    #elif defined( _SIMD_ )
    F32 x;
    stream >> x;
	m_F = SIMD_Scalar( x );
    #else
    stream >> m_F;
    #endif
}

/// Write to Stream
template < typename T_Stream >
NOALIAS __forceinline void HF32::Serialise( T_Stream& stream )
{
    stream << (F32)*this;
}

/// Round to integer towards minus infinity
NOALIAS __forceinline const HF32 RoundToIntMinusInf( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_rfim( src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_vrfim( src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vrfim( src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SSE_RoundToIntMinusInf( src.m_F ) );
    #else
    return HF32( floorf( src.m_F ) );
    #endif
}

/// Round to integer towards plus infinity
NOALIAS __forceinline const HF32 RoundToIntPlusInf( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_rfip( src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_vrfip( src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vrfip( src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SSE_RoundToIntPlusInf( src.m_F ) );
    #else
    return HF32( ceilf( src.m_F ) );
    #endif
}

/// Round to integer towards nearest
NOALIAS __forceinline const HF32 RoundToIntNearest( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_rfin( src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_vrfin( src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vrfin( src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SSE_RoundToIntNearest( src.m_F ) );
    #else
    return HF32( _copysign( floorf( fabsf( src.m_F ) + 0.5f ), src.m_F ) );
    #endif
}

/// Round to integer towards zero
NOALIAS __forceinline const HF32 RoundToIntZero( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_rfiz( src.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_vrfiz( src.m_F );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vrfiz( src.m_F );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SSE_RoundToIntZero( src.m_F ) );
    #else
    return HF32( (F32)(S32)src.m_F );
    #endif
}

NOALIAS __forceinline const HF32 Floor( const Maths::HF32 PCREF f )
{
    return RoundToIntMinusInf( f );
}

NOALIAS __forceinline const HF32 Ceil( const Maths::HF32 PCREF f )
{
    return RoundToIntPlusInf( f );
}

NOALIAS __forceinline const HF32 Round( const HF32 PCREF f )
{
    return RoundToIntNearest( f );
}

NOALIAS __forceinline const HF32 Trunc( const HF32 PCREF f )
{
    return RoundToIntZero( f );
}

NOALIAS __forceinline const HF32 CopySign( const HF32 PCREF value, const HF32 PCREF sign )
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_copysign( value.m_F, sign.m_F );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector unsigned int signMask( Const< vector unsigned int >::SignMask() );
    HF32 result( UNDEFINED );
    result.m_F = vec_sel( value.m_F, sign.m_F, signMask );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 signMask( c_Vector4_SignMask );
    HF32 result( UNDEFINED );
    result.m_F = __vsel( value.m_F, sign.m_F, signMask );
    return result;
    
    #elif defined( _SSE_ )
    return HF32( SIMD_OR( SIMD_AND( value.m_F, (__m128&)c_VectorAbsMask ), SIMD_AND( sign.m_F, (__m128&)c_Vector4_SignMask ) ) );
    #else
    return HF32( _copysign( value.m_F, sign.m_F ) );
    #endif
}





/// Templated Select if a == b
//template <>
NOALIAS __forceinline const HF32 Select_EQ( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false )
{
    //return (F32)__fsel( -Abs( a - b ) /* >= 0.0f */, result_if_true, result_if_false );
    return Select( a.EQ( b ), result_if_true, result_if_false );
}

/// Templated Select if a != b
//template <>
NOALIAS __forceinline const HF32 Select_NE( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false )
{
    //return (F32)__fsel( -Abs( a - b ) /* >= 0.0f */, result_if_false, result_if_true );
    return Select( a.NE( b ), result_if_true, result_if_false );
}

/// Templated Select if a >= b
//template <>
NOALIAS __forceinline const HF32 Select_GE( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false )
{
    //return (F32)__fsel( a - b /* >= 0.0f */, result_if_true, result_if_false );
    return Select( a.GE( b ), result_if_true, result_if_false );
}

/// Templated Select if a < b
//template <>
NOALIAS __forceinline const HF32 Select_LT( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false )
{
    //return (F32)__fsel( a - b /* < 0.0f */, result_if_false, result_if_true );
    return Select( a.LT( b ), result_if_true, result_if_false );
}


/// Templated Select if a <= b
//template <>
NOALIAS __forceinline const HF32 Select_LE( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false )
{
    //return (F32)__fsel( b - a /* >= 0.0f */, result_if_true, result_if_false );
    return Select( a.LE( b ), result_if_true, result_if_false );
}

/// Templated Select if a > b
//template <>
NOALIAS __forceinline const HF32 Select_GT( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF result_if_true, const HF32 PCREF result_if_false )
{
    //return (F32)__fsel( b - a /* < 0.0f */, result_if_false, result_if_true );
    return Select( a.GT( b ), result_if_true, result_if_false );
}


NOALIAS __forceinline const HF32 UnsafeDivide( const HF32 PCREF num, const HF32 PCREF denom )
{
    #if defined( _SPU_ )
    
    const vector float A = num.m_F;
    const vector float B = denom.m_F;
    vector float       rcp_B;
    SPU_VECTOR_RECIPROCAL_UNSAFE( B, rcp_B );
    HF32 result( UNDEFINED );
    result.m_F = spu_mul( A, rcp_B );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float A = num.m_F;
    const vector float B = denom.m_F;
    vector float       rcp_B;
    PPU_VECTOR_RECIPROCAL_UNSAFE( B, rcp_B );
    HF32 result( UNDEFINED );
    result.m_F = vec_mul( A, rcp_B );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 absMask( c_VectorAbsMask );
    const __vector4 A( num.m_F );
    const __vector4 B( denom.m_F );
    const __vector4 abs_B( __vand( B, absMask ) );
    const __vector4 valid( __vcmpgtfp( abs_B, fltMin ) );
    const __vector4 y0( __vrefp( B ) );
    const __vector4 t0( __vnmsubfp( y0, B, one ) );
    const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
    const __vector4 rcp_B( __vsel( y0, y1, valid ) ); // If B not in valid range, return reciprocal estimate y0
    
    HF32 result( UNDEFINED );
    result.m_F = __vmulfp( A, rcp_B );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_UnsafeDivide( num.m_F, denom.m_F ) );
    #else
    return HF32( num.m_F / denom.m_F );
    #endif
}





//
// Promote Non-HF32 RHS Operands to HF32
//

NOALIAS __forceinline const HF32 HF32::operator + ( const F32& src ) const __restrict { return operator + ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator - ( const F32& src ) const __restrict { return operator - ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator * ( const F32& src ) const __restrict { return operator * ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator / ( const F32& src ) const __restrict { return operator / ( HF32( src ) ); }

NOALIAS __forceinline const HF32 HF32::operator + ( const U32& src ) const __restrict { return operator + ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator - ( const U32& src ) const __restrict { return operator - ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator * ( const U32& src ) const __restrict { return operator * ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator / ( const U32& src ) const __restrict { return operator / ( HF32( src ) ); }

NOALIAS __forceinline const HF32 HF32::operator + ( const S32& src ) const __restrict { return operator + ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator - ( const S32& src ) const __restrict { return operator - ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator * ( const S32& src ) const __restrict { return operator * ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator / ( const S32& src ) const __restrict { return operator / ( HF32( src ) ); }

NOALIAS __forceinline const HF32 HF32::operator + ( const U16& src ) const __restrict { return operator + ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator - ( const U16& src ) const __restrict { return operator - ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator * ( const U16& src ) const __restrict { return operator * ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator / ( const U16& src ) const __restrict { return operator / ( HF32( src ) ); }

NOALIAS __forceinline const HF32 HF32::operator + ( const S16& src ) const __restrict { return operator + ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator - ( const S16& src ) const __restrict { return operator - ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator * ( const S16& src ) const __restrict { return operator * ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator / ( const S16& src ) const __restrict { return operator / ( HF32( src ) ); }

NOALIAS __forceinline const HF32 HF32::operator + ( const U8& src ) const __restrict { return operator + ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator - ( const U8& src ) const __restrict { return operator - ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator * ( const U8& src ) const __restrict { return operator * ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator / ( const U8& src ) const __restrict { return operator / ( HF32( src ) ); }

NOALIAS __forceinline const HF32 HF32::operator + ( const S8& src ) const __restrict { return operator + ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator - ( const S8& src ) const __restrict { return operator - ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator * ( const S8& src ) const __restrict { return operator * ( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::operator / ( const S8& src ) const __restrict { return operator / ( HF32( src ) ); }


NOALIAS __forceinline const HF32& HF32::operator += ( const F32& src ) __restrict { return operator += ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator -= ( const F32& src ) __restrict { return operator -= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator *= ( const F32& src ) __restrict { return operator *= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator /= ( const F32& src ) __restrict { return operator /= ( HF32( src ) ); }

NOALIAS __forceinline const HF32& HF32::operator += ( const S32& src ) __restrict { return operator += ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator -= ( const S32& src ) __restrict { return operator -= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator *= ( const S32& src ) __restrict { return operator *= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator /= ( const S32& src ) __restrict { return operator /= ( HF32( src ) ); }

NOALIAS __forceinline const HF32& HF32::operator += ( const U32& src ) __restrict { return operator += ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator -= ( const U32& src ) __restrict { return operator -= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator *= ( const U32& src ) __restrict { return operator *= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator /= ( const U32& src ) __restrict { return operator /= ( HF32( src ) ); }

NOALIAS __forceinline const HF32& HF32::operator += ( const S16& src ) __restrict { return operator += ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator -= ( const S16& src ) __restrict { return operator -= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator *= ( const S16& src ) __restrict { return operator *= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator /= ( const S16& src ) __restrict { return operator /= ( HF32( src ) ); }

NOALIAS __forceinline const HF32& HF32::operator += ( const U16& src ) __restrict { return operator += ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator -= ( const U16& src ) __restrict { return operator -= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator *= ( const U16& src ) __restrict { return operator *= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator /= ( const U16& src ) __restrict { return operator /= ( HF32( src ) ); }

NOALIAS __forceinline const HF32& HF32::operator += ( const S8& src ) __restrict { return operator += ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator -= ( const S8& src ) __restrict { return operator -= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator *= ( const S8& src ) __restrict { return operator *= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator /= ( const S8& src ) __restrict { return operator /= ( HF32( src ) ); }

NOALIAS __forceinline const HF32& HF32::operator += ( const U8& src ) __restrict { return operator += ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator -= ( const U8& src ) __restrict { return operator -= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator *= ( const U8& src ) __restrict { return operator *= ( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::operator /= ( const U8& src ) __restrict { return operator /= ( HF32( src ) ); }


NOALIAS __forceinline const HF32 HF32::Min( const F32& src ) const __restrict { return Min( HF32( src ) ); }
NOALIAS __forceinline const HF32 HF32::Max( const F32& src ) const __restrict { return Max( HF32( src ) ); }

NOALIAS __forceinline const HF32& HF32::MinEq( const F32& src ) __restrict { return MinEq( HF32( src ) ); }
NOALIAS __forceinline const HF32& HF32::MaxEq( const F32& src ) __restrict { return MaxEq( HF32( src ) ); }


NOALIAS __forceinline const S32 HF32::operator == ( const F32& src ) const __restrict { return operator == ( HF32( src ) ); }
NOALIAS __forceinline const S32 HF32::operator != ( const F32& src ) const __restrict { return operator != ( HF32( src ) ); }
NOALIAS __forceinline const S32 HF32::operator <= ( const F32& src ) const __restrict { return operator <= ( HF32( src ) ); }
NOALIAS __forceinline const S32 HF32::operator <  ( const F32& src ) const __restrict { return operator <  ( HF32( src ) ); }
NOALIAS __forceinline const S32 HF32::operator >= ( const F32& src ) const __restrict { return operator >= ( HF32( src ) ); }
NOALIAS __forceinline const S32 HF32::operator >  ( const F32& src ) const __restrict { return operator >  ( HF32( src ) ); }


//
// Promote non-HF32 LHS operands to HF32 when in binary ops with HF32 RHS
//

NOALIAS __forceinline const HF32 operator + ( const F32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) + rhs; }
NOALIAS __forceinline const HF32 operator - ( const F32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) - rhs; }
NOALIAS __forceinline const HF32 operator * ( const F32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) * rhs; }
NOALIAS __forceinline const HF32 operator / ( const F32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) / rhs; }

NOALIAS __forceinline const HF32 operator + ( const S32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) + rhs; }
NOALIAS __forceinline const HF32 operator - ( const S32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) - rhs; }
NOALIAS __forceinline const HF32 operator * ( const S32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) * rhs; }
NOALIAS __forceinline const HF32 operator / ( const S32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) / rhs; }

NOALIAS __forceinline const HF32 operator + ( const U32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) + rhs; }
NOALIAS __forceinline const HF32 operator - ( const U32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) - rhs; }
NOALIAS __forceinline const HF32 operator * ( const U32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) * rhs; }
NOALIAS __forceinline const HF32 operator / ( const U32& lhs, const HF32 PCREF rhs ) { return HF32( lhs ) / rhs; }

//
// Update operators for non-HF32 LHS operands taking HF32 RHS operands
//

NOALIAS __forceinline void operator += ( F32& lhs, const HF32 PCREF rhs ) { lhs += (F32)rhs; }
NOALIAS __forceinline void operator -= ( F32& lhs, const HF32 PCREF rhs ) { lhs -= (F32)rhs; }
NOALIAS __forceinline void operator *= ( F32& lhs, const HF32 PCREF rhs ) { lhs *= (F32)rhs; }
NOALIAS __forceinline void operator /= ( F32& lhs, const HF32 PCREF rhs ) { lhs /= (F32)rhs; }

//
// Non-Member Support functions
//

NOALIAS __forceinline const HF32 Select( const Scalar_Bool PCREF selector, const HF32 PCREF resultIfTrue, const HF32 PCREF resultIfFalse )
{
    #if defined( _SPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = spu_sel( resultIfFalse.m_F, resultIfTrue.m_F, selector.m_V );
    return result;
    
    #elif defined( _PPU_ )
    
    HF32 result( UNDEFINED );
    result.m_F = vec_sel( resultIfFalse.m_F, resultIfTrue.m_F, selector.m_V );
    return result;
    
    #elif defined( _VMX128_ )
    
    HF32 result( UNDEFINED );
    result.m_F = __vsel( resultIfFalse.m_F, resultIfTrue.m_F, selector.m_V );
    return result;
    
    #elif defined( _SIMD_ )
    return HF32( SIMD_Select( selector.m_V, resultIfTrue.m_F, resultIfFalse.m_F ) );
    #else
    return HF32( selector.m_F != 0x00000000 ? resultIfTrue.m_F : resultIfFalse.m_F );
    #endif
}

NOALIAS __forceinline const HF32            Min( const HF32 PCREF a, const HF32 PCREF b )   { return a.Min( b ); }
NOALIAS __forceinline const HF32            Min( const F32& a, const HF32 PCREF b )         { return HF32( a ).Min( b ); }
NOALIAS __forceinline const HF32            Min( const HF32 PCREF a, const F32& b )         { return a.Min( HF32( b ) ); }

NOALIAS __forceinline const HF32            Max( const HF32 PCREF a, const HF32 PCREF b )   { return a.Max( b ); }
NOALIAS __forceinline const HF32            Max( const F32& a, const HF32 PCREF b )         { return HF32( a ).Max( b ); }
NOALIAS __forceinline const HF32            Max( const HF32 PCREF a, const F32& b )         { return a.Max( HF32( b ) ); }

NOALIAS __forceinline const HF32            Abs( const HF32 PCREF src )                  { return src.Abs(); }
NOALIAS __forceinline const HF32            Sign( const HF32 PCREF src )                 { return src.Sign(); }
NOALIAS __forceinline const HF32            Sqrt( const HF32 PCREF src )                 { return src.Sqrt(); }
NOALIAS __forceinline const HF32            Squared( const HF32 PCREF src )              { return src * src; }
NOALIAS __forceinline const HF32            Rcp( const HF32 PCREF src )                  { return src.Rcp(); }
NOALIAS __forceinline const HF32            RcpSqrt( const HF32 PCREF src )              { return src.RcpSqrt(); }
//NOALIAS __forceinline const HF32            Reciprocal( const HF32 PCREF src )           { return src.Rcp(); }
NOALIAS __forceinline const HF32            ReciprocalSqrt( const HF32 PCREF src )       { return src.RcpSqrt(); }

NOALIAS __forceinline const S32             IsNonZero( const HF32 PCREF src )            { const HF32 zero(Constants<HF32>::Zero()); return src != zero; }
NOALIAS __forceinline const S32             IsZero( const HF32 PCREF src )               { const HF32 zero(Constants<HF32>::Zero()); return src == zero; }
NOALIAS __forceinline const Scalar_Bool     Finite( const HF32 PCREF src )               { return src.Finite(); }




/// MulAdd: a * b + c
NOALIAS __forceinline const HF32 MulAdd( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_madd( a.m_F, b.m_F, c.m_F );
    #elif defined( _PPU_ )
    result.m_F = vec_madd( a.m_F, b.m_F, c.m_F );
    #elif defined( _VMX128_ )
    result.m_F = __vmaddfp( a.m_F, b.m_F, c.m_F );
    #elif defined( _SIMD_ )
    result.m_F = _mm_add_ps( _mm_mul_ps( a.m_F, b.m_F ), c.m_F );
    #else
    result.m_F = ( a.m_F * b.m_F ) + c.m_F;
    #endif
    return result;
}

/// NegMulSub: c - a * b
NOALIAS __forceinline const HF32 NegMulSub( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c )
{
    HF32 result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_F = spu_nmsub( a.m_F, b.m_F, c.m_F );
    #elif defined( _PPU_ )
    result.m_F = vec_nmsub( a.m_F, b.m_F, c.m_F );
    #elif defined( _VMX128_ )
    result.m_F = __vnmsubfp( a.m_F, b.m_F, c.m_F );
    #elif defined( _SIMD_ )
    result.m_F = _mm_sub_ps( c.m_F, _mm_mul_ps( a.m_F, b.m_F ) );
    #else
    result.m_F = c.m_F - ( a.m_F * b.m_F );
    #endif
    return result;
}

/// Reciprocal
NOALIAS __forceinline const HF32 Reciprocal( const HF32 PCREF src )
{
    HF32 result( UNDEFINED );
    
    #if defined( _SPU_ )
    
    const vector float  zero = Const< vector float >::Zero();
    const vector float  B = src.m_F;
    vector float        rcp_B;
    vector unsigned int valid;
    SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    result.m_F = rcp_B;
    
    #elif defined( _PPU_ )
    
    const vector float zero = Const< vector float >::Zero();
    const vector float B = src.m_F;
    vector float       rcp_B;
    vector bool int    valid;
    PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    result.m_F = rcp_B;
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 fltMin( c_VectorFltMin );
    const __vector4 B( src.m_F );
    const __vector4 abs_B( __vand( B, c_VectorAbsMask ) );
    const __vector4 valid( __vcmpgtfp( abs_B, fltMin ) );
    const __vector4 y0( __vrefp( B ) );
    const __vector4 t0( __vnmsubfp( y0, B, one ) );
    const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
    result.m_F = __vsel( __vzero(), y1, valid );
    
    #elif defined( _SIMD_ )
    result.m_F = SIMD_ScalarReciprocal( src.m_F );
    #else
    result.m_F = Maths::Reciprocal( src.m_F );
    #endif
    
    return result;
}




//#if 0
//
//NOALIAS __forceinline const HF32 Exp( const HF32 PCREF x )
//{
//    #if defined( _VMX128_ )
//    
//    //
//    // e^z == e^z0 Sum[k=0,inf] (z-z0)^k / k!
//    //
//    
//    XMVectorExp(  );
//    
//    HF32 result( UNDEFINED );
//    result.m_F = __vexptefp( x.m_F );
//    return result;
//    
//    #elif defined( _SSE_ )
//    
//    #else
//    return HF32( powf( 2.0f, x.m_F ) );
//    #endif
//}
//
//NOALIAS __forceinline const HF32 Log( const HF32 PCREF x )
//{
//    #if defined( _VMX128_ )
//    
//    HF32 result( UNDEFINED );
//    result.m_F = __vlogefp( x.m_F );
//    return result;
//    
//    #elif defined( _SSE_ )
//    
//    #else
//    return HF32( logf( x.m_F ) / logf( 2.0f ) );
//    #endif
//}
//
///// Calculate x^y
//NOALIAS __forceinline const HF32 Pow( const HF32 PCREF x, const HF32 PCREF y )
//{
//    #if defined( _VMX128_ )
//    
//    XMVectorPow(  );
//    
//    //
//    // x^(m+n) = x^m . x^n
//    //
//    
//    HF32 result( UNDEFINED );
//    result.m_F = __vexptefp(  );
//    return result;
//    
//    #elif defined( _SSE_ )
//    
//    #else
//    return HF32( powf( x.m_F, y.m_F ) );
//    #endif
//}
//
//#endif












//
// Hardware 1 Element F32 Vector X Component Class
//

#if defined( MATHS_DONT_INITIALISE_TO_ZERO )
/// Default Constructor
NOALIAS __forceinline HVecX::HVecX()
{
}
#else
/// Default Constructor
NOALIAS __forceinline HVecX::HVecX()
    #if defined( _SPU_ )
    : m_X( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_X( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_X( SIMD_Zero() )
    #else
    : m_X( 0.0f )
    #endif
{
}
#endif

/// Zero Init Constructor
NOALIAS __forceinline HVecX::HVecX( const _ZERO_INIT /*zeroinit*/ )
    #if defined( _SPU_ )
    : m_X( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_X( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_X( SIMD_Zero() )
    #else
    : m_X( 0.0f )
    #endif
{
}

/// Undefined Constructor
NOALIAS __forceinline HVecX::HVecX( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline HVecX::HVecX( const HVecX& copy )
    : m_X( copy.m_X )
{
}

/// HVecY Constructor
NOALIAS __forceinline HVecX::HVecX( const HVecY PCREF src )
    #if defined( _SPU_ )
    : m_X( spu_rotate_left_qw_words<1>( src.m_Y ) )
    #elif defined( _PPU_ )
    : m_X( vec_vsldoi( src.m_Y, src.m_Y, 1 << 2 ) )
    #elif defined( _SIMD_ )
    : m_X( SIMD_VectorX_VectorY( src.m_Y ) )
    #else
    : m_X( src.m_Y )
    #endif
{
}

/// HVecZ Constructor
NOALIAS __forceinline HVecX::HVecX( const HVecZ PCREF src )
    #if defined( _SPU_ )
    : m_X( spu_rotate_left_qw_words<2>( src.m_Z ) )
    #elif defined( _PPU_ )
    : m_X( vec_vsldoi( src.m_Z, src.m_Z, 2 << 2 ) )
    #elif defined( _SIMD_ )
    : m_X( SIMD_VectorX_VectorZ( src.m_Z ) )
    #else
    : m_X( src.m_Z )
    #endif
{
}

/// HVecW Constructor
NOALIAS __forceinline HVecX::HVecX( const HVecW PCREF src )
    #if defined( _SPU_ )
    : m_X( spu_rotate_left_qw_words<3>( src.m_W ) )
    #elif defined( _PPU_ )
    : m_X( vec_vsldoi( src.m_W, src.m_W, 3 << 2 ) )
    #elif defined( _SIMD_ )
    : m_X( SIMD_VectorX_VectorW( src.m_W ) )
    #else
    : m_X( src.m_W )
    #endif
{
}

/// HVec2 Constructor : PS3FIX
NOALIAS __forceinline HVecX::HVecX( const HVec2 PCREF src )
    #if defined( _SPU_ )
    : m_X( src.m_V )
    #elif defined( _PPU_ )
    : m_X( src.m_V )
    #elif defined( _SIMD_ )
    : m_X( SIMD_VectorX_Vector2( src.m_V ) )
    #else
    : m_X( src.m_V[VX] )
    #endif
{
}

/// HVec3 Constructor
NOALIAS __forceinline HVecX::HVecX( const HVec3 PCREF src )
    #if defined( _SPU_ )
    : m_X( src.m_V )
    #elif defined( _PPU_ )
    : m_X( src.m_V )
    #elif defined( _SIMD_ )
    : m_X( SIMD_VectorX_Vector3( src.m_V ) )
    #else
    : m_X( src.m_V[VX] )
    #endif
{
}

/// HF32 Constructor
NOALIAS __forceinline HVecX::HVecX( const HF32 PCREF src )
    #if defined( _SPU_ )
    : m_X( src.m_F )
    #elif defined( _PPU_ )
    : m_X( src.m_F )
    #elif defined( _SIMD_ )
    : m_X( SIMD_VectorX_Scalar( src.m_F ) )
    #else
    : m_X( src.m_F )
    #endif
{
}

#if defined( _SIMD_ )
/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVecX::HVecX( const V128 PCREF src )
    : m_X( src )
{
}
#else
/// F32 Array Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVecX::HVecX( const F32* ptr )
    : m_X( ptr[VX] )
{
}
#endif

/// Copy Assignment
NOALIAS __forceinline const HVecX& HVecX::operator = ( const HVecX& copy )
{
    m_X = copy.m_X;
    return copy;
}

/// HF32 Assignment
NOALIAS __forceinline const HVecX& HVecX::operator = ( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_X = src.m_F;
    #elif defined( _PPU_ )
    m_X = src.m_F;
    #elif defined( _SIMD_ )
    m_X = SIMD_VectorX_Scalar( src.m_F );
    #else
    m_X = src.m_F;
    #endif
    return *this;
}

/// Accessor: Set HF32
NOALIAS __forceinline void HVecX::X( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_X = src.m_F;
    #elif defined( _PPU_ )
    m_X = src.m_F;
    #elif defined( _SIMD_ )
    m_X = SIMD_VectorX_Scalar( src.m_F );
    #else
    m_X = src.m_F;
    #endif
}

/// Accessor: Get HF32
NOALIAS __forceinline const HF32 HVecX::X() const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VX>( m_X );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_X, VX );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_VectorX( m_X ) );
    #else
    return HF32( m_X );
    #endif
}

/// Debug validation
NOALIAS __forceinline void HVecX::DebugValidate() const
{
    #if defined( MATHS_DEBUG_MAX )
    #if defined( _VMX128_ )
    U32 CR1, CR2, CR3;
    __vcmpeqfpR( __vzero(), __vspltw( m_X, VY ), &CR1 );
    __vcmpeqfpR( __vzero(), __vspltw( m_X, VZ ), &CR2 );
    __vcmpeqfpR( __vzero(), __vspltw( m_X, VW ), &CR3 );
    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
    {
        DebugBreak();
    }
    #elif defined( _SSE_ )
    if ( ( m_X.m128_f32[VY] != 0.0f ) ||
         ( m_X.m128_f32[VZ] != 0.0f ) ||
         ( m_X.m128_f32[VW] != 0.0f ) )
    {
        DebugBreak();
    }
    #endif
    #endif
}








//
// Hardware 1 Element F32 Vector Y Component Class
//

#if defined( MATHS_DONT_INITIALISE_TO_ZERO )
/// Default Constructor
NOALIAS __forceinline HVecY::HVecY()
{
}
#else
/// Default Constructor
NOALIAS __forceinline HVecY::HVecY()
    #if defined( _SPU_ )
    : m_Y( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_Y( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_Y( SIMD_Zero() )
    #else
    : m_Y( 0.0f )
    #endif
{
}
#endif

/// Zero Init Constructor
NOALIAS __forceinline HVecY::HVecY( const _ZERO_INIT /*zeroinit*/ )
    #if defined( _SPU_ )
    : m_Y( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_Y( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_Y( SIMD_Zero() )
    #else
    : m_Y( 0.0f )
    #endif
{
}

/// Undefined Constructor
NOALIAS __forceinline HVecY::HVecY( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline HVecY::HVecY( const HVecY& copy )
    : m_Y( copy.m_Y )
{
}

/// HVecX Constructor
NOALIAS __forceinline HVecY::HVecY( const HVecX PCREF src )
    #if defined( _SPU_ )
    : m_Y( spu_rotate_left_qw_words<3>( src.m_X ) )
    #elif defined( _PPU_ )
    : m_Y( vec_vsldoi( src.m_X, src.m_X, 3 << 2 ) )
    #elif defined( _SIMD_ )
    : m_Y( SIMD_VectorY_VectorX( src.m_X ) )
    #else
    : m_Y( src.m_X )
    #endif
{
}

/// HVecZ Constructor
NOALIAS __forceinline HVecY::HVecY( const HVecZ PCREF src )
    #if defined( _SPU_ )
    : m_Y( spu_rotate_left_qw_words<1>( src.m_Z ) )
    #elif defined( _PPU_ )
    : m_Y( vec_vsldoi( src.m_Z, src.m_Z, 1 << 2 ) )
    #elif defined( _SIMD_ )
    : m_Y( SIMD_VectorY_VectorZ( src.m_Z ) )
    #else
    : m_Y( src.m_Z )
    #endif
{
}

/// HVecW Constructor
NOALIAS __forceinline HVecY::HVecY( const HVecW PCREF src )
    #if defined( _SPU_ )
    : m_Y( spu_rotate_left_qw_words<2>( src.m_W ) )
    #elif defined( _PPU_ )
    : m_Y( vec_vsldoi( src.m_W, src.m_W, 2 << 2 ) )
    #elif defined( _SIMD_ )
    : m_Y( SIMD_VectorY_VectorW( src.m_W ) )
    #else
    : m_Y( src.m_W )
    #endif
{
}

/// HVec2 Constructor : PS3FIX
NOALIAS __forceinline HVecY::HVecY( const HVec2 PCREF src )
    #if defined( _SPU_ )
    : m_Y( src.m_V )
    #elif defined( _PPU_ )
    : m_Y( src.m_V )
    #elif defined( _SIMD_ )
    : m_Y( SIMD_VectorY_Vector2( src.m_V ) )
    #else
    : m_Y( src.m_V[VY] )
    #endif
{
}

/// HVec3 Constructor
NOALIAS __forceinline HVecY::HVecY( const HVec3 PCREF src )
    #if defined( _SPU_ )
    : m_Y( src.m_V )
    #elif defined( _PPU_ )
    : m_Y( src.m_V )
    #elif defined( _SIMD_ )
    : m_Y( SIMD_VectorY_Vector3( src.m_V ) )
    #else
    : m_Y( src.m_V[VY] )
    #endif
{
}

/// HF32 Constructor
NOALIAS __forceinline HVecY::HVecY( const HF32 PCREF src )
    #if defined( _SPU_ )
    : m_Y( src.m_F )
    #elif defined( _PPU_ )
    : m_Y( src.m_F )
    #elif defined( _SIMD_ )
    : m_Y( SIMD_VectorY_Scalar( src.m_F ) )
    #else
    : m_Y( src.m_F )
    #endif
{
}

#if defined( _SIMD_ )
/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVecY::HVecY( const V128 PCREF src )
    : m_Y( src )
{
}
#else
/// F32 Array Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVecY::HVecY( const F32* ptr )
    : m_Y( ptr[VY] )
{
}
#endif

/// Copy Assignment
NOALIAS __forceinline const HVecY& HVecY::operator = ( const HVecY& copy )
{
    m_Y = copy.m_Y;
    return copy;
}

/// HF32 Assignment
NOALIAS __forceinline const HVecY& HVecY::operator = ( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_Y = src.m_F;
    #elif defined( _PPU_ )
    m_Y = src.m_F;
    #elif defined( _SIMD_ )
    m_Y = SIMD_VectorY_Scalar( src.m_F );
    #else
    m_Y = src.m_F;
    #endif
    
    return *this;
}

/// Accessor: Set HF32
NOALIAS __forceinline void HVecY::Y( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_Y = src.m_F;
    #elif defined( _PPU_ )
    m_Y = src.m_F;
    #elif defined( _SIMD_ )
    m_Y = SIMD_VectorY_Scalar( src.m_F );
    #else
    m_Y = src.m_F;
    #endif
}

/// Accessor: Get HF32
NOALIAS __forceinline const HF32 HVecY::Y() const
{
    #if defined( _SPU_ )
    HF32 result( UNDEFINED );
    result.m_F = spu_splat<VY>( m_Y );
    return result;
    #elif defined( _PPU_ )
    HF32 result( UNDEFINED );
    result.m_F = vec_splat( m_Y, VY );
    return result;
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_VectorY( m_Y ) );
    #else
    return HF32( m_Y );
    #endif
}

/// Debug validation
NOALIAS __forceinline void HVecY::DebugValidate() const
{
    #if defined( MATHS_DEBUG_MAX )
    #if defined( _VMX128_ )
    U32 CR1, CR2, CR3;
    __vcmpeqfpR( __vzero(), __vspltw( m_Y, VX ), &CR1 );
    __vcmpeqfpR( __vzero(), __vspltw( m_Y, VZ ), &CR2 );
    __vcmpeqfpR( __vzero(), __vspltw( m_Y, VW ), &CR3 );
    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
    {
        DebugBreak();
    }
    #elif defined( _SSE_ )
    if ( ( m_Y.m128_f32[VX] != 0.0f ) ||
         ( m_Y.m128_f32[VZ] != 0.0f ) ||
         ( m_Y.m128_f32[VW] != 0.0f ) )
    {
        DebugBreak();
    }
    #endif
    #endif
}









//
// Hardware 1 Element F32 Vector Z Component Class
//

#if defined( MATHS_DONT_INITIALISE_TO_ZERO )
/// Default Constructor
NOALIAS __forceinline HVecZ::HVecZ()
{
}
#else
/// Default Constructor
NOALIAS __forceinline HVecZ::HVecZ()
    #if defined( _SPU_ )
    : m_Z( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_Z( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_Z( SIMD_Zero() )
    #else
    : m_Z( 0.0f )
    #endif
{
}
#endif

/// Zero Init Constructor
NOALIAS __forceinline HVecZ::HVecZ( const _ZERO_INIT /*zeroinit*/ )
    #if defined( _SPU_ )
    : m_Z( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_Z( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_Z( SIMD_Zero() )
    #else
    : m_Z( 0.0f )
    #endif
{
}

/// Undefined Constructor
NOALIAS __forceinline HVecZ::HVecZ( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline HVecZ::HVecZ( const HVecZ& copy )
    : m_Z( copy.m_Z )
{
}

/// HVecX Constructor
NOALIAS __forceinline HVecZ::HVecZ( const HVecX PCREF src )
    #if defined( _SPU_ )
    : m_Z( spu_rotate_left_qw_words<2>( src.m_X ) )
    #elif defined( _PPU_ )
    : m_Z( vec_vsldoi( src.m_X, src.m_X, 2 << 2 ) )
    #elif defined( _SIMD_ )
    : m_Z( SIMD_VectorZ_VectorX( src.m_X ) )
    #else
    : m_Z( src.m_X )
    #endif
{
}

/// HVecY Constructor
NOALIAS __forceinline HVecZ::HVecZ( const HVecY PCREF src )
    #if defined( _SPU_ )
    : m_Z( spu_rotate_left_qw_words<3>( src.m_Y ) )
    #elif defined( _PPU_ )
    : m_Z( vec_vsldoi( src.m_Y, src.m_Y, 3 << 2 ) )
    #elif defined( _SIMD_ )
    : m_Z( SIMD_VectorZ_VectorY( src.m_Y ) )
    #else
    : m_Z( src.m_Y )
    #endif
{
}

/// HVecW Constructor
NOALIAS __forceinline HVecZ::HVecZ( const HVecW PCREF src )
    #if defined( _SPU_ )
    : m_Z( spu_rotate_left_qw_words<1>( src.m_W ) )
    #elif defined( _PPU_ )
    : m_Z( vec_vsldoi( src.m_W, src.m_W, 1 << 2 ) )
    #elif defined( _SIMD_ )
    : m_Z( SIMD_VectorZ_VectorW( src.m_W ) )
    #else
    : m_Z( src.m_W )
    #endif
{
}

/// HVec3 Constructor
NOALIAS __forceinline HVecZ::HVecZ( const HVec3 PCREF src )
    #if defined( _SPU_ )
    : m_Z( src.m_V )
    #elif defined( _PPU_ )
    : m_Z( src.m_V )
    #elif defined( _SIMD_ )
    : m_Z( SIMD_VectorZ_Vector3( src.m_V ) )
    #else
    : m_Z( src.m_V[VZ] )
    #endif
{
}

/// HF32 Constructor
NOALIAS __forceinline HVecZ::HVecZ( const HF32 PCREF src )
    #if defined( _SPU_ )
    : m_Z( src.m_F )
    #elif defined( _PPU_ )
    : m_Z( src.m_F )
    #elif defined( _SIMD_ )
    : m_Z( SIMD_VectorZ_Scalar( src.m_F ) )
    #else
    : m_Z( src.m_F )
    #endif
{
}

#if defined( _SIMD_ )
/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVecZ::HVecZ( const V128 PCREF src )
    : m_Z( src )
{
}
#else
/// F32 Array Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVecZ::HVecZ( const F32* ptr )
    : m_Z( ptr[VZ] )
{
}
#endif

/// Copy Assignment
NOALIAS __forceinline const HVecZ& HVecZ::operator = ( const HVecZ& copy )
{
    m_Z = copy.m_Z;
    return copy;
}

/// HF32 Assignment
NOALIAS __forceinline const HVecZ& HVecZ::operator = ( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_Z = src.m_F;
    #elif defined( _PPU_ )
    m_Z = src.m_F;
    #elif defined( _SIMD_ )
    m_Z = SIMD_VectorZ_Scalar( src.m_F );
    #else
    m_Z = src.m_F;
    #endif
    return *this;
}

/// Accessor: Set HF32
NOALIAS __forceinline void HVecZ::Z( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_Z = src.m_F;
    #elif defined( _PPU_ )
    m_Z = src.m_F;
    #elif defined( _SIMD_ )
    m_Z = SIMD_VectorZ_Scalar( src.m_F );
    #else
    m_Z = src.m_F;
    #endif
}

/// Accessor: Get HF32
NOALIAS __forceinline const HF32 HVecZ::Z() const
{
    #if defined( _SPU_ )
    return HF32( spu_splat<VZ>( m_Z ) );
    #elif defined( _PPU_ )
    return HF32( vec_splat( m_Z, VZ ) );
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_VectorZ( m_Z ) );
    #else
    return HF32( m_Z );
    #endif
}

/// Debug validation
NOALIAS __forceinline void HVecZ::DebugValidate() const
{
    #if defined( MATHS_DEBUG_MAX )
    #if defined( _VMX128_ )
    U32 CR1, CR2, CR3;
    __vcmpeqfpR( __vzero(), __vspltw( m_Z, VX ), &CR1 );
    __vcmpeqfpR( __vzero(), __vspltw( m_Z, VY ), &CR2 );
    __vcmpeqfpR( __vzero(), __vspltw( m_Z, VW ), &CR3 );
    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
    {
        DebugBreak();
    }
    #elif defined( _SSE_ )
    if ( ( m_Z.m128_f32[VX] != 0.0f ) ||
         ( m_Z.m128_f32[VY] != 0.0f ) ||
         ( m_Z.m128_f32[VW] != 0.0f ) )
    {
        DebugBreak();
    }
    #endif
    #endif
}











//
// Hardware 1 Element F32 Vector W Component Class
//

#if defined( MATHS_DONT_INITIALISE_TO_ZERO )
/// Default Constructor
NOALIAS __forceinline HVecW::HVecW()
{
}
#else
/// Default Constructor
NOALIAS __forceinline HVecW::HVecW()
    #if defined( _SPU_ )
    : m_W( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_W( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_W( SIMD_Zero() )
    #else
    : m_W( 0.0f )
    #endif
{
}
#endif

/// Zero Init Constructor
NOALIAS __forceinline HVecW::HVecW( const _ZERO_INIT /*zeroinit*/ )
    #if defined( _SPU_ )
    : m_W( Const< vector float >::Zero() )
    #elif defined( _PPU_ )
    : m_W( Const< vector float >::Zero() )
    #elif defined( _SIMD_ )
    : m_W( SIMD_Zero() )
    #else
    : m_W( 0.0f )
    #endif
{
}

/// Undefined Constructor
NOALIAS __forceinline HVecW::HVecW( const _UNDEFINED /*undefined*/ )
{
}

/// Copy Constructor
NOALIAS __forceinline HVecW::HVecW( const HVecW& copy )
    : m_W( copy.m_W )
{
}

/// HVecX Constructor
NOALIAS __forceinline HVecW::HVecW( const HVecX PCREF src )
    #if defined( _SPU_ )
    : m_W( spu_rotate_left_qw_words<1>( src.m_X ) )
    #elif defined( _PPU_ )
    : m_W( vec_vsldoi( src.m_X, src.m_X, 1 << 2 ) )
    #elif defined( _SIMD_ )
    : m_W( SIMD_VectorW_VectorX( src.m_X ) )
    #else
    : m_W( src.m_X )
    #endif
{
}

/// HVecY Constructor
NOALIAS __forceinline HVecW::HVecW( const HVecY PCREF src )
    #if defined( _SPU_ )
    : m_W( spu_rotate_left_qw_words<2>( src.m_Y ) )
    #elif defined( _PPU_ )
    : m_W( vec_vsldoi( src.m_Y, src.m_Y, 2 << 2 ) )
    #elif defined( _SIMD_ )
    : m_W( SIMD_VectorW_VectorY( src.m_Y ) )
    #else
    : m_W( src.m_Y )
    #endif
{
}

/// HVecZ Constructor
NOALIAS __forceinline HVecW::HVecW( const HVecZ PCREF src )
    #if defined( _SPU_ )
    : m_W( spu_rotate_left_qw_words<3>( src.m_Z ) )
    #elif defined( _PPU_ )
    : m_W( vec_vsldoi( src.m_Z, src.m_Z, 3 << 2 ) )
    #elif defined( _SIMD_ )
    : m_W( SIMD_VectorW_VectorZ( src.m_Z ) )
    #else
    : m_W( src.m_Z )
    #endif
{
}

/// Explicit HVec4 constructor
NOALIAS __forceinline HVecW::HVecW( const HVec4 PCREF src )
    #if defined( _SPU_ )
    : m_W( src.m_V )
    #elif defined( _PPU_ )
    : m_W( src.m_V )
    #elif defined( _SIMD_ )
    : m_W( SIMD_VectorW_Vector4( src.m_V ) )
    #else
    : m_W( src.m_V[VW] )
    #endif
{
}

/// HF32 Constructor
NOALIAS __forceinline HVecW::HVecW( const HF32 PCREF src )
    #if defined( _SPU_ )
    : m_W( src.m_F )
    #elif defined( _PPU_ )
    : m_W( src.m_F )
    #elif defined( _SIMD_ )
    : m_W( SIMD_VectorW_Scalar( src.m_F ) )
    #else
    : m_W( src.m_F )
    #endif
{
}

#if defined( _SIMD_ )
/// V128 Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVecW::HVecW( const V128 PCREF src )
    : m_W( src )
{
}
#else
/// F32 Array Constructor: Explicit construction from unknown data!
NOALIAS __forceinline HVecW::HVecW( const F32* ptr )
    : m_W( ptr[VW] )
{
}
#endif

/// Copy Assignment
NOALIAS __forceinline const HVecW& HVecW::operator = ( const HVecW& copy )
{
    m_W = copy.m_W;
    return copy;
}

/// HF32 Assignment
NOALIAS __forceinline const HVecW& HVecW::operator = ( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_W = src.m_F;
    #elif defined( _PPU_ )
    m_W = src.m_F;
    #elif defined( _SIMD_ )
    m_W = SIMD_VectorW_Scalar( src.m_F );
    #else
    m_W = src.m_F;
    #endif
    return *this;
}

/// Accessor: Set HF32
NOALIAS __forceinline void HVecW::W( const HF32 PCREF src )
{
    #if defined( _SPU_ )
    m_W = src.m_F;
    #elif defined( _PPU_ )
    m_W = src.m_F;
    #elif defined( _SIMD_ )
    m_W = SIMD_VectorW_Scalar( src.m_F );
    #else
    m_W = src.m_F;
    #endif
}

/// Accessor: Get HF32
NOALIAS __forceinline const HF32 HVecW::W() const
{
    #if defined( _SPU_ )
    return HF32( spu_splat<VW>( m_W ) );
    #elif defined( _PPU_ )
    return HF32( vec_splat( m_W, VW ) );
    #elif defined( _SIMD_ )
    return HF32( SIMD_Scalar_VectorW( m_W ) );
    #else
    return HF32( m_W );
    #endif
}

/// Debug validation
NOALIAS __forceinline void HVecW::DebugValidate() const
{
    #if defined( MATHS_DEBUG_MAX )
    #if defined( _VMX128_ )
    U32 CR1, CR2, CR3;
    __vcmpeqfpR( __vzero(), __vspltw( m_W, VX ), &CR1 );
    __vcmpeqfpR( __vzero(), __vspltw( m_W, VY ), &CR2 );
    __vcmpeqfpR( __vzero(), __vspltw( m_W, VZ ), &CR3 );
    if ( !( CR1 & CR2 & CR3 & VMX_CR6_ALL_TRUE ) )
    {
        DebugBreak();
    }
    #elif defined( _SSE_ )
    if ( ( m_W.m128_f32[VX] != 0.0f ) ||
         ( m_W.m128_f32[VY] != 0.0f ) ||
         ( m_W.m128_f32[VZ] != 0.0f ) )
    {
        DebugBreak();
    }
    #endif
    #endif
}



}; // namespace Maths



NOALIAS __forceinline const Maths::HF32 abs( const Maths::HF32 PCREF a )
{
    return Maths::Abs( a );
}



#endif
