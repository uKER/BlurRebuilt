#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_BT_H
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_BT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_BT_H
#define MATHS_HVEC_NEW_G3V_MULTIVEC_BT_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVecBT
/// @{

/// Make MultiVecBT from BiVector & TriVector
NOALIAS __forceinline const MultiVecBT MakeMultiVecBT( const BiVector PCREF b, const TriVector PCREF t );

/// Get BiVector from MultiVecBT
NOALIAS __forceinline const BiVector GetBiVector( const MultiVecBT PCREF bt );

/// Get TriVector from MultiVecBT
NOALIAS __forceinline const TriVector GetTriVector( const MultiVecBT PCREF bt );




/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


