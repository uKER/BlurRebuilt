#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_LINE_H
#define SOFT_MATHS_HVEC_NEW_LINE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_LINE_H
#define MATHS_HVEC_NEW_LINE_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Line
/// @{

/// Return point on line segment ab with parametric value t
NOALIAS __forceinline const Point3 PointOnLine( const Point3 PCREF a, const Point3 PCREF b, const VF32 VECREF t );


/// Make Line from start & end points
NOALIAS __forceinline const Line MakeLine( const Point3 PCREF start, const Point3 PCREF end );

/// Make Line from start point, normal & length
NOALIAS __forceinline const Line MakeLine( const Point3 PCREF start, const Normal3 PCREF normal, const VF32 VECREF length );

/// Make Line from start point, normal & length
NOALIAS __forceinline const Line MakeLine( const Point3 PCREF start, const Normal3 PCREF normal, const VF32 VECREF length, const VF32 VECREF rcpLength );

/// Make Line from start point & vector from start to end
NOALIAS __forceinline const Line MakeLine( const Point3 PCREF start, const Vector3 PCREF vector );

/// Get Line Start
NOALIAS __forceinline const Point3 Start( const Line& line );

/// Get Line End
NOALIAS __forceinline const Point3 End( const Line& line );

/// Get Line Length
NOALIAS __forceinline const VF32 Length( const Line& line );

/// Get Line Reciprocal Length
NOALIAS __forceinline const VF32 RcpLength( const Line& line );

/// Get Line Vector
NOALIAS __forceinline const Vector3 Vector( const Line& line );

/// Get Line Normal
NOALIAS __forceinline const Normal3 Normal( const Line& line );

/// Set Line Start
NOALIAS __forceinline void SetStart( Line& line, const Point3 PCREF start );

/// Set Line End
NOALIAS __forceinline void SetEnd( Line& line, const Point3 PCREF end );

/// Set Line Length: adjusts the end point to set the desired length (start point & direction are preserved)
NOALIAS __forceinline void SetLength( Line& line, const VF32 VECREF length );

/// Get point on Line: start t=0.0, end t=1.0
NOALIAS __forceinline const Point3 Point( const Line& line, const VF32 VECREF t );

/// Test if two lines are parallel
/// The tolerance is the dot product of the normals of the two lines (cosine of angle between the lines)
NOALIAS __forceinline const VBool Parallel( const Line& line1, const Line& line2, const VF32 VECREF tolerance = Const<VF32>::Frac<9999,10000>() );

/// Test if two lines are coplanar
/// The tolerance is the maximum distance between the closest points on the lines for them to be considered coplanar
NOALIAS __forceinline const VBool CoPlanar( const Line& line1, const Line& line2, const VF32 VECREF tolerance = Const<VF32>::Frac<1,1000>() );

/// Test if two lines are skew
/// The tolerance is the minimum distance between the closest points on the lines for them to be considered skew
NOALIAS __forceinline const VBool Skew( const Line& line1, const Line& line2, const VF32 VECREF tolerance = Const<VF32>::Frac<1,1000>() );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

