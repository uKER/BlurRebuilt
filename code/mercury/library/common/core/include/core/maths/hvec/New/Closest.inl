#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLOSEST_INL
#define SOFT_MATHS_HVEC_NEW_CLOSEST_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLOSEST_INL
#define MATHS_HVEC_NEW_CLOSEST_INL
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
NOALIAS __forceinline const Point3 ClosestPointOnPlane( const Point3 PCREF point, const Plane PCREF plane )
{
    //return point - Normal( plane ) * Distance( point, plane );
    return NegMulSub( Normal( plane ), Distance( point, plane ), point );
}


/// Find closest point on surface of sphere to point p
/// NOTE: If p is exactly at the centre of the sphere, the centre point is returned
NOALIAS __forceinline const Point3 ClosestPointOnSphere( const Point3 PCREF p, const Sphere PCREF s )
{
    const Point3  c( Centre( s ) );
    const VF32    r( Radius( s ) );
    const Vector3 cp( p - c );
    const Normal3 ncp( Normalise( cp ) );
    return MulAdd( ncp, r, c ); // c + ncp * r;
}


/// Returns t value of closest point on the infinite line through the line ab to the point p
NOALIAS __forceinline const VF32 ClosestPointOnInfiniteLineT( const Point3 PCREF p, const Point3 PCREF a, const Point3 PCREF b )
{
    // Point : p
    // Line : a + (b-a) t
    // Closest t = (p-a).(b-a) / |b-a| ^2
    //           = (p-a).(b-a) / (b-a).(b-a)
    
    const Vector3 ap( p - a );
    const Vector3 ab( b - a );
    return ( ap | ab ) * Rcp( ab | ab );
}

/// Returns t value of closest point on the infinite line through the line ab to the point p
NOALIAS __forceinline const VF32 ClosestPointOnInfiniteLineT( const Point3 PCREF p, const Line& line )
{
    // Point : p
    // Line : a + (b-a)t
    // Closest t = (p-a).(b-a) / |b-a| ^2
    //           = ( (p-a).( (b-a) / |b-a| ) ) * ( 1.0 / |b-a| )
    //           = (p-a).(b-a) / (b-a).(b-a)
    
    const Point3  a( Start( line ) );
    const Point3  b( End( line ) );
    const VF32    rcp_length_ab( RcpLength( line ) );
    const Vector3 ap( p - a );
    const Vector3 ab( b - a );
    //const Normal3 nab( Normal( line ) );
    //return ( ap | nab ) * rcp_length_ab;
    return ( ap | ab ) * rcp_length_ab * rcp_length_ab;
}


/// Returns t value of closest point on the line ab to the point p
NOALIAS __forceinline const VF32 ClosestPointOnLineSegmentT( const Point3 PCREF p, const Point3 PCREF a, const Point3 PCREF b )
{
    // Point : p
    // Line : a + (b-a) t
    // Closest t = (p-a).(b-a) / |b-a| ^2
    //           = (p-a).(b-a) / (b-a).(b-a)
    
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    //const Vector3 ap( p - a );
    //const Vector3 ab( b - a );
    //return Clamp( ( ap | ab ) * Rcp( ab | ab ), zero, one );
    return Clamp( ClosestPointOnInfiniteLineT( p, a, b ), zero, one );
}

/// Returns t value of closest point on the line to the point p
NOALIAS __forceinline const VF32 ClosestPointOnLineSegmentT( const Point3 PCREF p, const Line& line )
{
    // Point : p
    // Line : a + (b-a)t
    // Closest t = (p-a).(b-a) / |b-a| ^2
    //           = ( (p-a).( (b-a) / |b-a| ) ) * ( 1.0 / |b-a| )
    
    const VF32 zero( Const<VF32>::Zero() );
    const VF32 one( Const<VF32>::One() );
    //return Clamp( ( ( p - Start( line ) ) | Normal( line ) ) * RcpLength( line ), zero, one );
    return Clamp( ClosestPointOnInfiniteLineT( p, line ), zero, one );
}


/// Returns closest point on the infinite line through ab to the point p
NOALIAS __forceinline const Point3 ClosestPointOnInfiniteLine( const Point3 PCREF p, const Point3 PCREF a, const Point3 PCREF b )
{
    const Vector3 ab( b - a );
    const VF32    t( ClosestPointOnInfiniteLineT( p, a, b ) );
    return MulAdd( ab, t, a );
}

/// Returns closest point on the infinite line through ab to the point p
NOALIAS __forceinline const Point3 ClosestPointOnInfiniteLine( const Point3 PCREF p, const Line& line )
{
    const Point3  a( Start( line ) );
    const Vector3 ab( Vector( line ) );
    const VF32    t( ClosestPointOnInfiniteLineT( p, line ) );
    return MulAdd( ab, t, a );
}


