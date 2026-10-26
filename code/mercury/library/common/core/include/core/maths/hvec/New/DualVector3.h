#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_DUAL_VECTOR3_H
#define SOFT_MATHS_HVEC_NEW_DUAL_VECTOR3_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_DUAL_VECTOR3_H
#define MATHS_HVEC_NEW_DUAL_VECTOR3_H
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
//NOALIAS __forceinline const DualVector3 MakeDualVector3( const Vec4F32 VECREF v0, const Vec4F32 VECREF ve );

/// Make DualVector3 from pair of Vector3's
//NOALIAS __forceinline const DualVector3 MakeDualVector3( const Vector3 PCREF v0, const Vector3 PCREF ve );

/// DualVector3 Get Real part
//NOALIAS __forceinline const Vector3 Real( const DualVector3& a );

/// DualVector3 Get Dual part
//NOALIAS __forceinline const Vector3 Dual( const DualVector3& a );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

