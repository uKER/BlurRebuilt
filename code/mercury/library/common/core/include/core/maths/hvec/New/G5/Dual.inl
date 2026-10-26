#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_DUAL_INL
#define SOFT_MATHS_HVEC_NEW_GA_DUAL_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_DUAL_INL
#define MATHS_HVEC_NEW_GA_DUAL_INL
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

NOALIAS __forceinline const DualPlane Dual( const Plane& p )
{
    return p << Const<PseudoScalar>::Inverse();
    //return p << Const<PseudoScalar>::One();
}

NOALIAS __forceinline const Plane Direct( const DualPlane& dp )
{
    //return dp << Const<PseudoScalar>::Inverse();
    return dp << Const<PseudoScalar>::One();
}


/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


