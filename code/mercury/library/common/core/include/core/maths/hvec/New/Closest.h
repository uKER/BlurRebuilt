#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLOSEST_H
#define SOFT_MATHS_HVEC_NEW_CLOSEST_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLOSEST_H
#define MATHS_HVEC_NEW_CLOSEST_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Closest
/// @{

/// Find closest point on plane to input point
NOALIAS __forceinline const Point3 ClosestPointOnPlane( const Point3 PCREF point, const Plane PCREF plane );


/// Find closest point on surface of sphere to point p
/// NOTE: If p is exactly at the centre of the sphere, the centre point is returned
NOALIAS __forceinline const Point3 ClosestPointOnSphere( const Point3 PCREF p, const Sphere PCREF s );


/// Returns t value of closest point on the infinite line through the line ab to the point p
NOALIAS __forceinline const VF32 ClosestPointOnInfiniteLineT( const Point3 PCREF p, const Point3 PCREF a, const Point3 PCREF b );

/// Returns t value of closest point on the infinite line through the line ab to the point p
NOALIAS __forceinline const VF32 ClosestPointOnInfiniteLineT( const Point3 PCREF p, const Line& line );


/// Returns t value of closest point on the line ab to the point p
NOALIAS __forceinline const VF32 ClosestPointOnLineSegmentT( const Point3 PCREF p, const Point3 PCREF a, const Point3 PCREF b );

/// Returns t value of closest point on this Line to the given point
NOALIAS __forceinline const VF32 ClosestPointOnLineSegmentT( const Point3 PCREF point, const Line& line );


/// Returns closest point on the infinite line through ab to the point p
NOALIAS __forceinline const Point3 ClosestPointOnInfiniteLine( const Point3 PCREF p, const Point3 PCREF a, const Point3 PCREF b );

/// Returns closest point on the infinite line through ab to the point p
NOALIAS __forceinline const Point3 ClosestPointOnInfiniteLine( const Point3 PCREF p, const Line& line );


/// Returns closest point on the line ab to the point p
NOALIAS __forceinline const Point3 ClosestPointOnLineSegment( const Point3 PCREF p, const Point3 PCREF a, const Point3 PCREF b );

/// Returns closest point on the line to the given point
NOALIAS __forceinline const Point3 ClosestPointOnLineSegment( const Point3 PCREF point, const Line& line );


/// Find closest point on triangle to input point
NOALIAS __forceinline const Point3 ClosestPointOnTriangle(
    const Point3 PCREF p,   // find point closest to p on triangle abc
    const Point3 PCREF a,   // triangle point a
    const Point3 PCREF b,   // triangle point b
    const Point3 PCREF c ); // triangle point c

/// Find closest point on triangle to input point ( optimised for p being very close to triangle )
/// NOTE: This version uses a suggested 'optimisation': in all test cases it's been shown to be slower than the standard version!
DEPRECATED NOALIAS __forceinline const Point3 ClosestPointOnTriangleV2(
    const Point3 PCREF p,   // find point closest to p on triangle abc
    const Point3 PCREF a,   // triangle point a
    const Point3 PCREF b,   // triangle point b
    const Point3 PCREF c ); // triangle point c



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

