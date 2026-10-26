#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_TRIANGLE_H
#define SOFT_MATHS_HVEC_NEW_TRIANGLE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_TRIANGLE_H
#define MATHS_HVEC_NEW_TRIANGLE_H
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
NOALIAS __forceinline const Triangle MakeTriangle( const Point3 PCREF pointA, const Point3 PCREF pointB, const Point3 PCREF pointC );

/// Get Triangle point A
NOALIAS __forceinline const Point3 PointA( const Triangle& triangle );

/// Get Triangle point B
NOALIAS __forceinline const Point3 PointB( const Triangle& triangle );

/// Get Triangle point C
NOALIAS __forceinline const Point3 PointC( const Triangle& triangle );

/// Get Triangle edge vector AB (U)
NOALIAS __forceinline const Vector3 EdgeVectorAB( const Triangle& triangle );

/// Get Triangle edge vector AC (V)
NOALIAS __forceinline const Vector3 EdgeVectorAC( const Triangle& triangle );

/// Get Triangle edge vector BC
NOALIAS __forceinline const Vector3 EdgeVectorBC( const Triangle& triangle );

/// Get Triangle edge length AB (U)
NOALIAS __forceinline const VF32 EdgeLengthAB( const Triangle& triangle );

/// Get Triangle edge length AC (V)
NOALIAS __forceinline const VF32 EdgeLengthAC( const Triangle& triangle );

/// Get Triangle surface vector
NOALIAS __forceinline const Vector3 SurfaceVector( const Triangle& triangle );

/// Get Triangle surface normal
NOALIAS __forceinline const Normal3 SurfaceNormal( const Triangle& triangle );

/// Get Triangle surface plane
NOALIAS __forceinline const Plane SurfacePlane( const Triangle& triangle );

/// Triangle Area
NOALIAS __forceinline const VF32 Area( const Triangle& triangle );

/// Triangle multiplication by Mat33
NOALIAS __forceinline const Triangle operator * ( const Triangle& triangle, const Mat33 MATREF matrix33 );

/// Triangle multiplication by Mat43
NOALIAS __forceinline const Triangle operator * ( const Triangle& triangle, const Mat43 MATREF matrix43 );

/// Triangle multiplication by Mat44
NOALIAS __forceinline const Triangle operator * ( const Triangle& triangle, const Mat44 MATREF matrix44 );





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

