#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_VT_INL
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_VT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_VT_INL
#define MATHS_HVEC_NEW_G3V_MULTIVEC_VT_INL
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
NOALIAS __forceinline const MultiVecVT MakeMultiVecVT( const Vector PCREF v, const TriVector PCREF t )
{
    MultiVecVT result;
    //result.m_VT = v.m_V;
    //SetW( &result.m_VT, t.m_T );
    result.m_VT = MakeVector4( v.m_V, t.m_T );
    return result;
}

/// Get Vector from MultiVecVT
NOALIAS __forceinline const Vector GetVector( const MultiVecVT PCREF vt )
{
    Vector result;
    result.m_V = MakeVector3( vt.m_VT );
    return result;
}

/// Get TriVector from MultiVecVT
NOALIAS __forceinline const TriVector GetTriVector( const MultiVecVT PCREF vt )
{
    TriVector result;
    result.m_T = W( vt.m_VT );
    return result;
}




/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


