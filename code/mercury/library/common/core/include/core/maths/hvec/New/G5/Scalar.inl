#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_SCALAR_INL
#define SOFT_MATHS_HVEC_NEW_GA_SCALAR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_SCALAR_INL
#define MATHS_HVEC_NEW_GA_SCALAR_INL
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

NOALIAS __forceinline const F32 GetF32( const Scalar& s )
{
    return GetF32( Element< BasisBits_s >( s ) );
}

NOALIAS __forceinline const Scalar Rcp( const Scalar& s )
{
    return Scalar( Rcp( Element<BasisBits_s>( s ) ) );
}

NOALIAS __forceinline const Scalar Sqrt( const Scalar& s )
{
    return Scalar( Sqrt( Element<BasisBits_s>( s ) ) );
}

NOALIAS __forceinline const Scalar RcpSqrt( const Scalar& s )
{
    return Scalar( RcpSqrt( Element<BasisBits_s>( s ) ) );
}






/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


