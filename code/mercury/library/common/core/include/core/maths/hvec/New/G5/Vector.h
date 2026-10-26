#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_VECTOR_H
#define SOFT_MATHS_HVEC_NEW_GA_VECTOR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_VECTOR_H
#define MATHS_HVEC_NEW_GA_VECTOR_H
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

NOALIAS __forceinline const Vector MakeVector( const Vector3 PCREF e3v );

NOALIAS __forceinline const Vector MakeVector( const Normal3 PCREF e3n );

NOALIAS __forceinline const Scalar Length( const Vector& v );

NOALIAS __forceinline const Vector Normalise( const Vector& v );



/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


