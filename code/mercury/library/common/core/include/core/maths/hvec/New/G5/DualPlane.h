#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_DUAL_PLANE_H
#define SOFT_MATHS_HVEC_NEW_GA_DUAL_PLANE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_DUAL_PLANE_H
#define MATHS_HVEC_NEW_GA_DUAL_PLANE_H
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

NOALIAS __forceinline const DualPlane MakeDualPlane( const Normal3 PCREF n, const VF32 VECREF d );

NOALIAS __forceinline const Scalar Distance( const DualPlane& plane, const Point& point );
NOALIAS __forceinline const Scalar Distance( const Point& point, const DualPlane& plane );



/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


