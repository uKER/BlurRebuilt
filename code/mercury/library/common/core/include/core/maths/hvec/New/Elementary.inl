#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_ELEMENTARY_INL
#define SOFT_MATHS_HVEC_NEW_ELEMENTARY_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_ELEMENTARY_INL
#define MATHS_HVEC_NEW_ELEMENTARY_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup ElementaryFunctions
/// @{

//
// VF32 Elementary Functions
//

/// VF32 Sine
NOALIAS __forceinline const VF32 Sin( const VF32 VECREF theta )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_ScalarSin( theta.m_V );
    #else
    result.m_V = sinf( theta.m_V );
    #endif
    return result;
}

/// VF32 Cosine
NOALIAS __forceinline const VF32 Cos( const VF32 VECREF theta )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_ScalarCos( theta.m_V );
    #else
    result.m_V = cosf( theta.m_V );
    #endif
    return result;
}

/// VF32 Sine & Cosine
NOALIAS __forceinline void SinCos( const VF32 VECREF theta, VF32* const __restrict pSinTheta, VF32* const __restrict pCosTheta )
{
    #if defined( _SPU_ )
    const vector float pi_div_2( Const< vector float >::PiDivTwo() );
    const vector float sincos( SIMD_VectorSin( spu_merge_xy( theta.m_V, vec_add( theta.m_V, pi_div_2 ) ) ) );
    pSinTheta->m_V = spu_splat<VX>( sincos );
    pCosTheta->m_V = spu_splat<VY>( sincos );
    #elif defined( _PPU_ )
    const vector float pi_div_2( Const< vector float >::PiDivTwo() );
    const vector float sincos( SIMD_VectorSin( vec_merge_xy( theta.m_V, vec_add( theta.m_V, pi_div_2 ) ) ) );
    pSinTheta->m_V = vec_splat( sincos, VX );
    pCosTheta->m_V = vec_splat( sincos, VY );
    #elif defined( _VMX128_ )
    const __vector4 pi_div_2( Const< __vector4 >::PiDivTwo() );
    const __vector4 sincos( SIMD_VectorSin( vmx_merge_xy( theta.m_V, __vaddfp( theta.m_V, pi_div_2 ) ) ) );
    pSinTheta->m_V = __vspltw( sincos, VX );
    pCosTheta->m_V = __vspltw( sincos, VY );
    #elif defined( _SSE_ )
    const __m128 pi_div_2( Const< __m128 >::PiDivTwo() );
    const __m128 sincos( SIMD_VectorSin( sse_merge_xy( theta.m_V, _mm_add_ps( theta.m_V, pi_div_2 ) ) ) );
    pSinTheta->m_V = sse_splat<VX>( sincos );
    pCosTheta->m_V = sse_splat<VY>( sincos );
    #else
    pSinTheta->m_V = sinf( theta.m_V );
    pCosTheta->m_V = cosf( theta.m_V );
    #endif
}

