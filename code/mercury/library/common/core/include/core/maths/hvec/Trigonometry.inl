#ifndef MATHS_TRIGONOMETRY_INL
#define MATHS_TRIGONOMETRY_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

template < typename T > NOALIAS __forceinline const T PowerMinusOne( const int K ) __attribute__(( const, always_inline ));
template < typename T > NOALIAS __forceinline const T PowerMinusOne( const int K )
{
    if ( K & 1 )
        return (T)-1.0;
    else
        return (T)1.0;
}

//template < typename T > NOALIAS /*__force*/inline const T Power( const int X, const int Y )
template < typename T > NOALIAS __forceinline const T Power( const int X, const int Y ) __attribute__(( const /*, always_inline */ ));
template < typename T > NOALIAS __forceinline const T Power( const int X, const int Y )
{
    if ( X == 0 )
    {
        if ( Y == 0 )
            return (T)1.0;
        else
            return (T)0.0;
    }
    else if ( Y > 0 )
        return Power<T>( X, Y-1 ) * (T)X;
    else if ( Y < 0 )
        return Power<T>( X, Y+1 ) / (T)X;
    else
        return (T)1.0; // X ^ 0 == 1
}

//template < typename T > NOALIAS /*__force*/inline const T Factorial( const int N )
template < typename T > NOALIAS __forceinline const T Factorial( const int N ) __attribute__(( const /*, always_inline */ ));
template < typename T > NOALIAS __forceinline const T Factorial( const int N )
{
    if ( N > 1 )
        return Factorial<T>( N-1 ) * (T)N;
    else
        return (T)1.0;
}

//template < typename T > NOALIAS /*__force*/inline const T Pochhammer( const T a, const int N )
template < typename T > NOALIAS __forceinline const T Pochhammer( const T a, const int N ) __attribute__(( const /*, always_inline */ ));
template < typename T > NOALIAS __forceinline const T Pochhammer( const T a, const int N )
{
    if ( N > 0 )
        return Pochhammer<T>( a, N-1 ) * ( a + (T)(N-1) );
    else
        return (T)1.0;
}

//template < typename T > NOALIAS /*__force*/inline const T Binomial( const int N, const int K )
template < typename T > NOALIAS __forceinline const T Binomial( const int N, const int K ) __attribute__(( const, always_inline ));
template < typename T > NOALIAS __forceinline const T Binomial( const int N, const int K )
{
    //const T zero( 0.0 );
    //const T one( 1.0 );
    //if ( ( K <= N ) && ( N < 0 ) ) return zero;
    ////else if ( K < 0 )    return zero;
    ////else if ( K == 0 )   return one;
    ////else if ( K == 1 )   return (T)N;
    ////else if ( N == K )   return one;
    ////else if ( N+1 == K ) return zero;
    ////else if ( N <= K )   return zero;
    //else                 return Factorial<T>( N ) / ( Factorial<T>( K ) * Factorial<T>( N - K ) );
    if ( ( N >= K ) && ( K >= 0 ) )
        return Factorial<T>( N ) / ( Factorial<T>( K ) * Factorial<T>( N - K ) );
    else
        return (T)0.0;
}

//template < typename T > NOALIAS __forceinline const T Bernoulli_Sum2( const int J, const int K, const int N )
//{
//    const T zero( 0.0 );
//    
//    if ( J <= K )
//        return ( PowerMinusOne<T>( J ) * Binomial<T>( K, J ) * Power<T>( J, N ) ) + Bernoulli_Sum2<T>( J+1, K, N );
//    else
//        return zero;
//}

//template < typename T > NOALIAS __forceinline const T Bernoulli_Sum1( const int K, const int N )
//{
//    const T zero( 0.0 );
//    const T one( 1.0 );
//    
//    if ( K <= N )
//        return ( ( one / ( (T)K + one ) ) * Bernoulli_Sum2<T>( 0, K, N ) ) + Bernoulli_Sum1<T>( K+1, N );
//    else
//        return zero;
//}

