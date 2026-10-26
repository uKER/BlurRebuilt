#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_POINT_INL
#define SOFT_MATHS_HVEC_NEW_GA_POINT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_POINT_INL
#define MATHS_HVEC_NEW_GA_POINT_INL
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

NOALIAS /*__force*/inline const Point MakePoint( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z )
{
    const VF32    one( Const<VF32>::One() );
    const VF32    half( Const<VF32>::Half() );
    const Vector3 v( MakeVector3( x, y, z ) );
    return Point( one, x, y, z, half * Dot( v, v ) );
}

NOALIAS /*__force*/inline const Point MakePoint( const Point3 PCREF p )
{
    const VF32    one( Const<VF32>::One() );
    const VF32    half( Const<VF32>::Half() );
    const Vector3 v( MakeVector3( p ) );
    return Point( one, X(p), Y(p), Z(p), half * Dot( v, v ) );
}

NOALIAS /*__force*/inline const Point MakePoint( const Vector& v )
{
    const VF32 one( Const<VF32>::One() );
    const VF32 half( Const<VF32>::Half() );
    return Point( one,
                  Element<BasisBits_e1>(v),
                  Element<BasisBits_e2>(v),
                  Element<BasisBits_e3>(v),
                  Element<BasisBits_s>( v % v ) * half );
}

//NOALIAS /*__force*/inline const Scalar Weight( const Point& p ) // SAME AS WEIGHT OF DUAL SPHERE!
//{
//    //const Infinity minusInfinity( Const<VF32>::MinusOne() );
//    //return minusInfinity % p;                    // Mathematical definition of the weight of a point...
//    return Scalar( Element< BasisBits_e0 >( p ) ); // Faster & easier way...
//}

NOALIAS /*__force*/inline const Scalar RcpWeight( const Point& p )
{
    return Rcp( Weight( p ) );
}

NOALIAS /*__force*/inline const PointN Normalise( const Point& p )
{
    return p * RcpWeight( p );
}

//NOALIAS /*__force*/inline const Scalar Distance( const Point& p0, const Point& p1 )
//{
//    const Scalar minusTwo( Const<VF32>::Int<-2>() );
//    //return Sqrt( minusTwo * ( Normalise( p0 ) % Normalise( p1 ) ) );
//    return Sqrt( minusTwo * ( p0 % p1 ) * Rcp( Weight( p0 ) * Weight( p1 ) ) );
//}





/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