/// VF32 Sine & Cosine of 2 input angles
NOALIAS __forceinline void SinCos2( const VF32 VECREF alpha, const VF32 VECREF beta,
                                    VF32* const __restrict pSinAlpha, VF32* const __restrict pCosAlpha,
                                    VF32* const __restrict pSinBeta,  VF32* const __restrict pCosBeta )
{
    #if defined( _SPU_ )
    const vector float pi_div_2( Const< vector float >::PiDivTwo() );
    const vector float sincos( SIMD_VectorSin( spu_merge_xy( spu_merge_xy( alpha.m_V, spu_add( pi_div_2, alpha.m_V ) ),
                                                             spu_merge_xy( beta.m_V,  spu_add( pi_div_2, beta.m_V ) ) ) ) );
    pSinAlpha->m_V = spu_splat<VX>( sincos );
    pSinBeta->m_V  = spu_splat<VY>( sincos );
    pCosAlpha->m_V = spu_splat<VZ>( sincos );
    pCosBeta->m_V  = spu_splat<VW>( sincos );
    #elif defined( _PPU_ )
    const vector float pi_div_2( Const< vector float >::PiDivTwo() );
    const vector float sincos( SIMD_VectorSin( vec_merge_xy( vec_merge_xy( alpha.m_V, vec_add( pi_div_2, alpha.m_V ) ),
                                                             vec_merge_xy( beta.m_V,  vec_add( pi_div_2, beta.m_V ) ) ) ) );
    pSinAlpha->m_V = vec_splat( sincos, VX );
    pSinBeta->m_V  = vec_splat( sincos, VY );
    pCosAlpha->m_V = vec_splat( sincos, VZ );
    pCosBeta->m_V  = vec_splat( sincos, VW );
    #elif defined( _VMX128_ )
    const __vector4 pi_div_2( Const< __vector4 >::PiDivTwo() );
    const __vector4 sincos( SIMD_VectorSin( vmx_merge_xy( vmx_merge_xy( alpha.m_V, __vaddfp( pi_div_2, alpha.m_V ) ),
                                                          vmx_merge_xy( beta.m_V,  __vaddfp( pi_div_2, beta.m_V ) ) ) ) );
    pSinAlpha->m_V = __vspltw( sincos, VX );
    pSinBeta->m_V  = __vspltw( sincos, VY );
    pCosAlpha->m_V = __vspltw( sincos, VZ );
    pCosBeta->m_V  = __vspltw( sincos, VW );
    #elif defined( _SSE_ )
    const __m128 pi_div_2( Const< __m128 >::PiDivTwo() );
    const __m128 sincos( SIMD_VectorSin( sse_merge_xy( sse_merge_xy( alpha.m_V, _mm_add_ps( pi_div_2, alpha.m_V ) ),
                                                       sse_merge_xy( beta.m_V,  _mm_add_ps( pi_div_2, beta.m_V ) ) ) ) );
    pSinAlpha->m_V = sse_splat<VX>( sincos );
    pSinBeta->m_V  = sse_splat<VY>( sincos );
    pCosAlpha->m_V = sse_splat<VZ>( sincos );
    pCosBeta->m_V  = sse_splat<VW>( sincos );
    #else
    pSinAlpha->m_V = sinf( alpha.m_V );
    pCosAlpha->m_V = cosf( alpha.m_V );
    pSinBeta->m_V = sinf( beta.m_V );
    pCosBeta->m_V = cosf( beta.m_V );
    #endif
}

/// VF32 Tangent
NOALIAS __forceinline const VF32 Tan( const VF32 VECREF theta )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_ScalarTan( theta.m_V );
    #else
    result.m_V = tanf( theta.m_V );
    #endif
    return result;
}

/// VF32 Arc-Sine
NOALIAS __forceinline const VF32 ArcSin( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_ScalarArcSin( x.m_V );
    #else
    result.m_V = asinf( Clamp( x.m_V, -1.0f, 1.0f ) );
    #endif
    return result;
}

/// VF32 Arc-Cosine
NOALIAS __forceinline const VF32 ArcCos( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_ScalarArcCos( x.m_V );
    #else
    result.m_V = acosf( Clamp( x.m_V, -1.0f, 1.0f ) );
    #endif
    return result;
}

/// VF32 Arc-Tangent
NOALIAS __forceinline const VF32 ArcTan( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_ScalarArcTan( x.m_V );
    #else
    result.m_V = atanf( x.m_V );
    #endif
    return result;
}

/// VF32 Arc-Tangent
NOALIAS __forceinline const VF32 ArcTan( const VF32 VECREF x, const VF32 VECREF y )
{
    VF32 result;
    #if defined( _SIMD_ )
    const VF32 zero( Const< VF32 >::Zero() );
    const VF32 PI( Const< VF32 >::Pi() );
    result.m_V = SIMD_ScalarArcTan( SIMD_UnsafeDivide<22>( x.m_V, y.m_V ) );
    const VF32 result_inverse( result + CopySign( PI, x ) );
    result = Select( ( x == zero ) & ( y == zero ), zero,
                     Select( x >= zero, Select( y >= zero, result, result_inverse ),
                                        Select( y >  zero, result, result_inverse ) ) );
    #else
    result.m_V = atan2f( x.m_V, y.m_V );
    #endif
    return result;
}

/// VF32 Natural Logarithm ( Base e )
NOALIAS __forceinline const VF32 Log( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_ScalarLog( x.m_V );
    #else
    result.m_V = logf( x.m_V );
    #endif
    return result;
}

/// VF32 Logarithm Base N
/// NOTE: It's more efficient to multiply Log(x) by a pre-computed 1/Log(n) for a large number of calculations with the same base!
NOALIAS __forceinline const VF32 Log( const VF32 VECREF n, const VF32 VECREF x )
{
    return Log( x ) / Log( n );
}

