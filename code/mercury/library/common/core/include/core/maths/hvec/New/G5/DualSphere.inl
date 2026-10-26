#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_DUAL_SPHERE_INL
#define SOFT_MATHS_HVEC_NEW_GA_DUAL_SPHERE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_DUAL_SPHERE_INL
#define MATHS_HVEC_NEW_GA_DUAL_SPHERE_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{

NOALIAS /*__force*/inline const DualSphere MakeDualSphere( const Point3 PCREF c, const VF32 VECREF r )
{
    const VF32    one( Const<VF32>::One() );
    const VF32    half( Const<VF32>::Half() );
    const Vector3 v( MakeVector3( c ) );
    return DualSphere( one, X(c), Y(c), Z(c), half * ( Dot( v, v ) - r * r ) );
}

NOALIAS /*__force*/inline const DualSphere MakeDualSphere( const Point& c, const VF32 VECREF r )
{
    const VF32 one( Const<VF32>::One() );
    const VF32 half( Const<VF32>::Half() );
    
    const PointN cn( Normalise( c ) );
    
    //return cn - ( half * r * r );
    return DualSphere( Element<BasisBits_no>(cn),
                       Element<BasisBits_e1>(cn),
                       Element<BasisBits_e2>(cn),
                       Element<BasisBits_e3>(cn),
                       Element<BasisBits_ni>(cn) - ( half * r * r ) );
}

NOALIAS /*__force*/inline const Scalar Weight( const DualSphere& ds )
{
    //const Infinity minusInfinity( Const<VF32>::MinusOne() );
    //return minusInfinity % ds;                    // Mathematical definition of the weight of a dual sphere...
    return Scalar( Element< BasisBits_e0 >( ds ) ); // Faster & easier way...
}

NOALIAS /*__force*/inline const Scalar RadiusSquared( const DualSphere& ds )
{
    const Scalar weight( Weight( ds ) );
    return ( ds % ds ) * Rcp( weight * weight );
}

//NOALIAS /*__force*/inline const Scalar Distance( const DualSphere& ds, const Point& p )
//{
//    const Scalar two( Const<VF32>::Two() );
//    const Scalar r_squared( RadiusSquared( ds ) );
//    const Scalar r( Sqrt( r_squared ) );
//    
//    // p % ds = 1/2 . alpha( r^2 - Distance^2( p, ds.c ) )
//    return Sqrt( r_squared - ( ( p % ds ) * two * Rcp( Weight( p ) * Weight( ds ) ) ) ) - r;
//}

NOALIAS /*__force*/inline const Scalar Distance( const DualSphere& ds1, const DualSphere& ds2 )
{
    const Scalar two( Const<VF32>::Two() );
    const Scalar r1_squared( RadiusSquared( ds1 ) );
    const Scalar r2_squared( RadiusSquared( ds2 ) );
    const Scalar r1( Sqrt( r1_squared ) );
    const Scalar r2( Sqrt( r2_squared ) );
    
    // ds1 % ds2 = 1/2 . alpha( r1^2 + r2^2 - Distance^2( ds1.c, ds2.c ) )
    return Sqrt( r1_squared + r2_squared - ( ( ds1 % ds2 ) * two * Rcp( Weight( ds1 ) * Weight( ds2 ) ) ) ) - r1 - r2;
}


/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


