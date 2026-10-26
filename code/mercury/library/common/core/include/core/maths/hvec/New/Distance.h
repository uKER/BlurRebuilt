#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_DISTANCE_H
#define SOFT_MATHS_HVEC_NEW_DISTANCE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_DISTANCE_H
#define MATHS_HVEC_NEW_DISTANCE_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Distance
/// @{

/// Vec4F32-Vec4F32 2D point Distance
NOALIAS __forceinline const VF32 Distance2( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 );

/// Vec4F32-Vec4F32 3D point Distance
NOALIAS __forceinline const VF32 Distance3( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 );

/// Vec4F32-Vec4F32 4D point Distance
NOALIAS __forceinline const VF32 Distance4( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 );


/// Vec4F32-Vec4F32 2D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared2( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 );

/// Vec4F32-Vec4F32 3D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared3( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 );

/// Vec4F32-Vec4F32 4D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared4( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 );


/// Vector4-Vector4 4D point Distance
NOALIAS __forceinline const VF32 Distance( const Vector4 VECREF p1, const Vector4 VECREF p2 );

/// Vector4-Vector4 4D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared( const Vector4 VECREF p1, const Vector4 VECREF p2 );


/// Point3-Point3 Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared( const Point3 VECREF point1, const Point3 VECREF point2 );

/// Point3-Point3 Distance
NOALIAS __forceinline const VF32 Distance( const Point3 VECREF point1, const Point3 VECREF point2 );

/// Plane-Point3 Distance
NOALIAS __forceinline const VF32 Distance( const Plane VECREF plane, const Point3 VECREF point );

/// Point3-Plane Distance
NOALIAS __forceinline const VF32 Distance( const Point3 VECREF point, const Plane VECREF plane );

/// Plane-Sphere Distance: Returns distance from plane to sphere ( +ve in front, -ve behind ) or zero for intersection
NOALIAS __forceinline const VF32 Distance( const Plane VECREF plane, const Sphere VECREF sphere );

/// Sphere-Plane Distance: Returns distance from plane to sphere ( +ve in front, -ve behind ) or zero for intersection
NOALIAS __forceinline const VF32 Distance( const Sphere VECREF sphere, const Plane VECREF plane );

/// Sphere-Point3 Distance
NOALIAS __forceinline const VF32 Distance( const Sphere VECREF sphere, const Point3 VECREF point );

/// Point3-Sphere Distance
NOALIAS __forceinline const VF32 Distance( const Point3 VECREF point, const Sphere VECREF sphere );

/// Sphere-Sphere Distance
NOALIAS __forceinline const VF32 Distance( const Sphere VECREF sphere1, const Sphere VECREF sphere2 );

/// Returns the distance between the closest points on the lines
NOALIAS __forceinline const VF32 Distance( const Line& line1, const Line& line2 );

/// Returns the distance squared between the given point and the closest point to it on the line
NOALIAS __forceinline const VF32 DistanceSquared( const Line& line, const Point3 VECREF point );

/// Returns the distance squared between the given point and the closest point to it on the line
NOALIAS __forceinline const VF32 DistanceSquared( const Point3 VECREF point, const Line& line );

/// Returns the distance between the given point and the closest point to it on the line
NOALIAS __forceinline const VF32 Distance( const Line& line, const Point3 VECREF point );

/// Returns the distance between the given point and the closest point to it on the line
NOALIAS __forceinline const VF32 Distance( const Point3 VECREF point, const Line& line );






/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

