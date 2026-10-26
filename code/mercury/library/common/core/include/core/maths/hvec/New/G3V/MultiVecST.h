#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_ST_H
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_ST_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_ST_H
#define MATHS_HVEC_NEW_G3V_MULTIVEC_ST_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVecST
/// @{

/// Make MultiVecST from Scalar & TriVector
NOALIAS __forceinline const MultiVecST MakeMultiVecST( const VF32 VECREF s, const TriVector PCREF t );

/// Get Scalar from MultiVecST
NOALIAS __forceinline const VF32 GetScalar( const MultiVecST PCREF st );

/// Get TriVector from MultiVecST
NOALIAS __forceinline const TriVector GetTriVector( const MultiVecST PCREF st );





/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


