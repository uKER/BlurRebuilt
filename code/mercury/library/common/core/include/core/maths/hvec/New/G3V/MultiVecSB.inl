#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_SB_INL
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_SB_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_SB_INL
#define MATHS_HVEC_NEW_G3V_MULTIVEC_SB_INL
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
//NOALIAS __forceinline const MultiVecSB MakeMultiVecSB( const VF32 VECREF s, const Vec4F32 VECREF b )
//{
//    MultiVecSB result;
//    //result.m_SB = b;
//    //SetW( &result.m_SB, s );
//    result.m_SB = MakeVector4( MakeVector3( b ), s );
//    return result;
//}

/// Make MultiVecSB from Scalar & BiVector
NOALIAS __forceinline const MultiVecSB MakeMultiVecSB( const VF32 VECREF s, const BiVector PCREF b )
{
    MultiVecSB result;
    //result.m_SB = b.m_B;
    //SetW( &result.m_SB, s );
    result.m_SB = MakeVector4( b.m_B, s );
    return result;
}

/// Get Scalar from MultiVecSB
NOALIAS __forceinline const VF32 GetScalar( const MultiVecSB PCREF sb )
{
    return W( sb.m_SB );
}

/// Get BiVector from MultiVecSB
NOALIAS __forceinline const BiVector GetBiVector( const MultiVecSB PCREF sb )
{
    BiVector result;
    result.m_B = MakeVector3( sb.m_SB );
    return result;
}



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


