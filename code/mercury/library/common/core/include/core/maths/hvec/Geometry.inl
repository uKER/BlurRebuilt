#ifndef GEOMETRY_INL
#define GEOMETRY_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

//
// -----------------------------------------------------------------------------------
// |-- Quadratic Result Class - Stores the two F32 results of a quadratic equation --|
// -----------------------------------------------------------------------------------
//

/// Undefined Constructor
	NOALIAS __forceinline C_QuadraticResult::C_QuadraticResult( const _UNDEFINED /*undefined*/ ) : m_T0(ZERO_INIT), m_T1(ZERO_INIT)
{
}

/// T0 & T1 Constructor : Ensures T0 is near & T1 is far
NOALIAS __forceinline C_QuadraticResult::C_QuadraticResult( const HF32 PCREF t0, const HF32 PCREF t1 )
    : m_T0( Min( t0, t1 ) )
    , m_T1( Max( t0, t1 ) )
{
}

/// Accessor: Get result T0
NOALIAS __forceinline const HF32 C_QuadraticResult::T0() const __restrict
{
    return m_T0;
}

/// Accessor: Get result T1
NOALIAS __forceinline const HF32 C_QuadraticResult::T1() const __restrict
{
    return m_T1;
}

/// Returns true if T0 & T1 in valid range [ 0.0f, 1.0f ]
NOALIAS __forceinline const S32 C_QuadraticResult::Hit() const __restrict
{
    const HF32 zero(ZERO_INIT);
    const HF32 one( c_ScalarOne );
    
    return m_T1.GE( zero ) & m_T0.LE( one );
}

/// Returns true if T0 in valid range [ 0.0f, 1.0f ]
NOALIAS __forceinline const S32 C_QuadraticResult::ValidT0() const __restrict
{
    const HF32 zero(ZERO_INIT);
    const HF32 one( c_ScalarOne );
    
    return m_T0.GE( zero ) & m_T0.LE( one );
}

/// Returns true if T1 in valid range [ 0.0f, 1.0f ]
NOALIAS __forceinline const S32 C_QuadraticResult::ValidT1() const __restrict
{
    const HF32 zero(ZERO_INIT);
    const HF32 one( c_ScalarOne );
    
    return m_T1.GE( zero ) & m_T1.LE( one );
}




/// Quadratic Equation Solver
NOALIAS __forceinline const C_QuadraticResult SolveQuadratic( const HF32 PCREF a, const HF32 PCREF b, const HF32 PCREF c )
{
    // http://mathworld.wolfram.com/QuadraticFormula.html
    
    #if defined( _VMX128_ )
    
    const __vector4 va( a.m_F );
    const __vector4 vb( b.m_F );
    const __vector4 vc( c.m_F );
    
    const __vector4 zero( __vzero() );
    const __vector4 one( c_ScalarOne );
    const __vector4 four( c_ScalarFour );
    const __vector4 half( c_VectorHalf );
    const __vector4 abs_mask( c_VectorAbsMask );
    const __vector4 sign_mask( c_VectorSignMask );
    const __vector4 flt_min( c_VectorFltMin );
    const __vector4 flt_max( c_VectorFltMax );
    const __vector4 nearly_zero( flt_min );
    const __vector4 error( __vxor( flt_max, sign_mask ) );
    
    const __vector4 two_va( __vaddfp( va, va ) );
    const __vector4 b_squared_minus_four_a_c( __vnmsubfp( __vmulfp( four, va ), vc, __vmulfp( vb, vb ) ) );
    const __vector4 abs_va( __vand( va, abs_mask ) );
    
    const __vector4 ok( __vand( __vcmpgefp( b_squared_minus_four_a_c, zero ), __vcmpgtfp( abs_va, nearly_zero ) ) );
    
    const __vector4 rcp2a_B( two_va );
    const __vector4 root_B( b_squared_minus_four_a_c );
    
    const __vector4 rcp2a_y0( __vrefp( rcp2a_B ) );
    const __vector4 root_y0( __vrsqrtefp( root_B ) );
    
    const __vector4 rcp2a_abs_B( __vand( rcp2a_B, abs_mask ) );
    const __vector4 rcp2a_abs_B_GT_flt_min( __vcmpgtfp( rcp2a_abs_B, flt_min ) );
    const __vector4 rcp2a_abs_B_LT_flt_max( __vcmpgtfp( flt_max, rcp2a_abs_B ) );
    const __vector4 rcp2a_valid_B( __vand( rcp2a_abs_B_GT_flt_min, rcp2a_abs_B_LT_flt_max ) );
    
    const __vector4 root_B_GT_flt_min( __vcmpgtfp( root_B, flt_min ) );
    const __vector4 root_B_LT_flt_max( __vcmpgtfp( flt_max, root_B ) );
    const __vector4 root_valid_B( __vand( root_B_GT_flt_min, root_B_LT_flt_max ) );
    
    const __vector4 rcp2a_t( __vnmsubfp( rcp2a_y0, rcp2a_B, one ) );
    
    const __vector4 root_t0( __vmulfp( root_y0, root_y0 ) );
    const __vector4 root_t1( __vmulfp( root_y0, half ) );
    const __vector4 root_t2( __vnmsubfp( root_B, root_t0, one ) );
    
    const __vector4 rcp2a_y1( __vmaddfp( rcp2a_y0, rcp2a_t, rcp2a_y0 ) );
    const __vector4 root_y1( __vmaddfp( root_t2, root_t1, root_y0 ) );
    
    const __vector4 rcp2a( __vsel( zero, rcp2a_y1, rcp2a_valid_B ) );
    const __vector4 root( __vmulfp( b_squared_minus_four_a_c, __vsel( zero, root_y1, root_valid_B ) ) );
    
    const __vector4 minus_root( __vxor( root, sign_mask ) );
    
    const __vector4 t0( __vmulfp( __vsubfp( root, vb ), rcp2a ) );
    const __vector4 t1( __vmulfp( __vsubfp( minus_root, vb ), rcp2a ) );
    
    const __vector4 result_t0( __vsel( error, t0, ok ) );
    const __vector4 result_t1( __vsel( error, t1, ok ) );
    
    C_QuadraticResult result( UNDEFINED );
    result.m_T0.m_F = __vminfp( result_t0, result_t1 );
    result.m_T1.m_F = __vmaxfp( result_t0, result_t1 );
    return result;
    
    #else
    
    const HF32 zero( Constants<HF32>::Zero() );
    const HF32 one( Constants<HF32>::One() );
    const HF32 four( Constants<HF32>::Four() );
    const HF32 nearlyZero( Constants<HF32>::Min() );
    const HF32 error( -Constants<HF32>::Max() );
    const HF32 square( ( b * b ) - ( four * a * c ) );
    
    const Scalar_Bool square_GE_zero( square.GE( zero ) );
    const Scalar_Bool abs_a_GT_nearlyZero( Abs( a ).GT( nearlyZero ) );
    //const Scalar_Bool quadratic_valid( square_GE_zero & abs_a_GT_nearlyZero );
    Scalar_Bool quadratic_valid = square_GE_zero & abs_a_GT_nearlyZero;
    
    //const HF32 root( Sqrt( Select( square_GE_zero, square, zero ) ) );
    //const HF32 rcp2a( Rcp( Select( abs_a_GT_nearlyZero, a + a, one ) ) );
    const HF32 root( Sqrt( square ) );
    const HF32 rcp2a( Reciprocal( a + a ) );
    
    const HF32 t0( (  root - b ) * rcp2a );
    const HF32 t1( ( -root - b ) * rcp2a );
    
    return C_QuadraticResult( Select( quadratic_valid, t0, error ),
                              Select( quadratic_valid, t1, error ) );
    
    #endif
}



/// Real Cubic Root: result = a ^ 1/3
NOALIAS __forceinline const HF32 RealCubeRoot( const HF32 PCREF a )
{
    // TODO: OPTIMISE THIS!
    // http://mathworld.wolfram.com/CubeRoot.html
    // X[n] = 1/3.( a/(X[n-1]^2) + 2.X[n-1] )
    return powf( (F32)a, 1.0f / 3.0f );
}




//
// ---------------------------------------------------------------------
// |-- C_Plane : Plane class stored as position & distance to origin --|
// ---------------------------------------------------------------------
//

NOALIAS __forceinline const HVec4 InitPlane( const C_Normal PCREF normal, const C_Vertex PCREF position )
{
    return HVec4( normal ) - HVecW( normal | ( position - C_Origin() ) );
}

NOALIAS __forceinline const HVec4 InitPlane( const C_Vertex PCREF A, const C_Vertex PCREF B, const C_Vertex PCREF C )
{
    const C_Normal normal( ( Normalise(( B - A ) ^ ( C - A )) ) );
    return HVec4( normal ) - HVecW( normal | ( A - C_Origin() ) );
}

/// Default Constructor ( Zero normal & zero distance to origin! )
NOALIAS __forceinline C_Plane::C_Plane()
{
}

// Zero Init constructor.
NOALIAS __forceinline C_Plane::C_Plane( _ZERO_INIT )
    : m_V( ZERO_INIT )
{
}

// Undefined constructor.
NOALIAS __forceinline C_Plane::C_Plane( _UNDEFINED )
    : m_V( UNDEFINED )
{
}

/// Copy Constructor
NOALIAS __forceinline C_Plane::C_Plane( const C_Plane& copy )
    : m_V( copy.m_V )
{
}

/// Explicit HVec4 Constructor
NOALIAS __forceinline C_Plane::C_Plane( const HVec4 PCREF src )
    : m_V( src )
{
}

/// Normal & Vertex Constructor
NOALIAS __forceinline C_Plane::C_Plane( const C_Normal PCREF normal, const C_Vertex PCREF position )
    : m_V( InitPlane( normal, position ) )
{
}

/// Vertex Triple Constructor
NOALIAS __forceinline C_Plane::C_Plane( const C_Vertex PCREF A, const C_Vertex PCREF B, const C_Vertex PCREF C )
    : m_V( InitPlane( A, B, C ) )
{
}

/// Cast to HVec4
NOALIAS __forceinline C_Plane::operator const HVec4 () const __restrict
{
    return m_V;
}

/// Copy Assignment
NOALIAS __forceinline const C_Plane& C_Plane::operator = ( const C_Plane& copy )
{
    m_V = copy.m_V;
    return copy;
}

/// Accessor: Get Normal
NOALIAS __forceinline const C_Normal C_Plane::Normal() const __restrict
{
    //#if !defined( PLATFORM_PS3 ) && !defined( PLATFORM_SPU )
    return C_Normal( HVec3( m_V ), C_Normal::UNIT_LENGTH_VECTOR );
    //#else
	//HVec3 temp( m_V );
    //return *(C_Normal*)&temp; // Uses C_Normal& cast to avoid additional normalise op!
    //#endif
}

