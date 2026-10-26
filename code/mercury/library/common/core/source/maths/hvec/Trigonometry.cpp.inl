
#include <core/maths/hvec/New/IncludeMinimal.h>
#include <core/maths/hvec/New/IncludeMinimal.inl>

namespace Maths
{

ALIGN16 const F32 g_Const_PI[8] =
{
    3.1415926535897932384626433832795f,  // PI
    6.283185307179586476925286766559f,   // 2.0 * PI
    0.31830988618379067153776752674503f, // 1.0 / PI
    0.15915494309189533576888376337251f, // 2.0 / PI
    1.5707963267948966192313216916398f,  // PI / 2.0
    0.78539816339744830961566084581988f, // PI / 4.0
    0.70710678118654752440084436210485f, // sin( PI / 4.0f )
    0.0f
};

extern ALIGN16 const F32 g_Const_BernoulliNumber[];
extern ALIGN16 const F32 g_Const_SinSeries[];
extern ALIGN16 const F32 g_Const_CosSeries[];
extern ALIGN16 const F32 g_Const_TanSeries[];
extern ALIGN16 const F32 g_Const_ArcSinSeries[];
extern ALIGN16 const F32 g_Const_ArcTanSeries[];
extern ALIGN16 const F32 g_Const_LogSeries[];
extern ALIGN16 const F32 g_Const_ExpSeries[];
extern ALIGN16 const F32 g_Const_ExpTable[];

#if defined( _SIMD_ )

/// SIMD Scalar Sin: Using power series expansion
NOALIAS const V128 SIMD_ScalarSin( const V128 PCREF in_theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Sin
    //
    // Sin(z) == Sum[k=0,inf] { (-1)^k . z^(2k+1) / (2k+1)! }
    // Sin(z) = -Sin(-z)
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    
    #if defined( _SPU_ )
    
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float PI( Const< vector float >::Pi() );
    const vector float PI_DivTwo( Const< vector float >::PiDivTwo() );
    const vector float TwoPI( Const< vector float >::TwoPi() );
    const vector float rcp_TwoPI( Const< vector float >::RcpTwoPi() );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const vector float theta( spu_nmsub( spu_rfim( spu_mul( spu_add( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    // --> Sin( z - PI ) = -Sin(z), for m = -1
    // --> Sin(z) = -Sin( z - PI )
    // --> Sin(z) = Sin( PI - z ), using Sin(z) = -Sin(-z)
    
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const vector float abs_theta( spu_and( theta, abs_mask ) );
    const vector unsigned int abs_theta_GT_PIdiv2( spu_cmpgt( abs_theta, PI_DivTwo ) );
    const vector float reflected_theta( spu_sub( PI, abs_theta ) );
    const vector float result_sign( theta );
    const vector float z1( spu_sel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    
    const vector float z2( spu_mul( z1, z1 ) );
    const vector float z3( spu_mul( z2, z1 ) );
    const vector float z4( spu_mul( z2, z2 ) );
    const vector float z5( spu_mul( z3, z2 ) );
    const vector float z6( spu_mul( z3, z3 ) );
    const vector float z7( spu_mul( z4, z3 ) );
    const vector float z8( spu_mul( z4, z4 ) );
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const vector float z_01_05( spu_merge_xy( z1, z5 ) );
    const vector float z_03_07( spu_merge_xy( z3, z7 ) );
    const vector float z_01_03_05_07( spu_merge_xy( z_01_05, z_03_07 ) );
    
    const vector float z12( spu_mul( z6, z6 ) );
    const vector float z16( spu_mul( z8, z8 ) );
    const vector float z24( spu_mul( z12, z12 ) );
    
    const vector float z_09_11_13_15( spu_mul( z_01_03_05_07, z8 ) );
    const vector float z_17_19_21_23( spu_mul( z_01_03_05_07, z16 ) );
    const vector float z_25_27_29_31( spu_mul( z_01_03_05_07, z24 ) );
    
    vector float const * const __restrict p_Const_SinSeries = (vector float*)g_Const_SinSeries;
    const vector float k_00_01_02_03( p_Const_SinSeries[0] );
    const vector float k_04_05_06_07( p_Const_SinSeries[1] );
    const vector float k_08_09_10_11( p_Const_SinSeries[2] );
    const vector float k_12_13_14_15( p_Const_SinSeries[3] );
    
    const vector float result( spu_add( spu_add( spu_dot_product4( z_01_03_05_07, k_00_01_02_03 ),
                                                 spu_dot_product4( z_09_11_13_15, k_04_05_06_07 ) ),
                                        spu_add( spu_dot_product4( z_17_19_21_23, k_08_09_10_11 ),
                                                 spu_dot_product4( z_25_27_29_31, k_12_13_14_15 ) ) ) );
    
    return spu_sel( result, result_sign, sign_mask );
    
    #elif defined( _PPU_ )
    
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float PI( Const< vector float >::Pi() );
    const vector float PI_DivTwo( Const< vector float >::PiDivTwo() );
    const vector float TwoPI( Const< vector float >::TwoPi() );
    const vector float rcp_TwoPI( Const< vector float >::RcpTwoPi() );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const vector float theta( vec_nmsub( vec_vrfim( vec_mul( vec_add( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    // --> Sin( z - PI ) = -Sin(z), for m = -1
    // --> Sin(z) = -Sin( z - PI )
    // --> Sin(z) = Sin( PI - z ), using Sin(z) = -Sin(-z)
    
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const vector float abs_theta( vec_and( theta, abs_mask ) );
    const vector bool int abs_theta_GT_PIdiv2( vec_cmpgt( abs_theta, PI_DivTwo ) );
    const vector float reflected_theta( vec_sub( PI, abs_theta ) );
    const vector float result_sign( theta );
    const vector float z1( vec_sel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    
    const vector float z2( vec_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( vec_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( vec_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( vec_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( vec_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( vec_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( vec_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const vector float z_01_05( vec_merge_xy( z1, z5 ) );                 // z1z5:     z5   + 4 --> 40
    const vector float z_03_07( vec_merge_xy( z3, z7 ) );                 // z3z7:     z7   + 4 --> 42
    const vector float z_01_03_05_07( vec_merge_xy( z_01_05, z_03_07 ) ); // z1z3z5z7: z3z7 + 4 --> 46
    
    const vector float z12( vec_mul( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const vector float z16( vec_mul( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const vector float z24( vec_mul( z12, z12 ) );                     // z24: z12 + 12 --> 61
    
    const vector float z_09_11_13_15( vec_mul( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const vector float z_17_19_21_23( vec_mul( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const vector float z_25_27_29_31( vec_mul( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    
    vector float const * const __restrict p_Const_SinSeries = (vector float*)g_Const_SinSeries;
    const vector float k_00_01_02_03( p_Const_SinSeries[0] );
    const vector float k_04_05_06_07( p_Const_SinSeries[1] );
    const vector float k_08_09_10_11( p_Const_SinSeries[2] );
    const vector float k_12_13_14_15( p_Const_SinSeries[3] );
    
    const vector float result( vec_add( vec_add( vec_dot_product4( z_01_03_05_07, k_00_01_02_03 ),
                                                 vec_dot_product4( z_09_11_13_15, k_04_05_06_07 ) ),
                                        vec_add( vec_dot_product4( z_17_19_21_23, k_08_09_10_11 ),
                                                 vec_dot_product4( z_25_27_29_31, k_12_13_14_15 ) ) ) );
    
    return vec_sel( result, result_sign, sign_mask );
    
    #elif defined( _VMX128_ )
    
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 sign_mask( Const< __vector4 >::SignMask() );
    
    const __vector4 PI( Const< __vector4 >::Pi() );
    const __vector4 PI_DivTwo( Const< __vector4 >::PiDivTwo() );
    const __vector4 TwoPI( Const< __vector4 >::TwoPi() );
    const __vector4 rcp_TwoPI( Const< __vector4 >::RcpTwoPi() );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const __vector4 theta( __vnmsubfp( __vrfim( __vmulfp( __vaddfp( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    // --> Sin( z - PI ) = -Sin(z), for m = -1
    // --> Sin(z) = -Sin( z - PI )
    // --> Sin(z) = Sin( PI - z ), using Sin(z) = -Sin(-z)
    
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const __vector4 abs_theta( __vand( theta, abs_mask ) );
    const __vector4 abs_theta_GT_PIdiv2( __vcmpgtfp( abs_theta, PI_DivTwo ) );
    const __vector4 reflected_theta( __vsubfp( PI, abs_theta ) );
    const __vector4 result_sign( theta );
    const __vector4 z1( __vsel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    
    const __vector4 z2( __vmulfp( z1, z1 ) ); // z2: z1 + 12     --> 12
    const __vector4 z3( __vmulfp( z2, z1 ) ); // z3: z2 + 12     --> 24
    const __vector4 z4( __vmulfp( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const __vector4 z5( __vmulfp( z3, z2 ) ); // z5: z3 + 12     --> 36
    const __vector4 z6( __vmulfp( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const __vector4 z7( __vmulfp( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const __vector4 z8( __vmulfp( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const __vector4 z_01_05( __vmrghw( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const __vector4 z_03_07( __vmrghw( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const __vector4 z_01_03_05_07( __vmrghw( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const __vector4 z12( __vmulfp( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const __vector4 z16( __vmulfp( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const __vector4 z24( __vmulfp( z12, z12 ) );                     // z24: z12 + 12 --> 61
    
    const __vector4 z_09_11_13_15( __vmulfp( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const __vector4 z_17_19_21_23( __vmulfp( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const __vector4 z_25_27_29_31( __vmulfp( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    
    __vector4 const * const __restrict p_Const_SinSeries = (__vector4*)g_Const_SinSeries;
    const __vector4 k_00_01_02_03( p_Const_SinSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_SinSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_SinSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_SinSeries[3] );
    
    const __vector4 result( __vaddfp( __vaddfp( __vmsum4fp( z_01_03_05_07, k_00_01_02_03 ),
                                                __vmsum4fp( z_09_11_13_15, k_04_05_06_07 ) ),
                                      __vaddfp( __vmsum4fp( z_17_19_21_23, k_08_09_10_11 ),
                                                __vmsum4fp( z_25_27_29_31, k_12_13_14_15 ) ) ) );
    
    return __vsel( result, result_sign, sign_mask );
    
    #elif defined( _SSE_ )
    
    const __m128 abs_mask( Const< __m128 >::AbsMask() );
    const __m128 sign_mask( Const< __m128 >::SignMask() );
    
    const __m128 PI( Const< __m128 >::Pi() );
    const __m128 PI_DivTwo( Const< __m128 >::PiDivTwo() );
    const __m128 TwoPI( Const< __m128 >::TwoPi() );
    const __m128 rcp_TwoPI( Const< __m128 >::RcpTwoPi() );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const __m128 theta( sse_nmsub( SSE_RoundToIntMinusInf( _mm_mul_ps( _mm_add_ps( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    // --> Sin( z - PI ) = -Sin(z), for m = -1
    // --> Sin(z) = -Sin( z - PI )
    // --> Sin(z) = Sin( PI - z ), using Sin(z) = -Sin(-z)
    
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const __m128 abs_theta( _mm_and_ps( theta, abs_mask ) );
    const __m128 abs_theta_GT_PIdiv2( _mm_cmpgt_ps( abs_theta, PI_DivTwo ) );
    const __m128 reflected_theta( _mm_sub_ps( PI, abs_theta ) );
    const __m128 result_sign( theta );
    const __m128 z1( sse_select( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    
    const __m128 z2( _mm_mul_ps( z1, z1 ) );
    const __m128 z3( _mm_mul_ps( z2, z1 ) );
    const __m128 z4( _mm_mul_ps( z2, z2 ) );
    const __m128 z5( _mm_mul_ps( z3, z2 ) );
    const __m128 z6( _mm_mul_ps( z3, z3 ) );
    const __m128 z7( _mm_mul_ps( z4, z3 ) );
    const __m128 z8( _mm_mul_ps( z4, z4 ) );
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const __m128 z_01_05( sse_merge_xy( z1, z5 ) );
    const __m128 z_03_07( sse_merge_xy( z3, z7 ) );
    const __m128 z_01_03_05_07( sse_merge_xy( z_01_05, z_03_07 ) );
    
    const __m128 z12( _mm_mul_ps( z6, z6 ) );
    const __m128 z16( _mm_mul_ps( z8, z8 ) );
    const __m128 z24( _mm_mul_ps( z12, z12 ) );
    
    const __m128 z_09_11_13_15( _mm_mul_ps( z_01_03_05_07, z8 ) );
    const __m128 z_17_19_21_23( _mm_mul_ps( z_01_03_05_07, z16 ) );
    const __m128 z_25_27_29_31( _mm_mul_ps( z_01_03_05_07, z24 ) );
    
    __m128 const * const __restrict p_Const_SinSeries = (__m128*)g_Const_SinSeries;
    const __m128 k_00_01_02_03( p_Const_SinSeries[0] );
    const __m128 k_04_05_06_07( p_Const_SinSeries[1] );
    const __m128 k_08_09_10_11( p_Const_SinSeries[2] );
    const __m128 k_12_13_14_15( p_Const_SinSeries[3] );
    
    const __m128 result( _mm_add_ps( _mm_add_ps( sse_dot_product4( z_01_03_05_07, k_00_01_02_03 ),
                                                 sse_dot_product4( z_09_11_13_15, k_04_05_06_07 ) ),
                                     _mm_add_ps( sse_dot_product4( z_17_19_21_23, k_08_09_10_11 ),
                                                 sse_dot_product4( z_25_27_29_31, k_12_13_14_15 ) ) ) );
    
    return sse_select( result, result_sign, sign_mask );
    
    #endif
}


/// SIMD Vector Sin: Using power series expansion
NOALIAS const V128 SIMD_VectorSin( const V128 PCREF in_theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Sin
    //
    // Sin(z) == Sum[k=0,inf] { (-1)^k . z^(2k+1) / (2k+1)! }
    // Sin(z) = -Sin(-z)
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    
    #if defined( _SPU_ )
    
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float PI( Const< vector float >::Pi() );
    const vector float PI_DivTwo( Const< vector float >::PiDivTwo() );
    const vector float TwoPI( Const< vector float >::TwoPi() );
    const vector float rcp_TwoPI( Const< vector float >::RcpTwoPi() );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const vector float theta( spu_nmsub( spu_rfim( spu_mul( spu_add( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    // --> Sin( z - PI ) = -Sin(z), for m = -1
    // --> Sin(z) = -Sin( z - PI )
    // --> Sin(z) = Sin( PI - z ), using Sin(z) = -Sin(-z)
    
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const vector float abs_theta( spu_and( theta, abs_mask ) );
    const vector unsigned int abs_theta_GT_PIdiv2( spu_cmpgt( abs_theta, PI_DivTwo ) );
    const vector float reflected_theta( spu_sub( PI, abs_theta ) );
    const vector float result_sign( theta );
    const vector float z1( spu_sel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    
    const vector float z2( spu_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( spu_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( spu_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( spu_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( spu_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( spu_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( spu_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const vector float z9( spu_mul( z5, z4 ) );
    const vector float z10( spu_mul( z5, z5 ) );
    const vector float z11( spu_mul( z6, z5 ) );
    const vector float z12( spu_mul( z6, z6 ) );
    const vector float z13( spu_mul( z7, z6 ) );
    const vector float z14( spu_mul( z7, z7 ) );
    const vector float z15( spu_mul( z8, z7 ) );
    const vector float z16( spu_mul( z8, z8 ) );
    const vector float z17( spu_mul( z9, z8 ) );
    const vector float z19( spu_mul( z10, z9 ) );
    const vector float z21( spu_mul( z11, z10 ) );
    const vector float z23( spu_mul( z12, z11 ) );
    const vector float z25( spu_mul( z13, z12 ) );
    const vector float z27( spu_mul( z14, z13 ) );
    const vector float z29( spu_mul( z15, z14 ) );
    const vector float z31( spu_mul( z16, z15 ) );
    
    vector float const * const __restrict p_Const_SinSeries = (vector float*)g_Const_SinSeries;
    const vector float k_00_01_02_03( p_Const_SinSeries[0] );
    const vector float k_04_05_06_07( p_Const_SinSeries[1] );
    const vector float k_08_09_10_11( p_Const_SinSeries[2] );
    const vector float k_12_13_14_15( p_Const_SinSeries[3] );
    
    const vector float result( spu_add( spu_add( spu_add( spu_madd( z1,  spu_splat<0>( k_00_01_02_03 ),
                                                          spu_mul ( z3,  spu_splat<1>( k_00_01_02_03 ) ) ),
                                                          spu_madd( z5,  spu_splat<2>( k_00_01_02_03 ),
                                                          spu_mul ( z7,  spu_splat<3>( k_00_01_02_03 ) ) ) ),
                                                 spu_add( spu_madd( z9,  spu_splat<0>( k_04_05_06_07 ),
                                                          spu_mul ( z11, spu_splat<1>( k_04_05_06_07 ) ) ),
                                                          spu_madd( z13, spu_splat<2>( k_04_05_06_07 ),
                                                          spu_mul ( z15, spu_splat<3>( k_04_05_06_07 ) ) ) ) ),
                                        spu_add( spu_add( spu_madd( z17, spu_splat<0>( k_08_09_10_11 ),
                                                          spu_mul ( z19, spu_splat<1>( k_08_09_10_11 ) ) ),
                                                          spu_madd( z21, spu_splat<2>( k_08_09_10_11 ),
                                                          spu_mul ( z23, spu_splat<3>( k_08_09_10_11 ) ) ) ),
                                                 spu_add( spu_madd( z25, spu_splat<0>( k_12_13_14_15 ),
                                                          spu_mul ( z27, spu_splat<1>( k_12_13_14_15 ) ) ),
                                                          spu_madd( z29, spu_splat<2>( k_12_13_14_15 ),
                                                          spu_mul ( z31, spu_splat<3>( k_12_13_14_15 ) ) ) ) ) ) );
    
    return spu_sel( result, result_sign, sign_mask );
    
    #elif defined( _PPU_ )
    
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float PI( Const< vector float >::Pi() );
    const vector float PI_DivTwo( Const< vector float >::PiDivTwo() );
    const vector float TwoPI( Const< vector float >::TwoPi() );
    const vector float rcp_TwoPI( Const< vector float >::RcpTwoPi() );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const vector float theta( vec_nmsub( vec_vrfim( vec_mul( vec_add( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    // --> Sin( z - PI ) = -Sin(z), for m = -1
    // --> Sin(z) = -Sin( z - PI )
    // --> Sin(z) = Sin( PI - z ), using Sin(z) = -Sin(-z)
    
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const vector float abs_theta( vec_and( theta, abs_mask ) );
    const vector bool int abs_theta_GT_PIdiv2( vec_cmpgt( abs_theta, PI_DivTwo ) );
    const vector float reflected_theta( vec_sub( PI, abs_theta ) );
    const vector float result_sign( theta );
    const vector float z1( vec_sel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    
    const vector float z2( vec_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( vec_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( vec_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( vec_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( vec_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( vec_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( vec_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const vector float z9( vec_mul( z5, z4 ) );
    const vector float z10( vec_mul( z5, z5 ) );
    const vector float z11( vec_mul( z6, z5 ) );
    const vector float z12( vec_mul( z6, z6 ) );
    const vector float z13( vec_mul( z7, z6 ) );
    const vector float z14( vec_mul( z7, z7 ) );
    const vector float z15( vec_mul( z8, z7 ) );
    const vector float z16( vec_mul( z8, z8 ) );
    const vector float z17( vec_mul( z9, z8 ) );
    const vector float z19( vec_mul( z10, z9 ) );
    const vector float z21( vec_mul( z11, z10 ) );
    const vector float z23( vec_mul( z12, z11 ) );
    const vector float z25( vec_mul( z13, z12 ) );
    const vector float z27( vec_mul( z14, z13 ) );
    const vector float z29( vec_mul( z15, z14 ) );
    const vector float z31( vec_mul( z16, z15 ) );
    
    vector float const * const __restrict p_Const_SinSeries = (vector float*)g_Const_SinSeries;
    const vector float k_00_01_02_03( p_Const_SinSeries[0] );
    const vector float k_04_05_06_07( p_Const_SinSeries[1] );
    const vector float k_08_09_10_11( p_Const_SinSeries[2] );
    const vector float k_12_13_14_15( p_Const_SinSeries[3] );
    
    const vector float result( vec_add( vec_add( vec_add( vec_madd( z1,  vec_splat( k_00_01_02_03, 0 ),
                                                          vec_mul ( z3,  vec_splat( k_00_01_02_03, 1 ) ) ),
                                                          vec_madd( z5,  vec_splat( k_00_01_02_03, 2 ),
                                                          vec_mul ( z7,  vec_splat( k_00_01_02_03, 3 ) ) ) ),
                                                 vec_add( vec_madd( z9,  vec_splat( k_04_05_06_07, 0 ),
                                                          vec_mul ( z11, vec_splat( k_04_05_06_07, 1 ) ) ),
                                                          vec_madd( z13, vec_splat( k_04_05_06_07, 2 ),
                                                          vec_mul ( z15, vec_splat( k_04_05_06_07, 3 ) ) ) ) ),
                                        vec_add( vec_add( vec_madd( z17, vec_splat( k_08_09_10_11, 0 ),
                                                          vec_mul ( z19, vec_splat( k_08_09_10_11, 1 ) ) ),
                                                          vec_madd( z21, vec_splat( k_08_09_10_11, 2 ),
                                                          vec_mul ( z23, vec_splat( k_08_09_10_11, 3 ) ) ) ),
                                                 vec_add( vec_madd( z25, vec_splat( k_12_13_14_15, 0 ),
                                                          vec_mul ( z27, vec_splat( k_12_13_14_15, 1 ) ) ),
                                                          vec_madd( z29, vec_splat( k_12_13_14_15, 2 ),
                                                          vec_mul ( z31, vec_splat( k_12_13_14_15, 3 ) ) ) ) ) ) );
    
    return vec_sel( result, result_sign, sign_mask );
    
    #elif defined( _VMX128_ )
    
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 sign_mask( Const< __vector4 >::SignMask() );
    
    const __vector4 PI( Const< __vector4 >::Pi() );
    const __vector4 PI_DivTwo( Const< __vector4 >::PiDivTwo() );
    const __vector4 TwoPI( Const< __vector4 >::TwoPi() );
    const __vector4 rcp_TwoPI( Const< __vector4 >::RcpTwoPi() );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const __vector4 theta( __vnmsubfp( __vrfim( __vmulfp( __vaddfp( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // Sin( z + m.PI ) = (-1)^m . Sin(z), where m is an integer,
    // --> Sin( z - PI ) = -Sin(z), for m = -1
    // --> Sin(z) = -Sin( z - PI )
    // --> Sin(z) = Sin( PI - z ), using Sin(z) = -Sin(-z)
    
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const __vector4 abs_theta( __vand( theta, abs_mask ) );
    const __vector4 abs_theta_GT_PIdiv2( __vcmpgtfp( abs_theta, PI_DivTwo ) );
    const __vector4 reflected_theta( __vsubfp( PI, abs_theta ) );
    const __vector4 result_sign( theta );
    const __vector4 z1( __vsel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    
    const __vector4 z2( __vmulfp( z1, z1 ) ); // z2: z1 + 12     --> 12
    const __vector4 z3( __vmulfp( z2, z1 ) ); // z3: z2 + 12     --> 24
    const __vector4 z4( __vmulfp( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const __vector4 z5( __vmulfp( z3, z2 ) ); // z5: z3 + 12     --> 36
    const __vector4 z6( __vmulfp( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const __vector4 z7( __vmulfp( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const __vector4 z8( __vmulfp( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const __vector4 z9( __vmulfp( z5, z4 ) );
    const __vector4 z10( __vmulfp( z5, z5 ) );
    const __vector4 z11( __vmulfp( z6, z5 ) );
    const __vector4 z12( __vmulfp( z6, z6 ) );
    const __vector4 z13( __vmulfp( z7, z6 ) );
    const __vector4 z14( __vmulfp( z7, z7 ) );
    const __vector4 z15( __vmulfp( z8, z7 ) );
    const __vector4 z16( __vmulfp( z8, z8 ) );
    const __vector4 z17( __vmulfp( z9, z8 ) );
    const __vector4 z19( __vmulfp( z10, z9 ) );
    const __vector4 z21( __vmulfp( z11, z10 ) );
    const __vector4 z23( __vmulfp( z12, z11 ) );
    const __vector4 z25( __vmulfp( z13, z12 ) );
    const __vector4 z27( __vmulfp( z14, z13 ) );
    const __vector4 z29( __vmulfp( z15, z14 ) );
    const __vector4 z31( __vmulfp( z16, z15 ) );
    
    __vector4 const * const __restrict p_Const_SinSeries = (__vector4*)g_Const_SinSeries;
    const __vector4 k_00_01_02_03( p_Const_SinSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_SinSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_SinSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_SinSeries[3] );
    
    const __vector4 result( __vaddfp( __vaddfp( __vaddfp( __vmaddfp( z1,  __vspltw( k_00_01_02_03, 0 ),
                                                           __vmulfp( z3,  __vspltw( k_00_01_02_03, 1 ) ) ),
                                                          __vmaddfp( z5,  __vspltw( k_00_01_02_03, 2 ),
                                                           __vmulfp( z7,  __vspltw( k_00_01_02_03, 3 ) ) ) ),
                                                __vaddfp( __vmaddfp( z9,  __vspltw( k_04_05_06_07, 0 ),
                                                           __vmulfp( z11, __vspltw( k_04_05_06_07, 1 ) ) ),
                                                          __vmaddfp( z13, __vspltw( k_04_05_06_07, 2 ),
                                                           __vmulfp( z15, __vspltw( k_04_05_06_07, 3 ) ) ) ) ),
                                      __vaddfp( __vaddfp( __vmaddfp( z17, __vspltw( k_08_09_10_11, 0 ),
                                                           __vmulfp( z19, __vspltw( k_08_09_10_11, 1 ) ) ),
                                                          __vmaddfp( z21, __vspltw( k_08_09_10_11, 2 ),
                                                           __vmulfp( z23, __vspltw( k_08_09_10_11, 3 ) ) ) ),
                                                __vaddfp( __vmaddfp( z25, __vspltw( k_12_13_14_15, 0 ),
                                                           __vmulfp( z27, __vspltw( k_12_13_14_15, 1 ) ) ),
                                                          __vmaddfp( z29, __vspltw( k_12_13_14_15, 2 ),
                                                           __vmulfp( z31, __vspltw( k_12_13_14_15, 3 ) ) ) ) ) ) );
    
    return __vsel( result, result_sign, sign_mask );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}


/// SIMD Scalar Cos: Using power series expansion
NOALIAS const V128 SIMD_ScalarCos( const V128 PCREF in_theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Cos/06/01/01/0002/
    // Cos(z) == Sum[k=0,inf] { (-1)^k . z^(2k) / (2k)! }
    // Cos(z) = Cos(-z)
    // Cos( z + m.PI ) = (-1)^m . Cos(z), where m is an integer,
    // --> Cos( -( z + m.PI ) ) = (-1)^m . Cos(z), using Cos(z) = Cos(-z)
    // --> Cos( -( z - PI ) ) = -Cos(z), for m = -1
    // --> Cos(z) = -Cos( PI - z )
    
    #if defined( _SPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float PI( Const< vector float >::Pi() );
    const vector float PI_DivTwo( Const< vector float >::PiDivTwo() );
    const vector float TwoPI( Const< vector float >::TwoPi() );
    const vector float rcp_TwoPI( Const< vector float >::RcpTwoPi() );
    
    // z0 = 1
    const vector float z0( one );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const vector float theta( spu_nmsub( spu_rfim( spu_mul( spu_add( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // From Cos(z) = Cos(-z) && Cos(z) = -Cos( PI - z )
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const vector float abs_theta( spu_and( theta, abs_mask ) );
    const vector unsigned int abs_theta_GT_PIdiv2( spu_cmpgt( abs_theta, PI_DivTwo ) );
    const vector float reflected_theta( spu_sub( PI, abs_theta ) );
    const vector float result_sign( (vector float)abs_theta_GT_PIdiv2 );
    const vector float z1( spu_sel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    // CPU cycles : z1 --> 0
    const vector float z2( spu_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( spu_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( spu_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z6( spu_mul( z3, z3 ) ); // z6: z3 + 12     --> 36
    const vector float z8( spu_mul( z4, z4 ) ); // z8: z4 + 12     --> 37
    
    const vector float z_00_04( spu_merge_xy( z0, z4 ) );                 // z0z4:     z4   + 4 --> 29
    const vector float z_02_06( spu_merge_xy( z2, z6 ) );                 // z2z6:     z6   + 4 --> 40
    const vector float z_00_02_04_06( spu_merge_xy( z_00_04, z_02_06 ) ); // z0z2z4z6: z2z6 + 4 --> 44
    
    const vector float z12( spu_mul( z6, z6 ) );                       // z12: z6  + 12 --> 48
    const vector float z16( spu_mul( z8, z8 ) );                       // z16: z8  + 12 --> 49
    const vector float z24( spu_mul( z12, z12 ) );                     // z24: z12 + 12 --> 60
    
    const vector float z_08_10_12_14( spu_mul( z_00_02_04_06, z8 ) );  // z0z2z4z6 + 12 --> 56
    const vector float z_16_18_20_22( spu_mul( z_00_02_04_06, z16 ) ); // z16      + 12 --> 61
    const vector float z_24_26_28_30( spu_mul( z_00_02_04_06, z24 ) ); // z24      + 12 --> 72
    
    vector float const * const __restrict p_Const_CosSeries = (vector float*)g_Const_CosSeries;
    const vector float k_00_01_02_03( p_Const_CosSeries[0] );
    const vector float k_04_05_06_07( p_Const_CosSeries[1] );
    const vector float k_08_09_10_11( p_Const_CosSeries[2] );
    const vector float k_12_13_14_15( p_Const_CosSeries[3] );
    
    const vector float result( spu_add( spu_add( spu_dot_product4( z_00_02_04_06, k_00_01_02_03 ),
                                                 spu_dot_product4( z_08_10_12_14, k_04_05_06_07 ) ),
                                        spu_add( spu_dot_product4( z_16_18_20_22, k_08_09_10_11 ),
                                                 spu_dot_product4( z_24_26_28_30, k_12_13_14_15 ) ) ) );
    
    return spu_sel( result, result_sign, sign_mask );
    
    #elif defined( _PPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float PI( Const< vector float >::Pi() );
    const vector float PI_DivTwo( Const< vector float >::PiDivTwo() );
    const vector float TwoPI( Const< vector float >::TwoPi() );
    const vector float rcp_TwoPI( Const< vector float >::RcpTwoPi() );
    
    // z0 = 1
    const vector float z0( one );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const vector float theta( vec_nmsub( vec_vrfim( vec_mul( vec_add( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // From Cos(z) = Cos(-z) && Cos(z) = -Cos( PI - z )
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const vector float abs_theta( vec_and( theta, abs_mask ) );
    const vector bool int abs_theta_GT_PIdiv2( vec_cmpgt( abs_theta, PI_DivTwo ) );
    const vector float reflected_theta( vec_sub( PI, abs_theta ) );
    const vector float result_sign( (vector float)abs_theta_GT_PIdiv2 );
    const vector float z1( vec_sel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    // CPU cycles : z1 --> 0
    const vector float z2( vec_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( vec_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( vec_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z6( vec_mul( z3, z3 ) ); // z6: z3 + 12     --> 36
    const vector float z8( vec_mul( z4, z4 ) ); // z8: z4 + 12     --> 37
    
    const vector float z_00_04( vec_merge_xy( z0, z4 ) );                 // z0z4:     z4   + 4 --> 29
    const vector float z_02_06( vec_merge_xy( z2, z6 ) );                 // z2z6:     z6   + 4 --> 40
    const vector float z_00_02_04_06( vec_merge_xy( z_00_04, z_02_06 ) ); // z0z2z4z6: z2z6 + 4 --> 44
    
    const vector float z12( vec_mul( z6, z6 ) );                       // z12: z6  + 12 --> 48
    const vector float z16( vec_mul( z8, z8 ) );                       // z16: z8  + 12 --> 49
    const vector float z24( vec_mul( z12, z12 ) );                     // z24: z12 + 12 --> 60
    
    const vector float z_08_10_12_14( vec_mul( z_00_02_04_06, z8 ) );  // z0z2z4z6 + 12 --> 56
    const vector float z_16_18_20_22( vec_mul( z_00_02_04_06, z16 ) ); // z16      + 12 --> 61
    const vector float z_24_26_28_30( vec_mul( z_00_02_04_06, z24 ) ); // z24      + 12 --> 72
    
    vector float const * const __restrict p_Const_CosSeries = (vector float*)g_Const_CosSeries;
    const vector float k_00_01_02_03( p_Const_CosSeries[0] );
    const vector float k_04_05_06_07( p_Const_CosSeries[1] );
    const vector float k_08_09_10_11( p_Const_CosSeries[2] );
    const vector float k_12_13_14_15( p_Const_CosSeries[3] );
    
    const vector float result( vec_add( vec_add( vec_dot_product4( z_00_02_04_06, k_00_01_02_03 ),
                                                 vec_dot_product4( z_08_10_12_14, k_04_05_06_07 ) ),
                                        vec_add( vec_dot_product4( z_16_18_20_22, k_08_09_10_11 ),
                                                 vec_dot_product4( z_24_26_28_30, k_12_13_14_15 ) ) ) );
    
    return vec_sel( result, result_sign, sign_mask );
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 sign_mask( Const< __vector4 >::SignMask() );
    
    const __vector4 PI( Const< __vector4 >::Pi() );
    const __vector4 PI_DivTwo( Const< __vector4 >::PiDivTwo() );
    const __vector4 TwoPI( Const< __vector4 >::TwoPi() );
    const __vector4 rcp_TwoPI( Const< __vector4 >::RcpTwoPi() );
    
    // z0 = 1
    const __vector4 z0( one );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const __vector4 theta( __vnmsubfp( __vrfim( __vmulfp( __vaddfp( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // From Cos(z) = Cos(-z) && Cos(z) = -Cos( PI - z )
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const __vector4 abs_theta( __vand( theta, abs_mask ) );
    const __vector4 abs_theta_GT_PIdiv2( __vcmpgtfp( abs_theta, PI_DivTwo ) );
    const __vector4 reflected_theta( __vsubfp( PI, abs_theta ) );
    const __vector4 result_sign( abs_theta_GT_PIdiv2 );
    const __vector4 z1( __vsel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    // CPU cycles : z1 --> 0
    const __vector4 z2( __vmulfp( z1, z1 ) ); // z2: z1 + 12     --> 12
    const __vector4 z3( __vmulfp( z2, z1 ) ); // z3: z2 + 12     --> 24
    const __vector4 z4( __vmulfp( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const __vector4 z6( __vmulfp( z3, z3 ) ); // z6: z3 + 12     --> 36
    const __vector4 z8( __vmulfp( z4, z4 ) ); // z8: z4 + 12     --> 37
    
    const __vector4 z_00_04( __vmrghw( z0, z4 ) );                   // z0z4:     z4   + 4 --> 29
    const __vector4 z_02_06( __vmrghw( z2, z6 ) );                   // z2z6:     z6   + 4 --> 40
    const __vector4 z_00_02_04_06( __vmrghw( z_00_04, z_02_06 ) );   // z0z2z4z6: z2z6 + 4 --> 44
    
    const __vector4 z12( __vmulfp( z6, z6 ) );                       // z12: z6  + 12 --> 48
    const __vector4 z16( __vmulfp( z8, z8 ) );                       // z16: z8  + 12 --> 49
    const __vector4 z24( __vmulfp( z12, z12 ) );                     // z24: z12 + 12 --> 60
    
    const __vector4 z_08_10_12_14( __vmulfp( z_00_02_04_06, z8 ) );  // z0z2z4z6 + 12 --> 56
    const __vector4 z_16_18_20_22( __vmulfp( z_00_02_04_06, z16 ) ); // z16      + 12 --> 61
    const __vector4 z_24_26_28_30( __vmulfp( z_00_02_04_06, z24 ) ); // z24      + 12 --> 72
    
    __vector4 const * const __restrict p_Const_CosSeries = (__vector4*)g_Const_CosSeries;
    const __vector4 k_00_01_02_03( p_Const_CosSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_CosSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_CosSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_CosSeries[3] );
    
    const __vector4 result( __vaddfp( __vaddfp( __vmsum4fp( z_00_02_04_06, k_00_01_02_03 ),
                                                __vmsum4fp( z_08_10_12_14, k_04_05_06_07 ) ),
                                      __vaddfp( __vmsum4fp( z_16_18_20_22, k_08_09_10_11 ),
                                                __vmsum4fp( z_24_26_28_30, k_12_13_14_15 ) ) ) );
    
    return __vsel( result, result_sign, sign_mask );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}


/// SIMD Vector Cos: Using power series expansion
NOALIAS const V128 SIMD_VectorCos( const V128 PCREF in_theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Cos/06/01/01/0002/
    // Cos(z) == Sum[k=0,inf] { (-1)^k . z^(2k) / (2k)! }
    // Cos(z) = Cos(-z)
    // Cos( z + m.PI ) = (-1)^m . Cos(z), where m is an integer,
    // --> Cos( -( z + m.PI ) ) = (-1)^m . Cos(z), using Cos(z) = Cos(-z)
    // --> Cos( -( z - PI ) ) = -Cos(z), for m = -1
    // --> Cos(z) = -Cos( PI - z )
    
    #if defined( _SPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float PI( Const< vector float >::Pi() );
    const vector float PI_DivTwo( Const< vector float >::PiDivTwo() );
    const vector float TwoPI( Const< vector float >::TwoPi() );
    const vector float rcp_TwoPI( Const< vector float >::RcpTwoPi() );
    
    // z0 = 1
    const vector float z0( one );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const vector float theta( spu_nmsub( spu_rfim( spu_mul( spu_add( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // From Cos(z) = Cos(-z) && Cos(z) = -Cos( PI - z )
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const vector float abs_theta( spu_and( theta, abs_mask ) );
    const vector unsigned int abs_theta_GT_PIdiv2( spu_cmpgt( abs_theta, PI_DivTwo ) );
    const vector float reflected_theta( spu_sub( PI, abs_theta ) );
    const vector float result_sign( (vector float)abs_theta_GT_PIdiv2 );
    const vector float z1( spu_sel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    const vector float z2( spu_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( spu_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( spu_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( spu_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( spu_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( spu_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( spu_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const vector float z9( spu_mul( z5, z4 ) );
    const vector float z10( spu_mul( z5, z5 ) );
    const vector float z11( spu_mul( z6, z5 ) );
    const vector float z12( spu_mul( z6, z6 ) );
    const vector float z13( spu_mul( z7, z6 ) );
    const vector float z14( spu_mul( z7, z7 ) );
    const vector float z15( spu_mul( z8, z7 ) );
    const vector float z16( spu_mul( z8, z8 ) );
    const vector float z18( spu_mul( z9, z9 ) );
    const vector float z20( spu_mul( z10, z10 ) );
    const vector float z22( spu_mul( z11, z11 ) );
    const vector float z24( spu_mul( z12, z12 ) );
    const vector float z26( spu_mul( z13, z13 ) );
    const vector float z28( spu_mul( z14, z14 ) );
    const vector float z30( spu_mul( z15, z15 ) );
    
    vector float const * const __restrict p_Const_CosSeries = (vector float*)g_Const_CosSeries;
    const vector float k_00_01_02_03( p_Const_CosSeries[0] );
    const vector float k_04_05_06_07( p_Const_CosSeries[1] );
    const vector float k_08_09_10_11( p_Const_CosSeries[2] );
    const vector float k_12_13_14_15( p_Const_CosSeries[3] );
    
    const vector float result( spu_add( spu_add( spu_add( spu_madd( z0,  spu_splat<0>( k_00_01_02_03 ),
                                                           spu_mul( z2,  spu_splat<1>( k_00_01_02_03 ) ) ),
                                                          spu_madd( z4,  spu_splat<2>( k_00_01_02_03 ),
                                                           spu_mul( z6,  spu_splat<3>( k_00_01_02_03 ) ) ) ),
                                                 spu_add( spu_madd( z8,  spu_splat<0>( k_04_05_06_07 ),
                                                           spu_mul( z10, spu_splat<1>( k_04_05_06_07 ) ) ),
                                                          spu_madd( z12, spu_splat<2>( k_04_05_06_07 ),
                                                           spu_mul( z14, spu_splat<3>( k_04_05_06_07 ) ) ) ) ),
                                        spu_add( spu_add( spu_madd( z16, spu_splat<0>( k_08_09_10_11 ),
                                                           spu_mul( z18, spu_splat<1>( k_08_09_10_11 ) ) ),
                                                          spu_madd( z20, spu_splat<2>( k_08_09_10_11 ),
                                                           spu_mul( z22, spu_splat<3>( k_08_09_10_11 ) ) ) ),
                                                 spu_add( spu_madd( z24, spu_splat<0>( k_12_13_14_15 ),
                                                           spu_mul( z26, spu_splat<1>( k_12_13_14_15 ) ) ),
                                                          spu_madd( z28, spu_splat<2>( k_12_13_14_15 ),
                                                           spu_mul( z30, spu_splat<3>( k_12_13_14_15 ) ) ) ) ) ) );
    
    return spu_sel( result, result_sign, sign_mask );
    
    #elif defined( _PPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float PI( Const< vector float >::Pi() );
    const vector float PI_DivTwo( Const< vector float >::PiDivTwo() );
    const vector float TwoPI( Const< vector float >::TwoPi() );
    const vector float rcp_TwoPI( Const< vector float >::RcpTwoPi() );
    
    // z0 = 1
    const vector float z0( one );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const vector float theta( vec_nmsub( vec_vrfim( vec_mul( vec_add( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // From Cos(z) = Cos(-z) && Cos(z) = -Cos( PI - z )
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const vector float abs_theta( vec_and( theta, abs_mask ) );
    const vector bool int abs_theta_GT_PIdiv2( vec_cmpgt( abs_theta, PI_DivTwo ) );
    const vector float reflected_theta( vec_sub( PI, abs_theta ) );
    const vector float result_sign( (vector float)abs_theta_GT_PIdiv2 );
    const vector float z1( vec_sel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    const vector float z2( vec_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( vec_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( vec_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( vec_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( vec_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( vec_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( vec_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const vector float z9( vec_mul( z5, z4 ) );
    const vector float z10( vec_mul( z5, z5 ) );
    const vector float z11( vec_mul( z6, z5 ) );
    const vector float z12( vec_mul( z6, z6 ) );
    const vector float z13( vec_mul( z7, z6 ) );
    const vector float z14( vec_mul( z7, z7 ) );
    const vector float z15( vec_mul( z8, z7 ) );
    const vector float z16( vec_mul( z8, z8 ) );
    const vector float z18( vec_mul( z9, z9 ) );
    const vector float z20( vec_mul( z10, z10 ) );
    const vector float z22( vec_mul( z11, z11 ) );
    const vector float z24( vec_mul( z12, z12 ) );
    const vector float z26( vec_mul( z13, z13 ) );
    const vector float z28( vec_mul( z14, z14 ) );
    const vector float z30( vec_mul( z15, z15 ) );
    
    vector float const * const __restrict p_Const_CosSeries = (vector float*)g_Const_CosSeries;
    const vector float k_00_01_02_03( p_Const_CosSeries[0] );
    const vector float k_04_05_06_07( p_Const_CosSeries[1] );
    const vector float k_08_09_10_11( p_Const_CosSeries[2] );
    const vector float k_12_13_14_15( p_Const_CosSeries[3] );
    
    const vector float result( vec_add( vec_add( vec_add( vec_madd( z0,  vec_splat( k_00_01_02_03, 0 ),
                                                           vec_mul( z2,  vec_splat( k_00_01_02_03, 1 ) ) ),
                                                          vec_madd( z4,  vec_splat( k_00_01_02_03, 2 ),
                                                           vec_mul( z6,  vec_splat( k_00_01_02_03, 3 ) ) ) ),
                                                 vec_add( vec_madd( z8,  vec_splat( k_04_05_06_07, 0 ),
                                                           vec_mul( z10, vec_splat( k_04_05_06_07, 1 ) ) ),
                                                          vec_madd( z12, vec_splat( k_04_05_06_07, 2 ),
                                                           vec_mul( z14, vec_splat( k_04_05_06_07, 3 ) ) ) ) ),
                                        vec_add( vec_add( vec_madd( z16, vec_splat( k_08_09_10_11, 0 ),
                                                           vec_mul( z18, vec_splat( k_08_09_10_11, 1 ) ) ),
                                                          vec_madd( z20, vec_splat( k_08_09_10_11, 2 ),
                                                           vec_mul( z22, vec_splat( k_08_09_10_11, 3 ) ) ) ),
                                                 vec_add( vec_madd( z24, vec_splat( k_12_13_14_15, 0 ),
                                                           vec_mul( z26, vec_splat( k_12_13_14_15, 1 ) ) ),
                                                          vec_madd( z28, vec_splat( k_12_13_14_15, 2 ),
                                                           vec_mul( z30, vec_splat( k_12_13_14_15, 3 ) ) ) ) ) ) );
    
    return vec_sel( result, result_sign, sign_mask );
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 sign_mask( Const< __vector4 >::SignMask() );
    
    const __vector4 PI( Const< __vector4 >::Pi() );
    const __vector4 PI_DivTwo( Const< __vector4 >::PiDivTwo() );
    const __vector4 TwoPI( Const< __vector4 >::TwoPi() );
    const __vector4 rcp_TwoPI( Const< __vector4 >::RcpTwoPi() );
    
    // z0 = 1
    const __vector4 z0( one );
    
    // Wrap theta to interval -PI <= theta <= +PI
    const __vector4 theta( __vnmsubfp( __vrfim( __vmulfp( __vaddfp( in_theta, PI ), rcp_TwoPI ) ), TwoPI, in_theta ) );
    
    // From Cos(z) = Cos(-z) && Cos(z) = -Cos( PI - z )
    // Calculate z1(theta) in range 0 <= theta <= +PI/2
    const __vector4 abs_theta( __vand( theta, abs_mask ) );
    const __vector4 abs_theta_GT_PIdiv2( __vcmpgtfp( abs_theta, PI_DivTwo ) );
    const __vector4 reflected_theta( __vsubfp( PI, abs_theta ) );
    const __vector4 result_sign( abs_theta_GT_PIdiv2 );
    const __vector4 z1( __vsel( theta, reflected_theta, abs_theta_GT_PIdiv2 ) );
    
    // CPU cycles : z1 --> 0
    const __vector4 z2( __vmulfp( z1, z1 ) ); // z2: z1 + 12     --> 12
    const __vector4 z3( __vmulfp( z2, z1 ) ); // z3: z2 + 12     --> 24
    const __vector4 z4( __vmulfp( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const __vector4 z5( __vmulfp( z3, z2 ) ); // z5: z3 + 12     --> 36
    const __vector4 z6( __vmulfp( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const __vector4 z7( __vmulfp( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const __vector4 z8( __vmulfp( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const __vector4 z9( __vmulfp( z5, z4 ) );
    const __vector4 z10( __vmulfp( z5, z5 ) );
    const __vector4 z11( __vmulfp( z6, z5 ) );
    const __vector4 z12( __vmulfp( z6, z6 ) );
    const __vector4 z13( __vmulfp( z7, z6 ) );
    const __vector4 z14( __vmulfp( z7, z7 ) );
    const __vector4 z15( __vmulfp( z8, z7 ) );
    const __vector4 z16( __vmulfp( z8, z8 ) );
    const __vector4 z18( __vmulfp( z9, z9 ) );
    const __vector4 z20( __vmulfp( z10, z10 ) );
    const __vector4 z22( __vmulfp( z11, z11 ) );
    const __vector4 z24( __vmulfp( z12, z12 ) );
    const __vector4 z26( __vmulfp( z13, z13 ) );
    const __vector4 z28( __vmulfp( z14, z14 ) );
    const __vector4 z30( __vmulfp( z15, z15 ) );
    
    __vector4 const * const __restrict p_Const_CosSeries = (__vector4*)g_Const_CosSeries;
    const __vector4 k_00_01_02_03( p_Const_CosSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_CosSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_CosSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_CosSeries[3] );
    
    const __vector4 result( __vaddfp( __vaddfp( __vaddfp( __vmaddfp( z0,  __vspltw( k_00_01_02_03, 0 ),
                                                           __vmulfp( z2,  __vspltw( k_00_01_02_03, 1 ) ) ),
                                                          __vmaddfp( z4,  __vspltw( k_00_01_02_03, 2 ),
                                                           __vmulfp( z6,  __vspltw( k_00_01_02_03, 3 ) ) ) ),
                                                __vaddfp( __vmaddfp( z8,  __vspltw( k_04_05_06_07, 0 ),
                                                           __vmulfp( z10, __vspltw( k_04_05_06_07, 1 ) ) ),
                                                          __vmaddfp( z12, __vspltw( k_04_05_06_07, 2 ),
                                                           __vmulfp( z14, __vspltw( k_04_05_06_07, 3 ) ) ) ) ),
                                      __vaddfp( __vaddfp( __vmaddfp( z16, __vspltw( k_08_09_10_11, 0 ),
                                                           __vmulfp( z18, __vspltw( k_08_09_10_11, 1 ) ) ),
                                                          __vmaddfp( z20, __vspltw( k_08_09_10_11, 2 ),
                                                           __vmulfp( z22, __vspltw( k_08_09_10_11, 3 ) ) ) ),
                                                __vaddfp( __vmaddfp( z24, __vspltw( k_12_13_14_15, 0 ),
                                                           __vmulfp( z26, __vspltw( k_12_13_14_15, 1 ) ) ),
                                                          __vmaddfp( z28, __vspltw( k_12_13_14_15, 2 ),
                                                           __vmulfp( z30, __vspltw( k_12_13_14_15, 3 ) ) ) ) ) ) );
    
    return __vsel( result, result_sign, sign_mask );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}


/// SIMD Scalar Tan: Using power series expansion
NOALIAS const V128 SIMD_ScalarTan( const V128 PCREF in_theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Tan
    //
    // Tan(z) == Sum[k=1,inf] { z^(2k-1) . -1^(k-1) . 2^(2k) . (2^(2k))-1 . B(2k) / (2k)! } : |z| < PI/2
    
    #if defined( _SPU_ )
    
    const vector float half( Const< vector float >::Half() );
    const vector float one( Const< vector float >::One() );
    const vector float half_PI( Const< vector float >::PiDivTwo() );
    const vector float PI( Const< vector float >::Pi() );
    const vector float rcp_PI( Const< vector float >::RcpPi() );
    
    // Wrap theta to interval -PI/2 <= theta <= +PI/2
    const vector float theta( spu_nmsub( spu_rfim( spu_mul( spu_add( in_theta, half_PI ), rcp_PI ) ), PI, in_theta ) );
    
    const vector float z1( spu_mul( theta, half ) );
    
    const vector float z2( spu_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( spu_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( spu_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( spu_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( spu_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( spu_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( spu_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const vector float z_01_05( spu_merge_xy( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const vector float z_03_07( spu_merge_xy( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const vector float z_01_03_05_07( spu_merge_xy( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const vector float z12( spu_mul( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const vector float z16( spu_mul( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const vector float z24( spu_mul( z12, z12 ) );                     // z24: z12 + 12 --> 61
    
    const vector float z_09_11_13_15( spu_mul( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const vector float z_17_19_21_23( spu_mul( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const vector float z_25_27_29_31( spu_mul( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    
    vector float const * const __restrict p_Const_TanSeries = (vector float*)g_Const_TanSeries;
    const vector float k_01_02_03_04( p_Const_TanSeries[0] );
    const vector float k_05_06_07_08( p_Const_TanSeries[1] );
    const vector float k_09_10_11_12( p_Const_TanSeries[2] );
    const vector float k_13_14_15_16( p_Const_TanSeries[3] );
    
    const vector float tanHalfTheta( spu_add( spu_add( spu_dot_product4( z_01_03_05_07, k_01_02_03_04 ),
                                                       spu_dot_product4( z_09_11_13_15, k_05_06_07_08 ) ),
                                              spu_add( spu_dot_product4( z_17_19_21_23, k_09_10_11_12 ),
                                                       spu_dot_product4( z_25_27_29_31, k_13_14_15_16 ) ) ) );
    
    // Double-angle formula: Tan( 2 * theta ) = 2 * Tan( theta ) / ( 1 - ( Tan^2( theta ) )
    return SIMD_UnsafeDivide<22>( spu_add( tanHalfTheta, tanHalfTheta ), spu_nmsub( tanHalfTheta, tanHalfTheta, one ) );
    
    #elif defined( _PPU_ )
    
    const vector float half( Const< vector float >::Half() );
    const vector float one( Const< vector float >::One() );
    const vector float half_PI( Const< vector float >::PiDivTwo() );
    const vector float PI( Const< vector float >::Pi() );
    const vector float rcp_PI( Const< vector float >::RcpPi() );
    
    // Wrap theta to interval -PI/2 <= theta <= +PI/2
    const vector float theta( vec_nmsub( vec_vrfim( vec_mul( vec_add( in_theta, half_PI ), rcp_PI ) ), PI, in_theta ) );
    
    const vector float z1( vec_mul( theta, half ) );
    
    const vector float z2( vec_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( vec_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( vec_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( vec_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( vec_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( vec_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( vec_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const vector float z_01_05( vec_merge_xy( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const vector float z_03_07( vec_merge_xy( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const vector float z_01_03_05_07( vec_merge_xy( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const vector float z12( vec_mul( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const vector float z16( vec_mul( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const vector float z24( vec_mul( z12, z12 ) );                     // z24: z12 + 12 --> 61
    
    const vector float z_09_11_13_15( vec_mul( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const vector float z_17_19_21_23( vec_mul( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const vector float z_25_27_29_31( vec_mul( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    
    vector float const * const __restrict p_Const_TanSeries = (vector float*)g_Const_TanSeries;
    const vector float k_01_02_03_04( p_Const_TanSeries[0] );
    const vector float k_05_06_07_08( p_Const_TanSeries[1] );
    const vector float k_09_10_11_12( p_Const_TanSeries[2] );
    const vector float k_13_14_15_16( p_Const_TanSeries[3] );
    
    const vector float tanHalfTheta( vec_add( vec_add( vec_dot_product4( z_01_03_05_07, k_01_02_03_04 ),
                                                       vec_dot_product4( z_09_11_13_15, k_05_06_07_08 ) ),
                                              vec_add( vec_dot_product4( z_17_19_21_23, k_09_10_11_12 ),
                                                       vec_dot_product4( z_25_27_29_31, k_13_14_15_16 ) ) ) );
    
    // Double-angle formula: Tan( 2 * theta ) = 2 * Tan( theta ) / ( 1 - ( Tan^2( theta ) )
    return SIMD_UnsafeDivide<22>( vec_add( tanHalfTheta, tanHalfTheta ), vec_nmsub( tanHalfTheta, tanHalfTheta, one ) );
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 half_PI( Const< __vector4 >::PiDivTwo() );
    const __vector4 PI( Const< __vector4 >::Pi() );
    const __vector4 rcp_PI( Const< __vector4 >::RcpPi() );
    
    // Wrap theta to interval -PI/2 <= theta <= +PI/2
    const __vector4 theta( __vnmsubfp( __vrfim( __vmulfp( __vaddfp( in_theta, half_PI ), rcp_PI ) ), PI, in_theta ) );
    
    const __vector4 z1( __vmulfp( theta, half ) );
    
    const __vector4 z2( __vmulfp( z1, z1 ) ); // z2: z1 + 12     --> 12
    const __vector4 z3( __vmulfp( z2, z1 ) ); // z3: z2 + 12     --> 24
    const __vector4 z4( __vmulfp( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const __vector4 z5( __vmulfp( z3, z2 ) ); // z5: z3 + 12     --> 36
    const __vector4 z6( __vmulfp( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const __vector4 z7( __vmulfp( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const __vector4 z8( __vmulfp( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const __vector4 z_01_05( __vmrghw( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const __vector4 z_03_07( __vmrghw( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const __vector4 z_01_03_05_07( __vmrghw( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const __vector4 z12( __vmulfp( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const __vector4 z16( __vmulfp( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const __vector4 z24( __vmulfp( z12, z12 ) );                     // z24: z12 + 12 --> 61
    
    const __vector4 z_09_11_13_15( __vmulfp( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const __vector4 z_17_19_21_23( __vmulfp( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const __vector4 z_25_27_29_31( __vmulfp( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    
    __vector4 const * const __restrict p_Const_TanSeries = (__vector4*)g_Const_TanSeries;
    const __vector4 k_01_02_03_04( p_Const_TanSeries[0] );
    const __vector4 k_05_06_07_08( p_Const_TanSeries[1] );
    const __vector4 k_09_10_11_12( p_Const_TanSeries[2] );
    const __vector4 k_13_14_15_16( p_Const_TanSeries[3] );
    
    const __vector4 tanHalfTheta( __vaddfp( __vaddfp( __vmsum4fp( z_01_03_05_07, k_01_02_03_04 ),
                                                      __vmsum4fp( z_09_11_13_15, k_05_06_07_08 ) ),
                                            __vaddfp( __vmsum4fp( z_17_19_21_23, k_09_10_11_12 ),
                                                      __vmsum4fp( z_25_27_29_31, k_13_14_15_16 ) ) ) );
    
    // Double-angle formula: Tan( 2 * theta ) = 2 * Tan( theta ) / ( 1 - ( Tan^2( theta ) )
    return SIMD_UnsafeDivide<22>( __vaddfp( tanHalfTheta, tanHalfTheta ), __vnmsubfp( tanHalfTheta, tanHalfTheta, one ) );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}


/// SIMD Vector Tan: Using power series expansion
NOALIAS const V128 SIMD_VectorTan( const V128 PCREF in_theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Tan
    //
    // Tan(z) == Sum[k=1,inf] { z^(2k-1) . -1^(k-1) . 2^(2k) . (2^(2k))-1 . B(2k) / (2k)! } : |z| < PI/2
    
    #if defined( _SPU_ )
    
    const vector float half( Const< vector float >::Half() );
    const vector float one( Const< vector float >::One() );
    const vector float half_PI( Const< vector float >::PiDivTwo() );
    const vector float PI( Const< vector float >::Pi() );
    const vector float rcp_PI( Const< vector float >::RcpPi() );
    
    // Wrap theta to interval -PI/2 <= theta <= +PI/2
    const vector float theta( spu_nmsub( spu_rfim( spu_mul( spu_add( in_theta, half_PI ), rcp_PI ) ), PI, in_theta ) );
    
    const vector float z1( spu_mul( theta, half ) );
    const vector float z2( spu_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( spu_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( spu_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( spu_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( spu_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( spu_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( spu_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const vector float z9( spu_mul( z5, z4 ) );
    const vector float z10( spu_mul( z5, z5 ) );
    const vector float z11( spu_mul( z6, z5 ) );
    const vector float z12( spu_mul( z6, z6 ) );
    const vector float z13( spu_mul( z7, z6 ) );
    const vector float z14( spu_mul( z7, z7 ) );
    const vector float z15( spu_mul( z8, z7 ) );
    const vector float z16( spu_mul( z8, z8 ) );
    const vector float z17( spu_mul( z9, z8 ) );
  //const vector float z18( spu_mul( z9, z9 ) );
    const vector float z19( spu_mul( z10, z9 ) );
  //const vector float z20( spu_mul( z10, z10 ) );
    const vector float z21( spu_mul( z11, z10 ) );
  //const vector float z22( spu_mul( z11, z11 ) );
    const vector float z23( spu_mul( z12, z11 ) );
  //const vector float z24( spu_mul( z12, z12 ) );
    const vector float z25( spu_mul( z13, z12 ) );
  //const vector float z26( spu_mul( z13, z13 ) );
    const vector float z27( spu_mul( z14, z13 ) );
  //const vector float z28( spu_mul( z14, z14 ) );
    const vector float z29( spu_mul( z15, z14 ) );
  //const vector float z30( spu_mul( z15, z15 ) );
    const vector float z31( spu_mul( z16, z15 ) );
    
    vector float const * const __restrict p_Const_TanSeries = (vector float*)g_Const_TanSeries;
    const vector float k_01_02_03_04( p_Const_TanSeries[0] );
    const vector float k_05_06_07_08( p_Const_TanSeries[1] );
    const vector float k_09_10_11_12( p_Const_TanSeries[2] );
    const vector float k_13_14_15_16( p_Const_TanSeries[3] );
    
    const vector float tanHalfTheta( spu_add( spu_add( spu_add( spu_madd( z1,  spu_splat<0>( k_01_02_03_04 ),
                                                                spu_mul ( z3,  spu_splat<1>( k_01_02_03_04 ) ) ),
                                                                spu_madd( z5,  spu_splat<2>( k_01_02_03_04 ),
                                                                spu_mul ( z7,  spu_splat<3>( k_01_02_03_04 ) ) ) ),
                                                       spu_add( spu_madd( z9,  spu_splat<0>( k_05_06_07_08 ),
                                                                spu_mul ( z11, spu_splat<1>( k_05_06_07_08 ) ) ),
                                                                spu_madd( z13, spu_splat<2>( k_05_06_07_08 ),
                                                                spu_mul ( z15, spu_splat<3>( k_05_06_07_08 ) ) ) ) ),
                                              spu_add( spu_add( spu_madd( z17, spu_splat<0>( k_09_10_11_12 ),
                                                                spu_mul ( z19, spu_splat<1>( k_09_10_11_12 ) ) ),
                                                                spu_madd( z21, spu_splat<2>( k_09_10_11_12 ),
                                                                spu_mul ( z23, spu_splat<3>( k_09_10_11_12 ) ) ) ),
                                                       spu_add( spu_madd( z25, spu_splat<0>( k_13_14_15_16 ),
                                                                spu_mul ( z27, spu_splat<1>( k_13_14_15_16 ) ) ),
                                                                spu_madd( z29, spu_splat<2>( k_13_14_15_16 ),
                                                                spu_mul ( z31, spu_splat<3>( k_13_14_15_16 ) ) ) ) ) ) );
    
    // Double-angle formula: Tan( 2 * theta ) = 2 * Tan( theta ) / ( 1 - ( Tan^2( theta ) )
    return SIMD_UnsafeDivide<22>( spu_add( tanHalfTheta, tanHalfTheta ), spu_nmsub( tanHalfTheta, tanHalfTheta, one ) );
    
    #elif defined( _PPU_ )
    
    const vector float half( Const< vector float >::Half() );
    const vector float one( Const< vector float >::One() );
    const vector float half_PI( Const< vector float >::PiDivTwo() );
    const vector float PI( Const< vector float >::Pi() );
    const vector float rcp_PI( Const< vector float >::RcpPi() );
    
    // Wrap theta to interval -PI/2 <= theta <= +PI/2
    const vector float theta( vec_nmsub( vec_vrfim( vec_mul( vec_add( in_theta, half_PI ), rcp_PI ) ), PI, in_theta ) );
    
    const vector float z1( vec_mul( theta, half ) );
    const vector float z2( vec_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( vec_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( vec_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( vec_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( vec_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( vec_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( vec_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const vector float z9( vec_mul( z5, z4 ) );
    const vector float z10( vec_mul( z5, z5 ) );
    const vector float z11( vec_mul( z6, z5 ) );
    const vector float z12( vec_mul( z6, z6 ) );
    const vector float z13( vec_mul( z7, z6 ) );
    const vector float z14( vec_mul( z7, z7 ) );
    const vector float z15( vec_mul( z8, z7 ) );
    const vector float z16( vec_mul( z8, z8 ) );
    const vector float z17( vec_mul( z9, z8 ) );
  //const vector float z18( vec_mul( z9, z9 ) );
    const vector float z19( vec_mul( z10, z9 ) );
  //const vector float z20( vec_mul( z10, z10 ) );
    const vector float z21( vec_mul( z11, z10 ) );
  //const vector float z22( vec_mul( z11, z11 ) );
    const vector float z23( vec_mul( z12, z11 ) );
  //const vector float z24( vec_mul( z12, z12 ) );
    const vector float z25( vec_mul( z13, z12 ) );
  //const vector float z26( vec_mul( z13, z13 ) );
    const vector float z27( vec_mul( z14, z13 ) );
  //const vector float z28( vec_mul( z14, z14 ) );
    const vector float z29( vec_mul( z15, z14 ) );
  //const vector float z30( vec_mul( z15, z15 ) );
    const vector float z31( vec_mul( z16, z15 ) );
    
    vector float const * const __restrict p_Const_TanSeries = (vector float*)g_Const_TanSeries;
    const vector float k_01_02_03_04( p_Const_TanSeries[0] );
    const vector float k_05_06_07_08( p_Const_TanSeries[1] );
    const vector float k_09_10_11_12( p_Const_TanSeries[2] );
    const vector float k_13_14_15_16( p_Const_TanSeries[3] );
    
    const vector float tanHalfTheta( vec_add( vec_add( vec_add( vec_madd( z1,  vec_splat( k_01_02_03_04, 0 ),
                                                                vec_mul ( z3,  vec_splat( k_01_02_03_04, 1 ) ) ),
                                                                vec_madd( z5,  vec_splat( k_01_02_03_04, 2 ),
                                                                vec_mul ( z7,  vec_splat( k_01_02_03_04, 3 ) ) ) ),
                                                       vec_add( vec_madd( z9,  vec_splat( k_05_06_07_08, 0 ),
                                                                vec_mul ( z11, vec_splat( k_05_06_07_08, 1 ) ) ),
                                                                vec_madd( z13, vec_splat( k_05_06_07_08, 2 ),
                                                                vec_mul ( z15, vec_splat( k_05_06_07_08, 3 ) ) ) ) ),
                                              vec_add( vec_add( vec_madd( z17, vec_splat( k_09_10_11_12, 0 ),
                                                                vec_mul ( z19, vec_splat( k_09_10_11_12, 1 ) ) ),
                                                                vec_madd( z21, vec_splat( k_09_10_11_12, 2 ),
                                                                vec_mul ( z23, vec_splat( k_09_10_11_12, 3 ) ) ) ),
                                                       vec_add( vec_madd( z25, vec_splat( k_13_14_15_16, 0 ),
                                                                vec_mul ( z27, vec_splat( k_13_14_15_16, 1 ) ) ),
                                                                vec_madd( z29, vec_splat( k_13_14_15_16, 2 ),
                                                                vec_mul ( z31, vec_splat( k_13_14_15_16, 3 ) ) ) ) ) ) );
    
    // Double-angle formula: Tan( 2 * theta ) = 2 * Tan( theta ) / ( 1 - ( Tan^2( theta ) )
    return SIMD_UnsafeDivide<22>( vec_add( tanHalfTheta, tanHalfTheta ), vec_nmsub( tanHalfTheta, tanHalfTheta, one ) );
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 half_PI( Const< __vector4 >::PiDivTwo() );
    const __vector4 PI( Const< __vector4 >::Pi() );
    const __vector4 rcp_PI( Const< __vector4 >::RcpPi() );
    
    // Wrap theta to interval -PI/2 <= theta <= +PI/2
    const __vector4 theta( __vnmsubfp( __vrfim( __vmulfp( __vaddfp( in_theta, half_PI ), rcp_PI ) ), PI, in_theta ) );
    
    const __vector4 z1( __vmulfp( theta, half ) );
    const __vector4 z2( __vmulfp( z1, z1 ) ); // z2: z1 + 12     --> 12
    const __vector4 z3( __vmulfp( z2, z1 ) ); // z3: z2 + 12     --> 24
    const __vector4 z4( __vmulfp( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const __vector4 z5( __vmulfp( z3, z2 ) ); // z5: z3 + 12     --> 36
    const __vector4 z6( __vmulfp( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const __vector4 z7( __vmulfp( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const __vector4 z8( __vmulfp( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    const __vector4 z9( __vmulfp( z5, z4 ) );
    const __vector4 z10( __vmulfp( z5, z5 ) );
    const __vector4 z11( __vmulfp( z6, z5 ) );
    const __vector4 z12( __vmulfp( z6, z6 ) );
    const __vector4 z13( __vmulfp( z7, z6 ) );
    const __vector4 z14( __vmulfp( z7, z7 ) );
    const __vector4 z15( __vmulfp( z8, z7 ) );
    const __vector4 z16( __vmulfp( z8, z8 ) );
    const __vector4 z17( __vmulfp( z9, z8 ) );
  //const __vector4 z18( __vmulfp( z9, z9 ) );
    const __vector4 z19( __vmulfp( z10, z9 ) );
  //const __vector4 z20( __vmulfp( z10, z10 ) );
    const __vector4 z21( __vmulfp( z11, z10 ) );
  //const __vector4 z22( __vmulfp( z11, z11 ) );
    const __vector4 z23( __vmulfp( z12, z11 ) );
  //const __vector4 z24( __vmulfp( z12, z12 ) );
    const __vector4 z25( __vmulfp( z13, z12 ) );
  //const __vector4 z26( __vmulfp( z13, z13 ) );
    const __vector4 z27( __vmulfp( z14, z13 ) );
  //const __vector4 z28( __vmulfp( z14, z14 ) );
    const __vector4 z29( __vmulfp( z15, z14 ) );
  //const __vector4 z30( __vmulfp( z15, z15 ) );
    const __vector4 z31( __vmulfp( z16, z15 ) );
    
    __vector4 const * const __restrict p_Const_TanSeries = (__vector4*)g_Const_TanSeries;
    const __vector4 k_01_02_03_04( p_Const_TanSeries[0] );
    const __vector4 k_05_06_07_08( p_Const_TanSeries[1] );
    const __vector4 k_09_10_11_12( p_Const_TanSeries[2] );
    const __vector4 k_13_14_15_16( p_Const_TanSeries[3] );
    
    const __vector4 tanHalfTheta( __vaddfp( __vaddfp( __vaddfp( __vmaddfp( z1,  __vspltw( k_01_02_03_04, 0 ),
                                                                 __vmulfp( z3,  __vspltw( k_01_02_03_04, 1 ) ) ),
                                                                __vmaddfp( z5,  __vspltw( k_01_02_03_04, 2 ),
                                                                 __vmulfp( z7,  __vspltw( k_01_02_03_04, 3 ) ) ) ),
                                                      __vaddfp( __vmaddfp( z9,  __vspltw( k_05_06_07_08, 0 ),
                                                                 __vmulfp( z11, __vspltw( k_05_06_07_08, 1 ) ) ),
                                                                __vmaddfp( z13, __vspltw( k_05_06_07_08, 2 ),
                                                                 __vmulfp( z15, __vspltw( k_05_06_07_08, 3 ) ) ) ) ),
                                            __vaddfp( __vaddfp( __vmaddfp( z17, __vspltw( k_09_10_11_12, 0 ),
                                                                 __vmulfp( z19, __vspltw( k_09_10_11_12, 1 ) ) ),
                                                                __vmaddfp( z21, __vspltw( k_09_10_11_12, 2 ),
                                                                 __vmulfp( z23, __vspltw( k_09_10_11_12, 3 ) ) ) ),
                                                      __vaddfp( __vmaddfp( z25, __vspltw( k_13_14_15_16, 0 ),
                                                                 __vmulfp( z27, __vspltw( k_13_14_15_16, 1 ) ) ),
                                                                __vmaddfp( z29, __vspltw( k_13_14_15_16, 2 ),
                                                                 __vmulfp( z31, __vspltw( k_13_14_15_16, 3 ) ) ) ) ) ) );
    
    // Double-angle formula: Tan( 2 * theta ) = 2 * Tan( theta ) / ( 1 - ( Tan^2( theta ) )
    return SIMD_UnsafeDivide<22>( __vaddfp( tanHalfTheta, tanHalfTheta ), __vnmsubfp( tanHalfTheta, tanHalfTheta, one ) );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}


static const Vec4F32 g_ArcSinSmallThetaRatio = MakeVec4F32( F32( 1.0 / 0.04 ), F32( 1.0 / 0.04 ), F32( 1.0 / 0.04 ), F32( 1.0 / 0.04 ) );

/// SIMD Scalar ArcSin: Using power series expansion
NOALIAS const V128 SIMD_ScalarArcSin( const V128 PCREF in_sinTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcSin
    //
    // ArcSin(z) == Sum[k=0,inf] { z^(2k+1) . Pochhammer( 0.5, k ) / (2k+1)k! }
    
    #if defined( _SPU_ )
    
    const vector float half( Const< vector float >::Half() );
    const vector float one( Const< vector float >::One() );
    const vector float minus_one( Const< vector float >::MinusOne() );
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float sinTheta( spu_min( spu_max( in_sinTheta, minus_one ), one ) );
    
    // Fundamental identity: Cos( theta ) = Sqrt( 1 - Sin^2( theta ) )
    
    //const vector float cosTheta( SIMD_Sqrt<22>( vec_nmsub( sinTheta, sinTheta, one ) ) );
    const vector float cosTheta( SIMD_Sqrt<24>( spu_nmsub( sinTheta, sinTheta, one ) ) );
    
    // Half-angle formula: Sin^2( theta / 2 ) = ( 1 - Cos( theta ) ) / 2
    //                     Sin( theta / 2 ) = Sqrt( ( 1 - Cos( theta ) ) / 2 )
    
    //const vector float z1( vec_sel( SIMD_Sqrt<22>( vec_nmsub( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    const vector float z1( spu_sel( SIMD_Sqrt<24>( spu_nmsub( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    
    const vector float z2( spu_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( spu_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( spu_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( spu_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( spu_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( spu_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( spu_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const vector float z_01_05( spu_merge_xy( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const vector float z_03_07( spu_merge_xy( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const vector float z_01_03_05_07( spu_merge_xy( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const vector float z12( spu_mul( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const vector float z16( spu_mul( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const vector float z24( spu_mul( z12, z12 ) );                     // z24: z12 + 12 --> 61
    
    const vector float z_09_11_13_15( spu_mul( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const vector float z_17_19_21_23( spu_mul( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const vector float z_25_27_29_31( spu_mul( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    
    vector float const * const __restrict p_Const_ArcSinSeries = (vector float*)g_Const_ArcSinSeries;
    const vector float k_00_01_02_03( p_Const_ArcSinSeries[0] );
    const vector float k_04_05_06_07( p_Const_ArcSinSeries[1] );
    const vector float k_08_09_10_11( p_Const_ArcSinSeries[2] );
    const vector float k_12_13_14_15( p_Const_ArcSinSeries[3] );
    
    const vector float halfTheta( spu_add( spu_add( spu_dot_product4( z_01_03_05_07, k_00_01_02_03 ),
                                                    spu_dot_product4( z_09_11_13_15, k_04_05_06_07 ) ),
                                           spu_add( spu_dot_product4( z_17_19_21_23, k_08_09_10_11 ),
                                                    spu_dot_product4( z_25_27_29_31, k_12_13_14_15 ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const vector float theta( spu_add( halfTheta, halfTheta ) );
    
    // For small sinTheta: ArcSin( sinTheta ) ~= sinTheta
    const vector float ratio( spu_min( spu_mul( spu_and( sinTheta, abs_mask ), g_ArcSinSmallThetaRatio.m_V ), one ) );
    return spu_madd( spu_sub( theta, sinTheta ), ratio, sinTheta );
    
    #elif defined( _PPU_ )
    
    const vector float half( Const< vector float >::Half() );
    const vector float one( Const< vector float >::One() );
    const vector float minus_one( Const< vector float >::MinusOne() );
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float sinTheta( vec_min( vec_max( in_sinTheta, minus_one ), one ) );
    
    // Fundamental identity: Cos( theta ) = Sqrt( 1 - Sin^2( theta ) )
    
    //const vector float cosTheta( SIMD_Sqrt<22>( vec_nmsub( sinTheta, sinTheta, one ) ) );
    const vector float cosTheta( SIMD_Sqrt<24>( vec_nmsub( sinTheta, sinTheta, one ) ) );
    
    // Half-angle formula: Sin^2( theta / 2 ) = ( 1 - Cos( theta ) ) / 2
    //                     Sin( theta / 2 ) = Sqrt( ( 1 - Cos( theta ) ) / 2 )
    
    //const vector float z1( vec_sel( SIMD_Sqrt<22>( vec_nmsub( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    const vector float z1( vec_sel( SIMD_Sqrt<24>( vec_nmsub( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    
    const vector float z2( vec_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( vec_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( vec_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( vec_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( vec_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( vec_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( vec_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const vector float z_01_05( vec_merge_xy( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const vector float z_03_07( vec_merge_xy( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const vector float z_01_03_05_07( vec_merge_xy( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const vector float z12( vec_mul( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const vector float z16( vec_mul( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const vector float z24( vec_mul( z12, z12 ) );                     // z24: z12 + 12 --> 61
    
    const vector float z_09_11_13_15( vec_mul( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const vector float z_17_19_21_23( vec_mul( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const vector float z_25_27_29_31( vec_mul( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    
    vector float const * const __restrict p_Const_ArcSinSeries = (vector float*)g_Const_ArcSinSeries;
    const vector float k_00_01_02_03( p_Const_ArcSinSeries[0] );
    const vector float k_04_05_06_07( p_Const_ArcSinSeries[1] );
    const vector float k_08_09_10_11( p_Const_ArcSinSeries[2] );
    const vector float k_12_13_14_15( p_Const_ArcSinSeries[3] );
    
    const vector float halfTheta( vec_add( vec_add( vec_dot_product4( z_01_03_05_07, k_00_01_02_03 ),
                                                    vec_dot_product4( z_09_11_13_15, k_04_05_06_07 ) ),
                                           vec_add( vec_dot_product4( z_17_19_21_23, k_08_09_10_11 ),
                                                    vec_dot_product4( z_25_27_29_31, k_12_13_14_15 ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const vector float theta( vec_add( halfTheta, halfTheta ) );
    
    // For small sinTheta: ArcSin( sinTheta ) ~= sinTheta
    const vector float ratio( vec_min( vec_mul( vec_and( sinTheta, abs_mask ), g_ArcSinSmallThetaRatio.m_V ), one ) );
    return vec_madd( vec_sub( theta, sinTheta ), ratio, sinTheta );
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 minus_one( Const< __vector4 >::MinusOne() );
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 sign_mask( Const< __vector4 >::SignMask() );
    
    const __vector4 sinTheta( __vminfp( __vmaxfp( in_sinTheta, minus_one ), one ) );
    
    // Fundamental identity: Cos( theta ) = Sqrt( 1 - Sin^2( theta ) )
    
    //const __vector4 cosTheta( SIMD_Sqrt<22>( __vnmsubfp( sinTheta, sinTheta, one ) ) );
    const __vector4 cosTheta( SIMD_Sqrt<24>( __vnmsubfp( sinTheta, sinTheta, one ) ) );
    
    // Half-angle formula: Sin^2( theta / 2 ) = ( 1 - Cos( theta ) ) / 2
    //                     Sin( theta / 2 ) = Sqrt( ( 1 - Cos( theta ) ) / 2 )
    
    //const __vector4 z1( __vsel( SIMD_Sqrt<22>( __vnmsubfp( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    const __vector4 z1( __vsel( SIMD_Sqrt<24>( __vnmsubfp( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    
    const __vector4 z2( __vmulfp( z1, z1 ) ); // z2: z1 + 12     --> 12
    const __vector4 z3( __vmulfp( z2, z1 ) ); // z3: z2 + 12     --> 24
    const __vector4 z4( __vmulfp( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const __vector4 z5( __vmulfp( z3, z2 ) ); // z5: z3 + 12     --> 36
    const __vector4 z6( __vmulfp( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const __vector4 z7( __vmulfp( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const __vector4 z8( __vmulfp( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const __vector4 z_01_05( __vmrghw( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const __vector4 z_03_07( __vmrghw( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const __vector4 z_01_03_05_07( __vmrghw( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const __vector4 z12( __vmulfp( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const __vector4 z16( __vmulfp( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const __vector4 z24( __vmulfp( z12, z12 ) );                     // z24: z12 + 12 --> 61
    
    const __vector4 z_09_11_13_15( __vmulfp( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const __vector4 z_17_19_21_23( __vmulfp( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const __vector4 z_25_27_29_31( __vmulfp( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    
    __vector4 const * const __restrict p_Const_ArcSinSeries = (__vector4*)g_Const_ArcSinSeries;
    const __vector4 k_00_01_02_03( p_Const_ArcSinSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_ArcSinSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_ArcSinSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_ArcSinSeries[3] );
    
    const __vector4 halfTheta( __vaddfp( __vaddfp( __vmsum4fp( z_01_03_05_07, k_00_01_02_03 ),
                                                   __vmsum4fp( z_09_11_13_15, k_04_05_06_07 ) ),
                                         __vaddfp( __vmsum4fp( z_17_19_21_23, k_08_09_10_11 ),
                                                   __vmsum4fp( z_25_27_29_31, k_12_13_14_15 ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const __vector4 theta( __vaddfp( halfTheta, halfTheta ) );
    
    // For small sinTheta: ArcSin( sinTheta ) ~= sinTheta
    const __vector4 ratio( __vminfp( __vmulfp( __vand( sinTheta, abs_mask ), g_ArcSinSmallThetaRatio.m_V ), one ) );
    return __vmaddfp( __vsubfp( theta, sinTheta ), ratio, sinTheta );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}

/// SIMD Vector ArcSin: Using power series expansion
NOALIAS const V128 SIMD_VectorArcSin( const V128 PCREF in_sinTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcSin
    //
    // ArcSin(z) == Sum[k=0,inf] { z^(2k+1) . Pochhammer( 0.5, k ) / (2k+1)k! }
    
    #if defined( _SPU_ )
    
    const vector float half( Const< vector float >::Half() );
    const vector float one( Const< vector float >::One() );
    const vector float minus_one( Const< vector float >::MinusOne() );
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float sinTheta( spu_min( spu_max( in_sinTheta, minus_one ), one ) );
    
    // Fundamental identity: Cos( theta ) = Sqrt( 1 - Sin^2( theta ) )
    
    //const vector float cosTheta( SIMD_Sqrt<22>( spu_nmsub( sinTheta, sinTheta, one ) ) );
    const vector float cosTheta( SIMD_Sqrt<24>( spu_nmsub( sinTheta, sinTheta, one ) ) );
    
    // Half-angle formula: Sin^2( theta / 2 ) = ( 1 - Cos( theta ) ) / 2
    //                     Sin( theta / 2 ) = Sqrt( ( 1 - Cos( theta ) ) / 2 )
    
    //const vector float z1( spu_sel( SIMD_Sqrt<22>( spu_nmsub( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    const vector float z1( spu_sel( SIMD_Sqrt<24>( spu_nmsub( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    
    const vector float z2( spu_mul( z1, z1 ) );
    const vector float z3( spu_mul( z2, z1 ) );
    const vector float z4( spu_mul( z2, z2 ) );
    const vector float z5( spu_mul( z3, z2 ) );
    const vector float z6( spu_mul( z3, z3 ) );
    const vector float z7( spu_mul( z4, z3 ) );
    const vector float z8( spu_mul( z4, z4 ) );
    const vector float z9( spu_mul( z5, z4 ) );
    const vector float z10( spu_mul( z5, z5 ) );
    const vector float z11( spu_mul( z6, z5 ) );
    const vector float z12( spu_mul( z6, z6 ) );
    const vector float z13( spu_mul( z7, z6 ) );
    const vector float z14( spu_mul( z7, z7 ) );
    const vector float z15( spu_mul( z8, z7 ) );
    const vector float z16( spu_mul( z8, z8 ) );
    const vector float z17( spu_mul( z9, z8 ) );
  //const vector float z18( spu_mul( z9, z9 ) );
    const vector float z19( spu_mul( z10, z9 ) );
  //const vector float z20( spu_mul( z10, z10 ) );
    const vector float z21( spu_mul( z11, z10 ) );
  //const vector float z22( spu_mul( z11, z11 ) );
    const vector float z23( spu_mul( z12, z11 ) );
  //const vector float z24( spu_mul( z12, z12 ) );
    const vector float z25( spu_mul( z13, z12 ) );
  //const vector float z26( spu_mul( z13, z13 ) );
    const vector float z27( spu_mul( z14, z13 ) );
  //const vector float z28( spu_mul( z14, z14 ) );
    const vector float z29( spu_mul( z15, z14 ) );
  //const vector float z30( spu_mul( z15, z15 ) );
    const vector float z31( spu_mul( z16, z15 ) );
    
    vector float const * const __restrict p_Const_ArcSinSeries = (vector float*)g_Const_ArcSinSeries;
    const vector float k_00_01_02_03( p_Const_ArcSinSeries[0] );
    const vector float k_04_05_06_07( p_Const_ArcSinSeries[1] );
    const vector float k_08_09_10_11( p_Const_ArcSinSeries[2] );
    const vector float k_12_13_14_15( p_Const_ArcSinSeries[3] );
    
    const vector float halfTheta( spu_add( spu_add( spu_add( spu_madd( z1,  spu_splat<0>( k_00_01_02_03 ),
                                                              spu_mul( z3,  spu_splat<1>( k_00_01_02_03 ) ) ),
                                                             spu_madd( z5,  spu_splat<2>( k_00_01_02_03 ),
                                                              spu_mul( z7,  spu_splat<3>( k_00_01_02_03 ) ) ) ),
                                                    spu_add( spu_madd( z9,  spu_splat<0>( k_04_05_06_07 ),
                                                              spu_mul( z11, spu_splat<1>( k_04_05_06_07 ) ) ),
                                                             spu_madd( z13, spu_splat<2>( k_04_05_06_07 ),
                                                              spu_mul( z15, spu_splat<3>( k_04_05_06_07 ) ) ) ) ),
                                           spu_add( spu_add( spu_madd( z17, spu_splat<0>( k_08_09_10_11 ),
                                                              spu_mul( z19, spu_splat<1>( k_08_09_10_11 ) ) ),
                                                             spu_madd( z21, spu_splat<2>( k_08_09_10_11 ),
                                                              spu_mul( z23, spu_splat<3>( k_08_09_10_11 ) ) ) ),
                                                    spu_add( spu_madd( z25, spu_splat<0>( k_12_13_14_15 ),
                                                              spu_mul( z27, spu_splat<1>( k_12_13_14_15 ) ) ),
                                                             spu_madd( z29, spu_splat<2>( k_12_13_14_15 ),
                                                              spu_mul( z31, spu_splat<3>( k_12_13_14_15 ) ) ) ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const vector float theta( spu_add( halfTheta, halfTheta ) );
    
    // For small sinTheta: ArcSin( sinTheta ) ~= sinTheta
    const vector float ratio( spu_min( spu_mul( spu_and( sinTheta, abs_mask ), g_ArcSinSmallThetaRatio.m_V ), one ) );
    return spu_madd( spu_sub( theta, sinTheta ), ratio, sinTheta );
    
    #elif defined( _PPU_ )
    
    const vector float half( Const< vector float >::Half() );
    const vector float one( Const< vector float >::One() );
    const vector float minus_one( Const< vector float >::MinusOne() );
    const vector float abs_mask( Const< vector float >::AbsMask() );
    const vector unsigned int sign_mask( Const< vector unsigned int >::SignMask() );
    
    const vector float sinTheta( vec_min( vec_max( in_sinTheta, minus_one ), one ) );
    
    // Fundamental identity: Cos( theta ) = Sqrt( 1 - Sin^2( theta ) )
    
    //const vector float cosTheta( SIMD_Sqrt<22>( vec_nmsub( sinTheta, sinTheta, one ) ) );
    const vector float cosTheta( SIMD_Sqrt<24>( vec_nmsub( sinTheta, sinTheta, one ) ) );
    
    // Half-angle formula: Sin^2( theta / 2 ) = ( 1 - Cos( theta ) ) / 2
    //                     Sin( theta / 2 ) = Sqrt( ( 1 - Cos( theta ) ) / 2 )
    
    //const vector float z1( vec_sel( SIMD_Sqrt<22>( vec_nmsub( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    const vector float z1( vec_sel( SIMD_Sqrt<24>( vec_nmsub( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    
    const vector float z2( vec_mul( z1, z1 ) );
    const vector float z3( vec_mul( z2, z1 ) );
    const vector float z4( vec_mul( z2, z2 ) );
    const vector float z5( vec_mul( z3, z2 ) );
    const vector float z6( vec_mul( z3, z3 ) );
    const vector float z7( vec_mul( z4, z3 ) );
    const vector float z8( vec_mul( z4, z4 ) );
    const vector float z9( vec_mul( z5, z4 ) );
    const vector float z10( vec_mul( z5, z5 ) );
    const vector float z11( vec_mul( z6, z5 ) );
    const vector float z12( vec_mul( z6, z6 ) );
    const vector float z13( vec_mul( z7, z6 ) );
    const vector float z14( vec_mul( z7, z7 ) );
    const vector float z15( vec_mul( z8, z7 ) );
    const vector float z16( vec_mul( z8, z8 ) );
    const vector float z17( vec_mul( z9, z8 ) );
  //const vector float z18( vec_mul( z9, z9 ) );
    const vector float z19( vec_mul( z10, z9 ) );
  //const vector float z20( vec_mul( z10, z10 ) );
    const vector float z21( vec_mul( z11, z10 ) );
  //const vector float z22( vec_mul( z11, z11 ) );
    const vector float z23( vec_mul( z12, z11 ) );
  //const vector float z24( vec_mul( z12, z12 ) );
    const vector float z25( vec_mul( z13, z12 ) );
  //const vector float z26( vec_mul( z13, z13 ) );
    const vector float z27( vec_mul( z14, z13 ) );
  //const vector float z28( vec_mul( z14, z14 ) );
    const vector float z29( vec_mul( z15, z14 ) );
  //const vector float z30( vec_mul( z15, z15 ) );
    const vector float z31( vec_mul( z16, z15 ) );
    
    vector float const * const __restrict p_Const_ArcSinSeries = (vector float*)g_Const_ArcSinSeries;
    const vector float k_00_01_02_03( p_Const_ArcSinSeries[0] );
    const vector float k_04_05_06_07( p_Const_ArcSinSeries[1] );
    const vector float k_08_09_10_11( p_Const_ArcSinSeries[2] );
    const vector float k_12_13_14_15( p_Const_ArcSinSeries[3] );
    
    const vector float halfTheta( vec_add( vec_add( vec_add( vec_madd( z1,  vec_splat( k_00_01_02_03, 0 ),
                                                              vec_mul( z3,  vec_splat( k_00_01_02_03, 1 ) ) ),
                                                             vec_madd( z5,  vec_splat( k_00_01_02_03, 2 ),
                                                              vec_mul( z7,  vec_splat( k_00_01_02_03, 3 ) ) ) ),
                                                    vec_add( vec_madd( z9,  vec_splat( k_04_05_06_07, 0 ),
                                                              vec_mul( z11, vec_splat( k_04_05_06_07, 1 ) ) ),
                                                             vec_madd( z13, vec_splat( k_04_05_06_07, 2 ),
                                                              vec_mul( z15, vec_splat( k_04_05_06_07, 3 ) ) ) ) ),
                                           vec_add( vec_add( vec_madd( z17, vec_splat( k_08_09_10_11, 0 ),
                                                              vec_mul( z19, vec_splat( k_08_09_10_11, 1 ) ) ),
                                                             vec_madd( z21, vec_splat( k_08_09_10_11, 2 ),
                                                              vec_mul( z23, vec_splat( k_08_09_10_11, 3 ) ) ) ),
                                                    vec_add( vec_madd( z25, vec_splat( k_12_13_14_15, 0 ),
                                                              vec_mul( z27, vec_splat( k_12_13_14_15, 1 ) ) ),
                                                             vec_madd( z29, vec_splat( k_12_13_14_15, 2 ),
                                                              vec_mul( z31, vec_splat( k_12_13_14_15, 3 ) ) ) ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const vector float theta( vec_add( halfTheta, halfTheta ) );
    
    // For small sinTheta: ArcSin( sinTheta ) ~= sinTheta
    const vector float ratio( vec_min( vec_mul( vec_and( sinTheta, abs_mask ), g_ArcSinSmallThetaRatio.m_V ), one ) );
    return vec_madd( vec_sub( theta, sinTheta ), ratio, sinTheta );
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 minus_one( Const< __vector4 >::MinusOne() );
    const __vector4 abs_mask( Const< __vector4 >::AbsMask() );
    const __vector4 sign_mask( Const< __vector4 >::SignMask() );
    
    const __vector4 sinTheta( __vminfp( __vmaxfp( in_sinTheta, minus_one ), one ) );
    
    // Fundamental identity: Cos( theta ) = Sqrt( 1 - Sin^2( theta ) )
    
    //const __vector4 cosTheta( SIMD_Sqrt<22>( __vnmsubfp( sinTheta, sinTheta, one ) ) );
    const __vector4 cosTheta( SIMD_Sqrt<24>( __vnmsubfp( sinTheta, sinTheta, one ) ) );
    
    // Half-angle formula: Sin^2( theta / 2 ) = ( 1 - Cos( theta ) ) / 2
    //                     Sin( theta / 2 ) = Sqrt( ( 1 - Cos( theta ) ) / 2 )
    
    //const __vector4 z1( __vsel( SIMD_Sqrt<22>( __vnmsubfp( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    const __vector4 z1( __vsel( SIMD_Sqrt<24>( __vnmsubfp( cosTheta, half, half ) ), sinTheta, sign_mask ) );
    
    const __vector4 z2( __vmulfp( z1, z1 ) );
    const __vector4 z3( __vmulfp( z2, z1 ) );
    const __vector4 z4( __vmulfp( z2, z2 ) );
    const __vector4 z5( __vmulfp( z3, z2 ) );
    const __vector4 z6( __vmulfp( z3, z3 ) );
    const __vector4 z7( __vmulfp( z4, z3 ) );
    const __vector4 z8( __vmulfp( z4, z4 ) );
    const __vector4 z9( __vmulfp( z5, z4 ) );
    const __vector4 z10( __vmulfp( z5, z5 ) );
    const __vector4 z11( __vmulfp( z6, z5 ) );
    const __vector4 z12( __vmulfp( z6, z6 ) );
    const __vector4 z13( __vmulfp( z7, z6 ) );
    const __vector4 z14( __vmulfp( z7, z7 ) );
    const __vector4 z15( __vmulfp( z8, z7 ) );
    const __vector4 z16( __vmulfp( z8, z8 ) );
    const __vector4 z17( __vmulfp( z9, z8 ) );
  //const __vector4 z18( __vmulfp( z9, z9 ) );
    const __vector4 z19( __vmulfp( z10, z9 ) );
  //const __vector4 z20( __vmulfp( z10, z10 ) );
    const __vector4 z21( __vmulfp( z11, z10 ) );
  //const __vector4 z22( __vmulfp( z11, z11 ) );
    const __vector4 z23( __vmulfp( z12, z11 ) );
  //const __vector4 z24( __vmulfp( z12, z12 ) );
    const __vector4 z25( __vmulfp( z13, z12 ) );
  //const __vector4 z26( __vmulfp( z13, z13 ) );
    const __vector4 z27( __vmulfp( z14, z13 ) );
  //const __vector4 z28( __vmulfp( z14, z14 ) );
    const __vector4 z29( __vmulfp( z15, z14 ) );
  //const __vector4 z30( __vmulfp( z15, z15 ) );
    const __vector4 z31( __vmulfp( z16, z15 ) );
    
    __vector4 const * const __restrict p_Const_ArcSinSeries = (__vector4*)g_Const_ArcSinSeries;
    const __vector4 k_00_01_02_03( p_Const_ArcSinSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_ArcSinSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_ArcSinSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_ArcSinSeries[3] );
    
    const __vector4 halfTheta( __vaddfp( __vaddfp( __vaddfp( __vmaddfp( z1,  __vspltw( k_00_01_02_03, 0 ),
                                                              __vmulfp( z3,  __vspltw( k_00_01_02_03, 1 ) ) ),
                                                             __vmaddfp( z5,  __vspltw( k_00_01_02_03, 2 ),
                                                              __vmulfp( z7,  __vspltw( k_00_01_02_03, 3 ) ) ) ),
                                                   __vaddfp( __vmaddfp( z9,  __vspltw( k_04_05_06_07, 0 ),
                                                              __vmulfp( z11, __vspltw( k_04_05_06_07, 1 ) ) ),
                                                             __vmaddfp( z13, __vspltw( k_04_05_06_07, 2 ),
                                                              __vmulfp( z15, __vspltw( k_04_05_06_07, 3 ) ) ) ) ),
                                         __vaddfp( __vaddfp( __vmaddfp( z17, __vspltw( k_08_09_10_11, 0 ),
                                                              __vmulfp( z19, __vspltw( k_08_09_10_11, 1 ) ) ),
                                                             __vmaddfp( z21, __vspltw( k_08_09_10_11, 2 ),
                                                              __vmulfp( z23, __vspltw( k_08_09_10_11, 3 ) ) ) ),
                                                   __vaddfp( __vmaddfp( z25, __vspltw( k_12_13_14_15, 0 ),
                                                              __vmulfp( z27, __vspltw( k_12_13_14_15, 1 ) ) ),
                                                             __vmaddfp( z29, __vspltw( k_12_13_14_15, 2 ),
                                                              __vmulfp( z31, __vspltw( k_12_13_14_15, 3 ) ) ) ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const __vector4 theta( __vaddfp( halfTheta, halfTheta ) );
    
    // For small sinTheta: ArcSin( sinTheta ) ~= sinTheta
    const __vector4 ratio( __vminfp( __vmulfp( __vand( sinTheta, abs_mask ), g_ArcSinSmallThetaRatio.m_V ), one ) );
    return __vmaddfp( __vsubfp( theta, sinTheta ), ratio, sinTheta );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}



static const Vec4F32 g_ArcTanSmallThetaRatio = MakeVec4F32( F32( 1.0 / 0.0225 ), F32( 1.0 / 0.0225 ), F32( 1.0 / 0.0225 ), F32( 1.0 / 0.0225 ) );
static const Vec4F32 g_ArcTanSmallThetaLimit = MakeVec4F32( F32( 0.0225 ), F32( 0.0225 ), F32( 0.0225 ), F32( 0.0225 ) );

/// SIMD Scalar ArcTan: Using power series expansion
NOALIAS const V128 SIMD_ScalarArcTan( const V128 PCREF tanTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcTan
    //
    // ArcTan(z) ==                  Sum[k=0,inf] { (-1)^k . z^(2k+1) / (2k+1) } : |z| < 1
    // ArcTan(z) == PI/2 . Sign(z) - Sum[k=0,inf] { (-1)^k / z^(2k+1) / (2k+1) } : |z| > 1
    
    #if defined( _SPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float absMask( Const< vector float >::AbsMask() );
    const vector unsigned int signMask( Const< vector unsigned int >::SignMask() );
    
    // 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta ),
    // Cos( theta ) = 1 / sqrt( 1 + Tan^2( theta ) )
    
    const vector float onePlusTanSquaredTheta( spu_madd( tanTheta, tanTheta, one ) );
    const vector float cosTheta( SIMD_RcpSqrt<24>( onePlusTanSquaredTheta ) );
    
    // Tan^2( halfTheta ) = Sin^2( halfTheta ) / Cos^2( halfTheta ),
    // using Cos( 2x ) = 1 - 2 Sin^2( x ), Sin^2( x ) = ( 1 - Cos( 2x ) ) / 2,
    //       Cos( 2x ) = 2 Cos^2( x ) - 1, Cos^2( x ) = ( 1 + Cos( 2x ) ) / 2,
    // Tan^2( halfTheta ) = ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ),
    // Tan( halfTheta ) = sqrt( ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ) )
    
    const vector float tanHalfTheta( SIMD_Sqrt<24>( spu_mul( spu_sub( one, cosTheta ), SIMD_Rcp<24>( spu_add( one, cosTheta ) ) ) ) );
    
    const vector float z1( tanHalfTheta );
    const vector float z2( spu_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( spu_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( spu_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( spu_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( spu_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( spu_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( spu_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    const vector float z_01_05( spu_merge_xy( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const vector float z_03_07( spu_merge_xy( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const vector float z_01_03_05_07( spu_merge_xy( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const vector float z10( spu_mul( z5, z5 ) );                       // z10: z5  + 12 --> 48
    const vector float z12( spu_mul( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const vector float z14( spu_mul( z7, z7 ) );                       // z14: z7  + 12 --> 50
    const vector float z16( spu_mul( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const vector float z20( spu_mul( z10, z10 ) );                     // z20: z10 + 12 --> 52 [50+]
    const vector float z24( spu_mul( z12, z12 ) );                     // z24: z12 + 12 --> 61
    const vector float z28( spu_mul( z14, z14 ) );                     // z28: z14 + 12 --> 62
    const vector float z32( spu_mul( z16, z16 ) );                     // z32: z16 + 12 --> 63
    const vector float z40( spu_mul( z20, z20 ) );                     // z40: z20 + 12 --> 64
    const vector float z48( spu_mul( z24, z24 ) );                     // z48: z24 + 12 --> 73
    const vector float z56( spu_mul( z28, z28 ) );                     // z56: z28 + 12 --> 74
    
    const vector float z_09_11_13_15( spu_mul( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const vector float z_17_19_21_23( spu_mul( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const vector float z_25_27_29_31( spu_mul( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    const vector float z_33_35_37_39( spu_mul( z_01_03_05_07, z32 ) ); // z32      + 12 --> 
    const vector float z_41_43_45_47( spu_mul( z_01_03_05_07, z40 ) ); // z40      + 12 --> 
    const vector float z_49_51_53_55( spu_mul( z_01_03_05_07, z48 ) ); // z48      + 12 --> 
    const vector float z_57_59_61_63( spu_mul( z_01_03_05_07, z56 ) ); // z56      + 12 --> 
    
    vector float const * const __restrict p_Const_ArcTanSeries = (vector float*)g_Const_ArcTanSeries;
    const vector float k_00_01_02_03( p_Const_ArcTanSeries[0] );
    const vector float k_04_05_06_07( p_Const_ArcTanSeries[1] );
    const vector float k_08_09_10_11( p_Const_ArcTanSeries[2] );
    const vector float k_12_13_14_15( p_Const_ArcTanSeries[3] );
    const vector float k_16_17_18_19( p_Const_ArcTanSeries[4] );
    const vector float k_20_21_22_23( p_Const_ArcTanSeries[5] );
    const vector float k_24_25_26_27( p_Const_ArcTanSeries[6] );
    const vector float k_28_29_30_31( p_Const_ArcTanSeries[7] );
    
    const vector float halfTheta( spu_add( spu_add( spu_add( spu_dot_product4( z_01_03_05_07, k_00_01_02_03 ),
                                                             spu_dot_product4( z_09_11_13_15, k_04_05_06_07 ) ),
                                                    spu_add( spu_dot_product4( z_17_19_21_23, k_08_09_10_11 ),
                                                             spu_dot_product4( z_25_27_29_31, k_12_13_14_15 ) ) ),
                                           spu_add( spu_add( spu_dot_product4( z_33_35_37_39, k_16_17_18_19 ),
                                                             spu_dot_product4( z_41_43_45_47, k_20_21_22_23 ) ),
                                                    spu_add( spu_dot_product4( z_49_51_53_55, k_24_25_26_27 ),
                                                             spu_dot_product4( z_57_59_61_63, k_28_29_30_31 ) ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const vector float theta( spu_sel( spu_add( halfTheta, halfTheta ), tanTheta, signMask ) );
    
    // For small tanTheta: ArcTan( tanTheta ) ~= tanTheta
    const vector float absTanTheta( spu_and( tanTheta, absMask ) );
    const vector float smallThetaRatio0( spu_min( spu_mul( absTanTheta, g_ArcTanSmallThetaRatio.m_V ), one ) );
    const vector float smallThetaRatio( spu_mul( smallThetaRatio0, smallThetaRatio0 ) );
    const vector float smallThetaTendsTo( tanTheta );
    const vector float smallTheta( spu_madd( spu_sub( theta, smallThetaTendsTo ), smallThetaRatio, smallThetaTendsTo ) );
    const vector float result( spu_sel( smallTheta, theta, spu_cmpgt( absTanTheta, g_ArcTanSmallThetaLimit.m_V ) ) );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float absMask( Const< vector float >::AbsMask() );
    const vector unsigned int signMask( Const< vector unsigned int >::SignMask() );
    
    // 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta ),
    // Cos( theta ) = 1 / sqrt( 1 + Tan^2( theta ) )
    
    const vector float onePlusTanSquaredTheta( vec_madd( tanTheta, tanTheta, one ) );
    const vector float cosTheta( SIMD_RcpSqrt<24>( onePlusTanSquaredTheta ) );
    
    // Tan^2( halfTheta ) = Sin^2( halfTheta ) / Cos^2( halfTheta ),
    // using Cos( 2x ) = 1 - 2 Sin^2( x ), Sin^2( x ) = ( 1 - Cos( 2x ) ) / 2,
    //       Cos( 2x ) = 2 Cos^2( x ) - 1, Cos^2( x ) = ( 1 + Cos( 2x ) ) / 2,
    // Tan^2( halfTheta ) = ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ),
    // Tan( halfTheta ) = sqrt( ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ) )
    
    const vector float tanHalfTheta( SIMD_Sqrt<24>( vec_mul( vec_sub( one, cosTheta ), SIMD_Rcp<24>( vec_add( one, cosTheta ) ) ) ) );
    
    const vector float z1( tanHalfTheta );
    const vector float z2( vec_mul( z1, z1 ) ); // z2: z1 + 12     --> 12
    const vector float z3( vec_mul( z2, z1 ) ); // z3: z2 + 12     --> 24
    const vector float z4( vec_mul( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const vector float z5( vec_mul( z3, z2 ) ); // z5: z3 + 12     --> 36
    const vector float z6( vec_mul( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const vector float z7( vec_mul( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const vector float z8( vec_mul( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    const vector float z_01_05( vec_merge_xy( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const vector float z_03_07( vec_merge_xy( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const vector float z_01_03_05_07( vec_merge_xy( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const vector float z10( vec_mul( z5, z5 ) );                       // z10: z5  + 12 --> 48
    const vector float z12( vec_mul( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const vector float z14( vec_mul( z7, z7 ) );                       // z14: z7  + 12 --> 50
    const vector float z16( vec_mul( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const vector float z20( vec_mul( z10, z10 ) );                     // z20: z10 + 12 --> 52 [50+]
    const vector float z24( vec_mul( z12, z12 ) );                     // z24: z12 + 12 --> 61
    const vector float z28( vec_mul( z14, z14 ) );                     // z28: z14 + 12 --> 62
    const vector float z32( vec_mul( z16, z16 ) );                     // z32: z16 + 12 --> 63
    const vector float z40( vec_mul( z20, z20 ) );                     // z40: z20 + 12 --> 64
    const vector float z48( vec_mul( z24, z24 ) );                     // z48: z24 + 12 --> 73
    const vector float z56( vec_mul( z28, z28 ) );                     // z56: z28 + 12 --> 74
    
    const vector float z_09_11_13_15( vec_mul( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const vector float z_17_19_21_23( vec_mul( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const vector float z_25_27_29_31( vec_mul( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    const vector float z_33_35_37_39( vec_mul( z_01_03_05_07, z32 ) ); // z32      + 12 --> 
    const vector float z_41_43_45_47( vec_mul( z_01_03_05_07, z40 ) ); // z40      + 12 --> 
    const vector float z_49_51_53_55( vec_mul( z_01_03_05_07, z48 ) ); // z48      + 12 --> 
    const vector float z_57_59_61_63( vec_mul( z_01_03_05_07, z56 ) ); // z56      + 12 --> 
    
    vector float const * const __restrict p_Const_ArcTanSeries = (vector float*)g_Const_ArcTanSeries;
    const vector float k_00_01_02_03( p_Const_ArcTanSeries[0] );
    const vector float k_04_05_06_07( p_Const_ArcTanSeries[1] );
    const vector float k_08_09_10_11( p_Const_ArcTanSeries[2] );
    const vector float k_12_13_14_15( p_Const_ArcTanSeries[3] );
    const vector float k_16_17_18_19( p_Const_ArcTanSeries[4] );
    const vector float k_20_21_22_23( p_Const_ArcTanSeries[5] );
    const vector float k_24_25_26_27( p_Const_ArcTanSeries[6] );
    const vector float k_28_29_30_31( p_Const_ArcTanSeries[7] );
    
    const vector float halfTheta( vec_add( vec_add( vec_add( vec_dot_product4( z_01_03_05_07, k_00_01_02_03 ),
                                                             vec_dot_product4( z_09_11_13_15, k_04_05_06_07 ) ),
                                                    vec_add( vec_dot_product4( z_17_19_21_23, k_08_09_10_11 ),
                                                             vec_dot_product4( z_25_27_29_31, k_12_13_14_15 ) ) ),
                                           vec_add( vec_add( vec_dot_product4( z_33_35_37_39, k_16_17_18_19 ),
                                                             vec_dot_product4( z_41_43_45_47, k_20_21_22_23 ) ),
                                                    vec_add( vec_dot_product4( z_49_51_53_55, k_24_25_26_27 ),
                                                             vec_dot_product4( z_57_59_61_63, k_28_29_30_31 ) ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const vector float theta( vec_sel( vec_add( halfTheta, halfTheta ), tanTheta, signMask ) );
    
    // For small tanTheta: ArcTan( tanTheta ) ~= tanTheta
    const vector float absTanTheta( vec_and( tanTheta, absMask ) );
    const vector float smallThetaRatio0( vec_min( vec_mul( absTanTheta, g_ArcTanSmallThetaRatio.m_V ), one ) );
    const vector float smallThetaRatio( vec_mul( smallThetaRatio0, smallThetaRatio0 ) );
    const vector float smallThetaTendsTo( tanTheta );
    const vector float smallTheta( vec_madd( vec_sub( theta, smallThetaTendsTo ), smallThetaRatio, smallThetaTendsTo ) );
    const vector float result( vec_sel( smallTheta, theta, vec_cmpgt( absTanTheta, g_ArcTanSmallThetaLimit.m_V ) ) );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 absMask( Const< __vector4 >::AbsMask() );
    const __vector4 signMask( Const< __vector4 >::SignMask() );
    
    // 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta ),
    // Cos( theta ) = 1 / sqrt( 1 + Tan^2( theta ) )
    
    const __vector4 onePlusTanSquaredTheta( __vmaddfp( tanTheta, tanTheta, one ) );
    const __vector4 cosTheta( SIMD_RcpSqrt<24>( onePlusTanSquaredTheta ) );
    
    // Tan^2( halfTheta ) = Sin^2( halfTheta ) / Cos^2( halfTheta ),
    // using Cos( 2x ) = 1 - 2 Sin^2( x ), Sin^2( x ) = ( 1 - Cos( 2x ) ) / 2,
    //       Cos( 2x ) = 2 Cos^2( x ) - 1, Cos^2( x ) = ( 1 + Cos( 2x ) ) / 2,
    // Tan^2( halfTheta ) = ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ),
    // Tan( halfTheta ) = sqrt( ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ) )
    
    const __vector4 tanHalfTheta( SIMD_Sqrt<24>( __vmulfp( __vsubfp( one, cosTheta ), SIMD_Rcp<24>( __vaddfp( one, cosTheta ) ) ) ) );
    
    const __vector4 z1( tanHalfTheta );
    const __vector4 z2( __vmulfp( z1, z1 ) ); // z2: z1 + 12     --> 12
    const __vector4 z3( __vmulfp( z2, z1 ) ); // z3: z2 + 12     --> 24
    const __vector4 z4( __vmulfp( z2, z2 ) ); // z4: z2 + 12 + 1 --> 25
    const __vector4 z5( __vmulfp( z3, z2 ) ); // z5: z3 + 12     --> 36
    const __vector4 z6( __vmulfp( z3, z3 ) ); // z6: z3 + 12 + 1 --> 37
    const __vector4 z7( __vmulfp( z4, z3 ) ); // z7: z4 + 12 + 2 --> 38
    const __vector4 z8( __vmulfp( z4, z4 ) ); // z8: z4 + 12 + 2 --> 39
    
    const __vector4 z_01_05( __vmrghw( z1, z5 ) );                   // z1z5:     z5   + 4 --> 40
    const __vector4 z_03_07( __vmrghw( z3, z7 ) );                   // z3z7:     z7   + 4 --> 42
    const __vector4 z_01_03_05_07( __vmrghw( z_01_05, z_03_07 ) );   // z1z3z5z7: z3z7 + 4 --> 46
    
    const __vector4 z10( __vmulfp( z5, z5 ) );                       // z10: z5  + 12 --> 48
    const __vector4 z12( __vmulfp( z6, z6 ) );                       // z12: z6  + 12 --> 49
    const __vector4 z14( __vmulfp( z7, z7 ) );                       // z14: z7  + 12 --> 50
    const __vector4 z16( __vmulfp( z8, z8 ) );                       // z16: z8  + 12 --> 51
    const __vector4 z20( __vmulfp( z10, z10 ) );                     // z20: z10 + 12 --> 52 [50+]
    const __vector4 z24( __vmulfp( z12, z12 ) );                     // z24: z12 + 12 --> 61
    const __vector4 z28( __vmulfp( z14, z14 ) );                     // z28: z14 + 12 --> 62
    const __vector4 z32( __vmulfp( z16, z16 ) );                     // z32: z16 + 12 --> 63
    const __vector4 z40( __vmulfp( z20, z20 ) );                     // z40: z20 + 12 --> 64
    const __vector4 z48( __vmulfp( z24, z24 ) );                     // z48: z24 + 12 --> 73
    const __vector4 z56( __vmulfp( z28, z28 ) );                     // z56: z28 + 12 --> 74
    
    const __vector4 z_09_11_13_15( __vmulfp( z_01_03_05_07, z8 ) );  // z1z3z5z7 + 12 --> 58 
    const __vector4 z_17_19_21_23( __vmulfp( z_01_03_05_07, z16 ) ); // z16      + 12 --> 63
    const __vector4 z_25_27_29_31( __vmulfp( z_01_03_05_07, z24 ) ); // z24      + 12 --> 73
    const __vector4 z_33_35_37_39( __vmulfp( z_01_03_05_07, z32 ) ); // z32      + 12 --> 
    const __vector4 z_41_43_45_47( __vmulfp( z_01_03_05_07, z40 ) ); // z40      + 12 --> 
    const __vector4 z_49_51_53_55( __vmulfp( z_01_03_05_07, z48 ) ); // z48      + 12 --> 
    const __vector4 z_57_59_61_63( __vmulfp( z_01_03_05_07, z56 ) ); // z56      + 12 --> 
    
    __vector4 const * const __restrict p_Const_ArcTanSeries = (__vector4*)g_Const_ArcTanSeries;
    const __vector4 k_00_01_02_03( p_Const_ArcTanSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_ArcTanSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_ArcTanSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_ArcTanSeries[3] );
    const __vector4 k_16_17_18_19( p_Const_ArcTanSeries[4] );
    const __vector4 k_20_21_22_23( p_Const_ArcTanSeries[5] );
    const __vector4 k_24_25_26_27( p_Const_ArcTanSeries[6] );
    const __vector4 k_28_29_30_31( p_Const_ArcTanSeries[7] );
    
    const __vector4 halfTheta( __vaddfp( __vaddfp( __vaddfp( __vmsum4fp( z_01_03_05_07, k_00_01_02_03 ),
                                                             __vmsum4fp( z_09_11_13_15, k_04_05_06_07 ) ),
                                                   __vaddfp( __vmsum4fp( z_17_19_21_23, k_08_09_10_11 ),
                                                             __vmsum4fp( z_25_27_29_31, k_12_13_14_15 ) ) ),
                                         __vaddfp( __vaddfp( __vmsum4fp( z_33_35_37_39, k_16_17_18_19 ),
                                                             __vmsum4fp( z_41_43_45_47, k_20_21_22_23 ) ),
                                                   __vaddfp( __vmsum4fp( z_49_51_53_55, k_24_25_26_27 ),
                                                             __vmsum4fp( z_57_59_61_63, k_28_29_30_31 ) ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const __vector4 theta( __vsel( __vaddfp( halfTheta, halfTheta ), tanTheta, signMask ) );
    
    // For small tanTheta: ArcTan( tanTheta ) ~= tanTheta
    const __vector4 absTanTheta( __vand( tanTheta, absMask ) );
    const __vector4 smallThetaRatio0( __vminfp( __vmulfp( absTanTheta, g_ArcTanSmallThetaRatio.m_V ), one ) );
    const __vector4 smallThetaRatio( __vmulfp( smallThetaRatio0, smallThetaRatio0 ) );
    const __vector4 smallThetaTendsTo( tanTheta );
    const __vector4 smallTheta( __vmaddfp( __vsubfp( theta, smallThetaTendsTo ), smallThetaRatio, smallThetaTendsTo ) );
    const __vector4 result( __vsel( smallTheta, theta, __vcmpgtfp( absTanTheta, g_ArcTanSmallThetaLimit.m_V ) ) );
    return result;
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}

/// SIMD Vector ArcTan: Using power series expansion
NOALIAS const V128 SIMD_VectorArcTan( const V128 PCREF tanTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcTan
    //
    // ArcTan(z) ==                  Sum[k=0,inf] { (-1)^k . z^(2k+1) / (2k+1) } : |z| < 1
    // ArcTan(z) == PI/2 . Sign(z) - Sum[k=0,inf] { (-1)^k / z^(2k+1) / (2k+1) } : |z| > 1
    
    #if defined( _SPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float absMask( Const< vector float >::AbsMask() );
    const vector unsigned int signMask( Const< vector unsigned int >::SignMask() );
    
    // 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta ),
    // Cos( theta ) = 1 / sqrt( 1 + Tan^2( theta ) )
    
    const vector float onePlusTanSquaredTheta( spu_madd( tanTheta, tanTheta, one ) );
    const vector float cosTheta( SIMD_RcpSqrt<24>( onePlusTanSquaredTheta ) );
    
    // Tan^2( halfTheta ) = Sin^2( halfTheta ) / Cos^2( halfTheta ),
    // using Cos( 2x ) = 1 - 2 Sin^2( x ), Sin^2( x ) = ( 1 - Cos( 2x ) ) / 2,
    //       Cos( 2x ) = 2 Cos^2( x ) - 1, Cos^2( x ) = ( 1 + Cos( 2x ) ) / 2,
    // Tan^2( halfTheta ) = ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ),
    // Tan( halfTheta ) = sqrt( ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ) )
    
    const vector float tanHalfTheta( SIMD_Sqrt<24>( spu_mul( spu_sub( one, cosTheta ), SIMD_Rcp<24>( spu_add( one, cosTheta ) ) ) ) );
    
    vector float const * const __restrict p_Const_ArcTanSeries = (vector float*)g_Const_ArcTanSeries;
    const vector float k_00_01_02_03( p_Const_ArcTanSeries[0] );
    const vector float k_04_05_06_07( p_Const_ArcTanSeries[1] );
    const vector float k_08_09_10_11( p_Const_ArcTanSeries[2] );
    const vector float k_12_13_14_15( p_Const_ArcTanSeries[3] );
    const vector float k_16_17_18_19( p_Const_ArcTanSeries[4] );
    const vector float k_20_21_22_23( p_Const_ArcTanSeries[5] );
    const vector float k_24_25_26_27( p_Const_ArcTanSeries[6] );
    const vector float k_28_29_30_31( p_Const_ArcTanSeries[7] );
    
    const vector float z1( tanHalfTheta );
    const vector float z2( spu_mul( z1, z1 ) );
    const vector float z3( spu_mul( z2, z1 ) );
    const vector float z4( spu_mul( z2, z2 ) );
    const vector float z5( spu_mul( z3, z2 ) );
    const vector float z6( spu_mul( z3, z3 ) );
    const vector float z7( spu_mul( z4, z3 ) );
    const vector float z8( spu_mul( z4, z4 ) );
    const vector float z9( spu_mul( z5, z4 ) );
    const vector float z10( spu_mul( z5, z5 ) );
    const vector float z11( spu_mul( z6, z5 ) );
    const vector float z12( spu_mul( z6, z6 ) );
    const vector float z13( spu_mul( z7, z6 ) );
    const vector float z14( spu_mul( z7, z7 ) );
    const vector float z15( spu_mul( z8, z7 ) );
    const vector float z16( spu_mul( z8, z8 ) );
    const vector float z17( spu_mul( z9, z8 ) );
    const vector float z18( spu_mul( z9, z9 ) );
    const vector float z19( spu_mul( z10, z9 ) );
    const vector float z20( spu_mul( z10, z10 ) );
    const vector float z21( spu_mul( z11, z10 ) );
    const vector float z22( spu_mul( z11, z11 ) );
    const vector float z23( spu_mul( z12, z11 ) );
    const vector float z24( spu_mul( z12, z12 ) );
    const vector float z25( spu_mul( z13, z12 ) );
    const vector float z26( spu_mul( z13, z13 ) );
    const vector float z27( spu_mul( z14, z13 ) );
    const vector float z28( spu_mul( z14, z14 ) );
    const vector float z29( spu_mul( z15, z14 ) );
    const vector float z30( spu_mul( z15, z15 ) );
    const vector float z31( spu_mul( z16, z15 ) );
    const vector float z32( spu_mul( z16, z16 ) );
    const vector float z33( spu_mul( z17, z16 ) );
    const vector float z35( spu_mul( z18, z17 ) );
    const vector float z37( spu_mul( z19, z18 ) );
    const vector float z39( spu_mul( z20, z19 ) );
    const vector float z41( spu_mul( z21, z20 ) );
    const vector float z43( spu_mul( z22, z21 ) );
    const vector float z45( spu_mul( z23, z22 ) );
    const vector float z47( spu_mul( z24, z23 ) );
    const vector float z49( spu_mul( z25, z24 ) );
    const vector float z51( spu_mul( z26, z25 ) );
    const vector float z53( spu_mul( z27, z26 ) );
    const vector float z55( spu_mul( z28, z27 ) );
    const vector float z57( spu_mul( z29, z28 ) );
    const vector float z59( spu_mul( z30, z29 ) );
    const vector float z61( spu_mul( z31, z30 ) );
    const vector float z63( spu_mul( z32, z31 ) );
    
    const vector float zk_01_03_05_07( spu_add( spu_madd( z1,  spu_splat<0>( k_00_01_02_03 ),
                                                spu_mul ( z3,  spu_splat<1>( k_00_01_02_03 ) ) ),
                                                spu_madd( z5,  spu_splat<2>( k_00_01_02_03 ),
                                                spu_mul ( z7,  spu_splat<3>( k_00_01_02_03 ) ) ) ) );
                                                                        
    const vector float zk_09_11_13_15( spu_add( spu_madd( z9,  spu_splat<0>( k_04_05_06_07 ),
                                                spu_mul ( z11, spu_splat<1>( k_04_05_06_07 ) ) ),
                                                spu_madd( z13, spu_splat<2>( k_04_05_06_07 ),
                                                spu_mul ( z15, spu_splat<3>( k_04_05_06_07 ) ) ) ) );
                                                                        
    const vector float zk_17_19_21_23( spu_add( spu_madd( z17, spu_splat<0>( k_08_09_10_11 ),
                                                spu_mul ( z19, spu_splat<1>( k_08_09_10_11 ) ) ),
                                                spu_madd( z21, spu_splat<2>( k_08_09_10_11 ),
                                                spu_mul ( z23, spu_splat<3>( k_08_09_10_11 ) ) ) ) );
                                                                        
    const vector float zk_25_27_29_31( spu_add( spu_madd( z25, spu_splat<0>( k_12_13_14_15 ),
                                                spu_mul ( z27, spu_splat<1>( k_12_13_14_15 ) ) ),
                                                spu_madd( z29, spu_splat<2>( k_12_13_14_15 ),
                                                spu_mul ( z31, spu_splat<3>( k_12_13_14_15 ) ) ) ) );
                                                                        
    const vector float zk_33_35_37_39( spu_add( spu_madd( z33, spu_splat<0>( k_16_17_18_19 ),
                                                spu_mul ( z35, spu_splat<1>( k_16_17_18_19 ) ) ),
                                                spu_madd( z37, spu_splat<2>( k_16_17_18_19 ),
                                                spu_mul ( z39, spu_splat<3>( k_16_17_18_19 ) ) ) ) );
                                                                        
    const vector float zk_41_43_45_47( spu_add( spu_madd( z41, spu_splat<0>( k_20_21_22_23 ),
                                                spu_mul ( z43, spu_splat<1>( k_20_21_22_23 ) ) ),
                                                spu_madd( z45, spu_splat<2>( k_20_21_22_23 ),
                                                spu_mul ( z47, spu_splat<3>( k_20_21_22_23 ) ) ) ) );
                                                                        
    const vector float zk_49_51_53_55( spu_add( spu_madd( z49, spu_splat<0>( k_24_25_26_27 ),
                                                spu_mul ( z51, spu_splat<1>( k_24_25_26_27 ) ) ),
                                                spu_madd( z53, spu_splat<2>( k_24_25_26_27 ),
                                                spu_mul ( z55, spu_splat<3>( k_24_25_26_27 ) ) ) ) );
                                                                        
    const vector float zk_57_59_61_63( spu_add( spu_madd( z57, spu_splat<0>( k_28_29_30_31 ),
                                                spu_mul ( z59, spu_splat<1>( k_28_29_30_31 ) ) ),
                                                spu_madd( z61, spu_splat<2>( k_28_29_30_31 ),
                                                spu_mul ( z63, spu_splat<3>( k_28_29_30_31 ) ) ) ) );
    
    const vector float halfTheta( spu_add( spu_add( spu_add( zk_01_03_05_07,
                                                             zk_09_11_13_15 ),
                                                    spu_add( zk_17_19_21_23,
                                                             zk_25_27_29_31 ) ),
                                           spu_add( spu_add( zk_33_35_37_39,
                                                             zk_41_43_45_47 ),
                                                    spu_add( zk_49_51_53_55,
                                                             zk_57_59_61_63 ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const vector float theta( spu_sel( spu_add( halfTheta, halfTheta ), tanTheta, signMask ) );
    
    // For small tanTheta: ArcTan( tanTheta ) ~= tanTheta
    const vector float absTanTheta( spu_and( tanTheta, absMask ) );
    const vector float smallThetaRatio0( spu_min( spu_mul( absTanTheta, g_ArcTanSmallThetaRatio.m_V ), one ) );
    const vector float smallThetaRatio( spu_mul( smallThetaRatio0, smallThetaRatio0 ) );
    const vector float smallThetaTendsTo( tanTheta );
    const vector float smallTheta( spu_madd( spu_sub( theta, smallThetaTendsTo ), smallThetaRatio, smallThetaTendsTo ) );
    const vector float result( spu_sel( smallTheta, theta, spu_cmpgt( absTanTheta, g_ArcTanSmallThetaLimit.m_V ) ) );
    return result;
    
    #elif defined( _PPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float absMask( Const< vector float >::AbsMask() );
    const vector unsigned int signMask( Const< vector unsigned int >::SignMask() );
    
    // 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta ),
    // Cos( theta ) = 1 / sqrt( 1 + Tan^2( theta ) )
    
    const vector float onePlusTanSquaredTheta( vec_madd( tanTheta, tanTheta, one ) );
    const vector float cosTheta( SIMD_RcpSqrt<24>( onePlusTanSquaredTheta ) );
    
    // Tan^2( halfTheta ) = Sin^2( halfTheta ) / Cos^2( halfTheta ),
    // using Cos( 2x ) = 1 - 2 Sin^2( x ), Sin^2( x ) = ( 1 - Cos( 2x ) ) / 2,
    //       Cos( 2x ) = 2 Cos^2( x ) - 1, Cos^2( x ) = ( 1 + Cos( 2x ) ) / 2,
    // Tan^2( halfTheta ) = ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ),
    // Tan( halfTheta ) = sqrt( ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ) )
    
    const vector float tanHalfTheta( SIMD_Sqrt<24>( vec_mul( vec_sub( one, cosTheta ), SIMD_Rcp<24>( vec_add( one, cosTheta ) ) ) ) );
    
    vector float const * const __restrict p_Const_ArcTanSeries = (vector float*)g_Const_ArcTanSeries;
    const vector float k_00_01_02_03( p_Const_ArcTanSeries[0] );
    const vector float k_04_05_06_07( p_Const_ArcTanSeries[1] );
    const vector float k_08_09_10_11( p_Const_ArcTanSeries[2] );
    const vector float k_12_13_14_15( p_Const_ArcTanSeries[3] );
    const vector float k_16_17_18_19( p_Const_ArcTanSeries[4] );
    const vector float k_20_21_22_23( p_Const_ArcTanSeries[5] );
    const vector float k_24_25_26_27( p_Const_ArcTanSeries[6] );
    const vector float k_28_29_30_31( p_Const_ArcTanSeries[7] );
    
    const vector float z1( tanHalfTheta );
    const vector float z2( vec_mul( z1, z1 ) );
    const vector float z3( vec_mul( z2, z1 ) );
    const vector float z4( vec_mul( z2, z2 ) );
    const vector float z5( vec_mul( z3, z2 ) );
    const vector float z6( vec_mul( z3, z3 ) );
    const vector float z7( vec_mul( z4, z3 ) );
    const vector float z8( vec_mul( z4, z4 ) );
    const vector float z9( vec_mul( z5, z4 ) );
    const vector float z10( vec_mul( z5, z5 ) );
    const vector float z11( vec_mul( z6, z5 ) );
    const vector float z12( vec_mul( z6, z6 ) );
    const vector float z13( vec_mul( z7, z6 ) );
    const vector float z14( vec_mul( z7, z7 ) );
    const vector float z15( vec_mul( z8, z7 ) );
    const vector float z16( vec_mul( z8, z8 ) );
    const vector float z17( vec_mul( z9, z8 ) );
    const vector float z18( vec_mul( z9, z9 ) );
    const vector float z19( vec_mul( z10, z9 ) );
    const vector float z20( vec_mul( z10, z10 ) );
    const vector float z21( vec_mul( z11, z10 ) );
    const vector float z22( vec_mul( z11, z11 ) );
    const vector float z23( vec_mul( z12, z11 ) );
    const vector float z24( vec_mul( z12, z12 ) );
    const vector float z25( vec_mul( z13, z12 ) );
    const vector float z26( vec_mul( z13, z13 ) );
    const vector float z27( vec_mul( z14, z13 ) );
    const vector float z28( vec_mul( z14, z14 ) );
    const vector float z29( vec_mul( z15, z14 ) );
    const vector float z30( vec_mul( z15, z15 ) );
    const vector float z31( vec_mul( z16, z15 ) );
    const vector float z32( vec_mul( z16, z16 ) );
    const vector float z33( vec_mul( z17, z16 ) );
    const vector float z35( vec_mul( z18, z17 ) );
    const vector float z37( vec_mul( z19, z18 ) );
    const vector float z39( vec_mul( z20, z19 ) );
    const vector float z41( vec_mul( z21, z20 ) );
    const vector float z43( vec_mul( z22, z21 ) );
    const vector float z45( vec_mul( z23, z22 ) );
    const vector float z47( vec_mul( z24, z23 ) );
    const vector float z49( vec_mul( z25, z24 ) );
    const vector float z51( vec_mul( z26, z25 ) );
    const vector float z53( vec_mul( z27, z26 ) );
    const vector float z55( vec_mul( z28, z27 ) );
    const vector float z57( vec_mul( z29, z28 ) );
    const vector float z59( vec_mul( z30, z29 ) );
    const vector float z61( vec_mul( z31, z30 ) );
    const vector float z63( vec_mul( z32, z31 ) );
    
    const vector float zk_01_03_05_07( vec_add( vec_madd( z1,  vec_splat( k_00_01_02_03, 0 ),
                                                vec_mul ( z3,  vec_splat( k_00_01_02_03, 1 ) ) ),
                                                vec_madd( z5,  vec_splat( k_00_01_02_03, 2 ),
                                                vec_mul ( z7,  vec_splat( k_00_01_02_03, 3 ) ) ) ) );
    
    const vector float zk_09_11_13_15( vec_add( vec_madd( z9,  vec_splat( k_04_05_06_07, 0 ),
                                                vec_mul ( z11, vec_splat( k_04_05_06_07, 1 ) ) ),
                                                vec_madd( z13, vec_splat( k_04_05_06_07, 2 ),
                                                vec_mul ( z15, vec_splat( k_04_05_06_07, 3 ) ) ) ) );
    
    const vector float zk_17_19_21_23( vec_add( vec_madd( z17, vec_splat( k_08_09_10_11, 0 ),
                                                vec_mul ( z19, vec_splat( k_08_09_10_11, 1 ) ) ),
                                                vec_madd( z21, vec_splat( k_08_09_10_11, 2 ),
                                                vec_mul ( z23, vec_splat( k_08_09_10_11, 3 ) ) ) ) );
    
    const vector float zk_25_27_29_31( vec_add( vec_madd( z25, vec_splat( k_12_13_14_15, 0 ),
                                                vec_mul ( z27, vec_splat( k_12_13_14_15, 1 ) ) ),
                                                vec_madd( z29, vec_splat( k_12_13_14_15, 2 ),
                                                vec_mul ( z31, vec_splat( k_12_13_14_15, 3 ) ) ) ) );
    
    const vector float zk_33_35_37_39( vec_add( vec_madd( z33, vec_splat( k_16_17_18_19, 0 ),
                                                vec_mul ( z35, vec_splat( k_16_17_18_19, 1 ) ) ),
                                                vec_madd( z37, vec_splat( k_16_17_18_19, 2 ),
                                                vec_mul ( z39, vec_splat( k_16_17_18_19, 3 ) ) ) ) );
    
    const vector float zk_41_43_45_47( vec_add( vec_madd( z41, vec_splat( k_20_21_22_23, 0 ),
                                                vec_mul ( z43, vec_splat( k_20_21_22_23, 1 ) ) ),
                                                vec_madd( z45, vec_splat( k_20_21_22_23, 2 ),
                                                vec_mul ( z47, vec_splat( k_20_21_22_23, 3 ) ) ) ) );
    
    const vector float zk_49_51_53_55( vec_add( vec_madd( z49, vec_splat( k_24_25_26_27, 0 ),
                                                vec_mul ( z51, vec_splat( k_24_25_26_27, 1 ) ) ),
                                                vec_madd( z53, vec_splat( k_24_25_26_27, 2 ),
                                                vec_mul ( z55, vec_splat( k_24_25_26_27, 3 ) ) ) ) );
    
    const vector float zk_57_59_61_63( vec_add( vec_madd( z57, vec_splat( k_28_29_30_31, 0 ),
                                                vec_mul ( z59, vec_splat( k_28_29_30_31, 1 ) ) ),
                                                vec_madd( z61, vec_splat( k_28_29_30_31, 2 ),
                                                vec_mul ( z63, vec_splat( k_28_29_30_31, 3 ) ) ) ) );
    
    const vector float halfTheta( vec_add( vec_add( vec_add( zk_01_03_05_07,
                                                             zk_09_11_13_15 ),
                                                    vec_add( zk_17_19_21_23,
                                                             zk_25_27_29_31 ) ),
                                           vec_add( vec_add( zk_33_35_37_39,
                                                             zk_41_43_45_47 ),
                                                    vec_add( zk_49_51_53_55,
                                                             zk_57_59_61_63 ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const vector float theta( vec_sel( vec_add( halfTheta, halfTheta ), tanTheta, signMask ) );
    
    // For small tanTheta: ArcTan( tanTheta ) ~= tanTheta
    const vector float absTanTheta( vec_and( tanTheta, absMask ) );
    const vector float smallThetaRatio0( vec_min( vec_mul( absTanTheta, g_ArcTanSmallThetaRatio.m_V ), one ) );
    const vector float smallThetaRatio( vec_mul( smallThetaRatio0, smallThetaRatio0 ) );
    const vector float smallThetaTendsTo( tanTheta );
    const vector float smallTheta( vec_madd( vec_sub( theta, smallThetaTendsTo ), smallThetaRatio, smallThetaTendsTo ) );
    const vector float result( vec_sel( smallTheta, theta, vec_cmpgt( absTanTheta, g_ArcTanSmallThetaLimit.m_V ) ) );
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 absMask( Const< __vector4 >::AbsMask() );
    const __vector4 signMask( Const< __vector4 >::SignMask() );
    
    // 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta ),
    // Cos( theta ) = 1 / sqrt( 1 + Tan^2( theta ) )
    
    const __vector4 onePlusTanSquaredTheta( __vmaddfp( tanTheta, tanTheta, one ) );
    const __vector4 cosTheta( SIMD_RcpSqrt<24>( onePlusTanSquaredTheta ) );
    
    // Tan^2( halfTheta ) = Sin^2( halfTheta ) / Cos^2( halfTheta ),
    // using Cos( 2x ) = 1 - 2 Sin^2( x ), Sin^2( x ) = ( 1 - Cos( 2x ) ) / 2,
    //       Cos( 2x ) = 2 Cos^2( x ) - 1, Cos^2( x ) = ( 1 + Cos( 2x ) ) / 2,
    // Tan^2( halfTheta ) = ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ),
    // Tan( halfTheta ) = sqrt( ( 1 - Cos( theta ) ) / ( 1 + Cos( theta ) ) )
    
    const __vector4 tanHalfTheta( SIMD_Sqrt<24>( __vmulfp( __vsubfp( one, cosTheta ), SIMD_Rcp<24>( __vaddfp( one, cosTheta ) ) ) ) );
    
    __vector4 const * const __restrict p_Const_ArcTanSeries = (__vector4*)g_Const_ArcTanSeries;
    const __vector4 k_00_01_02_03( p_Const_ArcTanSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_ArcTanSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_ArcTanSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_ArcTanSeries[3] );
    const __vector4 k_16_17_18_19( p_Const_ArcTanSeries[4] );
    const __vector4 k_20_21_22_23( p_Const_ArcTanSeries[5] );
    const __vector4 k_24_25_26_27( p_Const_ArcTanSeries[6] );
    const __vector4 k_28_29_30_31( p_Const_ArcTanSeries[7] );
    
    const __vector4 z1( tanHalfTheta );
    const __vector4 z2( __vmulfp( z1, z1 ) );
    const __vector4 z3( __vmulfp( z2, z1 ) );
    const __vector4 z4( __vmulfp( z2, z2 ) );
    const __vector4 z5( __vmulfp( z3, z2 ) );
    const __vector4 z6( __vmulfp( z3, z3 ) );
    const __vector4 z7( __vmulfp( z4, z3 ) );
    const __vector4 z8( __vmulfp( z4, z4 ) );
    const __vector4 z9( __vmulfp( z5, z4 ) );
    const __vector4 z10( __vmulfp( z5, z5 ) );
    const __vector4 z11( __vmulfp( z6, z5 ) );
    const __vector4 z12( __vmulfp( z6, z6 ) );
    const __vector4 z13( __vmulfp( z7, z6 ) );
    const __vector4 z14( __vmulfp( z7, z7 ) );
    const __vector4 z15( __vmulfp( z8, z7 ) );
    const __vector4 z16( __vmulfp( z8, z8 ) );
    const __vector4 z17( __vmulfp( z9, z8 ) );
    const __vector4 z18( __vmulfp( z9, z9 ) );
    const __vector4 z19( __vmulfp( z10, z9 ) );
    const __vector4 z20( __vmulfp( z10, z10 ) );
    const __vector4 z21( __vmulfp( z11, z10 ) );
    const __vector4 z22( __vmulfp( z11, z11 ) );
    const __vector4 z23( __vmulfp( z12, z11 ) );
    const __vector4 z24( __vmulfp( z12, z12 ) );
    const __vector4 z25( __vmulfp( z13, z12 ) );
    const __vector4 z26( __vmulfp( z13, z13 ) );
    const __vector4 z27( __vmulfp( z14, z13 ) );
    const __vector4 z28( __vmulfp( z14, z14 ) );
    const __vector4 z29( __vmulfp( z15, z14 ) );
    const __vector4 z30( __vmulfp( z15, z15 ) );
    const __vector4 z31( __vmulfp( z16, z15 ) );
    const __vector4 z32( __vmulfp( z16, z16 ) );
    const __vector4 z33( __vmulfp( z17, z16 ) );
    const __vector4 z35( __vmulfp( z18, z17 ) );
    const __vector4 z37( __vmulfp( z19, z18 ) );
    const __vector4 z39( __vmulfp( z20, z19 ) );
    const __vector4 z41( __vmulfp( z21, z20 ) );
    const __vector4 z43( __vmulfp( z22, z21 ) );
    const __vector4 z45( __vmulfp( z23, z22 ) );
    const __vector4 z47( __vmulfp( z24, z23 ) );
    const __vector4 z49( __vmulfp( z25, z24 ) );
    const __vector4 z51( __vmulfp( z26, z25 ) );
    const __vector4 z53( __vmulfp( z27, z26 ) );
    const __vector4 z55( __vmulfp( z28, z27 ) );
    const __vector4 z57( __vmulfp( z29, z28 ) );
    const __vector4 z59( __vmulfp( z30, z29 ) );
    const __vector4 z61( __vmulfp( z31, z30 ) );
    const __vector4 z63( __vmulfp( z32, z31 ) );
    
    const __vector4 zk_01_03_05_07( __vaddfp( __vmaddfp( z1,  __vspltw( k_00_01_02_03, 0 ),
                                               __vmulfp( z3,  __vspltw( k_00_01_02_03, 1 ) ) ),
                                              __vmaddfp( z5,  __vspltw( k_00_01_02_03, 2 ),
                                               __vmulfp( z7,  __vspltw( k_00_01_02_03, 3 ) ) ) ) );
    
    const __vector4 zk_09_11_13_15( __vaddfp( __vmaddfp( z9,  __vspltw( k_04_05_06_07, 0 ),
                                               __vmulfp( z11, __vspltw( k_04_05_06_07, 1 ) ) ),
                                              __vmaddfp( z13, __vspltw( k_04_05_06_07, 2 ),
                                               __vmulfp( z15, __vspltw( k_04_05_06_07, 3 ) ) ) ) );
    
    const __vector4 zk_17_19_21_23( __vaddfp( __vmaddfp( z17, __vspltw( k_08_09_10_11, 0 ),
                                               __vmulfp( z19, __vspltw( k_08_09_10_11, 1 ) ) ),
                                              __vmaddfp( z21, __vspltw( k_08_09_10_11, 2 ),
                                               __vmulfp( z23, __vspltw( k_08_09_10_11, 3 ) ) ) ) );
    
    const __vector4 zk_25_27_29_31( __vaddfp( __vmaddfp( z25, __vspltw( k_12_13_14_15, 0 ),
                                               __vmulfp( z27, __vspltw( k_12_13_14_15, 1 ) ) ),
                                              __vmaddfp( z29, __vspltw( k_12_13_14_15, 2 ),
                                               __vmulfp( z31, __vspltw( k_12_13_14_15, 3 ) ) ) ) );
    
    const __vector4 zk_33_35_37_39( __vaddfp( __vmaddfp( z33, __vspltw( k_16_17_18_19, 0 ),
                                               __vmulfp( z35, __vspltw( k_16_17_18_19, 1 ) ) ),
                                              __vmaddfp( z37, __vspltw( k_16_17_18_19, 2 ),
                                               __vmulfp( z39, __vspltw( k_16_17_18_19, 3 ) ) ) ) );
    
    const __vector4 zk_41_43_45_47( __vaddfp( __vmaddfp( z41, __vspltw( k_20_21_22_23, 0 ),
                                               __vmulfp( z43, __vspltw( k_20_21_22_23, 1 ) ) ),
                                              __vmaddfp( z45, __vspltw( k_20_21_22_23, 2 ),
                                               __vmulfp( z47, __vspltw( k_20_21_22_23, 3 ) ) ) ) );
    
    const __vector4 zk_49_51_53_55( __vaddfp( __vmaddfp( z49, __vspltw( k_24_25_26_27, 0 ),
                                               __vmulfp( z51, __vspltw( k_24_25_26_27, 1 ) ) ),
                                              __vmaddfp( z53, __vspltw( k_24_25_26_27, 2 ),
                                               __vmulfp( z55, __vspltw( k_24_25_26_27, 3 ) ) ) ) );
    
    const __vector4 zk_57_59_61_63( __vaddfp( __vmaddfp( z57, __vspltw( k_28_29_30_31, 0 ),
                                               __vmulfp( z59, __vspltw( k_28_29_30_31, 1 ) ) ),
                                              __vmaddfp( z61, __vspltw( k_28_29_30_31, 2 ),
                                               __vmulfp( z63, __vspltw( k_28_29_30_31, 3 ) ) ) ) );
    
    const __vector4 halfTheta( __vaddfp( __vaddfp( __vaddfp( zk_01_03_05_07,
                                                             zk_09_11_13_15 ),
                                                   __vaddfp( zk_17_19_21_23,
                                                             zk_25_27_29_31 ) ),
                                         __vaddfp( __vaddfp( zk_33_35_37_39,
                                                             zk_41_43_45_47 ),
                                                   __vaddfp( zk_49_51_53_55,
                                                             zk_57_59_61_63 ) ) ) );
    
    // TODO: Store muliply by two in the constants table!
    const __vector4 theta( __vsel( __vaddfp( halfTheta, halfTheta ), tanTheta, signMask ) );
    
    // For small tanTheta: ArcTan( tanTheta ) ~= tanTheta
    const __vector4 absTanTheta( __vand( tanTheta, absMask ) );
    const __vector4 smallThetaRatio0( __vminfp( __vmulfp( absTanTheta, g_ArcTanSmallThetaRatio.m_V ), one ) );
    const __vector4 smallThetaRatio( __vmulfp( smallThetaRatio0, smallThetaRatio0 ) );
    const __vector4 smallThetaTendsTo( tanTheta );
    const __vector4 smallTheta( __vmaddfp( __vsubfp( theta, smallThetaTendsTo ), smallThetaRatio, smallThetaTendsTo ) );
    const __vector4 result( __vsel( smallTheta, theta, __vcmpgtfp( absTanTheta, g_ArcTanSmallThetaLimit.m_V ) ) );
    return result;
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}



/// SIMD Scalar Natural Logarithm: Using power series expansion
NOALIAS const V128 SIMD_ScalarLog( const V128 PCREF in_z )
{
    // http://functions.wolfram.com/ElementaryFunctions/Log
    //
    // Log(1+z) == Sum[k=1,+inf] { (-1)^(k-1) . z^k / k }     /; |z| < 1
    // Log(z)   == Sum[k=1,+inf] { (-1)^(k-1) . (z-1)^k / k } /; |z-1| < 1, 0 < z < 2
    // Log(1/z) = -Log(z)
    // Log(0)   = -inf
    // Log(1)   = 0
    // Log(e)   = 1
    // Log(inf) = inf
    // Log(-ve) = complex!
    // Log2(z)  = Log(z) / Log(2), Log(z) = Log2(z) * Log(2)
    // Log(xz)  = Log(x) + Log(z)
    // Log(x/z) = Log(x) - Log(z)
    
    // Log(xz) = Log(x) + Log(z)
    //         = Log2(x) * Log(2) + Log(z)
    //   choosing integer power of 2 x such that 1 <= z < 2,
    //   Log2(x) = float exponent of xz,
    //   x = 2^Log2(x), 1/x = 2^-Log2(x)
    //   z = xz / 2^Log2(x)
    //     = xz * 2^-Log2(x) [z = xz with the float exponent set to zero]
    
    #if defined( _SPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float log2( Const< vector float >::Log2() );
    const vector float signMask( Const< vector float >::SignMask() );
    
    //const vector float intLog2_z( spu_rfip( spu_loge( in_z ) ) );
    //const vector float multiplier( spu_expte( spu_xor( intLog2_z, signMask ) ) );
    //const vector float z( spu_mul( in_z, multiplier ) );
    const vector float intLog2_z( spu_get_exponent( in_z ) );
    const vector float z( spu_set_zero_exponent( in_z ) );
    
    // CPU cycles : z --> 0
    
    const vector float z1( spu_sub( z, one ) );
    const vector float z2( spu_mul( z1, z1 ) );
    const vector float z3( spu_mul( z2, z1 ) );
    const vector float z4( spu_mul( z2, z2 ) );
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const vector float z_01_03( spu_merge_xy( z1, z3 ) );
    const vector float z_02_04( spu_merge_xy( z2, z4 ) );
    const vector float z_01_02_03_04( spu_merge_xy( z_01_03, z_02_04 ) );
    
    const vector float z5( spu_mul( z3, z2 ) );
    const vector float z6( spu_mul( z3, z3 ) );
    const vector float z7( spu_mul( z4, z3 ) );
    const vector float z8( spu_mul( z4, z4 ) );
    const vector float z10( spu_mul( z5, z5 ) );
    const vector float z12( spu_mul( z6, z6 ) );
    const vector float z14( spu_mul( z7, z7 ) );
    const vector float z16( spu_mul( z8, z8 ) );
    const vector float z20( spu_mul( z10, z10 ) );
    const vector float z24( spu_mul( z12, z12 ) );
    const vector float z28( spu_mul( z14, z14 ) );
    
    const vector float z_05_06_07_08( spu_mul( z_01_02_03_04, z4 ) );
    const vector float z_09_10_11_12( spu_mul( z_01_02_03_04, z8 ) );
    const vector float z_13_14_15_16( spu_mul( z_01_02_03_04, z12 ) );
    const vector float z_17_18_19_20( spu_mul( z_01_02_03_04, z16 ) );
    const vector float z_21_22_23_24( spu_mul( z_01_02_03_04, z20 ) );
    const vector float z_25_26_27_28( spu_mul( z_01_02_03_04, z24 ) );
    const vector float z_29_30_31_32( spu_mul( z_01_02_03_04, z28 ) );
    
    vector float const * const __restrict p_Const_LogSeries = (vector float*)g_Const_LogSeries;
    const vector float k_01_02_03_04( p_Const_LogSeries[0] );
    const vector float k_05_06_07_08( p_Const_LogSeries[1] );
    const vector float k_09_10_11_12( p_Const_LogSeries[2] );
    const vector float k_13_14_15_16( p_Const_LogSeries[3] );
    const vector float k_17_18_19_20( p_Const_LogSeries[4] );
    const vector float k_21_22_23_24( p_Const_LogSeries[5] );
    const vector float k_25_26_27_28( p_Const_LogSeries[6] );
    const vector float k_29_30_31_32( p_Const_LogSeries[7] );
    
    const vector float result( spu_add( spu_add( spu_add( spu_dot_product4( z_01_02_03_04, k_01_02_03_04 ),
                                                          spu_dot_product4( z_05_06_07_08, k_05_06_07_08 ) ),
                                                 spu_add( spu_dot_product4( z_09_10_11_12, k_09_10_11_12 ),
                                                          spu_dot_product4( z_13_14_15_16, k_13_14_15_16 ) ) ),
                                        spu_add( spu_add( spu_dot_product4( z_17_18_19_20, k_17_18_19_20 ),
                                                          spu_dot_product4( z_21_22_23_24, k_21_22_23_24 ) ),
                                                 spu_add( spu_dot_product4( z_25_26_27_28, k_25_26_27_28 ),
                                                          spu_dot_product4( z_29_30_31_32, k_29_30_31_32 ) ) ) ) );
    
    return spu_madd( intLog2_z, log2, result );
    
    #elif defined( _PPU_ )
    
    const vector float one( Const< vector float >::One() );
    const vector float log2( Const< vector float >::Log2() );
    const vector float signMask( Const< vector float >::SignMask() );
    
    const vector float intLog2_z( vec_vrfip( vec_loge( in_z ) ) ); // vec_vrfim ???
    const vector float multiplier( vec_expte( vec_xor( intLog2_z, signMask ) ) );
    const vector float z( vec_mul( in_z, multiplier ) );
    
    // CPU cycles : z --> 0
    
    const vector float z1( vec_sub( z, one ) );
    const vector float z2( vec_mul( z1, z1 ) );
    const vector float z3( vec_mul( z2, z1 ) );
    const vector float z4( vec_mul( z2, z2 ) );
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const vector float z_01_03( vec_merge_xy( z1, z3 ) );
    const vector float z_02_04( vec_merge_xy( z2, z4 ) );
    const vector float z_01_02_03_04( vec_merge_xy( z_01_03, z_02_04 ) );
    
    const vector float z5( vec_mul( z3, z2 ) );
    const vector float z6( vec_mul( z3, z3 ) );
    const vector float z7( vec_mul( z4, z3 ) );
    const vector float z8( vec_mul( z4, z4 ) );
    const vector float z10( vec_mul( z5, z5 ) );
    const vector float z12( vec_mul( z6, z6 ) );
    const vector float z14( vec_mul( z7, z7 ) );
    const vector float z16( vec_mul( z8, z8 ) );
    const vector float z20( vec_mul( z10, z10 ) );
    const vector float z24( vec_mul( z12, z12 ) );
    const vector float z28( vec_mul( z14, z14 ) );
    
    const vector float z_05_06_07_08( vec_mul( z_01_02_03_04, z4 ) );
    const vector float z_09_10_11_12( vec_mul( z_01_02_03_04, z8 ) );
    const vector float z_13_14_15_16( vec_mul( z_01_02_03_04, z12 ) );
    const vector float z_17_18_19_20( vec_mul( z_01_02_03_04, z16 ) );
    const vector float z_21_22_23_24( vec_mul( z_01_02_03_04, z20 ) );
    const vector float z_25_26_27_28( vec_mul( z_01_02_03_04, z24 ) );
    const vector float z_29_30_31_32( vec_mul( z_01_02_03_04, z28 ) );
    
    vector float const * const __restrict p_Const_LogSeries = (vector float*)g_Const_LogSeries;
    const vector float k_01_02_03_04( p_Const_LogSeries[0] );
    const vector float k_05_06_07_08( p_Const_LogSeries[1] );
    const vector float k_09_10_11_12( p_Const_LogSeries[2] );
    const vector float k_13_14_15_16( p_Const_LogSeries[3] );
    const vector float k_17_18_19_20( p_Const_LogSeries[4] );
    const vector float k_21_22_23_24( p_Const_LogSeries[5] );
    const vector float k_25_26_27_28( p_Const_LogSeries[6] );
    const vector float k_29_30_31_32( p_Const_LogSeries[7] );
    
    const vector float result( vec_add( vec_add( vec_add( vec_dot_product4( z_01_02_03_04, k_01_02_03_04 ),
                                                          vec_dot_product4( z_05_06_07_08, k_05_06_07_08 ) ),
                                                 vec_add( vec_dot_product4( z_09_10_11_12, k_09_10_11_12 ),
                                                          vec_dot_product4( z_13_14_15_16, k_13_14_15_16 ) ) ),
                                        vec_add( vec_add( vec_dot_product4( z_17_18_19_20, k_17_18_19_20 ),
                                                          vec_dot_product4( z_21_22_23_24, k_21_22_23_24 ) ),
                                                 vec_add( vec_dot_product4( z_25_26_27_28, k_25_26_27_28 ),
                                                          vec_dot_product4( z_29_30_31_32, k_29_30_31_32 ) ) ) ) );
    
    return vec_madd( intLog2_z, log2, result );
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 log2( Const< __vector4 >::Log2() );
    const __vector4 signMask( Const< __vector4 >::SignMask() );
    
    const __vector4 intLog2_z( __vrfip( __vlogefp( in_z ) ) ); // __vrfim ???
    const __vector4 multiplier( __vexptefp( __vxor( intLog2_z, signMask ) ) );
    const __vector4 z( __vmulfp( in_z, multiplier ) );
    
    // CPU cycles : z --> 0
    
    const __vector4 z1( __vsubfp( z, one ) );
    const __vector4 z2( __vmulfp( z1, z1 ) );
    const __vector4 z3( __vmulfp( z2, z1 ) );
    const __vector4 z4( __vmulfp( z2, z2 ) );
    
    // NOTE: Slightly faster and a lot less instructions than vector version!
    // ( Even faster if hyperthreaded since there's a lot of latencies than can be reduced by the other hyperthread ).
    
    const __vector4 z_01_03( __vmrghw( z1, z3 ) );
    const __vector4 z_02_04( __vmrghw( z2, z4 ) );
    const __vector4 z_01_02_03_04( __vmrghw( z_01_03, z_02_04 ) );
    
    const __vector4 z5( __vmulfp( z3, z2 ) );
    const __vector4 z6( __vmulfp( z3, z3 ) );
    const __vector4 z7( __vmulfp( z4, z3 ) );
    const __vector4 z8( __vmulfp( z4, z4 ) );
    const __vector4 z10( __vmulfp( z5, z5 ) );
    const __vector4 z12( __vmulfp( z6, z6 ) );
    const __vector4 z14( __vmulfp( z7, z7 ) );
    const __vector4 z16( __vmulfp( z8, z8 ) );
    const __vector4 z20( __vmulfp( z10, z10 ) );
    const __vector4 z24( __vmulfp( z12, z12 ) );
    const __vector4 z28( __vmulfp( z14, z14 ) );
    
    const __vector4 z_05_06_07_08( __vmulfp( z_01_02_03_04, z4 ) );
    const __vector4 z_09_10_11_12( __vmulfp( z_01_02_03_04, z8 ) );
    const __vector4 z_13_14_15_16( __vmulfp( z_01_02_03_04, z12 ) );
    const __vector4 z_17_18_19_20( __vmulfp( z_01_02_03_04, z16 ) );
    const __vector4 z_21_22_23_24( __vmulfp( z_01_02_03_04, z20 ) );
    const __vector4 z_25_26_27_28( __vmulfp( z_01_02_03_04, z24 ) );
    const __vector4 z_29_30_31_32( __vmulfp( z_01_02_03_04, z28 ) );
    
    __vector4 const * const __restrict p_Const_LogSeries = (__vector4*)g_Const_LogSeries;
    const __vector4 k_01_02_03_04( p_Const_LogSeries[0] );
    const __vector4 k_05_06_07_08( p_Const_LogSeries[1] );
    const __vector4 k_09_10_11_12( p_Const_LogSeries[2] );
    const __vector4 k_13_14_15_16( p_Const_LogSeries[3] );
    const __vector4 k_17_18_19_20( p_Const_LogSeries[4] );
    const __vector4 k_21_22_23_24( p_Const_LogSeries[5] );
    const __vector4 k_25_26_27_28( p_Const_LogSeries[6] );
    const __vector4 k_29_30_31_32( p_Const_LogSeries[7] );
    
    const __vector4 result( __vaddfp( __vaddfp( __vaddfp( __vmsum4fp( z_01_02_03_04, k_01_02_03_04 ),
                                                          __vmsum4fp( z_05_06_07_08, k_05_06_07_08 ) ),
                                                __vaddfp( __vmsum4fp( z_09_10_11_12, k_09_10_11_12 ),
                                                          __vmsum4fp( z_13_14_15_16, k_13_14_15_16 ) ) ),
                                      __vaddfp( __vaddfp( __vmsum4fp( z_17_18_19_20, k_17_18_19_20 ),
                                                          __vmsum4fp( z_21_22_23_24, k_21_22_23_24 ) ),
                                                __vaddfp( __vmsum4fp( z_25_26_27_28, k_25_26_27_28 ),
                                                          __vmsum4fp( z_29_30_31_32, k_29_30_31_32 ) ) ) ) );
    
    return __vmaddfp( intLog2_z, log2, result );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}

/// SIMD vector Natural Logarithm: Using power series expansion
NOALIAS const V128 SIMD_VectorLog( const V128 PCREF in_z )
{
    // http://functions.wolfram.com/ElementaryFunctions/Log
    //
    // Log(1+z) == Sum[k=1,+inf] { (-1)^(k-1) . z^k / k }     /; |z| < 1
    // Log(z)   == Sum[k=1,+inf] { (-1)^(k-1) . (z-1)^k / k } /; |z-1| < 1, 0 < z < 2
    // Log(1/z) = -Log(z)
    // Log(0)   = -inf
    // Log(1)   = 0
    // Log(e)   = 1
    // Log(inf) = inf
    // Log(-ve) = complex!
    // Log2(z)  = Log(z) / Log(2), Log(z) == Log2(z) * Log(2)
    // Log(xz)  = Log(x) + Log(z)
    // Log(x/z) = Log(x) - Log(z)
    
    #if defined( _SPU_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const< vector float >::Zero();
    return result;
    
    #elif defined( _PPU_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const< vector float >::Zero();
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 log2( Const< __vector4 >::Log2() );
    const __vector4 signMask( Const< __vector4 >::SignMask() );
    
    const __vector4 intLog2_z( __vrfip( __vlogefp( in_z ) ) );
    const __vector4 multiplier( __vexptefp( __vxor( intLog2_z, signMask ) ) );
    const __vector4 z( __vmulfp( in_z, multiplier ) );
    
    __vector4 const * const __restrict p_Const_LogSeries = (__vector4*)g_Const_LogSeries;
    const __vector4 k_01_02_03_04( p_Const_LogSeries[0] );
    const __vector4 k_05_06_07_08( p_Const_LogSeries[1] );
    const __vector4 k_09_10_11_12( p_Const_LogSeries[2] );
    const __vector4 k_13_14_15_16( p_Const_LogSeries[3] );
    const __vector4 k_17_18_19_20( p_Const_LogSeries[4] );
    const __vector4 k_21_22_23_24( p_Const_LogSeries[5] );
    const __vector4 k_25_26_27_28( p_Const_LogSeries[6] );
    const __vector4 k_29_30_31_32( p_Const_LogSeries[7] );
    
    const __vector4 z1( __vsubfp( z, one ) );
    const __vector4 z2( __vmulfp( z1, z1 ) );
    const __vector4 z3( __vmulfp( z2, z1 ) );
    const __vector4 z4( __vmulfp( z2, z2 ) );
    const __vector4 z5( __vmulfp( z3, z2 ) );
    const __vector4 z6( __vmulfp( z3, z3 ) );
    const __vector4 z7( __vmulfp( z4, z3 ) );
    const __vector4 z8( __vmulfp( z4, z4 ) );
    const __vector4 z9( __vmulfp( z5, z4 ) );
    const __vector4 z10( __vmulfp( z5, z5 ) );
    const __vector4 z11( __vmulfp( z6, z5 ) );
    const __vector4 z12( __vmulfp( z6, z6 ) );
    const __vector4 z13( __vmulfp( z7, z6 ) );
    const __vector4 z14( __vmulfp( z7, z7 ) );
    const __vector4 z15( __vmulfp( z8, z7 ) );
    const __vector4 z16( __vmulfp( z8, z8 ) );
    const __vector4 z17( __vmulfp( z9, z8 ) );
    const __vector4 z18( __vmulfp( z9, z9 ) );
    const __vector4 z19( __vmulfp( z10, z9 ) );
    const __vector4 z20( __vmulfp( z10, z10 ) );
    const __vector4 z21( __vmulfp( z11, z10 ) );
    const __vector4 z22( __vmulfp( z11, z11 ) );
    const __vector4 z23( __vmulfp( z12, z11 ) );
    const __vector4 z24( __vmulfp( z12, z12 ) );
    const __vector4 z25( __vmulfp( z13, z12 ) );
    const __vector4 z26( __vmulfp( z13, z13 ) );
    const __vector4 z27( __vmulfp( z14, z13 ) );
    const __vector4 z28( __vmulfp( z14, z14 ) );
    const __vector4 z29( __vmulfp( z15, z14 ) );
    const __vector4 z30( __vmulfp( z15, z15 ) );
    const __vector4 z31( __vmulfp( z16, z15 ) );
    const __vector4 z32( __vmulfp( z16, z16 ) );
    
    const __vector4 zk_01_02_03_04( __vaddfp( __vmaddfp( z1,  __vspltw( k_01_02_03_04, 0 ),
                                               __vmulfp( z2,  __vspltw( k_01_02_03_04, 1 ) ) ),
                                              __vmaddfp( z3,  __vspltw( k_01_02_03_04, 2 ),
                                               __vmulfp( z4,  __vspltw( k_01_02_03_04, 3 ) ) ) ) );
    
    const __vector4 zk_05_06_07_08( __vaddfp( __vmaddfp( z5,  __vspltw( k_05_06_07_08, 0 ),
                                               __vmulfp( z6,  __vspltw( k_05_06_07_08, 1 ) ) ),
                                              __vmaddfp( z7,  __vspltw( k_05_06_07_08, 2 ),
                                               __vmulfp( z8,  __vspltw( k_05_06_07_08, 3 ) ) ) ) );
    
    const __vector4 zk_09_10_11_12( __vaddfp( __vmaddfp( z9,  __vspltw( k_09_10_11_12, 0 ),
                                               __vmulfp( z10, __vspltw( k_09_10_11_12, 1 ) ) ),
                                              __vmaddfp( z11, __vspltw( k_09_10_11_12, 2 ),
                                               __vmulfp( z12, __vspltw( k_09_10_11_12, 3 ) ) ) ) );
    
    const __vector4 zk_13_14_15_16( __vaddfp( __vmaddfp( z13, __vspltw( k_13_14_15_16, 0 ),
                                               __vmulfp( z14, __vspltw( k_13_14_15_16, 1 ) ) ),
                                              __vmaddfp( z15, __vspltw( k_13_14_15_16, 2 ),
                                               __vmulfp( z16, __vspltw( k_13_14_15_16, 3 ) ) ) ) );
    
    const __vector4 zk_17_18_19_20( __vaddfp( __vmaddfp( z17, __vspltw( k_17_18_19_20, 0 ),
                                               __vmulfp( z18, __vspltw( k_17_18_19_20, 1 ) ) ),
                                              __vmaddfp( z19, __vspltw( k_17_18_19_20, 2 ),
                                               __vmulfp( z20, __vspltw( k_17_18_19_20, 3 ) ) ) ) );
    
    const __vector4 zk_21_22_23_24( __vaddfp( __vmaddfp( z21, __vspltw( k_21_22_23_24, 0 ),
                                               __vmulfp( z22, __vspltw( k_21_22_23_24, 1 ) ) ),
                                              __vmaddfp( z23, __vspltw( k_21_22_23_24, 2 ),
                                               __vmulfp( z24, __vspltw( k_21_22_23_24, 3 ) ) ) ) );
    
    const __vector4 zk_25_26_27_28( __vaddfp( __vmaddfp( z25, __vspltw( k_25_26_27_28, 0 ),
                                               __vmulfp( z26, __vspltw( k_25_26_27_28, 1 ) ) ),
                                              __vmaddfp( z27, __vspltw( k_25_26_27_28, 2 ),
                                               __vmulfp( z28, __vspltw( k_25_26_27_28, 3 ) ) ) ) );
    
    const __vector4 zk_29_30_31_32( __vaddfp( __vmaddfp( z29, __vspltw( k_29_30_31_32, 0 ),
                                               __vmulfp( z30, __vspltw( k_29_30_31_32, 1 ) ) ),
                                              __vmaddfp( z31, __vspltw( k_29_30_31_32, 2 ),
                                               __vmulfp( z32, __vspltw( k_29_30_31_32, 3 ) ) ) ) );
    
    const __vector4 result( __vaddfp( __vaddfp( __vaddfp( zk_01_02_03_04,
                                                          zk_05_06_07_08 ),
                                                __vaddfp( zk_09_10_11_12,
                                                          zk_13_14_15_16 ) ),
                                      __vaddfp( __vaddfp( zk_17_18_19_20,
                                                          zk_21_22_23_24 ),
                                                __vaddfp( zk_25_26_27_28,
                                                          zk_29_30_31_32 ) ) ) );
    
    return __vmaddfp( intLog2_z, log2, result );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}


/// SIMD scalar Exponential e ^ z : Using power series expansion
NOALIAS const V128 SIMD_ScalarExp( const V128 PCREF in_z )
{
    // http://functions.wolfram.com/ElementaryFunctions/Exp/
    //
    // Exp(z)  == Sum[k=0,+inf] { z^k / k! }
    // Exp(z)  == Exp(z0) . Sum[k=0,+inf] { (z-z0)^k / k! }
    // a^(x+y) = a^x . a^y
    // a^(x-y) = a^x / a^y
    // (a^x)^y = a^(xy)
    // (ab)^x  = a^x . b^x
    
    #if defined( _SPU_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const< vector float >::Zero();
    return result;
    
    #elif defined( _PPU_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const< vector float >::Zero();
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 plus_88( Const< __vector4 >::Int< 88 >() );
    const __vector4 minus_88( Const< __vector4 >::Int< -88 >() );
    
    // Clamp z to min/max exponential range
    const __vector4 z( __vminfp( __vmaxfp( in_z, minus_88 ), plus_88 ) );
    const __vector4 z_plus_half( __vaddfp( z, half ) );
    const __vector4 z0( __vrfim( z_plus_half ) );
    
    S32 int_z0;
    __stvewx( __vctsxs( z0, 0 ), &int_z0, 0x0 );
    
    const __vector4 zp0( one );
    const __vector4 zp1( __vsubfp( z, z0 ) );
    const __vector4 zp2( __vmulfp( zp1, zp1 ) );
    const __vector4 zp3( __vmulfp( zp2, zp1 ) );
    const __vector4 zp4( __vmulfp( zp2, zp2 ) );
    const __vector4 zp5( __vmulfp( zp3, zp2 ) );
    const __vector4 zp6( __vmulfp( zp3, zp3 ) );
    const __vector4 zp7( __vmulfp( zp4, zp3 ) );
    const __vector4 zp8( __vmulfp( zp4, zp4 ) );
    const __vector4 zp10( __vmulfp( zp5, zp5 ) );
    const __vector4 zp12( __vmulfp( zp6, zp6 ) );
    const __vector4 zp14( __vmulfp( zp7, zp7 ) );
    const __vector4 zp16( __vmulfp( zp8, zp8 ) );
    const __vector4 zp20( __vmulfp( zp10, zp10 ) );
    const __vector4 zp24( __vmulfp( zp12, zp12 ) );
    const __vector4 zp28( __vmulfp( zp14, zp14 ) );
    
    const __vector4 zp_00_01_02_03( __vmrghw( __vmrghw( zp0, zp2 ), __vmrghw( zp1, zp3 ) ) );
    const __vector4 zp_04_05_06_07( __vmulfp( zp_00_01_02_03, zp4 ) );
    const __vector4 zp_08_09_10_11( __vmulfp( zp_00_01_02_03, zp8 ) );
    const __vector4 zp_12_13_14_15( __vmulfp( zp_00_01_02_03, zp12 ) );
    const __vector4 zp_16_17_18_19( __vmulfp( zp_00_01_02_03, zp16 ) );
    const __vector4 zp_20_21_22_23( __vmulfp( zp_00_01_02_03, zp20 ) );
    const __vector4 zp_24_25_26_27( __vmulfp( zp_00_01_02_03, zp24 ) );
    const __vector4 zp_28_29_30_31( __vmulfp( zp_00_01_02_03, zp28 ) );
    
    __vector4 const * const __restrict p_Const_ExpSeries = (__vector4*)g_Const_ExpSeries;
    const __vector4 k_00_01_02_03( p_Const_ExpSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_ExpSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_ExpSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_ExpSeries[3] );
    const __vector4 k_16_17_18_19( p_Const_ExpSeries[4] );
    const __vector4 k_20_21_22_23( p_Const_ExpSeries[5] );
    const __vector4 k_24_25_26_27( p_Const_ExpSeries[6] );
    const __vector4 k_28_29_30_31( p_Const_ExpSeries[7] );
    
    const __vector4 sum_00_01_02_03( __vmsum4fp( zp_00_01_02_03, k_00_01_02_03 ) );
    const __vector4 sum_04_05_06_07( __vmsum4fp( zp_04_05_06_07, k_04_05_06_07 ) );
    const __vector4 sum_08_09_10_11( __vmsum4fp( zp_08_09_10_11, k_08_09_10_11 ) );
    const __vector4 sum_12_13_14_15( __vmsum4fp( zp_12_13_14_15, k_12_13_14_15 ) );
    const __vector4 sum_16_17_18_19( __vmsum4fp( zp_16_17_18_19, k_16_17_18_19 ) );
    const __vector4 sum_20_21_22_23( __vmsum4fp( zp_20_21_22_23, k_20_21_22_23 ) );
    const __vector4 sum_24_25_26_27( __vmsum4fp( zp_24_25_26_27, k_24_25_26_27 ) );
    const __vector4 sum_28_29_30_31( __vmsum4fp( zp_28_29_30_31, k_28_29_30_31 ) );
    
    const __vector4 sum( __vaddfp( __vaddfp( __vaddfp( sum_00_01_02_03, sum_04_05_06_07 ),
                                             __vaddfp( sum_08_09_10_11, sum_12_13_14_15 ) ),
                                   __vaddfp( __vaddfp( sum_16_17_18_19, sum_20_21_22_23 ),
                                             __vaddfp( sum_24_25_26_27, sum_28_29_30_31 ) ) ) );
    
    const S32 index_exp_z0 = int_z0 + MATHS_EXP_TABLE_ZERO_OFFSET;
    __vector4 const * const __restrict p_Const_ExpTable = (__vector4*)g_Const_ExpTable;
    const __vector4 exp_z0( __vspltw( __lvlx( p_Const_ExpTable, index_exp_z0 * sizeof( F32 ) ), VX ) );
    
    return __vmulfp( exp_z0, sum );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}

/// SIMD vector Exponential e ^ z : Using power series expansion
NOALIAS const V128 SIMD_VectorExp( const V128 PCREF in_z )
{
    // http://functions.wolfram.com/ElementaryFunctions/Exp/
    //
    // Exp(z)  == Sum[k=0,+inf] { z^k / k! }
    // Exp(z)  == Exp(z0) . Sum[k=0,+inf] { (z-z0)^k / k! }
    // a^(x+y) = a^x . a^y
    // a^(x-y) = a^x / a^y
    // (a^x)^y = a^(xy)
    // (ab)^x  = a^x . b^x
    
    #if defined( _SPU_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const< vector float >::Zero();
    return result;
    
    #elif defined( _PPU_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const< vector float >::Zero();
    return result;
    
    #elif defined( _VMX128_ )
    
    const __vector4 half( Const< __vector4 >::Half() );
    const __vector4 one( Const< __vector4 >::One() );
    const __vector4 plus_88( Const< __vector4 >::Int< 88 >() );
    const __vector4 minus_88( Const< __vector4 >::Int< -88 >() );
    
    // Clamp z to min/max exponential range
    const __vector4 z( __vminfp( __vmaxfp( in_z, minus_88 ), plus_88 ) );
    const __vector4 z_plus_half( __vaddfp( z, half ) );
    const __vector4 z0( __vrfim( z_plus_half ) );
    
    __declspec( align( 16 ) ) S32 int_z0[4];
    __stvx( __vctsxs( z0, 0 ), int_z0, 0x0 );
    
    const __vector4 zp0( one );
    const __vector4 zp1( __vsubfp( z, z0 ) );
    const __vector4 zp2( __vmulfp( zp1, zp1 ) );
    const __vector4 zp3( __vmulfp( zp2, zp1 ) );
    const __vector4 zp4( __vmulfp( zp2, zp2 ) );
    const __vector4 zp5( __vmulfp( zp3, zp2 ) );
    const __vector4 zp6( __vmulfp( zp3, zp3 ) );
    const __vector4 zp7( __vmulfp( zp4, zp3 ) );
    const __vector4 zp8( __vmulfp( zp4, zp4 ) );
    const __vector4 zp9( __vmulfp( zp5, zp4 ) );
    const __vector4 zp10( __vmulfp( zp5, zp5 ) );
    const __vector4 zp11( __vmulfp( zp6, zp5 ) );
    const __vector4 zp12( __vmulfp( zp6, zp6 ) );
    const __vector4 zp13( __vmulfp( zp7, zp6 ) );
    const __vector4 zp14( __vmulfp( zp7, zp7 ) );
    const __vector4 zp15( __vmulfp( zp8, zp7 ) );
    const __vector4 zp16( __vmulfp( zp8, zp8 ) );
    const __vector4 zp17( __vmulfp( zp9, zp8 ) );
    const __vector4 zp18( __vmulfp( zp9, zp9 ) );
    const __vector4 zp19( __vmulfp( zp10, zp9 ) );
    const __vector4 zp20( __vmulfp( zp10, zp10 ) );
    const __vector4 zp21( __vmulfp( zp11, zp10 ) );
    const __vector4 zp22( __vmulfp( zp11, zp11 ) );
    const __vector4 zp23( __vmulfp( zp12, zp11 ) );
    const __vector4 zp24( __vmulfp( zp12, zp12 ) );
    const __vector4 zp25( __vmulfp( zp13, zp12 ) );
    const __vector4 zp26( __vmulfp( zp13, zp13 ) );
    const __vector4 zp27( __vmulfp( zp14, zp13 ) );
    const __vector4 zp28( __vmulfp( zp14, zp14 ) );
    const __vector4 zp29( __vmulfp( zp15, zp14 ) );
    const __vector4 zp30( __vmulfp( zp15, zp15 ) );
    const __vector4 zp31( __vmulfp( zp16, zp15 ) );
    
    __vector4 const * const __restrict p_Const_ExpSeries = (__vector4*)g_Const_ExpSeries;
    const __vector4 k_00_01_02_03( p_Const_ExpSeries[0] );
    const __vector4 k_04_05_06_07( p_Const_ExpSeries[1] );
    const __vector4 k_08_09_10_11( p_Const_ExpSeries[2] );
    const __vector4 k_12_13_14_15( p_Const_ExpSeries[3] );
    const __vector4 k_16_17_18_19( p_Const_ExpSeries[4] );
    const __vector4 k_20_21_22_23( p_Const_ExpSeries[5] );
    const __vector4 k_24_25_26_27( p_Const_ExpSeries[6] );
    const __vector4 k_28_29_30_31( p_Const_ExpSeries[7] );
    
    const __vector4 sum_00_01_02_03( __vaddfp( __vmaddfp( zp0, __vspltw( k_00_01_02_03, 0 ),
                                                __vmulfp( zp1, __vspltw( k_00_01_02_03, 1 ) ) ),
                                               __vmaddfp( zp2, __vspltw( k_00_01_02_03, 2 ),
                                                __vmulfp( zp3, __vspltw( k_00_01_02_03, 3 ) ) ) ) );
    
    const __vector4 sum_04_05_06_07( __vaddfp( __vmaddfp( zp4, __vspltw( k_04_05_06_07, 0 ),
                                                __vmulfp( zp5, __vspltw( k_04_05_06_07, 1 ) ) ),
                                               __vmaddfp( zp6, __vspltw( k_04_05_06_07, 2 ),
                                                __vmulfp( zp7, __vspltw( k_04_05_06_07, 3 ) ) ) ) );
    
    const __vector4 sum_08_09_10_11( __vaddfp( __vmaddfp( zp8,  __vspltw( k_08_09_10_11, 0 ),
                                                __vmulfp( zp9,  __vspltw( k_08_09_10_11, 1 ) ) ),
                                               __vmaddfp( zp10, __vspltw( k_08_09_10_11, 2 ),
                                                __vmulfp( zp11, __vspltw( k_08_09_10_11, 3 ) ) ) ) );
    
    const __vector4 sum_12_13_14_15( __vaddfp( __vmaddfp( zp12, __vspltw( k_12_13_14_15, 0 ),
                                                __vmulfp( zp13, __vspltw( k_12_13_14_15, 1 ) ) ),
                                               __vmaddfp( zp14, __vspltw( k_12_13_14_15, 2 ),
                                                __vmulfp( zp15, __vspltw( k_12_13_14_15, 3 ) ) ) ) );
    
    const __vector4 sum_16_17_18_19( __vaddfp( __vmaddfp( zp16, __vspltw( k_16_17_18_19, 0 ),
                                                __vmulfp( zp17, __vspltw( k_16_17_18_19, 1 ) ) ),
                                               __vmaddfp( zp18, __vspltw( k_16_17_18_19, 2 ),
                                                __vmulfp( zp19, __vspltw( k_16_17_18_19, 3 ) ) ) ) );
    
    const __vector4 sum_20_21_22_23( __vaddfp( __vmaddfp( zp20, __vspltw( k_20_21_22_23, 0 ),
                                                __vmulfp( zp21, __vspltw( k_20_21_22_23, 1 ) ) ),
                                               __vmaddfp( zp22, __vspltw( k_20_21_22_23, 2 ),
                                                __vmulfp( zp23, __vspltw( k_20_21_22_23, 3 ) ) ) ) );
    
    const __vector4 sum_24_25_26_27( __vaddfp( __vmaddfp( zp24, __vspltw( k_24_25_26_27, 0 ),
                                                __vmulfp( zp25, __vspltw( k_24_25_26_27, 1 ) ) ),
                                               __vmaddfp( zp26, __vspltw( k_24_25_26_27, 2 ),
                                                __vmulfp( zp27, __vspltw( k_24_25_26_27, 3 ) ) ) ) );
    
    const __vector4 sum_28_29_30_31( __vaddfp( __vmaddfp( zp28, __vspltw( k_28_29_30_31, 0 ),
                                                __vmulfp( zp29, __vspltw( k_28_29_30_31, 1 ) ) ),
                                               __vmaddfp( zp30, __vspltw( k_28_29_30_31, 2 ),
                                                __vmulfp( zp31, __vspltw( k_28_29_30_31, 3 ) ) ) ) );
    
    const __vector4 sum( __vaddfp( __vaddfp( __vaddfp( sum_00_01_02_03,
                                                       sum_04_05_06_07 ),
                                             __vaddfp( sum_08_09_10_11,
                                                       sum_12_13_14_15 ) ),
                                   __vaddfp( __vaddfp( sum_16_17_18_19,
                                                       sum_20_21_22_23 ),
                                             __vaddfp( sum_24_25_26_27,
                                                       sum_28_29_30_31 ) ) ) );
    
    const S32 index_exp_z0_x = int_z0[0] + MATHS_EXP_TABLE_ZERO_OFFSET;
    const S32 index_exp_z0_y = int_z0[1] + MATHS_EXP_TABLE_ZERO_OFFSET;
    const S32 index_exp_z0_z = int_z0[2] + MATHS_EXP_TABLE_ZERO_OFFSET;
    const S32 index_exp_z0_w = int_z0[3] + MATHS_EXP_TABLE_ZERO_OFFSET;
    
    __vector4 const * const __restrict p_Const_ExpTable = (__vector4*)g_Const_ExpTable;
    const __vector4 exp_z0_x( __lvlx( p_Const_ExpTable, index_exp_z0_x * sizeof( F32 ) ) );
    const __vector4 exp_z0_y( __lvlx( p_Const_ExpTable, index_exp_z0_y * sizeof( F32 ) ) );
    const __vector4 exp_z0_z( __lvlx( p_Const_ExpTable, index_exp_z0_z * sizeof( F32 ) ) );
    const __vector4 exp_z0_w( __lvlx( p_Const_ExpTable, index_exp_z0_w * sizeof( F32 ) ) );
    const __vector4 exp_z0( __vmrghw( __vmrghw( exp_z0_x, exp_z0_z ),
                                      __vmrghw( exp_z0_y, exp_z0_w ) ) );
    
    return __vmulfp( exp_z0, sum );
    
    #elif defined( _SSE_ )
    
    assert( 0 && "Not implemented yet!" );
    V128 result = Const<__m128>::Zero();
    return result;
    
    #endif
}





/// SIMD Scalar Power x ^ y : Using power series expansion
//NOALIAS const V128 SIMD_ScalarPow( const V128 PCREF z, const V128 PCREF a )
//{
//    // http://functions.wolfram.com/ElementaryFunctions/Power/02/
//    //
//    // z^a == Sum[k=0,+inf] { Log(z)^k . a^k / k! }
//    // z^a = e^(a.Log(z))
//    
//    #if defined( _SPU_ )
//    
//    #elif defined( _PPU_ )
//    
//    #elif defined( _VMX128_ )
//    
//    return __vzero();
//    
//    #elif defined( _SSE_ )
//    
//    #endif
//}


#endif // #if defined( _SIMD_ )



























#if 0
/// HF32 Sin: Using power series expansion
NOALIAS const HF32 Sin( const HF32 PCREF theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Sin
    //
    // Sin(z) == Sum[k=0,inf] { z^(2k+1) . -1^k / (2k+1)! }
    
    const HF32 PI( g_Const_PI[ E_PI_CONSTANTS_PI ] );
    const HF32 twoPI( g_Const_PI[ E_PI_CONSTANTS_TWO_PI ] );
    const HF32 reciprocalTwoPI( g_Const_PI[ E_PI_CONSTANTS_RCP_TWO_PI ] );
    
    #if 0
    //const HF32 z1( theta - ( RoundToIntMinusInf( ( theta + PI ) * reciprocalTwoPI ) * twoPI ) );
    const HF32 z1( WrapToIntervalOffset( theta, twoPI, reciprocalTwoPI, PI ) );
    #else
	//[SteveP] using this for now to keep skeleton dynamics working
    const HF32 z1( theta - ( twoPI * RoundToIntNearest( theta * reciprocalTwoPI ) ) );
    #endif
    
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
    
    HVec4 const * const __restrict p_Const_SinSeries = (HVec4*)g_Const_SinSeries;
    const HVec4 k00_03( p_Const_SinSeries[0] );
    const HVec4 k04_07( p_Const_SinSeries[1] );
    const HVec4 k08_11( p_Const_SinSeries[2] );
    const HVec4 k12_15( p_Const_SinSeries[3] );
    
    #if defined( _SSE_ ) && !defined( _SSE2_ )
    _m_empty();
    #endif
    
    return ( MultiplySum4( HVec4( z1,  z3,  z5,  z7  ), k00_03 ) +
             MultiplySum4( HVec4( z9,  z11, z13, z15 ), k04_07 ) ) +
           ( MultiplySum4( HVec4( z17, z19, z21, z23 ), k08_11 ) +
             MultiplySum4( HVec4( z25, z27, z29, z31 ), k12_15 ) );
}
#endif

/// HF32 Tan: Using power series expansion
NOALIAS const HF32 Tan( const HF32 PCREF inputTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Tan
    //
    // Tan(z) == Sum[k=1,inf] { z^(2k-1) . -1^(k-1) . 2^(2k) . (2^(2k))-1 . B(2k) / (2k)! } : |z| < PI/2
    
    const HF32 half_PI( g_Const_PI[ E_PI_CONSTANTS_PI_DIV_TWO ] );
    const HF32 PI( g_Const_PI[ E_PI_CONSTANTS_PI ] );
    const HF32 reciprocal_PI( g_Const_PI[ E_PI_CONSTANTS_RCP_PI ] );
    const HF32 half( c_ScalarHalf );
    
    #if 0
    //const HF32 theta( inputTheta - ( PI * RoundToIntMinusInf( inputTheta * reciprocal_PI ) ) );
    const HF32 theta( WrapToIntervalOffset( inputTheta, PI, reciprocal_PI, half_PI ) );
    #else
	//[SteveP] using this for now to keep skeleton dynamics working
    const HF32 theta( inputTheta - ( PI * RoundToIntNearest( inputTheta * reciprocal_PI ) ) );
    #endif


    const HF32 halfTheta( theta * half );
    
    const HF32 z1( halfTheta );
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
    
    HVec4 const * const __restrict p_Const_TanSeries = (HVec4*)g_Const_TanSeries;
    const HVec4 k01_04( p_Const_TanSeries[0] );
    const HVec4 k05_08( p_Const_TanSeries[1] );
    const HVec4 k09_12( p_Const_TanSeries[2] );
    const HVec4 k13_16( p_Const_TanSeries[3] );
    
    const HF32 tanHalfTheta( ( MultiplySum4( HVec4( z1,  z3,  z5,  z7  ), k01_04 ) +
                               MultiplySum4( HVec4( z9,  z11, z13, z15 ), k05_08 ) ) +
                             ( MultiplySum4( HVec4( z17, z19, z21, z23 ), k09_12 ) +
                               MultiplySum4( HVec4( z25, z27, z29, z31 ), k13_16 ) ) );
    
    const HF32 one( c_ScalarOne );
    const HF32 two( c_ScalarTwo );
    
    // Double-angle formula: Tan( 2 * theta ) = 2 * Tan( theta ) / ( 1 - ( Tan^2( theta ) )
    
    #if defined( _SSE_ ) && !defined( _SSE2_ )
    _m_empty();
    #endif

    //return two * tanHalfTheta * Reciprocal( one - ( tanHalfTheta * tanHalfTheta ) );
    return UnsafeDivide( two * tanHalfTheta, one - ( tanHalfTheta * tanHalfTheta ) );
}


/// HF32 ArcSin: Using power series expansion
NOALIAS const HF32 ArcSin( const HF32 PCREF sinTheta )
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
    
    HVec4 const * const __restrict p_Const_ArcSinSeries = (HVec4*)g_Const_ArcSinSeries;
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
NOALIAS const HF32 ArcTan( const HF32 PCREF tanTheta )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcTan
    //
    // ArcTan(z) ==                  Sum[k=0,inf] { (-1)^k . z^(2k+1) / (2k+1) } : |z| < 1
    // ArcTan(z) == PI/2 . Sign(z) - Sum[k=0,inf] { (-1)^k / z^(2k+1) / (2k+1) } : |z| > 1
    
    const HF32 zero;
    const HF32 one( c_ScalarOne );
    const HF32 fltMax( c_ScalarFltMax );
    
    // Fundamental identity: 1 + Tan^2( theta ) = Sec^2( theta ) = 1 / Cos^2( theta )
    
    const HF32 cosTheta( ReciprocalSqrt( one + ( tanTheta * tanTheta ) ) );
    
    // Half-angle formulas: Tan^2( theta ) = Sin^2( theta ) / Cos^2( theta ) = ( 1 - Cos( 2 * theta ) ) / ( 1 + Cos( 2 * theta ) )
    
    const HF32 tanHalfTheta( Sqrt( ( one - cosTheta ) * Reciprocal( one + cosTheta ) ) );
    
    HVec4 const * const __restrict p_Const_ArcTanSeries = (HVec4*)g_Const_ArcTanSeries;
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




#if !defined (PLATFORM_PS3) && !defined (PLATFORM_SPU)
/// HVec4 Sin: Using power series expansion
NOALIAS const HVec4 Sin( const HVec4 PCREF theta )
{
    // http://functions.wolfram.com/ElementaryFunctions/Sin
    //
    // Sin(z) == Sum[k=0,inf] { z^(2k+1) . -1^k / (2k+1)! }
    
    const HF32 twoPI( g_Const_PI[ E_PI_CONSTANTS_TWO_PI ] );
    const HF32 reciprocalTwoPI( g_Const_PI[ E_PI_CONSTANTS_RCP_TWO_PI ] );
    
#if 0
    const HVec4 z1( theta - ( RoundToIntMinusInf( theta * reciprocalTwoPI ) ) * twoPI );
#else
	//[SteveP] using this for now to keep skeleton dynamics working
    const HVec4 z1( theta - ( RoundToIntNearest( theta * reciprocalTwoPI ) ) * twoPI );
#endif

    const HVec4 z2( z1 * z1 );
    const HVec4 z3( z2 * z1 );
    const HVec4 z4( z2 * z2 );
    const HVec4 z5( z3 * z2 );
    const HVec4 z6( z3 * z3 );
    const HVec4 z7( z4 * z3 );
    const HVec4 z8( z4 * z4 );
    const HVec4 z9( z5 * z4 );
    const HVec4 z10( z5 * z5 );
    const HVec4 z11( z6 * z5 );
    const HVec4 z12( z6 * z6 );
    const HVec4 z13( z7 * z6 );
    const HVec4 z14( z7 * z7 );
    const HVec4 z15( z8 * z7 );
    const HVec4 z16( z8 * z8 );
    const HVec4 z17( z9 * z8 );
//  const HVec4 z18( z9 * z9 );
    const HVec4 z19( z10 * z9 );
//  const HVec4 z20( z10 * z10 );
    const HVec4 z21( z11 * z10 );
//  const HVec4 z22( z11 * z11 );
    const HVec4 z23( z12 * z11 );
//  const HVec4 z24( z12 * z12 );
    const HVec4 z25( z13 * z12 );
//  const HVec4 z26( z13 * z13 );
    const HVec4 z27( z14 * z13 );
//  const HVec4 z28( z14 * z14 );
    const HVec4 z29( z15 * z14 );
//  const HVec4 z30( z15 * z15 );
    const HVec4 z31( z16 * z15 );
    
    HVec4 const * const __restrict p_Const_SinSeries = (HVec4*)g_Const_SinSeries;
    const HVec4 k00_03( p_Const_SinSeries[0] );
    const HVec4 k04_07( p_Const_SinSeries[1] );
    const HVec4 k08_11( p_Const_SinSeries[2] );
    const HVec4 k12_15( p_Const_SinSeries[3] );
    
    #if defined( _SSE_ ) && !defined( _SSE2_ )
    _m_empty();
    #endif
    
    return ( ( ( z1 /* k0=1.0 */  ) + ( z3  * k00_03.Y() ) ) + ( ( z5  * k00_03.Z() ) + ( z7  * k00_03.W() ) ) ) +
           ( ( ( z9  * k04_07.X() ) + ( z11 * k04_07.Y() ) ) + ( ( z13 * k04_07.Z() ) + ( z15 * k04_07.W() ) ) ) +
           ( ( ( z17 * k08_11.X() ) + ( z19 * k08_11.Y() ) ) + ( ( z21 * k08_11.Z() ) + ( z23 * k08_11.W() ) ) ) +
           ( ( ( z25 * k12_15.X() ) + ( z27 * k12_15.Y() ) ) + ( ( z29 * k12_15.Z() ) + ( z31 * k12_15.W() ) ) );
}

#endif

/// HVec4 ArcSin: Using power series expansion
NOALIAS const HVec4 ArcSin( const HVec4 PCREF z )
{
    // http://functions.wolfram.com/ElementaryFunctions/ArcSin
    //
    // ArcSin(z) == Sum[k=0,inf] { z^(2k+1) . Pochhammer( 0.5, k ) / (2k+1)k! }
    
    const HVec4 z1( z );
    const HVec4 z2( z1 * z1 );
    const HVec4 z3( z2 * z1 );
    const HVec4 z4( z2 * z2 );
    const HVec4 z5( z3 * z2 );
    const HVec4 z6( z3 * z3 );
    const HVec4 z7( z4 * z3 );
    const HVec4 z8( z4 * z4 );
    const HVec4 z9( z5 * z4 );
    const HVec4 z10( z5 * z5 );
    const HVec4 z11( z6 * z5 );
    const HVec4 z12( z6 * z6 );
    const HVec4 z13( z7 * z6 );
    const HVec4 z14( z7 * z7 );
    const HVec4 z15( z8 * z7 );
    const HVec4 z16( z8 * z8 );
    const HVec4 z17( z9 * z8 );
//  const HVec4 z18( z9 * z9 );
    const HVec4 z19( z10 * z9 );
//  const HVec4 z20( z10 * z10 );
    const HVec4 z21( z11 * z10 );
//  const HVec4 z22( z11 * z11 );
    const HVec4 z23( z12 * z11 );
//  const HVec4 z24( z12 * z12 );
    const HVec4 z25( z13 * z12 );
//  const HVec4 z26( z13 * z13 );
    const HVec4 z27( z14 * z13 );
//  const HVec4 z28( z14 * z14 );
    const HVec4 z29( z15 * z14 );
//  const HVec4 z30( z15 * z15 );
    const HVec4 z31( z16 * z15 );
    
    HVec4 const * const __restrict p_Const_ArcSinSeries = (HVec4*)g_Const_ArcSinSeries;
    const HVec4 k00_03( p_Const_ArcSinSeries[0] );
    const HVec4 k04_07( p_Const_ArcSinSeries[1] );
    const HVec4 k08_11( p_Const_ArcSinSeries[2] );
    const HVec4 k12_15( p_Const_ArcSinSeries[3] );
    
    return ( ( ( z1 /* k0=1.0 */  ) + ( z3  * k00_03.Y() ) ) + ( ( z5  * k00_03.Z() ) + ( z7  * k00_03.W() ) ) ) +
           ( ( ( z9  * k04_07.X() ) + ( z11 * k04_07.Y() ) ) + ( ( z13 * k04_07.Z() ) + ( z15 * k04_07.W() ) ) ) +
           ( ( ( z17 * k08_11.X() ) + ( z19 * k08_11.Y() ) ) + ( ( z21 * k08_11.Z() ) + ( z23 * k08_11.W() ) ) ) +
           ( ( ( z25 * k12_15.X() ) + ( z27 * k12_15.Y() ) ) + ( ( z29 * k12_15.Z() ) + ( z31 * k12_15.W() ) ) );
}


// Sin(z) == Sum[k=0,inf] { z^(2k+1) . -1^k / (2k+1)! }
ALIGN16 const F32 g_Const_SinSeries[16] =
{
#if PLATFORM_SPU
         1.0      ,
 -1.66666672E-01  ,
  8.33333377E-03  ,
 -1.98412701E-04  ,
  2.75573188E-06  ,
 -2.50521079E-08  ,
  1.60590444E-10  ,
 -7.64716361E-13  ,
  2.81145736E-15  ,
 -8.22063508E-18  ,
  1.95729415E-20  ,
 - 3.8681703E-23  ,
  6.44695034E-26  ,
 -9.18368981E-29  ,
  1.13099625E-31  ,
 -1.21612501E-34  ,
#else
    /* k = 0  */ (F32)( PowerMinusOne<double>( 0  ) / Factorial<double>( 2 * 0  + 1 ) ),
    /* k = 1  */ (F32)( PowerMinusOne<double>( 1  ) / Factorial<double>( 2 * 1  + 1 ) ),
    /* k = 2  */ (F32)( PowerMinusOne<double>( 2  ) / Factorial<double>( 2 * 2  + 1 ) ),
    /* k = 3  */ (F32)( PowerMinusOne<double>( 3  ) / Factorial<double>( 2 * 3  + 1 ) ),
    /* k = 4  */ (F32)( PowerMinusOne<double>( 4  ) / Factorial<double>( 2 * 4  + 1 ) ),
    /* k = 5  */ (F32)( PowerMinusOne<double>( 5  ) / Factorial<double>( 2 * 5  + 1 ) ),
    /* k = 6  */ (F32)( PowerMinusOne<double>( 6  ) / Factorial<double>( 2 * 6  + 1 ) ),
    /* k = 7  */ (F32)( PowerMinusOne<double>( 7  ) / Factorial<double>( 2 * 7  + 1 ) ),
    /* k = 8  */ (F32)( PowerMinusOne<double>( 8  ) / Factorial<double>( 2 * 8  + 1 ) ),
    /* k = 9  */ (F32)( PowerMinusOne<double>( 9  ) / Factorial<double>( 2 * 9  + 1 ) ),
    /* k = 10 */ (F32)( PowerMinusOne<double>( 10 ) / Factorial<double>( 2 * 10 + 1 ) ),
    /* k = 11 */ (F32)( PowerMinusOne<double>( 11 ) / Factorial<double>( 2 * 11 + 1 ) ),
    /* k = 12 */ (F32)( PowerMinusOne<double>( 12 ) / Factorial<double>( 2 * 12 + 1 ) ),
    /* k = 13 */ (F32)( PowerMinusOne<double>( 13 ) / Factorial<double>( 2 * 13 + 1 ) ),
    /* k = 14 */ (F32)( PowerMinusOne<double>( 14 ) / Factorial<double>( 2 * 14 + 1 ) ),
    /* k = 15 */ (F32)( PowerMinusOne<double>( 15 ) / Factorial<double>( 2 * 15 + 1 ) )
#endif
};




}; // namespace Maths




