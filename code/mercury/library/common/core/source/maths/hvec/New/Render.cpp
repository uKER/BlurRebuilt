#include "CorePch.h"
#include <core/maths/hvec/New/IncludeMinimal.h>
#include <core/maths/hvec/New/IncludeMinimal.inl>

#include <Display/Core/Includes.h>
#include <System/DebugLog.h>
#include <System/Debug/Debug.h>
#include <System/Serialise.h>
#include <Display.h>
#include <Display\Render\BasicRender.h>

namespace Maths
{
/// @addtogroup Render
//! @{

#if 0

/// Draw arrow
void DrawArrow( const Vector3 PCREF vec, const Point3 PCREF point, const RGBA colour )
{
    const VF32 arrow_head_length_ratio( Const<VF32>::Frac<1,5>() );
    const VF32 arrow_head_radius_ratio( Const<VF32>::Frac<1,20>() );
    
    const VF32 cos45deg( Const<VF32>::Sqrt2div2() );
    
    const Vector3 axis_y( Const<Vector3>::UnitY() );
    const Vector3 axis_z( Const<Vector3>::UnitZ() );
    
    const Vector3 perp_a( Select( Abs( vec | axis_y ) <= cos45deg, axis_y, axis_z ) );
    const Vector3 perp_b( perp_a ^ vec );
    const Vector3 perp_c( vec ^ perp_b );
    
    const Normal3 normal_z( Normalise( vec ) );
    const Normal3 normal_x( Normalise( perp_b ) );
    const Normal3 normal_y( Normalise( perp_c ) );
    
    const VF32 length( Length( vec ) );
    
    const Point3 end( point + vec );
    
    const F32 px( GetF32( X( point ) ) );
    const F32 py( GetF32( Y( point ) ) );
    const F32 pz( GetF32( Z( point ) ) );
    
    const F32 ex( GetF32( X( end ) ) );
    const F32 ey( GetF32( Y( end ) ) );
    const F32 ez( GetF32( Z( end ) ) );
    
    Display::BasicRender()->DrawColoured3DLine( px, py, pz, ex, ey, ez, colour );
    
    const Point3 centre( end - ( vec * arrow_head_length_ratio ) );
    
    const Point3 arrow_head0( centre + ( normal_x * length * arrow_head_radius_ratio ) );
    const Point3 arrow_head1( centre - ( normal_x * length * arrow_head_radius_ratio ) );
    const Point3 arrow_head2( centre + ( normal_y * length * arrow_head_radius_ratio ) );
    const Point3 arrow_head3( centre - ( normal_y * length * arrow_head_radius_ratio ) );
    
    Display::BasicRender()->DrawColoured3DCircle( HVec3( GetVec4F32( centre ) ),
                                                  HVec3( GetVec4F32( normal_z ) ),
                                                  GetF32( arrow_head_radius_ratio * length ),
                                                  colour );
    
    Display::BasicRender()->DrawColoured3DLine( GetF32( X( end ) ), GetF32( Y( end ) ), GetF32( Z( end ) ),
                                                GetF32( X( arrow_head0 ) ), GetF32( Y( arrow_head0 ) ), GetF32( Z( arrow_head0 ) ), colour );
    
    Display::BasicRender()->DrawColoured3DLine( GetF32( X( end ) ), GetF32( Y( end ) ), GetF32( Z( end ) ),
                                                GetF32( X( arrow_head1 ) ), GetF32( Y( arrow_head1 ) ), GetF32( Z( arrow_head1 ) ), colour );
    
    Display::BasicRender()->DrawColoured3DLine( GetF32( X( end ) ), GetF32( Y( end ) ), GetF32( Z( end ) ),
                                                GetF32( X( arrow_head2 ) ), GetF32( Y( arrow_head2 ) ), GetF32( Z( arrow_head2 ) ), colour );
    
    Display::BasicRender()->DrawColoured3DLine( GetF32( X( end ) ), GetF32( Y( end ) ), GetF32( Z( end ) ),
                                                GetF32( X( arrow_head3 ) ), GetF32( Y( arrow_head3 ) ), GetF32( Z( arrow_head3 ) ), colour );
}

/// Draw Mat33
void DrawMatrix( const Mat33 MATREF matrix, const Point3 PCREF point )
{
    const RGBA red( 0xFF, 0x00, 0x00 );
    const RGBA green( 0x00, 0xFF, 0x00 );
    const RGBA blue( 0x00, 0x00, 0xFF );
    
    DrawArrow( RowX( matrix ), point, red );
    DrawArrow( RowY( matrix ), point, green );
    DrawArrow( RowZ( matrix ), point, blue );
}

#endif

//! @}
}; // namespace Maths


