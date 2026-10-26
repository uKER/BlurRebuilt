#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_ROTOR_INL
#define SOFT_MATHS_HVEC_NEW_GA_ROTOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_ROTOR_INL
#define MATHS_HVEC_NEW_GA_ROTOR_INL
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

NOALIAS __forceinline const Rotor MakeRotor( const Normal3& n1, const Normal3& n2 )
{
    return MakeVector(n2) * MakeVector(n1);
}

NOALIAS __forceinline const Rotor MakeRotor( const Vector& v1, const Vector& v2 )
{
    return Normalise(v2) * Normalise(v1);
}




/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


