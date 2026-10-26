#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_VB_H
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_VB_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_VB_H
#define MATHS_HVEC_NEW_G3V_MULTIVEC_VB_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVecVB
/// @{

/// Make MultiVecVB from Vector & BiVector
NOALIAS __forceinline const MultiVecVB MakeMultiVecVB( const Vector PCREF v, const BiVector PCREF b );

/// Get Vector from MultiVecVB
NOALIAS __forceinline const Vector GetVector( const MultiVecVB& vb );

/// Get BiVector from MultiVecVB
NOALIAS __forceinline const BiVector GetBiVector( const MultiVecVB& vb );




/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


