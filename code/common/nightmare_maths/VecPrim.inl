
//#include "stdafx.h"



//
// Plane Class Member Functions
//

const HF32 C_Plane::Intersect( const C_Line& line ) const
{
    const HVec3 N = Normal();
    const HVec3 P = N * -DistanceToOrigin();
    const HF32 denom = N | line.m_V;
    if ( denom.Abs() > FLT_EPSILON )
    {
        return ( N | ( P - line.m_Q ) ) / denom;
    }
    else
    {
        return -FLT_MAX;
    }
}

const E_PlaneTest C_Plane::Test( const C_Sphere& sphere, const HF32& tolerance ) const
{
    const HF32 d( Distance( sphere.Centre() ) );
    const HF32 r( sphere.Radius() - tolerance );
    if ( d >= r )       return E_PlaneTestFront;
    else if ( -d >= r ) return E_PlaneTestBack;
    else                return E_PlaneTestIntersect;
}

const E_PlaneTest C_Plane::Test( const C_Box& box, const HF32& tolerance ) const
{
    HVec3 vertex[8];
    box.GetVertexArray( vertex );
    
    E_PlaneTest result( E_PlaneTestInvalid );
    
    for ( U32 i=0; i<8; i++ )
    {
        const HF32 d( Distance( vertex[i] ) );
        if ( abs( d ) >= tolerance )
        {
            if ( d >= 0.0f ) result = (E_PlaneTest)( result | E_PlaneTestFront );
            else             result = (E_PlaneTest)( result | E_PlaneTestBack );
            
            if ( result == E_PlaneTestIntersect )
            {
                return E_PlaneTestIntersect;
            }
        }
    }
    
    return result;
}






/// Multiplication of capsule by orthogonal, unit axes rotation + translation matrix
const C_Cylinder C_Cylinder::operator * ( const HMat43& rotTransMatrix ) const
{
    const HMat33& rotMatrix = (HMat33&)rotTransMatrix;
    
    return C_Cylinder(
        m_Start  * rotTransMatrix,
        m_End    * rotTransMatrix,
        m_Vector * rotMatrix,
        m_Normal * rotMatrix,
        m_Length,
        m_Radius );
}

/// Cylinder-Line Intersection - Walls only - no hits on end caps
const HF32 C_Cylinder::IntersectWalls( const C_Line& line, C_Normal& outputNormal ) const
{
    // Early rejection check: Test if distance between lines is greater than cylinder radius
    // First line is the cylinder axis, second line is the input line
    // x = x1 + ( x2 - x1 ) s
    // x = x3 + ( x4 - x3 ) t
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // d = |c.(axb)| / |axb|
    
    const C_Vector& a = m_Vector;
    const C_Vector& b = line.Vector();
    const C_Vector  axb( a ^ b );
    const HF32      axb_dot_axb( axb | axb );
    
    // Early rejection check for parallel lines
    if ( axb_dot_axb > FLT_EPSILON )
    {
        const C_Vector c( line.Start() - m_Start );
        const HF32     c_dot_axb( c | axb );
        const HF32     d2 = ( c_dot_axb * c_dot_axb ) / axb_dot_axb;
        const HF32     r( Radius() );
        const HF32     r2( r * r );
        
        if ( d2 <= r2 )
        {
            // Test for hits on cylinder walls
            // Uses Sphere-Line intersection algorithm with the line mapped onto the plane of the start end cap
            // and the sphere at the origin of the start end cap, with the same radius as the cylinder.
            // Solves Sphere-Line intersection using the Quadratic formula
            // Sphere:   P + r
            // Line:     Q + tv
            // Equation: t^2 (v.v) + t (2v.(Q-P)) + ((Q-P).(Q-P)-r^2) = 0
            // Solve using quadratic formula: a = v.v, b = 2v.(Q-P), c = ((Q-P).(Q-P)-r^2)
            
            const C_Plane   cylinderPlaneStart( m_Normal, m_Start );
            const C_Vertex& P = m_Start;
            const C_Vertex  Q( cylinderPlaneStart.OnPlane( line.Start() ) );
            const C_Vector  v( cylinderPlaneStart.ParallelToPlane( line.Vector() ) );
            const C_Vector  pq( Q - P );
            const HF32      zero;
            const HF32      one( 1.0f );
            const HF32      v_dot_pq( v | pq );
            const HF32      quadratic_a( v | v );
            const HF32      quadratic_b( v_dot_pq + v_dot_pq );
            const HF32      quadratic_c( ( pq | pq ) - r2 );
            
            const C_QuadraticResult quadraticResult( SolveQuadratic( quadratic_a, quadratic_b, quadratic_c ) );
            
            const HF32 t( quadraticResult.T0() );
            
            if ( ( zero < t ) && ( t < one ) )
            {
                const C_Plane  cylinderPlaneEnd( m_Normal, m_End );
                const C_Vertex hitVertex( line( t ) );
                
                // Test if hit point within end caps
                
                if ( ( cylinderPlaneStart.Distance( hitVertex ) >= zero ) &&
                     ( cylinderPlaneEnd.Distance( hitVertex ) <= zero ) )
                {
                    outputNormal = Q + v * t - P;
                    return t;
                }
            }
        }
    }
    
    return -FLT_MAX;
}

