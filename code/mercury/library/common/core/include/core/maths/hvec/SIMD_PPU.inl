#ifndef MATHS_HVEC_SIMD_PPU_INL
#define MATHS_HVEC_SIMD_PPU_INL

#ifdef _MSC_VER
#pragma once
#endif

#if defined( _PPU_ )

#include <altivec.h>
namespace Maths
{

inline const V128 SIMD_Multiply( const V128 a, const V128 b ) __attribute__(( pure, always_inline ));
inline const V128 SIMD_Multiply( const V128 a, const V128 b )
{
    //return vec_madd( a, b, (vector float)( 0.0f, 0.0f, 0.0f, 0.0f ) );
    return vec_madd( a, b, Const< vector float >::Zero() );
}

/// SIMD Vector Merge High Words
inline const V128 SIMD_MergeHigh( const V128 a, const V128 b ) __attribute__(( const, always_inline ));
inline const V128 SIMD_MergeHigh( const V128 a, const V128 b )
{
    return vec_mergeh( a, b );
}

/// SIMD Vector Merge Low Words
inline const V128 SIMD_MergeLow( const V128 a, const V128 b ) __attribute__(( const, always_inline ));
inline const V128 SIMD_MergeLow( const V128 a, const V128 b )
{
    return vec_mergel( a, b );
}

template < int I > NOALIAS inline const V128 SIMD_Splat4( const V128 a ) __attribute__(( const, always_inline ));
template < int I > NOALIAS inline const V128 SIMD_Splat4( const V128 a )
{
	return vec_splat( a, I );
}


//
// PS3 PPU SIMD Scalar & Vector Maths Macros & Inline Functions
//

template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7,
           unsigned char I8, unsigned char I9, unsigned char IA, unsigned char IB,
           unsigned char IC, unsigned char ID, unsigned char IE, unsigned char IF >
class PermuteByte
{
public:
    static const vector unsigned char Pattern __attribute__(( weak ));
};

template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7,
           unsigned char I8, unsigned char I9, unsigned char IA, unsigned char IB,
           unsigned char IC, unsigned char ID, unsigned char IE, unsigned char IF >
const vector unsigned char PermuteByte<I0,I1,I2,I3,I4,I5,I6,I7,I8,I9,IA,IB,IC,ID,IE,IF>::Pattern =
{
    I0, I1, I2, I3,
    I4, I5, I6, I7,
    I8, I9, IA, IB,
    IC, ID, IE, IF
};


template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7 >
class PermuteHalf
{
public:
    static const vector unsigned char Pattern __attribute__(( weak ));
};

template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7 >
const vector unsigned char PermuteHalf<I0,I1,I2,I3,I4,I5,I6,I7>::Pattern =
{
    (I0<<1)+0, (I0<<1)+1, (I1<<1)+0, (I1<<1)+1,
    (I2<<1)+0, (I2<<1)+1, (I3<<1)+0, (I3<<1)+1,
    (I4<<1)+0, (I4<<1)+1, (I5<<1)+0, (I5<<1)+1,
    (I6<<1)+0, (I6<<1)+1, (I7<<1)+0, (I7<<1)+1
};


template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW >
class PermuteWord
{
public:
    static const vector unsigned char Pattern __attribute__(( weak ));
};

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW >
const vector unsigned char PermuteWord<IX,IY,IZ,IW>::Pattern =
{
    (IX<<2)+0, (IX<<2)+1, (IX<<2)+2, (IX<<2)+3,
    (IY<<2)+0, (IY<<2)+1, (IY<<2)+2, (IY<<2)+3,
    (IZ<<2)+0, (IZ<<2)+1, (IZ<<2)+2, (IZ<<2)+3,
    (IW<<2)+0, (IW<<2)+1, (IW<<2)+2, (IW<<2)+3
};


template < unsigned char I > inline const vector unsigned char vec_permute_pattern_word() __attribute__(( pure, always_inline ));
template < unsigned char I > inline const vector unsigned char vec_permute_pattern_word()
{
    StaticAssert( I < 8, I_MUST_BE_LESS_THAN_8 );
    return PermuteWord<I,I,I,I>::Pattern;
}

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector unsigned char vec_permute_pattern_word() __attribute__(( pure, always_inline ));
template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector unsigned char vec_permute_pattern_word()
{
    StaticAssert( IX < 8, IX_MUST_BE_LESS_THAN_8 );
    StaticAssert( IY < 8, IY_MUST_BE_LESS_THAN_8 );
    StaticAssert( IZ < 8, IZ_MUST_BE_LESS_THAN_8 );
    StaticAssert( IW < 8, IW_MUST_BE_LESS_THAN_8 );
    return PermuteWord<IX,IY,IZ,IW>::Pattern;
}


//inline const vector bool int __builtin_vec_cmpeq( const vector bool int a, const vector bool int b ) __attribute__(( const, always_inline ));
//inline const vector bool int __builtin_vec_cmpeq( const vector bool int a, const vector bool int b )
//{
//    return (vector bool int)::vec_cmpeq( (vector unsigned int)a, (vector unsigned int)b );
//}
//
//inline const vector bool short __builtin_vec_cmpeq( const vector bool short a, const vector bool short b ) __attribute__(( const, always_inline ));
//inline const vector bool short __builtin_vec_cmpeq( const vector bool short a, const vector bool short b )
//{
//    return (vector bool short)::vec_cmpeq( (vector unsigned short)a, (vector unsigned short)b );
//}
//
//inline const vector bool char __builtin_vec_cmpeq( const vector bool char a, const vector bool char b ) __attribute__(( const, always_inline ));
//inline const vector bool char __builtin_vec_cmpeq( const vector bool char a, const vector bool char b )
//{
//    return (vector bool char)::vec_cmpeq( (vector unsigned char)a, (vector unsigned char)b );
//}



#define vec_merge_xy vec_mergeh
#define vec_merge_zw vec_mergel

/// Make vector3 from scalar x, y & z : the X component of each scalar must contain the value. (splatted scalars not required)
inline const vector float vec_make_vector3( const vector float x, const vector float y, const vector float z ) __attribute(( const, always_inline ));
inline const vector float vec_make_vector3( const vector float x, const vector float y, const vector float z )
{
    return vec_merge_xy( vec_merge_xy( x, z ), y ); // NOTE: w component is UNDEFINED, not zero!
}

/// Make vector4 from scalar x, y, z & w : the X component of each scalar must contain the value. (splatted scalars not required)
inline const vector float vec_make_vector4( const vector float x, const vector float y, const vector float z, const vector float w ) __attribute(( const, always_inline ));
inline const vector float vec_make_vector4( const vector float x, const vector float y, const vector float z, const vector float w )
{
    return vec_merge_xy( vec_merge_xy( x, z ),
                         vec_merge_xy( y, w ) );
}






inline const vector float vec_zero() __attribute__(( const, always_inline ));
inline const vector float vec_zero()
{
    return (vector float)vec_splat_s32( 0 );
}

inline const vector float vec_mul( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
inline const vector float vec_mul( const vector float a, const vector float b )
{
    return vec_madd( a, b, Const< vector float >::Zero() );
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define PPU_VECTOR_RECIPROCAL_UNSAFE( B, out_rcp_B )                            \
{                                                                               \
    const vector float        y0( vec_re( B ) );                                \
    const vector float        absMask( Const< vector float >::AbsMask() );      \
    const vector float        abs_B( vec_and( B, absMask ) );                   \
    const vector float        fltMin( Const< vector float >::FltMin() );        \
    const vector float        one( Const< vector float >::One() );              \
    const vector bool int     valid( vec_cmpgt( abs_B, fltMin ) );              \
    const vector float        t( vec_nmsub( y0, B, one ) );                     \
    const vector float        y1( vec_madd( y0, t, y0 ) );                      \
    /* If B is not in the valid range, return the reciprocal estimate */        \
    out_rcp_B = vec_sel( y0, y1, valid );                                       \
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define PPU_VECTOR_RECIPROCAL( B, invalid_result, out_rcp_B, out_valid )                \
{                                                                                       \
    const vector float        y0( vec_re( B ) );                                        \
    const vector float        absMask( Const< vector float >::AbsMask() );              \
    const vector float        abs_B( vec_and( B, absMask ) );                           \
    const vector float        fltMin( Const< vector float >::FltMin() );                \
    const vector float        fltMax( Const< vector float >::FltMax() );                \
    const vector float        one( Const< vector float >::One() );                      \
    const vector bool int     abs_B_GT_fltMin( vec_cmpgt( abs_B, fltMin ) );            \
    const vector bool int     abs_B_LT_fltMax( vec_cmpgt( fltMax, abs_B ) );            \
    const vector bool int     valid_B( vec_and( abs_B_GT_fltMin, abs_B_LT_fltMax ) );   \
    const vector float        t( vec_nmsub( y0, B, one ) );                             \
    const vector float        y1( vec_madd( y0, t, y0 ) );                              \
    /* TODO: Validate y0, y1? */                                                        \
    out_rcp_B = vec_sel( invalid_result, y1, valid_B );                                 \
    out_valid = valid_B;                                                                \
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define PPU_VECTOR_RECIPROCAL_SQRT( B, invalid_result, out_rcp_sqrt_B, out_valid )      \
{                                                                                       \
    const vector float    y0( vec_rsqrte( B ) );                                        \
    const vector float    fltMin( Const< vector float >::FltMin() );                    \
    const vector float    fltMax( Const< vector float >::FltMax() );                    \
    const vector float    half( Const< vector float >::Half() );                        \
    const vector float    one( Const< vector float >::One() );                          \
    const vector bool int B_GT_fltMin( vec_cmpgt( B, fltMin ) );                        \
    const vector bool int B_LT_fltMax( vec_cmpgt( fltMax, B ) );                        \
    const vector bool int valid_B( vec_and( B_GT_fltMin, B_LT_fltMax ) );               \
    const vector float    t0( vec_mul( y0, y0 ) );                                      \
    const vector float    t1( vec_mul( y0, half ) );                                    \
    const vector float    t2( vec_nmsub( B, t0, one ) );                                \
    const vector float    y1( vec_madd( t2, t1, y0 ) );                                 \
    /* TODO: Validate y0, y1? */                                                        \
    out_rcp_sqrt_B = vec_sel( invalid_result, y1, valid_B );                            \
    out_valid = valid_B;                                                                \
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define PPU_VECTOR_RECIPROCAL_SQRT3( B0, B1, B2, out_rcp_sqrt_B0, out_rcp_sqrt_B1, out_rcp_sqrt_B2 ) \
{                                                                                       \
    const vector float    zero( Const< vector float >::Zero() );                        \
    const vector float    B0_y0( vec_rsqrte( B0 ) );                                    \
    const vector float    B1_y0( vec_rsqrte( B1 ) );                                    \
    const vector float    B2_y0( vec_rsqrte( B2 ) );                                    \
    const vector float    fltMin( Const< vector float >::FltMin() );                    \
    const vector float    fltMax( Const< vector float >::FltMax() );                    \
    const vector float    half( Const< vector float >::Half() );                        \
    const vector float    one( Const< vector float >::One() );                          \
    const vector bool int B0_GT_fltMin( vec_cmpgt( B0, fltMin ) );                      \
    const vector bool int B1_GT_fltMin( vec_cmpgt( B1, fltMin ) );                      \
    const vector bool int B2_GT_fltMin( vec_cmpgt( B2, fltMin ) );                      \
    const vector bool int B0_LT_fltMax( vec_cmpgt( fltMax, B0 ) );                      \
    const vector bool int B1_LT_fltMax( vec_cmpgt( fltMax, B1 ) );                      \
    const vector bool int B2_LT_fltMax( vec_cmpgt( fltMax, B2 ) );                      \
    const vector bool int B0_valid( vec_and( B0_GT_fltMin, B0_LT_fltMax ) );            \
    const vector bool int B1_valid( vec_and( B1_GT_fltMin, B1_LT_fltMax ) );            \
    const vector bool int B2_valid( vec_and( B2_GT_fltMin, B2_LT_fltMax ) );            \
    const vector float    B0_t0( vec_mul( B0_y0, B0_y0 ) );                             \
    const vector float    B1_t0( vec_mul( B1_y0, B1_y0 ) );                             \
    const vector float    B2_t0( vec_mul( B2_y0, B2_y0 ) );                             \
    const vector float    B0_t1( vec_mul( B0_y0, half ) );                              \
    const vector float    B1_t1( vec_mul( B1_y0, half ) );                              \
    const vector float    B2_t1( vec_mul( B2_y0, half ) );                              \
    const vector float    B0_t2( vec_nmsub( B0, B0_t0, one ) );                         \
    const vector float    B1_t2( vec_nmsub( B1, B1_t0, one ) );                         \
    const vector float    B2_t2( vec_nmsub( B2, B2_t0, one ) );                         \
    const vector float    B0_y1( vec_madd( B0_t2, B0_t1, B0_y0 ) );                     \
    const vector float    B1_y1( vec_madd( B1_t2, B1_t1, B1_y0 ) );                     \
    const vector float    B2_y1( vec_madd( B2_t2, B2_t1, B2_y0 ) );                     \
    out_rcp_sqrt_B0 = vec_sel( zero, B0_y1, B0_valid );                                 \
    out_rcp_sqrt_B1 = vec_sel( zero, B1_y1, B1_valid );                                 \
    out_rcp_sqrt_B2 = vec_sel( zero, B2_y1, B2_valid );                                 \
}

template < int T_PrecisionBits > const vector float SIMD_Rcp( const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_Rcp( const vector float B )
{
    //const vector float zero = Const< vector float >::Zero();
    //vector float       rcp_B;
    //vector bool int    valid;
    //PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    //return rcp_B;
    
    const vector float      y0( vec_re( B ) );
    const vector float      absMask( Const< vector float >::AbsMask() );
    const vector float      abs_B( vec_and( B, absMask ) );
    const vector float      fltMin( Const< vector float >::FltMin() );
    const vector float      zero( Const< vector float >::Zero() );
    const vector float      one( Const< vector float >::One() );
    const vector bool int   valid( vec_cmpgt( abs_B, fltMin ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return vec_sel( zero, y0, valid );
    }
    else
    {
        const vector float t0( vec_nmsub( y0, B, one ) );
        const vector float y1( vec_madd( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return vec_sel( zero, y1, valid );
        }
        else
        {
            const vector float t1( vec_nmsub( y1, B, one ) );
            const vector float y2( vec_madd( y1, t1, y1 ) );
            return vec_sel( zero, y2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_Div( const vector float A, const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_Div( const vector float A, const vector float B )
{
    //const vector float zero = Const< vector float >::Zero();
    //vector float       rcp_B;
    //vector bool int    valid;
    //PPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    //return vec_madd( A, rcp_B, zero );
    
    const vector float      y0( vec_re( B ) );
    const vector float      zero = Const< vector float >::Zero();
    const vector float      one( Const< vector float >::One() );
    const vector float      flt_min( Const< vector float >::FltMin() );
    const vector float      abs_mask( Const< vector float >::AbsMask() );
    const vector float      abs_B( vec_and( B, abs_mask ) );
    const vector bool int   valid( vec_cmpgt( abs_B, flt_min ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        const vector float q0( vec_mul( A, y0 ) );
        return vec_sel( zero, q0, valid );
    }
    else
    {
        const vector float t0( vec_nmsub( y0, B, one ) );
        const vector float y1( vec_madd( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            const vector float q1( vec_mul( A, y1 ) );
            return vec_sel( zero, q1, valid );
        }
        else
        {
            const vector float t1( vec_nmsub( y1, B, one ) );
            const vector float y2( vec_madd( y1, t1, y1 ) );
            const vector float q2( vec_mul( A, y2 ) );
            return vec_sel( zero, q2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_UnsafeDivide( const vector float A, const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_UnsafeDivide( const vector float A, const vector float B )
{
    //const vector float zero = Const< vector float >::Zero();
    //vector float rcp_B;
    //PPU_VECTOR_RECIPROCAL_UNSAFE( B, rcp_B );
    //return vec_madd( A, rcp_B, zero );
    
    const vector float      y0( vec_re( B ) );
    const vector float      one( Const< vector float >::One() );
    const vector float      flt_min( Const< vector float >::FltMin() );
    const vector float      abs_mask( Const< vector float >::AbsMask() );
    const vector float      abs_B( vec_and( B, abs_mask ) );
    const vector bool int   valid( vec_cmpgt( abs_B, flt_min ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        const vector float q0( vec_mul( A, y0 ) );
        return vec_sel( y0, q0, valid );
    }
    else
    {
        const vector float t0( vec_nmsub( y0, B, one ) );
        const vector float y1( vec_madd( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            const vector float q1( vec_mul( A, vec_sel( y0, y1, valid ) ) );
            return vec_sel( y0, q1, valid );
        }
        else
        {
            const vector float t1( vec_nmsub( y1, B, one ) );
            const vector float y2( vec_madd( y1, t1, y1 ) );
            const vector float q2( vec_mul( A, vec_sel( y0, y2, valid ) ) );
            return vec_sel( y0, q2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_RcpSqrt( const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_RcpSqrt( const vector float B )
{
    //const vector float zero = Const< vector float >::Zero();
    //vector float       rcp_sqrt_B;
    //vector bool int    valid;
    //PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    //return rcp_sqrt_B;
    
    const vector float      y0( vec_rsqrte( B ) );
    const vector float      flt_min( Const< vector float >::FltMin() );
    const vector float      flt_max( Const< vector float >::FltMax() );
    const vector float      zero( Const< vector float >::Zero() );
    const vector float      half( Const< vector float >::Half() );
    const vector float      one( Const< vector float >::One() );
    const vector bool int   B_GT_flt_min( vec_cmpgt( B, flt_min ) );
    const vector bool int   B_LT_flt_max( vec_cmpgt( flt_max, B ) );
    const vector bool int   valid( vec_and( B_GT_flt_min, B_LT_flt_max ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return vec_sel( zero, y0, valid );
    }
    else
    {
        const vector float t0( vec_mul( y0, y0 ) );
        const vector float t1( vec_mul( y0, half ) );
        const vector float t2( vec_nmsub( B, t0, one ) );
        const vector float y1( vec_madd( t2, t1, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return vec_sel( zero, y1, valid );
        }
        else
        {
            const vector float t0( vec_mul( y1, y1 ) );
            const vector float t1( vec_mul( y1, half ) );
            const vector float t2( vec_nmsub( B, t0, one ) );
            const vector float y2( vec_madd( t2, t1, y1 ) );
            return vec_sel( zero, y2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_RcpSqrt( const vector float B, const vector float error_result ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_RcpSqrt( const vector float B, const vector float error_result )
{
    //vector float    rcp_sqrt_B;
    //vector bool int valid;
    //PPU_VECTOR_RECIPROCAL_SQRT( B, errorResult, rcp_sqrt_B, valid );
    //return rcp_sqrt_B;
    
    const vector float      y0( vec_rsqrte( B ) );
    const vector float      flt_min( Const< vector float >::FltMin() );
    const vector float      flt_max( Const< vector float >::FltMax() );
    const vector float      half( Const< vector float >::Half() );
    const vector float      one( Const< vector float >::One() );
    const vector bool int   B_GT_flt_min( vec_cmpgt( B, flt_min ) );
    const vector bool int   B_LT_flt_max( vec_cmpgt( flt_max, B ) );
    const vector bool int   valid( vec_and( B_GT_flt_min, B_LT_flt_max ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return vec_sel( error_result, y0, valid );
    }
    else
    {
        const vector float t0( vec_mul( y0, y0 ) );
        const vector float t1( vec_mul( y0, half ) );
        const vector float t2( vec_nmsub( B, t0, one ) );
        const vector float y1( vec_madd( t2, t1, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return vec_sel( error_result, y1, valid );
        }
        else
        {
            const vector float t0( vec_mul( y1, y1 ) );
            const vector float t1( vec_mul( y1, half ) );
            const vector float t2( vec_nmsub( B, t0, one ) );
            const vector float y2( vec_madd( t2, t1, y1 ) );
            return vec_sel( error_result, y2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_Sqrt( const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_Sqrt( const vector float B )
{
    //const vector float zero = Const< vector float >::Zero();
    //vector float       rcp_sqrt_B;
    //vector bool int    valid;
    //PPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    //return vec_madd( B, rcp_sqrt_B, zero );
    
    return vec_mul( B, SIMD_RcpSqrt< T_PrecisionBits >( B ) );
}





template < int RotLeftWord > NOALIAS __forceinline const vector float vec_rotate_left_qw_words( const vector float a ) __attribute__(( const, always_inline ));
template < int RotLeftWord > NOALIAS __forceinline const vector float vec_rotate_left_qw_words( const vector float a )
{
    return vec_vsldoi( a, a, RotLeftWord << 2 );
}

template < int RotRightWord > NOALIAS __forceinline const vector float vec_rotate_right_qw_words( const vector float a ) __attribute__(( const, always_inline ));
template < int RotRightWord > NOALIAS __forceinline const vector float vec_rotate_right_qw_words( const vector float a )
{
    return vec_vsldoi( a, a, ((4-RotRightWord)&3) << 2 );
}


template < int mask, int rotate > inline const vector float vec_vrlimi( vector float vDst, vector float vSrc ) __attribute__(( pure, always_inline ));
template < int mask, int rotate > inline const vector float vec_vrlimi( vector float vDst, vector float vSrc )
{
    if ( rotate )
    {
        vSrc = vec_vsldoi( vSrc, vSrc, rotate << 2 );
    }
    return vec_sel( vDst, vSrc, Const< vector unsigned int >::Mask( mask ) );
}




inline const vector float vec_load_left_aligned( F32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector float vec_load_left_aligned( F32 const * const __restrict pSrc )
{
    return vec_lvlx( 0x0, pSrc );
}

inline const vector signed short vec_load_left_aligned( S16 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed short vec_load_left_aligned( S16 const * const __restrict pSrc )
{
    return vec_lvlx( 0x0, pSrc );
}

inline const vector unsigned int vec_load_left_aligned( U32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned int vec_load_left_aligned( U32 const * const __restrict pSrc )
{
    return vec_lvlx( 0x0, pSrc );
}

inline const vector signed int vec_load_left_aligned( S32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed int vec_load_left_aligned( S32 const * const __restrict pSrc )
{
    return vec_lvlx( 0x0, pSrc );
}

inline const vector unsigned char vec_load_left_aligned( U8 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned char vec_load_left_aligned( U8 const * const __restrict pSrc )
{
    return vec_lvlx( 0x0, pSrc );
}

inline const vector signed char vec_load_left_aligned( S8 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed char vec_load_left_aligned( S8 const * const __restrict pSrc )
{
    return vec_lvlx( 0x0, pSrc );
}



inline const vector float vec_load_aligned( F32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector float vec_load_aligned( F32 const * const __restrict pSrc )
{
    return vec_lvx( 0x00, pSrc );
}

inline const vector float vec_load_unaligned( F32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector float vec_load_unaligned( F32 const * const __restrict pSrc )
{
    return vec_or( vec_lvlx( 0x00, pSrc ),
                   vec_lvrx( 0x10, pSrc ) );
}

inline const vector signed int vec_load_unaligned( S32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed int vec_load_unaligned( S32 const * const __restrict pSrc )
{
    return vec_or( vec_lvlx( 0x00, pSrc ),
                   vec_lvrx( 0x10, pSrc ) );
}

inline const vector unsigned int vec_load_unaligned( U32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned int vec_load_unaligned( U32 const * const __restrict pSrc )
{
    return vec_or( vec_lvlx( 0x00, pSrc ),
                   vec_lvrx( 0x10, pSrc ) );
}

inline const vector signed short vec_load_unaligned( S16 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed short vec_load_unaligned( S16 const * const __restrict pSrc )
{
    return vec_or( vec_lvlx( 0x00, pSrc ),
                   vec_lvrx( 0x10, pSrc ) );
}

inline const vector unsigned short vec_load_unaligned( U16 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned short vec_load_unaligned( U16 const * const __restrict pSrc )
{
    return vec_or( vec_lvlx( 0x00, pSrc ),
                   vec_lvrx( 0x10, pSrc ) );
}

inline const vector signed char vec_load_unaligned( S8 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed char vec_load_unaligned( S8 const * const __restrict pSrc )
{
    return vec_or( vec_lvlx( 0x00, pSrc ),
                   vec_lvrx( 0x10, pSrc ) );
}

inline const vector unsigned char vec_load_unaligned( U8 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned char vec_load_unaligned( U8 const * const __restrict pSrc )
{
    return vec_or( vec_lvlx( 0x00, pSrc ),
                   vec_lvrx( 0x10, pSrc ) );
}


inline void vec_store_aligned_vec4( const vector float src, void* const __restrict pDst ) __attribute__(( always_inline ));
inline void vec_store_aligned_vec4( const vector float src, void* const __restrict pDst )
{
    vec_stvx( src, 0x00, (F32*)pDst );
}

inline void vec_store_unaligned_vec4( const vector float src, void* const __restrict pDst ) __attribute__(( always_inline ));
inline void vec_store_unaligned_vec4( const vector float src, void* const __restrict pDst )
{
    vec_stvlx( src, 0x00, (F32*)pDst );
    vec_stvrx( src, 0x10, (F32*)pDst );
}

inline void vec_store_unaligned_vec3( const vector float src, void* const __restrict pDst ) __attribute__(( always_inline ));
inline void vec_store_unaligned_vec3( const vector float src, void* const __restrict pDst )
{
    vec_ste( vec_splat( src, VX ), 0x0, (F32*)pDst );
    vec_ste( vec_splat( src, VY ), 0x4, (F32*)pDst );
    vec_ste( vec_splat( src, VZ ), 0x8, (F32*)pDst );
}

inline void vec_store_unaligned_vec2( const vector float src, void* const __restrict pDst ) __attribute__(( always_inline ));
inline void vec_store_unaligned_vec2( const vector float src, void* const __restrict pDst )
{
    vec_ste( vec_splat( src, VX ), 0x0, (F32*)pDst );
    vec_ste( vec_splat( src, VY ), 0x4, (F32*)pDst );
}

inline void vec_store_unaligned_vec1( const vector float src, void* const __restrict pDst ) __attribute__(( always_inline ));
inline void vec_store_unaligned_vec1( const vector float src, void* const __restrict pDst )
{
    vec_ste( vec_splat( src, VX ), 0x0, (F32*)pDst );
}


inline void vec_store_unaligned_vec4u16( const vector unsigned short src, void* const __restrict pDst ) __attribute__(( always_inline ));
inline void vec_store_unaligned_vec4u16( const vector unsigned short src, void* const __restrict pDst )
{
    vec_ste( vec_splat( src, 0 ), 0x0, (U16*)pDst );
    vec_ste( vec_splat( src, 1 ), 0x2, (U16*)pDst );
    vec_ste( vec_splat( src, 2 ), 0x4, (U16*)pDst );
    vec_ste( vec_splat( src, 3 ), 0x6, (U16*)pDst );
}



/// PPU Select : result.x = x==0 ? a.x : b.x
///              result.y = y==0 ? a.y : b.y
///              result.z = z==0 ? a.z : b.z
///              result.w = w==0 ? a.w : b.w
template < bool x, bool y, bool z, bool w > inline const vector float vec_select( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
template < bool x, bool y, bool z, bool w > inline const vector float vec_select( const vector float a, const vector float b )
{
    return vec_sel( a, b, Const< vector unsigned int >::Mask< x, y, z, w >() );
}

/// PPU Select : result.x = x==0 ? a.x : b.x
///              result.y = y==0 ? a.y : b.y
///              result.z = z==0 ? a.z : b.z
///              result.w = w==0 ? a.w : b.w
template < bool x, bool y, bool z, bool w > inline const vector bool int vec_select( const vector bool int a, const vector bool int b ) __attribute__(( pure, always_inline ));
template < bool x, bool y, bool z, bool w > inline const vector bool int vec_select( const vector bool int a, const vector bool int b )
{
    return vec_sel( a, b, Const< vector unsigned int >::Mask< x, y, z, w >() );
}

template < int rot_left, bool x, bool y, bool z, bool w > inline const vector float vec_rotate_left_insert( const vector float a, vector float b ) __attribute__(( pure, always_inline ));
template < int rot_left, bool x, bool y, bool z, bool w > inline const vector float vec_rotate_left_insert( const vector float a, vector float b )
{
    if ( rot_left )
    {
        b = vec_vsldoi( b, b, rot_left << 2 );
    }
    
    return vec_sel( a, b, Const< vector unsigned int >::Mask< x, y, z, w >() );
}

template < bool x, bool y, bool z, bool w > inline const vector float vec_f32_mask() __attribute__(( pure, always_inline ));
template < bool x, bool y, bool z, bool w > inline const vector float vec_f32_mask()
{
    return Const< vector float >::Mask< x, y, z, w >();
}

template < bool x, bool y, bool z, bool w > inline const vector unsigned int vec_u32_mask() __attribute__(( pure, always_inline ));
template < bool x, bool y, bool z, bool w > inline const vector unsigned int vec_u32_mask()
{
    return Const< vector unsigned int >::Mask< x, y, z, w >();
}

template < bool x, bool y, bool z, bool w > inline const vector float vec_mask( const vector float v ) __attribute__(( pure, always_inline ));
template < bool x, bool y, bool z, bool w > inline const vector float vec_mask( const vector float v )
{
    return vec_and( v, Const< vector float >::Mask< x, y, z, w >() );
}

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector float vec_permute( const vector float v ) __attribute__(( pure, always_inline ));
template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector float vec_permute( const vector float v )
{
    StaticAssert( IX < 4, IX_MUST_BE_LESS_THAN_4 );
    StaticAssert( IY < 4, IY_MUST_BE_LESS_THAN_4 );
    StaticAssert( IZ < 4, IZ_MUST_BE_LESS_THAN_4 );
    StaticAssert( IW < 4, IW_MUST_BE_LESS_THAN_4 );
    return vec_perm( v, v, PermuteWord<IX,IY,IZ,IW>::Pattern );
}

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector float vec_permute( const vector float v0, const vector float v1 ) __attribute__(( pure, always_inline ));
template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector float vec_permute( const vector float v0, const vector float v1 )
{
    StaticAssert( IX < 8, IX_MUST_BE_LESS_THAN_8 );
    StaticAssert( IY < 8, IY_MUST_BE_LESS_THAN_8 );
    StaticAssert( IZ < 8, IZ_MUST_BE_LESS_THAN_8 );
    StaticAssert( IW < 8, IW_MUST_BE_LESS_THAN_8 );
    return vec_perm( v0, v1, PermuteWord<IX,IY,IZ,IW>::Pattern );
}

template < unsigned char I0, unsigned char I1, unsigned char I2 > inline const vector float vec_permute( const vector float v0, const vector float v1, const vector float v2 ) __attribute__(( pure, always_inline ));
template < unsigned char I0, unsigned char I1, unsigned char I2 > inline const vector float vec_permute( const vector float v0, const vector float v1, const vector float v2 )
{
    StaticAssert( I0 < 4, I0_MUST_BE_LESS_THAN_4 );
    StaticAssert( I1 < 4, I1_MUST_BE_LESS_THAN_4 );
    StaticAssert( I2 < 4, I2_MUST_BE_LESS_THAN_4 );
    
    const vector float zero = Const< vector float >::Zero();
    const vector float x = vec_splat( v0, I0 );
    const vector float y = vec_splat( v1, I1 );
    const vector float z = vec_splat( v2, I2 );
    
    return vec_mergeh( vec_mergeh( x, z ), vec_mergeh( y, zero ) );
}



inline const vector bool int vec_not( const vector bool int a ) __attribute__(( const, always_inline ));
inline const vector bool int vec_not( const vector bool int a )
{
    return vec_nor( a, a );
}

inline const vector bool int vec_nand( const vector bool int a, const vector bool int b ) __attribute__(( const, always_inline ));
inline const vector bool int vec_nand( const vector bool int a, const vector bool int b )
{
    return vec_not( vec_and( a, b ) );
}

inline const vector bool int vec_eqv( const vector bool int a, const vector bool int b ) __attribute__(( const, always_inline ));
inline const vector bool int vec_eqv( const vector bool int a, const vector bool int b )
{
    return vec_not( vec_xor( a, b ) );
}

inline const vector bool int vec_orc( const vector bool int a, const vector bool int b ) __attribute__(( const, always_inline ));
inline const vector bool int vec_orc( const vector bool int a, const vector bool int b )
{
    return vec_or( a, vec_not( b ) );
}


inline const vector float vec_not( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float vec_not( const vector float a )
{
    return (vector float)vec_nor( (vector unsigned int)a, (vector unsigned int)a );
}

inline const vector float vec_nand( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vec_nand( const vector float a, const vector float b )
{
    return vec_not( (vector float)vec_and( (vector unsigned int)a, (vector unsigned int)b ) );
}

inline const vector float vec_eqv( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vec_eqv( const vector float a, const vector float b )
{
    return vec_not( (vector float)vec_xor( (vector unsigned int)a, (vector unsigned int)b ) );
}

inline const vector float vec_orc( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vec_orc( const vector float a, const vector float b )
{
    return (vector float)vec_or( (vector unsigned int)a, (vector unsigned int)vec_not( b ) );
}


inline const vector bool int vec_horizontal_or( const vector bool int a ) __attribute__(( const, always_inline ));
inline const vector bool int vec_horizontal_or( const vector bool int a )
{
    return vec_or( vec_or( a,
                           vec_sld( a, a, 0x4 ) ),
                   vec_or( vec_sld( a, a, 0x8 ),
                           vec_sld( a, a, 0xC ) ) );
}

inline const vector bool int vec_horizontal_or( const vector bool short a ) __attribute__(( const, always_inline ));
inline const vector bool int vec_horizontal_or( const vector bool short a )
{
    const vector bool short b( vec_or( a, vec_sld( a, a, 0x8 ) ) );
    const vector bool short c( vec_or( b, vec_sld( b, b, 0x4 ) ) );
    const vector bool short d( vec_or( c, vec_sld( c, c, 0x2 ) ) );
    return (vector bool int)d;
}

inline const vector bool int vec_horizontal_or( const vector bool char a ) __attribute__(( const, always_inline ));
inline const vector bool int vec_horizontal_or( const vector bool char a )
{
    const vector bool char b( vec_or( a, vec_sld( a, a, 0x8 ) ) );
    const vector bool char c( vec_or( b, vec_sld( b, b, 0x4 ) ) );
    const vector bool char d( vec_or( c, vec_sld( c, c, 0x2 ) ) );
    const vector bool char e( vec_or( d, vec_sld( d, d, 0x1 ) ) );
    return (vector bool int)e;
}


inline const vector bool int vec_horizontal_and( const vector bool int a ) __attribute__(( const, always_inline ));
inline const vector bool int vec_horizontal_and( const vector bool int a )
{
    return vec_and( vec_and( a,
                             vec_sld( a, a, 0x4 ) ),
                    vec_and( vec_sld( a, a, 0x8 ),
                             vec_sld( a, a, 0xC ) ) );
}

inline const vector bool int vec_horizontal_and( const vector bool short a ) __attribute__(( const, always_inline ));
inline const vector bool int vec_horizontal_and( const vector bool short a )
{
    const vector bool short b( vec_and( a, vec_sld( a, a, 0x8 ) ) );
    const vector bool short c( vec_and( b, vec_sld( b, b, 0x4 ) ) );
    const vector bool short d( vec_and( c, vec_sld( c, c, 0x2 ) ) );
    return (vector bool int)d;
}

inline const vector bool int vec_horizontal_and( const vector bool char a ) __attribute__(( const, always_inline ));
inline const vector bool int vec_horizontal_and( const vector bool char a )
{
    const vector bool char b( vec_and( a, vec_sld( a, a, 0x8 ) ) );
    const vector bool char c( vec_and( b, vec_sld( b, b, 0x4 ) ) );
    const vector bool char d( vec_and( c, vec_sld( c, c, 0x2 ) ) );
    const vector bool char e( vec_and( d, vec_sld( d, d, 0x1 ) ) );
    return (vector bool int)e;
}


//inline const vector float vec_abs( const vector float a ) __attribute__(( const, always_inline ));
//inline const vector float vec_abs( const vector float a )
//{
//    //const vector float abs_mask = (vector float)(vector unsigned int)0x7FFFFFFF;
//    const vector float abs_mask = (vector float)Const< vector unsigned int >::AbsMask();
//    return vec_and( a, abs_mask );
//}

inline const vector float vec_negate( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float vec_negate( const vector float a )
{
    //const vector float sign_mask = (vector float)(vector unsigned int)0x80000000;
    const vector float sign_mask = Const< vector float >::SignMask();
    return vec_xor( a, sign_mask );
}

inline const vector float vec_sum2( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float vec_sum2( const vector float a )
{
    return vec_add( vec_splat( a, VX ),
                    vec_splat( a, VY ) );
}

inline const vector float vec_sum3( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float vec_sum3( const vector float a )
{
    return vec_add( vec_splat( a, VX ),
           vec_add( vec_splat( a, VY ),
                    vec_splat( a, VZ ) ) );
}

inline const vector float vec_sum4( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float vec_sum4( const vector float a )
{
    return vec_add( vec_add( vec_splat( a, VX ),
                             vec_splat( a, VY ) ),
                    vec_add( vec_splat( a, VZ ),
                             vec_splat( a, VW ) ) );
}

inline const vector float vec_length_squared2( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float vec_length_squared2( const vector float a )
{
    const vector float a2 = vec_mul( a, a );
    return vec_add( vec_splat( a2, VX ),
                    vec_splat( a2, VY ) );
}

inline const vector float vec_length_squared3( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float vec_length_squared3( const vector float a )
{
    const vector float a2 = vec_mul( a, a );
    return vec_add( vec_splat( a2, VX ),
           vec_add( vec_splat( a2, VY ),
                    vec_splat( a2, VZ ) ) );
}

inline const vector float vec_length_squared4( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float vec_length_squared4( const vector float a )
{
    const vector float a2 = vec_mul( a, a );
    return vec_add( vec_add( vec_splat( a2, VX ),
                             vec_splat( a2, VY ) ),
                    vec_add( vec_splat( a2, VZ ),
                             vec_splat( a2, VW ) ) );
}

inline const vector float vec_dot_product2( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vec_dot_product2( const vector float a, const vector float b )
{
    const vector float ab = vec_mul( a, b );
    return vec_add( vec_splat( ab, VX ),
                    vec_splat( ab, VY ) );
}

inline const vector float vec_dot_product3( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vec_dot_product3( const vector float a, const vector float b )
{
    const vector float ab = vec_mul( a, b );
    return vec_add( vec_splat( ab, VX ),
           vec_add( vec_splat( ab, VY ),
                    vec_splat( ab, VZ ) ) );
}

inline const vector float vec_dot_product4( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vec_dot_product4( const vector float a, const vector float b )
{
    const vector float ab = vec_mul( a, b );
    return vec_add( vec_add( vec_splat( ab, VX ),
                             vec_splat( ab, VY ) ),
                    vec_add( vec_splat( ab, VZ ),
                             vec_splat( ab, VW ) ) );
}

inline const vector float vec_cross_product2( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float vec_cross_product2( const vector float a, const vector float b )
{
    const vector float ax = vec_splat( a, VX );
    const vector float ay = vec_splat( a, VY );
    const vector float bx = vec_splat( b, VX );
    const vector float by = vec_splat( b, VY );
    return vec_nmsub( ay, bx, vec_mul( ax, by ) );
}

inline const vector float vec_cross_product3( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
inline const vector float vec_cross_product3( const vector float a, const vector float b )
{
    static const unsigned __ = V0W;
    const vector unsigned char permute_yzx = vec_permute_pattern_word<V0Y,V0Z,V0X,__>();
    const vector unsigned char permute_zxy = vec_permute_pattern_word<V0Z,V0X,V0Y,__>();
    
    const vector float a_yzx = vec_perm( a, a, permute_yzx );
    const vector float a_zxy = vec_perm( a, a, permute_zxy );
    const vector float b_yzx = vec_perm( b, b, permute_yzx );
    const vector float b_zxy = vec_perm( b, b, permute_zxy );
    
    return vec_nmsub( a_zxy, b_yzx, vec_mul( a_yzx, b_zxy ) );
}


inline void vec_transpose_matrix33(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ ) __attribute__(( /* pure, */ always_inline ));
inline void vec_transpose_matrix33(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ )
{
    const vector float inW( Const< vector float >::Zero() );
    const vector float tmp0( vec_merge_xy( inX, inZ ) );
    const vector float tmp1( vec_merge_xy( inY, inW ) );
    const vector float tmp2( vec_merge_zw( inX, inZ ) );
    const vector float tmp3( vec_merge_zw( inY, inW ) );
    *pOutX = vec_merge_xy( tmp0, tmp1 );
    *pOutY = vec_merge_zw( tmp0, tmp1 );
    *pOutZ = vec_merge_xy( tmp2, tmp3 );
}

inline void vec_transpose_matrix43(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    const vector float             inW,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ ) __attribute__(( /* pure, */ always_inline ));
inline void vec_transpose_matrix43(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    const vector float             inW,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ )
{
    const vector float tmp0( vec_merge_xy( inX, inZ ) );
    const vector float tmp1( vec_merge_xy( inY, inW ) );
    const vector float tmp2( vec_merge_zw( inX, inZ ) );
    const vector float tmp3( vec_merge_zw( inY, inW ) );
    *pOutX = vec_merge_xy( tmp0, tmp1 );
    *pOutY = vec_merge_zw( tmp0, tmp1 );
    *pOutZ = vec_merge_xy( tmp2, tmp3 );
}

inline void vec_transpose_matrix44(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    const vector float             inW,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ,
    vector float* const __restrict pOutW ) __attribute__(( /* pure, */ always_inline ));
inline void vec_transpose_matrix44(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    const vector float             inW,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ,
    vector float* const __restrict pOutW )
{
    const vector float tmp0( vec_merge_xy( inX, inZ ) );
    const vector float tmp1( vec_merge_xy( inY, inW ) );
    const vector float tmp2( vec_merge_zw( inX, inZ ) );
    const vector float tmp3( vec_merge_zw( inY, inW ) );
    *pOutX = vec_merge_xy( tmp0, tmp1 );
    *pOutY = vec_merge_zw( tmp0, tmp1 );
    *pOutZ = vec_merge_xy( tmp2, tmp3 );
    *pOutW = vec_merge_zw( tmp2, tmp3 );
}

inline const vector float vec_transpose_col_x( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( const, always_inline ));
inline const vector float vec_transpose_col_x( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return vec_merge_xy( vec_merge_xy( rowX, rowZ ),
                         vec_merge_xy( rowY, rowW ) );
}

inline const vector float vec_transpose_col_y( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( const, always_inline ));
inline const vector float vec_transpose_col_y( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return vec_merge_zw( vec_merge_xy( rowX, rowZ ),
                         vec_merge_xy( rowY, rowW ) );
}

inline const vector float vec_transpose_col_z( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( const, always_inline ));
inline const vector float vec_transpose_col_z( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return vec_merge_xy( vec_merge_zw( rowX, rowZ ),
                         vec_merge_zw( rowY, rowW ) );
}

inline const vector float vec_transpose_col_w( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( const, always_inline ));
inline const vector float vec_transpose_col_w( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return vec_merge_zw( vec_merge_zw( rowX, rowZ ),
                         vec_merge_zw( rowY, rowW ) );
}



/// vector3 <-- vector3 [Implicit W=0] * matrix33
inline const vector float vec_multiply_vector3_matrix33( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_vector3_matrix33( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    return vec_madd( vec_splat( v, VX ), rowX,
           vec_madd( vec_splat( v, VY ), rowY,
           vec_mul ( vec_splat( v, VZ ), rowZ ) ) );
}

/// vector3 <-- vector3 [Implicit W=0] * matrix43 [ Implicit column W=(0,0,0,1) ]
inline const vector float vec_multiply_vector3_matrix43( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_vector3_matrix43( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    return vec_madd( vec_splat( v, VX ), rowX,
           vec_madd( vec_splat( v, VY ), rowY,
           vec_mul ( vec_splat( v, VZ ), rowZ ) ) );
}

/// vector4 <-- vector3 [Implicit W=0] * matrix44
inline const vector float vec_multiply_vector3_matrix44( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_vector3_matrix44( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    return vec_madd( vec_splat( v, VX ), rowX,
           vec_madd( vec_splat( v, VY ), rowY,
           vec_mul ( vec_splat( v, VZ ), rowZ ) ) );
}



/// vector3 <-- matrix33 * vector3 [Implicit W=0]
inline const vector float vec_multiply_matrix33_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_matrix33_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float v )
{
    vector float colX, colY, colZ;
    vec_transpose_matrix33( rowX, rowY, rowZ, &colX, &colY, &colZ );
    return vec_madd( vec_splat( v, VX ), colX,
           vec_madd( vec_splat( v, VY ), colY,
           vec_mul ( vec_splat( v, VZ ), colZ ) ) );
}

/// vector4 <-- matrix43 [ Implicit column W=(0,0,0,1) ] * vector3 [Implicit W=0]
inline const vector float vec_multiply_matrix43_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW, const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_matrix43_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW, const vector float v )
{
    vector float colX, colY, colZ;
    vec_transpose_matrix43( rowX, rowY, rowZ, rowW, &colX, &colY, &colZ );
    return vec_madd( vec_splat( v, VX ), colX,
           vec_madd( vec_splat( v, VY ), colY,
           vec_mul ( vec_splat( v, VZ ), colZ ) ) );
}

/// vector4 <-- matrix44 * vector3 [Implicit W=0]
inline const vector float vec_multiply_matrix44_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW, const vector float v ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_matrix44_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW, const vector float v )
{
    vector float colX, colY, colZ, colW;
    vec_transpose_matrix44( rowX, rowY, rowZ, rowW, &colX, &colY, &colZ, &colW );
    return vec_madd( vec_splat( v, VX ), colX,
           vec_madd( vec_splat( v, VY ), colY,
           vec_mul ( vec_splat( v, VZ ), colZ ) ) );
}



/// vector4 <-- vector4 * matrix33 [ Implicit row W = column W = (0,0,0,1) ]
inline const vector float vec_multiply_vector4_matrix33( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( pure, always_inline ));
inline const vector float vec_multiply_vector4_matrix33( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    const vector float result = vec_madd( vec_splat( v, VX ), rowX,
                                vec_madd( vec_splat( v, VY ), rowY,
                                vec_mul ( vec_splat( v, VZ ), rowZ ) ) );
    return vec_select<0,0,0,1>( result, v );
}

/// vector4 <-- vector4 * matrix43 [ Implicit column W=(0,0,0,1) ]
inline const vector float vec_multiply_vector4_matrix43( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float vec_multiply_vector4_matrix43( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    const vector float result = vec_add( vec_madd( vec_splat( v, VX ), rowX,
                                         vec_mul ( vec_splat( v, VY ), rowY ) ),
                                         vec_madd( vec_splat( v, VZ ), rowZ,
                                         vec_mul ( vec_splat( v, VW ), rowW ) ) );
    return vec_select<0,0,0,1>( result, v );
}

/// vector4 <-- vector4 * matrix44
inline const vector float vec_multiply_vector4_matrix44( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_vector4_matrix44( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return vec_add( vec_madd( vec_splat( v, VX ), rowX,
                    vec_mul ( vec_splat( v, VY ), rowY ) ),
                    vec_madd( vec_splat( v, VZ ), rowZ,
                    vec_mul ( vec_splat( v, VW ), rowW ) ) );
}



/// point3 <-- point3 [Implicit W=1] * matrix33
inline const vector float vec_multiply_point3_matrix33( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_point3_matrix33( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    return vec_madd( vec_splat( p, VX ), rowX,
           vec_madd( vec_splat( p, VY ), rowY,
           vec_mul ( vec_splat( p, VZ ), rowZ ) ) );
}

/// point3 <-- point3 [Implicit W=1] * matrix43 [ Implicit column W=(0,0,0,1) ]
inline const vector float vec_multiply_point3_matrix43( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_point3_matrix43( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return vec_add( vec_madd( vec_splat( p, VX ), rowX,
                    vec_mul ( vec_splat( p, VY ), rowY ) ),
                    vec_madd( vec_splat( p, VZ ), rowZ,
                                                  rowW ) );
}

/// vector4 <-- point3 [Implicit W=1] * matrix44
inline const vector float vec_multiply_point3_matrix44( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( const, always_inline ));
inline const vector float vec_multiply_point3_matrix44( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return vec_add( vec_madd( vec_splat( p, VX ), rowX,
                    vec_mul ( vec_splat( p, VY ), rowY ) ),
                    vec_madd( vec_splat( p, VZ ), rowZ,
                                                  rowW ) );
}




}; // namespace Maths

#endif // #if defined( _PPU_ )

#endif // #ifndef MATHS_HVEC_SIMD_PPU_INL