//template < typename T > NOALIAS __forceinline const T Bernoulli_Sum2( const int J, const int K, const int N )
//{
//    const T zero( 0.0 );
//    
//    if ( J <= K )
//        return ( PowerMinusOne<T>( J ) * Power<T>( J, N ) * Binomial<T>( N+1, K-J ) / Binomial<T>( N, K ) ) + Bernoulli_Sum2<T>( J+1, K, N );
//    else
//        return zero;
//}

//template < typename T > NOALIAS __forceinline const T Bernoulli_Sum1( const int K, const int N )
//{
//    const T zero( 0.0 );
//    
//    if ( K <= N )
//        return Bernoulli_Sum2<T>( 1, K, N ) + Bernoulli_Sum1<T>( K+1, N );
//    else
//        return zero;
//}



//template < typename T > NOALIAS /*__force*/inline const T Bernoulli( const int N )
template < typename T > NOALIAS __forceinline const T Bernoulli( const int N ) __attribute__(( const, always_inline ));
template < typename T > NOALIAS __forceinline const T Bernoulli( const int N )
{
    // BUG! Both calculation methods result in failure to calculate B(14) onwards!?!?
    //const T zero( 0.0 );
    //const T one( 1.0 );
    //
    //if ( ( N >= 3 ) && ( N & 1 ) )
    //    return zero; // B(2n+1) = 0
    //else
    //{
    //    if ( N >= 0 )
    //        //return Bernoulli_Sum1<T>( 0, N );
    //        return ( one / ( (T)N + one ) ) * Bernoulli_Sum1<T>( 1, N );
    //    else
    //        return zero;
    //}
    
    if ( ( N >= 0 ) && ( N <= 31 ) )
        return g_Const_BernoulliNumber[N];
    else
        return (T)0.0;
}



#if defined( _SIMD_ )

/// SIMD scalar Cos: Using power series expansion
NOALIAS __forceinline const V128 SIMD_ScalarArcCos( const V128 PCREF x )
{
    #if defined( _SPU_ )
    return spu_sub( Const< vector float >::PiDivTwo(), SIMD_ScalarArcSin( x ) );
    #elif defined( _PPU_ )
    return vec_sub( Const< vector float >::PiDivTwo(), SIMD_ScalarArcSin( x ) );
    #elif defined( _VMX128_ )
    return __vsubfp( Const< __vector4 >::PiDivTwo(), SIMD_ScalarArcSin( x ) );
    #elif defined( _SSE_ )
    return _mm_sub_ps( Const< __m128 >::PiDivTwo(), SIMD_ScalarArcSin( x ) );
    #endif
}

/// SIMD vector Cos: Using power series expansion
NOALIAS __forceinline const V128 SIMD_VectorArcCos( const V128 PCREF x )
{
    #if defined( _SPU_ )
    return spu_sub( Const< vector float >::PiDivTwo(), SIMD_VectorArcSin( x ) );
    #elif defined( _PPU_ )
    return vec_sub( Const< vector float >::PiDivTwo(), SIMD_VectorArcSin( x ) );
    #elif defined( _VMX128_ )
    return __vsubfp( Const< __vector4 >::PiDivTwo(), SIMD_VectorArcSin( x ) );
    #elif defined( _SSE_ )
    return _mm_sub_ps( Const< __m128 >::PiDivTwo(), SIMD_VectorArcSin( x ) );
    #endif
}

#endif






/// HF32 Calculate Sin( theta ) from Cos( theta ), Using fundamental identity: Sin( theta ) = Sqrt( 1 - Cos^2( theta ) )
NOALIAS __forceinline const HF32 CosTheta_To_SinTheta( const HF32 PCREF cosTheta )
{
    const HF32 one( c_ScalarOne );
    return Sqrt( one - ( cosTheta * cosTheta ) );
}


