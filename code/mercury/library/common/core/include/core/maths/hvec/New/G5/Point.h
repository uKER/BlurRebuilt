#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_POINT_H
#define SOFT_MATHS_HVEC_NEW_GA_POINT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_POINT_H
#define MATHS_HVEC_NEW_GA_POINT_H
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

NOALIAS __forceinline const Point MakePoint( const VF32 VECREF x, const VF32 VECREF y, const VF32 VECREF z );

NOALIAS __forceinline const Point MakePoint( const Point3 PCREF e3p );

NOALIAS __forceinline const Point MakePoint( const Vector& v );

NOALIAS __forceinline const Scalar Weight( const Point& p );

NOALIAS __forceinline const Scalar RcpWeight( const Point& p );

NOALIAS __forceinline const Point Normalise( const Point& p );

NOALIAS __forceinline const Scalar Distance( const Point& p0, const Point& p1 );




/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


