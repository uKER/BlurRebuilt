#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{

/// Cubic Root
const C_CubicResult C_Complex::CubeRoot() const
{
    // http://scholar.hw.ac.uk/site/maths/topic19.asp?outline=
    // http://en.wikipedia.org/wiki/Cube_root
    
    const HF32 c_3( c_ScalarThree );
    const HF32 c_1_2( c_ScalarHalf );
    const HF32 c_1_3( Rcp( c_3 ) );
    const HF32 c_sqrt_3_div_2( Maths::Sqrt( c_3 ) * c_1_2 );
    const HF32 theta( Phase() * c_1_3 );
    const HF32 realCubeRoot_r( RealCubeRoot( Modulus() ) );
    
    #if 0
    
    const HF32 twoPIdiv3( PI * 2.0f / 3.0f );
    const HF32 fourPIdiv3( PI * 4.0f / 3.0f );
    
    return C_CubicResult( C_Complex( HVec2( cos( theta              ), sin( theta              ) ) * realCubeRoot_r ),
                          C_Complex( HVec2( cos( theta + twoPIdiv3  ), sin( theta + twoPIdiv3  ) ) * realCubeRoot_r ),
                          C_Complex( HVec2( cos( theta + fourPIdiv3 ), sin( theta + fourPIdiv3 ) ) * realCubeRoot_r ) );
    
    #else
    
    const C_Complex root0( HVec2( cos( theta ), sin( theta ) ) * realCubeRoot_r );
    const C_Complex root1( root0 * C_Complex( -c_1_2,  c_sqrt_3_div_2 ) );
    const C_Complex root2( root0 * C_Complex( -c_1_2, -c_sqrt_3_div_2 ) );
    
    return C_CubicResult( root0, root1, root2 );
    
    #endif
}



/// Cubic Root
const C_CubicResult64 C_Complex64::CubeRoot() const
{
    // http://scholar.hw.ac.uk/site/maths/topic19.asp?outline=
    // http://en.wikipedia.org/wiki/Cube_root
    
    //const F64 c_3( 3.0 );
    const F64 c_1_2( 0.5 );
    const F64 c_1_3( 1.0 / 3.0 );
    //const F64 c_sqrt_3_div_2( __fsqrt( 3.0 ) / 2.0 );
    
    #ifndef PLATFORM_PS3
    const F64 c_sqrt_3_div_2( Maths::Sqrt( 3.0 ) * c_1_2 );
    #else
    const F64 c_sqrt_3_div_2( (F64)sqrt( 3.0 ) * c_1_2 );
    #endif
    
    const F64 theta( Phase() * ( 1.0 / 3.0 ) );
    
    const F64 realCubeRoot_r( pow( Modulus(), c_1_3 ) );
    
    #if 0
    
    const F64 twoPIdiv3( F64_PI * 2.0 / 3.0 );
    const F64 fourPIdiv3( F64_PI * 4.0 / 3.0 );
    
    return C_CubicResult64( C_Complex64( cos( theta              ) * realCubeRoot_r, sin( theta              ) * realCubeRoot_r ),
                            C_Complex64( cos( theta + twoPIdiv3  ) * realCubeRoot_r, sin( theta + twoPIdiv3  ) * realCubeRoot_r ),
                            C_Complex64( cos( theta + fourPIdiv3 ) * realCubeRoot_r, sin( theta + fourPIdiv3 ) * realCubeRoot_r ) );
    
    #else
    
    const C_Complex64 root0( cos( theta ) * realCubeRoot_r, sin( theta ) * realCubeRoot_r );
    const C_Complex64 root1( root0 * C_Complex64( -c_1_2,  c_sqrt_3_div_2 ) );
    const C_Complex64 root2( root0 * C_Complex64( -c_1_2, -c_sqrt_3_div_2 ) );
    
    return C_CubicResult64( root0, root1, root2 );
    
    #endif
}