/// HF32 Calculate Cos( theta ) from Sin( theta ), Using fundamental identity: Cos( theta ) = Sqrt( 1 - Sin^2( theta ) )
NOALIAS __forceinline const HF32 SinTheta_To_CosTheta( const HF32 PCREF sinTheta )
{
    const HF32 one( c_ScalarOne );
    return Sqrt( one - ( sinTheta * sinTheta ) );
}


/// HF32 Calculate Sin( theta ) from Tan( theta ), using Fundamental identity: 1 + Cot^2( theta ) = Csc^2( theta ), Sin( theta ) = 1 / Sqrt( 1 + ( 1 / ( Tan^2( theta ) ) ) )
NOALIAS __forceinline const HF32 TanTheta_To_SinTheta( const HF32 PCREF tanTheta )
{
    const HF32 one( c_ScalarOne );
    return ReciprocalSqrt( one + Reciprocal( tanTheta * tanTheta ) );
}


/// HF32 Calculate Cos( theta ) from Tan( theta ), using Fundamental identity: 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta )
NOALIAS __forceinline const HF32 TanTheta_To_CosTheta( const HF32 PCREF tanTheta )
{
    const HF32 one( c_ScalarOne );
    return ReciprocalSqrt( one + ( tanTheta * tanTheta ) );
}


/// HF32 Calculate Sin( theta / 2 ) from Cos( theta ), using Half-angle formula: Sin^2( theta / 2 ) = ( 1 - Cos( theta ) ) / 2
NOALIAS __forceinline const HF32 CosTheta_To_SinHalfTheta( const HF32 PCREF cosTheta )
{
    const HF32 half( c_ScalarHalf );
    const HF32 one( c_ScalarOne );
    return Sqrt( ( one - cosTheta ) * half );
}

/// HF32 calculate Tan( theta / 2 ) from Cos( theta ), using Half-angle formulas: Tan^2( theta ) = Sin^2( theta ) / Cos^2( theta ) = ( 1 - Cos( 2 * theta ) ) / ( 1 + Cos( 2 * theta ) )
NOALIAS __forceinline const HF32 CosTheta_To_TanHalfTheta( const HF32 PCREF cosTheta )
{
    const HF32 one( c_ScalarOne );
    return Sqrt( ( one - cosTheta ) * Reciprocal( one + cosTheta ) );
}


/// HF32 Calculate Tan( 2 * theta ) from Tan( theta ), using Double-angle formula: Tan( 2 * theta ) = 2 * Tan( theta ) / ( 1 - ( Tan^2( theta ) )
NOALIAS __forceinline const HF32 TanTheta_To_TanTwoTheta( const HF32 PCREF tanTheta )
{
    const HF32 one( c_ScalarOne );
    const HF32 two( c_ScalarTwo );
    return two * tanTheta * Reciprocal( one - ( tanTheta * tanTheta ) );
}




/// HF32 Cos: Using power series expansion
NOALIAS __forceinline const HF32 Cos( const HF32 PCREF theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Cos
    //
    // Cos(z) = Sin( PI/2 - z )
    
    const HF32 halfPI( g_Const_PI[ E_PI_CONSTANTS_PI_DIV_TWO ] );
    
    return Sin( halfPI - theta );
}


/// HF32 ArcCos: Using power series expansion
NOALIAS __forceinline const HF32 ArcCos( const HF32 PCREF cosTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcCos
    //
    // ArcCos(z) == PI/2 - ArcSin(z)
    
    const HF32 halfPI( g_Const_PI[ E_PI_CONSTANTS_PI_DIV_TWO ] );
    
    return halfPI - ArcSin( cosTheta );
}


/// HF32 ArcTan: Using power series expansion
NOALIAS __forceinline const HF32 ArcTan( const HF32 PCREF y, const HF32 PCREF x )
{
    //const HF32 tanTheta( y / x );
    const HF32 tanTheta( UnsafeDivide( y, x ) );
    const HF32 theta_x_positive( ArcTan( tanTheta ) );
    const HF32 zero(ZERO_INIT);
    const HF32 PI( g_Const_PI[ E_PI_CONSTANTS_PI ] );
    const HF32 theta_x_negative( CopySign( PI, y ) + theta_x_positive );
    return Select( x.GE( zero ), theta_x_positive, theta_x_negative );
}