/// VF32 Exponential e ^ z
NOALIAS __forceinline const VF32 Exp( const VF32 VECREF x )
{
    VF32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_ScalarExp( x.m_V );
    #else
    result.m_V = expf( x.m_V );
    #endif
    return result;
}

/// VF32 Power x ^ y
/// WARNING: This implementation gives UNDEFINED results for negative x!
/// (If you need a power function that handles negative integer x correctly then let me know! JASOND)
NOALIAS __forceinline const VF32 Pow( const VF32 VECREF x, const VF32 VECREF y )
{
    // http://functions.wolfram.com/ElementaryFunctions/Power/27/02/0001/
    // z^a = e^(a.Log(z))
    
    const VF32 zero( Const< VF32 >::Zero() );
    const VF32 one( Const< VF32 >::One() );
    
    //if ( ( x < zero ) && ( Round( y ) == y ) )
    //  ????
    //else
        return Select( y == zero, one,
               Select( x == zero, zero,
               Select( y == one, x,
               Exp( y * Log( x ) ) ) ) );
}




//
// Vec4F32 Elementary Functions
//

/// Vec4F32 Sine
NOALIAS __forceinline const Vec4F32 Sin( const Vec4F32 VECREF theta )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_VectorSin( theta.m_V );
    #else
    result.m_V[VX] = sinf( theta.m_V[VX] );
    result.m_V[VY] = sinf( theta.m_V[VY] );
    result.m_V[VZ] = sinf( theta.m_V[VZ] );
    result.m_V[VW] = sinf( theta.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Cosine
NOALIAS __forceinline const Vec4F32 Cos( const Vec4F32 VECREF theta )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_VectorCos( theta.m_V );
    #else
    result.m_V[VX] = cosf( theta.m_V[VX] );
    result.m_V[VY] = cosf( theta.m_V[VY] );
    result.m_V[VZ] = cosf( theta.m_V[VZ] );
    result.m_V[VW] = cosf( theta.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Tangent
NOALIAS __forceinline const Vec4F32 Tan( const Vec4F32 VECREF theta )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_VectorTan( theta.m_V );
    #else
    result.m_V[VX] = tanf( theta.m_V[VX] );
    result.m_V[VY] = tanf( theta.m_V[VY] );
    result.m_V[VZ] = tanf( theta.m_V[VZ] );
    result.m_V[VW] = tanf( theta.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Sine & Cosine : returns Vec4F32 with elements x=sin(theta.x), y=cos(theta.y), z=sin(theta.z), w=cos(theta.w)
NOALIAS __forceinline const Vec4F32 SinCos( const Vec4F32 VECREF theta )
{
    //return Sin( Select( MakeVec4Bool<1,0,1,0>(), theta, Const< Vec4F32 >::PiDiv2() - theta ) );
    return Sin( Select( MakeVec4Bool<1,0,1,0>(), theta, theta + Const< Vec4F32 >::PiDiv2() ) );
}

/// Vec4F32 Arc-Sine
NOALIAS __forceinline const Vec4F32 ArcSin( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_VectorArcSin( x.m_V );
    #else
    result.m_V[VX] = asinf( Clamp( x.m_V[VX], -1.0f, 1.0f ) );
    result.m_V[VY] = asinf( Clamp( x.m_V[VY], -1.0f, 1.0f ) );
    result.m_V[VZ] = asinf( Clamp( x.m_V[VZ], -1.0f, 1.0f ) );
    result.m_V[VW] = asinf( Clamp( x.m_V[VW], -1.0f, 1.0f ) );
    #endif
    return result;
}

/// Vec4F32 Arc-Cosine
NOALIAS __forceinline const Vec4F32 ArcCos( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_VectorArcCos( x.m_V );
    #else
    result.m_V[VX] = acosf( Clamp( x.m_V[VX], -1.0f, 1.0f ) );
    result.m_V[VY] = acosf( Clamp( x.m_V[VY], -1.0f, 1.0f ) );
    result.m_V[VZ] = acosf( Clamp( x.m_V[VZ], -1.0f, 1.0f ) );
    result.m_V[VW] = acosf( Clamp( x.m_V[VW], -1.0f, 1.0f ) );
    #endif
    return result;
}

/// Vec4F32 Arc-Tangent
NOALIAS __forceinline const Vec4F32 ArcTan( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_VectorArcTan( x.m_V );
    #else
    result.m_V[VX] = atanf( x.m_V[VX] );
    result.m_V[VY] = atanf( x.m_V[VY] );
    result.m_V[VZ] = atanf( x.m_V[VZ] );
    result.m_V[VW] = atanf( x.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Arc-Tangent
NOALIAS __forceinline const Vec4F32 ArcTan( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    const Vec4F32 zero( Const< Vec4F32 >::Zero() );
    const Vec4F32 PI( Const< Vec4F32 >::Pi() );
    result.m_V = SIMD_VectorArcTan( SIMD_UnsafeDivide<22>( x.m_V, y.m_V ) );
    const Vec4F32 result_inverse( result + CopySign( PI, x ) );
    result = Select( ( x == zero ) & ( y == zero ), zero,
                     Select( x >= zero, Select( y >= zero, result, result_inverse ),
                                        Select( y >  zero, result, result_inverse ) ) );
    #else
    result.m_V[VX] = atan2f( x.m_V[VX], y.m_V[VX] );
    result.m_V[VY] = atan2f( x.m_V[VY], y.m_V[VY] );
    result.m_V[VZ] = atan2f( x.m_V[VZ], y.m_V[VZ] );
    result.m_V[VW] = atan2f( x.m_V[VW], y.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Natural Logarithm
NOALIAS __forceinline const Vec4F32 Log( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_VectorLog( x.m_V );
    #else
    result.m_V[VX] = logf( x.m_V[VX] );
    result.m_V[VY] = logf( x.m_V[VY] );
    result.m_V[VZ] = logf( x.m_V[VZ] );
    result.m_V[VW] = logf( x.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Logarithm Base N
/// NOTE: It's more efficient to multiply Log(x) by a pre-computed 1/Log(n) for a large number of calculations with the same base!
NOALIAS __forceinline const Vec4F32 Log( const Vec4F32 VECREF n, const Vec4F32 VECREF x )
{
    return Log( x ) / Log( n );
}

/// Vec4F32 Exponential e ^ z
NOALIAS __forceinline const Vec4F32 Exp( const Vec4F32 VECREF x )
{
    Vec4F32 result;
    #if defined( _SIMD_ )
    result.m_V = SIMD_VectorExp( x.m_V );
    #else
    result.m_V[VX] = expf( x.m_V[VX] );
    result.m_V[VY] = expf( x.m_V[VY] );
    result.m_V[VZ] = expf( x.m_V[VZ] );
    result.m_V[VW] = expf( x.m_V[VW] );
    #endif
    return result;
}

/// Vec4F32 Power x ^ y
/// WARNING: This implementation gives UNDEFINED results for negative x!
/// (If you need a power function that handles negative integer x correctly then let me know! JASOND)
NOALIAS __forceinline const Vec4F32 Pow( const Vec4F32 VECREF x, const Vec4F32 VECREF y )
{
    // http://functions.wolfram.com/ElementaryFunctions/Power/27/02/0001/
    // z^a = e^(a.Log(z))
    
    const Vec4F32 zero( Const< Vec4F32 >::Zero() );
    const Vec4F32 one( Const< Vec4F32 >::One() );
    
    //if ( ( x < zero ) && ( Round( y ) == y ) )
    //  ????
    //else
        return Select( y == zero, one,
               Select( x == zero, zero,
               Select( y == one, x,
               Exp( y * Log( x ) ) ) ) );
}






//
// Vector3 Elementary Functions
//

/// Vector3 Sine
NOALIAS __forceinline const Vector3 Sin( const Vector3 PCREF theta )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = Sin( theta.m_V );
    #else
    result.m_V.m_V[VX] = sinf( theta.m_V.m_V[VX] );
    result.m_V.m_V[VY] = sinf( theta.m_V.m_V[VY] );
    result.m_V.m_V[VZ] = sinf( theta.m_V.m_V[VZ] );
    #endif
    return result;
}

/// Vector3 Cosine
NOALIAS __forceinline const Vector3 Cos( const Vector3 PCREF theta )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = Cos( theta.m_V );
    #else
    result.m_V.m_V[VX] = cosf( theta.m_V.m_V[VX] );
    result.m_V.m_V[VY] = cosf( theta.m_V.m_V[VY] );
    result.m_V.m_V[VZ] = cosf( theta.m_V.m_V[VZ] );
    #endif
    return result;
}

/// Vector3 Tangent
NOALIAS __forceinline const Vector3 Tan( const Vector3 PCREF theta )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = Tan( theta.m_V );
    #else
    result.m_V.m_V[VX] = tanf( theta.m_V.m_V[VX] );
    result.m_V.m_V[VY] = tanf( theta.m_V.m_V[VY] );
    result.m_V.m_V[VZ] = tanf( theta.m_V.m_V[VZ] );
    #endif
    return result;
}

/// Vector3 Arc-Sine
NOALIAS __forceinline const Vector3 ArcSin( const Vector3 PCREF x )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = ArcSin( x.m_V );
    #else
    result.m_V.m_V[VX] = asinf( Clamp( x.m_V.m_V[VX], -1.0f, 1.0f ) );
    result.m_V.m_V[VY] = asinf( Clamp( x.m_V.m_V[VY], -1.0f, 1.0f ) );
    result.m_V.m_V[VZ] = asinf( Clamp( x.m_V.m_V[VZ], -1.0f, 1.0f ) );
    #endif
    return result;
}

/// Vector3 Arc-Cosine
NOALIAS __forceinline const Vector3 ArcCos( const Vector3 PCREF x )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = ArcCos( x.m_V );
    #else
    result.m_V.m_V[VX] = acosf( Clamp( x.m_V.m_V[VX], -1.0f, 1.0f ) );
    result.m_V.m_V[VY] = acosf( Clamp( x.m_V.m_V[VY], -1.0f, 1.0f ) );
    result.m_V.m_V[VZ] = acosf( Clamp( x.m_V.m_V[VZ], -1.0f, 1.0f ) );
    #endif
    return result;
}

/// Vector3 Arc-Tangent
NOALIAS __forceinline const Vector3 ArcTan( const Vector3 PCREF x )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = ArcTan( x.m_V );
    #else
    result.m_V.m_V[VX] = atanf( x.m_V.m_V[VX] );
    result.m_V.m_V[VY] = atanf( x.m_V.m_V[VY] );
    result.m_V.m_V[VZ] = atanf( x.m_V.m_V[VZ] );
    #endif
    return result;
}

/// Vector3 Arc-Tangent
NOALIAS __forceinline const Vector3 ArcTan( const Vector3 PCREF x, const Vector3 PCREF y )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = ArcTan( x.m_V, y.m_V );
    #else
    result.m_V.m_V[VX] = atan2f( x.m_V.m_V[VX], y.m_V.m_V[VX] );
    result.m_V.m_V[VY] = atan2f( x.m_V.m_V[VY], y.m_V.m_V[VY] );
    result.m_V.m_V[VZ] = atan2f( x.m_V.m_V[VZ], y.m_V.m_V[VZ] );
    #endif
    return result;
}

/// Vector3 Natural Logarithm
NOALIAS __forceinline const Vector3 Log( const Vector3 PCREF x )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = Log( x.m_V );
    #else
    result.m_V.m_V[VX] = logf( x.m_V.m_V[VX] );
    result.m_V.m_V[VY] = logf( x.m_V.m_V[VY] );
    result.m_V.m_V[VZ] = logf( x.m_V.m_V[VZ] );
    #endif
    return result;
}

/// Vector3 Logarithm Base N
/// NOTE: It's more efficient to multiply Log(x) by a pre-computed 1/Log(n) for a large number of calculations with the same base!
NOALIAS __forceinline const Vector3 Log( const Vector3 PCREF n, const Vector3 PCREF x )
{
    return Log( x ) / Log( n );
}

/// Vector3 Exponential e ^ z
NOALIAS __forceinline const Vector3 Exp( const Vector3 PCREF x )
{
    Vector3 result;
    #if defined( _SIMD_ )
    result.m_V = Exp( x.m_V );
    #else
    result.m_V.m_V[VX] = expf( x.m_V.m_V[VX] );
    result.m_V.m_V[VY] = expf( x.m_V.m_V[VY] );
    result.m_V.m_V[VZ] = expf( x.m_V.m_V[VZ] );
    #endif
    return result;
}

/// Vector3 Power x ^ y
/// WARNING: This implementation gives UNDEFINED results for negative x!
/// (If you need a power function that handles negative integer x correctly then let me know! JASOND)
NOALIAS __forceinline const Vector3 Pow( const Vector3 PCREF x, const Vector3 PCREF y )
{
    Vector3 result;
    result.m_V = Pow( x.m_V, y.m_V );
    return result;
}





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

