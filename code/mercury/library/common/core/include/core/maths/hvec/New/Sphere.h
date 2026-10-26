#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_SPHERE_H
#define SOFT_MATHS_HVEC_NEW_SPHERE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_SPHERE_H
#define MATHS_HVEC_NEW_SPHERE_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Sphere
/// @{

/// Make Sphere from centre & radius
NOALIAS __forceinline const Sphere MakeSphere( const Point3 PCREF centre, const VF32 VECREF radius );

/// Make Sphere from AxisBox (returns the bounding sphere of the box)
NOALIAS __forceinline const Sphere MakeSphere( const AxisBox& box );

/// Sphere get Centre
NOALIAS __forceinline const Point3 Centre( const Sphere PCREF sphere );

/// Sphere get Radius
NOALIAS __forceinline const VF32 Radius( const Sphere PCREF sphere );

/// Sphere get bounding box
NOALIAS __forceinline const AxisBox BoundingBox( const Sphere PCREF sphere );

/// Sphere + Vector3
NOALIAS __forceinline const Sphere operator + ( const Sphere PCREF sphere, const Vector3 PCREF vector );

/// Sphere - Vector3
NOALIAS __forceinline const Sphere operator - ( const Sphere PCREF sphere, const Vector3 PCREF vector );

/// Sphere * Mat33 (NOTE: the matrix must be orthonormalised!)
NOALIAS __forceinline const Sphere operator * ( const Sphere PCREF sphere, const Mat33 MATREF matrix33 );

/// Sphere * Mat43 (NOTE: the rotation part of matrix43 must be orthonormalised!)
NOALIAS __forceinline const Sphere operator * ( const Sphere PCREF sphere, const Mat43 MATREF matrix43 );

/// Sphere Transform: sphere * quaternion + vector
NOALIAS __forceinline const Sphere Transform( const Sphere PCREF sphere, const Quaternion PCREF quaternion, const Vector3 PCREF vector );

/// Sphere Serialise to stream
template < typename T_Stream >
__forceinline void Serialise( const Sphere PCREF sphere, T_Stream& stream );

/// Sphere Deserialise from stream
template < typename T_Stream >
__forceinline void Deserialise( Sphere& sphere, T_Stream& stream );


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