/// Returns closest point on the line ab to the point p
NOALIAS __forceinline const Point3 ClosestPointOnLineSegment( const Point3 PCREF p, const Point3 PCREF a, const Point3 PCREF b )
{
    const Vector3 ab( b - a );
    const VF32    t( ClosestPointOnLineSegmentT( p, a, b ) );
    return MulAdd( ab, t, a );
}

/// Returns closest point on the line to the given point
NOALIAS __forceinline const Point3 ClosestPointOnLineSegment( const Point3 PCREF point, const Line& line )
{
    const Point3  a( Start( line ) );
    const Vector3 ab( Vector( line ) );
    const VF32    t( ClosestPointOnLineSegmentT( point, line ) );
    return MulAdd( ab, t, a );
}


/// Find closest point on triangle to input point
NOALIAS __forceinline const Point3 ClosestPointOnTriangle(
    const Point3 PCREF p,  // find point closest to p on triangle abc
    const Point3 PCREF a,  // triangle point a
    const Point3 PCREF b,  // triangle point b
    const Point3 PCREF c ) // triangle point c
{
    const VF32 zero( Const<VF32>::Zero() );
    
    const Vector3 ab( b - a ); // tri edge ab
    const Vector3 bc( c - b ); // tri edge bc
    const Vector3 ca( a - c ); // tri edge ca
    const Vector3 ba( -ab );   // tri edge ba
    const Vector3 cb( -bc );   // tri edge cb
    const Vector3 ac( -ca );   // tri edge ac
    const Vector3 ap( p - a ); // vector from a to p
    const Vector3 bp( p - b ); // vector from b to p
    const Vector3 cp( p - c ); // vector from c to p
    
    const VF32    ab_dot_ap( ab | ap );
    const VF32    bc_dot_bp( bc | bp );
    const VF32    ca_dot_cp( ca | cp );
    const VF32    ac_dot_ap( ac | ap );
    const VF32    cb_dot_cp( cb | cp );
    const VF32    ba_dot_bp( ba | bp );
    
    const Vector3 tv( ab ^ ac );          // vector in direction of tri normal
    const Vector3 ab_cross_ap( ab ^ ap ); // vector perp to ab & ap, dot with tri normal negative if p outside of tri in direction of edge ab
    const Vector3 bc_cross_bp( bc ^ bp ); // vector perp to bc & bp, dot with tri normal negative if p outside of tri in direction of edge bc
    const Vector3 ca_cross_cp( ca ^ cp ); // vector perp to ca & cp, dot with tri normal negative if p outside of tri in direction of edge ca
    
    const VF32    tv_dot_ab_cross_ap( tv | ab_cross_ap ); // test if p outside of tri in direction of edge ab
    const VF32    tv_dot_bc_cross_bp( tv | bc_cross_bp ); // test if p outside of tri in direction of edge bc
    const VF32    tv_dot_ca_cross_cp( tv | ca_cross_cp ); // test if p outside of tri in direction of edge ca
    const VF32    tv_dot_ap( tv | ap );                   // distance from tri plane to p * length of tv
    
    const VF32    length_squared_ab( ab | ab );
    const VF32    length_squared_bc( bc | bc );
    const VF32    length_squared_ca( ca | ca );
    const VF32    length_squared_tv( tv | tv );
    const Vector4 length_squared( MakeVector4( length_squared_ab, length_squared_bc, length_squared_ca, length_squared_tv ) );
    const Vector4 rcp_length( RcpSqrt( length_squared ) );
    const VF32    rcp_length_ab( X( rcp_length ) );
    const VF32    rcp_length_bc( Y( rcp_length ) );
    const VF32    rcp_length_ca( Z( rcp_length ) );
    const VF32    rcp_length_tv( W( rcp_length ) );
    
    const Normal3 nab( MakeNormal3( ab * rcp_length_ab ) ); // normal in direction of edge ab
    const Normal3 nbc( MakeNormal3( bc * rcp_length_bc ) ); // normal in direction of edge bc
    const Normal3 nca( MakeNormal3( ca * rcp_length_ca ) ); // normal in direction of edge ca
    const Normal3 tn( MakeNormal3( tv * rcp_length_tv ) );  // normal of tri abc
    
    const VF32    nab_dot_ap( ab_dot_ap * rcp_length_ab ); // distance along edge ab of closest point to p
    const VF32    nbc_dot_bp( bc_dot_bp * rcp_length_bc ); // distance along edge bc of closest point to p
    const VF32    nca_dot_cp( ca_dot_cp * rcp_length_ca ); // distance along edge ca of closest point to p
    const VF32    tn_dot_ap( tv_dot_ap * rcp_length_tv );  // distance from tri plane to p
    
    const Point3  closest_ab( MulAdd( nab, nab_dot_ap, a ) ); // a + nab * nab_dot_ap
    const Point3  closest_bc( MulAdd( nbc, nbc_dot_bp, b ) ); // b + nbc * nbc_dot_bp
    const Point3  closest_ca( MulAdd( nca, nca_dot_cp, c ) ); // c + nca * nca_dot_cp
    
    const Point3  closest_abc( NegMulSub( tn, tn_dot_ap, p ) ); // p - tn * tn_dot_ap
    
    const VBool   setClosestA( ( ab_dot_ap <= zero ) & ( ac_dot_ap <= zero ) );
    const VBool   setClosestB( ( ba_dot_bp <= zero ) & ( bc_dot_bp <= zero ) );
    const VBool   setClosestC( ( ca_dot_cp <= zero ) & ( cb_dot_cp <= zero ) );
    const VBool   setClosestAB( ( ( ab_dot_ap >= zero ) & ( ba_dot_bp >= zero ) ) & ( tv_dot_ab_cross_ap <= zero ) );
    const VBool   setClosestBC( ( ( bc_dot_bp >= zero ) & ( cb_dot_cp >= zero ) ) & ( tv_dot_bc_cross_bp <= zero ) );
    const VBool   setClosestCA( ( ( ca_dot_cp >= zero ) & ( ac_dot_ap >= zero ) ) & ( tv_dot_ca_cross_cp <= zero ) );
    
    Point3 closest( Const<Point3>::Zero() );
    VBool  setClosest( Const<VBool>::True() );
    
    closest = Select( setClosest & setClosestA, a, closest ); setClosest = ANDC( setClosest, setClosestA );
    closest = Select( setClosest & setClosestB, b, closest ); setClosest = ANDC( setClosest, setClosestB );
    closest = Select( setClosest & setClosestC, c, closest ); setClosest = ANDC( setClosest, setClosestC );
    
    closest = Select( setClosest & setClosestAB, closest_ab, closest ); setClosest = ANDC( setClosest, setClosestAB );
    closest = Select( setClosest & setClosestBC, closest_bc, closest ); setClosest = ANDC( setClosest, setClosestBC );
    closest = Select( setClosest & setClosestCA, closest_ca, closest ); setClosest = ANDC( setClosest, setClosestCA );
    
    closest = Select( setClosest, closest_abc, closest );
    
    return closest;
}