/// Accessor: Get distance to origin
NOALIAS __forceinline const HF32 C_Plane::DistanceToOrigin() const __restrict
{
    //return HF32( HVecW( m_V ).W() );
    return m_V.W();
}

/// Distance from the Plane to the input Vertex
NOALIAS __forceinline const HF32 C_Plane::Distance( const C_Vertex PCREF v ) const __restrict
{
    #if defined( _VMX128_ )
    
    const __vector4 one( c_VectorOne );
    const __vector4 plane( m_V );
    const __vector4 vertex( v.m_V );
    const __vector4 vertex_xyz1( __vrlimi( vertex, one, VMX_MASK_INSERT_W, 0 ) );
    const __vector4 distance( __vmsum4fp( vertex_xyz1, plane ) );
    
    HF32 result( UNDEFINED );
    result.m_F = distance;
    return result;
    
    #elif defined( _SIMD_ )
//      #ifndef PLATFORM_SPU
    return ( m_V * ( HVec3( v ) || HVecW( c_UnitVectorW ) ) ).Sum();
//  	#else
//  	return ( m_V * ( HVec3( v ) || HVecW( HVec4(0.0f, 0.0f, 0.0f, 1.0f) ) ) ).Sum();
//     #endif // PLATFORM_SPU
	#else
    return ( m_V * ( HVec3( v ) || HVecW( 1.0f ) ) ).Sum();
    #endif
}

/// Unary Negation: Negate plane normal & distance from origin - plane still divides same space
NOALIAS __forceinline const C_Plane C_Plane::operator - () const __restrict
{
    return C_Plane( -m_V );
}

/// Returns copy of plane shifted along its normal by the given distance
NOALIAS __forceinline const C_Plane C_Plane::operator + ( const HF32 PCREF distance ) const __restrict
{
    return C_Plane( m_V - HVecW( distance ) );
}

/// Returns copy of plane shifted back along its normal by the given distance
NOALIAS __forceinline const C_Plane C_Plane::operator - ( const HF32 PCREF distance ) const __restrict
{
    return C_Plane( m_V + HVecW( distance ) );
}

/// Shifts the plane along its normal by the given distance
NOALIAS __forceinline void C_Plane::operator += ( const HF32 PCREF distance ) __restrict
{
    m_V = m_V - HVecW( distance );
}

/// Shifts the plane back along its normal by the given distance
NOALIAS __forceinline void C_Plane::operator -= ( const HF32 PCREF distance ) __restrict
{
    m_V = m_V + HVecW( distance );
}


#define MATHS_PLANE_X_MAT43_USE_RAW_INTRINSICS

/// Returns copy of plane rotated & translated by the given Matrix
NOALIAS __forceinline const C_Plane C_Plane::operator * ( const HMat43& m ) const __restrict
{
    #if defined( _VMX128_ ) && defined( MATHS_PLANE_X_MAT43_USE_RAW_INTRINSICS )
    
    const __vector4 zero( __vzero() );
    const __vector4 one( __vupkd3d( zero, VPACK_D3DCOLOR ) );
    const __vector4 sign( c_Vector4_SignMask );
    
    const __vector4 srcPlane( m_V.m_V );
    
    const __vector4 matrix_column_0( m.m_CX );
    const __vector4 matrix_column_1( m.m_CY );
    const __vector4 matrix_column_2( m.m_CZ );
    
    const __vector4 srcPlaneMinusDistance( __vxor( __vspltw( srcPlane, 3 ), sign ) );
    
    const __vector4 srcPlanePoint_temp( __vmulfp( srcPlane, srcPlaneMinusDistance ) );
    
    const __vector4 dstPlaneNormal_x( __vmsum3fp( srcPlane, matrix_column_0 ) );
    const __vector4 dstPlaneNormal_y( __vmsum3fp( srcPlane, matrix_column_1 ) );
    const __vector4 dstPlaneNormal_z( __vmsum3fp( srcPlane, matrix_column_2 ) );
    
    const __vector4 srcPlanePoint( __vrlimi( srcPlanePoint_temp, one, VMX_MASK_INSERT_W, 0 ) );
    
    const __vector4 dstPlanePoint_x( __vmsum4fp( srcPlanePoint, matrix_column_0 ) );
    const __vector4 dstPlanePoint_y( __vmsum4fp( srcPlanePoint, matrix_column_1 ) );
    const __vector4 dstPlanePoint_z( __vmsum4fp( srcPlanePoint, matrix_column_2 ) );
    
    const __vector4 dstPlaneNormal( __vmrghw( __vmrghw( dstPlaneNormal_x, dstPlaneNormal_z ), dstPlaneNormal_y ) );
    
    const __vector4 dstPlanePoint( __vmrghw( __vmrghw( dstPlanePoint_x, dstPlanePoint_z ), dstPlanePoint_y ) );
    
    const __vector4 dstPlaneDistance( __vxor( __vmsum3fp( dstPlaneNormal, dstPlanePoint ), sign ) );
    
    const __vector4 dstPlane( __vrlimi( dstPlaneNormal, dstPlaneDistance, VMX_MASK_INSERT_W, 0 ) );
    
    return C_Plane( dstPlane );
    
    #else
    
    const C_Normal normal( Normal() );
    const C_Vertex vertex( C_Origin() + ( normal * -DistanceToOrigin() ) );
    //return C_Plane( normal * m.Rotation(), vertex * m );
    
	//#if !defined( PLATFORM_PS3 ) && !defined( PLATFORM_SPU )
    return C_Plane( C_Normal( normal * m.Rotation(), C_Normal::UNIT_LENGTH_VECTOR ), vertex * m ); // NOTE: Hacky cast!
    //#else
	//HVec3 temp = normal * m.Rotation();
    //return C_Plane( *(C_Normal*)&temp, vertex * m ); // NOTE: Hacky cast!
    //#endif
    
    #endif
}

/// Returns copy of plane rotated & translated by the given Matrix
NOALIAS __forceinline const C_Plane C_Plane::operator * ( const HMat44& m ) const __restrict
{
    // D3DXPlaneTransform(Out, P, M)
    // Q = P / |P|
    // u = (Qa, Qb, Qc, 0)
    // D = Qd
    // A = (-Dux, -Duy, -Duz, 1)
    // B = A×M
    // v = u×M
    // q = 1/|v|
    // Outa = qvx
    // Outb = qvy
    // Outc = qvz
    // Outd = -qv | B 
    
    //const HMat44 mit( m.Inverse().Transposed() );
    //const C_Normal normal( Normal() );
    //const C_Vertex point( C_Origin() + ( normal * -DistanceToOrigin() ) );
    //const HVec4 tn( ( HVec3( normal ) || Constants<HF32>::Zero() ) * mit );
    //const HVec4 tp( ( HVec3( point ) || Constants<HF32>::One() ) * m ); // use mit instead?
    //return C_Plane( C_Normal( HVec3( tn ) /* / tn.W() */ ), // Divide by W pointless since we renormalise afterwards anyhow!
    //                C_Vertex( tp ) / tp.W() );
    
    const HMat44 M( m.Inverse().Transposed() );
    
    // Q = P/|P| : NOT necessary, plane is already normalised!
    
    // u = (Qa, Qb, Qc, 0)
    const HVec4 u( Normal() );
    
    // D = Qd
    const HF32 D( DistanceToOrigin() );
    
    // A = (-Dux, -Duy, -Duz, 1)
    const HVec4 A( ( HVec3(u) * -D ) || Constants<HF32>::One() );
    
    // B = A×M
    const HVec4 B( A * M );
    
    // v = u×M
    const HVec4 v( u * M );
    
    // q = 1/|v|
    const HF32 q( v.RcpLength() );
    
    // Outa = qvx
    // Outb = qvy
    // Outc = qvz
    // Outd = -qv | B
    C_Plane result( UNDEFINED );
    //result.m_V = HVec4( v.X() * q,
    //                    v.Y() * q,
    //                    v.Z() * q,
    //                    ( v | B ) * -q );
    result.m_V = ( HVec3(v) * q ) ||
                 ( ( v | B ) * -q );
    return result;
}

/// Determines if the given Line intersects this Plane, returns the t value of the line intersect point
NOALIAS __forceinline const HF32 C_Plane::Intersect( const C_Line& line ) const __restrict
{
    const C_Normal N( Normal() );
    const C_Vertex P( N * -DistanceToOrigin() );
    const HF32     denom( N | line.Vector() );
    const HF32     nearlyZero( c_ScalarFltMin );
    const HF32     errorResult( c_ScalarFltMaxNeg );
    
    //if ( denom.Abs() > nearlyZero )
    //{
    //    return ( N | ( P - line.Start() ) ) / denom;
    //}
    //else
    //{
    //    return -FLT_MAX;
    //}
    
    const HF32 num( N | ( P - line.Start() ) );
    const HF32 rcpDenom( Reciprocal( denom ) );
    const HF32 result( num * rcpDenom );
    
    return Select( Abs( denom ).GT( nearlyZero ), result, errorResult );
}

/// Determines if the given Plane is Co-Planar with this Plane, within the given tolerances
NOALIAS __forceinline const bool C_Plane::CoPlanar( const C_Plane PCREF src, const HF32 PCREF normalTolerance, const HF32 PCREF offsetTolerance ) const __restrict
{
    return ( ( Normal() | src.Normal() ) > normalTolerance ) &&
           ( ( DistanceToOrigin() - src.DistanceToOrigin() ) < offsetTolerance );
}

/// Comparison: Equal
NOALIAS __forceinline const Vector4_Bool C_Plane::operator == ( const C_Plane PCREF src ) const __restrict
{
    return m_V == src.m_V;
}

/// Comparison: Not-Equal
NOALIAS __forceinline const Vector4_Bool C_Plane::operator != ( const C_Plane PCREF src ) const __restrict
{
    return m_V != src.m_V;
}

/// Returns a copy of the given Vertex, translated along this Planes normal such that it lies on this Plane
NOALIAS __forceinline const C_Vertex C_Plane::OnPlane( const C_Vertex PCREF vertex ) const __restrict
{
    return vertex - ( Normal() * Distance( vertex ) );
}

/// Returns a copy of the given Vector, with any component in the direction of this Planes normal cancelled out
NOALIAS __forceinline const C_Vector C_Plane::ParallelToPlane( const C_Vector PCREF vector ) const __restrict
{
    const C_Normal normal( Normal() );
    return vector - ( normal * ( vector | normal ) );
}


NOALIAS __forceinline void C_Plane::FlipNormal( ) __restrict
{
	m_V[0] = -m_V[0];
	m_V[1] = -m_V[1];
	m_V[2] = -m_V[2];
	return;
}

/// Read from Stream
template < typename T_Stream > __forceinline void C_Plane::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_V;
}

/// Write to Stream
template < typename T_Stream > __forceinline void C_Plane::Serialise( T_Stream& stream ) __restrict
{
    stream << m_V;
}    







//
// ------------------------------------------------------------
// |-- C_Line : Line class stored as start, normal & length --|
// ------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_Line::C_Line()
{
}

