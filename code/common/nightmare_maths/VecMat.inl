
//#include "stdafx.h"

#if defined( _DEBUG )
#define MATHS_UNMASK_FLOAT_EXCEPTIONS
#endif

//void FP_Init()
//{
//    // Set x87 rounding mode to round-toward-zero
//    _control87( _RC_CHOP, _MCW_RC );
//    
//    // Set x87 precision to 24 bit
//    _control87( _PC_24, _MCW_PC );
//    
//    #ifdef _SSE_
//    
//    // Set SSE rounding mode to round-toward-zero
//    _MM_SET_ROUNDING_MODE( _MM_ROUND_TOWARD_ZERO );
//    
//    // Set SSE underflow flush to zero mode on (no denormals)
//    _MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
//    
//    #endif
//}
//
//void FP_UnmaskExceptions()
//{
//	//return;
//
//    // UnMask x87 INVALID & ZERODIVIDE
//    // Mask   x87 DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _clear87();
//    _control87( _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
//    
//    #ifdef _SSE_
//    
//    // UnMask SSE INVALID, DIV_ZERO
//    // Mask   SSE DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _MM_SET_EXCEPTION_STATE( 0 );
//    _MM_SET_EXCEPTION_MASK( _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
//    
//    #endif
//}
//
//void FP_MaskExceptions()
//{
//    // Mask   x87 INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _control87( _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
//    
//    #ifdef _SSE_
//    
//    // Mask   SSE INVALID, DIV_ZERO, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
//    _MM_SET_EXCEPTION_MASK( _MM_MASK_INVALID | _MM_MASK_DIV_ZERO | _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
//    
//    #endif
//}


