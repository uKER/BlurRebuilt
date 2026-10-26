#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_SV_H
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_SV_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_SV_H
#define MATHS_HVEC_NEW_G3V_MULTIVEC_SV_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVecSV
/// @{

/// Make MultiVecSV from Scalar & Vector
NOALIAS __forceinline const MultiVecSV MakeMultiVecSV( const VF32 VECREF s, const Vector PCREF v );

/// Get Scalar from MultiVecSV
NOALIAS __forceinline const VF32 GetScalar( const MultiVecSV PCREF sv );

/// Get Vector from MultiVecSV
NOALIAS __forceinline const Vector GetVector( const MultiVecSV PCREF sv );



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


