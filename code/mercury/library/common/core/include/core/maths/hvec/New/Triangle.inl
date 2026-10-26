#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_TRIANGLE_INL
#define SOFT_MATHS_HVEC_NEW_TRIANGLE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_TRIANGLE_INL
#define MATHS_HVEC_NEW_TRIANGLE_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Triangle
/// @{

/// Make Triangle from three points
NOALIAS __forceinline const Triangle MakeTriangle( const Point3 PCREF pointA, const Point3 PCREF pointB, const Point3 PCREF pointC )
{
    Triangle result;
    result.m_PointA       = pointA;
    result.m_EdgeVectorAB = pointB - pointA;
    result.m_EdgeVectorAC = pointC - pointA;
    return result;
}

/// Get Triangle point A
NOALIAS __forceinline const Point3 PointA( const Triangle& triangle )
{
    return triangle.m_PointA;
}

/// Get Triangle point B
NOALIAS __forceinline const Point3 PointB( const Triangle& triangle )
{
    return triangle.m_PointA + triangle.m_EdgeVectorAB;
}

/// Get Triangle point C
NOALIAS __forceinline const Point3 PointC( const Triangle& triangle )
{
    return triangle.m_PointA + triangle.m_EdgeVectorAC;
}

/// Get Triangle edge vector AB (U)
NOALIAS __forceinline const Vector3 EdgeVectorAB( const Triangle& triangle )
{
    return triangle.m_EdgeVectorAB;
}

/// Get Triangle edge vector AC (V)
NOALIAS __forceinline const Vector3 EdgeVectorAC( const Triangle& triangle )
{
    return triangle.m_EdgeVectorAC;
}

/// Get Triangle edge vector BC
NOALIAS __forceinline const Vector3 EdgeVectorBC( const Triangle& triangle )
{
    return triangle.m_EdgeVectorAC - triangle.m_EdgeVectorAB;
}

/// Get Triangle edge length AB (U)
NOALIAS __forceinline const VF32 EdgeLengthAB( const Triangle& triangle )
{
    return Length( triangle.m_EdgeVectorAB );
}

/// Get Triangle edge length AC (V)
NOALIAS __forceinline const VF32 EdgeLengthAC( const Triangle& triangle )
{
    return Length( triangle.m_EdgeVectorAC );
}

/// Get Triangle surface vector
NOALIAS __forceinline const Vector3 SurfaceVector( const Triangle& triangle )
{
    return EdgeVectorAB( triangle ) ^ EdgeVectorAC( triangle );
}

/// Get Triangle surface normal
NOALIAS __forceinline const Normal3 SurfaceNormal( const Triangle& triangle )
{
    return Normalise( EdgeVectorAB( triangle ) ^ EdgeVectorAC( triangle ) );
}

/// Get Triangle surface plane
NOALIAS __forceinline const Plane SurfacePlane( const Triangle& triangle )
{
    return MakePlane( PointA( triangle ), PointB( triangle ), PointC( triangle ) );
}

/// Triangle Area
NOALIAS __forceinline const VF32 Area( const Triangle& triangle )
{
    return Length( EdgeVectorAB( triangle ) ^ EdgeVectorBC( triangle ) ) * Const<VF32>::Half();
}

/// Triangle multiplication by Mat33
NOALIAS __forceinline const Triangle operator * ( const Triangle& triangle, const Mat33 MATREF matrix33 )
{
    return MakeTriangle( PointA( triangle ) * matrix33,
                         PointB( triangle ) * matrix33,
                         PointC( triangle ) * matrix33 );
}

/// Triangle multiplication by Mat43
NOALIAS __forceinline const Triangle operator * ( const Triangle& triangle, const Mat43 MATREF matrix43 )
{
    return MakeTriangle( PointA( triangle ) * matrix43,
                         PointB( triangle ) * matrix43,
                         PointC( triangle ) * matrix43 );
}

/// Triangle multiplication by Mat44
NOALIAS __forceinline const Triangle operator * ( const Triangle& triangle, const Mat44 MATREF matrix44 )
{
    return MakeTriangle( Normalise( PointA( triangle ) * matrix44 ),
                         Normalise( PointB( triangle ) * matrix44 ),
                         Normalise( PointC( triangle ) * matrix44 ) );
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