/// Cubic Equation Solver : a3.z^3 + a2.z^2 + a1.z + a0 = 0
const C_CubicResult SolveCubic( const HF32 PCREF in_a3, const HF32 PCREF in_a2, const HF32 PCREF in_a1, const HF32 PCREF in_a0 )
{
    // http://mathworld.wolfram.com/CubicFormula.html
    
    #if 0
    
    const HF32 c_1_2 ( c_ScalarHalf );
    const HF32 c_1_3 ( rcp( HF32( c_ScalarThree ) ) );
    
    const HF32 rcp_a3( rcp( in_a3 ) );
    const HF32 a2( in_a2 * rcp_a3 );
    const HF32 a1( in_a1 * rcp_a3 );
    const HF32 a0( in_a0 * rcp_a3 );
    
    const HF32 a2_div3( a2 * c_1_3 );
    const HF32 a2_squared_div9( a2_div3 * a2_div3 );
    const HF32 a2_cubed_div27( a2_squared_div9 * a2_div3 );
    
    const HF32 p( a1 - ( a2 * a2_div3 ) );
    
    const HF32 q( ( a1 * a2_div3 ) - a0 - ( a2_cubed_div27 + a2_cubed_div27 ) );
    
    const HF32 q_div_2( q * c_1_2 );
    const HF32 p_div_3( p * c_1_3 );
    
    const C_Complex k( C_Complex( ( q_div_2 * q_div_2 ) + ( p_div_3 * p_div_3 * p_div_3 ) ).Sqrt() );
    
    const C_CubicResult w( ( k + q_div_2 ).CubeRoot() ); // +/- k gives same pairs of results!
    
    const C_Complex x0( w.m_Root0 - ( p_div_3 / w.m_Root0 ) );
    const C_Complex x1( w.m_Root1 - ( p_div_3 / w.m_Root1 ) );
    const C_Complex x2( w.m_Root2 - ( p_div_3 / w.m_Root2 ) );
    
    const C_Complex z0( x0 - a2_div3 );
    const C_Complex z1( x1 - a2_div3 );
    const C_Complex z2( x2 - a2_div3 );
    
    return C_CubicResult( C_Complex( z0.Real(), z0.Imag() ),
                          C_Complex( z1.Real(), z1.Imag() ),
                          C_Complex( z2.Real(), z2.Imag() ) );
    
    #else
    
    const F64 c_1_2( 0.5 );
    const F64 c_1_3( 1.0 / 3.0 );
    
    const F64 rcp_a3( 1.0 / (F64)in_a3 );
    const F64 a2( (F64)in_a2 * rcp_a3 );
    const F64 a1( (F64)in_a1 * rcp_a3 );
    const F64 a0( (F64)in_a0 * rcp_a3 );
    
    const F64 a2_div3( a2 * c_1_3 );
    const F64 a2_squared_div9( a2_div3 * a2_div3 );
    const F64 a2_cubed_div27( a2_squared_div9 * a2_div3 );
    
    const F64 p( a1 - ( a2 * a2_div3 ) );
    
    const F64 q( ( a1 * a2_div3 ) - a0 - ( a2_cubed_div27 + a2_cubed_div27 ) );
    
    const F64 q_div_2( q * c_1_2 );
    const F64 p_div_3( p * c_1_3 );
    
    const C_Complex64 k( C_Complex64( ( q_div_2 * q_div_2 ) + ( p_div_3 * p_div_3 * p_div_3 ) ).Sqrt() );
    
    const C_CubicResult64 w( ( k + q_div_2 ).CubeRoot() );
    
    const C_Complex64 x0( w.m_Root0 - ( p_div_3 / w.m_Root0 ) );
    const C_Complex64 x1( w.m_Root1 - ( p_div_3 / w.m_Root1 ) );
    const C_Complex64 x2( w.m_Root2 - ( p_div_3 / w.m_Root2 ) );
    
    const C_Complex64 z0( x0 - a2_div3 );
    const C_Complex64 z1( x1 - a2_div3 );
    const C_Complex64 z2( x2 - a2_div3 );
    
    return C_CubicResult( C_Complex( (F32)z0.Real(), (F32)z0.Imag() ),
                          C_Complex( (F32)z1.Real(), (F32)z1.Imag() ),
                          C_Complex( (F32)z2.Real(), (F32)z2.Imag() ) );
    
    #endif
}














} // namespace Maths