/// Constructor: Vertex Start, Normal & Length to End
NOALIAS __forceinline C_Line::C_Line( const C_Vertex PCREF start, const C_Normal PCREF normal, const HF32 PCREF length ) :
    m_StartRcpLength( start, length.Reciprocal() ),
    m_NormalLength( normal, length )
{
}

/// Constructor: Vertex Start, Normal & Length to End
NOALIAS __forceinline C_Line::C_Line( const C_Vertex PCREF start, const C_PackedNormalLength PCREF normalLength ) :
    m_StartRcpLength( start, normalLength.Length().Reciprocal() ),
    m_NormalLength( normalLength )
{
}

/// Constructor: Vertex Start & Vertex End
NOALIAS __forceinline C_Line::C_Line( const C_Vertex PCREF start, const C_Vertex PCREF end )
    #if !defined( _VMX128_ )
    : m_StartRcpLength( start, ( end - start ).Length().Reciprocal() ),
      m_NormalLength( end - start )
    #endif
{
    #if defined( _VMX128_ )
    
    const __vector4 zero( c_VectorZero );
    const __vector4 one( c_ScalarOne );
    const __vector4 half( c_ScalarHalf );
    const __vector4 fltmin( c_ScalarFltMin );
    const __vector4 s( start.m_V.m_V );
    const __vector4 e( end.m_V.m_V );
    const __vector4 se( __vsubfp( e, s ) );
    const __vector4 l_squared( __vmsum3fp( se, se ) );
    const __vector4 rcp_l_est( __vrsqrtefp( l_squared ) );
    const __vector4 rcp_l( __vmaddfp( __vnmsubfp( l_squared, __vmulfp( rcp_l_est, rcp_l_est ), one ), __vmulfp( rcp_l_est, half ), rcp_l_est ) );
    const __vector4 l( __vsel( zero, __vmulfp( l_squared, rcp_l ), __vcmpgefp( l_squared, fltmin ) ) );
    const __vector4 n( __vmulfp( se, rcp_l ) );
    
    m_StartRcpLength.m_VertexRcpLength.m_V = __vrlimi( s, rcp_l, VMX_MASK_INSERT_W, 0 );
    m_NormalLength.m_NormalLength.m_V = __vrlimi( n, l, VMX_MASK_INSERT_W, 0 );
    
    #endif
}

/// Constructor: Vertex Start & Vector to End
NOALIAS __forceinline C_Line::C_Line( const C_Vertex PCREF start, const C_Vector PCREF vector )
    : m_StartRcpLength( start, vector.Length().Reciprocal() )
    , m_NormalLength( vector )
{
}

/// Accessor: Get Start
NOALIAS __forceinline const C_Vertex C_Line::Start() const __restrict
{
    return m_StartRcpLength.Vertex();
}

/// Accessor: Get End
NOALIAS __forceinline const C_Vertex C_Line::End() const __restrict
{
    return Start() + Vector();
}

/// Accessor: Get Normal
NOALIAS __forceinline const C_Normal C_Line::Normal() const __restrict
{
    return m_NormalLength.Normal();
}

/// Accessor: Get Vector
NOALIAS __forceinline const C_Vector C_Line::Vector() const __restrict
{
    return Normal() * Length();
}

/// Accessor: Get Length
NOALIAS __forceinline const HF32 C_Line::Length() const __restrict
{
    return m_NormalLength.Length();
}

/// Accessor: Get Reciprocal Length
NOALIAS __forceinline const HF32 C_Line::ReciprocalLength() const __restrict
{
    return m_StartRcpLength.RcpLength();
}

/// Set Length
NOALIAS __forceinline void C_Line::SetLength( const HF32 PCREF length ) __restrict
{
    m_StartRcpLength = C_VertexRcpLength( Start(), length.Reciprocal() );
    m_NormalLength   = C_PackedNormalLength( Normal(), length );
}

/// Calculate point on Line from T value
NOALIAS __forceinline const C_Vertex C_Line::Point( const HF32 PCREF t ) const __restrict
{
    return Start() + ( Vector() * t );
}

/// Calculate point on Line from T value
NOALIAS __forceinline const C_Vertex C_Line::operator () ( const HF32 PCREF t ) const __restrict
{
    return Start() + ( Vector() * t );
}

/// Intersection of two lines: Returns T value of intersection point on this line
NOALIAS __forceinline const HF32 C_Line::Intersect( const C_Line& line2 ) const __restrict
{
    const HF32 divideByZeroResult( c_ScalarFltMaxNeg );
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 one( c_ScalarOne );
    
    const C_Line& line1 = *this;
    
    // http://mathworld.wolfram.com/Line-LineIntersection.html
    // Line 1 : x = x1 + ( x2 - x1 ) s
    // Line 2 : x = x3 + ( x4 - x3 ) t
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // s = ( ( c x b ) . ( a x b ) ) / ( | a x b | ^2 )
    //   = ( ( c x b ) . ( a x b ) ) / ( ( a x b ) . ( a x b ) )
    
    const C_Vector a( line1.Vector() );
    const C_Vector b( line2.Vector() );
    const C_Vector c( line2.Start() - line1.Start() );
    const C_Vector cxb( c ^ b );
    const C_Vector axb( a ^ b );
    
    const HF32 denom( axb | axb );
    
    #if defined( _DEBUG )
    const HF32 s( ( cxb | axb ) / Select( denom.GT( nearlyZero ), denom, one ) );
    #else
    const HF32 s( ( cxb | axb ) / denom );
    #endif
    
    return Select( denom.GT( nearlyZero ), s, divideByZeroResult );
}

/// Returns the distance between the closest points on the lines
NOALIAS __forceinline const HF32 C_Line::Distance( const C_Line& line2 ) const __restrict
{
    const HF32 divideByZeroResult( c_ScalarFltMaxNeg );
    const HF32 nearlyZero( c_ScalarFltMin );
    const HF32 one( c_ScalarOne );
    
    const C_Line& line1 = *this;
    
    // http://mathworld.wolfram.com/Line-LineDistance.html
    // Line 1 : x1 + ( x2 - x1 ) s
    // Line 2 : x3 + ( x4 - x3 ) t
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // d = | c . ( a x b ) | / | a x b |
    
    const C_Vector a( line1.Normal() );
    const C_Vector b( line2.Normal() );
    const C_Vector c( line2.Start() - line1.Start() );
    const C_Vector axb( a ^ b );
    
    const HF32 denom2( axb.LengthSquared() );
    
    #if defined( _DEBUG )
    const HF32 d( Abs( c | axb ) * Select( denom2.GT( nearlyZero ), denom2, one ).ReciprocalSqrt() );
    #else
    const HF32 d( Abs( c | axb ) * denom2.ReciprocalSqrt() );
    #endif
    
    return Select( denom2.GT( nearlyZero ), d, divideByZeroResult );
}

/// Returns T value of closest point on this Line to the given point
NOALIAS __forceinline const HF32 C_Line::ClosestT( const C_Vertex PCREF point ) const __restrict
{
    // Point : x0
    // Line : x1 + ( x2 - x1 ) t
    // Closest t = - ( x1 - x0 ) . ( x2 - x1 ) / | x2 - x1 | ^2
    //           = - ( x1 - x0 ) . ( x2 - x1 ) / ( x2 - x1 ) . ( x2 - x1 )
    
    return ( ( point - Start() ) | Normal() ) * ReciprocalLength();
}

/// Returns the distance squared between the given point and the closest point to it on the line
NOALIAS __forceinline const HF32 C_Line::DistanceSquared( const C_Vertex PCREF point ) const __restrict
{
    // Point : x0
    // Line : x1 + ( x2 - x1 ) t
    // Minimum d^2 = | ( x2 - x1 ) x ( x1 - x0 ) | ^2 / | x2 - x1 | ^2
    // a = (x2-x1) x (x1-x0)
    // b = x2-x1
    // d^2 = a.a / b.b
    
    //const C_Vector vector( Vector() );
    //
    //const HF32 denom( vector | vector );
    //if ( denom.IsNonZero() )
    //{
    //    const C_Vector a( vector ^ ( m_Start - point ) );
    //    const HF32     num( a | a );
    //    return num / denom;
    //}
    //return FLT_MAX;
    
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
    
    const C_Vector dVec( Normal() ^ ( point - Start() ) );
    return dVec | dVec;
}

/// Returns the distance between the given point and the closest point to it on the line
NOALIAS __forceinline const HF32 C_Line::Distance( const C_Vertex PCREF point ) const __restrict
{
    return Sqrt( DistanceSquared( point ) );
}

