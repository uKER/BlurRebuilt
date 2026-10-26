#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_INTERSECT_INL
#define SOFT_MATHS_HVEC_NEW_INTERSECT_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_INTERSECT_INL
#define MATHS_HVEC_NEW_INTERSECT_INL
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
NOALIAS __forceinline const VBool Intersect( const Plane PCREF plane, const Point3 PCREF p1, const Point3 PCREF p2, Point3* const __restrict pIntersectPoint )
{
    const Vector3 p1p2( p2 - p1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    nearlyZero( Const<VF32>::Epsilon() );
    const VF32    d1( Distance( plane, p1 ) );
    const VF32    d2( Distance( plane, p2 ) );
    const VF32    d1_times_d2( d1 * d2 );
    const VF32    d1_minus_d2( d1 - d2 );
    const VF32    abs_d1_minus_d2( Abs( d1_minus_d2 ) );
    const VF32    rcp_d1_minus_d2( Rcp( d1_minus_d2 ) );
    const VF32    t( d1 * rcp_d1_minus_d2 ); // ( -d1 / ( d2 - d1 ) );
    *pIntersectPoint = MulAdd( p1p2, t, p1 );
    return ( d1_times_d2 < zero ) & 
           ( abs_d1_minus_d2 >= nearlyZero );
}

/// Line-Plane Intersection
NOALIAS __forceinline const VBool Intersect( const Point3 PCREF p1, const Point3 PCREF p2, const Plane PCREF plane, Point3* const __restrict pIntersectPoint )
{
    return Intersect( plane, p1, p2, pIntersectPoint );
}


/// Plane-Line Intersection
NOALIAS __forceinline const VBool Intersect( const Plane PCREF plane, const Point3 PCREF p1, const Point3 PCREF p2, Point3* const __restrict pIntersectPoint, VF32* const __restrict pIntersectT )
{
    const Vector3 p1p2( p2 - p1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    nearlyZero( Const<VF32>::Epsilon() );
    const VF32    d1( Distance( plane, p1 ) );
    const VF32    d2( Distance( plane, p2 ) );
    const VF32    d1_times_d2( d1 * d2 );
    const VF32    d1_minus_d2( d1 - d2 );
    const VF32    abs_d1_minus_d2( Abs( d1_minus_d2 ) );
    const VF32    rcp_d1_minus_d2( Rcp( d1_minus_d2 ) );
    const VF32    t( d1 * rcp_d1_minus_d2 ); // ( -d1 / ( d2 - d1 ) );
    *pIntersectPoint = MulAdd( p1p2, t, p1 );
    *pIntersectT = t;
    return ( d1_times_d2 < zero ) & 
           ( abs_d1_minus_d2 >= nearlyZero );
}

/// Line-Plane Intersection
NOALIAS __forceinline const VBool Intersect( const Point3 PCREF p1, const Point3 PCREF p2, const Plane PCREF plane, Point3* const __restrict pIntersectPoint, VF32* const __restrict pIntersectT )
{
    return Intersect( plane, p1, p2, pIntersectPoint, pIntersectT );
}


/// Plane-Line Intersection
NOALIAS __forceinline const VBool Intersect( const Plane PCREF plane, const Line& line, Point3* const __restrict pIntersectPoint )
{
    return Intersect( plane, Start( line ), End( line ), pIntersectPoint );
}

/// Line-Plane Intersection
NOALIAS __forceinline const VBool Intersect( const Line& line, const Plane PCREF plane, Point3* const __restrict pIntersectPoint )
{
    return Intersect( plane, Start( line ), End( line ), pIntersectPoint );
}


/// Plane-Line Intersection
NOALIAS __forceinline const VBool Intersect( const Plane PCREF plane, const Line& line, Point3* const __restrict pIntersectPoint, VF32* const __restrict pIntersectT )
{
    return Intersect( plane, Start( line ), End( line ), pIntersectPoint, pIntersectT );
}

/// Line-Plane Intersection
NOALIAS __forceinline const VBool Intersect( const Line& line, const Plane PCREF plane, Point3* const __restrict pIntersectPoint, VF32* const __restrict pIntersectT )
{
    return Intersect( plane, Start( line ), End( line ), pIntersectPoint, pIntersectT );
}


/// Sphere-Line Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
/// pResult will be filled in with the two t-values for the points on the line that intersect the sphere
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere, const Point3& lineStart, const Point3& lineEnd, QuadraticResult* const __restrict pResult )
{
    // Solve Sphere-Line intersection using the Quadratic Formula
    
    const Point3  p( Centre( sphere ) );
    const VF32    r( Radius( sphere ) );
    const VF32    r2( r * r );
    const Point3  q( lineStart );
    const Vector3 v( lineEnd - lineStart );
    const Vector3 pq( q - p );
    const VF32    v_dot_pq( v | pq );
    const VF32    a( v | v );
    const VF32    b( v_dot_pq + v_dot_pq );
    const VF32    c( ( pq | pq ) - r2 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const QuadraticResult result( SolveQuadratic( a, b, c ) );
    *pResult = result;
    return ( ( result.m_X1 >= zero ) & ( result.m_X1 <= one ) ) |
           ( ( result.m_X2 >= zero ) & ( result.m_X2 <= one ) );
}

/// Line-Sphere Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
/// pResult will be filled in with the two t-values for the points on the line that intersect the sphere
NOALIAS __forceinline const VBool Intersect( const Point3& lineStart, const Point3& lineEnd, const Sphere PCREF sphere, QuadraticResult* const __restrict pResult )
{
    return Intersect( sphere, lineStart, lineEnd, pResult );
}


/// Sphere-Line Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
/// pResult will be filled in with the two t-values for the points on the line that intersect the sphere
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere, const Line& line, QuadraticResult* const __restrict pResult )
{
    // Solve Sphere-Line intersection using the Quadratic Formula
    
    const Point3  lineStart( Start( line ) );
    const Point3  lineEnd( End( line ) );
    const Point3  p( Centre( sphere ) );
    const VF32    r( Radius( sphere ) );
    const VF32    r2( r * r );
    const Point3  q( lineStart );
    const Vector3 v( lineEnd - lineStart );
    const Vector3 pq( q - p );
    const VF32    v_dot_pq( v | pq );
    const VF32    a( v | v );
    const VF32    b( v_dot_pq + v_dot_pq );
    const VF32    c( ( pq | pq ) - r2 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const QuadraticResult result( SolveQuadratic( a, b, c ) );
    *pResult = result;
    return ( ( result.m_X1 >= zero ) & ( result.m_X1 <= one ) ) |
           ( ( result.m_X2 >= zero ) & ( result.m_X2 <= one ) );
}

/// Line-Sphere Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
/// pResult will be filled in with the two t-values for the points on the line that intersect the sphere
NOALIAS __forceinline const VBool Intersect( const Line& line, const Sphere PCREF sphere, QuadraticResult* const __restrict pResult )
{
    return Intersect( sphere, line, pResult );
}


/// Sphere-Line Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere, const Point3& lineStart, const Point3& lineEnd )
{
    QuadraticResult result;
    return Intersect( sphere, lineStart, lineEnd, &result );
}

/// Line-Sphere Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
NOALIAS __forceinline const VBool Intersect( const Point3& lineStart, const Point3& lineEnd, const Sphere PCREF sphere )
{
    QuadraticResult result;
    return Intersect( sphere, lineStart, lineEnd, &result );
}


/// Sphere-Line Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere, const Line& line )
{
    QuadraticResult result;
    return Intersect( sphere, line, &result );
}

/// Line-Sphere Intersection: returns true if one or both of the 2 resulting line t-values is in the range [ 0.0f, 1.0f ]
NOALIAS __forceinline const VBool Intersect( const Line& line, const Sphere PCREF sphere )
{
    QuadraticResult result;
    return Intersect( sphere, line, &result );
}


/// Sphere-Sphere Intersection
NOALIAS __forceinline const VBool Intersect( const Sphere PCREF sphere1, const Sphere PCREF sphere2 )
{
    const VF32 d2( DistanceSquared( Centre( sphere1 ), Centre( sphere2 ) ) );
    const VF32 rSum( Radius( sphere1 ) + Radius( sphere2 ) );
    return d2 < ( rSum * rSum );
}


/// AxisBox-AxisBox Intersection
NOALIAS __forceinline const VBool Intersect( const AxisBox& box1, const AxisBox& box2 )
{
    return AllTrue3( ( box1.m_Min <= box2.m_Max ) &
                     ( box1.m_Max >= box2.m_Min ) );
}

/// AxisBox-AxisBox Intersection
NOALIAS __forceinline const VBool Intersect( const AxisBox& box1, const AxisBox& box2, AxisBox* const __restrict pResult )
{
    const Vector3 resultMin( Max( Min( box1 ), Min( box2 ) ) );
    const Vector3 resultMax( Min( Max( box1 ), Max( box2 ) ) );
    *pResult = MakeAxisBox( resultMin, resultMax );
    return AllTrue3( ( box1.m_Min <= box2.m_Max ) &
                     ( box1.m_Max >= box2.m_Min ) );
}


/// Line-Line Intersection: Returns true if the lines are coplanar & intersect with the t-values in the range [ 0.0f, 1.0f ]
/// NOTE: The lines must be coplanar. [ This can be tested with the line functions CoPlanar() & Skew() ]
/// The coplanar tolerance is the maximum distance between the closest points on the lines
NOALIAS __forceinline const VBool Intersect(
    const Line&             line1,
    const Line&             line2,
    VF32* const __restrict  pT1,
    VF32* const __restrict  pT2,
    const VF32 VECREF       coplanarTolerance )
{
    // http://mathworld.wolfram.com/Line-LineIntersection.html
    // http://mathworld.wolfram.com/Coplanar.html
    // Line 1 : x = x1 + ( x2 - x1 ) s
    // Line 2 : x = x3 + ( x4 - x3 ) t
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // s = ( ( c x b ) . ( a x b ) ) / ( | a x b | ^2 )
    //   = ( ( c x b ) . ( a x b ) ) / ( ( a x b ) . ( a x b ) )
    // t = ( ( -c x a ) . ( b x a ) ) / ( | a x b | ^2 )
    //   = ( -( c x a ) . -( a x b ) ) / ( | a x b | ^2 )
    //   = ( ( c x a ) . ( a x b ) ) / ( ( a x b ) . ( a x b ) )
    // coplanar: (x3-x1).((x2-x1)^(x4-x3)) = 0
    
    const Vector3 a( Vector( line1 ) );
    const Vector3 b( Vector( line2 ) );
    const Normal3 na( Normal( line1 ) );
    const Normal3 nb( Normal( line2 ) );
    const Vector3 c( Start( line2 ) - Start( line1 ) );
    const Vector3 cxa( c ^ a );
    const Vector3 cxb( c ^ b );
    const Vector3 axb( a ^ b );
    const VF32    denom( axb | axb );
    const VF32    rcpDenom( Rcp( denom ) );
    const VF32    s( ( cxb | axb ) * rcpDenom );
    const VF32    t( ( cxa | axb ) * rcpDenom );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    //const VF32    epsilon( Const<VF32>::Epsilon() );
    const VBool   coplanar( Abs( c | Normalise( axb ) ) <= coplanarTolerance );
    //const VBool   notParallel( Abs( denom ) > epsilon );
    const VF32    parallelTolerance( Const<VF32>::Frac<9999,10000>() );
    const VBool   notParallel( Abs( na | nb ) <= parallelTolerance );
    *pT1 = s;
    *pT2 = t;
    return ( coplanar & notParallel ) &
           ( ( ( s >= zero ) & ( s <= one ) ) &
             ( ( t >= zero ) & ( t <= one ) ) );
}

/// Line-Line Intersection: Returns true if the lines are coplanar & intersect with the t-values in the range [ 0.0f, 1.0f ]
/// NOTE: The lines must be coplanar. [ This can be tested with the line functions CoPlanar() & Skew() ]
/// The coplanar tolerance is the maximum distance between the closest points on the lines
NOALIAS __forceinline const VBool Intersect(
    const Line&      line1,
    const Line&      line2,
    const VF32 VECREF coplanarTolerance )
{
    VF32 t1, t2;
    return Intersect( line1, line2, &t1, &t2, coplanarTolerance );
}


/// Line-Line Intersection: Returns true if the lines are coplanar & intersect with any t-values
/// NOTE: The lines must be coplanar. [ This can be tested with the line functions CoPlanar() & Skew() ]
/// The coplanar tolerance is the maximum distance between the closest points on the lines
NOALIAS __forceinline const VBool IntersectInfiniteLines(
    const Line&             line1,
    const Line&             line2,
    VF32* const __restrict  pT1,
    VF32* const __restrict  pT2,
    const VF32 VECREF       coplanarTolerance )
{
    // http://mathworld.wolfram.com/Line-LineIntersection.html
    // http://mathworld.wolfram.com/Coplanar.html
    // Line 1 : x = x1 + ( x2 - x1 ) s
    // Line 2 : x = x3 + ( x4 - x3 ) t
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // s = ( ( c x b ) . ( a x b ) ) / ( | a x b | ^2 )
    //   = ( ( c x b ) . ( a x b ) ) / ( ( a x b ) . ( a x b ) )
    // t = ( ( -c x a ) . ( b x a ) ) / ( | a x b | ^2 )
    //   = ( -( c x a ) . -( a x b ) ) / ( | a x b | ^2 )
    //   = ( ( c x a ) . ( a x b ) ) / ( ( a x b ) . ( a x b ) )
    // coplanar: (x3-x1).((x2-x1)^(x4-x3)) = 0
    
    const Vector3 a( Vector( line1 ) );
    const Vector3 b( Vector( line2 ) );
    const Normal3 na( Normal( line1 ) );
    const Normal3 nb( Normal( line2 ) );
    const Vector3 c( Start( line2 ) - Start( line1 ) );
    const Vector3 cxa( c ^ a );
    const Vector3 cxb( c ^ b );
    const Vector3 axb( a ^ b );
    const VF32    denom( axb | axb );
    const VF32    rcpDenom( Rcp( denom ) );
    const VF32    s( ( cxb | axb ) * rcpDenom );
    const VF32    t( ( cxa | axb ) * rcpDenom );
    //const VF32    nearlyZero( Const<VF32>::Epsilon() );
    const VBool   coplanar( Abs( c | Normalise( axb ) ) <= coplanarTolerance );
    const VF32    parallelTolerance( Const<VF32>::Frac<9999,10000>() );
    const VBool   notParallel( Abs( na | nb ) <= parallelTolerance );
    *pT1 = s;
    *pT2 = t;
    return ( coplanar & notParallel );
}

/// Line-Line Intersection: Returns true if the lines are coplanar & intersect with any t-values
/// NOTE: The lines must be coplanar. [ This can be tested with the line functions CoPlanar() & Skew() ]
/// The coplanar tolerance is the maximum distance between the closest points on the lines
NOALIAS __forceinline const VBool IntersectInfiniteLines24(
    const Line&             line1,
    const Line&             line2,
    VF32* const __restrict  pT1,
    VF32* const __restrict  pT2,
    const VF32 VECREF       coplanarTolerance )
{
    // http://mathworld.wolfram.com/Line-LineIntersection.html
    // http://mathworld.wolfram.com/Coplanar.html
    // Line 1 : x = x1 + ( x2 - x1 ) s
    // Line 2 : x = x3 + ( x4 - x3 ) t
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // s = ( ( c x b ) . ( a x b ) ) / ( | a x b | ^2 )
    //   = ( ( c x b ) . ( a x b ) ) / ( ( a x b ) . ( a x b ) )
    // t = ( ( -c x a ) . ( b x a ) ) / ( | a x b | ^2 )
    //   = ( -( c x a ) . -( a x b ) ) / ( | a x b | ^2 )
    //   = ( ( c x a ) . ( a x b ) ) / ( ( a x b ) . ( a x b ) )
    // coplanar: (x3-x1).((x2-x1)^(x4-x3)) = 0
    
    const Vector3 a( Vector( line1 ) );
    const Vector3 b( Vector( line2 ) );
    const Normal3 na( Normal( line1 ) );
    const Normal3 nb( Normal( line2 ) );
    const Vector3 c( Start( line2 ) - Start( line1 ) );
    const Vector3 cxa( c ^ a );
    const Vector3 cxb( c ^ b );
    const Vector3 axb( a ^ b );
    const VF32    denom( axb | axb );
    const VF32    rcpDenom( Rcp24( denom ) );
    const VF32    s( ( cxb | axb ) * rcpDenom );
    const VF32    t( ( cxa | axb ) * rcpDenom );
    const VBool   coplanar( Abs( c | Normalise24( axb ) ) <= coplanarTolerance );
    const VF32    parallelTolerance( Const<VF32>::Frac<9999,10000>() );
    const VBool   notParallel( Abs( na | nb ) <= parallelTolerance );
    *pT1 = s;
    *pT2 = t;
    return ( coplanar & notParallel );
    
    #if 0
    // PARALLEL
    const Normal3 na( Normal( line1 ) );
    const Normal3 nb( Normal( line2 ) );
    return Abs( na | nb ) >= tolerance;
    
    // COPLANAR
    // http://mathworld.wolfram.com/Line-LineIntersection.html
    // http://mathworld.wolfram.com/Coplanar.html
    // http://mathworld.wolfram.com/SkewLines.html
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // coplanar: (x3-x1).((x2-x1)^(x4-x3)) = 0
    
    const Vector3 a( Vector( line1 ) );
    const Vector3 b( Vector( line2 ) );
    const Vector3 c( Start( line2 ) - Start( line1 ) );
    return Abs( c | Normalise( a ^ b ) ) <= tolerance;
    #endif
}

/// Plane-Plane Intersection : Returns true if the planes are not parallel. If true then pLine is the line of intersection between the planes.
NOALIAS __forceinline const VBool Intersect(
    const Plane PCREF plane1,
    const Plane PCREF plane2,
    Line * const __restrict pLine )
{
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilonSquared( Const<VF32>::EpsilonSquared() );
    const Normal3 N1( Normal( plane1 ) );
    const Normal3 N2( Normal( plane2 ) );
    const Vector3 V3( N1 ^ N2 );
    const Normal3 N3( Normalise( V3 ) );
    const VF32    d1( DistanceToOrigin( plane1 ) );
    const VF32    d2( DistanceToOrigin( plane2 ) );
    const VF32    d3( zero );
    
    // Intersection point of the three planes, P:
    // N1 | P = -d1 : ( N1.x * P.x ) + ( N1.y * P.y ) + ( N1.z * P.z ) = -d1
    // N2 | P = -d2 : ( N2.x * P.x ) + ( N2.y * P.y ) + ( N2.z * P.z ) = -d2
    // N3 | P = -d3 : ( N3.x * P.x ) + ( N3.y * P.y ) + ( N3.z * P.z ) = -d3
    
    const Mat33   abc( MakeMat33( N1, N2, N3 ) );
    const Vector3 d( -MakeVector3( d1, d2, d3 ) );
    
    Vector3 P;
    const VBool valid_P( CramersRule( abc, d, &P ) );
    
    *pLine = MakeLine( MakePoint3( P ), N3, one, one );
    
    return valid_P & ( LengthSquared( V3 ) > epsilonSquared );
}


/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect(
    const Point3 PCREF lineStart,
    const Point3 PCREF lineEnd,
    const Point3 PCREF pointA,
    const Point3 PCREF pointB,
    const Point3 PCREF pointC )
{
    // Ray: R(t) = O + t.D
    //      O = lineStart
    //      D = ( lineEnd - lineStart ) / | lineEnd - lineStart |
    //      t = distance along line
    // Tri: T(u,v) = (1 - u - v).V0 + u.V1 + v.V2, u >= 0, v >= 0, u + v <= 1
    //      V0 = pointA
    //      V1 = pointB
    //      V2 = pointC
    //
    // Intersection: O + t.D = (1-u-v).V0 + u.V1 + v.V2
    //
    //                          [ t ]
    // [ -D, V1 - V0, V2 - V0 ] [ u ] = O - V0
    //                          [ v ]
    //
    // E1 = V1 - V0
    // E2 = V2 - V0
    // T  = O  - V0
    //
    // Using Cramer's rule:
    //
    // [ t ]          1       [ | T,  E1, E2 | ]
    // [ u ] = -------------- [ | -D, T,  E2 | ]
    // [ v ]   | -D, E1, E2 | [ | -D, E1, T  | ]
    //
    // | A, B, C | = -(A x C) . B = -(C x B) . A
    //
    // [ t ]          1       [ (T x E1) . E2 ]     1    [ Q . E2 ]
    // [ u ] = -------------- [ (D x E2) . T  ] = ------ [ P . T  ]
    // [ v ]   (D x E2) . E1  [ (T x E1) . D  ]   P . E1 [ Q . D  ]
    //
    // where:
    // P = D x E2
    // Q = T x E1
    //
    
    const Vector3 lineVector( lineEnd - lineStart );
    const VF32    rcpLineLength( RcpSqrt( LengthSquared( lineVector ) ) );
    const Normal3 D( MakeNormal3( lineVector * rcpLineLength ) );
    const Vector3 E1( pointB - pointA );
    const Vector3 E2( pointC - pointA );
    const Vector3 T( lineStart - pointA );
    const Vector3 P( D ^ E2 );
    const Vector3 Q( T ^ E1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    det( P | E1 );
    const VF32    absDet( Abs( det ) );
    const VF32    rcpDet( Rcp( det ) );
    const VF32    t( rcpDet * ( Q | E2 ) );
    const VF32    u( rcpDet * ( P | T ) );
    const VF32    v( rcpDet * ( Q | D ) );
    const VF32    one_minus_w( u + v );
    const VF32    normalised_t( t * rcpLineLength );
    
    return ( ( zero <= normalised_t ) & ( normalised_t <= one ) ) &
           ( ( ( absDet > epsilon ) & ( one_minus_w <= one ) ) &
             ( ( u >= zero ) & ( v >= zero ) ) );
}

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW(
    const Point3 PCREF lineStart,
    const Point3 PCREF lineEnd,
    const Point3 PCREF pointA,
    const Point3 PCREF pointB,
    const Point3 PCREF pointC )
{
    const Vector3 lineVector( lineEnd - lineStart );
    const VF32    rcpLineLength( RcpSqrt( LengthSquared( lineVector ) ) );
    const Normal3 D( MakeNormal3( lineVector * rcpLineLength ) );
    const Vector3 E1( pointB - pointA );
    const Vector3 E2( pointC - pointA );
    const Vector3 triNormalVec( E1 ^ E2 );
    const Vector3 T( lineStart - pointA );
    const Vector3 P( D ^ E2 );
    const Vector3 Q( T ^ E1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    det( E1 | P );
    const VF32    rcpDet( Rcp( det ) );
    const VF32    u( rcpDet * ( T | P ) );
    const VF32    v( rcpDet * ( D | Q ) );
    const VF32    hitDistance( rcpDet * ( E2 | Q ) );
    const VF32    hitT( hitDistance * rcpLineLength );
    
    return ( ( ( zero <= hitT ) & ( hitT <= one ) ) &
             ( ( triNormalVec | D ) < zero ) ) &
           ( ( ( Abs( det ) > epsilon ) & ( ( u + v ) <= one ) ) &
             ( ( u >= zero ) & ( v >= zero ) ) );
}

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect(
    const Point3 PCREF         lineStart,
    const Point3 PCREF         lineEnd,
    const Point3 PCREF         pointA,
    const Point3 PCREF         pointB,
    const Point3 PCREF         pointC,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV )
{
    const Vector3 lineVector( lineEnd - lineStart );
    const VF32    rcpLineLength( RcpSqrt( LengthSquared( lineVector ) ) );
    const Normal3 D( MakeNormal3( lineVector * rcpLineLength ) );
    const Vector3 E1( pointB - pointA );
    const Vector3 E2( pointC - pointA );
    const Vector3 T( lineStart - pointA );
    const Vector3 P( D ^ E2 );
    const Vector3 Q( T ^ E1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    det( E1 | P );
    const VF32    rcpDet( Rcp( det ) );
    const VF32    u( rcpDet * ( T | P ) );
    const VF32    v( rcpDet * ( D | Q ) );
    const VF32    hitDistance( rcpDet * ( E2 | Q ) );
    const VF32    hitT( hitDistance * rcpLineLength );
    const Point3  hitPoint( MulAdd( lineVector, hitT, lineStart ) );
    
    *pHitPoint = hitPoint;
    *pHitLineT = hitT;
    *pHitUV    = MakeVector2( u, v );
    
    return ( ( zero <= hitT ) & ( hitT <= one ) ) &
           ( ( ( Abs( det ) > epsilon ) & ( ( u + v ) <= one ) ) &
             ( ( u >= zero ) & ( v >= zero ) ) );
}

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW(
    const Point3 PCREF         lineStart,
    const Point3 PCREF         lineEnd,
    const Point3 PCREF         pointA,
    const Point3 PCREF         pointB,
    const Point3 PCREF         pointC,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV )
{
    const Vector3 lineVector( lineEnd - lineStart );
    const VF32    rcpLineLength( RcpSqrt( LengthSquared( lineVector ) ) );
    const Normal3 D( MakeNormal3( lineVector * rcpLineLength ) );
    const Vector3 E1( pointB - pointA );
    const Vector3 E2( pointC - pointA );
    const Vector3 triNormalVec( E1 ^ E2 );
    const Vector3 T( lineStart - pointA );
    const Vector3 P( D ^ E2 );
    const Vector3 Q( T ^ E1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    det( E1 | P );
    const VF32    rcpDet( Rcp( det ) );
    const VF32    u( rcpDet * ( T | P ) );
    const VF32    v( rcpDet * ( D | Q ) );
    const VF32    hitDistance( rcpDet * ( E2 | Q ) );
    const VF32    hitT( hitDistance * rcpLineLength );
    const Point3  hitPoint( MulAdd( lineVector, hitT, lineStart ) );
    
    *pHitPoint = hitPoint;
    *pHitLineT = hitT;
    *pHitUV    = MakeVector2( u, v );
    
    return ( ( ( zero <= hitT ) & ( hitT <= one ) ) &
             ( ( triNormalVec | D ) < zero ) ) &
           ( ( ( Abs( det ) > epsilon ) & ( ( u + v ) <= one ) ) &
             ( ( u >= zero ) & ( v >= zero ) ) );
}

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect( const Line& line, const Triangle& triangle )
{
    const Point3  lineStart( Start( line ) );
    //const Vector3 lineVector( Vector( line ) );
    const Normal3 D( Normal( line ) );
    const VF32    rcpLineLength( RcpLength( line ) );
    const Point3  pointA( PointA( triangle ) );
    const Vector3 E1( EdgeVectorAB( triangle ) );
    const Vector3 E2( EdgeVectorAC( triangle ) );
    const Vector3 T( lineStart - pointA );
    const Vector3 P( D ^ E2 );
    const Vector3 Q( T ^ E1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    det( E1 | P );
    const VF32    rcpDet( Rcp( det ) );
    const VF32    u( rcpDet * ( T | P ) );
    const VF32    v( rcpDet * ( D | Q ) );
    const VF32    hitDistance( rcpDet * ( E2 | Q ) );
    const VF32    hitT( hitDistance * rcpLineLength );
    
    return ( ( zero <= hitT ) & ( hitT <= one ) ) &
           ( ( ( Abs( det ) > epsilon ) & ( ( u + v ) <= one ) ) &
             ( ( u >= zero ) & ( v >= zero ) ) );
}

/// Triangle-Line Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect( const Triangle& triangle, const Line& line )
{
    return Intersect( line, triangle );
}

/// Line-Triangle Intersection with Back Face Cull: returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW( const Line& line, const Triangle& triangle )
{
    const Point3  lineStart( Start( line ) );
    //const Vector3 lineVector( Vector( line ) );
    const Normal3 D( Normal( line ) );
    const VF32    rcpLineLength( RcpLength( line ) );
    const Point3  pointA( PointA( triangle ) );
    const Vector3 E1( EdgeVectorAB( triangle ) );
    const Vector3 E2( EdgeVectorAC( triangle ) );
    const Vector3 triNormalVec( E1 ^ E2 );
    const Vector3 T( lineStart - pointA );
    const Vector3 P( D ^ E2 );
    const Vector3 Q( T ^ E1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    det( E1 | P );
    const VF32    rcpDet( Rcp( det ) );
    const VF32    u( rcpDet * ( T | P ) );
    const VF32    v( rcpDet * ( D | Q ) );
    const VF32    hitDistance( rcpDet * ( E2 | Q ) );
    const VF32    hitT( hitDistance * rcpLineLength );
    
    return ( ( ( zero <= hitT ) & ( hitT <= one ) ) &
             ( ( triNormalVec | D ) < zero ) ) &
           ( ( ( Abs( det ) > epsilon ) & ( ( u + v ) <= one ) ) &
             ( ( u >= zero ) & ( v >= zero ) ) );
}

/// Triangle-Line Intersection with Back Face Cull: returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW( const Triangle& triangle, const Line& line )
{
    return IntersectCullCCW( line, triangle );
}

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect(
    const Line&                line,
    const Triangle&            triangle,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV )
{
    const Point3  lineStart( Start( line ) );
    const Vector3 lineVector( Vector( line ) );
    const Normal3 D( Normal( line ) );
    const VF32    rcpLineLength( RcpLength( line ) );
    const Point3  pointA( PointA( triangle ) );
    const Vector3 E1( EdgeVectorAB( triangle ) );
    const Vector3 E2( EdgeVectorAC( triangle ) );
    const Vector3 T( lineStart - pointA );
    const Vector3 P( D ^ E2 );
    const Vector3 Q( T ^ E1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    det( E1 | P );
    const VF32    rcpDet( Rcp( det ) );
    const VF32    u( rcpDet * ( T | P ) );
    const VF32    v( rcpDet * ( D | Q ) );
    const VF32    hitDistance( rcpDet * ( E2 | Q ) );
    const VF32    hitT( hitDistance * rcpLineLength );
    const Point3  hitPoint( MulAdd( lineVector, hitT, lineStart ) );
    
    *pHitPoint = hitPoint;
    *pHitLineT = hitT;
    *pHitUV    = MakeVector2( u, v );
    
    return ( ( zero <= hitT ) & ( hitT <= one ) ) &
           ( ( ( Abs( det ) > epsilon ) & ( ( u + v ) <= one ) ) &
             ( ( u >= zero ) & ( v >= zero ) ) );
}

/// Line-Triangle Intersection : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool Intersect(
    const Triangle&            triangle,
    const Line&                line,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV )
{
    return Intersect( line, triangle, pHitPoint, pHitLineT, pHitUV );
}


/// Line-Triangle Intersection with Back Face Cull : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW(
    const Line&                line,
    const Triangle&            triangle,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV )
{
    const Point3  lineStart( Start( line ) );
    const Vector3 lineVector( Vector( line ) );
    const Normal3 D( Normal( line ) );
    const VF32    rcpLineLength( RcpLength( line ) );
    const Point3  pointA( PointA( triangle ) );
    const Vector3 E1( EdgeVectorAB( triangle ) );
    const Vector3 E2( EdgeVectorAC( triangle ) );
    const Vector3 triNormalVec( E1 ^ E2 );
    const Vector3 T( lineStart - pointA );
    const Vector3 P( D ^ E2 );
    const Vector3 Q( T ^ E1 );
    const VF32    zero( Const<VF32>::Zero() );
    const VF32    one( Const<VF32>::One() );
    const VF32    epsilon( Const<VF32>::Epsilon() );
    const VF32    det( E1 | P );
    const VF32    rcpDet( Rcp( det ) );
    const VF32    u( rcpDet * ( T | P ) );
    const VF32    v( rcpDet * ( D | Q ) );
    const VF32    hitDistance( rcpDet * ( E2 | Q ) );
    const VF32    hitT( hitDistance * rcpLineLength );
    const Point3  hitPoint( MulAdd( lineVector, hitT, lineStart ) );
    
    *pHitPoint = hitPoint;
    *pHitLineT = hitT;
    *pHitUV    = MakeVector2( u, v );
    
    return ( ( ( zero <= hitT ) & ( hitT <= one ) ) &
             ( ( triNormalVec | D ) < zero ) ) &
           ( ( ( Abs( det ) > epsilon ) & ( ( u + v ) <= one ) ) &
             ( ( u >= zero ) & ( v >= zero ) ) );
}

/// Line-Triangle Intersection with Back Face Cull : returns true if hit is within triangle and between start & end of line
NOALIAS __forceinline const VBool IntersectCullCCW(
    const Triangle&            triangle,
    const Line&                line,
    Point3  * const __restrict pHitPoint,
    VF32    * const __restrict pHitLineT,
    Vector2 * const __restrict pHitUV )
{
    return IntersectCullCCW( line, triangle, pHitPoint, pHitLineT, pHitUV );
}


/// Triangle-Triangle Intersection
NOALIAS /*__force*/inline const VBool Intersect(
    const Triangle&          triangle1,
    const Triangle&          triangle2,
    Point3* const __restrict pP1,
    Point3* const __restrict pP2 )
{
    const VF32    zero( Const<VF32>::Zero() );
    //const VF32    one( Const<VF32>::One() );
    //const VF32    epsilonSquared( Const<VF32>::EpsilonSquared() );
    
    const Point3  V1_0( PointA( triangle1 ) ), V1_1( PointB( triangle1 ) ), V1_2( PointC( triangle1 ) );
    const Point3  V2_0( PointA( triangle2 ) ), V2_1( PointB( triangle2 ) ), V2_2( PointC( triangle2 ) );
    const Vector3 N1( SurfaceVector( triangle1 ) );
    const Vector3 N2( SurfaceVector( triangle2 ) );
    const VF32    d1( -N1 | MakeVector3( V1_0 ) );
    const VF32    d2( -N2 | MakeVector3( V2_0 ) );
    const VF32    dV1_0( ( N2 | MakeVector3( V1_0 ) ) + d2 );
    const VF32    dV1_1( ( N2 | MakeVector3( V1_1 ) ) + d2 );
    const VF32    dV1_2( ( N2 | MakeVector3( V1_2 ) ) + d2 );
    const VF32    dV2_0( ( N1 | MakeVector3( V2_0 ) ) + d1 );
    const VF32    dV2_1( ( N1 | MakeVector3( V2_1 ) ) + d1 );
    const VF32    dV2_2( ( N1 | MakeVector3( V2_2 ) ) + d1 );
    
    //const VF32 deltaV1_01( dV1_0 * dV1_1 ); // TODO: replace multiply with XorSign
    //const VF32 deltaV1_02( dV1_0 * dV1_2 );
    //const VF32 deltaV2_01( dV2_0 * dV2_1 );
    //const VF32 deltaV2_02( dV2_0 * dV2_2 );
    
    const VF32 deltaV1_01( XorSign( dV1_0, dV1_1 ) );
    const VF32 deltaV1_02( XorSign( dV1_0, dV1_2 ) );
    const VF32 deltaV2_01( XorSign( dV2_0, dV2_1 ) );
    const VF32 deltaV2_02( XorSign( dV2_0, dV2_2 ) );
    
    const Vector3 D( N1 ^ N2 );
    const Mat33   abc( MakeMat33( N1, N2, D ) );
    const Vector3 d( -MakeVector3( d1, d2, zero ) );
    
    // Trivial rejection if either triangle's points all lie on one side of the other triangle's plane
    if ( ( ( deltaV1_01 >= zero ) & ( deltaV1_02 >= zero ) ) |
         ( ( deltaV2_01 >= zero ) & ( deltaV2_02 >= zero ) ) )
        return Const<VBool>::False();
    
    // Intersection point of the three planes, O:
    // N1 | O = -d1 : ( N1.x * O.x ) + ( N1.y * O.y ) + ( N1.z * O.z ) = -d1
    // N2 | O = -d2 : ( N2.x * O.x ) + ( N2.y * O.y ) + ( N2.z * O.z ) = -d2
    // D  | O = 0   : ( D.x  * O.x ) + ( D.y  * O.y ) + ( D.z  * O.z ) = 0
    
    Vector3 VO;
    const VBool valid_O( CramersRule( abc, d, &VO ) );
    //const VBool valid_O( CramersRule24( abc, d, &VO ) );
    const Point3 O( MakePoint3( VO ) );
    
    // N1-N2 co-planar check ( No need to check length of D, the Cramer's rule function will return false if any row is zero! )
    //if ( valid_O /* & ( LengthSquared( D ) > epsilonSquared ) */ )
    //{
        const VBool origin1_0( ( deltaV1_01 < zero ) & ( deltaV1_02 < zero ) );
        const VBool origin1_1( deltaV1_01 < zero );
        const VBool origin2_0( ( deltaV2_01 < zero ) & ( deltaV2_02 < zero ) );
        const VBool origin2_1( deltaV2_01 < zero );
        
        const Point3 P1_0( Select( origin1_0, V1_0, Select( origin1_1, V1_1, V1_2 ) ) );
        const Point3 P1_1( Select( origin1_0, V1_1, Select( origin1_1, V1_2, V1_0 ) ) );
        const Point3 P1_2( Select( origin1_0, V1_2, Select( origin1_1, V1_0, V1_1 ) ) );
        
        const Point3 P2_0( Select( origin2_0, V2_0, Select( origin2_1, V2_1, V2_2 ) ) );
        const Point3 P2_1( Select( origin2_0, V2_1, Select( origin2_1, V2_2, V2_0 ) ) );
        const Point3 P2_2( Select( origin2_0, V2_2, Select( origin2_1, V2_0, V2_1 ) ) );
        
        // Line 1 : x = x1 + ( x2 - x1 ) s
        // Line 2 : x = x3 + ( x4 - x3 ) t
        // a = x2 - x1
        // b = x4 - x3
        // c = x3 - x1
        // s = ( ( c x b ) . ( a x b ) ) / ( | a x b | ^2 )
        //   = ( ( c x b ) . ( a x b ) ) / ( ( a x b ) . ( a x b ) )
        
        const Vector3 a( D );
        const Vector3 b1( P1_1 - P1_0 ), b2( P1_2 - P1_0 ), b3( P2_1 - P2_0 ), b4( P2_2 - P2_0 );
        const Vector3 c1( P1_0 - O ),    c2( P1_0 - O ),    c3( P2_0 - O ),    c4( P2_0 - O );
        const Vector3 axb1( a ^ b1 ),    axb2( a ^ b2 ),    axb3( a ^ b3 ),    axb4( a ^ b4 );
        const Vector3 cxb1( c1 ^ b1 ),   cxb2( c2 ^ b2 ),   cxb3( c3 ^ b3 ),   cxb4( c4 ^ b4 );
        
        const Vector4 rcpDenom( Rcp( MakeVector4( axb1 | axb1, axb2 | axb2, axb3 | axb3, axb4 | axb4 ) ) );
        //const Vector4 rcpDenom( Rcp24( MakeVector4( axb1 | axb1, axb2 | axb2, axb3 | axb3, axb4 | axb4 ) ) );
        
        const Vector4 s( MakeVector4( cxb1 | axb1, cxb2 | axb2, cxb3 | axb3, cxb4 | axb4 ) * rcpDenom );
        
        const VF32 t1_0( X( s ) ), t1_1( Y( s ) ), t2_0( Z( s ) ), t2_1( W( s ) );
        
        const VF32 min_t1( Min( t1_0, t1_1 ) );
        const VF32 max_t1( Max( t1_0, t1_1 ) );
        const VF32 min_t2( Min( t2_0, t2_1 ) );
        const VF32 max_t2( Max( t2_0, t2_1 ) );
        
        const VF32 t1( Max( min_t1, min_t2 ) );
        const VF32 t2( Min( max_t1, max_t2 ) );
        
        const Point3 P1( O + D * t1 );
        const Point3 P2( O + D * t2 );
        
        *pP1 = P1;
        *pP2 = P2;
        
        return valid_O &
                 ( ( max_t1 >= min_t2 ) &
                   ( max_t2 >= min_t1 ) );
    //}
    
    //return Const<VBool>::False();
}

/// Triangle-Triangle Intersection
/// NOTE: This is only to be used for speed comparison with the function above!
NOALIAS /*__force*/inline const VBool Intersect2(
    const Triangle&          triangle1,
    const Triangle&          triangle2,
    Point3* const __restrict pP1,
    Point3* const __restrict pP2 )
{
	using namespace Maths;
    
    const HVec3 t10( PointA( triangle1 ) );
    const HVec3 t11( PointB( triangle1 ) );
    const HVec3 t12( PointC( triangle1 ) );
    const HVec3 t20( PointA( triangle2 ) );
    const HVec3 t21( PointB( triangle2 ) );
    const HVec3 t22( PointC( triangle2 ) );
    
	const HF32 zero(Maths::Constants<HF32>::Zero());
	const HF32 one(Maths::Constants<HF32>::One());
	const HF32 epsilon(Maths::Constants<HF32>::Epsilon());
	//calculate the normal of the triangle t1 plane
	const HVec3 vt10(t11-t10);
	const HVec3 vt11(t12-t10);
	const C_Normal n1(vt10^vt11);
	const HF32 d1(-(n1|t10)); //the constant-normal form of the t1 plane


	//calculate the normal of the traingle t2 plane
	const HVec3 vt20(t21-t20);
	const HVec3 vt21(t22-t20);
	const C_Normal n2(vt20^vt21); 
	const HF32 d2(-(n2|t20));//the constant-normal form of the t2 plane

	//the signed distances of each individual vertex of t1 to the t2 plane
	const HF32 dt10((n2|t10) + d2);
	const HF32 dt11((n2|t11) + d2);
	const HF32 dt12((n2|t12) + d2);



	const HF32 mt101=dt10*dt11;
	const HF32 mt102=dt10*dt12;
	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	if(mt101.GT(zero) && mt102.GT(zero)) 
		return Const<VBool>::False();

	const HF32 dt20((n1|t20) + d1);
	const HF32 dt21((n1|t21) + d1);
	const HF32 dt22((n1|t22) + d1);

	const HF32 mt201=dt20*dt21;
	const HF32 mt202=dt20*dt22;
	//the vertexs of t1 are all in the same side of the t2 plane, so no intersection
	if(mt201.GT(zero) && mt202.GT(zero))
		return Const<VBool>::False();

	
	/*
		the code below insure t10 is always on the other side of the t2 plane
	*/
	const Maths::Scalar_Bool a0(mt101.LT(zero) & mt102.LT(zero)); //no change
	const Maths::Scalar_Bool a1(mt101.GT(zero) & mt102.LT(zero)); //Swap 0 and 2, else Swap 0 and 1

	const Maths::HVec3 nt10(Select(a0, t10, Select(a1, t12, t11)));
	const Maths::HVec3 nt11(Select(a0, t11, Select(a1, t11, t10)));
	const Maths::HVec3 nt12(Select(a0, t12, Select(a1, t10, t12)));
	const Maths::HF32  ndt10(Select(a0, dt10, Select(a1, dt12, dt11)));
	const Maths::HF32  ndt11(Select(a0, dt11, Select(a1, dt11, dt10)));
	const Maths::HF32  ndt12(Select(a0, dt12, Select(a1, dt10, dt12)));


	/*
		the code below insure t20 is always on the other side of the t1 plane
	*/
	const Maths::Scalar_Bool b0(mt201.LT(zero) & mt202.LT(zero)); //no change
	const Maths::Scalar_Bool b1(mt201.GT(zero) & mt202.LT(zero)); //Swap 0 and 2, else swap 0 and 1

	const Maths::HVec3 nt20(Select(b0, t20, Select(b1, t22, t21)));
	const Maths::HVec3 nt21(Select(b0, t21, Select(b1, t21, t20)));
	const Maths::HVec3 nt22(Select(b0, t22, Select(b1, t20, t22)));
	const Maths::HF32  ndt20(Select(b0, dt20, Select(b1, dt22, dt21)));
	const Maths::HF32  ndt21(Select(b0, dt21, Select(b1, dt21, dt20)));
	const Maths::HF32  ndt22(Select(b0, dt22, Select(b1, dt20, dt22)));

	/*
		after the above test, the t1 plane and the t2 plane will intersect in a line, whose direction is the
		cross product of these two plane's normal
	*/
	const HVec3 dir(n1^n2);

	//calculate the intersect intervals of t1 and the line
	const HF32 pt10(dir|nt10);
	const HF32 pt11(dir|nt11);
	const HF32 pt12(dir|nt12);

	const HF32 _t1_min=pt10+(pt11-pt10)*(ndt10/(ndt10-ndt11));
	const HF32 _t1_max=pt10+(pt12-pt10)*(ndt10/(ndt10-ndt12));

	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
	const Maths::Scalar_Bool con1(_t1_min.GT(_t1_max));
	const Maths::HF32 t1_min(Select(con1, _t1_max, _t1_min));
	const Maths::HF32 t1_max(Select(con1, _t1_min, _t1_max));
	const Maths::HVec3 nnt11(Select(con1, nt12, nt11));
	const Maths::HVec3 nnt12(Select(con1, nt11, nt12));
	const Maths::HF32 ddt11(Select(con1, ndt12, ndt11));
	const Maths::HF32 ddt12(Select(con1, ndt11, ndt12));


	//calculate the intersect intervals of t2 and the line
	const HF32 pt20(dir|nt20);
	const HF32 pt21(dir|nt21);
	const HF32 pt22(dir|nt22);

	const HF32 _t2_min=pt20+(pt21-pt20)*(ndt20/(ndt20-ndt21));
	const HF32 _t2_max=pt20+(pt22-pt20)*(ndt20/(ndt20-ndt22));

	//swap 1 and 2 to make sure 1 corresponding to the min and 2 corresponding to the max
	const Maths::Scalar_Bool con2(_t2_min.GT(_t2_max));
	const Maths::HF32 t2_min(Select(con2, _t2_max, _t2_min));
	const Maths::HF32 t2_max(Select(con2, _t2_min, _t2_max));
	const Maths::HVec3 nnt21(Select(con2, nt22, nt21));
	const Maths::HVec3 nnt22(Select(con2, nt21, nt22));
	const Maths::HF32 ddt21(Select(con2, ndt22, ndt21));
	const Maths::HF32 ddt22(Select(con2, ndt21, ndt22));


	//Triangle intersect in one line
	if(t2_max.GE(t1_min) && t2_min.LE(t1_max))
	{
		const Maths::HF32 adt12(ddt12.Abs());
		const Maths::HF32 adt11(ddt11.Abs());
		const Maths::HF32 adt10(ndt10.Abs());

		const Maths::HF32 adt22(ddt22.Abs());
		const Maths::HF32 adt21(ddt21.Abs());
		const Maths::HF32 adt20(ndt20.Abs());

		const Maths::HVec3 v210(nnt21 - nt20);
		const Maths::HF32 p210=adt20/(adt20+adt21);
		const Maths::HVec3 contact210(nt20 + p210 * v210);

		const Maths::HVec3 v110(nnt11 - nt10);
		const Maths::HF32 p110=adt10/(adt10+adt11);
		const Maths::HVec3 contact110(nt10 + p110 * v110);

		const Maths::HVec3 v120(nnt12 - nt10);
		const Maths::HF32 p120=adt10/(adt10+adt12);
		const Maths::HVec3 contact120(nt10 + p120 * v120);

		const Maths::HVec3 v220(nnt22 - nt20);
		const Maths::HF32 p220=adt20/(adt20+adt22);
		const Maths::HVec3 contact220(nt20 + p220 * v220);

		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(nt10, "nt10");
		//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw3dText(nt20, "nt20");

		const Maths::Scalar_Bool con1(t1_min.LE(t2_min));
		const Maths::Scalar_Bool con2(t1_max.LE(t2_max));

		//assert(patch.m_NumContactPoints < MaxContacts);
		//C_ContactPoint& point1(patch.m_ContactPoints[patch.m_NumContactPoints++]);
		//point1.m_ContactPoint = Select(con1, contact210, contact110);
		//point1.m_HashValue = hashValue << 1;
		//point1.m_PenetrationDepth=adt20;
		//point1.m_PhysicsMaterialIndices = physicsMaterial;
		//point1.m_EffectMaterialIndices = effectMaterial;
        *pP1 = Select(con1, contact210, contact110);

		//assert(patch.m_NumContactPoints < MaxContacts);
		//C_ContactPoint& point2(patch.m_ContactPoints[patch.m_NumContactPoints++]);
		//point2.m_ContactPoint = Select(con2, contact120, contact220);
		//point2.m_HashValue = (hashValue << 1) | 1;
		//point2.m_PenetrationDepth=adt20;
		//point2.m_PhysicsMaterialIndices = physicsMaterial;
		//point2.m_EffectMaterialIndices = effectMaterial;
        *pP2 = Select(con2, contact120, contact220);
        
        return Const<VBool>::True();
	}
    return Const<VBool>::False();
}







/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

