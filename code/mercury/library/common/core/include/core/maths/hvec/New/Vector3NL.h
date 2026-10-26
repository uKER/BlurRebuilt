#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VECTOR3NL_H
#define SOFT_MATHS_HVEC_NEW_VECTOR3NL_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VECTOR3NL_H
#define MATHS_HVEC_NEW_VECTOR3NL_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

/// New Vector Maths Library
namespace Maths
{
/// @addtogroup Vector3NL
/// @{

/// Make Vector3NL from Vector3 (faster but only 22 bit precision)
NOALIAS __forceinline const Vector3NL MakeVector3NL( const Vector3 PCREF vector );

/// Make Vector3NL from Vector3 (slower but full 24 bit precision)
NOALIAS __forceinline const Vector3NL MakeVector3NL_24( const Vector3 PCREF vector );

/// Make Vector3NL from Normal3 & VF32 length
NOALIAS __forceinline const Vector3NL MakeVector3NL( const Normal3 PCREF normal, const VF32 VECREF length );

/// Get Vector3NL vector
NOALIAS __forceinline const Vector3 Vector( const Vector3NL PCREF vector3nl );

/// Get Vector3NL normal
NOALIAS __forceinline const Normal3 Normal( const Vector3NL PCREF vector3nl );

/// Get Vector3NL length
NOALIAS __forceinline const VF32 Length( const Vector3NL PCREF vector3nl );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