/// Returns t-value of closest point on line, closest point on line, normal from closest point on line to point & distance from closest point on line to point
NOALIAS __forceinline const HF32 C_Line::ClosestPoint( const C_Vertex PCREF point, C_Vertex& out_closest_point, C_Normal& out_normal, HF32& out_distance ) const __restrict
{
    // Point : p
    // Line : x1 + ( x2 - x1 ) t
    // Closest t = ( p - x1 ) . ( x2 - x1 ) / | x2 - x1 | ^2
    //           = ( p - x1 ) . ( x2 - x1 ) / ( x2 - x1 ) . ( x2 - x1 )
    
    //using namespace Maths;
    
    #if defined( _VMX128_ )
    
    const __vector4 p( (HVec3)point );
    const __vector4 x1( (HVec3)Start() );
    const __vector4 x1x2( (HVec3)Normal() );
    const __vector4 x1p( __vsubfp( p, x1 ) );
    
    const __vector4 x1p_yzx( __vpermwi( x1p, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 x1p_zxy( __vpermwi( x1p, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 x1x2_yzx( __vpermwi( x1x2, VPERMWI_CONST( VY, VZ, VX, VW ) ) );
    const __vector4 x1x2_zxy( __vpermwi( x1x2, VPERMWI_CONST( VZ, VX, VY, VW ) ) );
    const __vector4 x1p_cross_x1x2( __vnmsubfp( x1p_yzx, x1x2_zxy, __vmulfp( x1p_zxy, x1x2_yzx ) ) );
    
    const __vector4 x1p_dot_x1x2( __vmsum3fp( x1p, x1x2 ) );
    
    const __vector4 zero( __vzero() );
    const __vector4 half( c_VectorHalf );
    const __vector4 one( c_VectorOne );
    const __vector4 fltMax( c_VectorFltMax );
    
    const __vector4 x1p_cross_x1x2_length2( __vmsum3fp( x1p_cross_x1x2, x1p_cross_x1x2 ) );
    
    const __vector4 closest_p( __vaddfp( x1, __vmulfp( x1x2, x1p_dot_x1x2 ) ) );
    
    const __vector4 B( x1p_cross_x1x2_length2 ); // Calculate A / B : A == 1.0f, B == x1p_cross_x1x2_length2
    const __vector4 y0( __vrsqrtefp( B ) );
    const __vector4 t0( __vmulfp( y0, y0 ) );
    const __vector4 t1( __vmulfp( y0, half ) );
    const __vector4 t2( __vnmsubfp( B, t0, one ) );
    const __vector4 y1( __vmaddfp( t2, t1, y0 ) );
    const __vector4 B_GT_zero( __vcmpgtfp( B, zero ) );
    const __vector4 B_LT_fltMax( __vcmpgtfp( fltMax, B ) );
    const __vector4 valid( __vand( B_GT_zero, B_LT_fltMax ) );
    const __vector4 x1p_cross_x1x2_rcp_length( __vsel( y0, y1, valid ) );
    
    const __vector4 x1p_cross_x1x2_length( __vsel( zero, __vmulfp( x1p_cross_x1x2_length2, x1p_cross_x1x2_rcp_length ), valid ) );
    
    const __vector4 normal( __vmulfp( __vsubfp( p, closest_p ), x1p_cross_x1x2_rcp_length ) );
    
    out_closest_point = C_Vertex( HVec3( closest_p ) );
    out_normal        = C_Normal( HVec3( normal ), C_Normal::UNIT_LENGTH_VECTOR );
    out_distance      = HF32( x1p_cross_x1x2_length );
    return HF32( x1p_dot_x1x2 );
    
    #else
    
    const C_Vertex x1( Start() );
    const C_Normal x1x2( Normal() );
    const C_Vector x1p( point - x1 );
    const C_Vector x1p_cross_x1x2( x1p ^ x1x2 );
    const HF32     x1p_dot_x1x2( x1p | x1x2 );
    
    C_Normal normal_x1p_cross_x1x2( UNDEFINED );
    HF32     length_x1p_cross_x1x2( UNDEFINED );
    HF32     length2_x1p_cross_x1x2( UNDEFINED );
    HF32     rcp_length_x1p_cross_x1x2( UNDEFINED );
    NormalLength( x1p_cross_x1x2, normal_x1p_cross_x1x2, length_x1p_cross_x1x2, length2_x1p_cross_x1x2, rcp_length_x1p_cross_x1x2 );
    
    const HF32     t( x1p_dot_x1x2 );
    const HF32     distance( length_x1p_cross_x1x2 );
    const C_Vertex closest_point( x1 + ( x1x2 * t ) );
    const C_Normal normal( ( point - closest_point ) * rcp_length_x1p_cross_x1x2, C_Normal::UNIT_LENGTH_VECTOR );
    
    out_closest_point = closest_point;
    out_normal        = normal;
    out_distance      = distance;
    return t;
    
    #endif
}

/// Read from Stream
template < typename T_Stream >
__forceinline void C_Line::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_StartRcpLength;
    stream >> m_NormalLength;
}

/// Write to Stream
template < typename T_Stream >
__forceinline void C_Line::Serialise( T_Stream& stream ) __restrict
{
	stream << m_StartRcpLength;
    stream << m_NormalLength;
}    







//
// ------------------------------------------------------------------------------
// |-- C_Sphere : Sphere class stored as a HVec4 with centre XYZ and radius W --|
// ------------------------------------------------------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_Sphere::C_Sphere()
{
}

#if defined( _SIMD_ )
/// Explicit V128 Constructor
NOALIAS __forceinline C_Sphere::C_Sphere( const V128 PCREF src )
    : m_V( src )
{
}
#endif

/// Copy Constructor
NOALIAS __forceinline C_Sphere::C_Sphere( const C_Sphere& copy )
    : m_V( copy.m_V )
{
}

/// Constructor: HF32 XYZ & Radius
NOALIAS __forceinline C_Sphere::C_Sphere( const HF32 PCREF x, const HF32 PCREF y, const HF32 PCREF z, const HF32 PCREF r )
    : m_V( x, y, z, r )
{
}

/// Constructor: Vertex & Radius
NOALIAS __forceinline C_Sphere::C_Sphere( const C_Vertex PCREF centre, const HF32 PCREF radius )
    : m_V( HVec3( centre ) || HVecW( radius ) )
{
}

/// Accessor: Returns the centre as a Vertex
NOALIAS __forceinline const C_Vertex C_Sphere::Centre() const __restrict
{
    return C_Vertex( m_V );
}

/// Accessor: Returns the radius as a HF32
NOALIAS __forceinline const HF32 C_Sphere::Radius() const __restrict
{
    return HF32( HVecW( m_V ).W() );
}

/// Line-Sphere Intersection
NOALIAS __forceinline const C_QuadraticResult C_Sphere::Intersect( const C_Line& line ) const __restrict
{
    // Solve Sphere-Line intersection using the Quadratic formula
    
    const C_Vertex P( Centre() );
    const HF32     r( Radius() );
    const HF32     r2( r * r );
    
    //if ( line.DistanceSquared( P ) > r2 )
    //{
    //    return C_QuadraticResult( -FLT_MAX, -FLT_MAX );
    //}
    
    const C_Vertex Q( line.Start() );
    const C_Vector v( line.Vector() );
    const C_Vector pq( Q - P );
    const HF32     two( c_ScalarTwo );
    const HF32     a( v | v );
    const HF32     b( two * ( v | pq ) );
    const HF32     c( ( pq | pq ) - r2 );
    
    return SolveQuadratic( a, b, c );
}

/// Point inside sphere
NOALIAS __forceinline const S32 C_Sphere::Inside( const C_Vertex PCREF point ) const __restrict
{
    const HF32 d2( Centre().DistanceSquared( point ) );
    const HF32 r( Radius() );
    return d2 <= ( r * r );
}

/// Distance from sphere
NOALIAS __forceinline const HF32 C_Sphere::Distance( const C_Vertex PCREF point ) const __restrict
{
    return Centre().Distance( point) - Radius();
}

/// Sphere-Sphere Distance
NOALIAS __forceinline const HF32 C_Sphere::Distance( const C_Sphere PCREF sphere ) const __restrict
{
    const HF32 d( Centre().Distance( sphere.Centre() ) );
    const HF32 rSum( Radius() + sphere.Radius() );
    return d - rSum;
}

/// Sphere-Sphere Intersection
NOALIAS __forceinline const Scalar_Bool C_Sphere::Intersect( const C_Sphere PCREF src ) const __restrict
{
    const HF32 d2( Centre().DistanceSquared( src.Centre() ) );
    const HF32 rSum( Radius() + src.Radius() );
    return d2.LT( rSum * rSum );
}

/// Sphere * Rotation + Translation Matrix (Unit Axes assumed!)
NOALIAS __forceinline const C_Sphere C_Sphere::operator * ( const HMat43& m ) const __restrict
{
    return C_Sphere( Centre() * m, Radius() );
}

/// Check if two spheres are equal to some tolerance.
NOALIAS __forceinline const bool C_Sphere::operator == (const C_Sphere& rkSphere) const __restrict
{
	return (Centre().DistanceSquared(rkSphere.Centre()) < 0.001f) && (Radius() - rkSphere.Radius() < 0.001f);
}

/// Check if two spheres are equal to some tolerance.
NOALIAS __forceinline const bool C_Sphere::operator != (const C_Sphere& rkSphere) const __restrict
{
	return !(operator == (rkSphere));
}

NOALIAS __forceinline const C_Sphere C_Sphere::Transformed( const C_Quaternion PCREF rotation, const C_Vector PCREF translation ) const __restrict
{
	return C_Sphere( Centre().Transformed( rotation, translation ), Radius() );
}

/// Finite check
NOALIAS __forceinline const Scalar_Bool C_Sphere::Finite() const __restrict
{
    return HVec4( m_V ).Finite();
}

/// Read from Stream
template < typename T_Stream > __forceinline void C_Sphere::Deserialise( T_Stream& stream ) __restrict
{
    HVec4 vec;
    stream >> vec;
	#if (defined (PLATFORM_PC) && !defined(_SIMD_))
    m_V.m_V[0] = vec.m_V[0];
    m_V.m_V[1] = vec.m_V[1];
    m_V.m_V[2] = vec.m_V[2];
    m_V.m_V[3] = vec.m_V[3];
    #else
	m_V.m_V = vec.m_V;
    #endif
}

/// Write to Stream
template < typename T_Stream > __forceinline void C_Sphere::Serialise( T_Stream& stream ) __restrict
{
   HVec4 vec(m_V);
   stream << vec;
}    





//
// -----------------------------------
// |-- C_Cylinder : Cylinder class --|
// -----------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_Cylinder::C_Cylinder()
{
}

/// Start + End + Radius Constructor
NOALIAS __forceinline C_Cylinder::C_Cylinder( const C_Vertex PCREF start, const C_Vertex PCREF end, const HF32 PCREF radius ) :
    m_StartRadius( start, radius ),
    m_NormalLength( end - start )
{
}

/// Start + Vector + Radius Constructor
NOALIAS __forceinline C_Cylinder::C_Cylinder( const C_Vertex PCREF start, const C_Vector PCREF vector, const HF32 PCREF radius ) :
    m_StartRadius( start, radius ),
    m_NormalLength( vector )
{
}

/// Start + Normal + Length + Radius Constructor
NOALIAS __forceinline C_Cylinder::C_Cylinder( const C_Vertex PCREF start, const C_Normal PCREF normal, const HF32 PCREF length, const HF32 PCREF radius ) :
    m_StartRadius( start, radius ),
    m_NormalLength( normal, length )
{
}

/// Accessor: Get Start
NOALIAS __forceinline const C_Vertex C_Cylinder::Start() const __restrict
{
    return m_StartRadius.Centre();
}

/// Accessor: Get End
NOALIAS __forceinline const C_Vertex C_Cylinder::End() const __restrict
{
    return m_StartRadius.Centre() + m_NormalLength.Vector();
}

/// Accessor: Get Vector
NOALIAS __forceinline const C_Vector C_Cylinder::Vector() const __restrict
{
    return m_NormalLength.Vector();
}

/// Accessor: Get Normal
NOALIAS __forceinline const C_Normal C_Cylinder::Normal() const __restrict
{
    return m_NormalLength.Normal();
}

/// Accessor: Get Length
NOALIAS __forceinline const HF32 C_Cylinder::Length() const __restrict
{
    return m_NormalLength.Length();
}

/// Accessor: Get Radius
NOALIAS __forceinline const HF32 C_Cylinder::Radius() const __restrict
{
    return m_StartRadius.Radius();
}

/// Read from Stream
template < typename T_Stream > __forceinline void C_Cylinder::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_StartRadius;
    stream >> m_NormalLength;
}

/// Write to Stream
template < typename T_Stream > __forceinline void C_Cylinder::Serialise( T_Stream& stream ) __restrict
{
    stream << m_StartRadius;
    stream << m_NormalLength;
}






//
// ---------------------------------
// |-- C_Capsule : Capsule class --|
// ---------------------------------
//

/// Default Constructor
NOALIAS __forceinline C_Capsule::C_Capsule()
{
}

/// Start + End + Radius Constructor
NOALIAS __forceinline C_Capsule::C_Capsule( const C_Vertex PCREF start, const C_Vertex PCREF end, const HF32 PCREF radius ) :
    m_StartRadius( start, radius ),
    m_NormalLength( end - start )
{
}

/// Start + Vector + Radius Constructor
NOALIAS __forceinline C_Capsule::C_Capsule( const C_Vertex PCREF start, const C_Vector PCREF vector, const HF32 PCREF radius ) :
    m_StartRadius( start, radius ),
    m_NormalLength( vector )
{
}

/// Start + Normal + Length + Radius Constructor
NOALIAS __forceinline C_Capsule::C_Capsule( const C_Vertex PCREF start, const C_Normal PCREF normal, const HF32 PCREF length, const HF32 PCREF radius ) :
    m_StartRadius( start, radius ),
    m_NormalLength( normal, length )
{
}

/// Accessor: Get Start
NOALIAS __forceinline const C_Vertex C_Capsule::Start() const __restrict
{
    return m_StartRadius.Centre();
}

/// Accessor: Get End
NOALIAS __forceinline const C_Vertex C_Capsule::End() const __restrict
{
    return m_StartRadius.Centre() + m_NormalLength.Vector();
}

/// Accessor: Get Vector
NOALIAS __forceinline const C_Vector C_Capsule::Vector() const __restrict
{
    return m_NormalLength.Vector();
}

/// Accessor: Get Normal
NOALIAS __forceinline const C_Normal C_Capsule::Normal() const __restrict
{
    return m_NormalLength.Normal();
}

/// Accessor: Get Length
NOALIAS __forceinline const HF32 C_Capsule::Length() const __restrict
{
    return m_NormalLength.Length();
}

/// Accessor: Get Radius
NOALIAS __forceinline const HF32 C_Capsule::Radius() const __restrict
{
    return m_StartRadius.Radius();
}

/// Read from Stream
template < typename T_Stream > __forceinline void C_Capsule::Deserialise( T_Stream& stream ) __restrict
{
    stream >> m_StartRadius;
    stream >> m_NormalLength;
}

/// Write to Stream
template < typename T_Stream > __forceinline void C_Capsule::Serialise( T_Stream& stream ) __restrict
{
    stream << m_StartRadius;
    stream << m_NormalLength;
}    






//
// ---------------------------------------------------
// |-- C_AxisBox : Axis Aligned Bounding Box class --|
// ---------------------------------------------------
//

/// Construct from Box3 storage
NOALIAS __forceinline C_AxisBox::C_AxisBox( const MathsStorage::Box3 * const pStorage ) :
	m_Min( HVec3( &pStorage->vmin ) ), m_Max( HVec3( &pStorage->vmax) )
{
}

/// Cast to Box3
NOALIAS __forceinline C_AxisBox::operator const MathsStorage::Box3 () const __restrict
{
	MathsStorage::Box3 rtn;
	rtn.vmin = m_Min;
	rtn.vmax = m_Max;
	return rtn;
}

/// Default Constructor: Sets invalid extents
NOALIAS __forceinline C_AxisBox::C_AxisBox() :
    m_Min( HF32( +FLT_MAX ) ),
    m_Max( HF32( -FLT_MAX ) )
{
}

/// Copy Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( const C_AxisBox& copy ) :
    m_Min( copy.m_Min ),
    m_Max( copy.m_Max )
{
}

/// HVec3 Min & Max Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( const HVec3 PCREF min, const HVec3 PCREF max ) :
	m_Min( min ),
	m_Max( max )
{
}

/// Vertex Pair Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( const C_Vertex PCREF A, const C_Vertex PCREF B ) :
    m_Min( A.Min( B ) ),
    m_Max( A.Max( B ) )
{
}

/// Vertex Triple Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( const C_Vertex PCREF A, const C_Vertex PCREF B, const C_Vertex PCREF C ) :
    m_Min( A.Min( B ).Min( C ) ),
    m_Max( A.Max( B ).Max( C ) )
{
}

NOALIAS __forceinline C_AxisBox::C_AxisBox( const HVec3 PCREF A, const HVec3 PCREF B, const HVec3 PCREF C ) :
    m_Min( A.Min( B ).Min( C ) ),
    m_Max( A.Max( B ).Max( C ) )
{
}

/// Sphere Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( const C_Sphere PCREF sphere )
{ 
    const C_Vertex centre( sphere.Centre() );
    const C_Vector radiusVec( sphere.Radius() );
    m_Min = centre - radiusVec;
    m_Max = centre + radiusVec;
}

/// Sphere Array Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( C_Sphere const * sphere, const U32 spheres ) :
    m_Min( HF32( +FLT_MAX ) ),
    m_Max( HF32( -FLT_MAX ) )
{ 
    for ( U32 i=0; i<spheres; i++ )
    {
        const C_Vertex centre( sphere[i].Centre() );
        const C_Vector radiusVec( sphere[i].Radius() );
        m_Min = Maths::Min( m_Min, centre - radiusVec );
        m_Max = Maths::Max( m_Max, centre + radiusVec );
    }
}

/// Vertex Array Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( C_Vertex const * vertex, const U32 vertices ) :
	m_Min( HF32( +FLT_MAX ) ),
	m_Max( HF32( -FLT_MAX ) )
{ 
	for ( U32 i=0; i<vertices; i++ )
	{
		m_Min = Maths::Min( m_Min, vertex[i] );
		m_Max = Maths::Max( m_Max, vertex[i] );
	}
}

/// Transformed Vertex Array Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( C_Vertex const * vertex, const U32 vertices, const HMat43& matrix ) :
	m_Min( HF32( +FLT_MAX ) ),
	m_Max( HF32( -FLT_MAX ) )
{
	for( U32 i=0; i<vertices; ++i )
	{
		C_Vertex newVertex = vertex[i] * matrix;
		m_Min = Maths::Min( m_Min, newVertex );
		m_Max = Maths::Max( m_Max, newVertex );
	}
}


/// Capsule Constructor
NOALIAS __forceinline C_AxisBox::C_AxisBox( const C_Capsule& capsule )
{
    const C_Vector radiusVec( capsule.Radius() );
    const C_Vertex minStartEnd( capsule.Start().Min( capsule.End() ) );
    const C_Vertex maxStartEnd( capsule.Start().Max( capsule.End() ) );
    m_Min = minStartEnd - radiusVec;
    m_Max = maxStartEnd + radiusVec;
}

/// Accessor: Get Min
NOALIAS __forceinline const HVec3 C_AxisBox::Min() const __restrict
{
    return m_Min;
}

/// Accessor: Get Max
NOALIAS __forceinline const HVec3 C_AxisBox::Max() const __restrict
{
    return m_Max;
}

/// Accessor: Set Min
NOALIAS __forceinline void C_AxisBox::Min( const HVec3 PCREF min ) __restrict
{
    m_Min = min;
}

/// Accessor: Set Max
NOALIAS __forceinline void C_AxisBox::Max( const HVec3 PCREF max ) __restrict
{
    m_Max = max;
}

/// Get Centre
NOALIAS __forceinline const HVec3 C_AxisBox::Centre() const  __restrict
{
    const HF32 half( c_ScalarHalf );
    return ( m_Min + m_Max ) * half;
}

/// Get Diagonal
NOALIAS __forceinline const HVec3 C_AxisBox::Diagonal() const __restrict
{
    return m_Max - m_Min;
}

/// Get Bounding Sphere
NOALIAS __forceinline const C_Sphere C_AxisBox::BoundingSphere() const __restrict
{
    return C_Sphere( Centre(), Length( Diagonal() ) * 0.5f );
}

/// Set Zero extents
NOALIAS __forceinline void C_AxisBox::SetZeroExtents() __restrict
{
    //m_Min = HVec3( 0.0f );
    //m_Max = HVec3( 0.0f );
    m_Min = HVec3( c_VectorZero );
    m_Max = HVec3( c_VectorZero );
}

/// Set invalid extents
NOALIAS __forceinline void C_AxisBox::SetInvalidExtents() __restrict
{
    //m_Min = HVec3( +FLT_MAX );
    //m_Max = HVec3( -FLT_MAX );
    m_Min = HVec3( c_VectorFltMax );
    m_Max = HVec3( c_VectorFltMaxNeg );
}

/// Update extents with vertex
NOALIAS __forceinline void C_AxisBox::UpdateExtents( const C_Vertex PCREF v ) __restrict
{
    m_Min = Maths::Min( m_Min, v.m_V );
    m_Max = Maths::Max( m_Max, v.m_V );
}

/// Update extents with another bounding box
NOALIAS __forceinline void C_AxisBox::UpdateExtents( const C_AxisBox& box ) __restrict
{
    m_Min = Maths::Min( m_Min, box.m_Min ); 
    m_Max = Maths::Max( m_Max, box.m_Max ); 
}

/// Update extents with a sphere
NOALIAS __forceinline void C_AxisBox::UpdateExtents( const C_Sphere PCREF sphere ) __restrict   
{ 
    const HVec3 centre( sphere.Centre() );
    const HVec3 radiusVec( sphere.Radius() );
    m_Min = Maths::Min( m_Min, centre - radiusVec ); 
    m_Max = Maths::Max( m_Max, centre + radiusVec ); 
}

/// Update extents with a capsule
NOALIAS __forceinline void C_AxisBox::UpdateExtents( const C_Capsule& capsule ) __restrict   
{ 
    const C_Vector radiusVec( capsule.Radius() );
    const C_Vertex minStartEnd( capsule.Start().Min( capsule.End() ) );
    const C_Vertex maxStartEnd( capsule.Start().Max( capsule.End() ) );
    m_Min = Maths::Min( m_Min, minStartEnd - radiusVec );
    m_Max = Maths::Max( m_Max, maxStartEnd + radiusVec );
}

/// Determine if box inside another box
NOALIAS __forceinline const U32 C_AxisBox::Inside( const C_AxisBox& other ) const __restrict
{
    return m_Min.All_GE( other.m_Min ) & m_Max.All_LE( other.m_Max );
}

/// Determine if point inside box
NOALIAS __forceinline const U32 C_AxisBox::Inside( const C_Vertex PCREF v ) const __restrict
{
    return v.All_GE( m_Min ) & v.All_LE( m_Max );
}

/// Determine if point inside box
NOALIAS __forceinline const Scalar_Bool C_AxisBox::Intersects( const HVec3 PCREF v ) const __restrict
{
    return Scalar_Bool( v.All_GE( m_Min ) & v.All_LE( m_Max ) );
}

/// Determine if sphere intersects this box : NOTE: not yet 100% accurate - may return true when sphere not actually overlapping corners of box!
NOALIAS __forceinline const Scalar_Bool C_AxisBox::Intersects( const C_Sphere PCREF sphere ) const __restrict
{
    const C_Vertex centre( sphere.Centre() );
    const C_Vector radiusVec( sphere.Radius() );
    
    //if ( ( ( centre - radiusVec ) > m_Max ) ||
    //     ( ( centre + radiusVec ) < m_Min ) )
    //if ( ( ( centre - radiusVec ).Any_GT( m_Max ) ) |
    //     ( ( centre + radiusVec ).Any_LT( m_Min ) ) )
    //{
    //    return false;
    //}
    //
    //return true;
    return Scalar_Bool( m_Min.All_LE( centre + radiusVec ) & m_Max.All_GE( centre - radiusVec ) );
}

/// Determine if this box intersects another box
NOALIAS __forceinline const Scalar_Bool C_AxisBox::Intersects( const C_AxisBox& other ) const __restrict
{
	const register U32 one( m_Min.All_LE( other.m_Max ) );
	const register U32 two( m_Max.All_GE( other.m_Min ) );
    return Scalar_Bool( one & two );
}

/// Determine if this box intersects another box
//NOALIAS __forceinline const Scalar_Bool C_AxisBox::Intersects( const C_Vertex& rkP1, const C_Vertex& rkP2, const C_Vertex& rkP3 ) const __restrict
//{
//	// Move triangle and box such that the centre of the AABB is now at the origin,
//	// and compute vectors for the edges of the triangle.
//	/*const Maths::HVec3 centre = Centre();
//	const Maths::HVec3 boxMin = Min() - centre;
//	const Maths::HVec3 boxMax = Max() - centre;
//	const Maths::HVec3 v0 = rkP1 - centre;
//	const Maths::HVec3 v1 = rkP2 - centre;
//	const Maths::HVec3 v2 = rkP3 - centre;
//	const Maths::HVec3 edge0 = v0 - v2;
//	const Maths::HVec3 edge1 = v2 - v1;
//	const Maths::HVec3 edge2 = v1 - v0;
//	const F32 fEX = fabsf(edge0.X());
//	const F32 fEY = fabsf(edge0.Y());
//	const F32 fEZ = fabsf(edge0.Z());
//	AXISTEST_X01(e0[Z], e0[Y], fez, fey);
//	AXISTEST_Y02(e0[Z], e0[X], fez, fex);
//	AXISTEST_Z12(e0[Y], e0[X], fey, fex);*/
//
//	// TODO: Implement this.
//	return Scalar_Bool( true );
//}

/// Split this box into 2 other boxes, in the position specified in the X axis
NOALIAS __forceinline const bool C_AxisBox::SplitAxisX( const HVecX PCREF xPos, C_AxisBox& front, C_AxisBox& back ) __restrict
{
    return SplitAxis( HVec3( xPos ), Create_Vector3_Bool<1,0,0>(), front, back );
}

/// Split this box into 2 other boxes, in the position specified in the Y axis
NOALIAS __forceinline const bool C_AxisBox::SplitAxisY( const HVecY PCREF yPos, C_AxisBox& front, C_AxisBox& back ) __restrict
{
    return SplitAxis( HVec3( yPos ), Create_Vector3_Bool<0,1,0>(), front, back );
}

/// Split this box into 2 other boxes, in the position specified in the Z axis
NOALIAS __forceinline const bool C_AxisBox::SplitAxisZ( const HVecZ PCREF zPos, C_AxisBox& front, C_AxisBox& back ) __restrict
{
    return SplitAxis( HVec3( zPos ), Create_Vector3_Bool<0,0,1>(), front, back );
}

/// Calculate half the surface area of the box
NOALIAS __forceinline const HF32 C_AxisBox::HalfSurfaceArea() const __restrict
{
    return BoxHalfSurfaceArea( m_Max - m_Min );
}

/// Calculate the surface area of the box
NOALIAS __forceinline const HF32 C_AxisBox::SurfaceArea() const __restrict
{
    return BoxSurfaceArea( m_Max - m_Min );
}

/// Calculate the volume of the box
NOALIAS __forceinline const HF32 C_AxisBox::Volume() const __restrict
{
    const HVec3 dimensions( m_Max - m_Min );
    return HF32( HVecX( dimensions ) ) * HF32( HVecY( dimensions ) ) * HF32( HVecZ( dimensions ) );
}

/// Get Array of all 8 vertices of the box
NOALIAS __forceinline void C_AxisBox::GetVertexArray( C_VertexArray& vertexArray ) const __restrict
{
    const HVecX minX( m_Min );
    const HVecY minY( m_Min );
    const HVecZ minZ( m_Min );
    const HVecX maxX( m_Max );
    const HVecY maxY( m_Max );
    const HVecZ maxZ( m_Max );
    
    vertexArray[0] = HVec3( minX, minY, minZ );
    vertexArray[1] = HVec3( minX, minY, maxZ );
    vertexArray[2] = HVec3( minX, maxY, minZ );
    vertexArray[3] = HVec3( minX, maxY, maxZ );
    vertexArray[4] = HVec3( maxX, minY, minZ );
    vertexArray[5] = HVec3( maxX, minY, maxZ );
    vertexArray[6] = HVec3( maxX, maxY, minZ );
    vertexArray[7] = HVec3( maxX, maxY, maxZ );
}

/// Get the face vertex indices for the face with exterior normal pointing in the positive X direction
NOALIAS __forceinline const C_AxisBox::C_Face C_AxisBox::GetFacePosX() const __restrict
{
    return C_Face( 6, 7, 5, 4 );
}

/// Get the face vertex indices for the face with exterior normal pointing in the negative X direction
NOALIAS __forceinline const C_AxisBox::C_Face C_AxisBox::GetFaceNegX() const __restrict
{
    return C_Face( 3, 2, 0, 1 );
}

/// Get the face vertex indices for the face with exterior normal pointing in the positive Y direction
NOALIAS __forceinline const C_AxisBox::C_Face C_AxisBox::GetFacePosY() const __restrict
{
    return C_Face( 3, 7, 6, 2 );
}

/// Get the face vertex indices for the face with exterior normal pointing in the negative Y direction
NOALIAS __forceinline const C_AxisBox::C_Face C_AxisBox::GetFaceNegY() const __restrict
{
    return C_Face( 0, 4, 5, 1 );
}

/// Get the face vertex indices for the face with exterior normal pointing in the positive Z direction
NOALIAS __forceinline const C_AxisBox::C_Face C_AxisBox::GetFacePosZ() const __restrict
{
    return C_Face( 7, 3, 1, 5 );
}

/// Get the face vertex indices for the face with exterior normal pointing in the negative Z direction
NOALIAS __forceinline const C_AxisBox::C_Face C_AxisBox::GetFaceNegZ() const __restrict
{
    return C_Face( 2, 6, 4, 0 );
}

/// Split this box into 2 other boxes, in arbitrary axis specified in axisBitMask (position must be zero in other axes!)
NOALIAS __forceinline const bool C_AxisBox::SplitAxis( const HVec3 PCREF pos, const Vector3_Bool PCREF axisSelector, C_AxisBox& resultFront, C_AxisBox& resultBack ) __restrict
{
    const Vector3_Bool pos_in_box( ( m_Min < pos ) & ( pos < m_Max ) & axisSelector );
    const Scalar_Bool  pos_in_axis( X( pos_in_box ) | Y( pos_in_box ) | Z( pos_in_box ) );
    if ( pos_in_axis )
    {
        const HVec3 zero(ZERO_INIT);
        const HVec3 mask( pos != zero );
        
        resultBack.m_Min = m_Min;
        resultBack.m_Max = OR( ANDNOT( m_Max, mask ), pos );
        
        resultFront.m_Min = OR( ANDNOT( m_Min, mask ), pos );
        resultFront.m_Max = m_Max;
        
        return true;
    }
    
    return false;
}

/// Axis Box Face Constructor
NOALIAS __forceinline C_AxisBox::C_Face::C_Face( const U8 v0, const U8 v1, const U8 v2, const U8 v3 )
    : m_V0( v0 )
    , m_V1( v1 )
    , m_V2( v2 )
    , m_V3( v3 )
{
}
	

/// Read from Stream
template < typename Archive > __forceinline void C_AxisBox::Deserialise( Archive& ar ) __restrict
{
    ar >> m_Min;
    ar >> m_Max;
}

/// Write to Stream
template < typename Archive > __forceinline void C_AxisBox::Serialise( Archive& ar ) __restrict
{
	ar << m_Min;
	ar << m_Max;
}



//
// ------------------------------------
// |-- Non-Member Support Functions --|
// ------------------------------------
//

NOALIAS __forceinline const C_Plane Select( const Scalar_Bool PCREF selector, const C_Plane PCREF resultIfTrue, const C_Plane PCREF resultIfFalse )
{
    C_Plane result( UNDEFINED );
    #if defined( _SPU_ )
    result.m_V.m_V = spu_sel( resultIfFalse.m_V.m_V, resultIfTrue.m_V.m_V, selector.m_V );
    #elif defined( _PPU_ )
    result.m_V.m_V = vec_sel( resultIfFalse.m_V.m_V, resultIfTrue.m_V.m_V, selector.m_V );
    #elif defined( _SIMD_ )
    result.m_V.m_V = SIMD_Select( selector.m_V, resultIfTrue.m_V.m_V, resultIfFalse.m_V.m_V );
    #else
    result.m_V = ( selector.m_F != 0x00000000 ? resultIfTrue.m_V : resultIfFalse.m_V );
    #endif
    return result;
}


NOALIAS __forceinline const Maths::Scalar_Bool IntersectLines(
    const C_Vertex PCREF    in_vertex_A0,
    const C_Vertex PCREF    in_vertex_A1,
    const C_Vertex PCREF    in_vertex_B0,
    const C_Vertex PCREF    in_vertex_B1,
    C_Vertex&               out_intersect_AB )
{
    #if defined( _VMX128_ )
    
    const U32 __(3);
    
    const __vector4 a0( (__vector4&)in_vertex_A0 );
    const __vector4 a1( (__vector4&)in_vertex_A1 );
    const __vector4 b0( (__vector4&)in_vertex_B0 );
    const __vector4 b1( (__vector4&)in_vertex_B1 );
    
    const __vector4 a( __vsubfp( a1, a0 ) );
    const __vector4 b( __vsubfp( b1, b0 ) );
    const __vector4 c( __vsubfp( b0, a0 ) );
    
    const __vector4 half( c_ScalarHalf );
    const __vector4 one( c_ScalarOne );
    const __vector4 nearlyZero( c_ScalarFltMin );
    
    const __vector4 a_yzx( __vpermwi( a, VPERMWI_CONST(VY,VZ,VX,__) ) );
    const __vector4 a_zxy( __vpermwi( a, VPERMWI_CONST(VZ,VX,VY,__) ) );
    const __vector4 b_yzx( __vpermwi( b, VPERMWI_CONST(VY,VZ,VX,__) ) );
    const __vector4 b_zxy( __vpermwi( b, VPERMWI_CONST(VZ,VX,VY,__) ) );
    const __vector4 c_yzx( __vpermwi( c, VPERMWI_CONST(VY,VZ,VX,__) ) );
    const __vector4 c_zxy( __vpermwi( c, VPERMWI_CONST(VZ,VX,VY,__) ) );
    
    const __vector4 a_half( __vmulfp( a, half ) );
    const __vector4 b_half( __vmulfp( b, half ) );
    const __vector4 a0_half( __vmulfp( a0, half ) );
    const __vector4 b0_half( __vmulfp( b0, half ) );
    
    const __vector4 axb_tmp( __vmulfp( a_zxy, b_yzx ) );
    const __vector4 cxa_tmp( __vmulfp( c_zxy, a_yzx ) );
    const __vector4 cxb_tmp( __vmulfp( c_zxy, b_yzx ) );
    const __vector4 axb( __vnmsubfp( a_yzx, b_zxy, axb_tmp ) );
    const __vector4 cxa( __vnmsubfp( c_yzx, a_zxy, cxa_tmp ) );
    const __vector4 cxb( __vnmsubfp( c_yzx, b_zxy, cxb_tmp ) );
    
    const __vector4 axb_len2( __vmsum3fp( axb, axb ) );
    
    const __vector4 axb_rcp_len2_est( __vrefp( axb_len2 ) );
    const __vector4 axb_rcp_len2_nms( __vnmsubfp( axb_rcp_len2_est, axb_len2, one ) );
    const __vector4 axb_rcp_len2    ( __vmaddfp( axb_rcp_len2_est, axb_rcp_len2_nms, axb_rcp_len2_est ) );
    
    const __vector4 cxb_dot_axb( __vmsum3fp( cxb, axb ) );
    const __vector4 cxa_dot_axb( __vmsum3fp( cxa, axb ) );
    
    const __vector4 tA( __vmulfp( cxb_dot_axb, axb_rcp_len2 ) );
    const __vector4 tB( __vmulfp( cxa_dot_axb, axb_rcp_len2 ) );
    
    //const __vector4 closestA( __vmaddfp( a, tA, a0 ) );
    //const __vector4 closestB( __vmaddfp( b, tB, b0 ) );
    
    const __vector4 closestA_half( __vmaddfp( a_half, tA, a0_half ) );
    const __vector4 closestB_half( __vmaddfp( b_half, tB, b0_half ) );
    
    // TODO: Could optimise this by pre-computing half_a, half_b, half_a0 & half_b0
    //const __vector4 closest_vecAB( __vsubfp( closestB, closestA ) );
    //const __vector4 intersectAB( __vmaddfp( closest_vecAB, half, closestA ) );
    const __vector4 intersectAB( __vaddfp( closestA_half, closestB_half ) );
    
    const __vector4 valid( __vcmpgtfp( axb_len2, nearlyZero ) );
    
    (__vector4&)out_intersect_AB = intersectAB;
    
    return Scalar_Bool( valid );
    
    #else
    
    const HF32      half( c_ScalarHalf );
    const HF32      nearlyZero( c_ScalarFltMin );
    
    const C_Vector  a( in_vertex_A1 - in_vertex_A0 );
    const C_Vector  b( in_vertex_B1 - in_vertex_B0 );
    const C_Vector  c( in_vertex_B0 - in_vertex_A0 );
    const C_Vector  axb( a ^ b );
    const C_Vector  cxa( c ^ a );
    const C_Vector  cxb( c ^ b );
    const HF32      axb_len2( axb | axb );
    const HF32      axb_rcp_len( ReciprocalSqrt( axb_len2 ) );
    const HF32      axb_rcp_len2( Reciprocal( axb_len2 ) );
    
    const HF32      distance( Abs( c | axb ) * axb_rcp_len );
    const HF32      tA( ( cxb | axb ) * axb_rcp_len2 );
    const HF32      tB( ( cxa | axb ) * axb_rcp_len2 );
    const C_Vertex  closestA( in_vertex_A0 + ( a * tA ) );
    const C_Vertex  closestB( in_vertex_B0 + ( b * tB ) );
    const C_Vertex  intersectAB( ( closestA + closestB ) * half );
    
    out_intersect_AB = intersectAB;
    
    return axb_len2.GT( nearlyZero );
    
    #endif
}


NOALIAS __forceinline const Maths::Scalar_Bool IntersectLines(
    const C_Vertex PCREF    in_vertex_A0,
    const C_Vertex PCREF    in_vertex_A1,
    const C_Vertex PCREF    in_vertex_B0,
    const C_Vertex PCREF    in_vertex_B1,
    HF32&                   out_distance,
    HF32&                   out_t_A,
    HF32&                   out_t_B,
    C_Vertex&               out_closest_A,
    C_Vertex&               out_closest_B,
    C_Vertex&               out_intersect_AB,
    C_Normal&               out_normal )
{
    #if defined( _VMX128_ )
    
    const U32 __( VW );
    
    const __vector4 a0( (__vector4&)in_vertex_A0 );
    const __vector4 a1( (__vector4&)in_vertex_A1 );
    const __vector4 b0( (__vector4&)in_vertex_B0 );
    const __vector4 b1( (__vector4&)in_vertex_B1 );
    
    const __vector4 a( __vsubfp( a1, a0 ) );
    const __vector4 b( __vsubfp( b1, b0 ) );
    const __vector4 c( __vsubfp( b0, a0 ) );
    
    const __vector4 half( c_ScalarHalf );
    const __vector4 one( c_ScalarOne );
    const __vector4 abs_mask( c_VectorAbsMask );
    const __vector4 nearlyZero( c_ScalarFltMin );
    
    const __vector4 a_yzx( __vpermwi( a, VPERMWI_CONST(VY,VZ,VX,__) ) );
    const __vector4 a_zxy( __vpermwi( a, VPERMWI_CONST(VZ,VX,VY,__) ) );
    const __vector4 b_yzx( __vpermwi( b, VPERMWI_CONST(VY,VZ,VX,__) ) );
    const __vector4 b_zxy( __vpermwi( b, VPERMWI_CONST(VZ,VX,VY,__) ) );
    const __vector4 c_yzx( __vpermwi( c, VPERMWI_CONST(VY,VZ,VX,__) ) );
    const __vector4 c_zxy( __vpermwi( c, VPERMWI_CONST(VZ,VX,VY,__) ) );
    
    const __vector4 a_half( __vmulfp( a, half ) );
    const __vector4 b_half( __vmulfp( b, half ) );
    const __vector4 a0_half( __vmulfp( a0, half ) );
    const __vector4 b0_half( __vmulfp( b0, half ) );
    
    const __vector4 axb_tmp( __vmulfp( a_zxy, b_yzx ) );
    const __vector4 cxa_tmp( __vmulfp( c_zxy, a_yzx ) );
    const __vector4 cxb_tmp( __vmulfp( c_zxy, b_yzx ) );
    const __vector4 axb( __vnmsubfp( a_yzx, b_zxy, axb_tmp ) );
    const __vector4 cxa( __vnmsubfp( c_yzx, a_zxy, cxa_tmp ) );
    const __vector4 cxb( __vnmsubfp( c_yzx, b_zxy, cxb_tmp ) );
    
    const __vector4 axb_len2   ( __vmsum3fp( axb, axb ) );
    const __vector4 c_dot_axb  ( __vmsum3fp( c, axb ) );
    const __vector4 cxb_dot_axb( __vmsum3fp( cxb, axb ) );
    const __vector4 cxa_dot_axb( __vmsum3fp( cxa, axb ) );
    
    const __vector4 axb_rcp_len_est     ( __vrsqrtefp( axb_len2 ) );
    const __vector4 axb_rcp_len2_est    ( __vrefp( axb_len2 ) );
    
    const __vector4 axb_rcp_len_est2    ( __vmulfp( axb_rcp_len_est, axb_rcp_len_est ) );
    const __vector4 axb_rcp_len_est_half( __vmulfp( axb_rcp_len_est, half ) );
    
    const __vector4 axb_rcp_len_nms     ( __vnmsubfp( axb_len2, axb_rcp_len_est2, one ) );
    const __vector4 axb_rcp_len2_nms    ( __vnmsubfp( axb_rcp_len2_est, axb_len2, one ) );
    
    const __vector4 axb_rcp_len         ( __vmaddfp( axb_rcp_len_nms, axb_rcp_len_est_half, axb_rcp_len_est ) );
    const __vector4 axb_rcp_len2        ( __vmaddfp( axb_rcp_len2_est, axb_rcp_len2_nms, axb_rcp_len2_est ) );
    
    const __vector4 abs_c_dot_axb( __vand( c_dot_axb, abs_mask ) );
    const __vector4 distance     ( __vmulfp( abs_c_dot_axb, axb_rcp_len ) );
    
    const __vector4 tA( __vmulfp( cxb_dot_axb, axb_rcp_len2 ) );
    const __vector4 tB( __vmulfp( cxa_dot_axb, axb_rcp_len2 ) );
    
    const __vector4 closestA( __vmaddfp( a, tA, a0 ) );
    const __vector4 closestB( __vmaddfp( b, tB, b0 ) );
    
    const __vector4 closestA_half( __vmaddfp( a_half, tA, a0_half ) );
    const __vector4 closestB_half( __vmaddfp( b_half, tB, b0_half ) );
    
    // TODO: Could optimise this by pre-computing half_a, half_b, half_a0 & half_b0
    //const __vector4 closest_vecAB( __vsubfp( closestB, closestA ) );
    //const __vector4 intersectAB( __vmaddfp( closest_vecAB, half, closestA ) );
    const __vector4 intersectAB( __vaddfp( closestA_half, closestB_half ) );
    
    const __vector4 valid( __vcmpgtfp( axb_len2, nearlyZero ) );
    
    const __vector4 normal( __vmulfp( axb, axb_rcp_len ) );
    
    (__vector4&)out_distance     = distance;
    (__vector4&)out_t_A          = tA;
    (__vector4&)out_t_B          = tB;
    (__vector4&)out_closest_A    = closestA;
    (__vector4&)out_closest_B    = closestB;
    (__vector4&)out_intersect_AB = intersectAB;
    (__vector4&)out_normal       = normal;
    
    return Scalar_Bool( valid );
    
    #else
    
    const HF32      half( c_ScalarHalf );
    const HF32      nearlyZero( c_ScalarFltMin );
    
    const C_Vector  a( in_vertex_A1 - in_vertex_A0 );
    const C_Vector  b( in_vertex_B1 - in_vertex_B0 );
    const C_Vector  c( in_vertex_B0 - in_vertex_A0 );
    const C_Vector  axb( a ^ b );
    const C_Vector  cxa( c ^ a );
    const C_Vector  cxb( c ^ b );
    const HF32      axb_len2( axb | axb );
    const HF32      axb_rcp_len( ReciprocalSqrt( axb_len2 ) );
    const HF32      axb_rcp_len2( Reciprocal( axb_len2 ) );
    
    const HF32      distance( Abs( c | axb ) * axb_rcp_len );
    const HF32      tA( ( cxb | axb ) * axb_rcp_len2 );
    const HF32      tB( ( cxa | axb ) * axb_rcp_len2 );
    const C_Vertex  closestA( in_vertex_A0 + ( a * tA ) );
    const C_Vertex  closestB( in_vertex_B0 + ( b * tB ) );
    const C_Vertex  intersectAB( ( closestA + closestB ) * half );
    
    //#if !defined( PLATFORM_PS3 ) && !defined( PLATFORM_SPU )
    //const C_Normal  normal( (C_Normal&)( axb * axb_rcp_len ) );
    //#else
    const C_Normal  normal( axb * axb_rcp_len, C_Normal::UNIT_LENGTH_VECTOR );
    //#endif
    
    out_distance     = distance;
    out_t_A          = tA;
    out_t_B          = tB;
    out_closest_A    = closestA;
    out_closest_B    = closestB;
    out_intersect_AB = intersectAB;
    out_normal       = normal;
    
    return axb_len2.GT( nearlyZero );
    
    #endif
}


NOALIAS __forceinline const HF32 LineClosestPoint(
    const C_Vertex PCREF    x1,
    const C_Vertex PCREF    x2,
    const C_Vertex PCREF    p,
    C_Vertex&               out_closest_point, // Out: Closest point on line x1->x2 to input point p
    C_Normal&               out_normal,        // Out: Normal from closest point on line x1->x2 to point p
    HF32&                   out_distance )     // Out: Distance between point p and closest point on line x1->x2
{
    // Point : p
    // Line : x1 + ( x2 - x1 ) t
    // Closest t = ( p - x1 ) . ( x2 - x1 ) / | x2 - x1 | ^2
    //           = ( p - x1 ) . ( x2 - x1 ) / ( x2 - x1 ) . ( x2 - x1 )
    
    const C_Vector x1x2( x2 - x1 );
    const C_Vector x1p( p - x1 );
    const HF32     length2_x1x2( x1x2 | x1x2 );
    const HF32     x1p_dot_x1x2( x1p | x1x2 );
    const C_Vector x1p_cross_x1x2( x1p ^ x1x2 );
    const HF32     length_x1x2( Sqrt( length2_x1x2 ) );
    const HF32     rcp_length2_x1x2( Reciprocal( length2_x1x2 ) );
    const HF32     rcp_length_x1x2( ReciprocalSqrt( length2_x1x2 ) );
    const HF32     length_x1p_cross_x1x2( Length( x1p_cross_x1x2 ) );
    const HF32     rcp_length_x1p_cross_x1x2( RcpLength( x1p_cross_x1x2 ) );
    const HF32     t( x1p_dot_x1x2 * rcp_length2_x1x2 );
    const HF32     d( length_x1p_cross_x1x2 * rcp_length_x1x2 );
    const HF32     rcp_d( length_x1x2 * rcp_length_x1p_cross_x1x2 );
    const C_Vertex closest_p( x1 + ( x1x2 * t ) );
    const C_Normal normal( ( p - closest_p ) * rcp_d, C_Normal::UNIT_LENGTH_VECTOR );
    
    out_closest_point = closest_p;
    out_normal        = normal;
    out_distance      = d;
    return t;
}
    



NOALIAS __forceinline const Maths::Scalar_Bool IntersectPlanes(
    const C_Plane PCREF in_plane0,
    const C_Plane PCREF in_plane1,
    C_Vertex&           out_point,
    C_Normal&           out_normal )
{
    // http://mathworld.wolfram.com/Plane-PlaneIntersection.html
    
    const HVec3 N0( in_plane0.Normal() );
    const HVec3 N1( in_plane1.Normal() );
    const HF32  d0( in_plane0.DistanceToOrigin() );
    const HF32  d1( in_plane1.DistanceToOrigin() );
    
    const HVec3 A( N0 ^ N1 );
    
    const HF32 N0x( N0.X() );
    const HF32 N0y( N0.Y() );
    const HF32 N0z( N0.Z() );
    
    const HF32 N1x( N1.X() );
    const HF32 N1y( N1.Y() );
    const HF32 N1z( N1.Z() );
    
    const HF32 N0x_N1y( N0x * N1y );
    const HF32 N0x_N1z( N0x * N1z );
    const HF32 N0y_N1x( N0y * N1x );
    const HF32 N0y_N1z( N0y * N1z );
    const HF32 N0z_N1x( N0z * N1x );
    const HF32 N0z_N1y( N0z * N1y );
    
    //const C_Normalised A_normalised( A.Normalised() );
    const C_Normalised A_normalised( NormalLengthSquaredAndReciprocal( A ) );
    
    const HVec3 divisors( N0y_N1z - N0z_N1y,
                          N0x_N1z - N0z_N1x,
                          N0x_N1y - N0y_N1x );
    
    const HVec3 reciprocals( Reciprocal( divisors ) );
    
    const HF32 N0x_d1( N0x * d1 );
    const HF32 N0y_d1( N0y * d1 );
    const HF32 N0z_d1( N0z * d1 );
    
    const HF32 N1x_d0( N1x * d0 );
    const HF32 N1y_d0( N1y * d0 );
    const HF32 N1z_d0( N1z * d0 );
    
    const HF32 N0x_d1_minus_N1x_d0( N0x_d1 - N1x_d0 );
    const HF32 N0y_d1_minus_N1y_d0( N0y_d1 - N1y_d0 );
    const HF32 N0z_d1_minus_N1z_d0( N0z_d1 - N1z_d0 );
    
    const HF32 rcp_N0y_N1z_minus_N0z_N1y( reciprocals.X() );
    const HF32 rcp_N0x_N1z_minus_N0z_N1x( reciprocals.Y() );
    const HF32 rcp_N0x_N1y_minus_N0y_N1x( reciprocals.Z() );
    
    const HF32 P0y(  N0z_d1_minus_N1z_d0 * ( rcp_N0y_N1z_minus_N0z_N1y ) ); // ( N0z_d1 - N1z_d0 ) / ( N1z_N0y - N0z_N1y )
    const HF32 P0z( -N0y_d1_minus_N1y_d0 * ( rcp_N0y_N1z_minus_N0z_N1y ) ); // ( N0y_d1 - N1y_d0 ) / ( N1y_N0z - N0y_N1z )
    
    const HF32 P1x(  N0z_d1_minus_N1z_d0 * ( rcp_N0x_N1z_minus_N0z_N1x ) ); // ( N0z_d1 - N1z_d0 ) / ( N1z_N0x - N0z_N1x )
    const HF32 P1z( -N0x_d1_minus_N1x_d0 * ( rcp_N0x_N1z_minus_N0z_N1x ) ); // ( N0x_d1 - N1x_d0 ) / ( N1x_N0z - N0x_N1z )
    
    const HF32 P2x(  N0y_d1_minus_N1y_d0 * ( rcp_N0x_N1y_minus_N0y_N1x ) ); // ( N0y_d1 - N1y_d0 ) / ( N1y_N0x - N0y_N1x )
    const HF32 P2y( -N0x_d1_minus_N1x_d0 * ( rcp_N0x_N1y_minus_N0y_N1x ) ); // ( N0x_d1 - N1x_d0 ) / ( N1x_N0y - N0x_N1y )
    
    const HF32 zero(ZERO_INIT);
    
    const HVec3 P0( zero, P0y, P0z );
    const HVec3 P1( P1x, zero, P1z );
    const HVec3 P2( P2x, P2y, zero );
    
    const HF32 Ax( A.X() );
    const HF32 Ay( A.Y() );
    const HF32 Az( A.Z() );
    
    const Scalar_Bool select0( Ax.GE( Ay ) & Ax.GE( Az ) );
    const Scalar_Bool select1( Ay.GE( Ax ) & Ay.GE( Az ) );
    
    const HVec3 P( Select( select0, P0, Select( select1, P1, P2 ) ) );
    
    const HVec3 A_normal( A_normalised.Normal() );
    
    out_point = P;
    out_normal = A_normal;
    
    return A_normalised.Length().GT( zero );
}


/// Calculate the point at which three planes intersect
NOALIAS __forceinline const Maths::Scalar_Bool IntersectPlanes(
	const C_Plane PCREF in_plane0,
	const C_Plane PCREF in_plane1,
	const C_Plane PCREF in_plane2,
	C_Vertex&			out_point )
{
	const C_Normal n0( in_plane0.Normal() );
	const C_Normal n1( in_plane1.Normal() );
	const C_Normal n2( in_plane2.Normal() );

	const C_Vector n1_n2 = n1 ^ n2;
	const C_Vector n2_n0 = n2 ^ n0;
	const C_Vector n0_n1 = n0 ^ n1;

	const Maths::HF32 vCosTheta = n0 | n1_n2;
	
	const Scalar_Bool select0 = vCosTheta.Abs().LT(Maths::Constants<Maths::HF32>::Epsilon());
	const Scalar_Bool select1 = !vCosTheta.Finite();

	if ( select0 | select1 )
		return !select1;

	Maths::HF32 vSecTheta = vCosTheta.Rcp();

	const C_Vector v0 = n1_n2 * in_plane0.DistanceToOrigin();
	const C_Vector v1 = n2_n0 * in_plane0.DistanceToOrigin();
	const C_Vector v2 = n0_n1 * in_plane0.DistanceToOrigin();

	out_point = Maths::C_Vertex(-(v0 + v1 + v2) * vSecTheta);

	return !select1;
}


/// Determine if plane intersects sphere
NOALIAS __forceinline const Maths::Scalar_Bool IntersectPlaneSphere(
    const C_Plane  PCREF plane,
    const C_Sphere PCREF sphere )
{
    const HVec3 sphereCentre( sphere.Centre() );
    const HF32  sphereRadius( sphere.Radius() );
    const HF32  planeDistanceToSphereCentre( plane.Distance( sphereCentre ) );
    return Abs( planeDistanceToSphereCentre ).LE( sphereRadius );
}


/// Determine if a sphere is above the plane
NOALIAS __forceinline const Maths::Scalar_Bool SphereAbovePlane(
    const C_Sphere PCREF sphere,
    const C_Plane  PCREF plane )
{
    const HVec3 sphereCentre( sphere.Centre() );
    const HF32  sphereRadius( sphere.Radius() );
    const HF32  plane_DistanceTo_sphereCentre( plane.Distance( sphereCentre ) );
    return plane_DistanceTo_sphereCentre.GT( sphereRadius );
}

/// Determine if a sphere is below the plane
NOALIAS __forceinline const Maths::Scalar_Bool SphereBelowPlane(
    const C_Sphere PCREF sphere,
    const C_Plane  PCREF plane )
{
    const HVec3 sphereCentre( sphere.Centre() );
    const HF32  sphereRadius( sphere.Radius() );
    const HF32  plane_DistanceTo_sphereCentre( plane.Distance( sphereCentre ) );
    return plane_DistanceTo_sphereCentre.LT( -sphereRadius );
}

}; // namespace Maths





#endif
