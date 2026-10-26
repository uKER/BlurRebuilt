#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_BT_INL
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_BT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_BT_INL
#define MATHS_HVEC_NEW_G3V_MULTIVEC_BT_INL
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
NOALIAS __forceinline const MultiVecBT MakeMultiVecBT( const BiVector PCREF b, const TriVector PCREF t )
{
    MultiVecBT result;
    //result.m_BT = b.m_B;
    //SetW( &result.m_BT, t.m_T );
    result.m_BT = MakeVector4( b.m_B, t.m_T );
    return result;
}

/// Get BiVector from MultiVecBT
NOALIAS __forceinline const BiVector GetBiVector( const MultiVecBT PCREF bt )
{
    BiVector result;
    result.m_B = MakeVector3( bt.m_BT );
    return result;
}

/// Get TriVector from MultiVecBT
NOALIAS __forceinline const TriVector GetTriVector( const MultiVecBT PCREF bt )
{
    TriVector result;
    result.m_T = W( bt.m_BT );
    return result;
}





/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


