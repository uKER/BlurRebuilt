#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_SB_H
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_SB_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_SB_H
#define MATHS_HVEC_NEW_G3V_MULTIVEC_SB_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVecSB
/// @{

/// Make MultiVecSB from Scalar & Vec4F32 BiVector
//NOALIAS __forceinline const MultiVecSB MakeMultiVecSB( const VF32 VECREF s, const Vec4F32 VECREF b );

/// Make MultiVecSB from Scalar & BiVector
NOALIAS __forceinline const MultiVecSB MakeMultiVecSB( const VF32 VECREF s, const BiVector PCREF b );

/// Get Scalar from MultiVecSB
NOALIAS __forceinline const VF32 GetScalar( const MultiVecSB PCREF sb );

/// Get BiVector from MultiVecSB
NOALIAS __forceinline const BiVector GetBiVector( const MultiVecSB PCREF sb );




/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


