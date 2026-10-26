#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_VECTOR_INL
#define SOFT_MATHS_HVEC_NEW_GA_VECTOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_VECTOR_INL
#define MATHS_HVEC_NEW_GA_VECTOR_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

#if defined( COMPILER_VC )
#pragma warning( push )
#pragma warning( disable : 4503 )
#endif

namespace Maths
{
namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{

NOALIAS __forceinline const Vector MakeVector( const Vector3 PCREF e3v )
{
    //return Vector( X(e3v), Y(e3v), Z(e3v) );
    return Vector( e3v.m_V );
}

NOALIAS __forceinline const Vector MakeVector( const Normal3 PCREF e3n )
{
    //return Vector( X(e3n), Y(e3n), Z(e3n) );
    return Vector( e3n.m_V );
}

NOALIAS __forceinline const Scalar Length( const Vector& v )
{
    return v % v;
}

NOALIAS __forceinline const Vector Normalise( const Vector& v )
{
    return v * RcpSqrt( Length( v ) );
}







/// @}
}; // namespace GA
}; // namespace Maths

#if defined( COMPILER_VC )
#pragma warning( pop )
#endif

#endif // #if defined( INCLUDE_THIS )


