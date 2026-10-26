#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_INTERSECT_H
#define SOFT_MATHS_HVEC_NEW_INTERSECT_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_INTERSECT_H
#define MATHS_HVEC_NEW_INTERSECT_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Intersect
/// @{

/// Plane-Line Intersection
NOALIAS __forceinline const VBool Intersect( const Plane PCREF plane, const Point3 PCREF p1, const Point3 PCREF p2, Point3* const __restrict pIntersectPoint );

/// Line-Plane Intersection
NOALIAS __forceinline const VBool Intersect( const Point3 PCREF p1, const Point3 PCREF p2, const Plane PCREF plane, Point3* const __restrict pIntersectPoint );


/// Plane-Line Intersection
NOALIAS __forceinline const VBool Intersect( const Plane PCREF plane, const Point3 PCREF p1, const Point3 PCREF p2, Point3* const __restrict pIntersectPoint, VF32* const __restrict pIntersectT );

/// Line-Plane Intersection
NOALIAS __forceinline const VBool Intersect( const Point3 PCREF p1, const Point3 PCREF p2, const Plane PCREF plane, Point3* const __restrict pIntersectPoint, VF32* const __restrict pIntersectT );


/// Plane-Line Intersection
NOALIAS __forceinline const VBool Intersect( const Plane PCREF plane, const Line& line, Point3* const __restrict pIntersectPoint );

/// Line-Plane Intersection
NOALIAS __forceinline const VBool Intersect( const Line& line, const Plane PCREF plane, Point3* const __restrict pIntersectPoint );


/// Plane-Line Intersection
NOALIAS __forceinline const VBool Intersect( const Plane PCREF plane, const Line& line, Point3* const __restrict pIntersectPoint, VF32* const __restrict pIntersectT );

/// Line-Plane Intersection
NOALIAS __forceinline const VBool Intersect( const Line& line, const Plane PCREF plane, Point3* const __restrict pIntersectPoint, VF32* const __restrict pIntersectT );


/// Sphere-Line Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
/// pResult will be filled in with the two t-values for the points on the line that intersect the sphere
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere, const Point3& lineStart, const Point3& lineEnd, QuadraticResult* const __restrict pResult );

/// Line-Sphere Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
/// pResult will be filled in with the two t-values for the points on the line that intersect the sphere
NOALIAS __forceinline const VBool Intersect( const Point3& lineStart, const Point3& lineEnd, const Sphere PCREF sphere, QuadraticResult* const __restrict pResult );


/// Sphere-Line Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
/// pResult will be filled in with the two t-values for the points on the line that intersect the sphere
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere, const Line& line, QuadraticResult* const __restrict pResult );

/// Line-Sphere Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
/// pResult will be filled in with the two t-values for the points on the line that intersect the sphere
NOALIAS __forceinline const VBool Intersect( const Line& line, const Sphere PCREF sphere, QuadraticResult* const __restrict pResult );


/// Sphere-Line Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere, const Point3& lineStart, const Point3& lineEnd );

/// Line-Sphere Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
NOALIAS __forceinline const VBool Intersect( const Point3& lineStart, const Point3& lineEnd, const Sphere PCREF sphere );


/// Sphere-Line Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere, const Line& line );

/// Line-Sphere Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
NOALIAS __forceinline const VBool Intersect( const Line& line, const Sphere PCREF sphere );


/// Sphere-Sphere Intersection
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere1, const Sphere PCREF sphere2 );


/// AxisBox-AxisBox Intersection
NOALIAS __forceinline const VBool Intersect( const AxisBox& box1, const AxisBox& box2 );

/// AxisBox-AxisBox Intersection
NOALIAS __forceinline const VBool Intersect( const AxisBox& box1, const AxisBox& box2, AxisBox* const __restrict pResult );


/// Line-Line Intersection: Returns true if the lines are coplanar & intersect with the t-values in the range [ 0.0f, 1.0f ]
/// NOTE: The lines must be coplanar. [ This can be tested with the line functions CoPlanar() & Skew() ]
/// The coplanar tolerance is the maximum distance between the closest points on the lines
NOALIAS __forceinline const VBool Intersect(
    const Line&            line1,
    const Line&            line2,
    VF32* const __restrict pT1,
    VF32* const __restrict pT2,
    const VF32 VECREF       coplanarTolerance = Const<VF32>::Frac<1,1000>() );

