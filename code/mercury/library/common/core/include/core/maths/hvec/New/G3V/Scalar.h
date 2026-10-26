#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_SCALAR_H
#define SOFT_MATHS_HVEC_NEW_G3V_SCALAR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_SCALAR_H
#define MATHS_HVEC_NEW_G3V_SCALAR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_Scalar
/// @{

/// Scalar Reversion
NOALIAS __forceinline const VF32 Reverse( const VF32 VECREF s );

/// Scalar Grade-Involution
NOALIAS __forceinline const VF32 Involution( const VF32 VECREF s );



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