inline void FP_Init( const bool floatPrecision24bit = false, const bool floatFlushDenormalsToZero = true )
{
    #if defined( COMPILER_VC )
    
    // Set float rounding mode to round-to-nearest
    _controlfp( _RC_NEAR, _MCW_RC );
    
    #if !defined( _WIN64 ) && !defined( WIN64 )
    if ( floatPrecision24bit )
    {
        // Set float precision to 24 bits
        _controlfp( _PC_24, _MCW_PC );
    }
    #endif
    
    if ( floatFlushDenormalsToZero )
    {
        // Set float denormal flush to zero mode
        _controlfp( _DN_FLUSH, _MCW_DN );
    }
    
    // Clear float exceptions 
    _clearfp();
    
    #if defined( PLATFORM_XENON )
    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _controlfp( _EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    #else
    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _control87( _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    #endif
    
    #if defined( _SSE_ )
    
    // Clear SSE exception state
    _MM_SET_EXCEPTION_STATE( 0 );
    
    // Mask all SSE exceptions
    _MM_SET_EXCEPTION_MASK( _MM_MASK_INVALID | _MM_MASK_DIV_ZERO | _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
    
    // Set SSE rounding mode to round-to-nearest
    _MM_SET_ROUNDING_MODE( _MM_ROUND_NEAREST );
    
    // Set SSE denormal flush to zero mode
    _MM_SET_FLUSH_ZERO_MODE( _MM_FLUSH_ZERO_ON );
    
    #endif
    
    #endif
}

inline void FP_UnmaskExceptions()
{
    #if defined( COMPILER_VC )
    
    // Clear float exceptions 
    _clearfp();
    
    #if defined( PLATFORM_XENON )
    // UnMask float INVALID & ZERODIVIDE
    // Mask   float DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _controlfp( _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    #else
    // UnMask float INVALID & ZERODIVIDE
    // Mask   float DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _control87( _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    #endif
    
    #if defined( _SSE_ )
    
    // Clear SSE exception state
    _MM_SET_EXCEPTION_STATE( 0 );
    
    // UnMask SSE INVALID, DIV_ZERO
    // Mask   SSE DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _MM_SET_EXCEPTION_MASK( _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
    
    #endif
    
    #if defined( PLATFORM_XENON )
    __asm
    {
        mfmsr r11;
        ori r11, r11, 0x900;
        mtmsr r11;
    }
    #endif
    
    #endif
}

inline void FP_MaskExceptions()
{
    #if defined( COMPILER_VC )
    
    // Clear float exceptions 
    _clearfp();
    
    #if defined( PLATFORM_XENON )
    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _controlfp( _EM_INVALID | _EM_ZERODIVIDE | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    #else
    // Mask float INVALID, ZERODIVIDE, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _control87( _EM_INVALID | _EM_ZERODIVIDE | _EM_DENORMAL | _EM_OVERFLOW | _EM_UNDERFLOW | _EM_INEXACT, _MCW_EM );
    #endif
    
    #if defined( _SSE_ )
    
    // Clear SSE exception state
    _MM_SET_EXCEPTION_STATE( 0 );
    
    // Mask SSE INVALID, DIV_ZERO, DENORMAL, OVERFLOW, UNDERFLOW & INEXACT
    _MM_SET_EXCEPTION_MASK( _MM_MASK_INVALID | _MM_MASK_DIV_ZERO | _MM_MASK_DENORM | _MM_MASK_OVERFLOW | _MM_MASK_UNDERFLOW | _MM_MASK_INEXACT );
    
    #endif
    
    #endif
}


#ifdef _SSE_

// Code from Intel AP-928 : Streaming SIMD Extensions - Inverse of 4x4 Matrix - SIMD version

const HMat44 HMat44::Matrix44InverseCramersRuleSIMD() const
{
    __m128 minor0, minor1, minor2, minor3;
    __m128 row0, row1, row2, row3;
    __m128 det, tmp1;
    
    minor0 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, _MM_SHUFFLE(1,0,1,0) );
    minor1 = _mm_shuffle_ps( m_AZ.m_V, m_AW.m_V, _MM_SHUFFLE(1,0,1,0) );
    
    row0 = _mm_shuffle_ps( minor0, minor1, _MM_SHUFFLE(2,0,2,0) );
    row1 = _mm_shuffle_ps( minor1, minor0, _MM_SHUFFLE(3,1,3,1) );
    
    minor2 = _mm_shuffle_ps( m_AX.m_V, m_AY.m_V, _MM_SHUFFLE(3,2,3,2) );
    minor3 = _mm_shuffle_ps( m_AZ.m_V, m_AW.m_V, _MM_SHUFFLE(3,2,3,2) );
    
    row2 = _mm_shuffle_ps( minor2, minor3, _MM_SHUFFLE(1,0,1,0) );
    row3 = _mm_shuffle_ps( minor3, minor2, _MM_SHUFFLE(3,1,3,1) );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row2, row3 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor0 = _mm_mul_ps( row1, tmp1 );
    minor1 = _mm_mul_ps( row0, tmp1 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor0 = _mm_sub_ps( _mm_mul_ps( row1, tmp1 ), minor0 );
    minor1 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor1 );
    minor1 = _mm_shuffle_ps( minor1, minor1, 0x4E );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row1, row2 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor0 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor0 );
    minor3 = _mm_mul_ps( row0, tmp1 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor0 = _mm_sub_ps( minor0, _mm_mul_ps( row3, tmp1 ) );
    minor3 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor3 );
    minor3 = _mm_shuffle_ps( minor3, minor3, 0x4E );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( _mm_shuffle_ps( row1, row1, 0x4E ), row3 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    row2   = _mm_shuffle_ps( row2, row2, 0x4E );
    minor0 = _mm_add_ps( _mm_mul_ps( row2, tmp1 ), minor0 );
    minor2 = _mm_mul_ps( row0, tmp1 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor0 = _mm_sub_ps( minor0, _mm_mul_ps( row2, tmp1 ) );
    minor2 = _mm_sub_ps( _mm_mul_ps( row0, tmp1 ), minor2 );
    minor2 = _mm_shuffle_ps( minor2, minor2, 0x4E );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row0, row1 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor2 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor2 );
    minor3 = _mm_sub_ps( _mm_mul_ps( row2, tmp1 ), minor3 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor2 = _mm_sub_ps( _mm_mul_ps( row3, tmp1 ), minor2 );
    minor3 = _mm_sub_ps( minor3, _mm_mul_ps( row2, tmp1 ) );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row0, row3 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor1 = _mm_sub_ps( minor1, _mm_mul_ps( row2, tmp1 ) );
    minor2 = _mm_add_ps( _mm_mul_ps( row1, tmp1 ), minor2 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor1 = _mm_add_ps( _mm_mul_ps( row2, tmp1 ), minor1 );
    minor2 = _mm_sub_ps( minor2, _mm_mul_ps( row1, tmp1 ) );
    
    // -----------------------------------------------
    
    tmp1   = _mm_mul_ps( row0, row2 );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0xB1 );
    minor1 = _mm_add_ps( _mm_mul_ps( row3, tmp1 ), minor1 );
    minor3 = _mm_sub_ps( minor3, _mm_mul_ps( row1, tmp1 ) );
    tmp1   = _mm_shuffle_ps( tmp1, tmp1, 0x4E );
    minor1 = _mm_sub_ps( minor1, _mm_mul_ps( row3, tmp1 ) );
    minor3 = _mm_add_ps( _mm_mul_ps( row1, tmp1 ), minor3 );
    
    // -----------------------------------------------
    
    det  = _mm_mul_ps( row0, minor0 );
    det  = _mm_add_ps( _mm_shuffle_ps( det, det, 0x4E ), det );
    det  = _mm_add_ss( _mm_shuffle_ps( det, det, 0xB1 ), det );
    tmp1 = _mm_rcp_ss( det );
    det  = _mm_sub_ss( _mm_add_ss( tmp1, tmp1 ), _mm_mul_ss( det, _mm_mul_ss( tmp1, tmp1 ) ) );
    det  = _mm_shuffle_ps( det, det, 0x00 );
    
    minor0 = _mm_mul_ps( det, minor0 );
    minor1 = _mm_mul_ps( det, minor1 );
    minor2 = _mm_mul_ps( det, minor2 );
    minor3 = _mm_mul_ps( det, minor3 );
    
    return HMat44( minor0, minor1, minor2, minor3 );
}

#endif

// Code from Intel AP-928 : Streaming SIMD Extensions - Inverse of 4x4 Matrix - FP version

void Matrix44InverseCramersRuleFP( const F32* mat, F32* dst )
{
    F32 tmp[12];  // temp array for pairs
    F32 src[16];  // array of transpose source matrix
    F32 det;      // determinant
    
    // transpose matrix
    for ( int i = 0; i < 4; i++ )
    {
        src[i]      = mat[i*4];
        src[i + 4]  = mat[i*4 + 1];
        src[i + 8]  = mat[i*4 + 2];
        src[i + 12] = mat[i*4 + 3];
    }
    
    // calculate pairs for first 8 elements (cofactors)
    tmp[0]  = src[10] * src[15];
    tmp[1]  = src[11] * src[14];
    tmp[2]  = src[9]  * src[15];
    tmp[3]  = src[11] * src[13];
    tmp[4]  = src[9]  * src[14];
    tmp[5]  = src[10] * src[13];
    tmp[6]  = src[8]  * src[15];
    tmp[7]  = src[11] * src[12];
    tmp[8]  = src[8]  * src[14];
    tmp[9]  = src[10] * src[12];
    tmp[10] = src[8]  * src[13];
    tmp[11] = src[9]  * src[12];
    
    // calculate first 8 elements (cofactors)
    dst[0]  = tmp[0] * src[5] + tmp[3] * src[6] + tmp[4]  * src[7];
    dst[0] -= tmp[1] * src[5] + tmp[2] * src[6] + tmp[5]  * src[7];
    dst[1]  = tmp[1] * src[4] + tmp[6] * src[6] + tmp[9]  * src[7];
    dst[1] -= tmp[0] * src[4] + tmp[7] * src[6] + tmp[8]  * src[7];
    dst[2]  = tmp[2] * src[4] + tmp[7] * src[5] + tmp[10] * src[7];
    dst[2] -= tmp[3] * src[4] + tmp[6] * src[5] + tmp[11] * src[7];
    dst[3]  = tmp[5] * src[4] + tmp[8] * src[5] + tmp[11] * src[6];
    dst[3] -= tmp[4] * src[4] + tmp[9] * src[5] + tmp[10] * src[6];
    dst[4]  = tmp[1] * src[1] + tmp[2] * src[2] + tmp[5]  * src[3];
    dst[4] -= tmp[0] * src[1] + tmp[3] * src[2] + tmp[4]  * src[3];
    dst[5]  = tmp[0] * src[0] + tmp[7] * src[2] + tmp[8]  * src[3];
    dst[5] -= tmp[1] * src[0] + tmp[6] * src[2] + tmp[9]  * src[3];
    dst[6]  = tmp[3] * src[0] + tmp[6] * src[1] + tmp[11] * src[3];
    dst[6] -= tmp[2] * src[0] + tmp[7] * src[1] + tmp[10] * src[3];
    dst[7]  = tmp[4] * src[0] + tmp[9] * src[1] + tmp[10] * src[2];
    dst[7] -= tmp[5] * src[0] + tmp[8] * src[1] + tmp[11] * src[2];
    
    // calculate pairs for second 8 elements (cofactors)
    tmp[0]  = src[2] * src[7];
    tmp[1]  = src[3] * src[6];
    tmp[2]  = src[1] * src[7];
    tmp[3]  = src[3] * src[5];
    tmp[4]  = src[1] * src[6];
    tmp[5]  = src[2] * src[5];
    tmp[6]  = src[0] * src[7];
    tmp[7]  = src[3] * src[4];
    tmp[8]  = src[0] * src[6];
    tmp[9]  = src[2] * src[4];
    tmp[10] = src[0] * src[5];
    tmp[11] = src[1] * src[4];
    
    // calculate second 8 elements (cofactors)
    dst[8]   = tmp[0]  * src[13] + tmp[3]  * src[14] + tmp[4]  * src[15];
    dst[8]  -= tmp[1]  * src[13] + tmp[2]  * src[14] + tmp[5]  * src[15];
    dst[9]   = tmp[1]  * src[12] + tmp[6]  * src[14] + tmp[9]  * src[15];
    dst[9]  -= tmp[0]  * src[12] + tmp[7]  * src[14] + tmp[8]  * src[15];
    dst[10]  = tmp[2]  * src[12] + tmp[7]  * src[13] + tmp[10] * src[15];
    dst[10] -= tmp[3]  * src[12] + tmp[6]  * src[13] + tmp[11] * src[15];
    dst[11]  = tmp[5]  * src[12] + tmp[8]  * src[13] + tmp[11] * src[14];
    dst[11] -= tmp[4]  * src[12] + tmp[9]  * src[13] + tmp[10] * src[14];
    dst[12]  = tmp[2]  * src[10] + tmp[5]  * src[11] + tmp[1]  * src[9];
    dst[12] -= tmp[4]  * src[11] + tmp[0]  * src[9]  + tmp[3]  * src[10];
    dst[13]  = tmp[8]  * src[11] + tmp[0]  * src[8]  + tmp[7]  * src[10];
    dst[13] -= tmp[6]  * src[10] + tmp[9]  * src[11] + tmp[1]  * src[8];
    dst[14]  = tmp[6]  * src[9]  + tmp[11] * src[11] + tmp[3]  * src[8];
    dst[14] -= tmp[10] * src[11] + tmp[2]  * src[8]  + tmp[7]  * src[9];
    dst[15]  = tmp[10] * src[10] + tmp[4]  * src[8]  + tmp[9]  * src[9];
    dst[15] -= tmp[8]  * src[9]  + tmp[11] * src[10] + tmp[5]  * src[8];
    
    // calculate determinant
    det = src[0] * dst[0] + src[1] * dst[1] + src[2] * dst[2] + src[3] * dst[3];
    
    // calculate matrix inverse
    det = 1/det;
    
    for ( int j = 0; j < 16; j++ )
    {
        dst[j] *= det;
    }
}





/*
 * S = sin (a)
 * C = cos (a)
 * H = sqrt (X*X + Z*Z)
 *
 * [   v   ]  [  C  S        ]  [  1           ]  [ -Z/H  X/H    ]
 *            [ -S  C        ]  [     H  Y     ]  [     1        ]
 *            [        1     ]  [    -Y  H     ]  [ -X/H -Z/H    ]
 *            [           1  ]  [           1  ]  [           1  ]
 *
 */

void HMat33::Align( F32 a, const HVec3& vNormal )
{
    static const F32 dtor = PI / 180.0f;
    
    a *= dtor;
    
    const HF32 s( sin( a ) );
    const HF32 c( cos( a ) );
    
    const HVec3 normal( Normalise( vNormal ) );
    
    const HF32 h( Length( normal.SwizzleXZtoXY() ) );
    
    if ( h != 0.0f )
    {
        const HF32 nx( normal.X() );
        const HF32 ny( normal.Y() );
        const HF32 nz( normal.Z() );
        const HF32 rcph( HReciprocal( h ) );
        
        AX( HVec3( ( -nz * c - s * ny * nx ) * rcph,
                   s * h,
                   ( c * nx - s * ny * nz ) * rcph ) );
        
        AY( HVec3( ( nz * s - c * ny * nx ) * rcph,
                   c * h,
                   ( -s * nx - c * ny * nz ) * rcph ) );
        
        AZ( -normal );
    }
    else
    {
        AX( HVec3( s, 0.0f, -c ) );
        AY( HVec3( c, 0.0f, s ) );
        AZ( HVec3( 0.0f, -1.0f, 0.0f ) );
    }
}











