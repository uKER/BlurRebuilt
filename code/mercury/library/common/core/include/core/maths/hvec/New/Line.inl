#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_LINE_INL
#define SOFT_MATHS_HVEC_NEW_LINE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_LINE_INL
#define MATHS_HVEC_NEW_LINE_INL
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
NOALIAS __forceinline const Point3 PointOnLine( const Point3 PCREF a, const Point3 PCREF b, const VF32 VECREF t )
{
    return MulAdd( b - a, t, a );
}


/// Make Line from start & end points
NOALIAS __forceinline const Line MakeLine( const Point3 PCREF start, const Point3 PCREF end )
{
    Line result;
    const Vector3 vector( end - start );
    const VF32 lengthSquared( LengthSquared( vector ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt( lengthSquared, &length, &rcpLength );
    result.m_StartLength  = MakeVec4F32( start, length );
    result.m_EndRcpLength = MakeVec4F32( end, rcpLength );
    return result;
}

/// Make Line from start point, normal & length
NOALIAS __forceinline const Line MakeLine( const Point3 PCREF start, const Normal3 PCREF normal, const VF32 VECREF length )
{
    Line result;
    result.m_StartLength  = MakeVec4F32( start, length );
    result.m_EndRcpLength = MakeVec4F32( MulAdd( normal, length, start ), Rcp( length ) );
    return result;
}

/// Make Line from start point, normal & length
NOALIAS __forceinline const Line MakeLine( const Point3 PCREF start, const Normal3 PCREF normal, const VF32 VECREF length, const VF32 VECREF rcpLength )
{
    Line result;
    result.m_StartLength  = MakeVec4F32( start, length );
    result.m_EndRcpLength = MakeVec4F32( MulAdd( normal, length, start ), rcpLength );
    return result;
}

/// Make Line from start point & vector from start to end
NOALIAS __forceinline const Line MakeLine( const Point3 PCREF start, const Vector3 PCREF vector )
{
    Line result;
    const Point3 end( start + vector );
    const VF32 lengthSquared( LengthSquared( vector ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt( lengthSquared, &length, &rcpLength );
    result.m_StartLength  = MakeVec4F32( start, length );
    result.m_EndRcpLength = MakeVec4F32( end, rcpLength );
    return result;
}

/// Get Line Start
NOALIAS __forceinline const Point3 Start( const Line& line )
{
    return MakePoint3( line.m_StartLength );
}

/// Get Line End
NOALIAS __forceinline const Point3 End( const Line& line )
{
    return MakePoint3( line.m_EndRcpLength );
}

/// Get Line Length
NOALIAS __forceinline const VF32 Length( const Line& line )
{
    return W( line.m_StartLength );
}

/// Get Line Reciprocal Length
NOALIAS __forceinline const VF32 RcpLength( const Line& line )
{
    return W( line.m_EndRcpLength );
}

/// Get Line Vector
NOALIAS __forceinline const Vector3 Vector( const Line& line )
{
    return End( line ) - Start( line );
}

/// Get Line Normal
NOALIAS __forceinline const Normal3 Normal( const Line& line )
{
    return MakeNormal3( Vector( line ) * RcpLength( line ) );
}

/// Set Line Start
NOALIAS __forceinline void SetStart( Line& line, const Point3 PCREF start )
{
    const Point3  end( End( line ) );
    const Vector3 vector( end - start );
    const VF32    lengthSquared( LengthSquared( vector ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt( lengthSquared, &length, &rcpLength );
    line.m_StartLength  = MakeVec4F32( start, length );
    line.m_EndRcpLength = MakeVec4F32( end, rcpLength );
}

/// Set Line End
NOALIAS __forceinline void SetEnd( Line& line, const Point3 PCREF end )
{
    const Point3  start( Start( line ) );
    const Vector3 vector( end - start );
    const VF32    lengthSquared( LengthSquared( vector ) );
    VF32 length, rcpLength;
    SqrtAndRcpSqrt( lengthSquared, &length, &rcpLength );
    line.m_StartLength  = MakeVec4F32( start, length );
    line.m_EndRcpLength = MakeVec4F32( end, rcpLength );
}

/// Set Line Length: adjusts the end point to set the desired length (start point & direction are preserved)
NOALIAS __forceinline void SetLength( Line& line, const VF32 VECREF length )
{
    const VF32    rcpLength( Rcp( length ) );
    const Point3  start( Start( line ) );
    const Normal3 normal( Normalise( End( line ) - start ) );
    const Point3  end( MulAdd( normal, length, start ) );
    line.m_StartLength  = MakeVec4F32( start, length );
    line.m_EndRcpLength = MakeVec4F32( end, rcpLength );
}

/// Get point on Line: start t=0.0, end t=1.0
NOALIAS __forceinline const Point3 Point( const Line& line, const VF32 VECREF t )
{
    //return Start( line ) + Vector( line ) * t;
    return MulAdd( Vector( line ), t, Start( line ) );
}

/// Test if two lines are parallel
/// The tolerance is the dot product of the normals of the two lines (cosine of angle between the lines)
NOALIAS __forceinline const VBool Parallel( const Line& line1, const Line& line2, const VF32 VECREF tolerance )
{
    const Normal3 na( Normal( line1 ) );
    const Normal3 nb( Normal( line2 ) );
    return Abs( na | nb ) >= tolerance;
}

/// Test if two lines are coplanar
/// The tolerance is the maximum distance between the closest points on the lines for them to be considered coplanar
NOALIAS __forceinline const VBool CoPlanar( const Line& line1, const Line& line2, const VF32 VECREF tolerance )
{
    // http://mathworld.wolfram.com/Line-LineIntersection.html
    // http://mathworld.wolfram.com/Coplanar.html
    // http://mathworld.wolfram.com/SkewLines.html
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // coplanar: (x3-x1).((x2-x1)^(x4-x3)) = 0
    
    const Vector3 va( Vector( line1 ) );
    const Vector3 vb( Vector( line2 ) );
    const Vector3 vc( Start( line2 ) - Start( line1 ) );
    return Abs( vc | Normalise( va ^ vb ) ) <= tolerance;
}

/// Test if two lines are skew
/// The tolerance is the minimum distance between the closest points on the lines for them to be considered skew
NOALIAS __forceinline const VBool Skew( const Line& line1, const Line& line2, const VF32 VECREF tolerance )
{
    // http://mathworld.wolfram.com/Line-LineIntersection.html
    // http://mathworld.wolfram.com/Coplanar.html
    // http://mathworld.wolfram.com/SkewLines.html
    // a = x2 - x1
    // b = x4 - x3
    // c = x3 - x1
    // skew: (x3-x1).((x2-x1)^(x4-x3)) != 0
    
    const Vector3 a( Vector( line1 ) );
    const Vector3 b( Vector( line2 ) );
    const Vector3 c( Start( line2 ) - Start( line1 ) );
    return Abs( c | Normalise( a ^ b ) ) > tolerance;
}

/// Line Serialise to stream
template < typename T_Stream >
__forceinline void Serialise( const Line& line, T_Stream& stream )
{
	stream << line.m_StartLength;
    stream << line.m_EndRcpLength;
}    

/// Line Deserialise from stream
template < typename T_Stream >
__forceinline void Deserialise( Line& line, T_Stream& stream )
{
    stream >> line.m_StartLength;
    stream >> line.m_EndRcpLength;
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

