#include "CorePch.h"
//#include <core/Core.h>
//#include <core/Maths.h>

namespace Maths
{
//
// ---------------------------------------------------------------------
// |-- C_Plane : Plane class stored as position & distance to origin --|
// ---------------------------------------------------------------------
//

/// Tests if the given Sphere is in front, behind or intersecting this Plane
NOALIAS const E_PlaneTest C_Plane::Test( const C_Sphere PCREF sphere, const HF32 PCREF tolerance ) const __restrict
{
    const HF32 d( Distance( sphere.Centre() ) );
    const HF32 r( sphere.Radius() - tolerance );
    if ( d >= r )       return E_PlaneTestFront;
    else if ( -d >= r ) return E_PlaneTestBack;
    else                return E_PlaneTestIntersect;
}

/// Tests if the given Box is in front, behind or intersecting this Plane
NOALIAS const E_PlaneTest C_Plane::Test( const C_AxisBox& box, const HF32 PCREF tolerance ) const __restrict
{
    HVec3 vertex[8];
    box.GetVertexArray( vertex );
    
    E_PlaneTest result( E_PlaneTestInvalid );
    
    for ( U32 i=0; i<8; i++ )
    {
        const HF32 d( Distance( vertex[i] ) );
        if ( Abs( d ) >= tolerance )
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

//#ifdef SHOOTER_VECMAT_DEBUG_PRINT
//void C_Plane::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    sprintf_s( buffer, 256, "%s Plane( Normal( %+11.6f, %+11.6f, %+11.6f ), Offset( %+11.6f ) )\n", text, (F32)m_V.X(), (F32)m_V.Y(), (F32)m_V.Z(), (F32)m_V.W() );
//    OutputDebugString( buffer );
//}
//#endif




//
// ------------------------------------------------------------
// |-- C_Line : Line class stored as start, normal & length --|
// ------------------------------------------------------------
//

//#ifdef SHOOTER_VECMAT_DEBUG_PRINT
//void C_Line::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    
//    sprintf_s( buffer, 256, "%s Start", text );
//    Start().DbgPrint( buffer );
//    
//    sprintf_s( buffer, 256, "%s Vector", text );
//    Vector().DbgPrint( buffer );
//}
//#endif





//
// ------------------------------------------------------------------------------
// |-- C_Sphere : Sphere class stored as a HVec4 with centre XYZ and radius W --|
// ------------------------------------------------------------------------------
//

//#ifdef SHOOTER_VECMAT_DEBUG_PRINT
//void C_Sphere::DbgPrint( char* text ) const
//{
//    char buffer[256];
//    HVec4 v( m_V );
//    sprintf_s( buffer, 256, "%s C_Sphere( Centre( %+11.6f, %+11.6f, %+11.6f ), Radius( %+11.6f ) )\n", text, (F32)v.X(), (F32)v.Y(), (F32)v.Z(), (F32)v.W() );
//    OutputDebugString( buffer );
//}
//#endif










//
// -----------------------------------
// |-- C_Cylinder : Cylinder Class --|
// -----------------------------------
//

/// Multiplication of capsule by orthogonal, unit axes rotation + translation matrix
NOALIAS const C_Cylinder C_Cylinder::operator * ( const HMat43& rotTransMatrix ) const __restrict
{
    const C_Vertex newStart( Start() * rotTransMatrix );
    const C_Vertex newEnd( End() * rotTransMatrix );
    
    return C_Cylinder( newStart, Normalise( newEnd - newStart ), Length(), Radius() );
}


/// Cylinder-Vertex Distance
NOALIAS const HF32 C_Cylinder::Distance( const C_Vertex PCREF vertex ) const __restrict
{
    const HF32      zero;
    const HF32      radius( Radius() );
    const HF32      length( Length() );
    const C_Vertex  start( Start() );
    const C_Vertex  end( End() );
    const C_Normal  normal( Normal() );
    const C_Vector  startToVertex( vertex - start );
    const HF32      dp( normal | startToVertex );
    const C_Vector  lineParaStartToVertex( normal * dp );
    const C_Vector  linePerpStartToVertex( startToVertex - lineParaStartToVertex );
    const HF32      linePerpStartToVertexDistance( linePerpStartToVertex.Length() );
    const C_Vertex  lineClosestPoint( start + lineParaStartToVertex );
    const C_Vector  linePerpToRim( linePerpStartToVertex * radius * linePerpStartToVertex.ReciprocalLength() );
    
    #if 0
    
    // TODO: Eliminate these branches with conditional select logic!
    if ( dp < zero )
    {
        if ( linePerpStartToVertexDistance <= radius )
        {
            return start.Distance( lineClosestPoint );
        }
        else
        {
            return vertex.Distance( start + linePerpToRim );
        }
    }
    else if ( dp > length )
    {
        if ( linePerpStartToVertexDistance <= radius )
        {
            return end.Distance( lineClosestPoint );
        }
        else
        {
            return vertex.Distance( end + linePerpToRim );
        }
    }
    else // if ( ( zero <= dp ) && ( dp <= length ) )
    {
        return linePerpStartToVertexDistance - radius;
    }
    
    #else
    
    // This generates same results as above but computes all results in parallel and selects correct result
    // TODO: Use a select function rather than a constructor!!!
    
    const Scalar_Bool vertexWithinRadius( linePerpStartToVertexDistance.LE( radius ) );
    
    const HF32 result_if_dp_LT_zero( Select( vertexWithinRadius, start.Distance( lineClosestPoint ), vertex.Distance( start + linePerpToRim ) ) );
    
    const HF32 result_if_dp_GT_length( Select( vertexWithinRadius, end.Distance( lineClosestPoint ), vertex.Distance( end + linePerpToRim ) ) );
    
    const HF32 result_if_dp_within_zero_to_length( linePerpStartToVertexDistance - radius );
    
    const HF32 result_if_dp_GE_zero( Select( dp.GT( length ), result_if_dp_GT_length, result_if_dp_within_zero_to_length ) );
    
    return HF32( Select( dp.LT( zero ), result_if_dp_LT_zero, result_if_dp_GE_zero ) );
    
    #endif
}


/// Cylinder-Line Intersection - Walls only - no hits on end caps
NOALIAS const HF32 C_Cylinder::IntersectWalls( const C_Line& line, C_Normal& outputNormal ) const __restrict
{
    // Early rejection check: Test if distance between lines is greater than cylinder radius
    // First line is the cylinder axis, second line is the input line
    // x = x1 + ( x2 - x1 ) s
    // x = x3 + ( x4 - x3 ) t
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // d = |c.(axb)| / |axb|
    
    const C_Vertex cylinderStart( Start() );
    const C_Vertex cylinderEnd( End() );
    const C_Vector cylinderVector( Vector() );
    const C_Normal cylinderNormal( Normal() );
    
    const C_Vertex lineStart( line.Start() );
    const C_Vector lineVector( line.Vector() );
    
    const C_Vector a = cylinderVector;
    const C_Vector b = lineVector;
    const C_Vector  axb( a ^ b );
    const HF32      axb_dot_axb( axb | axb );
    const HF32      rcp_axb_dot_axb( Reciprocal( axb_dot_axb ) );
    
    // Early rejection check for parallel lines
    //if ( axb_dot_axb.GT( Constants<HF32>::Epsilon() ) )
    {
        const C_Vector c( lineStart - cylinderStart );
        const HF32     c_dot_axb( c | axb );
        const HF32     d2 = ( c_dot_axb * c_dot_axb ) * rcp_axb_dot_axb;
        const HF32     r( Radius() );
        const HF32     r2( r * r );
        
        //if ( d2.LE( r2 ) )
        if ( axb_dot_axb.GT( Constants<HF32>::Epsilon() ) & d2.LE( r2 ) )
        {
            // Test for hits on cylinder walls
            // Uses Sphere-Line intersection algorithm with the line mapped onto the plane of the start end cap
            // and the sphere at the origin of the start end cap, with the same radius as the cylinder.
            // Solves Sphere-Line intersection using the Quadratic formula
            // Sphere:   P + r
            // Line:     Q + tv
            // Equation: t^2 (v.v) + t (2v.(Q-P)) + ((Q-P).(Q-P)-r^2) = 0
            // Solve using quadratic formula: a = v.v, b = 2v.(Q-P), c = ((Q-P).(Q-P)-r^2)
            
            const C_Plane   cylinderPlaneStart( cylinderNormal, cylinderStart );
            const C_Vertex P = cylinderStart;
            const C_Vertex  Q( cylinderPlaneStart.OnPlane( lineStart ) );
            const C_Vector  v( cylinderPlaneStart.ParallelToPlane( lineVector ) );
            const C_Vector  pq( Q - P );
            const HF32      zero( Constants<HF32>::Zero() );
            const HF32      one( Constants<HF32>::One() );
            const HF32      v_dot_pq( v | pq );
            const HF32      quadratic_a( v | v );
            const HF32      quadratic_b( v_dot_pq + v_dot_pq );
            const HF32      quadratic_c( ( pq | pq ) - r2 );
            
            const C_QuadraticResult quadraticResult( SolveQuadratic( quadratic_a, quadratic_b, quadratic_c ) );
            
            const HF32 t( quadraticResult.T0() );
            
            //if ( zero.LT( t ) && t.LT( one ) )
            {
                const C_Plane  cylinderPlaneEnd( cylinderNormal, cylinderEnd );
                const C_Vertex hitVertex( line( t ) );
                
                outputNormal = Q + ( v * t ) - P;
                
                // Test if hit point within end caps
                
                if ( ( zero.LT( t ) & t.LT( one ) ) &
                     ( cylinderPlaneStart.Distance( hitVertex ).GE( zero ) & cylinderPlaneEnd.Distance( hitVertex ).LE( zero ) ) )
                {
                    return t;
                }
            }
        }
    }
    
    //return -FLT_MAX;
    
    outputNormal = C_Normal( ZERO_INIT );
    return -Constants<HF32>::Max();
}

/// Cylinder-Line Intersection - End caps only - no hits on walls
NOALIAS const HF32 C_Cylinder::IntersectEndCaps( const C_Line& /*line*/, C_Normal& /*outputNormal*/ ) const __restrict
{
    assert( 0 && "Not implemented yet!\n" );
    return -FLT_MAX;
}

/// Cylinder-Line Intersection - Full: Combination of Walls + End Caps
NOALIAS const HF32 C_Cylinder::Intersect( const C_Line& /*line*/, C_Normal& /*outputNormal*/ ) const __restrict
{
    assert( 0 && "Not implemented yet!\n" );
    return -FLT_MAX;
}










//
// ---------------------------------
// |-- C_Capsule : Capsule Class --|
// ---------------------------------
//

/// Multiplication of capsule by orthogonal, unit axes rotation + translation matrix
NOALIAS const C_Capsule C_Capsule::operator * ( const HMat43& rotTransMatrix ) const __restrict
{
    const C_Vertex newStart( Start() * rotTransMatrix );
    const C_Vertex newEnd( End() * rotTransMatrix );
    
    return C_Capsule( newStart, Normalise( newEnd - newStart ), Length(), Radius() );
}





//
// ------------------------------------
// |-- Standalone Support Functions --|
// ------------------------------------
//

/// Intersect a line with a box
NOALIAS const Scalar_Bool Intersect( const C_Line& line, const C_AxisBox& box, const HVec3 PCREF rcpLineVector, HF32& resultEnterT, HF32& resultExitT )
{
    const HF32 fltMax( c_ScalarFltMax );
    
    HF32 enterT( -fltMax );
    HF32 exitT( -fltMax );
    
    const HVec3 v_zero;
	const HVec3 Q( line.Start().m_V );
	const HVec3 V( line.Vector().m_V );
	const HVec3 boxMin( box.Min().m_V );
	const HVec3 boxMax( box.Max().m_V );
    
    // Calculate intersection points on the plane for each side of the box
    
    const HVec3 reciprocal_V( rcpLineVector );
    const HVec3 T0( ( boxMin - Q ) * reciprocal_V );
    const HVec3 T1( ( boxMax - Q ) * reciprocal_V );

	/*const HVec3 tmin(Min(T0, T1));
	const HVec3 tmax(Max(T0, T1));

	resultEnterT = (Min(tmin.X(), Min(tmin.Y(), tmin.Z())));
	resultExitT = (Max(tmax.X(), Max(tmax.Y(), tmax.Z())));

	const Vector3_Bool bvalid(Abs(Q) > epsilon | (V >= boxMin) & (V <= boxMax));
	return X(bvalid) && Y(bvalid) && Z(bvalid) && resultEnterT.LE(one) && resultExitT.GE(zero);*/
    
    const HVec3 xP0( Q + V * T0.X() );
    const HVec3 xP1( Q + V * T1.X() );
    const HVec3 yP0( Q + V * T0.Y() );
    const HVec3 yP1( Q + V * T1.Y() );
    const HVec3 zP0( Q + V * T0.Z() );
    const HVec3 zP1( Q + V * T1.Z() );
    
    // Check if each intersection point is within the bounds of it's face
    
    const Vector3_Bool in_xP0( ( boxMin <= xP0 ) & ( xP0 <= boxMax ) );
    const Vector3_Bool in_xP1( ( boxMin <= xP1 ) & ( xP1 <= boxMax ) );
    const Vector3_Bool in_yP0( ( boxMin <= yP0 ) & ( yP0 <= boxMax ) );
    const Vector3_Bool in_yP1( ( boxMin <= yP1 ) & ( yP1 <= boxMax ) );
    const Vector3_Bool in_zP0( ( boxMin <= zP0 ) & ( zP0 <= boxMax ) );
    const Vector3_Bool in_zP1( ( boxMin <= zP1 ) & ( zP1 <= boxMax ) );
    
    const Scalar_Bool in_xP0_YZ( Y( in_xP0 ) & Z( in_xP0 ) );
    const Scalar_Bool in_xP1_YZ( Y( in_xP1 ) & Z( in_xP1 ) );
    const Scalar_Bool in_yP0_XZ( X( in_yP0 ) & Z( in_yP0 ) );
    const Scalar_Bool in_yP1_XZ( X( in_yP1 ) & Z( in_yP1 ) );
    const Scalar_Bool in_zP0_XY( X( in_zP0 ) & Y( in_zP0 ) );
    const Scalar_Bool in_zP1_XY( X( in_zP1 ) & Y( in_zP1 ) );
    
    // Calculate direction of ray in each axis
    
    const Vector3_Bool V_GT_zero( V > v_zero );
    const Vector3_Bool V_LT_zero( V < v_zero );
    
    const Scalar_Bool V_GT_zero_X( X( V_GT_zero ) );
    const Scalar_Bool V_LT_zero_X( X( V_LT_zero ) );
    const Scalar_Bool V_GT_zero_Y( Y( V_GT_zero ) );
    const Scalar_Bool V_LT_zero_Y( Y( V_LT_zero ) );
    const Scalar_Bool V_GT_zero_Z( Z( V_GT_zero ) );
    const Scalar_Bool V_LT_zero_Z( Z( V_LT_zero ) );
    
    // Calculate entry & exit points of the line through the box
    
    enterT = Select( V_GT_zero_X & in_xP0_YZ, T0.X(), Select( V_LT_zero_X & in_xP1_YZ, T1.X(), enterT ) );
    exitT  = Select( V_LT_zero_X & in_xP0_YZ, T0.X(), Select( V_GT_zero_X & in_xP1_YZ, T1.X(), exitT ) );
    enterT = Select( V_GT_zero_Y & in_yP0_XZ, T0.Y(), Select( V_LT_zero_Y & in_yP1_XZ, T1.Y(), enterT ) );
    exitT  = Select( V_LT_zero_Y & in_yP0_XZ, T0.Y(), Select( V_GT_zero_Y & in_yP1_XZ, T1.Y(), exitT ) );
    enterT = Select( V_GT_zero_Z & in_zP0_XY, T0.Z(), Select( V_LT_zero_Z & in_zP1_XY, T1.Z(), enterT ) );
    exitT  = Select( V_LT_zero_Z & in_zP0_XY, T0.Z(), Select( V_GT_zero_Z & in_zP1_XY, T1.Z(), exitT ) );
    
    resultEnterT = enterT;
    resultExitT = exitT;
    
    const HF32 zero;
    const HF32 one( c_ScalarOne );
    //return ( exitT >= zero ) && ( enterT <= one );
    //return ( exitT >= zero ) & ( enterT <= one );
    return exitT.GE( zero ) & enterT.LE( one );
}

NOALIAS const Scalar_Bool Intersect( const C_Line& line, const C_AxisBox& box, HF32& resultEnterT, HF32& resultExitT )
{
	const HF32 fltMax( c_ScalarFltMax );
    
    HF32 enterT( -fltMax );
    HF32 exitT( -fltMax );
    
    const C_Vector v_zero;
    const C_Vertex Q( line.Start() );
    const C_Vector V( line.Vector() );
    const HVec3 boxMin( box.Min() );
    const HVec3 boxMax( box.Max() );
    
    // Calculate intersection points on the plane for each side of the box
    
    const HVec3 reciprocal_V( Reciprocal( HVec3( V ) ) );
    const HVec3 T0( HVec3( boxMin - Q ) * reciprocal_V );
    const HVec3 T1( HVec3( boxMax - Q ) * reciprocal_V );
    
    const HVec3 xP0( Q + V * T0.X() );
    const HVec3 xP1( Q + V * T1.X() );
    const HVec3 yP0( Q + V * T0.Y() );
    const HVec3 yP1( Q + V * T1.Y() );
    const HVec3 zP0( Q + V * T0.Z() );
    const HVec3 zP1( Q + V * T1.Z() );
    
    // Check if each intersection point is within the bounds of it's face
    
    const Vector3_Bool in_xP0( ( boxMin <= xP0 ) & ( xP0 <= boxMax ) );
    const Vector3_Bool in_xP1( ( boxMin <= xP1 ) & ( xP1 <= boxMax ) );
    const Vector3_Bool in_yP0( ( boxMin <= yP0 ) & ( yP0 <= boxMax ) );
    const Vector3_Bool in_yP1( ( boxMin <= yP1 ) & ( yP1 <= boxMax ) );
    const Vector3_Bool in_zP0( ( boxMin <= zP0 ) & ( zP0 <= boxMax ) );
    const Vector3_Bool in_zP1( ( boxMin <= zP1 ) & ( zP1 <= boxMax ) );
    
    const Scalar_Bool in_xP0_YZ( Y( in_xP0 ) & Z( in_xP0 ) );
    const Scalar_Bool in_xP1_YZ( Y( in_xP1 ) & Z( in_xP1 ) );
    const Scalar_Bool in_yP0_XZ( X( in_yP0 ) & Z( in_yP0 ) );
    const Scalar_Bool in_yP1_XZ( X( in_yP1 ) & Z( in_yP1 ) );
    const Scalar_Bool in_zP0_XY( X( in_zP0 ) & Y( in_zP0 ) );
    const Scalar_Bool in_zP1_XY( X( in_zP1 ) & Y( in_zP1 ) );
    
    // Calculate direction of ray in each axis
    
    const Vector3_Bool V_GT_zero( HVec3(V) > v_zero );
    const Vector3_Bool V_LT_zero( HVec3(V) < v_zero );
    
    const Scalar_Bool V_GT_zero_X( X( V_GT_zero ) );
    const Scalar_Bool V_LT_zero_X( X( V_LT_zero ) );
    const Scalar_Bool V_GT_zero_Y( Y( V_GT_zero ) );
    const Scalar_Bool V_LT_zero_Y( Y( V_LT_zero ) );
    const Scalar_Bool V_GT_zero_Z( Z( V_GT_zero ) );
    const Scalar_Bool V_LT_zero_Z( Z( V_LT_zero ) );
    
    // Calculate entry & exit points of the line through the box
    
    enterT = Select( V_GT_zero_X & in_xP0_YZ, T0.X(), Select( V_LT_zero_X & in_xP1_YZ, T1.X(), enterT ) );
    exitT  = Select( V_LT_zero_X & in_xP0_YZ, T0.X(), Select( V_GT_zero_X & in_xP1_YZ, T1.X(), exitT ) );
    enterT = Select( V_GT_zero_Y & in_yP0_XZ, T0.Y(), Select( V_LT_zero_Y & in_yP1_XZ, T1.Y(), enterT ) );
    exitT  = Select( V_LT_zero_Y & in_yP0_XZ, T0.Y(), Select( V_GT_zero_Y & in_yP1_XZ, T1.Y(), exitT ) );
    enterT = Select( V_GT_zero_Z & in_zP0_XY, T0.Z(), Select( V_LT_zero_Z & in_zP1_XY, T1.Z(), enterT ) );
    exitT  = Select( V_LT_zero_Z & in_zP0_XY, T0.Z(), Select( V_GT_zero_Z & in_zP1_XY, T1.Z(), exitT ) );
    
    resultEnterT = enterT;
    resultExitT = exitT;

	const HF32 zero;
    const HF32 one( c_ScalarOne );
    //return ( exitT >= zero ) && ( enterT <= one );
    //return ( exitT >= zero ) & ( enterT <= one );
    return exitT.GE( zero ) & enterT.LE( one );
    
    /*const HF32 zero;
    const HF32 one( c_ScalarOne );
    //return ( exitT >= zero ) && ( enterT <= one );
    //return ( exitT >= zero ) & ( enterT <= one );
    return exitT.GE( zero ) & enterT.LE( one );

	const HVec3 epsilon(Maths::Constants<HF32>::Epsilon(), Maths::Constants<HF32>::Epsilon(), Maths::Constants<HF32>::Epsilon());
	const HVec3 Q( line.Start().m_V );
	const HVec3 V( line.Vector().m_V );
	const HVec3 recipV(V.Reciprocal());

	
    const HVec3 boxMin( box.Min() );
    const HVec3 boxMax( box.Max() );
	const HVec3 absV(Abs(V));

	const HVec3 MinQ((boxMin - Q)* recipV);
	const HVec3 MaxQ((boxMax - Q)* recipV);
	
	const HVec3 t1 = Min(MinQ, MaxQ);
	const HVec3 t2 = Max(MinQ, MaxQ);
	
	resultEnterT = Min(t1.X(), Min(t1.Y(), t1.Z()));
	resultExitT = Max(t2.X(), Max(t2.Y(), t2.Z()));
	return resultEnterT.LE(resultExitT);*/
}

// Axis box support functions

NOALIAS const S32 GetLongestAxis( const C_AxisBox& box, C_Normal& longestAxis, HF32& longestAxisLength )
{
	const Maths::HVec3 boundingBoxSize = box.Diagonal();

	S32 longestAxisIndex = -1;
	longestAxisLength = 0.0f;

	for(S32 loop=0; loop<3; loop++)
	{
		Maths::C_Normal axis;
		switch(loop)
		{
		default:
		case 0:
			axis = Normalise(HVec3(1.0f, 0.0f, 0.0f));
			break;
		case 1:
			axis = Normalise(HVec3(0.0f, 1.0f, 0.0f));
			break;
		case 2:
			axis = Normalise(HVec3(0.0f, 0.0f, 1.0f));
			break;
		}

		F32 axisSize = axis|(boundingBoxSize);

		if(axisSize > longestAxisLength)
		{
			longestAxisLength = axisSize;
			longestAxis = axis;
			longestAxisIndex = loop;
		}
	}

	return longestAxisIndex;
}

NOALIAS const C_AxisBox Intersection( const C_AxisBox& boxA, const C_AxisBox& boxB )
{
	// max is smallest max
	const Maths::HVec3 boxMax = Maths::Min( boxA.Max(), boxB.Max() );
	// min is biggest min
	const Maths::HVec3 boxMin = Maths::Max( boxA.Min(), boxB.Min() );

	Maths::HVec3 diag = boxMax - boxMin;
	diag = Maths::Max(diag, Maths::HVec3(0.0f, 0.0f, 0.0f));

	return C_AxisBox( boxMin, boxMin + diag );
}

NOALIAS const Maths::C_AxisBox Clip( const Maths::C_AxisBox& box, const Maths::C_AxisBox& limit )
{
	const Maths::HVec3 boxMin = Maths::Max( box.Min(), limit.Min() );
	const Maths::HVec3 boxMax = Maths::Min( box.Max(), limit.Max() );

	return C_AxisBox( boxMin, boxMax );
}

NOALIAS const C_Vertex Clip( const C_Vertex PCREF point, const Maths::C_AxisBox& box )
{
	return C_Vertex( Maths::Min( Maths::Max( point, box.Min() ), box.Max() ) );
}

NOALIAS const HF32 Distance( const C_Vertex PCREF point, const C_AxisBox& box )
{
	const C_Vertex clippedPoint = Clip( point, box );

	return clippedPoint.Distance(point);
}

NOALIAS const HF32 Distance( const C_AxisBox& boxA, const C_AxisBox& boxB )
{
	const Maths::HVec3 halfDiagonalA = boxA.Diagonal() * 0.5f;

	// shrink A to its centre and expand B by the size of A (its almost like the minkowski sum?)
	const Maths::C_Vertex boxACentre( boxA.Centre() );
	const Maths::C_AxisBox expandedB( boxB.Min() - halfDiagonalA, boxB.Max() + halfDiagonalA);

	return Distance( boxACentre, expandedB );
}


}; // namespace Maths



