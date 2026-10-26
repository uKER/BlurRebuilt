#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_SV_INL
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_SV_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_SV_INL
#define MATHS_HVEC_NEW_G3V_MULTIVEC_SV_INL
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
NOALIAS __forceinline const MultiVecSV MakeMultiVecSV( const VF32 VECREF s, const Vector PCREF v )
{
    MultiVecSV result;
    //result.m_SV = v.m_V;
    //SetW( &result.m_SV, s );
    result.m_SV = MakeVector4( v.m_V, s );
    return result;
}

/// Get Scalar from MultiVecSV
NOALIAS __forceinline const VF32 GetScalar( const MultiVecSV PCREF sv )
{
    return W( sv.m_SV );
}

/// Get Vector from MultiVecSV
NOALIAS __forceinline const Vector GetVector( const MultiVecSV PCREF sv )
{
    Vector result;
    result.m_V = MakeVector3( sv.m_SV );
    return result;
}



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


