#ifndef MATHS_HVEC_SIMD_SPU_INL
#define MATHS_HVEC_SIMD_SPU_INL

#ifdef _MSC_VER
#pragma once
#endif

#if defined( _SPU_ )

namespace Maths
{

//
// PS3 SPU SIMD Scalar & Vector Maths Macros & Inline Functions
//

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define SPU_VECTOR_RECIPROCAL_UNSAFE( B, out_rcp_B )                                    \
{                                                                                       \
    const vector float        y0( spu_re( B ) );                                        \
    const vector float        abs_mask( Const< vector float >::AbsMask() );             \
    const vector float        abs_B( spu_and( B, abs_mask ) );                          \
    const vector float        flt_min( Const< vector float >::FltMin() );               \
    const vector float        one( Const< vector float >::One() );                      \
    const vector unsigned int valid( spu_cmpgt( abs_B, flt_min ) );                     \
    const vector float        t( spu_nmsub( y0, B, one ) );                             \
    const vector float        y1( spu_madd( y0, t, y0 ) );                              \
    /* If B is not in the valid range, return the reciprocal estimate */                \
    out_rcp_B = spu_sel( y0, y1, valid );                                               \
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define SPU_VECTOR_RECIPROCAL( B, invalid_result, out_rcp_B, out_valid )                \
{                                                                                       \
    const vector float        y0( spu_re( B ) );                                        \
    const vector float        abs_mask( Const< vector float >::AbsMask() );             \
    const vector float        abs_B( spu_and( B, abs_mask ) );                          \
    const vector float        flt_min( Const< vector float >::FltMin() );               \
    const vector float        flt_max( Const< vector float >::FltMax() );               \
    const vector float        one( Const< vector float >::One() );                      \
    const vector unsigned int abs_B_GT_flt_min( spu_cmpgt( abs_B, flt_min ) );          \
    const vector unsigned int abs_B_LT_flt_max( spu_cmpgt( flt_max, abs_B ) );          \
    const vector unsigned int valid_B( spu_and( abs_B_GT_flt_min, abs_B_LT_flt_max ) ); \
    const vector float        t( spu_nmsub( y0, B, one ) );                             \
    const vector float        y1( spu_madd( y0, t, y0 ) );                              \
    /* TODO: Validate y0, y1? */                                                        \
    out_rcp_B = spu_sel( invalid_result, y1, valid_B );                                 \
    out_valid = valid_B;                                                                \
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define SPU_VECTOR_RECIPROCAL_SQRT( B, invalid_result, out_rcp_sqrt_B, out_valid )      \
{                                                                                       \
    const vector float        y0( spu_rsqrte( B ) );                                    \
    const vector float        flt_min( Const< vector float >::FltMin() );               \
    const vector float        flt_max( Const< vector float >::FltMax() );               \
    const vector float        half( Const< vector float >::Half() );                    \
    const vector float        one( Const< vector float >::One() );                      \
    const vector unsigned int B_GT_flt_min( spu_cmpgt( B, flt_min ) );                  \
    const vector unsigned int B_LT_flt_max( spu_cmpgt( flt_max, B ) );                  \
    const vector unsigned int valid_B( spu_and( B_GT_flt_min, B_LT_flt_max ) );         \
    const vector float        t0( spu_mul( y0, y0 ) );                                  \
    const vector float        t1( spu_mul( y0, half ) );                                \
    const vector float        t2( spu_nmsub( B, t0, one ) );                            \
    const vector float        y1( spu_madd( t2, t1, y0 ) );                             \
    /* TODO: Validate y0, y1? */                                                        \
    out_rcp_sqrt_B = spu_sel( invalid_result, y1, valid_B );                            \
    out_valid = valid_B;                                                                \
}

// altivec_pem.pdf: 4.2.2.1 Floating-Point Division & Square-Root
#define SPU_VECTOR_RECIPROCAL_SQRT3( B0, B1, B2, out_rcp_sqrt_B0, out_rcp_sqrt_B1, out_rcp_sqrt_B2 ) \
{                                                                                       \
    const vector float        zero( Const< vector float >::Zero() );                    \
    const vector float        B0_y0( spu_rsqrte( B0 ) );                                \
    const vector float        B1_y0( spu_rsqrte( B1 ) );                                \
    const vector float        B2_y0( spu_rsqrte( B2 ) );                                \
    const vector float        flt_min( Const< vector float >::FltMin() );               \
    const vector float        flt_max( Const< vector float >::FltMax() );               \
    const vector float        half( Const< vector float >::Half() );                    \
    const vector float        one( Const< vector float >::One() );                      \
    const vector unsigned int B0_GT_flt_min( spu_cmpgt( B0, flt_min ) );                \
    const vector unsigned int B1_GT_flt_min( spu_cmpgt( B1, flt_min ) );                \
    const vector unsigned int B2_GT_flt_min( spu_cmpgt( B2, flt_min ) );                \
    const vector unsigned int B0_LT_flt_max( spu_cmpgt( flt_max, B0 ) );                \
    const vector unsigned int B1_LT_flt_max( spu_cmpgt( flt_max, B1 ) );                \
    const vector unsigned int B2_LT_flt_max( spu_cmpgt( flt_max, B2 ) );                \
    const vector unsigned int B0_valid( spu_and( B0_GT_flt_min, B0_LT_flt_max ) );      \
    const vector unsigned int B1_valid( spu_and( B1_GT_flt_min, B1_LT_flt_max ) );      \
    const vector unsigned int B2_valid( spu_and( B2_GT_flt_min, B2_LT_flt_max ) );      \
    const vector float        B0_t0( spu_mul( B0_y0, B0_y0 ) );                         \
    const vector float        B1_t0( spu_mul( B1_y0, B1_y0 ) );                         \
    const vector float        B2_t0( spu_mul( B2_y0, B2_y0 ) );                         \
    const vector float        B0_t1( spu_mul( B0_y0, half ) );                          \
    const vector float        B1_t1( spu_mul( B1_y0, half ) );                          \
    const vector float        B2_t1( spu_mul( B2_y0, half ) );                          \
    const vector float        B0_t2( spu_nmsub( B0, B0_t0, one ) );                     \
    const vector float        B1_t2( spu_nmsub( B1, B1_t0, one ) );                     \
    const vector float        B2_t2( spu_nmsub( B2, B2_t0, one ) );                     \
    const vector float        B0_y1( spu_madd( B0_t2, B0_t1, B0_y0 ) );                 \
    const vector float        B1_y1( spu_madd( B1_t2, B1_t1, B1_y0 ) );                 \
    const vector float        B2_y1( spu_madd( B2_t2, B2_t1, B2_y0 ) );                 \
    out_rcp_sqrt_B0 = spu_sel( zero, B0_y1, B0_valid );                                 \
    out_rcp_sqrt_B1 = spu_sel( zero, B1_y1, B1_valid );                                 \
    out_rcp_sqrt_B2 = spu_sel( zero, B2_y1, B2_valid );                                 \
}

template < int T_PrecisionBits > const vector float SIMD_Rcp( const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_Rcp( const vector float B )
{
    //const vector float  zero = Const< vector float >::Zero();
    //vector float        rcp_B;
    //vector unsigned int valid;
    //SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    //return rcp_B;
    
    const vector float        y0( spu_re( B ) );
    const vector float        zero = Const< vector float >::Zero();
    const vector float        one( Const< vector float >::One() );
    const vector float        flt_min( Const< vector float >::FltMin() );
    const vector float        abs_mask( Const< vector float >::AbsMask() );
    const vector float        abs_B( spu_and( B, abs_mask ) );
    const vector unsigned int valid( spu_cmpgt( abs_B, flt_min ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return spu_sel( zero, y0, valid );
    }
    else
    {
        const vector float t0( spu_nmsub( y0, B, one ) );
        const vector float y1( spu_madd( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return spu_sel( zero, y1, valid );
        }
        else
        {
            const vector float t1( spu_nmsub( y1, B, one ) );
            const vector float y2( spu_madd( y1, t1, y1 ) );
            return spu_sel( zero, y2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_Div( const vector float A, const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_Div( const vector float A, const vector float B )
{
    //const vector float  zero = Const< vector float >::Zero();
    //vector float        rcp_B;
    //vector unsigned int valid;
    //SPU_VECTOR_RECIPROCAL( B, zero, rcp_B, valid );
    //return spu_mul( A, rcp_B );
    
    const vector float        y0( spu_re( B ) );
    const vector float        zero = Const< vector float >::Zero();
    const vector float        one( Const< vector float >::One() );
    const vector float        flt_min( Const< vector float >::FltMin() );
    const vector float        abs_mask( Const< vector float >::AbsMask() );
    const vector float        abs_B( spu_and( B, abs_mask ) );
    const vector unsigned int valid( spu_cmpgt( abs_B, flt_min ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        const vector float q0( spu_mul( A, y0 ) );
        return spu_sel( zero, q0, valid );
    }
    else
    {
        const vector float t0( spu_nmsub( y0, B, one ) );
        const vector float y1( spu_madd( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            const vector float q1( spu_mul( A, y1 ) );
            return spu_sel( zero, q1, valid );
        }
        else
        {
            const vector float t1( spu_nmsub( y1, B, one ) );
            const vector float y2( spu_madd( y1, t1, y1 ) );
            const vector float q2( spu_mul( A, y2 ) );
            return spu_sel( zero, q2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_UnsafeDivide( const vector float A, const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_UnsafeDivide( const vector float A, const vector float B )
{
    //vector float rcp_B;
    //SPU_VECTOR_RECIPROCAL_UNSAFE( B, rcp_B );
    //return spu_mul( A, rcp_B );
    
    const vector float        y0( spu_re( B ) );
    const vector float        one( Const< vector float >::One() );
    const vector float        flt_min( Const< vector float >::FltMin() );
    const vector float        abs_mask( Const< vector float >::AbsMask() );
    const vector float        abs_B( spu_and( B, abs_mask ) );
    const vector unsigned int valid( spu_cmpgt( abs_B, flt_min ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        const vector float q0( spu_mul( A, y0 ) );
        return spu_sel( y0, q0, valid );
    }
    else
    {
        const vector float t0( spu_nmsub( y0, B, one ) );
        const vector float y1( spu_madd( y0, t0, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            const vector float q1( spu_mul( A, spu_sel( y0, y1, valid ) ) );
            return spu_sel( y0, q1, valid );
        }
        else
        {
            const vector float t1( spu_nmsub( y1, B, one ) );
            const vector float y2( spu_madd( y1, t1, y1 ) );
            const vector float q2( spu_mul( A, spu_sel( y0, y2, valid ) ) );
            return spu_sel( y0, q2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_RcpSqrt( const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_RcpSqrt( const vector float B )
{
    //const vector float  zero = Const< vector float >::Zero();
    //vector float        rcp_sqrt_B;
    //vector unsigned int valid;
    //SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    //return rcp_sqrt_B;
    
    const vector float        y0( spu_rsqrte( B ) );
    const vector float        flt_min( Const< vector float >::FltMin() );
    const vector float        flt_max( Const< vector float >::FltMax() );
    const vector float        zero( Const< vector float >::Zero() );
    const vector float        half( Const< vector float >::Half() );
    const vector float        one( Const< vector float >::One() );
    const vector unsigned int B_GT_flt_min( spu_cmpgt( B, flt_min ) );
    const vector unsigned int B_LT_flt_max( spu_cmpgt( flt_max, B ) );
    const vector unsigned int valid( spu_and( B_GT_flt_min, B_LT_flt_max ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return spu_sel( zero, y0, valid );
    }
    else
    {
        const vector float t0( spu_mul( y0, y0 ) );
        const vector float t1( spu_mul( y0, half ) );
        const vector float t2( spu_nmsub( B, t0, one ) );
        const vector float y1( spu_madd( t2, t1, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return spu_sel( zero, y1, valid );
        }
        else
        {
            const vector float t0( spu_mul( y1, y1 ) );
            const vector float t1( spu_mul( y1, half ) );
            const vector float t2( spu_nmsub( B, t0, one ) );
            const vector float y2( spu_madd( t2, t1, y1 ) );
            return spu_sel( zero, y2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_RcpSqrt( const vector float B, const vector float error_result ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_RcpSqrt( const vector float B, const vector float error_result )
{
    //vector float        rcp_sqrt_B;
    //vector unsigned int valid;
    //SPU_VECTOR_RECIPROCAL_SQRT( B, error_result, rcp_sqrt_B, valid );
    //return rcp_sqrt_B;
    
    const vector float        y0( spu_rsqrte( B ) );
    const vector float        flt_min( Const< vector float >::FltMin() );
    const vector float        flt_max( Const< vector float >::FltMax() );
    const vector float        half( Const< vector float >::Half() );
    const vector float        one( Const< vector float >::One() );
    const vector unsigned int B_GT_flt_min( spu_cmpgt( B, flt_min ) );
    const vector unsigned int B_LT_flt_max( spu_cmpgt( flt_max, B ) );
    const vector unsigned int valid( spu_and( B_GT_flt_min, B_LT_flt_max ) );
    
    if ( T_PrecisionBits <= 11 )
    {
        return spu_sel( error_result, y0, valid );
    }
    else
    {
        const vector float t0( spu_mul( y0, y0 ) );
        const vector float t1( spu_mul( y0, half ) );
        const vector float t2( spu_nmsub( B, t0, one ) );
        const vector float y1( spu_madd( t2, t1, y0 ) );
        
        if ( T_PrecisionBits <= 22 )
        {
            return spu_sel( error_result, y1, valid );
        }
        else
        {
            const vector float t0( spu_mul( y1, y1 ) );
            const vector float t1( spu_mul( y1, half ) );
            const vector float t2( spu_nmsub( B, t0, one ) );
            const vector float y2( spu_madd( t2, t1, y1 ) );
            return spu_sel( error_result, y2, valid );
        }
    }
}

template < int T_PrecisionBits > const vector float SIMD_Sqrt( const vector float B ) __attribute__(( pure, always_inline ));
template < int T_PrecisionBits > const vector float SIMD_Sqrt( const vector float B )
{
    //const vector float  zero = Const< vector float >::Zero();
    //vector float        rcp_sqrt_B;
    //vector unsigned int valid;
    //SPU_VECTOR_RECIPROCAL_SQRT( B, zero, rcp_sqrt_B, valid );
    //return spu_madd( B, rcp_sqrt_B, zero );
    
    return spu_mul( B, SIMD_RcpSqrt< T_PrecisionBits >( B ) );
}



template < unsigned short I > const vector unsigned char spu_mask_byte() __attribute__(( const, always_inline ));
template < unsigned short I > const vector unsigned char spu_mask_byte()
{
    return spu_maskb( I );
}

template < unsigned short I > const vector unsigned short spu_mask_half() __attribute__(( const, always_inline ));
template < unsigned short I > const vector unsigned short spu_mask_half()
{
    //return spu_maskh( I );
    const unsigned short I0( ( I & 0x80 ) >> 7 );
    const unsigned short I1( ( I & 0x40 ) >> 6 );
    const unsigned short I2( ( I & 0x20 ) >> 5 );
    const unsigned short I3( ( I & 0x10 ) >> 4 );
    const unsigned short I4( ( I & 0x08 ) >> 3 );
    const unsigned short I5( ( I & 0x04 ) >> 2 );
    const unsigned short I6( ( I & 0x02 ) >> 1 );
    const unsigned short I7( ( I & 0x01 ) >> 0 );
    return (vector unsigned short)spu_maskb( ( I0 << 15 ) | ( I0 << 14 ) |
                                             ( I1 << 13 ) | ( I1 << 12 ) |
                                             ( I2 << 11 ) | ( I2 << 10 ) |
                                             ( I3 <<  9 ) | ( I3 <<  8 ) |
                                             ( I4 <<  7 ) | ( I4 <<  6 ) |
                                             ( I5 <<  5 ) | ( I5 <<  4 ) |
                                             ( I6 <<  3 ) | ( I6 <<  2 ) |
                                             ( I7 <<  1 ) | ( I7 <<  0 ) );
}

template < unsigned short I > const vector unsigned int spu_mask_word() __attribute__(( const, always_inline ));
template < unsigned short I > const vector unsigned int spu_mask_word()
{
    //return spu_maskw( I );
    const unsigned short I0( ( I & 0x8 ) >> 3 );
    const unsigned short I1( ( I & 0x4 ) >> 2 );
    const unsigned short I2( ( I & 0x2 ) >> 1 );
    const unsigned short I3( ( I & 0x1 ) >> 0 );
    return (vector unsigned int)spu_maskb( ( I0 << 15 ) | ( I0 << 14 ) | ( I0 << 13 ) | ( I0 << 12 ) |
                                           ( I1 << 11 ) | ( I1 << 10 ) | ( I1 <<  9 ) | ( I1 <<  8 ) |
                                           ( I2 <<  7 ) | ( I2 <<  6 ) | ( I2 <<  5 ) | ( I2 <<  4 ) |
                                           ( I3 <<  3 ) | ( I3 <<  2 ) | ( I3 <<  1 ) | ( I3 <<  0 ) );
}



template < int bytes > inline const vector float spu_shift_left_qw_bytes( const vector float src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector float spu_shift_left_qw_bytes( const vector float src )
{
    return spu_slqwbyte( src, bytes );
}

template < int bytes > inline const vector float spu_shift_right_qw_bytes( const vector float src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector float spu_shift_right_qw_bytes( const vector float src )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

template < int bytes > inline const vector float spu_rotate_left_qw_bytes( const vector float src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector float spu_rotate_left_qw_bytes( const vector float src )
{
    return spu_rlqwbyte( src, bytes );
}

template < int bytes > inline const vector float spu_rotate_right_qw_bytes( const vector float src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector float spu_rotate_right_qw_bytes( const vector float src )
{
    return spu_rlqwbyte( src, 16 - bytes );
}

inline const vector float spu_shift_left_qw_bytes( const vector float src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector float spu_shift_left_qw_bytes( const vector float src, const int bytes )
{
    return spu_slqwbyte( src, bytes );
}

inline const vector float spu_shift_right_qw_bytes( const vector float src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector float spu_shift_right_qw_bytes( const vector float src, const int bytes )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

inline const vector float spu_rotate_left_qw_bytes( const vector float src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector float spu_rotate_left_qw_bytes( const vector float src, const int bytes )
{
    return spu_rlqwbyte( src, bytes );
}

inline const vector float spu_rotate_right_qw_bytes( const vector float src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector float spu_rotate_right_qw_bytes( const vector float src, const int bytes )
{
    return spu_rlqwbyte( src, -bytes );
}



template < int bytes > inline const vector signed int spu_shift_left_qw_bytes( const vector signed int src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed int spu_shift_left_qw_bytes( const vector signed int src )
{
    return spu_slqwbyte( src, bytes );
}

template < int bytes > inline const vector signed int spu_shift_right_qw_bytes( const vector signed int src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed int spu_shift_right_qw_bytes( const vector signed int src )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

template < int bytes > inline const vector signed int spu_rotate_left_qw_bytes( const vector signed int src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed int spu_rotate_left_qw_bytes( const vector signed int src )
{
    return spu_rlqwbyte( src, bytes );
}

template < int bytes > inline const vector signed int spu_rotate_right_qw_bytes( const vector signed int src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed int spu_rotate_right_qw_bytes( const vector signed int src )
{
    return spu_rlqwbyte( src, 16 - bytes );
}

inline const vector signed int spu_shift_left_qw_bytes( const vector signed int src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed int spu_shift_left_qw_bytes( const vector signed int src, const int bytes )
{
    return spu_slqwbyte( src, bytes );
}

inline const vector signed int spu_shift_right_qw_bytes( const vector signed int src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed int spu_shift_right_qw_bytes( const vector signed int src, const int bytes )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

inline const vector signed int spu_rotate_left_qw_bytes( const vector signed int src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed int spu_rotate_left_qw_bytes( const vector signed int src, const int bytes )
{
    return spu_rlqwbyte( src, bytes );
}

inline const vector signed int spu_rotate_right_qw_bytes( const vector signed int src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed int spu_rotate_right_qw_bytes( const vector signed int src, const int bytes )
{
    return spu_rlqwbyte( src, -bytes );
}



template < int bytes > inline const vector unsigned int spu_shift_left_qw_bytes( const vector unsigned int src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned int spu_shift_left_qw_bytes( const vector unsigned int src )
{
    return spu_slqwbyte( src, bytes );
}

template < int bytes > inline const vector unsigned int spu_shift_right_qw_bytes( const vector unsigned int src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned int spu_shift_right_qw_bytes( const vector unsigned int src )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

template < int bytes > inline const vector unsigned int spu_rotate_left_qw_bytes( const vector unsigned int src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned int spu_rotate_left_qw_bytes( const vector unsigned int src )
{
    return spu_rlqwbyte( src, bytes );
}

template < int bytes > inline const vector unsigned int spu_rotate_right_qw_bytes( const vector unsigned int src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned int spu_rotate_right_qw_bytes( const vector unsigned int src )
{
    return spu_rlqwbyte( src, 16 - bytes );
}

inline const vector unsigned int spu_shift_left_qw_bytes( const vector unsigned int src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_shift_left_qw_bytes( const vector unsigned int src, const int bytes )
{
    return spu_slqwbyte( src, bytes );
}

inline const vector unsigned int spu_shift_right_qw_bytes( const vector unsigned int src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_shift_right_qw_bytes( const vector unsigned int src, const int bytes )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

inline const vector unsigned int spu_rotate_left_qw_bytes( const vector unsigned int src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_rotate_left_qw_bytes( const vector unsigned int src, const int bytes )
{
    return spu_rlqwbyte( src, bytes );
}

inline const vector unsigned int spu_rotate_right_qw_bytes( const vector unsigned int src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_rotate_right_qw_bytes( const vector unsigned int src, const int bytes )
{
    return spu_rlqwbyte( src, -bytes );
}



template < int bytes > inline const vector signed short spu_shift_left_qw_bytes( const vector signed short src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed short spu_shift_left_qw_bytes( const vector signed short src )
{
    return spu_slqwbyte( src, bytes );
}

template < int bytes > inline const vector signed short spu_shift_right_qw_bytes( const vector signed short src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed short spu_shift_right_qw_bytes( const vector signed short src )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

template < int bytes > inline const vector signed short spu_rotate_left_qw_bytes( const vector signed short src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed short spu_rotate_left_qw_bytes( const vector signed short src )
{
    return spu_rlqwbyte( src, bytes );
}

template < int bytes > inline const vector signed short spu_rotate_right_qw_bytes( const vector signed short src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed short spu_rotate_right_qw_bytes( const vector signed short src )
{
    return spu_rlqwbyte( src, 16 - bytes );
}

inline const vector signed short spu_shift_left_qw_bytes( const vector signed short src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed short spu_shift_left_qw_bytes( const vector signed short src, const int bytes )
{
    return spu_slqwbyte( src, bytes );
}

inline const vector signed short spu_shift_right_qw_bytes( const vector signed short src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed short spu_shift_right_qw_bytes( const vector signed short src, const int bytes )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

inline const vector signed short spu_rotate_left_qw_bytes( const vector signed short src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed short spu_rotate_left_qw_bytes( const vector signed short src, const int bytes )
{
    return spu_rlqwbyte( src, bytes );
}

inline const vector signed short spu_rotate_right_qw_bytes( const vector signed short src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed short spu_rotate_right_qw_bytes( const vector signed short src, const int bytes )
{
    return spu_rlqwbyte( src, -bytes );
}



template < int bytes > inline const vector unsigned short spu_shift_left_qw_bytes( const vector unsigned short src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned short spu_shift_left_qw_bytes( const vector unsigned short src )
{
    return spu_slqwbyte( src, bytes );
}

template < int bytes > inline const vector unsigned short spu_shift_right_qw_bytes( const vector unsigned short src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned short spu_shift_right_qw_bytes( const vector unsigned short src )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

template < int bytes > inline const vector unsigned short spu_rotate_left_qw_bytes( const vector unsigned short src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned short spu_rotate_left_qw_bytes( const vector unsigned short src )
{
    return spu_rlqwbyte( src, bytes );
}

template < int bytes > inline const vector unsigned short spu_rotate_right_qw_bytes( const vector unsigned short src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned short spu_rotate_right_qw_bytes( const vector unsigned short src )
{
    return spu_rlqwbyte( src, 16 - bytes );
}

inline const vector unsigned short spu_shift_left_qw_bytes( const vector unsigned short src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_shift_left_qw_bytes( const vector unsigned short src, const int bytes )
{
    return spu_slqwbyte( src, bytes );
}

inline const vector unsigned short spu_shift_right_qw_bytes( const vector unsigned short src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_shift_right_qw_bytes( const vector unsigned short src, const int bytes )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

inline const vector unsigned short spu_rotate_left_qw_bytes( const vector unsigned short src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_rotate_left_qw_bytes( const vector unsigned short src, const int bytes )
{
    return spu_rlqwbyte( src, bytes );
}

inline const vector unsigned short spu_rotate_right_qw_bytes( const vector unsigned short src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_rotate_right_qw_bytes( const vector unsigned short src, const int bytes )
{
    return spu_rlqwbyte( src, -bytes );
}



template < int bytes > inline const vector signed char spu_shift_left_qw_bytes( const vector signed char src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed char spu_shift_left_qw_bytes( const vector signed char src )
{
    return spu_slqwbyte( src, bytes );
}

template < int bytes > inline const vector signed char spu_shift_right_qw_bytes( const vector signed char src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed char spu_shift_right_qw_bytes( const vector signed char src )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

template < int bytes > inline const vector signed char spu_rotate_left_qw_bytes( const vector signed char src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed char spu_rotate_left_qw_bytes( const vector signed char src )
{
    return spu_rlqwbyte( src, bytes );
}

template < int bytes > inline const vector signed char spu_rotate_right_qw_bytes( const vector signed char src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector signed char spu_rotate_right_qw_bytes( const vector signed char src )
{
    return spu_rlqwbyte( src, 16 - bytes );
}

inline const vector signed char spu_shift_left_qw_bytes( const vector signed char src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed char spu_shift_left_qw_bytes( const vector signed char src, const int bytes )
{
    return spu_slqwbyte( src, bytes );
}

inline const vector signed char spu_shift_right_qw_bytes( const vector signed char src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed char spu_shift_right_qw_bytes( const vector signed char src, const int bytes )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

inline const vector signed char spu_rotate_left_qw_bytes( const vector signed char src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed char spu_rotate_left_qw_bytes( const vector signed char src, const int bytes )
{
    return spu_rlqwbyte( src, bytes );
}

inline const vector signed char spu_rotate_right_qw_bytes( const vector signed char src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector signed char spu_rotate_right_qw_bytes( const vector signed char src, const int bytes )
{
    return spu_rlqwbyte( src, -bytes );
}



template < int bytes > inline const vector unsigned char spu_shift_left_qw_bytes( const vector unsigned char src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned char spu_shift_left_qw_bytes( const vector unsigned char src )
{
    return spu_slqwbyte( src, bytes );
}

template < int bytes > inline const vector unsigned char spu_shift_right_qw_bytes( const vector unsigned char src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned char spu_shift_right_qw_bytes( const vector unsigned char src )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

template < int bytes > inline const vector unsigned char spu_rotate_left_qw_bytes( const vector unsigned char src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned char spu_rotate_left_qw_bytes( const vector unsigned char src )
{
    return spu_rlqwbyte( src, bytes );
}

template < int bytes > inline const vector unsigned char spu_rotate_right_qw_bytes( const vector unsigned char src ) __attribute__(( const, always_inline ));
template < int bytes > inline const vector unsigned char spu_rotate_right_qw_bytes( const vector unsigned char src )
{
    return spu_rlqwbyte( src, 16 - bytes );
}

inline const vector unsigned char spu_shift_left_qw_bytes( const vector unsigned char src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_shift_left_qw_bytes( const vector unsigned char src, const int bytes )
{
    return spu_slqwbyte( src, bytes );
}

inline const vector unsigned char spu_shift_right_qw_bytes( const vector unsigned char src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_shift_right_qw_bytes( const vector unsigned char src, const int bytes )
{
    return spu_rlmaskqwbyte( src, -bytes );
}

inline const vector unsigned char spu_rotate_left_qw_bytes( const vector unsigned char src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_rotate_left_qw_bytes( const vector unsigned char src, const int bytes )
{
    return spu_rlqwbyte( src, bytes );
}

inline const vector unsigned char spu_rotate_right_qw_bytes( const vector unsigned char src, const int bytes ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_rotate_right_qw_bytes( const vector unsigned char src, const int bytes )
{
    return spu_rlqwbyte( src, -bytes );
}


/// SPU shift right logical
template < signed char t_Shift > inline const vector signed short spu_srl( const vector signed short src ) __attribute__(( const, always_inline ));
template < signed char t_Shift > inline const vector signed short spu_srl( const vector signed short src )
{
    return spu_rlmask( src, -t_Shift );
}

/// SPU shift right logical
template < signed char t_Shift > inline const vector unsigned short spu_srl( const vector unsigned short src ) __attribute__(( const, always_inline ));
template < signed char t_Shift > inline const vector unsigned short spu_srl( const vector unsigned short src )
{
    return spu_rlmask( src, -t_Shift );
}

/// SPU shift right logical
template < signed char t_Shift > inline const vector signed int spu_srl( const vector signed int src ) __attribute__(( const, always_inline ));
template < signed char t_Shift > inline const vector signed int spu_srl( const vector signed int src )
{
    return spu_rlmask( src, -t_Shift );
}

/// SPU shift right logical
template < signed char t_Shift > inline const vector unsigned int spu_srl( const vector unsigned int src ) __attribute__(( const, always_inline ));
template < signed char t_Shift > inline const vector unsigned int spu_srl( const vector unsigned int src )
{
    return spu_rlmask( src, -t_Shift );
}


/// SPU shift right arithmetic
template < signed char t_Shift > inline const vector signed short spu_sra( const vector signed short src ) __attribute__(( const, always_inline ));
template < signed char t_Shift > inline const vector signed short spu_sra( const vector signed short src )
{
    return spu_rlmaska( src, -t_Shift );
}

/// SPU shift right arithmetic
template < signed char t_Shift > inline const vector unsigned short spu_sra( const vector unsigned short src ) __attribute__(( const, always_inline ));
template < signed char t_Shift > inline const vector unsigned short spu_sra( const vector unsigned short src )
{
    return spu_rlmaska( src, -t_Shift );
}

/// SPU shift right arithmetic
template < signed char t_Shift > inline const vector signed int spu_sra( const vector signed int src ) __attribute__(( const, always_inline ));
template < signed char t_Shift > inline const vector signed int spu_sra( const vector signed int src )
{
    return spu_rlmaska( src, -t_Shift );
}

/// SPU shift right arithmetic
template < signed char t_Shift > inline const vector unsigned int spu_sra( const vector unsigned int src ) __attribute__(( const, always_inline ));
template < signed char t_Shift > inline const vector unsigned int spu_sra( const vector unsigned int src )
{
    return spu_rlmaska( src, -t_Shift );
}


inline const vector float spu_load_aligned( F32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector float spu_load_aligned( F32 const * const __restrict pSrc )
{
    vector float const * const __restrict pVecSrc = (vector float*)pSrc;
    return pVecSrc[0];
}

inline const vector float spu_load_unaligned( F32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector float spu_load_unaligned( F32 const * const __restrict pSrc )
{
    vector float const * const __restrict pVecSrc = (vector float*)pSrc;
    const vector float qw0 = pVecSrc[0];
    const vector float qw1 = pVecSrc[1];
    const int shift = (int)pSrc & 0xF;
    return spu_or( spu_shift_left_qw_bytes( qw0, shift ),
                   spu_shift_right_qw_bytes( qw1, 16 - shift ) );
}

inline const vector signed int spu_load_unaligned( S32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed int spu_load_unaligned( S32 const * const __restrict pSrc )
{
    vector signed int const * const __restrict pVecSrc = (vector signed int*)pSrc;
    const vector signed int qw0 = pVecSrc[0];
    const vector signed int qw1 = pVecSrc[1];
    const int shift = (int)pSrc & 0xF;
    return spu_or( spu_shift_left_qw_bytes( qw0, shift ),
                   spu_shift_right_qw_bytes( qw1, 16 - shift ) );
}

inline const vector unsigned int spu_load_unaligned( U32 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned int spu_load_unaligned( U32 const * const __restrict pSrc )
{
    vector unsigned int const * const __restrict pVecSrc = (vector unsigned int*)pSrc;
    const vector unsigned int qw0 = pVecSrc[0];
    const vector unsigned int qw1 = pVecSrc[1];
    const int shift = (int)pSrc & 0xF;
    return spu_or( spu_shift_left_qw_bytes( qw0, shift ),
                   spu_shift_right_qw_bytes( qw1, 16 - shift ) );
}

inline const vector signed short spu_load_unaligned( S16 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed short spu_load_unaligned( S16 const * const __restrict pSrc )
{
    vector signed short const * const __restrict pVecSrc = (vector signed short*)pSrc;
    const vector signed short qw0 = pVecSrc[0];
    const vector signed short qw1 = pVecSrc[1];
    const int shift = (int)pSrc & 0xF;
    return spu_or( spu_shift_left_qw_bytes( qw0, shift ),
                   spu_shift_right_qw_bytes( qw1, 16 - shift ) );
}

inline const vector unsigned short spu_load_unaligned( U16 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned short spu_load_unaligned( U16 const * const __restrict pSrc )
{
    vector unsigned short const * const __restrict pVecSrc = (vector unsigned short*)pSrc;
    const vector unsigned short qw0 = pVecSrc[0];
    const vector unsigned short qw1 = pVecSrc[1];
    const int shift = (int)pSrc & 0xF;
    return spu_or( spu_shift_left_qw_bytes( qw0, shift ),
                   spu_shift_right_qw_bytes( qw1, 16 - shift ) );
}

inline const vector signed char spu_load_unaligned( S8 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector signed char spu_load_unaligned( S8 const * const __restrict pSrc )
{
    vector signed char const * const __restrict pVecSrc = (vector signed char*)pSrc;
    const vector signed char qw0 = pVecSrc[0];
    const vector signed char qw1 = pVecSrc[1];
    const int shift = (int)pSrc & 0xF;
    return spu_or( spu_shift_left_qw_bytes( qw0, shift ),
                   spu_shift_right_qw_bytes( qw1, 16 - shift ) );
}

inline const vector unsigned char spu_load_unaligned( U8 const * const __restrict pSrc ) __attribute__(( pure, always_inline ));
inline const vector unsigned char spu_load_unaligned( U8 const * const __restrict pSrc )
{
    vector unsigned char const * const __restrict pVecSrc = (vector unsigned char*)pSrc;
    const vector unsigned char qw0 = pVecSrc[0];
    const vector unsigned char qw1 = pVecSrc[1];
    const int shift = (int)pSrc & 0xF;
    return spu_or( spu_shift_left_qw_bytes( qw0, shift ),
                   spu_shift_right_qw_bytes( qw1, 16 - shift ) );
}


inline void spu_store_unaligned_vector_float( vector float src, vector float * const __restrict pDst ) __attribute__(( always_inline ));
inline void spu_store_unaligned_vector_float( vector float src, vector float * const __restrict pDst )
{
    const vector float qw0 = pDst[0];
    const vector float qw1 = pDst[1];
    const int shift = (int)pDst & 0xF;
    src = spu_rotate_right_qw_bytes( src, shift );
    
    #if 0
    // This version supports all masks
    //const vector unsigned int mask = spu_maskw( 0xF ); // 0b1111
    const vector unsigned int mask = spu_mask_word< 0xF >(); // 0b1111
    pDst[0] = spu_sel( qw0, src, spu_shift_right_qw_bytes( mask, shift ) );
    pDst[1] = spu_sel( qw1, src, spu_shift_left_qw_bytes( mask, 16 - shift ) );
    #else
    // This version is optimised & only works for a full QW mask [ spu_maskw(0xF) ]
    //const vector unsigned int mask = spu_shift_right_qw_bytes( spu_maskw( 0xF ), shift );
    const vector unsigned int mask = spu_shift_right_qw_bytes( spu_mask_word< 0xF >(), shift );
    pDst[0] = spu_sel( qw0, src, mask );
    pDst[1] = spu_sel( src, qw1, mask );
    #endif
}

inline void spu_store_aligned_vec4( vector float src, F32 * const __restrict pDst ) __attribute__(( always_inline ));
inline void spu_store_aligned_vec4( vector float src, F32 * const __restrict pDst )
{
    vector float * const __restrict pVecDst = (vector float*)pDst;
    pVecDst[0] = src;
}

inline void spu_store_unaligned_vec4( vector float src, F32 * const __restrict pDst ) __attribute__(( always_inline ));
inline void spu_store_unaligned_vec4( vector float src, F32 * const __restrict pDst )
{
    //pDst[0] = spu_extract( src, VX );
    //pDst[1] = spu_extract( src, VY );
    //pDst[2] = spu_extract( src, VZ );
    //pDst[3] = spu_extract( src, VW );
    
    vector float * const __restrict pVecDst = (vector float*)pDst;
    const vector float qw0 = pVecDst[0];
    const vector float qw1 = pVecDst[1];
    const int shift = (int)pDst & 0xF;
    src = spu_rotate_right_qw_bytes( src, shift );
    
    #if 0
    // This version supports all masks
    //const vector unsigned int mask = spu_maskw( 0xF ); // 0b1111
    const vector unsigned int mask = spu_mask_word< 0xF >(); // 0b1111
    pVecDst[0] = spu_sel( qw0, src, spu_shift_right_qw_bytes( mask, shift ) );
    pVecDst[1] = spu_sel( qw1, src, spu_shift_left_qw_bytes( mask, 16 - shift ) );
    #else
    // This version is optimised & only works for a full QW mask [ spu_maskw(0xF) ]
    //const vector unsigned int mask = spu_shift_right_qw_bytes( spu_maskw( 0xF ), shift );
    const vector unsigned int mask = spu_shift_right_qw_bytes( spu_mask_word< 0xF >(), shift );
    pVecDst[0] = spu_sel( qw0, src, mask );
    pVecDst[1] = spu_sel( src, qw1, mask );
    #endif
}

inline void spu_store_unaligned_vec3( vector float src, F32 * const __restrict pDst ) __attribute__(( always_inline ));
inline void spu_store_unaligned_vec3( vector float src, F32 * const __restrict pDst )
{
    //pDst[0] = spu_extract( src, VX );
    //pDst[1] = spu_extract( src, VY );
    //pDst[2] = spu_extract( src, VZ );
    
    vector float * const __restrict pVecDst = (vector float*)pDst;
    const vector float qw0 = pVecDst[0];
    const vector float qw1 = pVecDst[1];
    const int shift = (int)pDst & 0xF;
    //const vector unsigned int mask = spu_maskw( 0xE ); // 0b1110
    const vector unsigned int mask = spu_mask_word< 0xE >(); // 0b1110
    src = spu_rotate_right_qw_bytes( src, shift );
    pVecDst[0] = spu_sel( qw0, src, spu_shift_right_qw_bytes( mask, shift ) );
    pVecDst[1] = spu_sel( qw1, src, spu_shift_left_qw_bytes( mask, 16 - shift ) );
}

inline void spu_store_unaligned_vec2( vector float src, F32 * const __restrict pDst ) __attribute__(( always_inline ));
inline void spu_store_unaligned_vec2( vector float src, F32 * const __restrict pDst )
{
    //pDst[0] = spu_extract( src, VX );
    //pDst[1] = spu_extract( src, VY );
    
    vector float * const __restrict pVecDst = (vector float*)pDst;
    const vector float qw0 = pVecDst[0];
    const vector float qw1 = pVecDst[1];
    const int shift = (int)pDst & 0xF;
    //const vector unsigned int mask = spu_maskw( 0xC ); // 0b1100
    const vector unsigned int mask = spu_mask_word< 0xC >(); // 0b1100
    src = spu_rotate_right_qw_bytes( src, shift );
    pVecDst[0] = spu_sel( qw0, src, spu_shift_right_qw_bytes( mask, shift ) );
    pVecDst[1] = spu_sel( qw1, src, spu_shift_left_qw_bytes( mask, 16 - shift ) );
}

inline void spu_store_unaligned_vec1( vector float src, F32 * const __restrict pDst ) __attribute__(( always_inline ));
inline void spu_store_unaligned_vec1( vector float src, F32 * const __restrict pDst )
{
    //pDst[0] = spu_extract( src, VX );
    
    vector float * const __restrict pVecDst = (vector float*)pDst;
    const vector float qw0 = pVecDst[0];
    const vector float qw1 = pVecDst[1];
    const int shift = (int)pDst & 0xF;
    //const vector unsigned int mask = spu_maskw( 0x8 ); // 0b1000
    const vector unsigned int mask = spu_mask_word< 0x8 >(); // 0b1000
    src = spu_rotate_right_qw_bytes( src, shift );
    pVecDst[0] = spu_sel( qw0, src, spu_shift_right_qw_bytes( mask, shift ) );
    pVecDst[1] = spu_sel( qw1, src, spu_shift_left_qw_bytes( mask, 16 - shift ) );
}


inline void spu_store_unaligned_vec4u16( vector unsigned short src, U16 * const __restrict pDst ) __attribute__(( always_inline ));
inline void spu_store_unaligned_vec4u16( vector unsigned short src, U16 * const __restrict pDst )
{
    vector unsigned short * const __restrict pVecDst = (vector unsigned short*)pDst;
    const vector unsigned short qw0 = pVecDst[0];
    const vector unsigned short qw1 = pVecDst[1];
    const int shift = (int)pDst & 0xF;
    const vector unsigned short mask = spu_mask_half< 0xF0 >(); // 0b11110000
    src = spu_rotate_right_qw_bytes( src, shift );
    pVecDst[0] = spu_sel( qw0, src, spu_shift_right_qw_bytes( mask, shift ) );
    pVecDst[1] = spu_sel( qw1, src, spu_shift_left_qw_bytes( mask, 16 - shift ) );
}





template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7,
           unsigned char I8, unsigned char I9, unsigned char IA, unsigned char IB,
           unsigned char IC, unsigned char ID, unsigned char IE, unsigned char IF >
class ShuffleByte
{
public:
    static const vector unsigned char Pattern __attribute__(( weak ));
};

template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7,
           unsigned char I8, unsigned char I9, unsigned char IA, unsigned char IB,
           unsigned char IC, unsigned char ID, unsigned char IE, unsigned char IF >
const vector unsigned char ShuffleByte<I0,I1,I2,I3,I4,I5,I6,I7,I8,I9,IA,IB,IC,ID,IE,IF>::Pattern =
{
    I0, I1, I2, I3,
    I4, I5, I6, I7,
    I8, I9, IA, IB,
    IC, ID, IE, IF
};


template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7 >
class ShuffleHalf
{
public:
    static const vector unsigned char Pattern __attribute__(( weak ));
};

template < unsigned char I0, unsigned char I1, unsigned char I2, unsigned char I3,
           unsigned char I4, unsigned char I5, unsigned char I6, unsigned char I7 >
const vector unsigned char ShuffleHalf<I0,I1,I2,I3,I4,I5,I6,I7>::Pattern =
{
    (I0<<1)+0, (I0<<1)+1, (I1<<1)+0, (I1<<1)+1,
    (I2<<1)+0, (I2<<1)+1, (I3<<1)+0, (I3<<1)+1,
    (I4<<1)+0, (I4<<1)+1, (I5<<1)+0, (I5<<1)+1,
    (I6<<1)+0, (I6<<1)+1, (I7<<1)+0, (I7<<1)+1
};


template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW >
class ShuffleWord
{
public:
    static const vector unsigned char Pattern __attribute__(( weak ));
};

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW >
const vector unsigned char ShuffleWord<IX,IY,IZ,IW>::Pattern =
{
    (IX<<2)+0, (IX<<2)+1, (IX<<2)+2, (IX<<2)+3,
    (IY<<2)+0, (IY<<2)+1, (IY<<2)+2, (IY<<2)+3,
    (IZ<<2)+0, (IZ<<2)+1, (IZ<<2)+2, (IZ<<2)+3,
    (IW<<2)+0, (IW<<2)+1, (IW<<2)+2, (IW<<2)+3
};


template < unsigned char I > inline const vector unsigned char spu_shuffle_pattern_word() __attribute__(( pure, always_inline ));
template < unsigned char I > inline const vector unsigned char spu_shuffle_pattern_word()
{
    StaticAssert( I < 8, I_MUST_BE_LESS_THAN_8 );
    return ShuffleWord<I,I,I,I>::Pattern;
}

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector unsigned char spu_shuffle_pattern_word() __attribute__(( pure, always_inline ));
template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector unsigned char spu_shuffle_pattern_word()
{
    StaticAssert( IX < 8, IX_MUST_BE_LESS_THAN_8 );
    StaticAssert( IY < 8, IY_MUST_BE_LESS_THAN_8 );
    StaticAssert( IZ < 8, IZ_MUST_BE_LESS_THAN_8 );
    StaticAssert( IW < 8, IW_MUST_BE_LESS_THAN_8 );
    return ShuffleWord<IX,IY,IZ,IW>::Pattern;
}


/// SPU select : result.x = x==0 ? a.x : b.x
///              result.y = y==0 ? a.y : b.y
///              result.z = z==0 ? a.z : b.z
///              result.w = w==0 ? a.w : b.w
template < unsigned char x, unsigned char y, unsigned char z, unsigned char w > inline const vector float spu_select( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
template < unsigned char x, unsigned char y, unsigned char z, unsigned char w > inline const vector float spu_select( const vector float a, const vector float b )
{
    StaticAssert( x < 2, X_MUST_BE_LESS_THAN_2 );
    StaticAssert( y < 2, Y_MUST_BE_LESS_THAN_2 );
    StaticAssert( z < 2, Z_MUST_BE_LESS_THAN_2 );
    StaticAssert( w < 2, W_MUST_BE_LESS_THAN_2 );
    return spu_shuffle( a, b, spu_shuffle_pattern_word< (x*4)+0, (y*4)+1, (z*4)+2, (w*4)+3 >() );
}

template < unsigned char x, unsigned char y, unsigned char z, unsigned char w > inline const vector unsigned int spu_select( const vector unsigned int a, const vector unsigned int b ) __attribute__(( pure, always_inline ));
template < unsigned char x, unsigned char y, unsigned char z, unsigned char w > inline const vector unsigned int spu_select( const vector unsigned int a, const vector unsigned int b )
{
    StaticAssert( x < 2, X_MUST_BE_LESS_THAN_2 );
    StaticAssert( y < 2, Y_MUST_BE_LESS_THAN_2 );
    StaticAssert( z < 2, Z_MUST_BE_LESS_THAN_2 );
    StaticAssert( w < 2, W_MUST_BE_LESS_THAN_2 );
    return spu_shuffle( a, b, spu_shuffle_pattern_word< (x*4)+0, (y*4)+1, (z*4)+2, (w*4)+3 >() );
}

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector float spu_permute( const vector float a ) __attribute__(( pure, always_inline ));
template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector float spu_permute( const vector float a )
{
    StaticAssert( IX < 4, IX_MUST_BE_LESS_THAN_4 );
    StaticAssert( IY < 4, IY_MUST_BE_LESS_THAN_4 );
    StaticAssert( IZ < 4, IZ_MUST_BE_LESS_THAN_4 );
    StaticAssert( IW < 4, IW_MUST_BE_LESS_THAN_4 );
    return spu_shuffle( a, a, spu_shuffle_pattern_word<IX,IY,IZ,IW>() );
}

template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector float spu_permute( const vector float a, const vector float b ) __attribute(( pure, always_inline ));
template < unsigned char IX, unsigned char IY, unsigned char IZ, unsigned char IW > inline const vector float spu_permute( const vector float a, const vector float b )
{
    StaticAssert( IX < 8, IX_MUST_BE_LESS_THAN_8 );
    StaticAssert( IY < 8, IY_MUST_BE_LESS_THAN_8 );
    StaticAssert( IZ < 8, IZ_MUST_BE_LESS_THAN_8 );
    StaticAssert( IW < 8, IW_MUST_BE_LESS_THAN_8 );
    return spu_shuffle( a, b, spu_shuffle_pattern_word<IX,IY,IZ,IW>() );
}

template < unsigned char IA, unsigned char IB, unsigned char IC > inline const vector float spu_permute( const vector float va, const vector float vb, const vector float vc ) __attribute(( pure, always_inline ));
template < unsigned char IA, unsigned char IB, unsigned char IC > inline const vector float spu_permute( const vector float va, const vector float vb, const vector float vc )
{
    StaticAssert( IA < 4, IA_MUST_BE_LESS_THAN_4 );
    StaticAssert( IB < 4, IB_MUST_BE_LESS_THAN_4 );
    StaticAssert( IC < 4, IC_MUST_BE_LESS_THAN_4 );
	return spu_insert( spu_extract( va, IA ),
	       spu_insert( spu_extract( vb, IB ),
	       spu_insert( spu_extract( vc, IC ),
	       spu_promote( 0.0f, VW ), VZ ), VY ), VX );
}

template < unsigned char I > inline const vector signed int spu_splat( const vector signed int a ) __attribute(( pure, always_inline ));
template < unsigned char I > inline const vector signed int spu_splat( const vector signed int a )
{
    StaticAssert( I < 4, I_MUST_BE_LESS_THAN_4 );
    return spu_shuffle( a, a, spu_shuffle_pattern_word<I>() );
}

template < unsigned char I > inline const vector unsigned int spu_splat( const vector unsigned int a ) __attribute(( pure, always_inline ));
template < unsigned char I > inline const vector unsigned int spu_splat( const vector unsigned int a )
{
    StaticAssert( I < 4, I_MUST_BE_LESS_THAN_4 );
    return spu_shuffle( a, a, spu_shuffle_pattern_word<I>() );
}

template < unsigned char I > inline const vector float spu_splat( const vector float a ) __attribute(( pure, always_inline ));
template < unsigned char I > inline const vector float spu_splat( const vector float a )
{
    StaticAssert( I < 4, I_MUST_BE_LESS_THAN_4 );
    return spu_shuffle( a, a, spu_shuffle_pattern_word<I>() );
}


inline const vector float spu_merge_xy( const vector float a, const vector float b ) __attribute(( pure, always_inline ));
inline const vector float spu_merge_xy( const vector float a, const vector float b )
{
    return spu_shuffle( a, b, spu_shuffle_pattern_word<0,4,1,5>() );
}

inline const vector float spu_merge_zw( const vector float a, const vector float b ) __attribute(( pure, always_inline ));
inline const vector float spu_merge_zw( const vector float a, const vector float b )
{
    return spu_shuffle( a, b, spu_shuffle_pattern_word<2,6,3,7>() );
}

/// Make vector3 from scalar x, y & z : the X component of each scalar must contain the value. (splatted scalars not required)
inline const vector float spu_make_vector3( const vector float x, const vector float y, const vector float z ) __attribute(( pure, always_inline ));
inline const vector float spu_make_vector3( const vector float x, const vector float y, const vector float z )
{
    return spu_merge_xy( spu_merge_xy( x, z ), y ); // NOTE: w component is UNDEFINED, not zero!
}

/// Make vector4 from scalar x, y, z & w : the X component of each scalar must contain the value. (splatted scalars not required)
inline const vector float spu_make_vector4( const vector float x, const vector float y, const vector float z, const vector float w ) __attribute(( pure, always_inline ));
inline const vector float spu_make_vector4( const vector float x, const vector float y, const vector float z, const vector float w )
{
    return spu_merge_xy( spu_merge_xy( x, z ),
                         spu_merge_xy( y, w ) );
}


inline const vector signed char spu_merge_hi( const vector signed char a, const vector signed char b ) __attribute(( pure, always_inline ));
inline const vector signed char spu_merge_hi( const vector signed char a, const vector signed char b )
{
    return spu_shuffle( a, b, ShuffleByte< 0, 16,
                                           1, 17,
                                           2, 18,
                                           3, 19,
                                           4, 20,
                                           5, 21,
                                           6, 22,
                                           7, 23 >::Pattern );
}

inline const vector signed char spu_merge_lo( const vector signed char a, const vector signed char b ) __attribute(( pure, always_inline ));
inline const vector signed char spu_merge_lo( const vector signed char a, const vector signed char b )
{
    return spu_shuffle( a, b, ShuffleByte< 8,  24,
                                           9,  25,
                                           10, 26,
                                           11, 27,
                                           12, 28,
                                           13, 29,
                                           14, 30,
                                           15, 31 >::Pattern );
}

inline const vector unsigned char spu_merge_hi( const vector unsigned char a, const vector unsigned char b ) __attribute(( pure, always_inline ));
inline const vector unsigned char spu_merge_hi( const vector unsigned char a, const vector unsigned char b )
{
    return spu_shuffle( a, b, ShuffleByte< 0, 16,
                                           1, 17,
                                           2, 18,
                                           3, 19,
                                           4, 20,
                                           5, 21,
                                           6, 22,
                                           7, 23 >::Pattern );
}

inline const vector unsigned char spu_merge_lo( const vector unsigned char a, const vector unsigned char b ) __attribute(( pure, always_inline ));
inline const vector unsigned char spu_merge_lo( const vector unsigned char a, const vector unsigned char b )
{
    return spu_shuffle( a, b, ShuffleByte< 8,  24,
                                           9,  25,
                                           10, 26,
                                           11, 27,
                                           12, 28,
                                           13, 29,
                                           14, 30,
                                           15, 31 >::Pattern );
}


inline const vector signed short spu_merge_hi( const vector signed short a, const vector signed short b ) __attribute(( pure, always_inline ));
inline const vector signed short spu_merge_hi( const vector signed short a, const vector signed short b )
{
    return spu_shuffle( a, b, ShuffleHalf< 0, 8,
                                           1, 9,
                                           2, 10,
                                           3, 11 >::Pattern );
}

inline const vector signed short spu_merge_lo( const vector signed short a, const vector signed short b ) __attribute(( pure, always_inline ));
inline const vector signed short spu_merge_lo( const vector signed short a, const vector signed short b )
{
    return spu_shuffle( a, b, ShuffleHalf< 4, 12,
                                           5, 13,
                                           6, 14,
                                           7, 15 >::Pattern );
}

inline const vector unsigned short spu_merge_hi( const vector unsigned short a, const vector unsigned short b ) __attribute(( pure, always_inline ));
inline const vector unsigned short spu_merge_hi( const vector unsigned short a, const vector unsigned short b )
{
    return spu_shuffle( a, b, ShuffleHalf< 0, 8,
                                           1, 9,
                                           2, 10,
                                           3, 11 >::Pattern );
}

inline const vector unsigned short spu_merge_lo( const vector unsigned short a, const vector unsigned short b ) __attribute(( pure, always_inline ));
inline const vector unsigned short spu_merge_lo( const vector unsigned short a, const vector unsigned short b )
{
    return spu_shuffle( a, b, ShuffleHalf< 4, 12,
                                           5, 13,
                                           6, 14,
                                           7, 15 >::Pattern );
}


inline const vector signed int spu_merge_hi( const vector signed int a, const vector signed int b ) __attribute(( pure, always_inline ));
inline const vector signed int spu_merge_hi( const vector signed int a, const vector signed int b )
{
    return spu_shuffle( a, b, ShuffleWord< 0, 4,
                                           1, 5 >::Pattern );
}

inline const vector signed int spu_merge_lo( const vector signed int a, const vector signed int b ) __attribute(( pure, always_inline ));
inline const vector signed int spu_merge_lo( const vector signed int a, const vector signed int b )
{
    return spu_shuffle( a, b, ShuffleWord< 2, 6,
                                           3, 7 >::Pattern );
}

inline const vector unsigned int spu_merge_hi( const vector unsigned int a, const vector unsigned int b ) __attribute(( pure, always_inline ));
inline const vector unsigned int spu_merge_hi( const vector unsigned int a, const vector unsigned int b )
{
    return spu_shuffle( a, b, ShuffleWord< 0, 4,
                                           1, 5 >::Pattern );
}

inline const vector unsigned int spu_merge_lo( const vector unsigned int a, const vector unsigned int b ) __attribute(( pure, always_inline ));
inline const vector unsigned int spu_merge_lo( const vector unsigned int a, const vector unsigned int b )
{
    return spu_shuffle( a, b, ShuffleWord< 2, 6,
                                           3, 7 >::Pattern );
}



// SPU logical intrinsics: spu_and, spu_andc, spu_eqv, spu_nand, spu_nor, spu_or, spu_orc, spu_xor, spu_orx

inline const vector unsigned int spu_not( const vector unsigned int a ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_not( const vector unsigned int a )
{
    return spu_nor( a, a );
}

inline const vector unsigned short spu_not( const vector unsigned short a ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_not( const vector unsigned short a )
{
    return spu_nor( a, a );
}

inline const vector unsigned char spu_not( const vector unsigned char a ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_not( const vector unsigned char a )
{
    return spu_nor( a, a );
}

inline const vector float spu_not( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float spu_not( const vector float a )
{
    return spu_nor( a, a );
}


//inline const vector float spu_and( const vector float a, const vector unsigned int b ) __attribute__(( const, always_inline ));
//inline const vector float spu_and( const vector float a, const vector unsigned int b )
//{
//    return (vector float)spu_and( a, (vector float)b );
//}


inline const vector unsigned int spu_horizontal_or( const vector unsigned int a ) __attribute(( const, always_inline ));
inline const vector unsigned int spu_horizontal_or( const vector unsigned int a )
{
    return spu_splat<0>( spu_orx( a ) );
}

inline const vector unsigned int spu_horizontal_or( const vector unsigned short a ) __attribute(( const, always_inline ));
inline const vector unsigned int spu_horizontal_or( const vector unsigned short a )
{
    const vector unsigned int b( spu_orx( (vector unsigned int)a ) );
    const vector unsigned int c( spu_shuffle( b, b, ShuffleByte<0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2>::Pattern ) );
    const vector unsigned int d( spu_orx( c ) );
    const vector unsigned int e( spu_shuffle( d, d, ShuffleWord<0,0,0,0>::Pattern ) );
    return e;
}

inline const vector unsigned int spu_horizontal_or( const vector unsigned char a ) __attribute(( const, always_inline ));
inline const vector unsigned int spu_horizontal_or( const vector unsigned char a )
{
    const vector unsigned int b( spu_orx( (vector unsigned int)a ) );
    const vector unsigned int c( spu_shuffle( b, b, ShuffleByte<0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3>::Pattern ) );
    const vector unsigned int d( spu_orx( c ) );
    const vector unsigned int e( spu_shuffle( d, d, ShuffleWord<0,0,0,0>::Pattern ) );
    return e;
}


inline const vector unsigned int spu_horizontal_and( const vector unsigned int a ) __attribute(( const, always_inline ));
inline const vector unsigned int spu_horizontal_and( const vector unsigned int a )
{
    return spu_not( spu_splat<0>( spu_orx( spu_not( a ) ) ) );
}

inline const vector unsigned int spu_horizontal_andnot( const vector unsigned int a ) __attribute(( const, always_inline ));
inline const vector unsigned int spu_horizontal_andnot( const vector unsigned int a )
{
    return spu_not( spu_splat<0>( spu_orx( a ) ) );
}

inline const vector unsigned int spu_horizontal_and( const vector unsigned short a ) __attribute(( const, always_inline ));
inline const vector unsigned int spu_horizontal_and( const vector unsigned short a )
{
    const vector unsigned int b( spu_orx( (vector unsigned int)spu_not( a ) ) );
    const vector unsigned int c( spu_shuffle( b, b, ShuffleByte<0,0,0,0,0,0,0,0,2,2,2,2,2,2,2,2>::Pattern ) );
    const vector unsigned int d( spu_orx( c ) );
    const vector unsigned int e( spu_shuffle( d, d, ShuffleWord<0,0,0,0>::Pattern ) );
    return spu_not( e );
}

inline const vector unsigned int spu_horizontal_and( const vector unsigned char a ) __attribute(( const, always_inline ));
inline const vector unsigned int spu_horizontal_and( const vector unsigned char a )
{
    const vector unsigned int b( spu_orx( (vector unsigned int)spu_not( a ) ) );
    const vector unsigned int c( spu_shuffle( b, b, ShuffleByte<0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3>::Pattern ) );
    const vector unsigned int d( spu_orx( c ) );
    const vector unsigned int e( spu_shuffle( d, d, ShuffleWord<0,0,0,0>::Pattern ) );
    return spu_not( e );
}


//inline const vector float spu_horizontal_or( const vector float a ) __attribute(( const, always_inline ));
//inline const vector float spu_horizontal_or( const vector float a )
//{
//    return (vector float)spu_splat<0>( spu_orx( (vector unsigned int)a ) );
//}
//
//inline const vector float spu_horizontal_and( const vector float a ) __attribute(( const, always_inline ));
//inline const vector float spu_horizontal_and( const vector float a )
//{
//    return (vector float)spu_not( spu_splat<0>( spu_orx( (vector unsigned int)spu_not( a ) ) ) );
//}


template < unsigned char I > inline const vector float spu_rotate_left_qw_words( const vector float a ) __attribute(( pure, always_inline ));
template < unsigned char I > inline const vector float spu_rotate_left_qw_words( const vector float a )
{
    StaticAssert( I <= 4, I_MUST_BE_LESS_THAN_OR_EQUAL_TO_4 );
    return spu_rotate_left_qw_bytes<I*4>( a );
}

template < unsigned char I > inline const vector float spu_rotate_left_qw_words( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
template < unsigned char I > inline const vector float spu_rotate_left_qw_words( const vector float a, const vector float b )
{
    const vector unsigned char pattern = ShuffleByte< (I<<2) +  0, (I<<2) +  1, (I<<2) +  2, (I<<2) +  3,
                                                      (I<<2) +  4, (I<<2) +  5, (I<<2) +  6, (I<<2) +  7,
                                                      (I<<2) +  8, (I<<2) +  9, (I<<2) + 10, (I<<2) + 11,
                                                      (I<<2) + 12, (I<<2) + 13, (I<<2) + 14, (I<<2) + 15 >::Pattern;
    return spu_shuffle( a, b, pattern );
}

template < unsigned int maskX, unsigned int maskY, unsigned int maskZ, unsigned int maskW >
inline const vector float spu_vrlimi( vector float dst, const vector float a, const U32 vrot ) __attribute__(( pure, always_inline ));
template < unsigned int maskX, unsigned int maskY, unsigned int maskZ, unsigned int maskW >
inline const vector float spu_vrlimi( vector float dst, const vector float a, const U32 vrot )
{
    ////vector rotate left immediate and mask insert...
    //#if 1
    //const U32 vrotls2 = vrot<<2;
    //#if 0
    //// This Code Causes 4.1.1 to Have an internal compiler error.
    //const vector unsigned char pattern = (vector unsigned char)
    //{ (unsigned char)(vrotls2),      (unsigned char)(vrotls2 +  1), (unsigned char)(vrotls2 +  2), (unsigned char)(vrotls2 +  3),
    //  (unsigned char)(vrotls2 +  4), (unsigned char)(vrotls2 +  5), (unsigned char)(vrotls2 +  6), (unsigned char)(vrotls2 +  7),
    //  (unsigned char)(vrotls2 +  8), (unsigned char)(vrotls2 +  9), (unsigned char)(vrotls2 + 10), (unsigned char)(vrotls2 + 11),
    //  (unsigned char)(vrotls2 + 12), (unsigned char)(vrotls2 + 13), (unsigned char)(vrotls2 + 14), (unsigned char)(vrotls2 + 15)};
    //#else
    //const unsigned char vrotls2c = vrotls2;
    //const vector unsigned char pattern = (vector unsigned char)
    //{   vrotls2c + 0,   vrotls2c + 1,   vrotls2c + 2,   vrotls2c + 3,
    //    vrotls2c + 4,   vrotls2c + 5,   vrotls2c + 6,   vrotls2c + 7,
    //    vrotls2c + 8,   vrotls2c + 9,   vrotls2c + 10,  vrotls2c + 11,
    //    vrotls2c + 12,  vrotls2c + 13,  vrotls2c + 14,  vrotls2c + 15 };
    //#endif
    //#else
    //const vector unsigned int pattern( ShuffleWord< vrot, vrot + 1, vrot + 2, vrot + 3 >::Pattern );
    //#endif
    //
    ////For compatibility with the 360 version, vmask's 4 LSBs contain the masks indicating if we should choose the 
    ////

    //vector float rotated = spu_shuffle(a, a, pattern);
    
    const vector float rotated = spu_rlqwbyte( a, vrot * 4 );
	//Now need to return a masked vector which selects from the two based on this...
	vector unsigned char mask = (vector unsigned char)ShuffleWord<maskX, maskY, maskZ, maskW>::Pattern;
	//return spu_sel(dst, rotated, mask);
	return spu_shuffle(dst, rotated, mask);
}

template <U32 uImmed>
inline vector float spu_spltw(vector float src)
{
	/*float val = spu_extract(src, uImmed);
	return spu_splats(val);*/
	//Do this with shuffle. Much better...
	const vector unsigned char pattern = ShuffleWord<uImmed, uImmed, uImmed, uImmed>::Pattern;
	return spu_shuffle(src, src, pattern);
}

template <U32 uImmed>
inline vector signed int spu_spltw(vector signed int src)
{
	/*float val = spu_extract(src, uImmed);
	return spu_splats(val);*/
	//Do this with shuffle. Much better...
	const vector unsigned char pattern = ShuffleWord<uImmed, uImmed, uImmed, uImmed>::Pattern;
	return spu_shuffle(src, src, pattern);
}

inline const vector float spu_mrghw(const vector float a, const vector float b)
{
	const vector unsigned char pattern = ShuffleWord<0, 4, 1, 5>::Pattern;
	return spu_shuffle(a, b, pattern);
}

inline const vector signed int spu_mrghw(const vector signed int a, const vector signed int b)
{
	const vector unsigned char pattern = ShuffleWord<0, 4, 1, 5>::Pattern;
	return spu_shuffle(a, b, pattern);
}

inline const vector float spu_mrglw(const vector float a, const vector float b)
{
	const vector unsigned char pattern = ShuffleWord<2, 6, 3, 7>::Pattern;
	return spu_shuffle(a, b, pattern);
}

inline const vector signed int spu_mrglw(const vector signed int a, const vector signed int b)
{
	const vector unsigned char pattern = ShuffleWord<2, 6, 3, 7>::Pattern;
	return spu_shuffle(a, b, pattern);
}


inline const vector float spu_abs( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float spu_abs( const vector float a )
{
    return (vector float)( spu_rlmask( spu_sl( (vector unsigned int)a, 1 ), -1 ) );
}

inline const vector float spu_negate( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_negate( const vector float a )
{
    const vector float sign_mask( Const< vector float >::SignMask() );
    return spu_xor( a, sign_mask );
}



inline const vector float spu_min( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float spu_min( const vector float a, const vector float b )
{
    return spu_sel( a, b, spu_cmpgt( a, b ) );
}

inline const vector signed int spu_min( const vector signed int a, const vector signed int b ) __attribute__(( const, always_inline ));
inline const vector signed int spu_min( const vector signed int a, const vector signed int b )
{
    return spu_sel( a, b, spu_cmpgt( a, b ) );
}

inline const vector unsigned int spu_min( const vector unsigned int a, const vector unsigned int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_min( const vector unsigned int a, const vector unsigned int b )
{
    return spu_sel( a, b, spu_cmpgt( a, b ) );
}

inline const vector signed short spu_min( const vector signed short a, const vector signed short b ) __attribute__(( const, always_inline ));
inline const vector signed short spu_min( const vector signed short a, const vector signed short b )
{
    return spu_sel( a, b, spu_cmpgt( a, b ) );
}

inline const vector unsigned short spu_min( const vector unsigned short a, const vector unsigned short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_min( const vector unsigned short a, const vector unsigned short b )
{
    return spu_sel( a, b, spu_cmpgt( a, b ) );
}

inline const vector signed char spu_min( const vector signed char a, const vector signed char b ) __attribute__(( const, always_inline ));
inline const vector signed char spu_min( const vector signed char a, const vector signed char b )
{
    return spu_sel( a, b, spu_cmpgt( a, b ) );
}

inline const vector unsigned char spu_min( const vector unsigned char a, const vector unsigned char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_min( const vector unsigned char a, const vector unsigned char b )
{
    return spu_sel( a, b, spu_cmpgt( a, b ) );
}



inline const vector float spu_max( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector float spu_max( const vector float a, const vector float b )
{
    return spu_sel( b, a, spu_cmpgt( a, b ) );
}

inline const vector signed int spu_max( const vector signed int a, const vector signed int b ) __attribute__(( const, always_inline ));
inline const vector signed int spu_max( const vector signed int a, const vector signed int b )
{
    return spu_sel( b, a, spu_cmpgt( a, b ) );
}

inline const vector unsigned int spu_max( const vector unsigned int a, const vector unsigned int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_max( const vector unsigned int a, const vector unsigned int b )
{
    return spu_sel( b, a, spu_cmpgt( a, b ) );
}

inline const vector signed short spu_max( const vector signed short a, const vector signed short b ) __attribute__(( const, always_inline ));
inline const vector signed short spu_max( const vector signed short a, const vector signed short b )
{
    return spu_sel( b, a, spu_cmpgt( a, b ) );
}

inline const vector unsigned short spu_max( const vector unsigned short a, const vector unsigned short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_max( const vector unsigned short a, const vector unsigned short b )
{
    return spu_sel( b, a, spu_cmpgt( a, b ) );
}

inline const vector signed char spu_max( const vector signed char a, const vector signed char b ) __attribute__(( const, always_inline ));
inline const vector signed char spu_max( const vector signed char a, const vector signed char b )
{
    return spu_sel( b, a, spu_cmpgt( a, b ) );
}

inline const vector unsigned char spu_max( const vector unsigned char a, const vector unsigned char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_max( const vector unsigned char a, const vector unsigned char b )
{
    return spu_sel( b, a, spu_cmpgt( a, b ) );
}



inline const vector float spu_clamp( const vector float a, const vector float lo, const vector float hi ) __attribute__(( const, always_inline ));
inline const vector float spu_clamp( const vector float a, const vector float lo, const vector float hi )
{
    return spu_min( spu_max( a, lo ), hi );
}

inline const vector signed int spu_clamp( const vector signed int a, const vector signed int lo, const vector signed int hi ) __attribute__(( const, always_inline ));
inline const vector signed int spu_clamp( const vector signed int a, const vector signed int lo, const vector signed int hi )
{
    return spu_min( spu_max( a, lo ), hi );
}

inline const vector unsigned int spu_clamp( const vector unsigned int a, const vector unsigned int lo, const vector unsigned int hi ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_clamp( const vector unsigned int a, const vector unsigned int lo, const vector unsigned int hi )
{
    return spu_min( spu_max( a, lo ), hi );
}

inline const vector signed short spu_clamp( const vector signed short a, const vector signed short lo, const vector signed short hi ) __attribute__(( const, always_inline ));
inline const vector signed short spu_clamp( const vector signed short a, const vector signed short lo, const vector signed short hi )
{
    return spu_min( spu_max( a, lo ), hi );
}

inline const vector unsigned short spu_clamp( const vector unsigned short a, const vector unsigned short lo, const vector unsigned short hi ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_clamp( const vector unsigned short a, const vector unsigned short lo, const vector unsigned short hi )
{
    return spu_min( spu_max( a, lo ), hi );
}

inline const vector signed char spu_clamp( const vector signed char a, const vector signed char lo, const vector signed char hi ) __attribute__(( const, always_inline ));
inline const vector signed char spu_clamp( const vector signed char a, const vector signed char lo, const vector signed char hi )
{
    return spu_min( spu_max( a, lo ), hi );
}

inline const vector unsigned char spu_clamp( const vector unsigned char a, const vector unsigned char lo, const vector unsigned char hi ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_clamp( const vector unsigned char a, const vector unsigned char lo, const vector unsigned char hi )
{
    return spu_min( spu_max( a, lo ), hi );
}



inline const vector unsigned int spu_cmpne( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmpne( const vector float a, const vector float b )
{
    return spu_not( spu_cmpeq( a, b ) );
}

inline const vector unsigned int spu_cmpne( const vector signed int a, const vector signed int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmpne( const vector signed int a, const vector signed int b )
{
    return spu_not( spu_cmpeq( a, b ) );
}

inline const vector unsigned int spu_cmpne( const vector unsigned int a, const vector unsigned int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmpne( const vector unsigned int a, const vector unsigned int b )
{
    return spu_not( spu_cmpeq( a, b ) );
}

inline const vector unsigned short spu_cmpne( const vector signed short a, const vector signed short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_cmpne( const vector signed short a, const vector signed short b )
{
    return spu_not( spu_cmpeq( a, b ) );
}

inline const vector unsigned short spu_cmpne( const vector unsigned short a, const vector unsigned short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_cmpne( const vector unsigned short a, const vector unsigned short b )
{
    return spu_not( spu_cmpeq( a, b ) );
}

inline const vector unsigned char spu_cmpne( const vector signed char a, const vector signed char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_cmpne( const vector signed char a, const vector signed char b )
{
    return spu_not( spu_cmpeq( a, b ) );
}

inline const vector unsigned char spu_cmpne( const vector unsigned char a, const vector unsigned char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_cmpne( const vector unsigned char a, const vector unsigned char b )
{
    return spu_not( spu_cmpeq( a, b ) );
}


inline const vector unsigned int spu_cmpge( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmpge( const vector float a, const vector float b )
{
    return spu_not( spu_cmpgt( b, a ) );
}

inline const vector unsigned int spu_cmpge( const vector signed int a, const vector signed int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmpge( const vector signed int a, const vector signed int b )
{
    return spu_not( spu_cmpgt( b, a ) );
}

inline const vector unsigned int spu_cmpge( const vector unsigned int a, const vector unsigned int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmpge( const vector unsigned int a, const vector unsigned int b )
{
    return spu_not( spu_cmpgt( b, a ) );
}

inline const vector unsigned short spu_cmpge( const vector signed short a, const vector signed short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_cmpge( const vector signed short a, const vector signed short b )
{
    return spu_not( spu_cmpgt( b, a ) );
}

inline const vector unsigned short spu_cmpge( const vector unsigned short a, const vector unsigned short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_cmpge( const vector unsigned short a, const vector unsigned short b )
{
    return spu_not( spu_cmpgt( b, a ) );
}

inline const vector unsigned char spu_cmpge( const vector signed char a, const vector signed char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_cmpge( const vector signed char a, const vector signed char b )
{
    return spu_not( spu_cmpgt( b, a ) );
}

inline const vector unsigned char spu_cmpge( const vector unsigned char a, const vector unsigned char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_cmpge( const vector unsigned char a, const vector unsigned char b )
{
    return spu_not( spu_cmpgt( b, a ) );
}


inline const vector unsigned int spu_cmple( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmple( const vector float a, const vector float b )
{
    return spu_cmpge( b, a );
}

inline const vector unsigned int spu_cmple( const vector signed int a, const vector signed int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmple( const vector signed int a, const vector signed int b )
{
    return spu_cmpge( b, a );
}

inline const vector unsigned int spu_cmple( const vector unsigned int a, const vector unsigned int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmple( const vector unsigned int a, const vector unsigned int b )
{
    return spu_cmpge( b, a );
}

inline const vector unsigned short spu_cmple( const vector signed short a, const vector signed short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_cmple( const vector signed short a, const vector signed short b )
{
    return spu_cmpge( b, a );
}

inline const vector unsigned short spu_cmple( const vector unsigned short a, const vector unsigned short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_cmple( const vector unsigned short a, const vector unsigned short b )
{
    return spu_cmpge( b, a );
}

inline const vector unsigned char spu_cmple( const vector signed char a, const vector signed char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_cmple( const vector signed char a, const vector signed char b )
{
    return spu_cmpge( b, a );
}

inline const vector unsigned char spu_cmple( const vector unsigned char a, const vector unsigned char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_cmple( const vector unsigned char a, const vector unsigned char b )
{
    return spu_cmpge( b, a );
}


inline const vector unsigned int spu_cmplt( const vector float a, const vector float b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmplt( const vector float a, const vector float b )
{
    return spu_cmpgt( b, a );
}

inline const vector unsigned int spu_cmplt( const vector signed int a, const vector signed int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmplt( const vector signed int a, const vector signed int b )
{
    return spu_cmpgt( b, a );
}

inline const vector unsigned int spu_cmplt( const vector unsigned int a, const vector unsigned int b ) __attribute__(( const, always_inline ));
inline const vector unsigned int spu_cmplt( const vector unsigned int a, const vector unsigned int b )
{
    return spu_cmpgt( b, a );
}

inline const vector unsigned short spu_cmplt( const vector signed short a, const vector signed short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_cmplt( const vector signed short a, const vector signed short b )
{
    return spu_cmpgt( b, a );
}

inline const vector unsigned short spu_cmplt( const vector unsigned short a, const vector unsigned short b ) __attribute__(( const, always_inline ));
inline const vector unsigned short spu_cmplt( const vector unsigned short a, const vector unsigned short b )
{
    return spu_cmpgt( b, a );
}

inline const vector unsigned char spu_cmplt( const vector signed char a, const vector signed char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_cmplt( const vector signed char a, const vector signed char b )
{
    return spu_cmpgt( b, a );
}

inline const vector unsigned char spu_cmplt( const vector unsigned char a, const vector unsigned char b ) __attribute__(( const, always_inline ));
inline const vector unsigned char spu_cmplt( const vector unsigned char a, const vector unsigned char b )
{
    return spu_cmpgt( b, a );
}


inline const vector float spu_copysign( const vector float value, const vector float sign ) __attribute__(( pure, always_inline ));
inline const vector float spu_copysign( const vector float value, const vector float sign )
{
    return spu_sel( value, sign, Const< vector unsigned int >::SignMask() );
}

inline const vector float spu_rfiz( const vector float a ) __attribute__(( const, always_inline ));
inline const vector float spu_rfiz( const vector float a )
{
    return spu_convtf( spu_convts( a, 0 ), 0 );
}

inline const vector float spu_rfin( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_rfin( const vector float a )
{
    #if 0
    //const vector float zero = (vector float)spu_splats( 0 );
    const vector float zero = Const< vector float >::Zero();
    const vector unsigned int a_is_negative = spu_cmplt( a, zero );
    //const vector float half = spu_splats( 0.5f );
    const vector float half = Const< vector float >::Half();
    const vector float pos_adjusted_a = spu_add( a, half );
    const vector float neg_adjusted_a = spu_sub( a, half );
    const vector float adjusted_a = spu_sel( pos_adjusted_a, neg_adjusted_a, a_is_negative );
    return spu_convtf( spu_convts( adjusted_a, 0 ), 0 );
    #else
    // _copysign( floorf( fabsf( x.m_V ) + 0.5f ), x.m_V )
    const vector unsigned int abs_mask( Const< vector unsigned int >::AbsMask() );
    const vector float half( Const< vector float >::Half() );
    return spu_sel( a, spu_convtf( spu_convts( spu_add( spu_and( a, (vector float)abs_mask ), half ), 0 ), 0 ), abs_mask );
    #endif
}

inline const vector float spu_rfim( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_rfim( const vector float a )
{
    #if 0
    //const vector float zero = (vector float)spu_splats( 0 );
    const vector float zero = Const< vector float >::Zero();
    const vector unsigned int a_is_negative = spu_cmplt( a, zero );
    //const vector float one_minus_1_ulp = spu_splats( 1.0f - FLT_EPSILON ); // 1.0f - 1 ulp ~= 0.9999f
    const vector float one_minus_1_ulp = Const< vector float >::OneMinusEpsilon(); // 1.0f - 1 ulp ~= 0.9999f
    const vector float pos_adjusted_a = a;
    const vector float neg_adjusted_a = spu_sub( a, one_minus_1_ulp );
    const vector float adjusted_a = spu_sel( pos_adjusted_a, neg_adjusted_a, a_is_negative );
    return spu_convtf( spu_convts( adjusted_a, 0 ), 0 );
    #else
    const vector float zero( Const< vector float >::Zero() );
    const vector float one( Const< vector float >::One() );
    const vector float trunc_a( spu_convtf( spu_convts( a, 0 ), 0 ) );
    const vector unsigned int negative_a( spu_cmpgt( zero, a ) );
    const vector float neg_to_pos( spu_and( spu_sub( one, trunc_a ), (vector float)negative_a ) ); // If a -ve : Minimum integer value to add to a such that the result is positive.
    return spu_sub( spu_convtf( spu_convts( spu_add( a, neg_to_pos ), 0 ), 0 ), neg_to_pos );
    #endif
}

inline const vector float spu_rfip( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_rfip( const vector float a )
{
    #if 0
    //const vector float zero = (vector float)spu_splats( 0 );
    const vector float zero = Const< vector float >::Zero();
    const vector unsigned int a_is_negative = spu_cmplt( a, zero );
    //const vector float one_minus_1_ulp = spu_splats( 1.0f - FLT_EPSILON ); // 1.0f - 1 ulp ~= 0.9999f
    const vector float one_minus_1_ulp = Const< vector float >::OneMinusEpsilon(); // 1.0f - 1 ulp ~= 0.9999f
    const vector float pos_adjusted_a = spu_add( a, one_minus_1_ulp );
    const vector float neg_adjusted_a = a;
    const vector float adjusted_a = spu_sel( pos_adjusted_a, neg_adjusted_a, a_is_negative );
    return spu_convtf( spu_convts( adjusted_a, 0 ), 0 );
    #else
    const vector float zero( Const< vector float >::Zero() );
    const vector float one( Const< vector float >::One() );
    const vector float trunc_a( spu_convtf( spu_convts( a, 0 ), 0 ) );
    const vector unsigned int positive_a( spu_not( spu_cmpgt( zero, a ) ) );
    const vector float pos_to_neg( spu_and( spu_add( one, trunc_a ), (vector float)positive_a ) ); // If a +ve : Minimum integer value to subtract from a such that the result is negative.
    return spu_add( spu_convtf( spu_convts( spu_sub( a, pos_to_neg ), 0 ), 0 ), pos_to_neg );
    #endif
}


inline const vector float spu_sum2( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_sum2( const vector float a )
{
    return spu_add( spu_splat<VX>( a ),
                    spu_splat<VY>( a ) );
}

inline const vector float spu_sum3( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_sum3( const vector float a )
{
    return spu_add( spu_add( spu_splat<VX>( a ),
                             spu_splat<VY>( a ) ),
                             spu_splat<VZ>( a ) );
}

inline const vector float spu_sum4( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_sum4( const vector float a )
{
    return spu_add( spu_add( spu_splat<VX>( a ),
                             spu_splat<VY>( a ) ),
                    spu_add( spu_splat<VZ>( a ),
                             spu_splat<VW>( a ) ) );
}

inline const vector float spu_length_squared2( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_length_squared2( const vector float a )
{
    const vector float a2 = spu_mul( a, a );
    return spu_add( spu_splat<VX>( a2 ),
                    spu_splat<VY>( a2 ) );
}

inline const vector float spu_length_squared3( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_length_squared3( const vector float a )
{
    const vector float a2 = spu_mul( a, a );
    return spu_add( spu_add( spu_splat<VX>( a2 ),
                             spu_splat<VY>( a2 ) ),
                             spu_splat<VZ>( a2 ) );
}

inline const vector float spu_length_squared4( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_length_squared4( const vector float a )
{
    const vector float a2 = spu_mul( a, a );
    return spu_add( spu_add( spu_splat<VX>( a2 ),
                             spu_splat<VY>( a2 ) ),
                    spu_add( spu_splat<VZ>( a2 ),
                             spu_splat<VW>( a2 ) ) );
}

inline const vector float spu_dot_product2( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
inline const vector float spu_dot_product2( const vector float a, const vector float b )
{
    const vector float ab = spu_mul( a, b );
    return spu_add( spu_splat<VX>( ab ),
                    spu_splat<VY>( ab ) );
}

inline const vector float spu_dot_product3( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
inline const vector float spu_dot_product3( const vector float a, const vector float b )
{
    const vector float ab = spu_mul( a, b );
    return spu_add( spu_add( spu_splat<VX>( ab ),
                             spu_splat<VY>( ab ) ),
                             spu_splat<VZ>( ab ) );
}

inline const vector float spu_dot_product4( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
inline const vector float spu_dot_product4( const vector float a, const vector float b )
{
    const vector float ab = spu_mul( a, b );
    return spu_add( spu_add( spu_splat<VX>( ab ),
                             spu_splat<VY>( ab ) ),
                    spu_add( spu_splat<VZ>( ab ),
                             spu_splat<VW>( ab ) ) );
}

inline const vector float spu_cross_product2( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
inline const vector float spu_cross_product2( const vector float a, const vector float b )
{
    return spu_nmsub( spu_splat<VY>( a ), spu_splat<VX>( b ),
             spu_mul( spu_splat<VX>( a ), spu_splat<VY>( b ) ) );
}

inline const vector float spu_cross_product3( const vector float a, const vector float b ) __attribute__(( pure, always_inline ));
inline const vector float spu_cross_product3( const vector float a, const vector float b )
{
    static const unsigned __ = V0W;
    const vector unsigned char permute_yzx = spu_shuffle_pattern_word<V0Y,V0Z,V0X,__>();
    const vector unsigned char permute_zxy = spu_shuffle_pattern_word<V0Z,V0X,V0Y,__>();
    
    const vector float a_yzx = spu_shuffle( a, a, permute_yzx );
    const vector float a_zxy = spu_shuffle( a, a, permute_zxy );
    const vector float b_yzx = spu_shuffle( b, b, permute_yzx );
    const vector float b_zxy = spu_shuffle( b, b, permute_zxy );
    
    return spu_nmsub( a_zxy, b_yzx,
             spu_mul( a_yzx, b_zxy ) );
}




inline void spu_transpose_matrix33(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ ) __attribute__(( /* pure, */ always_inline ));
inline void spu_transpose_matrix33(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ )
{
    const vector float inW( Const< vector float >::Zero() );
    const vector float tmp0( spu_merge_xy( inX, inZ ) );
    const vector float tmp1( spu_merge_xy( inY, inW ) );
    const vector float tmp2( spu_merge_zw( inX, inZ ) );
    const vector float tmp3( spu_merge_zw( inY, inW ) );
    *pOutX = spu_merge_xy( tmp0, tmp1 );
    *pOutY = spu_merge_zw( tmp0, tmp1 );
    *pOutZ = spu_merge_xy( tmp2, tmp3 );
}

inline void spu_transpose_matrix43(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    const vector float             inW,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ ) __attribute__(( /* pure, */ always_inline ));
inline void spu_transpose_matrix43(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    const vector float             inW,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ )
{
    const vector float tmp0( spu_merge_xy( inX, inZ ) );
    const vector float tmp1( spu_merge_xy( inY, inW ) );
    const vector float tmp2( spu_merge_zw( inX, inZ ) );
    const vector float tmp3( spu_merge_zw( inY, inW ) );
    *pOutX = spu_merge_xy( tmp0, tmp1 );
    *pOutY = spu_merge_zw( tmp0, tmp1 );
    *pOutZ = spu_merge_xy( tmp2, tmp3 );
}

inline void spu_transpose_matrix44(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    const vector float             inW,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ,
    vector float* const __restrict pOutW ) __attribute__(( /* pure, */ always_inline ));
inline void spu_transpose_matrix44(
    const vector float             inX,
    const vector float             inY,
    const vector float             inZ,
    const vector float             inW,
    vector float* const __restrict pOutX,
    vector float* const __restrict pOutY,
    vector float* const __restrict pOutZ,
    vector float* const __restrict pOutW )
{
    const vector float tmp0( spu_merge_xy( inX, inZ ) );
    const vector float tmp1( spu_merge_xy( inY, inW ) );
    const vector float tmp2( spu_merge_zw( inX, inZ ) );
    const vector float tmp3( spu_merge_zw( inY, inW ) );
    *pOutX = spu_merge_xy( tmp0, tmp1 );
    *pOutY = spu_merge_zw( tmp0, tmp1 );
    *pOutZ = spu_merge_xy( tmp2, tmp3 );
    *pOutW = spu_merge_zw( tmp2, tmp3 );
}

inline const vector float spu_transpose_col_x( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float spu_transpose_col_x( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return spu_merge_xy( spu_merge_xy( rowX, rowZ ),
                         spu_merge_xy( rowY, rowW ) );
}

inline const vector float spu_transpose_col_y( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float spu_transpose_col_y( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return spu_merge_zw( spu_merge_xy( rowX, rowZ ),
                         spu_merge_xy( rowY, rowW ) );
}

inline const vector float spu_transpose_col_z( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float spu_transpose_col_z( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return spu_merge_xy( spu_merge_zw( rowX, rowZ ),
                         spu_merge_zw( rowY, rowW ) );
}

inline const vector float spu_transpose_col_w( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float spu_transpose_col_w( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return spu_merge_zw( spu_merge_zw( rowX, rowZ ),
                         spu_merge_zw( rowY, rowW ) );
}



/// vector3 <-- vector3 [Implicit W=0] * matrix33
inline const vector float spu_multiply_vector3_matrix33( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_vector3_matrix33( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    return spu_madd( spu_splat<VX>( v ), rowX,
           spu_madd( spu_splat<VY>( v ), rowY,
           spu_mul ( spu_splat<VZ>( v ), rowZ ) ) );
}

/// vector3 <-- vector3 [Implicit W=0] * matrix43 [ Implicit column W=(0,0,0,1) ]
inline const vector float spu_multiply_vector3_matrix43( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_vector3_matrix43( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    return spu_madd( spu_splat<VX>( v ), rowX,
           spu_madd( spu_splat<VY>( v ), rowY,
           spu_mul ( spu_splat<VZ>( v ), rowZ ) ) );
}

/// vector4 <-- vector3 [Implicit W=0] * matrix44
inline const vector float spu_multiply_vector3_matrix44( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_vector3_matrix44( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    return spu_madd( spu_splat<VX>( v ), rowX,
           spu_madd( spu_splat<VY>( v ), rowY,
           spu_mul ( spu_splat<VZ>( v ), rowZ ) ) );
}



/// vector3 <-- matrix33 * vector3 [Implicit W=0]
inline const vector float spu_multiply_matrix33_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float v ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_matrix33_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float v )
{
    vector float colX, colY, colZ;
    spu_transpose_matrix33( rowX, rowY, rowZ, &colX, &colY, &colZ );
    return spu_madd( spu_splat<VX>( v ), colX,
           spu_madd( spu_splat<VY>( v ), colY,
           spu_mul ( spu_splat<VZ>( v ), colZ ) ) );
}

/// vector4 <-- matrix43 [ Implicit column W=(0,0,0,1) ] * vector3 [Implicit W=0]
inline const vector float spu_multiply_matrix43_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW, const vector float v ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_matrix43_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW, const vector float v )
{
    vector float colX, colY, colZ;
    spu_transpose_matrix43( rowX, rowY, rowZ, rowW, &colX, &colY, &colZ );
    return spu_madd( spu_splat<VX>( v ), colX,
           spu_madd( spu_splat<VY>( v ), colY,
           spu_mul ( spu_splat<VZ>( v ), colZ ) ) );
}

/// vector4 <-- matrix44 * vector3 [Implicit W=0]
inline const vector float spu_multiply_matrix44_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW, const vector float v ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_matrix44_vector3( const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW, const vector float v )
{
    vector float colX, colY, colZ, colW;
    spu_transpose_matrix44( rowX, rowY, rowZ, rowW, &colX, &colY, &colZ, &colW );
    return spu_madd( spu_splat<VX>( v ), colX,
           spu_madd( spu_splat<VY>( v ), colY,
           spu_mul ( spu_splat<VZ>( v ), colZ ) ) );
}


/// vector4 <-- vector4 * matrix33 [ Implicit row W = column W = (0,0,0,1) ]
inline const vector float spu_multiply_vector4_matrix33( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_vector4_matrix33( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    const vector float result = spu_madd( spu_splat<VX>( v ), rowX,
                                spu_madd( spu_splat<VZ>( v ), rowY,
                                spu_mul ( spu_splat<VW>( v ), rowZ ) ) );
    return spu_select<0,0,0,1>( result, v );
}

/// vector4 <-- vector4 * matrix43 [ Implicit column W=(0,0,0,1) ]
inline const vector float spu_multiply_vector4_matrix43( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_vector4_matrix43( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    const vector float result = spu_add( spu_madd( spu_splat<VX>( v ), rowX,
                                         spu_mul ( spu_splat<VY>( v ), rowY ) ),
                                         spu_madd( spu_splat<VZ>( v ), rowZ,
                                         spu_mul ( spu_splat<VW>( v ), rowW ) ) );
    return spu_select<0,0,0,1>( result, v );
}

/// vector4 <-- vector4 * matrix44
inline const vector float spu_multiply_vector4_matrix44( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_vector4_matrix44( const vector float v, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return spu_add( spu_madd( spu_splat<VX>( v ), rowX,
                    spu_mul ( spu_splat<VY>( v ), rowY ) ),
                    spu_madd( spu_splat<VZ>( v ), rowZ,
                    spu_mul ( spu_splat<VW>( v ), rowW ) ) );
}



/// point3 <-- point3 [Implicit W=1] * matrix33
inline const vector float spu_multiply_point3_matrix33( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_point3_matrix33( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ )
{
    return spu_madd( spu_splat<VX>( p ), rowX,
           spu_madd( spu_splat<VY>( p ), rowY,
           spu_mul ( spu_splat<VZ>( p ), rowZ ) ) );
}

/// point3 <-- point3 [Implicit W=1] * matrix43 [ Implicit column W=(0,0,0,1) ]
inline const vector float spu_multiply_point3_matrix43( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_point3_matrix43( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return spu_add( spu_madd( spu_splat<VX>( p ), rowX,
                    spu_mul ( spu_splat<VY>( p ), rowY ) ),
                    spu_madd( spu_splat<VZ>( p ), rowZ,
                                                  rowW ) );
}

/// vector4 <-- point3 [Implicit W=1] * matrix44
inline const vector float spu_multiply_point3_matrix44( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW ) __attribute__(( pure, always_inline ));
inline const vector float spu_multiply_point3_matrix44( const vector float p, const vector float rowX, const vector float rowY, const vector float rowZ, const vector float rowW )
{
    return spu_add( spu_madd( spu_splat<VX>( p ), rowX,
                    spu_mul ( spu_splat<VY>( p ), rowY ) ),
                    spu_madd( spu_splat<VZ>( p ), rowZ,
                                                  rowW ) );
}



/// SPU pack 2 S32 vectors into an S16 vector with Signed Saturation
inline const vector signed short spu_pack_ss( const vector signed int a, const vector signed int b ) __attribute__(( pure, always_inline ));
inline const vector signed short spu_pack_ss( const vector signed int a, const vector signed int b )
{
    const vector signed int limit_lo( spu_splats( (signed int)-32768 ) );
    const vector signed int limit_hi( spu_splats( (signed int)32767 ) );
    const vector signed int a_sat( spu_clamp( a, limit_lo, limit_hi ) );
    const vector signed int b_sat( spu_clamp( b, limit_lo, limit_hi ) );
    const vector unsigned char pattern( ShuffleHalf< 1,  3,  5,  7,  
                                                     9,  11, 13, 15 >::Pattern );
    return (vector signed short)spu_shuffle( a_sat, b_sat, pattern );
}

/// SPU pack 2 S16 vectors into an S8 vector with Signed Saturation
inline const vector signed char spu_pack_ss( const vector signed short a, const vector signed short b ) __attribute__(( pure, always_inline ));
inline const vector signed char spu_pack_ss( const vector signed short a, const vector signed short b )
{
    const vector signed short limit_lo( spu_splats( (signed short)-128 ) );
    const vector signed short limit_hi( spu_splats( (signed short)127 ) );
    const vector signed short a_sat( spu_clamp( a, limit_lo, limit_hi ) );
    const vector signed short b_sat( spu_clamp( b, limit_lo, limit_hi ) );
    const vector unsigned char pattern( ShuffleByte< 1,  3,  5,  7,  9,  11, 13, 15,
                                                     17, 19, 21, 23, 25, 27, 29, 31 >::Pattern );
    return (vector signed char)spu_shuffle( a_sat, b_sat, pattern );
}

/// SPU pack 2 S16 vectors into a U8 vector with Unsigned Saturation
inline const vector unsigned char spu_pack_us( const vector signed short a, const vector signed short b ) __attribute__(( pure, always_inline ));
inline const vector unsigned char spu_pack_us( const vector signed short a, const vector signed short b )
{
    const vector signed short limit_lo( spu_splats( (signed short)0 ) );
    const vector signed short limit_hi( spu_splats( (signed short)255 ) );
    const vector signed short a_sat( spu_clamp( a, limit_lo, limit_hi ) );
    const vector signed short b_sat( spu_clamp( b, limit_lo, limit_hi ) );
    const vector unsigned char pattern( ShuffleByte< 1,  3,  5,  7,  9,  11, 13, 15,
                                                     17, 19, 21, 23, 25, 27, 29, 31 >::Pattern );
    return (vector unsigned char)spu_shuffle( a_sat, b_sat, pattern );
}

/// SPU pack 2 U16 vectors into a U8 vector with Unsigned Saturation
inline const vector unsigned char spu_pack_us( const vector unsigned short a, const vector unsigned short b ) __attribute__(( pure, always_inline ));
inline const vector unsigned char spu_pack_us( const vector unsigned short a, const vector unsigned short b )
{
    const vector unsigned short limit_hi( spu_splats( (unsigned short)255 ) );
    const vector unsigned short a_sat( spu_min( a, limit_hi ) );
    const vector unsigned short b_sat( spu_min( b, limit_hi ) );
    const vector unsigned char pattern( ShuffleByte< 1,  3,  5,  7,  9,  11, 13, 15,
                                                     17, 19, 21, 23, 25, 27, 29, 31 >::Pattern );
    return (vector unsigned char)spu_shuffle( a_sat, b_sat, pattern );
}


/// SPU pack 2 S32 vectors into an S16 vector (modulo)
inline const vector signed short spu_pack_mod( const vector signed int a, const vector signed int b ) __attribute__(( pure, always_inline ));
inline const vector signed short spu_pack_mod( const vector signed int a, const vector signed int b )
{
    const vector unsigned char pattern( ShuffleHalf< 1,  3,  5,  7,  
                                                     9,  11, 13, 15 >::Pattern );
    return (vector signed short)spu_shuffle( a, b, pattern );
}

/// SPU pack 2 U32 vectors into an U16 vector (modulo)
inline const vector unsigned short spu_pack_mod( const vector unsigned int a, const vector unsigned int b ) __attribute__(( pure, always_inline ));
inline const vector unsigned short spu_pack_mod( const vector unsigned int a, const vector unsigned int b )
{
    const vector unsigned char pattern( ShuffleHalf< 1,  3,  5,  7,  
                                                     9,  11, 13, 15 >::Pattern );
    return (vector unsigned short)spu_shuffle( a, b, pattern );
}


/// SPU pack 2 S16 vectors into an S8 vector (modulo)
inline const vector signed char spu_pack_mod( const vector signed short a, const vector signed short b ) __attribute__(( pure, always_inline ));
inline const vector signed char spu_pack_mod( const vector signed short a, const vector signed short b )
{
    const vector unsigned char pattern( ShuffleByte< 1,  3,  5,  7,  9,  11, 13, 15,
                                                     17, 19, 21, 23, 25, 27, 29, 31 >::Pattern );
    return (vector signed char)spu_shuffle( a, b, pattern );
}

/// SPU pack 2 U16 vectors into an U8 vector (modulo)
inline const vector unsigned char spu_pack_mod( const vector unsigned short a, const vector unsigned short b ) __attribute__(( pure, always_inline ));
inline const vector unsigned char spu_pack_mod( const vector unsigned short a, const vector unsigned short b )
{
    const vector unsigned char pattern( ShuffleByte< 1,  3,  5,  7,  9,  11, 13, 15,
                                                     17, 19, 21, 23, 25, 27, 29, 31 >::Pattern );
    return (vector unsigned char)spu_shuffle( a, b, pattern );
}


inline const vector signed int spu_unpack_hi( const vector signed short a ) __attribute__(( pure, always_inline ));
inline const vector signed int spu_unpack_hi( const vector signed short a )
{
    const vector unsigned char pattern( ShuffleHalf< 0, 0, 1, 1, 2, 2, 3, 3 >::Pattern );
    const vector signed short a_hi( spu_shuffle( a, a, pattern ) );
    return spu_extend( a_hi );
}

inline const vector signed int spu_unpack_lo( const vector signed short a ) __attribute__(( pure, always_inline ));
inline const vector signed int spu_unpack_lo( const vector signed short a )
{
    const vector unsigned char pattern( ShuffleHalf< 4, 4, 5, 5, 6, 6, 7, 7 >::Pattern );
    const vector signed short a_lo( spu_shuffle( a, a, pattern ) );
    return spu_extend( a_lo );
}


inline const vector float spu_get_exponent( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_get_exponent( const vector float a )
{
    const vector signed int biased_exponent = spu_srl<23>( spu_and( (vector signed int)a, spu_splats( (S32)0x7F800000 ) ) );
    const vector signed int exponent = spu_sub( biased_exponent, spu_splats( (S32)127 ) );
    return spu_convtf( exponent, 0 );
}

inline const vector float spu_set_exponent( const vector float a, const vector float exp ) __attribute__(( pure, always_inline ));
inline const vector float spu_set_exponent( const vector float a, const vector float exp )
{
    const vector signed int int_exp( spu_sl( spu_add( spu_convts( exp, 0 ), spu_splats( (S32)127 ) ), 23 ) );
    const vector signed int int_a( spu_and( (vector signed int)a, spu_splats( (S32)0x807FFFFF ) ) );
    return (vector float)spu_or( int_exp, int_a );
}

inline const vector float spu_set_exponent( const vector float a, const vector signed int exp ) __attribute__(( pure, always_inline ));
inline const vector float spu_set_exponent( const vector float a, const vector signed int exp )
{
    const vector signed int int_exp( spu_sl( spu_add( exp, spu_splats( (S32)127 ) ), 23 ) );
    const vector signed int int_a( spu_and( (vector signed int)a, spu_splats( (S32)0x807FFFFF ) ) );
    return (vector float)spu_or( int_exp, int_a );
}

inline const vector float spu_set_zero_exponent( const vector float a ) __attribute__(( pure, always_inline ));
inline const vector float spu_set_zero_exponent( const vector float a )
{
    return spu_or( spu_and( a, (vector float)spu_splats( (S32)0x807FFFFF ) ), (vector float)spu_splats( (S32)0x3F800000 ) );
}




inline const V128 SIMD_Multiply( const V128 a, const V128 b ) __attribute__(( const, always_inline ));
inline const V128 SIMD_Multiply( const V128 a, const V128 b )
{
	return spu_mul( a, b );
}

inline const V128 SIMD_Add( const V128 a, const V128 b ) __attribute__(( const, always_inline ));
inline const V128 SIMD_Add( const V128 a, const V128 b )
{
	return spu_add( a, b );
}

inline const V128 SIMD_Sub( const V128 a, const V128 b ) __attribute__(( const, always_inline ));
inline const V128 SIMD_Sub( const V128 a, const V128 b )
{
	return spu_sub( a, b );
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

template < int I > inline const V128 SIMD_Splat4( const V128 src ) __attribute__(( const, always_inline ));
template < int I > inline const V128 SIMD_Splat4( const V128 src )
{
    return spu_splats( spu_extract( src, I ) );
}

}; // namespace Maths

#endif // #if defined( _SPU_ )

#endif // #ifndef MATHS_HVEC_SIMD_SPU_INL




