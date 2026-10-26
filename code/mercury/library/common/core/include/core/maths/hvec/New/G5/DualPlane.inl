#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_DUAL_PLANE_INL
#define SOFT_MATHS_HVEC_NEW_GA_DUAL_PLANE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_DUAL_PLANE_INL
#define MATHS_HVEC_NEW_GA_DUAL_PLANE_INL
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

NOALIAS /*__force*/inline const DualPlane MakeDualPlane( const Normal3 PCREF n, const VF32 VECREF d )
{
    return DualPlane( X(n), Y(n), Z(n), d );
}

NOALIAS /*__force*/inline const Scalar Weight( const DualPlane& dp )
{
    return Sqrt( dp % dp );
}

NOALIAS /*__force*/inline const Scalar RcpWeight( const DualPlane& dp )
{
    //return Rcp( Weight( dp ) );
    return RcpSqrt( dp % dp );
}

NOALIAS /*__force*/inline const Scalar Distance( const DualPlane& dp, const Point& p )
{
    //return ( dp % p ) * Rcp( Weight( dp ) * Weight( p ) );
    return ( p % dp ) * Rcp( Weight( dp ) * Weight( p ) );
}

NOALIAS /*__force*/inline const Scalar Distance( const Point& p, const DualPlane& dp )
{
    //return ( dp % p ) * Rcp( Weight( dp ) * Weight( p ) );
    return ( p % dp ) * Rcp( Weight( dp ) * Weight( p ) );
}




/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


