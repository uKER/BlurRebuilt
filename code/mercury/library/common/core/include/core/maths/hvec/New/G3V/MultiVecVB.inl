#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_VB_INL
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_VB_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_VB_INL
#define MATHS_HVEC_NEW_G3V_MULTIVEC_VB_INL
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
NOALIAS __forceinline const MultiVecVB MakeMultiVecVB( const Vector PCREF v, const BiVector PCREF b )
{
    MultiVecVB result;
    result.m_V = v.m_V;
    result.m_B = b.m_B;
    return result;
}

/// Get Vector from MultiVecVB
NOALIAS __forceinline const Vector GetVector( const MultiVecVB& vb )
{
    return MakeVector( vb.m_V );
}

/// Get BiVector from MultiVecVB
NOALIAS __forceinline const BiVector GetBiVector( const MultiVecVB& vb )
{
    return MakeBiVector( vb.m_B );
}




/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


