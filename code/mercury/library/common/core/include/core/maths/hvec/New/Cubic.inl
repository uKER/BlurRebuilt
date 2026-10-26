#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CUBIC_INL
#define SOFT_MATHS_HVEC_NEW_CUBIC_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CUBIC_INL
#define MATHS_HVEC_NEW_CUBIC_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Cubic
/// @{

/// VF32 Real Cube Root
NOALIAS __forceinline const VF32 CubeRoot( const VF32 VECREF a )
{
    // http://mathworld.wolfram.com/CubeRoot.html
    // http://en.wikipedia.org/wiki/Cube_root
    
    return CopySign( Pow( Abs( a ), Const<VF32>::Third() ), a );
}


/// Cubic Root Solver: a3.z^3 + a2.z^2 + a1.z + a0 = 0
NOALIAS __forceinline void SolveCubic(
    const VF32 VECREF in_a3,
    const VF32 VECREF in_a2,
    const VF32 VECREF in_a1,
    const VF32 VECREF in_a0,
    Complex* const __restrict pRoot1,
    Complex* const __restrict pRoot2,
    Complex* const __restrict pRoot3 )
{
    // http://mathworld.wolfram.com/CubicFormula.html
    
    const VF32 half( Const<VF32>::Half() );
    const VF32 third( Const<VF32>::Third() );
    const VF32 nearlyZero( Const<VF32>::Epsilon() );
    
    const VF32 rcp_a3( Rcp24( in_a3 ) );
    const VF32 a2( in_a2 * rcp_a3 );
    const VF32 a1( in_a1 * rcp_a3 );
    const VF32 a0( in_a0 * rcp_a3 );
    
    const VF32 a2_div3( a2 * third );
    const VF32 a2_squared_div9( a2_div3 * a2_div3 );
    const VF32 a2_cubed_div27( a2_squared_div9 * a2_div3 );
    
    const VF32 p( a1 - ( a2 * a2_div3 ) );
    
    const VF32 q( ( a1 * a2_div3 ) - a0 - ( a2_cubed_div27 + a2_cubed_div27 ) );
    
    const VF32 q_div_2( q * half );
    const VF32 p_div_3( p * third );
    
    const Complex k( Sqrt( MakeComplex( ( q_div_2 * q_div_2 ) + ( p_div_3 * p_div_3 * p_div_3 ) ) ) );
    
    Complex k_pos_w1, k_pos_w2, k_pos_w3;
    Complex k_neg_w1, k_neg_w2, k_neg_w3;
    CubeRoot( q_div_2 + k, &k_pos_w1, &k_pos_w2, &k_pos_w3 );
    CubeRoot( q_div_2 - k, &k_neg_w1, &k_neg_w2, &k_neg_w3 );
    
    const Complex w1( Select( AbsSquared( k_pos_w1 ) > nearlyZero, k_pos_w1, k_neg_w1 ) );
    const Complex w2( Select( AbsSquared( k_pos_w2 ) > nearlyZero, k_pos_w2, k_neg_w2 ) );
    const Complex w3( Select( AbsSquared( k_pos_w3 ) > nearlyZero, k_pos_w3, k_neg_w3 ) );
    
    const Complex x1( w1 - ( p_div_3 / w1 ) );
    const Complex x2( w2 - ( p_div_3 / w2 ) );
    const Complex x3( w3 - ( p_div_3 / w3 ) );
    
    *pRoot1 = x1 - a2_div3;
    *pRoot2 = x2 - a2_div3;
    *pRoot3 = x3 - a2_div3;
}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


