#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_DUAL_SPHERE_H
#define SOFT_MATHS_HVEC_NEW_GA_DUAL_SPHERE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_DUAL_SPHERE_H
#define MATHS_HVEC_NEW_GA_DUAL_SPHERE_H
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

NOALIAS __forceinline const DualSphere MakeDualSphere( const Point3 PCREF c, const VF32 VECREF r );

NOALIAS __forceinline const DualSphere MakeDualSphere( const Point& c, const VF32 VECREF r );

NOALIAS __forceinline const Scalar Distance( const DualSphere& ds, const Point& p );



/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


