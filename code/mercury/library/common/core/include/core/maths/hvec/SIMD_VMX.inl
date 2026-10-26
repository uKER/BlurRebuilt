#ifndef MATHS_HVEC_SIMD_VMX_INL
#define MATHS_HVEC_SIMD_VMX_INL

#ifdef _MSC_VER
#pragma once
#endif

#if defined( PLATFORM_PS3 )
#include <alloca.h>
#endif

#if !defined( PLATFORM_SPU )
#include "memory.h"
#endif

namespace Maths
{
#if defined( _VMX128_ )

//
// VMX & VMX128 SIMD Scalar & Vector Maths Macros
//


#define vmx_merge_xy __vmrghw
#define vmx_merge_zw __vmrglw


NOALIAS __forceinline const __vector4 vmx_not( const __vector4 a )
{
    return __vnor( a, a );
}

#define vmx_or   __vor
#define vmx_and  __vand
#define vmx_xor  __vxor
#define vmx_nor  __vnor
#define vmx_andc __vandc

NOALIAS __forceinline const __vector4 vmx_nand( const __vector4 a, const __vector4 b )
{
    return vmx_not( vmx_and( a, b ) );
}

NOALIAS __forceinline const __vector4 vmx_eqv( const __vector4 a, const __vector4 b )
{
    return vmx_not( vmx_xor( a, b ) );
}

NOALIAS __forceinline const __vector4 vmx_orc( const __vector4 a, const __vector4 b )
{
    return vmx_or( a, vmx_not( b ) );
}





template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7,
           unsigned char I8, unsigned char I9, unsigned char IA, unsigned char IB,
           unsigned char IC, unsigned char ID, unsigned char IE, unsigned char IF >
class PermuteByte
{
public:
    static const __vector4 Pattern;
};

template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7,
           unsigned char I8, unsigned char I9, unsigned char IA, unsigned char IB,
           unsigned char IC, unsigned char ID, unsigned char IE, unsigned char IF >
const __vector4 PermuteByte<I0,I1,I2,I3,I4,I5,I6,I7,I8,I9,IA,IB,IC,ID,IE,IF>::Pattern =
{
    TypePun_IntToFloat( I0, I1, I2, I3 ),
    TypePun_IntToFloat( I4, I5, I6, I7 ),
    TypePun_IntToFloat( I8, I9, IA, IB ),
    TypePun_IntToFloat( IC, ID, IE, IF )
};


template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7 >
class PermuteHalf
{
public:
    static const __vector4 Pattern;
};

template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7 >
const __vector4 PermuteHalf<I0,I1,I2,I3,I4,I5,I6,I7>::Pattern =
{
    TypePun_IntToFloat( (I0<<1)+0, (I0<<1)+1, (I1<<1)+0, (I1<<1)+1 ),
    TypePun_IntToFloat( (I2<<1)+0, (I2<<1)+1, (I3<<1)+0, (I3<<1)+1 ),
    TypePun_IntToFloat( (I4<<1)+0, (I4<<1)+1, (I5<<1)+0, (I5<<1)+1 ),
    TypePun_IntToFloat( (I6<<1)+0, (I6<<1)+1, (I7<<1)+0, (I7<<1)+1 )
};


template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW >
class PermuteWord
{
public:
    static const __vector4 Pattern;
};

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW >
const __vector4 PermuteWord<IX,IY,IZ,IW>::Pattern =
{
    TypePun_IntToFloat( (IX<<2)+0, (IX<<2)+1, (IX<<2)+2, (IX<<2)+3 ),
    TypePun_IntToFloat( (IY<<2)+0, (IY<<2)+1, (IY<<2)+2, (IY<<2)+3 ),
    TypePun_IntToFloat( (IZ<<2)+0, (IZ<<2)+1, (IZ<<2)+2, (IZ<<2)+3 ),
    TypePun_IntToFloat( (IW<<2)+0, (IW<<2)+1, (IW<<2)+2, (IW<<2)+3 )
};


/// Make vector3 from scalar x, y & z : the X component of each scalar must contain the value. (splatted scalars not required)
NOALIAS __forceinline const __vector4 vmx_make_vector3( const __vector4 x, const __vector4 y, const __vector4 z )
{
    return vmx_merge_xy( vmx_merge_xy( x, z ), y ); // NOTE: w component is UNDEFINED, not zero!
}

/// Make vector4 from scalar x, y, z & w : the X component of each scalar must contain the value. (splatted scalars not required)
NOALIAS __forceinline const __vector4 vmx_make_vector4( const __vector4 x, const __vector4 y, const __vector4 z, const __vector4 w )
{
    return vmx_merge_xy( vmx_merge_xy( x, z ),
                         vmx_merge_xy( y, w ) );
}

NOALIAS __forceinline const __vector4 vmx_horizontal_or_int32( const __vector4 v )
{
    return __vor( __vor( v,
                         __vsldoi( v, v, 0x4 ) ),
                  __vor( __vsldoi( v, v, 0x8 ),
                         __vsldoi( v, v, 0xC ) ) );
}

NOALIAS __forceinline const __vector4 vmx_horizontal_or_int16( const __vector4 a )
{
    const __vector4 b( __vor( a, __vsldoi( a, a, 0x8 ) ) );
    const __vector4 c( __vor( b, __vsldoi( b, b, 0x4 ) ) );
    const __vector4 d( __vor( c, __vsldoi( c, c, 0x2 ) ) );
    return d;
}

NOALIAS __forceinline const __vector4 vmx_horizontal_or_int8( const __vector4 a )
{
    const __vector4 b( __vor( a, __vsldoi( a, a, 0x8 ) ) );
    const __vector4 c( __vor( b, __vsldoi( b, b, 0x4 ) ) );
    const __vector4 d( __vor( c, __vsldoi( c, c, 0x2 ) ) );
    const __vector4 e( __vor( d, __vsldoi( d, d, 0x1 ) ) );
    return e;
}

NOALIAS __forceinline const __vector4 vmx_horizontal_and_int32( const __vector4 v )
{
    return __vand( __vand( v,
                           __vsldoi( v, v, 0x4 ) ),
                   __vand( __vsldoi( v, v, 0x8 ),
                           __vsldoi( v, v, 0xC ) ) );
}

NOALIAS __forceinline const __vector4 vmx_horizontal_and_int16( const __vector4 a )
{
    const __vector4 b( __vand( a, __vsldoi( a, a, 0x8 ) ) );
    const __vector4 c( __vand( b, __vsldoi( b, b, 0x4 ) ) );
    const __vector4 d( __vand( c, __vsldoi( c, c, 0x2 ) ) );
    return d;
}

NOALIAS __forceinline const __vector4 vmx_horizontal_and_int8( const __vector4 a )
{
    const __vector4 b( __vand( a, __vsldoi( a, a, 0x8 ) ) );
    const __vector4 c( __vand( b, __vsldoi( b, b, 0x4 ) ) );
    const __vector4 d( __vand( c, __vsldoi( c, c, 0x2 ) ) );
    const __vector4 e( __vand( d, __vsldoi( d, d, 0x1 ) ) );
    return e;
}

template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const __vector4 vmx_permute( const __vector4 v )
{
    StaticAssert( IX < 4, IX_MUST_BE_LESS_THAN_4 );
    StaticAssert( IY < 4, IY_MUST_BE_LESS_THAN_4 );
    StaticAssert( IZ < 4, IZ_MUST_BE_LESS_THAN_4 );
    StaticAssert( IW < 4, IW_MUST_BE_LESS_THAN_4 );
    
    return __vpermwi( v, VPERMWI_CONST( IX, IY, IZ, IW ) );
}

template < unsigned int IX, unsigned int IY, unsigned int IZ, unsigned int IW >
NOALIAS __forceinline const __vector4 vmx_permute( const __vector4 a, const __vector4 b )
{
    StaticAssert( IX < 8, IX_MUST_BE_LESS_THAN_8 );
    StaticAssert( IY < 8, IY_MUST_BE_LESS_THAN_8 );
    StaticAssert( IZ < 8, IZ_MUST_BE_LESS_THAN_8 );
    StaticAssert( IW < 8, IW_MUST_BE_LESS_THAN_8 );
    
    return __vperm( a, b, PermuteWord<IX,IY,IZ,IW>::Pattern );
}

template < unsigned char I0, unsigned char I1, unsigned char I2 >
NOALIAS __forceinline const __vector4 vmx_permute( const __vector4 v0, const __vector4 v1, const __vector4 v2 )
{
    StaticAssert( I0 < 4, I0_MUST_BE_LESS_THAN_4 );
    StaticAssert( I1 < 4, I1_MUST_BE_LESS_THAN_4 );
    StaticAssert( I2 < 4, I2_MUST_BE_LESS_THAN_4 );
    
    const __vector4 zero = Const< __vector4 >::Zero();
    const __vector4 x = __vspltw( v0, I0 );
    const __vector4 y = __vspltw( v1, I1 );
    const __vector4 z = __vspltw( v2, I2 );
    
    return vmx_merge_xy( vmx_merge_xy( x, z ), vmx_merge_xy( y, zero ) );
}


template < int RotateLeft >
NOALIAS __forceinline const __vector4 vmx_rotate_left_qw_words( const __vector4 a )
{
    #if 1
    __vector4 result = a;
    return __vrlimi( result, result, VMX_MASK_INSERT_XYZW, RotateLeft );
    #else
    return __vsldoi( a, a, RotateLeft << 2 );
    #endif
}

template < int RotateRight >
NOALIAS __forceinline const __vector4 vmx_rotate_right_qw_words( const __vector4 a )
{
    #if 1
    __vector4 result = a;
    return __vrlimi( result, result, VMX_MASK_INSERT_XYZW, ( 4 - RotateRight ) & 3 );
    #else
    return __vsldoi( a, a, ((4-RotateRight)&3) << 2 );
    #endif
}


//#define vmx_load_aligned(pSrc) __lvx( pSrc, 0x00 )
template < typename T >
NOALIAS __forceinline const __vector4 vmx_load_aligned( T const * const __restrict pSrc )
{
    return __lvx( pSrc, 0x00 );
}

//#define vmx_store_aligned_vec4(src,pDst) __stvx( src, pDst, 0x00 );
template < typename T >
NOALIAS __forceinline void vmx_store_aligned_vec4( const __vector4 src, T* const __restrict pDst )
{
    __stvx( src, pDst, 0x00 );
}

//#define vmx_load_unaligned(pSrc) __vor( __lvlx( pSrc, 0x00 ), __lvrx( pSrc, 0x10 ) )
template < typename T >
NOALIAS __forceinline const __vector4 vmx_load_unaligned( T const * const __restrict pSrc )
{
    const __vector4 a( __lvlx( pSrc, 0x00 ) );
    const __vector4 b( __lvrx( pSrc, 0x10 ) );
    return __vor( a, b );
}

// NOTE: Can't convert this macro into a function since 'offset' must be a compile time constant!
#define vmx_load_unaligned_offset(pSrc,offset) \
    __vor( __lvlx( pSrc, 0x00 + (offset) ),    \
           __lvrx( pSrc, 0x10 + (offset) ) )

//#define vmx_store_unaligned_vec4(src,pDst) __stvlx( src, pDst, 0x00 ); __stvrx( src, pDst, 0x10 );
template < typename T >
NOALIAS __forceinline void vmx_store_unaligned_vec4( const __vector4 src, T* const __restrict pDst )
{
    ReadWriteBarrier();
    __stvlx( src, pDst, 0x00 ); ReadWriteBarrier();
    __stvrx( src, pDst, 0x10 ); ReadWriteBarrier();
}

//#define vmx_store_unaligned_vec3(src,pDst) __stvewx( __vspltw( src, VX ), pDst, 0x0 ); \
//                                           __stvewx( __vspltw( src, VY ), pDst, 0x4 ); \
//                                           __stvewx( __vspltw( src, VZ ), pDst, 0x8 );
template < typename T >
NOALIAS __forceinline void vmx_store_unaligned_vec3( const __vector4 src, T* const __restrict pDst )
{
    ReadWriteBarrier();
    __stvewx( __vspltw( src, VX ), pDst, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VY ), pDst, 0x4 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VZ ), pDst, 0x8 ); ReadWriteBarrier();
}

//#define vmx_store_unaligned_vec2(src,pDst) __stvewx( __vspltw( src, VX ), pDst, 0x0 ); \
//                                           __stvewx( __vspltw( src, VY ), pDst, 0x4 );
template < typename T >
NOALIAS __forceinline void vmx_store_unaligned_vec2( const __vector4 src, T* const __restrict pDst )
{
    ReadWriteBarrier();
    __stvewx( __vspltw( src, VX ), pDst, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VY ), pDst, 0x4 ); ReadWriteBarrier();
}

