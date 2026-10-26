#ifndef MATHS_HVEC_NEW_RENDER_H
#define MATHS_HVEC_NEW_RENDER_H
#define INCLUDE_THIS
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup NewMathsRender
//! @{

/// Draw point
void DrawPoint( const Point3 PCREF point, const RGBA colour );

/// Draw point
void Draw( const Point3 PCREF point, const RGBA colour );

/// Draw line
void DrawLine( const Point3 PCREF start, const Point3 PCREF end, const RGBA colour );

/// Draw circle
void DrawCircle( const Point3 PCREF point, const Normal3 PCREF normal, const VF32 VECREF radius, const RGBA colour );

/// Draw arrow head
void DrawArrowHead( const Point3 PCREF target, const Normal3 PCREF direction, const VF32 VECREF length, const VF32 VECREF radius, const RGBA colour );

/// Draw arrow
void DrawArrow( const Point3 PCREF start, const Point3 PCREF end, const RGBA colour );

/// Draw arrow
void DrawArrow( const Point3 PCREF point, const Vector3 PCREF vec, const RGBA colour );

/// Draw arrow
void DrawArrow( const Point3 PCREF point, const Normal3 PCREF normal, const VF32 VECREF length, const RGBA colour );

/// Draw arrow
void DrawArrow( const Line& line, const RGBA colour );

/// Draw arc
void DrawArc( const Point3 PCREF point, const Normal3 PCREF normal, const VF32 VECREF radius, const VF32 VECREF angle, const RGBA colour );

/// Draw arc segment with arrowhead
void DrawArcSegmentArrow( const Point3 PCREF point, const Normal3 PCREF normal, const VF32 VECREF radius, const VF32 VECREF angle, const RGBA colour );

/// Draw Line
void Draw( const Line& line, const RGBA colour );

/// Draw Sphere
void Draw( const Sphere PCREF sphere, const RGBA colour );

/// Draw AxisBox
void Draw( const AxisBox& box, const RGBA colour );

/// Draw OrientedBox
void Draw( const OrientedBox& box, const RGBA colour );

/// Draw Mat33
void Draw( const Mat33 MATREF matrix, const Point3 PCREF point );

/// Draw Mat33: Variant that takes the parameter 'point' as a Vector3 instead of a Point3
void Draw( const Mat33 MATREF matrix, const Vector3 PCREF point );

/// Draw Mat43
void Draw( const Mat43 MATREF matrix );

/// Draw Quaternion
void Draw( const Quaternion PCREF quaternion, const Point3 PCREF point );

/// Draw Quaternion: Variant that takes the parameter 'point' as a Vector3 instead of a Point3
void Draw( const Quaternion PCREF quaternion, const Vector3 PCREF point );

/// Draw QuatTrans
void Draw( const QuatTrans& qt );

/// Draw DualQuaternion
void Draw( const DualQuaternion& dq );

/// Draw Plane
void Draw( const Plane PCREF plane, const RGBA colour );

/// Draw Triangle
void Draw( const Triangle PCREF triangle, const RGBA colour );




//! @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

