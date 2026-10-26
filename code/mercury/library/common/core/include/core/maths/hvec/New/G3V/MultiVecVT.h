#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_VT_H
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_VT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_VT_H
#define MATHS_HVEC_NEW_G3V_MULTIVEC_VT_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVecVT
/// @{

/// Make MultiVecVT from Vector & TriVector
NOALIAS __forceinline const MultiVecVT MakeMultiVecVT( const Vector PCREF v, const TriVector PCREF t );

/// Get Vector from MultiVecVT
NOALIAS __forceinline const Vector GetVector( const MultiVecVT PCREF vt );

/// Get TriVector from MultiVecVT
NOALIAS __forceinline const TriVector GetTriVector( const MultiVecVT PCREF vt );





/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