/// HVec4 Calculate Sine from Cosine, Using fundamental identity: Sin(theta) = Sqrt( 1 - Cos^2(theta) )
NOALIAS __forceinline const HVec4 CosTheta_To_SinTheta( const HVec4 PCREF cosTheta )
{
    const HVec4 one( c_VectorOne );
    return Sqrt( one - ( cosTheta * cosTheta ) );
}


/// HVec4 Calculate Cosine from Sine, Using fundamental identity: Cos(theta) = Sqrt( 1 - Sin^2(theta) )
NOALIAS __forceinline const HVec4 SinTheta_To_CosTheta( const HVec4 PCREF sinTheta )
{
    const HVec4 one( c_VectorOne );
    return Sqrt( one - ( sinTheta * sinTheta ) );
}








/// HVec4 Cos: Using power series expansion
NOALIAS __forceinline const HVec4 Cos( const HVec4 PCREF theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Cos
    //
    // Cos(z) = Sin( PI/2 - z )
    
    const HF32 halfPI( g_Const_PI[ E_PI_CONSTANTS_PI_DIV_TWO ] );
    
    return Sin( HVec4( halfPI ) - theta );
}


/// HVec4 ArcCos: Using power series expansion
NOALIAS __forceinline const HVec4 ArcCos( const HVec4 PCREF z )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcCos
    //
    // ArcCos(z) == PI/2 - ArcSin(z)
    
    const HF32 halfPI( g_Const_PI[ E_PI_CONSTANTS_PI_DIV_TWO ] );
    
    return HVec4( halfPI ) - ArcSin( z );
}















//
// --------------------------
// | *** WALTER'S HACKS *** |
// --------------------------
//

/// HF32 ArcSin: Using power series expansion
NOALIAS FORCE_INLINE const HF32 ArcSin_Inline( const HF32 PCREF sinTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcSin
    //
    // ArcSin(z) == Sum[k=0,inf] { z^(2k+1) . Pochhammer( 0.5, k ) / (2k+1)k! }
    
    const HF32 half( c_ScalarHalf );
    const HF32 one( c_ScalarOne );
    
    // Fundamental identity: Cos( theta ) = Sqrt( 1 - Sin^2( theta ) )
    
    const HF32 cosTheta( Sqrt( one - ( sinTheta * sinTheta ) ) );
    
    // Half-angle formula: Sin^2( theta ) = ( 1 - Cos( 2 * theta ) ) / 2
    
    const HF32 sinHalfTheta( CopySign( Sqrt( ( one - cosTheta ) * half ), sinTheta ) );
    
    const HF32 z1( sinHalfTheta );
    const HF32 z2( z1 * z1 );
    const HF32 z3( z2 * z1 );
    const HF32 z4( z2 * z2 );
    const HF32 z5( z3 * z2 );
    const HF32 z6( z3 * z3 );
    const HF32 z7( z4 * z3 );
    const HF32 z8( z4 * z4 );
    const HF32 z9( z5 * z4 );
    const HF32 z10( z5 * z5 );
    const HF32 z11( z6 * z5 );
    const HF32 z12( z6 * z6 );
    const HF32 z13( z7 * z6 );
    const HF32 z14( z7 * z7 );
    const HF32 z15( z8 * z7 );
    const HF32 z16( z8 * z8 );
    const HF32 z17( z9 * z8 );
//  const HF32 z18( z9 * z9 );
    const HF32 z19( z10 * z9 );
//  const HF32 z20( z10 * z10 );
    const HF32 z21( z11 * z10 );
//  const HF32 z22( z11 * z11 );
    const HF32 z23( z12 * z11 );
//  const HF32 z24( z12 * z12 );
    const HF32 z25( z13 * z12 );
//  const HF32 z26( z13 * z13 );
    const HF32 z27( z14 * z13 );
//  const HF32 z28( z14 * z14 );
    const HF32 z29( z15 * z14 );
//  const HF32 z30( z15 * z15 );
    const HF32 z31( z16 * z15 );
    
    HVec4 const * const __restrict p_Const_ArcSinSeries = (HVec4*)(void*)g_Const_ArcSinSeries;
    const HVec4 k00_03( p_Const_ArcSinSeries[0] );
    const HVec4 k04_07( p_Const_ArcSinSeries[1] );
    const HVec4 k08_11( p_Const_ArcSinSeries[2] );
    const HVec4 k12_15( p_Const_ArcSinSeries[3] );
    
    const HF32 halfTheta( ( MultiplySum4( HVec4( z1,  z3,  z5,  z7  ), k00_03 ) +
                            MultiplySum4( HVec4( z9,  z11, z13, z15 ), k04_07 ) ) +
                          ( MultiplySum4( HVec4( z17, z19, z21, z23 ), k08_11 ) +
                            MultiplySum4( HVec4( z25, z27, z29, z31 ), k12_15 ) ) );
    
    const HF32 two( c_ScalarTwo );
    
    // TODO: Store muliply by two in the constants table!
    return halfTheta * two;
}

