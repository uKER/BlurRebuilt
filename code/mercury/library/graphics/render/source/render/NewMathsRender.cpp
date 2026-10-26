#include "StdAfx.h"
#include "Display/Render/BasicRender.h"
#include "Render/Debug/ImmediateDebugRender.h"
#include <core/maths/hvec/New/IncludeGeneral.h>
#include <core/maths/hvec/New/IncludeGeneral.inl>
#include "Render/Render/NewMathsRender.h"

namespace Maths
{
/// @addtogroup NewMathsRender
//! @{

/// Draw point
void DrawPoint( const Point3 PCREF point, const RGBA colour )
{
    DbUtil::I_DebugRender* pRender = Render::Debug::Immediate();
    pRender->Draw3dPoint( HVec3( point ), 0.05f, colour );
    pRender->Draw3dSphereOutline( HVec3( point ), 0.05f, colour );
}

/// Draw point
void Draw( const Point3 PCREF point, const RGBA colour )
{
    DrawPoint( point, colour );
}

/// Draw line
void DrawLine( const Point3 PCREF start, const Point3 PCREF end, const RGBA colour )
{
    Display::BasicRender()->DrawColoured3DLine( GetF32( X( start ) ), GetF32( Y( start ) ), GetF32( Z( start ) ),
                                                GetF32( X( end ) ), GetF32( Y( end ) ), GetF32( Z( end ) ), colour );
}

/// Draw circle
void DrawCircle( const Point3 PCREF point, const Normal3 PCREF normal, const VF32 VECREF radius, const RGBA colour )
{
    Display::BasicRender()->DrawColoured3DCircle( HVec3( point ), HVec3( normal ), GetF32( radius ), colour );
}

/// Draw arrow head
void DrawArrowHead( const Point3 PCREF target, const Normal3 PCREF direction, const VF32 VECREF length, const VF32 VECREF radius, const RGBA colour )
{
    const VF32    cos45deg( Const<VF32>::Sqrt2div2() );
    const Vector3 axis_y( Const<Vector3>::UnitY() );
    const Vector3 axis_z( Const<Vector3>::UnitZ() );
    const Vector3 perp_a( Select( Abs( direction | axis_y ) <= cos45deg, axis_y, axis_z ) );
    const Vector3 perp_b( perp_a ^ direction );
    const Vector3 perp_c( direction ^ perp_b );
    const Normal3 normal_x( Normalise( perp_b ) );
    const Normal3 normal_y( Normalise( perp_c ) );
    const Point3  centre( target - ( direction * length ) );
    const Point3  arrow_head0( centre + ( normal_x * radius ) );
    const Point3  arrow_head1( centre - ( normal_x * radius ) );
    const Point3  arrow_head2( centre + ( normal_y * radius ) );
    const Point3  arrow_head3( centre - ( normal_y * radius ) );
    DrawLine( target, arrow_head0, colour );
    DrawLine( target, arrow_head1, colour );
    DrawLine( target, arrow_head2, colour );
    DrawLine( target, arrow_head3, colour );
    DrawCircle( centre, direction, radius, colour );
}

/// Draw arrow
void DrawArrow( const Point3 PCREF start, const Point3 PCREF end, const RGBA colour )
{
    const Vector3 vec( end - start );
    const VF32    length( Length( vec ) );
    const VF32    arrow_head_length_ratio( Const<VF32>::Frac<1,5>() );
    const VF32    arrow_head_radius_ratio( Const<VF32>::Frac<1,20>() );
    DrawLine( start, end, colour );
    DrawArrowHead( end, Normalise( vec ), length * arrow_head_length_ratio, length * arrow_head_radius_ratio, colour );
}

/// Draw arrow
void DrawArrow( const Point3 PCREF point, const Vector3 PCREF vec, const RGBA colour )
{
    const Point3 end( point + vec );
    const VF32   length( Length( vec ) );
    const VF32   arrow_head_length_ratio( Const<VF32>::Frac<1,5>() );
    const VF32   arrow_head_radius_ratio( Const<VF32>::Frac<1,20>() );
    DrawLine( point, end, colour );
    DrawArrowHead( end, Normalise( vec ), length * arrow_head_length_ratio, length * arrow_head_radius_ratio, colour );
}

/// Draw arrow
void DrawArrow( const Point3 PCREF point, const Normal3 PCREF normal, const VF32 VECREF length, const RGBA colour )
{
    const Point3  end( point + ( normal * length ) );
    const VF32    arrow_head_length_ratio( Const<VF32>::Frac<1,5>() );
    const VF32    arrow_head_radius_ratio( Const<VF32>::Frac<1,20>() );
    DrawLine( point, end, colour );
    DrawArrowHead( end, normal, length * arrow_head_length_ratio, length * arrow_head_radius_ratio, colour );
}

/// Draw arrow
void DrawArrow( const Line& line, const RGBA colour )
{
    DrawArrow( Start( line ), End( line ), colour );
}

/// Draw arc
void DrawArc( const Point3 PCREF point, const Normal3 PCREF normal, const VF32 VECREF radius, const VF32 VECREF angle, const RGBA colour )
{
    const VF32       zero( Const<VF32>::Zero() );
    //const VF32       one( Const<VF32>::One() );
    const VF32       rcpTwoPI( Const<VF32>::Rcp2Pi() );
    const VF32       cos45deg( Const<VF32>::Sqrt2div2() );
    const Quaternion q0( MakeUnitQuaternion( normal, zero ) );
    const Quaternion q1( MakeUnitQuaternion( normal, angle ) );
    
    // Cross products for deriving third matrix axis from 2 input axes
    // ax = ay ^ az;
    // ay = az ^ ax;
    // az = ax ^ ay;
    //const Normal3 unit_x( MakeNormal3( Const<Vec4F32>::UnitX() ) );
    const Normal3 unit_y( MakeNormal3( Const<Vec4F32>::UnitY() ) );
    const Normal3 unit_z( MakeNormal3( Const<Vec4F32>::UnitZ() ) );
    const Vector3 perp( MakeVector3( Select( Abs( normal | unit_y ) <= cos45deg,
                                             Normalise( normal ^ ( unit_y ^ normal ) ),
                                             Normalise( ( normal ^ unit_z ) ^ normal ) ) ) );
    
    const VF32 vn( Round( Abs( angle ) * rcpTwoPI * Const<VF32>::Int<64>() ) );
    const VF32 rcp_vn( Rcp( vn ) );
    VF32       t( rcp_vn );
    Vector3    last( perp );
    const S32 n( GetS32( vn ) );
    for ( S32 i=1; i<=n; i++, t += rcp_vn )
    {
        const Vector3 curr( perp * Slerp( q0, q1, t ) );
        DrawLine( point + ( last * radius ),
                  point + ( curr * radius ),
                  colour );
        last = curr;
    }
}

/// Draw arc segment with arrowhead
void DrawArcSegmentArrow( const Point3 PCREF point, const Normal3 PCREF normal, const VF32 VECREF radius, const VF32 VECREF angle, const RGBA colour )
{
    const VF32       zero( Const<VF32>::Zero() );
    //const VF32       one( Const<VF32>::One() );
    //const VF32       epsilon( Const<VF32>::Epsilon() );
    const VF32       rcpTwoPI( Const<VF32>::Rcp2Pi() );
    const VF32       cos45deg( Const<VF32>::Sqrt2div2() );
    const Quaternion q0( MakeUnitQuaternion( normal, zero ) );
    const Quaternion q1( MakeUnitQuaternion( normal, angle ) );
    
    // Cross products for deriving third matrix axis from 2 input axes
    // ax = ay ^ az;
    // ay = az ^ ax;
    // az = ax ^ ay;
    //const Normal3 unit_x( MakeNormal3( Const<Vec4F32>::UnitX() ) );
    const Normal3 unit_y( MakeNormal3( Const<Vec4F32>::UnitY() ) );
    const Normal3 unit_z( MakeNormal3( Const<Vec4F32>::UnitZ() ) );
    const Vector3 perp( MakeVector3( Select( Abs( normal | unit_y ) <= cos45deg,
                                             Normalise( normal ^ ( unit_y ^ normal ) ),
                                             Normalise( ( normal ^ unit_z ) ^ normal ) ) ) );
    
    const VF32 vn( Round( angle * rcpTwoPI * Const<VF32>::Int<64>() ) );
    const VF32 rcp_vn( Rcp( vn ) );
    VF32       t( rcp_vn );
    Vector3    last( perp );
    Vector3    arrow_vector( perp );
    const S32 n( GetS32( vn ) );
    for ( S32 i=1; i<=n; i++, t += rcp_vn )
    {
        const Vector3 curr( perp * Slerp( q0, q1, t ) );
        DrawLine( point + ( last * radius ),
                  point + ( curr * radius ),
                  colour );
        arrow_vector = curr - last;
        last = curr;
    }
    DrawLine( point, point + ( perp * radius ), colour );
    DrawLine( point, point + ( last * radius ), colour );
    if ( Abs( angle ) > Const<VF32>::Frac<1,100>() )
    {
        const VF32 oneDiv10( Const<VF32>::Frac<1,10>() );
        const VF32 oneDiv40( Const<VF32>::Frac<1,40>() );
        DrawArrowHead( point + ( last * radius ), Normalise( arrow_vector ), oneDiv10, oneDiv40, colour );
    }
}

/// Draw Line
void Draw( const Line& line, const RGBA colour )
{
    DrawLine( Start( line ), End( line ), colour );
}

/// Draw Sphere
void Draw( const Sphere PCREF sphere, const RGBA colour )
{
    Display::BasicRender()->DrawColoured3DSphere( HVec3( Centre( sphere ) ), GetF32( Radius( sphere ) ), colour );
}

/// Draw Mat33
void Draw( const Mat33 MATREF matrix, const Point3 PCREF point )
{
    const RGBA red( 0xFF, 0x00, 0x00 );
    const RGBA green( 0x00, 0xFF, 0x00 );
    const RGBA blue( 0x00, 0x00, 0xFF );
    
    DrawArrow( point, RowX( matrix ), red );
    DrawArrow( point, RowY( matrix ), green );
    DrawArrow( point, RowZ( matrix ), blue );
}

/// Draw Mat33: Variant that takes the parameter 'point' as a Vector3 instead of a Point3
void Draw( const Mat33 MATREF matrix, const Vector3 PCREF point )
{
    Draw( matrix, MakePoint3( point ) );
}

/// Draw AxisBox
void Draw( const AxisBox& box, const RGBA colour )
{
    const Vector3 boxMin( Min( box ) );
    const Vector3 boxMax( Max( box ) );
    
    const Point3 p0( MakePoint3( X( boxMin ), Y( boxMin ), Z( boxMin ) ) );
    const Point3 p1( MakePoint3( X( boxMin ), Y( boxMin ), Z( boxMax ) ) );
    const Point3 p2( MakePoint3( X( boxMin ), Y( boxMax ), Z( boxMin ) ) );
    const Point3 p3( MakePoint3( X( boxMin ), Y( boxMax ), Z( boxMax ) ) );
    const Point3 p4( MakePoint3( X( boxMax ), Y( boxMin ), Z( boxMin ) ) );
    const Point3 p5( MakePoint3( X( boxMax ), Y( boxMin ), Z( boxMax ) ) );
    const Point3 p6( MakePoint3( X( boxMax ), Y( boxMax ), Z( boxMin ) ) );
    const Point3 p7( MakePoint3( X( boxMax ), Y( boxMax ), Z( boxMax ) ) );
    
    DrawLine( p0, p1, colour );
    DrawLine( p2, p3, colour );
    DrawLine( p4, p5, colour );
    DrawLine( p6, p7, colour );
    
    DrawLine( p0, p2, colour );
    DrawLine( p1, p3, colour );
    DrawLine( p4, p6, colour );
    DrawLine( p5, p7, colour );
    
    DrawLine( p0, p4, colour );
    DrawLine( p1, p5, colour );
    DrawLine( p2, p6, colour );
    DrawLine( p3, p7, colour );
}

/// Draw OrientedBox
void Draw( const OrientedBox& obb, const RGBA colour )
{
    const AxisBox box( GetAxisBox( obb ) );
    const Vector3 boxMin( Min( box ) );
    const Vector3 boxMax( Max( box ) );
    const Mat43   matrix( Matrix( obb ) );
    
    const Point3 p0( MakePoint3( X( boxMin ), Y( boxMin ), Z( boxMin ) ) * matrix );
    const Point3 p1( MakePoint3( X( boxMin ), Y( boxMin ), Z( boxMax ) ) * matrix );
    const Point3 p2( MakePoint3( X( boxMin ), Y( boxMax ), Z( boxMin ) ) * matrix );
    const Point3 p3( MakePoint3( X( boxMin ), Y( boxMax ), Z( boxMax ) ) * matrix );
    const Point3 p4( MakePoint3( X( boxMax ), Y( boxMin ), Z( boxMin ) ) * matrix );
    const Point3 p5( MakePoint3( X( boxMax ), Y( boxMin ), Z( boxMax ) ) * matrix );
    const Point3 p6( MakePoint3( X( boxMax ), Y( boxMax ), Z( boxMin ) ) * matrix );
    const Point3 p7( MakePoint3( X( boxMax ), Y( boxMax ), Z( boxMax ) ) * matrix );
    
    Draw( matrix );
    
    DrawLine( p0, p1, colour );
    DrawLine( p2, p3, colour );
    DrawLine( p4, p5, colour );
    DrawLine( p6, p7, colour );
    
    DrawLine( p0, p2, colour );
    DrawLine( p1, p3, colour );
    DrawLine( p4, p6, colour );
    DrawLine( p5, p7, colour );
    
    DrawLine( p0, p4, colour );
    DrawLine( p1, p5, colour );
    DrawLine( p2, p6, colour );
    DrawLine( p3, p7, colour );
}

/// Draw Mat43
void Draw( const Mat43 MATREF matrix )
{
    const Point3 point( MakePoint3( RowW( matrix ) ) );
    const RGBA   red( 0xFF, 0x00, 0x00 );
    const RGBA   green( 0x00, 0xFF, 0x00 );
    const RGBA   blue( 0x00, 0x00, 0xFF );
    
    DrawArrow( point, RowX( matrix ), red );
    DrawArrow( point, RowY( matrix ), green );
    DrawArrow( point, RowZ( matrix ), blue );
}

/// Draw Quaternion
void Draw( const Quaternion PCREF quaternion, const Point3 PCREF point )
{
    const Normal3 axis( RotationAxis( quaternion ) );
    const VF32    angle( RotationAngle( quaternion ) );
    const VF32    half( Const<VF32>::Half() );
    const VF32    twoPI( Const<VF32>::TwoPi() );
    const RGBA    red( 0xFF, 0x00, 0x00 );
    const RGBA    green( 0x00, 0xFF, 0x00 );
    const RGBA    blue( 0x00, 0x00, 0xFF );
    
    DrawArrow( point, axis, half, blue );
    DrawArcSegmentArrow( point, axis, half, angle, green );
    DrawArc( point, axis, half, angle - twoPI, red );
}

/// Draw Quaternion: Variant that takes the parameter 'point' as a Vector3 instead of a Point3
void Draw( const Quaternion PCREF quaternion, const Vector3 PCREF point )
{
    Draw( quaternion, MakePoint3( point ) );
}

/// Draw QuatTrans
void Draw( const QuatTrans& qt )
{
    //Draw( Quat( qt ), Trans( qt ) );
    Draw( MakeMat43( MakeMat33( Quat( qt ) ), Trans( qt ) ) );
}

/// Draw DualQuaternion
void Draw( const DualQuaternion& dq )
{
    //Draw( Quat( dq ), Trans( dq ) );
    Draw( MakeMat43( MakeMat33( Quat( dq ) ), Trans( dq ) ) );
}

/// Draw Plane
void Draw( const Plane PCREF plane, const RGBA colour )
{
    const Point3  point( Point( plane ) );
    const Normal3 normal( Normal( plane ) );
    DrawCircle( point, normal, Const<VF32>::One(), colour );
    DrawArrow( point, normal, Const<VF32>::One(), colour );
}

/// Draw Triangle
void Draw( const Triangle PCREF triangle, const RGBA colour )
{
    const Point3  pointA( PointA( triangle ) );
    const Point3  pointB( PointB( triangle ) );
    const Point3  pointC( PointC( triangle ) );
    const Normal3 normal( SurfaceNormal( triangle ) );
    const Point3  centre( ( pointA + pointB + pointC ) * Const<VF32>::Third() );
    DrawLine( pointA, pointB, colour );
    DrawLine( pointB, pointC, colour );
    DrawLine( pointC, pointA, colour );
    DrawArrow( centre, normal, Const<VF32>::One(), colour );
}



//! @}
}; // namespace Maths


