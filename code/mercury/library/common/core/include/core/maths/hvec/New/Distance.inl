#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_DISTANCE_INL
#define SOFT_MATHS_HVEC_NEW_DISTANCE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_DISTANCE_INL
#define MATHS_HVEC_NEW_DISTANCE_INL
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
NOALIAS __forceinline const VF32 Distance2( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 )
{
    return Length2( p2 - p1 );
}

/// Vec4F32-Vec4F32 3D point Distance
NOALIAS __forceinline const VF32 Distance3( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 )
{
    return Length3( p2 - p1 );
}

/// Vec4F32-Vec4F32 4D point Distance
NOALIAS __forceinline const VF32 Distance4( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 )
{
    return Length4( p2 - p1 );
}


/// Vec4F32-Vec4F32 2D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared2( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 )
{
    return LengthSquared2( p2 - p1 );
}

/// Vec4F32-Vec4F32 3D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared3( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 )
{
    return LengthSquared3( p2 - p1 );
}

/// Vec4F32-Vec4F32 4D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared4( const Vec4F32 VECREF p1, const Vec4F32 VECREF p2 )
{
    return LengthSquared4( p2 - p1 );
}


/// Vector4-Vector4 4D point Distance
NOALIAS __forceinline const VF32 Distance( const Vector4 VECREF p1, const Vector4 VECREF p2 )
{
    return Distance4( p1.m_V, p2.m_V );
}

/// Vector4-Vector4 4D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared( const Vector4 VECREF p1, const Vector4 VECREF p2 )
{
    return DistanceSquared4( p1.m_V, p2.m_V );
}

/// Vector4-Vector4 4D point Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared( const Vector3 VECREF p1, const Vector3 VECREF p2 )
{
    return DistanceSquared3( p1.m_V, p2.m_V );
}

/// Point3-Point3 Distance Squared
NOALIAS __forceinline const VF32 DistanceSquared( const Point3 VECREF point1, const Point3 VECREF point2 )
{
    const Vector3 delta( point2 - point1 );
    return LengthSquared( delta );
}

/// Point3-Point3 Distance
NOALIAS __forceinline const VF32 Distance( const Point3 VECREF point1, const Point3 VECREF point2 )
{
    const Vector3 delta( point2 - point1 );
    return Length( delta );
}

/// Plane-Point3 Distance
NOALIAS __forceinline const VF32 Distance( const Plane VECREF plane, const Point3 VECREF point )
{
    const Vec4F32 point_xyz1( MakeVec4F32( point, Const<VF32>::One() ) );
    return Dot4( plane.m_V, point_xyz1 );
}

/// Point3-Plane Distance
NOALIAS __forceinline const VF32 Distance( const Point3 VECREF point, const Plane VECREF plane )
{
    return Distance( plane, point );
}

/// Plane-Sphere Distance: Returns distance from plane to sphere ( +ve in front, -ve behind ) or zero for intersection
NOALIAS __forceinline const VF32 Distance( const Plane VECREF plane, const Sphere VECREF sphere )
{
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 distance_to_centre( Distance( plane, Centre( sphere ) ) );
    const VF32 radius( Radius( sphere ) );
    return CopySign( Max( Abs( distance_to_centre ) - radius, zero ), distance_to_centre );
}

/// Sphere-Plane Distance: Returns distance from plane to sphere ( +ve in front, -ve behind ) or zero for intersection
NOALIAS __forceinline const VF32 Distance( const Sphere VECREF sphere, const Plane VECREF plane )
{
    return Distance( plane, sphere );
}

/// Sphere-Point3 Distance
NOALIAS __forceinline const VF32 Distance( const Sphere VECREF sphere, const Point3 VECREF point )
{
    return Distance( Centre( sphere ), point ) - Radius( sphere );
}

/// Point3-Sphere Distance
NOALIAS __forceinline const VF32 Distance( const Point3 VECREF point, const Sphere VECREF sphere )
{
    return Distance( sphere, point );
}

/// Sphere-Sphere Distance
NOALIAS __forceinline const VF32 Distance( const Sphere VECREF sphere1, const Sphere VECREF sphere2 )
{
    const VF32 d( Distance( Centre( sphere1 ), Centre( sphere2 ) ) );
    const VF32 rSum( Radius( sphere1 ) + Radius( sphere2 ) );
    return d - rSum;
}

/// Returns the distance between the closest points on the lines
NOALIAS __forceinline const VF32 Distance( const Line& line1, const Line& line2 )
{
    // http://mathworld.wolfram.com/Line-LineDistance.html
    // Line 1 : x1 + ( x2 - x1 ) s
    // Line 2 : x3 + ( x4 - x3 ) t
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // d = | c . ( a x b ) | / | a x b |
    
    const Vector3 a( Vector( line1 ) );
    const Vector3 b( Vector( line2 ) );
    const Vector3 c( Start( line2 ) - Start( line1 ) );
    const VF32    d( Abs( c | Normalise( a ^ b ) ) );
    return d;
}

/// Returns the distance squared between the given point and the closest point to it on the line
NOALIAS __forceinline const VF32 DistanceSquared( const Line& line, const Point3 VECREF point )
{
    // Point : x0
    // Line : x1 + ( x2 - x1 ) t
    // Minimum d^2 = | ( x2 - x1 ) x ( x1 - x0 ) | ^2 / | x2 - x1 | ^2
    // a = (x2-x1) x (x1-x0)
    // b = x2-x1
    // d^2 = a.a / b.b
    // 
    // Line normal, N = (x2-x1) / |x2-x1|
    // theta = angle between N & (x0-x1)
    // | N x (x0-x1) | = |N| * |x0-x1| * sin( theta )
    //                 =  1  * |x0-x1| * sin( theta )
    // d = perpendicular distance between point x0 and line (x2-x1)
    // From right angle triangle :
    // sin( theta ) = d / |x0-x1|
    // d = |x0-x1| * sin( theta )
    //   = | N x (x0-x1) |
    // dVec = N x (x0-x1)
    // d^2 = dVec . dVec
    
    const Vector3 dVec( Normal( line ) ^ ( point - Start( line ) ) );
    return dVec | dVec;
}

/// Returns the distance squared between the given point and the closest point to it on the line
NOALIAS __forceinline const VF32 DistanceSquared( const Point3 VECREF point, const Line& line )
{
    return DistanceSquared( line, point );
}

/// Returns the distance between the given point and the closest point to it on the line
NOALIAS __forceinline const VF32 Distance( const Line& line, const Point3 VECREF point )
{
    return Sqrt( DistanceSquared( line, point ) );
}

/// Returns the distance between the given point and the closest point to it on the line
NOALIAS __forceinline const VF32 Distance( const Point3 VECREF point, const Line& line )
{
    return Distance( line, point );
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

