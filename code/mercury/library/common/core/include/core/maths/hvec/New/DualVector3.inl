#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_DUAL_VECTOR3_INL
#define SOFT_MATHS_HVEC_NEW_DUAL_VECTOR3_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_DUAL_VECTOR3_INL
#define MATHS_HVEC_NEW_DUAL_VECTOR3_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup DualVector3
/// @{

/// Make DualVector3 from pair of Vec4F32's
//NOALIAS __forceinline const DualVector3 MakeDualVector3( const Vec4F32 VECREF v0, const Vec4F32 VECREF ve )
//{
//    DualVector3 result;
//    result.m_V0.m_V = v0;
//    result.m_Ve.m_V = ve;
//    return result;
//}

/// Make DualVector3 from pair of Vector3's
//NOALIAS __forceinline const DualVector3 MakeDualVector3( const Vector3 PCREF v0, const Vector3 PCREF ve )
//{
//    DualVector3 result;
//    result.m_V0 = v0;
//    result.m_Ve = ve;
//    return result;
//}

/// DualVector3 Get Real part
//NOALIAS __forceinline const Vector3 Real( const DualVector3& a )
//{
//    return a.m_V0;
//}

/// DualVector3 Get Dual part
//NOALIAS __forceinline const Vector3 Dual( const DualVector3& a )
//{
//    return a.m_Ve;
//}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

