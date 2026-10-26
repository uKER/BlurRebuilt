#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_SCALAR_H
#define SOFT_MATHS_HVEC_NEW_GA_SCALAR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_SCALAR_H
#define MATHS_HVEC_NEW_GA_SCALAR_H
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

NOALIAS __forceinline const F32 GetF32( const Scalar& s );

NOALIAS __forceinline const Scalar Rcp( const Scalar& s );

NOALIAS __forceinline const Scalar Sqrt( const Scalar& s );

NOALIAS __forceinline const Scalar RcpSqrt( const Scalar& s );




/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