//#define vmx_store_unaligned_vec1(src,pDst) __stvewx( __vspltw( src, VX ), pDst, 0x0 );
template < typename T >
NOALIAS __forceinline void vmx_store_unaligned_vec1( const __vector4 src, T* const __restrict pDst )
{
    ReadWriteBarrier();
    __stvewx( __vspltw( src, VX ), pDst, 0x0 ); ReadWriteBarrier();
}


template < typename T >
NOALIAS __forceinline void vmx_store_uncached_vec4( const __vector4 src, T* const __restrict pDst )
{
    ReadWriteBarrier();
    __stvewx( __vspltw( src, VX ), pDst, 0x0 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VY ), pDst, 0x4 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VZ ), pDst, 0x8 ); ReadWriteBarrier();
    __stvewx( __vspltw( src, VW ), pDst, 0xC ); ReadWriteBarrier();
}



// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define VMX_VECTOR_RECIPROCAL( B, invalid_result, out_rcp_B, out_valid )        \
{                                                                               \
    const __vector4 y0( __vrefp( B ) );                                         \
    const __vector4 absMask( c_VectorAbsMask );                                 \
    const __vector4 abs_B( __vand( B, absMask ) );                              \
    const __vector4 fltMin( c_VectorFltMin );                                   \
    const __vector4 fltMax( c_VectorFltMax );                                   \
    const __vector4 one( c_VectorOne );                                         \
    const __vector4 abs_B_GT_fltMin( __vcmpgtfp( abs_B, fltMin ) );             \
    const __vector4 abs_B_LT_fltMax( __vcmpgtfp( fltMax, abs_B ) );             \
    const __vector4 valid_B( __vand( abs_B_GT_fltMin, abs_B_LT_fltMax ) );      \
    const __vector4 t( __vnmsubfp( y0, B, one ) );                              \
    const __vector4 y1( __vmaddfp( y0, t, y0 ) );                               \
    /* TODO: Validate y0, y1? */                                                \
    out_rcp_B = __vsel( invalid_result, y1, valid_B );                          \
    out_valid = valid_B;                                                        \
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define VMX_VECTOR_RECIPROCAL_SQRT( B, invalid_result, out_rcp_sqrt_B, out_valid )  \
{                                                                                   \
    const __vector4 y0( __vrsqrtefp( B ) );                                         \
    const __vector4 fltMin( c_VectorFltMin );                                       \
    const __vector4 fltMax( c_VectorFltMax );                                       \
    const __vector4 half( c_VectorHalf );                                           \
    const __vector4 one( c_VectorOne );                                             \
    const __vector4 B_GT_fltMin( __vcmpgtfp( B, fltMin ) );                         \
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );                         \
    const __vector4 valid_B( __vand( B_GT_fltMin, B_LT_fltMax ) );                  \
    const __vector4 t0( __vmulfp( y0, y0 ) );                                       \
    const __vector4 t1( __vmulfp( y0, half ) );                                     \
    const __vector4 t2( __vnmsubfp( B, t0, one ) );                                 \
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );                                  \
    /* TODO: Validate y0, y1? */                                                    \
    out_rcp_sqrt_B = __vsel( invalid_result, y1, valid_B );                         \
    out_valid = valid_B;                                                            \
}