/// Cylinder-Line Intersection - End caps only - no hits on walls
const HF32 C_Cylinder::IntersectEndCaps( const C_Line& line, C_Normal& outputNormal ) const
{
    OutputDebugString( DBSTR "NOT IMPLEMENTED YET!" );
    return -FLT_MAX;
}

/// Cylinder-Line Intersection - Full: Combination of Walls + End Caps
const HF32 C_Cylinder::Intersect( const C_Line& line, C_Normal& outputNormal ) const
{
    OutputDebugString( DBSTR "NOT IMPLEMENTED YET!" );
    return -FLT_MAX;
}





/// Multiplication of capsule by orthogonal, unit axes rotation + translation matrix
const C_Capsule C_Capsule::operator * ( const HMat43& rotTransMatrix ) const
{
    const HMat33& rotMatrix = (HMat33&)rotTransMatrix;
    
    return C_Capsule(
        m_Start  * rotTransMatrix,
        m_End    * rotTransMatrix,
        m_Vector * rotMatrix,
        m_Normal * rotMatrix,
        m_Length,
        m_Radius );
}





const bool Intersect( const C_Line& line, const C_Box& box, HF32& enterT, HF32& exitT )
{
    enterT = exitT = -FLT_MAX;
    
    const C_Vector zeroVector;
    const C_Vertex& Q = line.Q();
    const C_Vector& V = line.V();
    const C_Vertex boxMin( box.Min() );
    const C_Vertex boxMax( box.Max() );
    
    const E_SignBits vgtz = V > zeroVector;
    const E_SignBits vltz = V < zeroVector;
    
    // Check for intersection in X axis
    
    if ( ( vgtz | vltz ) & E_SignBitsX )
    {
        const HF32 reciprocalVX( Reciprocal( V.X() ) );
        
        const HF32 xT0( ( boxMin - Q ).X() * reciprocalVX );
        const C_Vertex xP0( Q + V * xT0 );
        if ( ( ( boxMin <= xP0 ) & ( xP0 <= boxMax ) & E_SignBitsYZ ) == E_SignBitsYZ )
        {
            if ( vgtz & E_SignBitsX )
            {
                enterT = xT0;
            }
            else // if ( vltz & E_SignBitsX )
            {
                exitT = xT0;
            }
        }
        
        const HF32 xT1( ( boxMax - Q ).X() * reciprocalVX );
        const C_Vertex xP1( Q + V * xT1 );
        if ( ( ( boxMin <= xP1 ) & ( xP1 <= boxMax ) & E_SignBitsYZ ) == E_SignBitsYZ )
        {
            if ( vgtz & E_SignBitsX )
            {
                exitT = xT1;
            }
            else // if ( vltz & E_SignBitsX )
            {
                enterT = xT1;
            }
        }
    }
    
    // Check for intersection in Y axis
    
    if ( ( vgtz | vltz ) & E_SignBitsY )
    {
        const HF32 reciprocalVY( Reciprocal( V.Y() ) );
        
        const HF32 yT0( ( boxMin - Q ).Y() * reciprocalVY );
        const C_Vertex yP0( Q + V * yT0 );
        if ( ( ( boxMin <= yP0 ) & ( yP0 <= boxMax ) & E_SignBitsXZ ) == E_SignBitsXZ )
        {
            if ( vgtz & E_SignBitsY )
            {
                enterT = yT0;
            }
            else // if ( vltz & E_SignBitsY )
            {
                exitT = yT0;
            }
        }
        
        const HF32 yT1( ( boxMax - Q ).Y() * reciprocalVY );
        const C_Vertex yP1( Q + V * yT1 );
        if ( ( ( boxMin <= yP1 ) & ( yP1 <= boxMax ) & E_SignBitsXZ ) == E_SignBitsXZ )
        {
            if ( vgtz & E_SignBitsY )
            {
                exitT = yT1;
            }
            else // if ( vltz & E_SignBitsY )
            {
                enterT = yT1;
            }
        }
    }
    
    // Check for intersection in Z axis
    
    if ( ( vgtz | vltz ) & E_SignBitsZ )
    {
        const HF32 reciprocalVZ( Reciprocal( V.Z() ) );
        
        const HF32 zT0( ( boxMin - Q ).Z() * reciprocalVZ );
        const C_Vertex zP0( Q + V * zT0 );
        if ( ( ( boxMin <= zP0 ) & ( zP0 <= boxMax ) & E_SignBitsXY ) == E_SignBitsXY )
        {
            if ( vgtz & E_SignBitsZ )
            {
                enterT = zT0;
            }
            else // if ( vltz & E_SignBitsZ )
            {
                exitT = zT0;
            }
        }
        
        const HF32 zT1( ( boxMax - Q ).Z() * reciprocalVZ );
        const C_Vertex zP1( Q + V * zT1 );
        if ( ( ( boxMin <= zP1 ) & ( zP1 <= boxMax ) & E_SignBitsXY ) == E_SignBitsXY )
        {
            if ( vgtz & E_SignBitsZ )
            {
                exitT = zT1;
            }
            else // if ( vltz & E_SignBitsZ )
            {
                enterT = zT1;
            }
        }
    }
    
    const HF32 zero;
    const HF32 one( 1.0f );
    return ( exitT >= zero ) && ( enterT <= one );
}