/// Line-Line Intersection: Returns true if the lines are coplanar & intersect with the t-values in the range [ 0.0f, 1.0f ]
/// NOTE: The lines must be coplanar. [ This can be tested with the line functions CoPlanar() & Skew() ]
/// The coplanar tolerance is the maximum distance between the closest points on the lines
NOALIAS __forceinline const VBool Intersect(
    const Line&      line1,
    const Line&      line2,
    const VF32 VECREF coplanarTolerance = Const<VF32>::Frac<1,1000>() );


/// Line-Line Intersection: Returns true if the lines are coplanar & intersect with any t-values
/// NOTE: The lines must be coplanar. [ This can be tested with the line functions CoPlanar() & Skew() ]
/// The coplanar tolerance is the maximum distance between the closest points on the lines
NOALIAS __forceinline const VBool IntersectInfiniteLines(
    const Line&             line1,
    const Line&             line2,
    VF32* const __restrict  pT1,
    VF32* const __restrict  pT2,
    const VF32 VECREF       coplanarTolerance = Const<VF32>::Frac<1,1000>() );

/// Line-Line Intersection: Returns true if the lines are coplanar & intersect with any t-values
/// NOTE: The lines must be coplanar. [ This can be tested with the line functions CoPlanar() & Skew() ]
/// The coplanar tolerance is the maximum distance between the closest points on the lines
/// 24 bit precision reciprocal/normalise version
NOALIAS __forceinline const VBool IntersectInfiniteLines24(
    const Line&             line1,
    const Line&             line2,
    VF32* const __restrict  pT1,
    VF32* const __restrict  pT2,
    const VF32 VECREF       coplanarTolerance = Const<VF32>::Frac<1,1000>() );


/// Plane-Plane Intersection : Returns true if the planes are not parallel. If true then pLine is the line of intersection between the planes.
NOALIAS __forceinline const VBool Intersect(
    const Plane PCREF plane1,
    const Plane PCREF plane2,
    Line * const __restrict pLine );


/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect(
    const Point3 PCREF lineStart,
    const Point3 PCREF lineEnd,
    const Point3 PCREF pointA,
    const Point3 PCREF pointB,
    const Point3 PCREF pointC );

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW(
    const Point3 PCREF lineStart,
    const Point3 PCREF lineEnd,
    const Point3 PCREF pointA,
    const Point3 PCREF pointB,
    const Point3 PCREF pointC );


/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect(
    const Point3 PCREF         lineStart,
    const Point3 PCREF         lineEnd,
    const Point3 PCREF         pointA,
    const Point3 PCREF         pointB,
    const Point3 PCREF         pointC,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV );

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW(
    const Point3 PCREF         lineStart,
    const Point3 PCREF         lineEnd,
    const Point3 PCREF         pointA,
    const Point3 PCREF         pointB,
    const Point3 PCREF         pointC,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV );


/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect( const Line& line, const Triangle& triangle );

/// Triangle-Line Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect( const Triangle& triangle, const Line& line );


/// Line-Triangle Intersection with Back Face Cull: returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW( const Line& line, const Triangle& triangle );

/// Triangle-Line Intersection with Back Face Cull: returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW( const Triangle& triangle, const Line& line );


/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect(
    const Line&                line,
    const Triangle&            triangle,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV );

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect(
    const Triangle&            triangle,
    const Line&                line,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV );

/// Line-Triangle Intersection with Back Face Cull : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW(
    const Line&                line,
    const Triangle&            triangle,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV );

/// Line-Triangle Intersection with Back Face Cull : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW(
    const Triangle&            triangle,
    const Line&                line,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV );


/// Triangle-Triangle Intersection
NOALIAS /*__force*/inline const VBool Intersect(
    const Triangle&          triangle1,
    const Triangle&          triangle2,
    Point3* const __restrict pP1,
    Point3* const __restrict pP2 );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