/// HF32 ArcTan: Using power series expansion
NOALIAS FORCE_INLINE const HF32 ArcTan_Inline( const HF32 PCREF tanTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcTan
    //
    // ArcTan(z) ==                  Sum[k=0,inf] { (-1)^k . z^(2k+1) / (2k+1) } : |z| < 1
    // ArcTan(z) == PI/2 . Sign(z) - Sum[k=0,inf] { (-1)^k / z^(2k+1) / (2k+1) } : |z| > 1
    
    const HF32 zero(ZERO_INIT);
    const HF32 one( c_ScalarOne );
    const HF32 fltMax( c_ScalarFltMax );
    
    // Fundamental identity: 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta )
    
    const HF32 cosTheta( ReciprocalSqrt( one + ( tanTheta * tanTheta ) ) );
    
    // Half-angle formulas: Tan^2( theta ) = Sin^2( theta ) / Cos^2( theta ) = ( 1 - Cos( 2 * theta ) ) / ( 1 + Cos( 2 * theta ) )
    
    const HF32 tanHalfTheta( Sqrt( ( one - cosTheta ) * Reciprocal( one + cosTheta ) ) );
    
    HVec4 const * const __restrict p_Const_ArcTanSeries = (HVec4*)(void*)g_Const_ArcTanSeries;
    const HVec4 k00_03( p_Const_ArcTanSeries[0] );
    const HVec4 k04_07( p_Const_ArcTanSeries[1] );
    const HVec4 k08_11( p_Const_ArcTanSeries[2] );
    const HVec4 k12_15( p_Const_ArcTanSeries[3] );
    const HVec4 k16_19( p_Const_ArcTanSeries[4] );
    const HVec4 k20_23( p_Const_ArcTanSeries[5] );
    const HVec4 k24_27( p_Const_ArcTanSeries[6] );
    const HVec4 k28_31( p_Const_ArcTanSeries[7] );
    
    const HF32 z1( tanHalfTheta );
    const HF32 z2( z1 * z1 );
    const HF32 z3( z2 * z1 );
    const HF32 z4( z2 * z2 );
    const HF32 z5( z3 * z2 );
    const HF32 z6( z3 * z3 );
    const HF32 z7( z4 * z3 );
    const HF32 z8( z4 * z4 );
    const HF32 z9( z5 * z4 );
    const HF32 z10( z5 * z5 );
    const HF32 z11( z6 * z5 );
    const HF32 z12( z6 * z6 );
    const HF32 z13( z7 * z6 );
    const HF32 z14( z7 * z7 );
    const HF32 z15( z8 * z7 );
    const HF32 z16( z8 * z8 );
    const HF32 z17( z9 * z8 );
    const HF32 z18( z9 * z9 );
    const HF32 z19( z10 * z9 );
    const HF32 z20( z10 * z10 );
    const HF32 z21( z11 * z10 );
    const HF32 z22( z11 * z11 );
    const HF32 z23( z12 * z11 );
    const HF32 z24( z12 * z12 );
    const HF32 z25( z13 * z12 );
    const HF32 z26( z13 * z13 );
    const HF32 z27( z14 * z13 );
    const HF32 z28( z14 * z14 );
    const HF32 z29( z15 * z14 );
    const HF32 z30( z15 * z15 );
    const HF32 z31( z16 * z15 );
    const HF32 z32( z16 * z16 );
    const HF32 z33( z17 * z16 );
//  const HF32 z34( z17 * z17 );
    const HF32 z35( z18 * z17 );
//  const HF32 z36( z18 * z18 );
    const HF32 z37( z19 * z18 );
//  const HF32 z38( z19 * z19 );
    const HF32 z39( z20 * z19 );
//  const HF32 z40( z20 * z20 );
    const HF32 z41( z21 * z20 );
//  const HF32 z42( z21 * z21 );
    const HF32 z43( z22 * z21 );
//  const HF32 z44( z22 * z22 );
    const HF32 z45( z23 * z22 );
//  const HF32 z46( z23 * z23 );
    const HF32 z47( z24 * z23 );
//  const HF32 z48( z24 * z24 );
    const HF32 z49( z25 * z24 );
//  const HF32 z50( z25 * z25 );
    const HF32 z51( z26 * z25 );
//  const HF32 z52( z26 * z26 );
    const HF32 z53( z27 * z26 );
//  const HF32 z54( z27 * z27 );
    const HF32 z55( z28 * z27 );
//  const HF32 z56( z28 * z28 );
    const HF32 z57( z29 * z28 );
//  const HF32 z58( z29 * z29 );
    const HF32 z59( z30 * z29 );
//  const HF32 z60( z30 * z30 );
    const HF32 z61( z31 * z30 );
//  const HF32 z62( z31 * z31 );
    const HF32 z63( z32 * z31 );
    
    const HF32 halfTheta( ( ( MultiplySum4( HVec4( z1,  z3,  z5,  z7  ), k00_03 ) +
                              MultiplySum4( HVec4( z9,  z11, z13, z15 ), k04_07 ) ) +
                            ( MultiplySum4( HVec4( z17, z19, z21, z23 ), k08_11 ) +
                              MultiplySum4( HVec4( z25, z27, z29, z31 ), k12_15 ) ) ) +
                          ( ( MultiplySum4( HVec4( z33, z35, z37, z39 ), k16_19 ) +
                              MultiplySum4( HVec4( z41, z43, z45, z47 ), k20_23 ) ) +
                            ( MultiplySum4( HVec4( z49, z51, z53, z55 ), k24_27 ) +
                              MultiplySum4( HVec4( z57, z59, z61, z63 ), k28_31 ) ) ) );
    
    const HF32 two( c_ScalarTwo );
    
    // TODO: Store muliply by two in the constants table!
    return CopySign( halfTheta * two, tanTheta );
}

/// HF32 ArcTan: Using power series expansion
NOALIAS __forceinline const HF32 ArcTan_Inline( const HF32 PCREF y, const HF32 PCREF x )
{
    //const HF32 tanTheta( y / x  );
    const HF32 tanTheta( UnsafeDivide( y, x ) );
    const HF32 theta_x_positive( ArcTan_Inline( tanTheta ) );
    const HF32 zero(ZERO_INIT);
    const HF32 PI( g_Const_PI[ E_PI_CONSTANTS_PI ] );
    const HF32 theta_x_negative( CopySign( PI, y ) + theta_x_positive );
    return Select( x.GE( zero ), theta_x_positive, theta_x_negative );
}




}; // namespace Maths



#endif