/// Find closest point on triangle to input point ( optimised for p being very close to triangle )
/// NOTE: This version uses a suggested 'optimisation': in all test cases it's been shown to be slower than the standard version!
DEPRECATED NOALIAS __forceinline const Point3 ClosestPointOnTriangleV2(
    const Point3 PCREF p,  // find point closest to p on triangle abc
    const Point3 PCREF a,  // triangle point a
    const Point3 PCREF b,  // triangle point b
    const Point3 PCREF c ) // triangle point c
{
    const VF32 zero( Const<VF32>::Zero() );
    
    const Vector3 ab( b - a ); // tri edge ab
    const Vector3 bc( c - b ); // tri edge bc
    const Vector3 ca( a - c ); // tri edge ca
    const Vector3 ap( p - a ); // vector from a to p
    const Vector3 bp( p - b ); // vector from b to p
    const Vector3 cp( p - c ); // vector from c to p
    const Vector3 ba( -ab );   // tri edge ba
    const Vector3 cb( -bc );   // tri edge cb
    const Vector3 ac( -ca );   // tri edge ac
    
    const Vector3 tv( ab ^ ac );          // tri vector in direction of tri normal
    const Vector3 ab_cross_ap( ab ^ ap ); // vector perp to ab & ap, dot with tri normal negative if p outside of tri in direction of edge ab
    const Vector3 bc_cross_bp( bc ^ bp ); // vector perp to bc & bp, dot with tri normal negative if p outside of tri in direction of edge bc
    const Vector3 ca_cross_cp( ca ^ cp ); // vector perp to ca & cp, dot with tri normal negative if p outside of tri in direction of edge ca
    
    const VF32    rcp_length_tv( RcpLength( tv ) ); // reciprocal length of tv
    
    const VF32    tv_dot_ab_cross_ap( tv | ab_cross_ap ); // test if p inside tri edge ab
    const VF32    tv_dot_bc_cross_bp( tv | bc_cross_bp ); // test if p inside tri edge bc
    const VF32    tv_dot_ca_cross_cp( tv | ca_cross_cp ); // test if p inside tri edge ca
    const VF32    tv_dot_ap( tv | ap );                   // distance from tri plane to p * length of tv
    
    const VF32    tn_dot_ap( tv_dot_ap * rcp_length_tv );       // distance from tri plane to p
    const Normal3 tn( MakeNormal3( tv * rcp_length_tv ) );      // tri normal of tri abc
    const Point3  closest_abc( NegMulSub( tn, tn_dot_ap, p ) ); // p - tn * tn_dot_ap
    
    if ( ( tv_dot_ab_cross_ap >= zero ) &
         ( tv_dot_bc_cross_bp >= zero ) &
         ( tv_dot_ca_cross_cp >= zero ) )
    {
        return closest_abc;
    }
    
    const VF32    ab_dot_ap( ab | ap ); // for testing if p outside of tri in direction of point a
    const VF32    ac_dot_ap( ac | ap ); // for testing if p outside of tri in direction of point a
    const VF32    ba_dot_bp( ba | bp ); // for testing if p outside of tri in direction of point b
    const VF32    bc_dot_bp( bc | bp ); // for testing if p outside of tri in direction of point b
    const VF32    ca_dot_cp( ca | cp ); // for testing if p outside of tri in direction of point c
    const VF32    cb_dot_cp( cb | cp ); // for testing if p outside of tri in direction of point c
    
    const VF32    length_squared_ab( ab | ab );
    const VF32    length_squared_bc( bc | bc );
    const VF32    length_squared_ca( ca | ca );
    const Vector3 length_squared( MakeVector3( length_squared_ab, length_squared_bc, length_squared_ca ) );
    const Vector3 rcp_length( RcpSqrt( length_squared ) );
    const VF32    rcp_length_ab( X( rcp_length ) );
    const VF32    rcp_length_bc( Y( rcp_length ) );
    const VF32    rcp_length_ca( Z( rcp_length ) );
    
    const Normal3 nab( MakeNormal3( ab * rcp_length_ab ) ); // normal in direction of edge ab
    const Normal3 nbc( MakeNormal3( bc * rcp_length_bc ) ); // normal in direction of edge bc
    const Normal3 nca( MakeNormal3( ca * rcp_length_ca ) ); // normal in direction of edge ca
    
    const VF32    nab_dot_ap( ab_dot_ap * rcp_length_ab ); // distance along edge ab of closest point to p
    const VF32    nbc_dot_bp( bc_dot_bp * rcp_length_bc ); // distance along edge bc of closest point to p
    const VF32    nca_dot_cp( ca_dot_cp * rcp_length_ca ); // distance along edge ca of closest point to p
    
    const Point3  closest_ab( MulAdd( nab, nab_dot_ap, a ) ); // a + nab * nab_dot_ap
    const Point3  closest_bc( MulAdd( nbc, nbc_dot_bp, b ) ); // b + nbc * nbc_dot_bp
    const Point3  closest_ca( MulAdd( nca, nca_dot_cp, c ) ); // c + nca * nca_dot_cp
    
    const VBool   setClosestA( ( ab_dot_ap <= zero ) & ( ac_dot_ap <= zero ) );
    const VBool   setClosestB( ( ba_dot_bp <= zero ) & ( bc_dot_bp <= zero ) );
    const VBool   setClosestC( ( ca_dot_cp <= zero ) & ( cb_dot_cp <= zero ) );
    const VBool   setClosestAB( tv_dot_ab_cross_ap <= zero );
    const VBool   setClosestBC( tv_dot_bc_cross_bp <= zero );
    //const VBool   setClosestCA( tv_dot_ca_cross_cp <= zero );
    
    Point3 closest( Const<Point3>::Zero() );
    VBool  setClosest( Const<VBool>::True() );
    
    closest = Select( setClosest & setClosestA, a, closest ); setClosest = ANDC( setClosest, setClosestA );
    closest = Select( setClosest & setClosestB, b, closest ); setClosest = ANDC( setClosest, setClosestB );
    closest = Select( setClosest & setClosestC, c, closest ); setClosest = ANDC( setClosest, setClosestC );
    
    closest = Select( setClosest & setClosestAB, closest_ab, closest ); setClosest = ANDC( setClosest, setClosestAB );
    closest = Select( setClosest & setClosestBC, closest_bc, closest ); setClosest = ANDC( setClosest, setClosestBC );
    //closest = Select( setClosest & setClosestCA, closest_ca, closest ); setClosest = ANDC( setClosest, setClosestCA );
    closest = Select( setClosest, closest_ca, closest );
    
    return closest;
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