/// VMX Vector Reciprocal : 1 / B : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __vector4 SIMD_Rcp( const __vector4 B )
{
    // Error analysis: Rcp<11> : 0 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.001780353
    // mean error       =     0.000031698
    // rms error        =     0.000083457
    // peak error ratio =     0.000193348
    // mean error ratio =     0.000068766
    // rms error ratio  =     0.000081832
    
    // Error analysis: Rcp<22> : 1 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.000001064
    // mean error       =     0.000000014
    // rms error        =     0.000000037
    // peak error ratio =     0.000000118
    // mean error ratio =     0.000000030
    // rms error ratio  =     0.000000037
    
    // Error analysis: Rcp<24> : 2 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.000000791
    // mean error       =     0.000000014
    // rms error        =     0.000000036
    // peak error ratio =     0.000000089
    // mean error ratio =     0.000000029
    // rms error ratio  =     0.000000036
    
    const __vector4 y0( __vrefp( B ) );
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 flt_min( Const< __vector4 >::FltMin() );
    const __vector4 abs_B( __vand( B, abs_mask ) );
    const __vector4 zero( Const< __vector4 >::Zero() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 valid( __vcmpgtfp( abs_B, flt_min ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return __vsel( zero, y0, valid );
    }
    else
    {
        const __vector4 t0( __vnmsubfp( y0, B, one ) );
        const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return __vsel( zero, y1, valid );
        }
        else
        {
            const __vector4 t1( __vnmsubfp( y1, B, one ) );
            const __vector4 y2( __vmaddfp( y1, t1, y1 ) );
            return __vsel( zero, y2, valid );
        }
    }
}

/// VMX Vector Divide : A / B : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __vector4 SIMD_Div( const __vector4 A, const __vector4 B )
{
    // Error analysis: Div<11> : 0 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.001847247
    // mean error       =     0.000096883
    // rms error        =     0.000176686
    // peak error ratio =     0.000190400
    // mean error ratio =     0.000065466
    // rms error ratio  =     0.000079927
    
    // Error analysis: Div<22> : 1 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.000001597
    // mean error       =     0.000000058
    // rms error        =     0.000000110
    // peak error ratio =     0.000000181
    // mean error ratio =     0.000000038
    // rms error ratio  =     0.000000049
    
    // Error analysis: Div<24> : 2 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.000001501
    // mean error       =     0.000000057
    // rms error        =     0.000000108
    // peak error ratio =     0.000000173
    // mean error ratio =     0.000000038
    // rms error ratio  =     0.000000048
    
    const __vector4 y0( __vrefp( B ) );
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 flt_min( Const< __vector4 >::FltMin() );
    const __vector4 abs_B( __vand( B, abs_mask ) );
    const __vector4 zero( Const< __vector4 >::Zero() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 valid( __vcmpgtfp( abs_B, flt_min ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        const __vector4 q0( __vmulfp( A, y0 ) );
        return __vsel( zero, q0, valid );
    }
    else
    {
        const __vector4 t0( __vnmsubfp( y0, B, one ) );
        const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            const __vector4 q1( __vmulfp( A, y1 ) );
            return __vsel( zero, q1, valid );
        }
        else
        {
            const __vector4 t1( __vnmsubfp( y1, B, one ) );
            const __vector4 y2( __vmaddfp( y1, t1, y1 ) );
            const __vector4 q2( __vmulfp( A, y2 ) );
            return __vsel( zero, q2, valid );
        }
    }
}

/// VMX Vector Reciprocal Sqrt : 1 / sqrt( B ) : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __vector4 SIMD_RcpSqrt( const __vector4 B )
{
    // Error analysis: RcpSqrt<11> : 0 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.001498365
    // mean error       =     0.000020277
    // rms error        =     0.000046215
    // peak error ratio =     0.000195880
    // mean error ratio =     0.000033668
    // rms error ratio  =     0.000057681
    
    // Error analysis: RcpSqrt<22> : 1 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.000000744
    // mean error       =     0.000000009
    // rms error        =     0.000000020
    // peak error ratio =     0.000000132
    // mean error ratio =     0.000000014
    // rms error ratio  =     0.000000024
    
    // Error analysis: RcpSqrt<24> : 2 NR
    // samples = 1000001, domain = [ -10.000000, 10.000000 ]
    // peak error       =     0.000000774
    // mean error       =     0.000000008
    // rms error        =     0.000000018
    // peak error ratio =     0.000000101
    // mean error ratio =     0.000000012
    // rms error ratio  =     0.000000022
    
    const __vector4 y0( __vrsqrtefp( B ) );
    const __vector4 flt_min( Const< __vector4 >::FltMin() );
    const __vector4 flt_max( Const< __vector4 >::FltMax() );
    const __vector4 B_GT_flt_min( __vcmpgtfp( B, flt_min ) );
    const __vector4 B_LT_flt_max( __vcmpgtfp( flt_max, B ) );
    const __vector4 zero( Const< __vector4 >::Zero() );
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 valid( __vand( B_GT_flt_min, B_LT_flt_max ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return __vsel( zero, y0, valid );
    }
    else
    {
        const __vector4 t0( __vmulfp( y0, y0 ) );
        const __vector4 t1( __vmulfp( y0, half ) );
        const __vector4 t2( __vnmsubfp( B, t0, one ) );
        const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return __vsel( zero, y1, valid );
        }
        else
        {
            const __vector4 t0( __vmulfp( y1, y1 ) );
            const __vector4 t1( __vmulfp( y1, half ) );
            const __vector4 t2( __vnmsubfp( B, t0, one ) );
            const __vector4 y2( __vmaddfp( t2, t1, y1 ) );
            return __vsel( zero, y2, valid );
        }
    }
}

/// VMX Vector Reciprocal Sqrt : 1 / sqrt( B ) : Infinite/Indefinite/NaN results are converted to errorResult!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __vector4 SIMD_RcpSqrt( const __vector4 B, const __vector4 errorResult )
{
    const __vector4 y0( __vrsqrtefp( B ) );
    const __vector4 flt_min( Const< __vector4 >::FltMin() );
    const __vector4 flt_max( Const< __vector4 >::FltMax() );
    const __vector4 B_GT_flt_min( __vcmpgtfp( B, flt_min ) );
    const __vector4 B_LT_flt_max( __vcmpgtfp( flt_max, B ) );
    //const __vector4 zero( Const< __vector4 >::Zero() );
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 valid( __vand( B_GT_flt_min, B_LT_flt_max ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return __vsel( errorResult, y0, valid );
    }
    else
    {
        const __vector4 t0( __vmulfp( y0, y0 ) );
        const __vector4 t1( __vmulfp( y0, half ) );
        const __vector4 t2( __vnmsubfp( B, t0, one ) );
        const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return __vsel( errorResult, y1, valid );
        }
        else
        {
            const __vector4 t0( __vmulfp( y1, y1 ) );
            const __vector4 t1( __vmulfp( y1, half ) );
            const __vector4 t2( __vnmsubfp( B, t0, one ) );
            const __vector4 y2( __vmaddfp( t2, t1, y1 ) );
            return __vsel( errorResult, y2, valid );
        }
    }
}

/// VMX Vector Sqrt : sqrt( A ) : Infinite/Indefinite/NaN results are truncated to zero!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __vector4 SIMD_Sqrt( const __vector4 A )
{
    const __vector4 y0( __vrsqrtefp( A ) );
    const __vector4 flt_min( Const< __vector4 >::FltMin() );
    const __vector4 flt_max( Const< __vector4 >::FltMax() );
    const __vector4 A_GT_flt_min( __vcmpgtfp( A, flt_min ) );
    const __vector4 A_LT_flt_max( __vcmpgtfp( flt_max, A ) );
    const __vector4 zero( Const< __vector4 >::Zero() );
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 valid( __vand( A_GT_flt_min, A_LT_flt_max ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        const __vector4 sqrt_A( __vmulfp( A, y0 ) );
        return __vsel( zero, sqrt_A, valid );
    }
    else
    {
        const __vector4 t0( __vmulfp( y0, y0 ) );
        const __vector4 t1( __vmulfp( y0, half ) );
        const __vector4 t2( __vnmsubfp( A, t0, one ) );
        const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            const __vector4 sqrt_A( __vmulfp( A, y1 ) );
            return __vsel( zero, sqrt_A, valid );
        }
        else
        {
            const __vector4 t0( __vmulfp( y1, y1 ) );
            const __vector4 t1( __vmulfp( y1, half ) );
            const __vector4 t2( __vnmsubfp( A, t0, one ) );
            const __vector4 y2( __vmaddfp( t2, t1, y1 ) );
            
            const __vector4 sqrt_A( __vmulfp( A, y2 ) );
            return __vsel( zero, sqrt_A, valid );
        }
    }
}







/// VMX Vector Divide : A / B : Infinite/Indefinite/NaN results are preserved!
template < int T_PrecisionBits > /// T_PrecisionBits --> [ 11, 22, 24 ]
NOALIAS __forceinline const __vector4 SIMD_UnsafeDivide( const __vector4 A, const __vector4 B )
{
    const __vector4 y0( __vrefp( B ) );
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 flt_min( Const< __vector4 >::FltMin() );
    const __vector4 abs_B( __vand( B, abs_mask ) );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 valid( __vcmpgtfp( abs_B, flt_min ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        const __vector4 q0( __vmulfp( A, y0 ) );
        return __vsel( y0, q0, valid );
    }
    else
    {
        const __vector4 t0( __vnmsubfp( y0, B, one ) );
        const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            const __vector4 q1( __vmulfp( A, y1 ) );
            return __vsel( y0, q1, valid );
        }
        else
        {
            const __vector4 t1( __vnmsubfp( y1, B, one ) );
            const __vector4 y2( __vmaddfp( y1, t1, y1 ) );
            const __vector4 q2( __vmulfp( A, y2 ) );
            return __vsel( y0, q2, valid );
        }
    }
}



NOALIAS __forceinline void vmx_transpose_matrix33(
    const __vector4             inX,
    const __vector4             inY,
    const __vector4             inZ,
    __vector4* const __restrict pOutX,
    __vector4* const __restrict pOutY,
    __vector4* const __restrict pOutZ )
{
    const __vector4 inW( Const< __vector4 >::Zero() );
    const __vector4 tmp0( vmx_merge_xy( inX, inZ ) );
    const __vector4 tmp1( vmx_merge_xy( inY, inW ) );
    const __vector4 tmp2( vmx_merge_zw( inX, inZ ) );
    const __vector4 tmp3( vmx_merge_zw( inY, inW ) );
    *pOutX = vmx_merge_xy( tmp0, tmp1 );
    *pOutY = vmx_merge_zw( tmp0, tmp1 );
    *pOutZ = vmx_merge_xy( tmp2, tmp3 );
}

NOALIAS __forceinline void vmx_transpose_matrix34(
    const __vector4             inX,
    const __vector4             inY,
    const __vector4             inZ,
    const __vector4             inW,
    __vector4* const __restrict pOutX,
    __vector4* const __restrict pOutY,
    __vector4* const __restrict pOutZ )
{
    const __vector4 tmp0( vmx_merge_xy( inX, inZ ) );
    const __vector4 tmp1( vmx_merge_xy( inY, inW ) );
    const __vector4 tmp2( vmx_merge_zw( inX, inZ ) );
    const __vector4 tmp3( vmx_merge_zw( inY, inW ) );
    *pOutX = vmx_merge_xy( tmp0, tmp1 );
    *pOutY = vmx_merge_zw( tmp0, tmp1 );
    *pOutZ = vmx_merge_xy( tmp2, tmp3 );
}

NOALIAS __forceinline void vmx_transpose_matrix43(
    const __vector4             inX,
    const __vector4             inY,
    const __vector4             inZ,
    __vector4* const __restrict pOutX,
    __vector4* const __restrict pOutY,
    __vector4* const __restrict pOutZ,
    __vector4* const __restrict pOutW )
{
    const __vector4 inW( Const< __vector4 >::UnitW() );
    const __vector4 tmp0( vmx_merge_xy( inX, inZ ) );
    const __vector4 tmp1( vmx_merge_xy( inY, inW ) );
    const __vector4 tmp2( vmx_merge_zw( inX, inZ ) );
    const __vector4 tmp3( vmx_merge_zw( inY, inW ) );
    *pOutX = vmx_merge_xy( tmp0, tmp1 );
    *pOutY = vmx_merge_zw( tmp0, tmp1 );
    *pOutZ = vmx_merge_xy( tmp2, tmp3 );
    *pOutW = vmx_merge_zw( tmp2, tmp3 );
}

NOALIAS __forceinline void vmx_transpose_matrix44(
    const __vector4             inX,
    const __vector4             inY,
    const __vector4             inZ,
    const __vector4             inW,
    __vector4* const __restrict pOutX,
    __vector4* const __restrict pOutY,
    __vector4* const __restrict pOutZ,
    __vector4* const __restrict pOutW )
{
    //const __vector4 tmp0( vmx_merge_xy( inX, inZ ) );
    //const __vector4 tmp1( vmx_merge_xy( inY, inW ) );
    //const __vector4 tmp2( vmx_merge_zw( inX, inZ ) );
    //const __vector4 tmp3( vmx_merge_zw( inY, inW ) );
    //*pOutX = vmx_merge_xy( tmp0, tmp1 );
    //*pOutY = vmx_merge_zw( tmp0, tmp1 );
    //*pOutZ = vmx_merge_xy( tmp2, tmp3 );
    //*pOutW = vmx_merge_zw( tmp2, tmp3 );
    const __vector4 x0_y0_z0_w0( inX );                                      // [ x0 y0 z0 w0 ]
    const __vector4 x1_y1_z1_w1( inY );                                      // [ x1 y1 z1 w1 ]
    const __vector4 x2_y2_z2_w2( inZ );                                      // [ x2 y2 z2 w2 ]
    const __vector4 x3_y3_z3_w3( inW );                                      // [ x3 y3 z3 w3 ]
    const __vector4 x0_x2_y0_y2( vmx_merge_xy( x0_y0_z0_w0, x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const __vector4 x1_x3_y1_y3( vmx_merge_xy( x1_y1_z1_w1, x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const __vector4 z0_z2_w0_w2( vmx_merge_zw( x0_y0_z0_w0, x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const __vector4 z1_z3_w1_w3( vmx_merge_zw( x1_y1_z1_w1, x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    *pOutX = vmx_merge_xy( x0_x2_y0_y2, x1_x3_y1_y3 );                       // [ x0 x1 x2 x3 ]
    *pOutY = vmx_merge_zw( x0_x2_y0_y2, x1_x3_y1_y3 );                       // [ y0 y1 y2 y3 ]
    *pOutZ = vmx_merge_xy( z0_z2_w0_w2, z1_z3_w1_w3 );                       // [ z0 z1 z2 z3 ]
    *pOutW = vmx_merge_zw( z0_z2_w0_w2, z1_z3_w1_w3 );                       // [ w0 w1 w2 w3 ]
}


NOALIAS __forceinline const __vector4 vmx_transpose_row_x( const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 colW )
{
    return vmx_merge_xy( vmx_merge_xy( colX, colZ ),
                         vmx_merge_xy( colY, colW ) );
}

NOALIAS __forceinline const __vector4 vmx_transpose_row_y( const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 colW )
{
    return vmx_merge_zw( vmx_merge_xy( colX, colZ ),
                         vmx_merge_xy( colY, colW ) );
}

NOALIAS __forceinline const __vector4 vmx_transpose_row_z( const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 colW )
{
    return vmx_merge_xy( vmx_merge_zw( colX, colZ ),
                         vmx_merge_zw( colY, colW ) );
}

NOALIAS __forceinline const __vector4 vmx_transpose_row_w( const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 colW )
{
    return vmx_merge_zw( vmx_merge_zw( colX, colZ ),
                         vmx_merge_zw( colY, colW ) );
}



NOALIAS __forceinline const __vector4 vmx_abs( const __vector4 a )
{
    return __vand( a, Const< __vector4 >::AbsMask() );
}

NOALIAS __forceinline const __vector4 vmx_negate( const __vector4 a )
{
    return __vxor( a, Const< __vector4 >::SignMask() );
}


NOALIAS __forceinline const __vector4 vmx_dot_product2( const __vector4 a, const __vector4 b )
{
    return __vmsum3fp( __vrlimi( a, __vzero(), VMX_MASK_INSERT_Z, 0 ),
                       __vrlimi( b, __vzero(), VMX_MASK_INSERT_Z, 0 ) );
}

#define vmx_dot_product3 __vmsum3fp
#define vmx_dot_product4 __vmsum4fp



NOALIAS __forceinline const __vector4 vmx_cross_product2( const __vector4 a, const __vector4 b )
{
    return __vnmsubfp( __vspltw( a, VY ), __vspltw( b, VX ),
             __vmulfp( __vspltw( a, VX ), __vspltw( b, VY ) ) );
}

NOALIAS __forceinline const __vector4 vmx_cross_product3( const __vector4 a, const __vector4 b )
{
    // a.yzx * b.zxy - a.zxy * b.yzx
    return __vnmsubfp( __vpermwi( a, VPERMWI_CONST(VZ,VX,VY,VW) ), __vpermwi( b, VPERMWI_CONST(VY,VZ,VX,VW) ),
             __vmulfp( __vpermwi( a, VPERMWI_CONST(VY,VZ,VX,VW) ), __vpermwi( b, VPERMWI_CONST(VZ,VX,VY,VW) ) ) );
}



/// vector3 <-- vector3 [Implicit W=0] * matrix33
NOALIAS __forceinline const __vector4 vmx_multiply_vector3_matrix33( const __vector4 v, const __vector4 colX, const __vector4 colY, const __vector4 colZ )
{
    return vmx_make_vector3( __vmsum3fp( v, colX ),
                             __vmsum3fp( v, colY ),
                             __vmsum3fp( v, colZ ) );
}

/// vector3 <-- vector3 [Implicit W=0] * matrix43 [ Implicit column W = (0,0,0,1) ]
NOALIAS __forceinline const __vector4 vmx_multiply_vector3_matrix43( const __vector4 v, const __vector4 colX, const __vector4 colY, const __vector4 colZ )
{
    return vmx_make_vector3( __vmsum3fp( v, colX ),
                             __vmsum3fp( v, colY ),
                             __vmsum3fp( v, colZ ) );
}

/// vector4 <-- vector3 [Implicit W=0] * matrix44
NOALIAS __forceinline const __vector4 vmx_multiply_vector3_matrix44( const __vector4 v, const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 colW )
{
    return vmx_make_vector4( __vmsum3fp( v, colX ),
                             __vmsum3fp( v, colY ),
                             __vmsum3fp( v, colZ ),
                             __vmsum3fp( v, colW ) );
}



/// point3 <-- point3 [Implicit W=1] * matrix33
NOALIAS __forceinline const __vector4 vmx_multiply_point3_matrix33( const __vector4 p, const __vector4 colX, const __vector4 colY, const __vector4 colZ )
{
    return vmx_make_vector3( __vmsum3fp( p, colX ),
                             __vmsum3fp( p, colY ),
                             __vmsum3fp( p, colZ ) );
}

/// point3 <-- point3 [Implicit W=1] * matrix43 [ Implicit column W = (0,0,0,1) ]
NOALIAS __forceinline const __vector4 vmx_multiply_point3_matrix43( const __vector4 p, const __vector4 colX, const __vector4 colY, const __vector4 colZ )
{
    const __vector4 xyz1( __vrlimi( p, Const< __vector4 >::One(), VMX_MASK_INSERT_W, 0 ) );
    return vmx_make_vector3( __vmsum4fp( xyz1, colX ),
                             __vmsum4fp( xyz1, colY ),
                             __vmsum4fp( xyz1, colZ ) );
}

/// vector4 <-- point3 [Implicit W=1] * matrix44
NOALIAS __forceinline const __vector4 vmx_multiply_point3_matrix44( const __vector4 p, const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 colW )
{
    const __vector4 xyz1( __vrlimi( p, Const< __vector4 >::One(), VMX_MASK_INSERT_W, 0 ) );
    return vmx_make_vector4( __vmsum4fp( xyz1, colX ),
                             __vmsum4fp( xyz1, colY ),
                             __vmsum4fp( xyz1, colZ ),
                             __vmsum4fp( xyz1, colW ) );
}



/// vector4 <-- vector4 * matrix33 [ Implicit row W = column W = (0,0,0,1) ]
NOALIAS __forceinline const __vector4 vmx_multiply_vector4_matrix33( const __vector4 v, const __vector4 colX, const __vector4 colY, const __vector4 colZ )
{
    return vmx_make_vector4( __vmsum3fp( v, colX ),
                             __vmsum3fp( v, colY ),
                             __vmsum3fp( v, colZ ),
                               __vspltw( v, VW ) );
}

/// vector4 <-- vector4 * matrix43 [ Implicit column W = (0,0,0,1) ]
NOALIAS __forceinline const __vector4 vmx_multiply_vector4_matrix43( const __vector4 v, const __vector4 colX, const __vector4 colY, const __vector4 colZ )
{
    return vmx_make_vector4( __vmsum4fp( v, colX ),
                             __vmsum4fp( v, colY ),
                             __vmsum4fp( v, colZ ),
                               __vspltw( v, VW ) );
}

/// vector4 <-- vector4 * matrix44
NOALIAS __forceinline const __vector4 vmx_multiply_vector4_matrix44( const __vector4 v, const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 colW )
{
    return vmx_make_vector4( __vmsum4fp( v, colX ),
                             __vmsum4fp( v, colY ),
                             __vmsum4fp( v, colZ ),
                             __vmsum4fp( v, colW ) );
}



/// vector3 <-- matrix33 * vector3 [Implicit W=0]
NOALIAS __forceinline const __vector4 vmx_multiply_matrix33_vector3( const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 v )
{
    __vector4 rowX, rowY, rowZ;
    vmx_transpose_matrix33( colX, colY, colZ, &rowX, &rowY, &rowZ );
    return vmx_make_vector3( __vmsum3fp( rowX, v ),
                             __vmsum3fp( rowY, v ),
                             __vmsum3fp( rowZ, v ) );
}

/// vector4 <-- matrix43 [ Implicit column W = (0,0,0,1) ] * vector3 [Implicit W=0]
NOALIAS __forceinline const __vector4 vmx_multiply_matrix43_vector3( const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 v )
{
    __vector4 rowX, rowY, rowZ, rowW;
    vmx_transpose_matrix43( colX, colY, colZ, &rowX, &rowY, &rowZ, &rowW );
    return vmx_make_vector4( __vmsum3fp( rowX, v ),
                             __vmsum3fp( rowY, v ),
                             __vmsum3fp( rowZ, v ),
                             __vmsum3fp( rowW, v ) );
}

/// vector4 <-- matrix44 * vector3 [Implicit W=0]
NOALIAS __forceinline const __vector4 vmx_multiply_matrix44_vector3( const __vector4 colX, const __vector4 colY, const __vector4 colZ, const __vector4 colW, const __vector4 v )
{
    __vector4 rowX, rowY, rowZ, rowW;
    vmx_transpose_matrix44( colX, colY, colZ, colW, &rowX, &rowY, &rowZ, &rowW );
    return vmx_make_vector4( __vmsum3fp( rowX, v ),
                             __vmsum3fp( rowY, v ),
                             __vmsum3fp( rowZ, v ),
                             __vmsum3fp( rowW, v ) );
}






//
// VMX & VMX128 SIMD Scalar & Vector Maths Functions
//


//#define VMX_VPERM_WORD(i)   ( ( ((((i)&7)*4)+0) << 24 ) | ( ((((i)&7)*4)+1) << 16 ) | ( ((((i)&7)*4)+2) << 8 ) | ((((i)&7)*4)+3) )
//
//#define VMX_VPERM_AX        VMX_VPERM_WORD(0)
//#define VMX_VPERM_AY        VMX_VPERM_WORD(1)
//#define VMX_VPERM_AZ        VMX_VPERM_WORD(2)
//#define VMX_VPERM_AW        VMX_VPERM_WORD(3)
//#define VMX_VPERM_BX        VMX_VPERM_WORD(4)
//#define VMX_VPERM_BY        VMX_VPERM_WORD(5)
//#define VMX_VPERM_BZ        VMX_VPERM_WORD(6)
//#define VMX_VPERM_BW        VMX_VPERM_WORD(7)


//static const V128 c_Mask_0000 = __vspltisw( 0 );
//static const V128 c_Mask_1111 = __vspltisw( -1 );

//#define c_Mask_0000 __vspltisw( 0 )
//#define c_Mask_1111 __vspltisw( -1 )
//
//static const V128 c_Mask_1010 = __vmrghw( c_Mask_1111, c_Mask_0000 );
//static const V128 c_Mask_0101 = __vmrghw( c_Mask_0000, c_Mask_1111 );
//
//static const V128 c_Mask_1000 = __vmrghw( c_Mask_1010, c_Mask_0000 );
//static const V128 c_Mask_0100 = __vmrghw( c_Mask_0000, c_Mask_1010 );
//static const V128 c_Mask_0010 = __vmrghw( c_Mask_0101, c_Mask_0000 );
//static const V128 c_Mask_0001 = __vmrghw( c_Mask_0000, c_Mask_0101 );
//
//static const V128 c_Mask_1100 = __vmrghw( c_Mask_1010, c_Mask_1010 );
//static const V128 c_Mask_1110 = __vmrghw( c_Mask_1111, c_Mask_1010 );
//
//static const V128 c_Mask_0011 = __vmrghw( c_Mask_0101, c_Mask_0101 );
//static const V128 c_Mask_0111 = __vmrghw( c_Mask_0101, c_Mask_1111 );
//
//static const V128 c_Mask_0110 = __vmrghw( c_Mask_0101, c_Mask_1010 );
//static const V128 c_Mask_1001 = __vmrghw( c_Mask_1010, c_Mask_0101 );
//static const V128 c_Mask_1011 = __vmrghw( c_Mask_1111, c_Mask_0101 );
//static const V128 c_Mask_1101 = __vmrghw( c_Mask_1010, c_Mask_1111 );



/// Vector Mask Generators
//FORCE_INLINE const V128 SIMD_Mask_0000() { return __vspltisw( 0 ); }
//FORCE_INLINE const V128 SIMD_Mask_1111() { return __vspltisw( -1 ); }
//FORCE_INLINE const V128 SIMD_Mask_1010() { return __vmrghw( SIMD_Mask_1111(), SIMD_Mask_0000() ); }
//FORCE_INLINE const V128 SIMD_Mask_0101() { return __vmrghw( SIMD_Mask_0000(), SIMD_Mask_1111() ); }
//FORCE_INLINE const V128 SIMD_Mask_1000() { return __vmrghw( SIMD_Mask_1010(), SIMD_Mask_0000() ); }
//FORCE_INLINE const V128 SIMD_Mask_0100() { return __vmrghw( SIMD_Mask_0000(), SIMD_Mask_1010() ); }
//FORCE_INLINE const V128 SIMD_Mask_0010() { return __vmrghw( SIMD_Mask_0101(), SIMD_Mask_0000() ); }
//FORCE_INLINE const V128 SIMD_Mask_0001() { return __vmrghw( SIMD_Mask_0000(), SIMD_Mask_0101() ); }
//FORCE_INLINE const V128 SIMD_Mask_1100() { return __vmrghw( SIMD_Mask_1010(), SIMD_Mask_1010() ); }
//FORCE_INLINE const V128 SIMD_Mask_1110() { return __vmrghw( SIMD_Mask_1111(), SIMD_Mask_1010() ); }
//FORCE_INLINE const V128 SIMD_Mask_0011() { return __vmrghw( SIMD_Mask_0101(), SIMD_Mask_0101() ); }
//FORCE_INLINE const V128 SIMD_Mask_0111() { return __vmrghw( SIMD_Mask_0101(), SIMD_Mask_1111() ); }
//FORCE_INLINE const V128 SIMD_Mask_0110() { return __vmrghw( SIMD_Mask_0101(), SIMD_Mask_1010() ); }
//FORCE_INLINE const V128 SIMD_Mask_1001() { return __vmrghw( SIMD_Mask_1010(), SIMD_Mask_0101() ); }
//FORCE_INLINE const V128 SIMD_Mask_1011() { return __vmrghw( SIMD_Mask_1111(), SIMD_Mask_0101() ); }
//FORCE_INLINE const V128 SIMD_Mask_1101() { return __vmrghw( SIMD_Mask_1010(), SIMD_Mask_1111() ); }


/// Vector Mask Generator Template : Default
template < bool x, bool y, bool z, bool w >
NOALIAS FORCE_INLINE
const V128 SIMD_Mask()
{
    //const V128 XXXX = ( x ? __vspltisw( -1 ) : __vspltisw( 0 ) );
    //const V128 YYYY = ( y ? __vspltisw( -1 ) : __vspltisw( 0 ) );
    //const V128 ZZZZ = ( z ? __vspltisw( -1 ) : __vspltisw( 0 ) );
    //const V128 WWWW = ( w ? __vspltisw( -1 ) : __vspltisw( 0 ) );
    //return __vmrghw( __vmrghw( XXXX, ZZZZ ), __vmrghw( YYYY, WWWW ) );
    const __vector4 zero( __vzero() );
    const __vector4 one( __vspltisw( -1 ) );
    return __vrlimi( zero, one, (x?VMX_MASK_INSERT_X:0) | (y?VMX_MASK_INSERT_Y:0) | (z?VMX_MASK_INSERT_Z:0) | (w?VMX_MASK_INSERT_W:0), 0 );
}

/// Vector Mask Generator Template : Specialisation for 0000
template <>
NOALIAS FORCE_INLINE
const V128 SIMD_Mask< 0, 0, 0, 0 >()
{
    return __vzero();
}

/// Vector Mask Generator Template : Specialisation for 1111
template <>
NOALIAS FORCE_INLINE
const V128 SIMD_Mask< 1, 1, 1, 1 >()
{
    return __vspltisw( -1 );
}

/// Vector Mask Generator Template : Specialisation for 1010
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 1, 0, 1, 0 >()
//{
//    //return __vmrghw( SIMD_Mask<1,1,1,1>(), SIMD_Mask<0,0,0,0>() );
//    //return SIMD_Mask_1010();
//    return c_Mask_1010;
//}

/// Vector Mask Generator Template : Specialisation for 0101
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 0, 1, 0, 1 >()
//{
//    //return __vmrghw( SIMD_Mask<0,0,0,0>(), SIMD_Mask<1,1,1,1>() );
//    //return SIMD_Mask_0101();
//    return c_Mask_0101;
//}

/// Vector Mask Generator Template : Specialisation for 1000
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 1, 0, 0, 0 >()
//{
//    //return __vmrghw( SIMD_Mask<1,0,1,0>(), SIMD_Mask<0,0,0,0>() );
//    //return SIMD_Mask_1000();
//    return c_Mask_1000;
//}

/// Vector Mask Generator Template : Specialisation for 0100
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 0, 1, 0, 0 >()
//{
//    //return __vmrghw( SIMD_Mask<0,0,0,0>(), SIMD_Mask<1,0,1,0>() );
//    //return SIMD_Mask_0100();
//    return c_Mask_0100;
//}

/// Vector Mask Generator Template : Specialisation for 0010
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 0, 0, 1, 0 >()
//{
//    //return __vmrghw( SIMD_Mask<0,1,0,1>(), SIMD_Mask<0,0,0,0>() );
//    //return SIMD_Mask_0010();
//    return c_Mask_0010;
//}

/// Vector Mask Generator Template : Specialisation for 0001
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 0, 0, 0, 1 >()
//{
//    //return __vmrghw( SIMD_Mask<0,0,0,0>(), SIMD_Mask<0,1,0,1>() );
//    //return SIMD_Mask_0001();
//    return c_Mask_0001;
//}

/// Vector Mask Generator Template : Specialisation for 1100
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 1, 1, 0, 0 >()
//{
//    //return __vmrghw( SIMD_Mask<1,0,1,0>(), SIMD_Mask<1,0,1,0>() );
//    //return SIMD_Mask_1100();
//    return c_Mask_1100;
//}

/// Vector Mask Generator Template : Specialisation for 1110
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 1, 1, 1, 0 >()
//{
//    //return __vmrghw( SIMD_Mask<1,1,1,1>(), SIMD_Mask<1,0,1,0>() );
//    //return SIMD_Mask_1110();
//    return c_Mask_1110;
//}

/// Vector Mask Generator Template : Specialisation for 0011
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 0, 0, 1, 1 >()
//{
//    //return __vmrghw( SIMD_Mask<0,1,0,1>(), SIMD_Mask<0,1,0,1>() );
//    //return SIMD_Mask_0011();
//    return c_Mask_0011;
//}

/// Vector Mask Generator Template : Specialisation for 0111
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 0, 1, 1, 1 >()
//{
//    //return __vmrghw( SIMD_Mask<0,1,0,1>(), SIMD_Mask<1,1,1,1>() );
//    //return SIMD_Mask_0111();
//    return c_Mask_0111;
//}

/// Vector Mask Generator Template : Specialisation for 0110
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 0, 1, 1, 0 >()
//{
//    //return __vmrghw( SIMD_Mask<0,1,0,1>(), SIMD_Mask<1,0,1,0>() );
//    //return SIMD_Mask_0110();
//    return c_Mask_0110;
//}

/// Vector Mask Generator Template : Specialisation for 1001
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 1, 0, 0, 1 >()
//{
//    //return __vmrghw( SIMD_Mask<1,0,1,0>(), SIMD_Mask<0,1,0,1>() );
//    //return SIMD_Mask_1001();
//    return c_Mask_1001;
//}

/// Vector Mask Generator Template : Specialisation for 1011
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 1, 0, 1, 1 >()
//{
//    //return __vmrghw( SIMD_Mask<1,1,1,1>(), SIMD_Mask<0,1,0,1>() );
//    //return SIMD_Mask_1011();
//    return c_Mask_1011;
//}

/// Vector Mask Generator Template : Specialisation for 1101
//template <>
//FORCE_INLINE
//const V128 SIMD_Mask< 1, 1, 0, 1 >()
//{
//    //return __vmrghw( SIMD_Mask<1,0,1,0>(), SIMD_Mask<1,1,1,1>() );
//    //return SIMD_Mask_1101();
//    return c_Mask_1101;
//}



/// Vector Masking Function Template
template < bool x, bool y, bool z, bool w >
NOALIAS FORCE_INLINE
const V128 SIMD_Mask( const V128 PCREF src )
{
    return __vrlimi( src, __vzero(), (x?0:VMX_MASK_INSERT_X) | (y?0:VMX_MASK_INSERT_Y) | (z?0:VMX_MASK_INSERT_Z) | (w?0:VMX_MASK_INSERT_W), 0 );
}






/// SIMD Scalar/Vector Zero
NOALIAS FORCE_INLINE
const V128 SIMD_Zero()
{
    return __vzero();
}

/// SIMD Scalar F32 Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar( const F32& f )
{
    //const V128 result = { f, f, f, f };
    //return result;
    return __vspltw( __lvlx( &f, 0 ), 0 );
}

/// SIMD Scalar F32 Constructor
NOALIAS __forceinline const V128 SIMD_Scalar( F32 const * const __restrict ptr )
{
    return __vspltw( __lvlx( ptr, 0 ), 0 );
}

/// SIMD Scalar S32 Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar( const S32& i )
{
    //const __vector4i integer4 = { i, i, i, i };
    //return __vcfsx( (V128&)integer4, 0 );
    return __vcfsx( __vspltw( __lvlx( &i, 0 ), 0 ), 0 );
}

/// SIMD Scalar U32 Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar( const U32& i )
{
    //const __vector4i integer4 = { i, i, i, i };
    //return __vcfux( (V128&)integer4, 0 );
    return __vcfux( __vspltw( __lvlx( &i, 0 ), 0 ), 0 );
}

/// SIMD Scalar Vector2 X Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector2X( const V128 PCREF x )
{
    return __vspltw( x, VX );
}

/// SIMD Scalar Vector2 Y Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector2Y( const V128 PCREF y )
{
    return __vspltw( y, VY );
}

/// SIMD Scalar Vector3 X Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector3X( const V128 PCREF x )
{
    return __vspltw( x, VX );
}

/// SIMD Scalar Vector3 Y Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector3Y( const V128 PCREF y )
{
    return __vspltw( y, VY );
}

/// SIMD Scalar Vector3 Z Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector3Z( const V128 PCREF z )
{
    return __vspltw( z, VZ );
}

/// SIMD Scalar Vector4 X Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector4X( const V128 PCREF x )
{
    return __vspltw( x, VX );
}

/// SIMD Scalar Vector4 Y Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector4Y( const V128 PCREF y )
{
    return __vspltw( y, VY );
}

/// SIMD Scalar Vector4 Z Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector4Z( const V128 PCREF z )
{
    return __vspltw( z, VZ );
}

/// SIMD Scalar Vector4 W Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_Vector4W( const V128 PCREF w )
{
    return __vspltw( w, VW );
}

/// SIMD Scalar VectorX Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_VectorX( const V128 PCREF x )
{
    return __vspltw( x, VX );
}

/// SIMD Scalar VectorY Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_VectorY( const V128 PCREF y )
{
    return __vspltw( y, VY );
}

/// SIMD Scalar VectorZ Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_VectorZ( const V128 PCREF z )
{
    return __vspltw( z, VZ );
}

/// SIMD Scalar VectorW Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Scalar_VectorW( const V128 PCREF w )
{
    return __vspltw( w, VW );
}

/// SIMD Scalar Broadcast to XYZW
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarBroadcastToXYZW( const V128 PCREF src )
{
    return src;
}

/// SIMD Scalar Cast to F32
NOALIAS FORCE_INLINE
const F32 SIMD_ScalarToF32( const V128 PCREF src )
{
    //return src.x;
    F32 F32_from_HF32;
    __stvewx( src, &F32_from_HF32, 0 );
    return F32_from_HF32;
}

/// SIMD Scalar Cast to S32
NOALIAS FORCE_INLINE
const S32 SIMD_ScalarToS32( const V128 PCREF src )
{
    //return ((__vector4i&)__vctsxs( src, 0 ))[0];
    S32 result;
    __stvewx( __vctsxs( src, 0 ), &result, 0 );
    return result;
}

/// SIMD Scalar Abs
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarAbs( const V128 PCREF src )
{
    return __vand( src, c_VectorAbsMask );
}

/// SIMD Scalar Sqrt
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarSqrt( const V128 PCREF src )
{
    return SIMD_Sqrt4( src );
}

/// SIMD Scalar Reciprocal
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarReciprocal( const V128 PCREF src )
{
    return SIMD_Reciprocal4( src );
}

/// SIMD Scalar Reciprocal Sqrt
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarReciprocalSqrt( const V128 PCREF src )
{
    return SIMD_ReciprocalSqrt4( src );
}

/// SIMD Scalar Negate
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarNegate( const V128 PCREF src )
{
    return __vxor( src, c_Vector4_SignMask );
}

/// SIMD Scalar Add
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarAdd( const V128 PCREF a, const V128 PCREF b )
{
    return __vaddfp( a, b );
}

/// SIMD Scalar Subtract
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarSubtract( const V128 PCREF a, const V128 PCREF b )
{
    return __vsubfp( a, b );
}

/// SIMD Scalar Multiply
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarMultiply( const V128 PCREF a, const V128 PCREF b )
{
    return __vmulfp( a, b );
}

/// SIMD Scalar Divide
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarDivide( const V128 PCREF a, const V128 PCREF b )
{
    return __vmulfp( a, SIMD_ScalarReciprocal( b ) );
}

/// SIMD Scalar Min
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarMin( const V128 PCREF a, const V128 PCREF b )
{
    return __vminfp( a, b );
}

/// SIMD Scalar Max
NOALIAS FORCE_INLINE
const V128 SIMD_ScalarMax( const V128 PCREF a, const V128 PCREF b )
{
    return __vmaxfp( a, b );
}

/// SIMD Scalar Compare EQ
NOALIAS FORCE_INLINE
const S32 SIMD_ScalarCompareEQ( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpeqfpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Scalar Compare NE
NOALIAS FORCE_INLINE
const S32 SIMD_ScalarCompareNE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpeqfpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Scalar Compare LE
NOALIAS FORCE_INLINE
const S32 SIMD_ScalarCompareLE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgtfpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Scalar Compare LT
NOALIAS FORCE_INLINE
const S32 SIMD_ScalarCompareLT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgefpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Scalar Compare GE
NOALIAS FORCE_INLINE
const S32 SIMD_ScalarCompareGE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgefpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Scalar Compare GT
NOALIAS FORCE_INLINE
const S32 SIMD_ScalarCompareGT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgtfpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Scalar Comparison Selector EQ
NOALIAS FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorEQ( const V128 PCREF a, const V128 PCREF b )
{
    return C_SelectorScalar( __vcmpeqfp( a, b ) );
}

/// SIMD Scalar Comparison Selector NE
NOALIAS FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorNE( const V128 PCREF a, const V128 PCREF b )
{
    const V128 eq( __vcmpeqfp( a, b ) );
    return C_SelectorScalar( __vnor( eq, eq ) );
}

/// SIMD Scalar Comparison Selector LE
NOALIAS FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorLE( const V128 PCREF a, const V128 PCREF b )
{
    return C_SelectorScalar( __vcmpgefp( b, a ) );
}

/// SIMD Scalar Comparison Selector LT
NOALIAS FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorLT( const V128 PCREF a, const V128 PCREF b )
{
    return C_SelectorScalar( __vcmpgtfp( b, a ) );
}

/// SIMD Scalar Comparison Selector GE
NOALIAS FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorGE( const V128 PCREF a, const V128 PCREF b )
{
    return C_SelectorScalar( __vcmpgefp( a, b ) );
}

/// SIMD Scalar Comparison Selector GT
NOALIAS FORCE_INLINE
const C_SelectorScalar SIMD_ScalarSelectorGT( const V128 PCREF a, const V128 PCREF b )
{
    return C_SelectorScalar( __vcmpgtfp( a, b ) );
}

/// SIMD Scalar Selector Test : if ( selector == 0xFFFFFFFF ) returns 1 else returns 0
NOALIAS __forceinline const U32 SIMD_TestScalarSelector( const V128 PCREF src )
{
    U32 CR;
    __vcmpequwR( src, __vzero(), &CR );
    //return ( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
    return ( CR & VMX_CR6_ALL_FALSE ) >> VMX_CR6_ALL_FALSE_BIT;
}







/// SIMD Scalar FP Round To Integer : Towards minus infinity
//NOALIAS __forceinline const V128 SIMD_Scalar_RoundToIntMinusInf( const V128 PCREF src )
//{
//    return __vrfim( src );
//}

/// SIMD Scalar FP Round To Integer : Towards plus infinity
//NOALIAS __forceinline const V128 SIMD_Scalar_RoundToIntPlusInf( const V128 PCREF src )
//{
//    return __vrfip( src );
//}

/// SIMD Scalar FP Round To Integer : Towards nearest
//NOALIAS __forceinline const V128 SIMD_Scalar_RoundToIntNearest( const V128 PCREF src )
//{
//    return __vrfin( src );
//}

/// SIMD Scalar FP Round To Integer : Towards zero
//NOALIAS __forceinline const V128 SIMD_Scalar_RoundToIntZero( const V128 PCREF src )
//{
//    return __vrfiz( src );
//}













/// SIMD VectorX Constructor from float
NOALIAS FORCE_INLINE
const V128 SIMD_VectorX_Float( const F32& f )
{
    //const V128 result = { f, 0.0f, 0.0f, 0.0f };
    //return result;
    //return __vrlimi( __vzero(), __lvlx( &f, 0 ), VMX_MASK_INSERT_X, 0 );
    return __vrlimi( __lvlx( &f, 0 ), __vzero(), VMX_MASK_INSERT_YZW, 0 );
}

/// SIMD VectorX Constructor from Scalar
NOALIAS FORCE_INLINE
const V128 SIMD_VectorX_Scalar( const V128 PCREF f )
{
    return __vrlimi( __vzero(), f, VMX_MASK_INSERT_X, 0 );
}

/// SIMD VectorX Constructor from VectorY
NOALIAS FORCE_INLINE
const V128 SIMD_VectorX_VectorY( const V128 PCREF y )
{
    return __vrlimi( __vzero(), y, VMX_MASK_INSERT_X, VMX_ROTATE_Y_TO_X );
}

/// SIMD VectorX Constructor from VectorZ
NOALIAS FORCE_INLINE
const V128 SIMD_VectorX_VectorZ( const V128 PCREF z )
{
    return __vrlimi( __vzero(), z, VMX_MASK_INSERT_X, VMX_ROTATE_Z_TO_X );
}

/// SIMD VectorX Constructor from VectorW
NOALIAS FORCE_INLINE
const V128 SIMD_VectorX_VectorW( const V128 PCREF w )
{
    return __vrlimi( __vzero(), w, VMX_MASK_INSERT_X, VMX_ROTATE_W_TO_X );
}

/// SIMD VectorX Constructor from Vector2
NOALIAS FORCE_INLINE
const V128 SIMD_VectorX_Vector2( const V128 PCREF src )
{
    return __vrlimi( __vzero(), src, VMX_MASK_INSERT_X, 0 );
}

/// SIMD VectorX Constructor from Vector3
NOALIAS FORCE_INLINE
const V128 SIMD_VectorX_Vector3( const V128 PCREF src )
{
    return __vrlimi( __vzero(), src, VMX_MASK_INSERT_X, 0 );
}

/// SIMD VectorX Constructor from Vector4
NOALIAS FORCE_INLINE
const V128 SIMD_VectorX_Vector4( const V128 PCREF src )
{
    return __vrlimi( __vzero(), src, VMX_MASK_INSERT_X, 0 );
}


/// SIMD VectorY Constructor from float
NOALIAS FORCE_INLINE
const V128 SIMD_VectorY_Float( const F32& f )
{
    //const V128 result = { 0.0f, f, 0.0f, 0.0f };
    //return result;
    return __vrlimi( __vzero(), __lvlx( &f, 0 ), VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y );
}

/// SIMD VectorY Constructor from Scalar
NOALIAS FORCE_INLINE
const V128 SIMD_VectorY_Scalar( const V128 PCREF f )
{
    return __vrlimi( __vzero(), f, VMX_MASK_INSERT_Y, 0 );
}

/// SIMD VectorY Constructor from VectorX
NOALIAS FORCE_INLINE
const V128 SIMD_VectorY_VectorX( const V128 PCREF x )
{
    return __vrlimi( __vzero(), x, VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y );
}

/// SIMD VectorY Constructor from VectorZ
NOALIAS FORCE_INLINE
const V128 SIMD_VectorY_VectorZ( const V128 PCREF z )
{
    return __vrlimi( __vzero(), z, VMX_MASK_INSERT_Y, VMX_ROTATE_Z_TO_Y );
}

/// SIMD VectorY Constructor from VectorW
NOALIAS FORCE_INLINE
const V128 SIMD_VectorY_VectorW( const V128 PCREF w )
{
    return __vrlimi( __vzero(), w, VMX_MASK_INSERT_Y, VMX_ROTATE_W_TO_Y );
}

/// SIMD VectorY Constructor from Vector2
NOALIAS FORCE_INLINE
const V128 SIMD_VectorY_Vector2( const V128 PCREF src )
{
    return __vrlimi( __vzero(), src, VMX_MASK_INSERT_Y, 0 );
}

/// SIMD VectorY Constructor from Vector3
NOALIAS FORCE_INLINE
const V128 SIMD_VectorY_Vector3( const V128 PCREF src )
{
    return __vrlimi( __vzero(), src, VMX_MASK_INSERT_Y, 0 );
}

/// SIMD VectorY Constructor from Vector4
NOALIAS FORCE_INLINE
const V128 SIMD_VectorY_Vector4( const V128 PCREF src )
{
    return __vrlimi( __vzero(), src, VMX_MASK_INSERT_Y, 0 );
}



/// SIMD VectorZ Constructor from float
NOALIAS FORCE_INLINE
const V128 SIMD_VectorZ_Float( const F32& f )
{
    //const V128 result = { 0.0f, 0.0f, f, 0.0f };
    //return result;
    return __vrlimi( __vzero(), __lvlx( &f, 0 ), VMX_MASK_INSERT_Z, VMX_ROTATE_X_TO_Z );
}

/// SIMD VectorZ Constructor from Scalar
NOALIAS FORCE_INLINE
const V128 SIMD_VectorZ_Scalar( const V128 PCREF f )
{
    return __vrlimi( __vzero(), f, VMX_MASK_INSERT_Z, 0 );
}

/// SIMD VectorZ Constructor from VectorX
NOALIAS FORCE_INLINE
const V128 SIMD_VectorZ_VectorX( const V128 PCREF x )
{
    return __vrlimi( __vzero(), x, VMX_MASK_INSERT_Z, VMX_ROTATE_X_TO_Z );
}

/// SIMD VectorZ Constructor from VectorY
NOALIAS FORCE_INLINE
const V128 SIMD_VectorZ_VectorY( const V128 PCREF y )
{
    return __vrlimi( __vzero(), y, VMX_MASK_INSERT_Z, VMX_ROTATE_Y_TO_Z );
}

/// SIMD VectorZ Constructor from VectorW
NOALIAS FORCE_INLINE
const V128 SIMD_VectorZ_VectorW( const V128 PCREF w )
{
    return __vrlimi( __vzero(), w, VMX_MASK_INSERT_Z, VMX_ROTATE_W_TO_Z );
}

/// SIMD VectorZ Constructor from Vector3
NOALIAS FORCE_INLINE
const V128 SIMD_VectorZ_Vector3( const V128 PCREF src )
{
    return __vrlimi( __vzero(), src, VMX_MASK_INSERT_Z, 0 );
}

/// SIMD VectorZ Constructor from Vector4
NOALIAS FORCE_INLINE
const V128 SIMD_VectorZ_Vector4( const V128 PCREF src )
{
    return __vrlimi( __vzero(), src, VMX_MASK_INSERT_Z, 0 );
}



/// SIMD VectorW Constructor from float
NOALIAS FORCE_INLINE
const V128 SIMD_VectorW_Float( const F32& f )
{
    //const V128 result = { 0.0f, 0.0f, 0.0f, f };
    //return result;
    return __vrlimi( __vzero(), __lvlx( &f, 0 ), VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
}

/// SIMD VectorW Constructor from Scalar
NOALIAS FORCE_INLINE
const V128 SIMD_VectorW_Scalar( const V128 PCREF f )
{
    //return __vrlimi( __vzero(), f, VMX_MASK_INSERT_W, 0 );
    //return __vrlimi( f, __vzero(), VMX_MASK_INSERT_XYZ, 0 );
    return f; // OK ???
}

/// SIMD VectorW Constructor from VectorX
NOALIAS FORCE_INLINE
const V128 SIMD_VectorW_VectorX( const V128 PCREF x )
{
    return __vrlimi( __vzero(), x, VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
}

/// SIMD VectorW Constructor from VectorY
NOALIAS FORCE_INLINE
const V128 SIMD_VectorW_VectorY( const V128 PCREF y )
{
    return __vrlimi( __vzero(), y, VMX_MASK_INSERT_W, VMX_ROTATE_Y_TO_W );
}

/// SIMD VectorW Constructor from VectorZ
NOALIAS FORCE_INLINE
const V128 SIMD_VectorW_VectorZ( const V128 PCREF z )
{
    return __vrlimi( __vzero(), z, VMX_MASK_INSERT_W, VMX_ROTATE_Z_TO_W );
}

/// SIMD VectorW Constructor from Vector4
NOALIAS FORCE_INLINE
const V128 SIMD_VectorW_Vector4( const V128 PCREF src )
{
	//return __vrlimi( __vzero(), src, VMX_MASK_INSERT_W, 0 );
	//return __vrlimi( src, __vzero(), VMX_MASK_INSERT_XYZ, 0 );
	return src; // OK ???
}









/// SIMD Vector2 Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Vector2( const F32& x, const F32& y )
{
    //const V128 result = { x, y, 0.0f, 0.0f };
    //return result;
    return __vrlimi( __vrlimi( __lvlx( &x, 0 ),
                               __lvlx( &y, 0 ), VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y ),
                               __vzero(),       VMX_MASK_INSERT_ZW, 0 );
}

/// SIMD Vector3 Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Vector3( const F32& x, const F32& y, const F32& z )
{
    //const V128 result = { x, y, z, 0.0f };
    //return result;
    return __vrlimi( __vrlimi( __vrlimi( __lvlx( &x, 0 ),
                                         __lvlx( &y, 0 ), VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y ),
                                         __lvlx( &z, 0 ), VMX_MASK_INSERT_Z, VMX_ROTATE_X_TO_Z ),
                                         __vzero(),       VMX_MASK_INSERT_W, 0 );
}

/// SIMD Vector4 Constructor
NOALIAS FORCE_INLINE
const V128 SIMD_Vector4( const F32& x, const F32& y, const F32& z, const F32& w )
{
    //const V128 result = { x, y, z, w };
    //return result;
    return __vrlimi( __vrlimi( __vrlimi( __lvlx( &x, 0 ),
                                         __lvlx( &y, 0 ), VMX_MASK_INSERT_Y, VMX_ROTATE_X_TO_Y ),
                                         __lvlx( &z, 0 ), VMX_MASK_INSERT_Z, VMX_ROTATE_X_TO_Z ),
                                         __lvlx( &w, 0 ), VMX_MASK_INSERT_W, VMX_ROTATE_X_TO_W );
}

/// SIMD Vector2 Constructor from Scalar X & Y
NOALIAS FORCE_INLINE
const V128 SIMD_Vector2( const V128 PCREF x, const V128 PCREF y )
{
    //return __vmrghw( __vmrghw( x, __vzero() ), __vmrghw( y, __vzero() ) );
    return __vrlimi( __vrlimi( __vzero(), x, VMX_MASK_INSERT_X, 0 ), y, VMX_MASK_INSERT_Y, 0 );
}

/// SIMD Vector3 Constructor from Scalar X, Y & Z
NOALIAS FORCE_INLINE
const V128 SIMD_Vector3( const V128 PCREF x, const V128 PCREF y, const V128 PCREF z )
{
    return __vmrghw( __vmrghw( x, z ), __vmrghw( y, __vzero() ) );
}

/// SIMD Vector4 Constructor from Scalar X, Y, Z & W
NOALIAS FORCE_INLINE
const V128 SIMD_Vector4( const V128 PCREF x, const V128 PCREF y, const V128 PCREF z, const V128 PCREF w )
{
    return __vmrghw( __vmrghw( x, z ), __vmrghw( y, w ) );
}

/// SIMD Vector2 Scalar Constructor: Source data in HF32 format [ VMX - source replicated in all components ]
NOALIAS FORCE_INLINE
const V128 SIMD_Vector2_Scalar( const V128 PCREF xy )
{
    //return __vand( xy, SIMD_Mask<1,1,0,0>() );
    return __vrlimi( xy, __vzero(), VMX_MASK_INSERT_ZW, 0 );
}

/// SIMD Vector3 Scalar Constructor: Source data in HF32 format [ VMX - source replicated in all components ]
NOALIAS FORCE_INLINE
const V128 SIMD_Vector3_Scalar( const V128 PCREF xyz )
{
    //return __vand( xyz, SIMD_Mask<1,1,1,0>() );
    return __vrlimi( xyz, __vzero(), VMX_MASK_INSERT_W, 0 );
}

/// SIMD Vector4 Scalar Constructor: Source data in HF32 format [ VMX - source replicated in all components ]
NOALIAS FORCE_INLINE
const V128 SIMD_Vector4_Scalar( const V128 PCREF xyzw )
{
    return xyzw;
}

/// SIMD Vector4 Constructor from Vector3 & VectorW
NOALIAS __forceinline
const V128 SIMD_Vector4_Vector3W( const V128 PCREF vec3, const V128 PCREF vecW )
{
    return __vrlimi( vec3, vecW, VMX_MASK_INSERT_W, 0 );
}

/// SIMD Vector4 Constructor from VectorW
NOALIAS __forceinline
const V128 SIMD_Vector4_VectorW( const V128 PCREF vecW )
{
    return __vrlimi( vecW, __vzero(), VMX_MASK_INSERT_XYZ, 0 );
}

/// SIMD Vector3 Constructor from Vector4
NOALIAS FORCE_INLINE
const V128 SIMD_Vector3_Vector4( const V128 PCREF vec4 )
{
    return __vrlimi( vec4, __vzero(), VMX_MASK_INSERT_W, 0 );
}

/// SIMD Vector Truncate To Int
//NOALIAS FORCE_INLINE
//const V4_S32 SIMD_TruncateToInt( const V128 PCREF a )
//{
//	return V4_S32( __vrfiz( a ) );
//}

/// SIMD Vector Unaligned Load
NOALIAS FORCE_INLINE
const V128 SIMD_LoadUnaligned( const F32* ptr )
{
    return __loadunalignedvector( ptr );
}

/// SIMD Vector Unaligned Store
NOALIAS FORCE_INLINE
void SIMD_StoreUnaligned( const V128 PCREF a, F32* ptr )
{
    __storeunalignedvector( a, ptr );
}

/// SIMD Select: Selects fields from the inputs based on the selector mask: result = ( selector ? a : b )
NOALIAS FORCE_INLINE
const V128 SIMD_Select( const V128 PCREF selector, const V128 PCREF a, const V128 PCREF b )
{
    return __vsel( b, a, selector ); // NOTE: __vsel() selector is inverted!
}

/// SIMD Vector Permute
template < int ax, int ay, int az, int aw >
NOALIAS FORCE_INLINE
const V128 SIMD_Permute( const V128 PCREF a )
{
    return __vpermwi( a, VPERMWI_CONST( ax, ay, az, aw ) );
}

/// SIMD Vector Permute : 2 Sources
template < int ax, int ay, int bz, int bw >
NOALIAS FORCE_INLINE
const V128 SIMD_Permute( const V128 PCREF a, const V128 PCREF b )
{
    const V128 x = __vspltw( a, ax );
    const V128 y = __vspltw( a, ay );
    const V128 z = __vspltw( b, bz );
    const V128 w = __vspltw( b, bw );
    return __vmrghw( __vmrghw( x, z ), __vmrghw( y, w ) );
}

/// SIMD Vector Permute : 3 Sources
template < int ax, int by, int cz >
NOALIAS FORCE_INLINE
const V128 SIMD_Permute( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c )
{
    const V128 x = __vspltw( a, ax );
    const V128 y = __vspltw( b, by );
    const V128 z = __vspltw( c, cz );
    const V128 w = __vzero();
    return __vmrghw( __vmrghw( x, z ), __vmrghw( y, w ) );
}

/// SIMD Vector Permute : 3 Sources
template < int ax, int by, int cz, int cw >
NOALIAS FORCE_INLINE
const V128 SIMD_Permute( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c )
{
    const V128 x = __vspltw( a, ax );
    const V128 y = __vspltw( b, by );
    const V128 z = __vspltw( c, cz );
    const V128 w = __vspltw( c, cw );
    return __vmrghw( __vmrghw( x, z ), __vmrghw( y, w ) );
}

/// SIMD Vector Permute : 4 Sources
template < int ax, int by, int cz, int dw >
NOALIAS FORCE_INLINE
const V128 SIMD_Permute( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c, const V128 PCREF d )
{
    const V128 x = __vspltw( a, ax );
    const V128 y = __vspltw( b, by );
    const V128 z = __vspltw( c, cz );
    const V128 w = __vspltw( d, dw );
    return __vmrghw( __vmrghw( x, z ), __vmrghw( y, w ) );
}

/// SIMD Vector Splat2: Replicate component I to XY - Z&W unchanged
template < int I >
NOALIAS FORCE_INLINE
const V128 SIMD_Splat2( const V128 PCREF a )
{
    return __vpermwi( a, VPERMWI_CONST( I, I, VZ, VW ) );
}

/// SIMD Vector Splat3: Replicate component I to XYZ - W unchanged
template < int I >
NOALIAS FORCE_INLINE
const V128 SIMD_Splat3( const V128 PCREF a )
{
    return __vpermwi( a, VPERMWI_CONST( I, I, I, VW ) );
}

/// SIMD Vector Splat4: Replicate component I to XYZW
template < int I >
NOALIAS FORCE_INLINE
const V128 SIMD_Splat4( const V128 PCREF a )
{
    return __vspltw( a, I );
}

/// SIMD Vector Merge High Words
NOALIAS FORCE_INLINE
const V128 SIMD_MergeHigh( const V128 PCREF a, const V128 PCREF b )
{
    return __vmrghw( a, b );
}

/// SIMD Vector Merge Low Words
NOALIAS FORCE_INLINE
const V128 SIMD_MergeLow( const V128 PCREF a, const V128 PCREF b )
{
    return __vmrglw( a, b );
}

/// SIMD Vector Set X : Returns src with X set to scalar HF32
NOALIAS FORCE_INLINE
const V128 SIMD_SetX( const V128 PCREF src, const V128 PCREF scalar )
{
    //return __vsel( src, scalar, SIMD_Mask<1,0,0,0>() );
    return __vrlimi( src, scalar, VMX_MASK_INSERT_X, 0 );
}

/// SIMD Vector Set Y : Returns src with Y set to scalar HF32
NOALIAS FORCE_INLINE
const V128 SIMD_SetY( const V128 PCREF src, const V128 PCREF scalar )
{
    //return __vsel( src, scalar, SIMD_Mask<0,1,0,0>() );
    return __vrlimi( src, scalar, VMX_MASK_INSERT_Y, 0 );
}

/// SIMD Vector Set Z : Returns src with Z set to scalar HF32
NOALIAS FORCE_INLINE
const V128 SIMD_SetZ( const V128 PCREF src, const V128 PCREF scalar )
{
    //return __vsel( src, scalar, SIMD_Mask<0,0,1,0>() );
    return __vrlimi( src, scalar, VMX_MASK_INSERT_Z, 0 );
}

/// SIMD Vector Set W : Returns src with W set to scalar HF32
NOALIAS FORCE_INLINE
const V128 SIMD_SetW( const V128 PCREF src, const V128 PCREF scalar )
{
    //return __vsel( src, scalar, SIMD_Mask<0,0,0,1>() );
    return __vrlimi( src, scalar, VMX_MASK_INSERT_W, 0 );
}

/// SIMD Vector Bitwise NOT
NOALIAS FORCE_INLINE
const V128 SIMD_NOT( const V128 PCREF a )
{
    return __vnor( a, a );
}

/// SIMD Vector Bitwise AND
NOALIAS FORCE_INLINE
const V128 SIMD_AND( const V128 PCREF a, const V128 PCREF b )
{
    return __vand( a, b );
}

/// SIMD Vector Bitwise ANDNOT: a & ~b
NOALIAS FORCE_INLINE
const V128 SIMD_ANDNOT( const V128 PCREF a, const V128 PCREF b )
{
    return __vandc( a, b );
}

/// SIMD Vector Bitwise OR
NOALIAS FORCE_INLINE
const V128 SIMD_OR( const V128 PCREF a, const V128 PCREF b )
{
    return __vor( a, b );
}

/// SIMD Vector Bitwise XOR
NOALIAS FORCE_INLINE
const V128 SIMD_XOR( const V128 PCREF a, const V128 PCREF b )
{
    return __vxor( a, b );
}

/// SIMD Vector FP Abs
NOALIAS FORCE_INLINE
const V128 SIMD_Abs( const V128 PCREF src )
{
    return __vand( src, c_VectorAbsMask );
}

/// SIMD Vector FP Add
NOALIAS FORCE_INLINE
const V128 SIMD_Add( const V128 PCREF a, const V128 PCREF b )
{
    return __vaddfp( a, b );
}

/// SIMD Vector FP Subtract
NOALIAS FORCE_INLINE
const V128 SIMD_Subtract( const V128 PCREF a, const V128 PCREF b )
{
    return __vsubfp( a, b );
}

/// SIMD Vector FP Multiply
NOALIAS FORCE_INLINE
const V128 SIMD_Multiply( const V128 PCREF a, const V128 PCREF b )
{
    return __vmulfp( a, b );
}

/// SIMD Vector FP Multiply Add : A * B + C
NOALIAS FORCE_INLINE
const V128 SIMD_MultiplyAdd( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c )
{
    return __vmaddfp( a, b, c );
}

/// SIMD Vector FP Subtract Multiply : A - B * C
NOALIAS FORCE_INLINE
const V128 SIMD_SubtractMultiply( const V128 PCREF a, const V128 PCREF b, const V128 PCREF c )
{
    return __vnmsubfp( b, c, a ); // vnmsubfp(x,y,z) == -(x*y-z) == z-x*y
}

/// SIMD Vector FP Dot Product 2 - Result is HF32 [ VMX : replicated to all elements ]
NOALIAS FORCE_INLINE
const V128 SIMD_DotProduct2( const V128 PCREF a, const V128 PCREF b )
{
    return __vmsum3fp( __vrlimi( a, __vzero(), VMX_MASK_INSERT_ZW, 0 ), __vrlimi( b, __vzero(), VMX_MASK_INSERT_ZW, 0 ) );
}

/// SIMD Vector FP Dot Product 3 - Result is HF32 [ VMX : replicated to all elements ]
NOALIAS FORCE_INLINE
const V128 SIMD_DotProduct3( const V128 PCREF a, const V128 PCREF b )
{
    return __vmsum3fp( a, b );
}

/// SIMD Vector FP Dot Product 4 - Result is HF32 [ VMX : replicated to all elements ]
NOALIAS FORCE_INLINE
const V128 SIMD_DotProduct4( const V128 PCREF a, const V128 PCREF b )
{
    return __vmsum4fp( a, b );
}

/// SIMD Vector FP Sum of Elements 2 - Result is HF32 [ VMX : replicated to all components ]
NOALIAS FORCE_INLINE
const V128 SIMD_Sum2( const V128 PCREF a )
{
    //return __vmsum3fp( __vand( a, SIMD_Mask<1,1,0,0>() ), c_VectorOne );
    return __vaddfp( __vspltw( a, VX ), __vspltw( a, VY ) );
}

/// SIMD Vector FP Sum of Elements 3 - Result is HF32 [ VMX : replicated to all components ]
NOALIAS FORCE_INLINE
const V128 SIMD_Sum3( const V128 PCREF a )
{
    return __vmsum3fp( a, c_VectorOne );
}

/// SIMD Vector FP Sum of Elements 4 - Result is HF32 [ VMX : replicated to all components ]
NOALIAS FORCE_INLINE
const V128 SIMD_Sum4( const V128 PCREF a )
{
    return __vmsum4fp( a, c_VectorOne );
}

/// SIMD Vector FP Cross Product
NOALIAS FORCE_INLINE
const V128 SIMD_CrossProduct( const V128 PCREF a, const V128 PCREF b )
{
    static const unsigned __ = VW;
    const V128 a_yzx = SIMD_Permute<VY,VZ,VX,__>( a );
    const V128 a_zxy = SIMD_Permute<VZ,VX,VY,__>( a );
    const V128 b_yzx = SIMD_Permute<VY,VZ,VX,__>( b );
    const V128 b_zxy = SIMD_Permute<VZ,VX,VY,__>( b );
    return SIMD_SubtractMultiply( SIMD_Multiply( a_yzx, b_zxy ), a_zxy, b_yzx );
}

/// SIMD Vector FP Min
NOALIAS FORCE_INLINE
const V128 SIMD_Min( const V128 PCREF a, const V128 PCREF b )
{
    return __vminfp( a, b );
}

/// SIMD Vector FP Max
NOALIAS FORCE_INLINE
const V128 SIMD_Max( const V128 PCREF a, const V128 PCREF b )
{
    return __vmaxfp( a, b );
}

/// SIMD Vector Reciprocal X
NOALIAS FORCE_INLINE
const V128 SIMD_Reciprocal1( const V128 PCREF a )
{
    const V128 re_a = __vrlimi( __vrefp( a ), __vzero(), VMX_MASK_INSERT_YZW, 0 );
    const V128 result = __vmaddfp( re_a, __vnmsubfp( re_a, a, c_VectorOne ), re_a );
    return result;
}

/// SIMD Vector Reciprocal XY
NOALIAS FORCE_INLINE
const V128 SIMD_Reciprocal2( const V128 PCREF a )
{
    const V128 re_a = __vrlimi( __vrefp( a ), __vzero(), VMX_MASK_INSERT_ZW, 0 );
    const V128 result = __vmaddfp( re_a, __vnmsubfp( re_a, a, c_VectorOne ), re_a );
    return result;
}

/// SIMD Vector Reciprocal XYZ
NOALIAS FORCE_INLINE
const V128 SIMD_Reciprocal3( const V128 PCREF a )
{
    const V128 re_a = __vrlimi( __vrefp( a ), __vzero(), VMX_MASK_INSERT_W, 0 );
    const V128 result = __vmaddfp( re_a, __vnmsubfp( re_a, a, c_VectorOne ), re_a );
    return result;
}

/// SIMD Vector Reciprocal XYZW
NOALIAS FORCE_INLINE
const V128 SIMD_Reciprocal4( const V128 PCREF a )
{
    #if 1
    const __vector4 one( c_VectorOne );
    const __vector4 B( a );
    const __vector4 y0( __vrefp( B ) );
    const __vector4 t0( __vnmsubfp( y0, B, one ) );
    const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
    // TODO?: Use validation method from ReciprocalSqrt?
    return __vsel( y0, y1, __vcmpeqfp( y1, y1 ) );
    #else
    // Full 24 bit division of A/B from AltiVec PEM
    const __vector4 one( c_VectorOne );
    const __vector4 A( one );
    const __vector4 B( a );
    const __vector4 y0( __vrefp( B ) );
    const __vector4 t0( __vnmsubfp( y0, B, one ) );
    const __vector4 y1( __vmaddfp( y0, t0, y0 ) );
    const __vector4 t1( __vnmsubfp( y1, B, one ) );
    const __vector4 y2( __vmaddfp( y1, t1, y1 ) );
    const __vector4 Q1( /*__vmulfp( A, y2 )*/ y2 ); // A * y2 = y2
    const __vector4 R( __vnmsubfp( B, Q1, A ) );
    const __vector4 Q2( __vmaddfp( R, y2, Q1 ) );
    return __vsel( y0, Q2, __vcmpeqfp( Q2, Q2 ) );
    #endif
}

/// SIMD Vector Reciprocal Square Root X
NOALIAS FORCE_INLINE
const V128 SIMD_ReciprocalSqrt1( const V128 PCREF B )
{
    //const V128 rse_a = __vrlimi( __vrsqrtefp( a ), __vzero(), VMX_MASK_INSERT_YZW, 0 );
    //const V128 result = __vmaddfp( __vnmsubfp( a, __vmulfp( rse_a, rse_a ), c_VectorOne ), __vmulfp( rse_a, c_VectorHalf ), rse_a );
    //return result;
    const __vector4 zero( __vzero() );
    const __vector4 half( c_VectorHalf );
    const __vector4 one( c_VectorOne );
    const __vector4 fltMax( c_VectorFltMax );
    const __vector4 y0( __vrlimi( __vrsqrtefp( B ), zero, VMX_MASK_INSERT_YZW, 0 ) );
    const __vector4 t0( __vmulfp( y0, y0 ) );
    const __vector4 t1( __vmulfp( y0, half ) );
    const __vector4 t2( __vnmsubfp( B, t0, one ) );
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );
    const __vector4 B_GT_zero( __vcmpgtfp( B, zero ) );
    const __vector4 valid_y1( __vand( B_GT_zero, B_LT_fltMax ) );
    return __vsel( y0, y1, valid_y1 );
}

/// SIMD Vector Reciprocal Square Root XY
NOALIAS FORCE_INLINE
const V128 SIMD_ReciprocalSqrt2( const V128 PCREF B )
{
    //const V128 rse_a = __vrlimi( __vrsqrtefp( a ), __vzero(), VMX_MASK_INSERT_ZW, 0 );
    //const V128 result = __vmaddfp( __vnmsubfp( a, __vmulfp( rse_a, rse_a ), c_VectorOne ), __vmulfp( rse_a, c_VectorHalf ), rse_a );
    //return result;
    const __vector4 zero( __vzero() );
    const __vector4 half( c_VectorHalf );
    const __vector4 one( c_VectorOne );
    const __vector4 fltMax( c_VectorFltMax );
    const __vector4 y0( __vrlimi( __vrsqrtefp( B ), zero, VMX_MASK_INSERT_ZW, 0 ) );
    const __vector4 t0( __vmulfp( y0, y0 ) );
    const __vector4 t1( __vmulfp( y0, half ) );
    const __vector4 t2( __vnmsubfp( B, t0, one ) );
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );
    const __vector4 B_GT_zero( __vcmpgtfp( B, zero ) );
    const __vector4 valid_y1( __vand( B_GT_zero, B_LT_fltMax ) );
    return __vsel( y0, y1, valid_y1 );
}

/// SIMD Vector Reciprocal Square Root XYZ
NOALIAS FORCE_INLINE
const V128 SIMD_ReciprocalSqrt3( const V128 PCREF B )
{
    //const V128 rse_a = __vrlimi( __vrsqrtefp( a ), __vzero(), VMX_MASK_INSERT_W, 0 );
    //const V128 result = __vmaddfp( __vnmsubfp( a, __vmulfp( rse_a, rse_a ), c_VectorOne ), __vmulfp( rse_a, c_VectorHalf ), rse_a );
    //return result;
    const __vector4 zero( __vzero() );
    const __vector4 half( c_VectorHalf );
    const __vector4 one( c_VectorOne );
    const __vector4 fltMax( c_VectorFltMax );
    const __vector4 y0( __vrlimi( __vrsqrtefp( B ), zero, VMX_MASK_INSERT_W, 0 ) );
    const __vector4 t0( __vmulfp( y0, y0 ) );
    const __vector4 t1( __vmulfp( y0, half ) );
    const __vector4 t2( __vnmsubfp( B, t0, one ) );
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );
    const __vector4 B_GT_zero( __vcmpgtfp( B, zero ) );
    const __vector4 valid_y1( __vand( B_GT_zero, B_LT_fltMax ) );
    return __vsel( y0, y1, valid_y1 );
}

/// SIMD Vector Reciprocal Square Root XYZW
NOALIAS FORCE_INLINE
const V128 SIMD_ReciprocalSqrt4( const V128 PCREF B )
{
    const __vector4 zero( __vzero() );
    const __vector4 half( c_VectorHalf );
    const __vector4 one( c_VectorOne );
    const __vector4 fltMax( c_VectorFltMax );
    const __vector4 y0( __vrsqrtefp( B ) );
    const __vector4 t0( __vmulfp( y0, y0 ) );
    const __vector4 t1( __vmulfp( y0, half ) );
    const __vector4 t2( __vnmsubfp( B, t0, one ) );
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );
    const __vector4 B_GT_zero( __vcmpgtfp( B, zero ) );
    const __vector4 valid_y1( __vand( B_GT_zero, B_LT_fltMax ) );
    return __vsel( y0, y1, valid_y1 );
}

/// SIMD Vector Square Root X
NOALIAS FORCE_INLINE
const V128 SIMD_Sqrt1( const V128 PCREF a )
{
    const V128 valid = __vcmpgefp( a, c_VectorFltMin );
    const V128 result = __vmulfp( a, SIMD_ReciprocalSqrt1( a ) );
    return __vsel( __vzero(), result, valid );
}

/// SIMD Vector Square Root XY
NOALIAS FORCE_INLINE
const V128 SIMD_Sqrt2( const V128 PCREF a )
{
    const V128 valid = __vcmpgefp( a, c_VectorFltMin );
    const V128 result = __vmulfp( a, SIMD_ReciprocalSqrt2( a ) );
    return __vsel( __vzero(), result, valid );
}

/// SIMD Vector Square Root XYZ
NOALIAS FORCE_INLINE
const V128 SIMD_Sqrt3( const V128 PCREF a )
{
    const V128 valid = __vcmpgefp( a, c_VectorFltMin );
    const V128 result = __vmulfp( a, SIMD_ReciprocalSqrt3( a ) );
    return __vsel( __vzero(), result, valid );
}

/// SIMD Vector Square Root XYZW
NOALIAS FORCE_INLINE
const V128 SIMD_Sqrt4( const V128 PCREF a )
{
    const V128 valid = __vcmpgefp( a, c_VectorFltMin );
    const V128 result = __vmulfp( a, SIMD_ReciprocalSqrt4( a ) );
    return __vsel( __vzero(), result, valid );
}

/// SIMD Vector Normalise XY
NOALIAS FORCE_INLINE
const V128 SIMD_Normalise2( const V128 PCREF a )
{
    return __vmulfp( a, SIMD_ReciprocalSqrt2( SIMD_DotProduct2( a, a ) ) );
}

/// SIMD Vector Normalise XYZ
NOALIAS FORCE_INLINE
const V128 SIMD_Normalise3( const V128 PCREF a )
{
    return __vmulfp( a, SIMD_ReciprocalSqrt3( SIMD_DotProduct3( a, a ) ) );
}

/// SIMD Vector Normalise XYZW
NOALIAS FORCE_INLINE
const V128 SIMD_Normalise4( const V128 PCREF a )
{
    return __vmulfp( a, SIMD_ReciprocalSqrt4( SIMD_DotProduct4( a, a ) ) );
}

/// SIMD Vector Compare ==
NOALIAS FORCE_INLINE
const V128 SIMD_Vector_Selector_EQ( const V128 PCREF a, const V128 PCREF b )
{
    return __vcmpeqfp( a, b );
}

/// SIMD Vector Compare !=
NOALIAS FORCE_INLINE
const V128 SIMD_Vector_Selector_NE( const V128 PCREF a, const V128 PCREF b )
{
    const V128 eq = __vcmpeqfp( a, b );
    return __vnor( eq, eq );
}

/// SIMD Vector Compare >=
NOALIAS FORCE_INLINE
const V128 SIMD_Vector_Selector_GE( const V128 PCREF a, const V128 PCREF b )
{
    return __vcmpgefp( a, b );
}

/// SIMD Vector Compare >
NOALIAS FORCE_INLINE
const V128 SIMD_Vector_Selector_GT( const V128 PCREF a, const V128 PCREF b )
{
    return __vcmpgtfp( a, b );
}

/// SIMD Vector Compare <=
NOALIAS FORCE_INLINE
const V128 SIMD_Vector_Selector_LE( const V128 PCREF a, const V128 PCREF b )
{
    return __vcmpgefp( b, a );
}

/// SIMD Vector Compare <
NOALIAS FORCE_INLINE
const V128 SIMD_Vector_Selector_LT( const V128 PCREF a, const V128 PCREF b )
{
    return __vcmpgtfp( b, a );
}


/// SIMD Vector3 FP Round To Integer : Towards minus infinity
//NOALIAS __forceinline const V128 SIMD_Vector3_RoundToIntMinusInf( const V128 PCREF src )
//{
//    return __vrfim( src );
//}

/// SIMD Vector3 FP Round To Integer : Towards plus infinity
//NOALIAS __forceinline const V128 SIMD_Vector3_RoundToIntPlusInf( const V128 PCREF src )
//{
//    return __vrfip( src );
//}

/// SIMD Vector3 FP Round To Integer : Towards nearest
//NOALIAS __forceinline const V128 SIMD_Vector3_RoundToIntNearest( const V128 PCREF src )
//{
//    return __vrfin( src );
//}

/// SIMD Vector3 FP Round To Integer : Towards zero
//NOALIAS __forceinline const V128 SIMD_Vector3_RoundToIntZero( const V128 PCREF src )
//{
//    return __vrfiz( src );
//}



/// SIMD Vector3 FP Compare == ( true iff true for ALL 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_EQ( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a3, b3, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare !=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_NE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a3, b3, &CR );
    return ( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare >=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_GE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgefpR( a3, b3, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare <=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_LE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgefpR( b3, a3, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare >
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_GT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgtfpR( a3, b3, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare <
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_All_LT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgtfpR( b3, a3, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}




/// SIMD Vector3 FP Compare == ( true if true for ANY of 3 fields )
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_EQ( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a3, b3, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare !=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_NE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpeqfpR( a3, b3, &CR );
    return ~( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare >=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_GE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgefpR( a3, b3, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare <=
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_LE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgefpR( b3, a3, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare >
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_GT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgtfpR( a3, b3, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector3 FP Compare <
NOALIAS __forceinline const U32 SIMD_Vector3_Compare_Any_LT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    const __vector4 a3( __vpermwi( a, VPERMWI_CONST(VX,VY,VZ,VZ) ) ); // Replicate Z into W
    const __vector4 b3( __vpermwi( b, VPERMWI_CONST(VX,VY,VZ,VZ) ) );
    __vcmpgtfpR( b3, a3, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}




/// SIMD Vector3 FP Round To Integer : Towards minus infinity
//NOALIAS __forceinline const V128 SIMD_Vector4_RoundToIntMinusInf( const V128 PCREF src )
//{
//    return __vrfim( src );
//}

/// SIMD Vector3 FP Round To Integer : Towards plus infinity
//NOALIAS __forceinline const V128 SIMD_Vector4_RoundToIntPlusInf( const V128 PCREF src )
//{
//    return __vrfip( src );
//}

/// SIMD Vector3 FP Round To Integer : Towards nearest
//NOALIAS __forceinline const V128 SIMD_Vector4_RoundToIntNearest( const V128 PCREF src )
//{
//    return __vrfin( src );
//}

/// SIMD Vector3 FP Round To Integer : Towards zero
//NOALIAS __forceinline const V128 SIMD_Vector4_RoundToIntZero( const V128 PCREF src )
//{
//    return __vrfiz( src );
//}



/// SIMD Vector4 FP Compare == ( true iff true for ALL 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_EQ( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpeqfpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare !=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_NE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpeqfpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare >=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_GE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgefpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare <=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_LE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgefpR( b, a, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare >
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_GT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgtfpR( a, b, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare <
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_All_LT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgtfpR( b, a, &CR );
    return ( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}




/// SIMD Vector4 FP Compare == ( true if true for ANY of 4 fields )
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_EQ( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpeqfpR( a, b, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare !=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_NE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpeqfpR( a, b, &CR );
    return ~( CR >> VMX_CR6_ALL_TRUE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare >=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_GE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgefpR( a, b, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare <=
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_LE( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgefpR( b, a, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare >
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_GT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgtfpR( a, b, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}

/// SIMD Vector4 FP Compare <
NOALIAS __forceinline const U32 SIMD_Vector4_Compare_Any_LT( const V128 PCREF a, const V128 PCREF b )
{
    U32 CR;
    __vcmpgtfpR( b, a, &CR );
    return ~( CR >> VMX_CR6_ALL_FALSE_BIT ) & 1;
}








//FORCE_INLINE
//const V128 SIMD_RotateLeft1( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 4 );
//}
//
//FORCE_INLINE
//const V128 SIMD_RotateLeft2( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 8 );
//}
//
//FORCE_INLINE
//const V128 SIMD_RotateLeft3( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 12 );
//}
//
//FORCE_INLINE
//const V128 SIMD_RotateRight1( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 12 );
//}
//
//FORCE_INLINE
//const V128 SIMD_RotateRight2( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 8 );
//}
//
//FORCE_INLINE
//const V128 SIMD_RotateRight3( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 4 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_X_to_Y( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 12 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_X_to_Z( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 8 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_X_to_W( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 4 );
//}
//
//
//FORCE_INLINE
//const V128 SIMD_Rotate_Y_to_X( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 4 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_Y_to_Z( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 12 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_Y_to_W( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 8 );
//}
//
//
//FORCE_INLINE
//const V128 SIMD_Rotate_Z_to_X( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 8 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_Z_to_Y( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 4 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_Z_to_W( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 12 );
//}
//
//
//FORCE_INLINE
//const V128 SIMD_Rotate_W_to_X( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 12 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_W_to_Y( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 8 );
//}
//
//FORCE_INLINE
//const V128 SIMD_Rotate_W_to_Z( const V128 PCREF src )
//{
//    return __vsldoi( src, src, 4 );
//}


#endif // #if defined( _VMX128_ )

}; // namespace Maths


#endif // SIMD_VMX_INL




