#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_PLANE_H
#define SOFT_MATHS_HVEC_NEW_PLANE_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_PLANE_H
#define MATHS_HVEC_NEW_PLANE_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Plane
/// @{

/// Plane: Normal & Distance-to-Origin Constructor
NOALIAS __forceinline const Plane MakePlane( const Normal3 PCREF normal, const VF32 VECREF d );

/// Plane: Point & Normal Constructor
NOALIAS __forceinline const Plane MakePlane( const Point3 PCREF point, const Normal3 PCREF normal );

/// Plane: Normal & Point Constructor
NOALIAS __forceinline const Plane MakePlane( const Normal3 PCREF normal, const Point3 PCREF point );

/// Plane: Point Triple Constructor (faster but only 22 bit precision)
NOALIAS __forceinline const Plane MakePlane( const Point3 PCREF pointA, const Point3 PCREF pointB, const Point3 PCREF pointC );

/// Plane: Point Triple Constructor (slower but full 24 bit precision)
NOALIAS __forceinline const Plane MakePlane24( const Point3 PCREF pointA, const Point3 PCREF pointB, const Point3 PCREF pointC );

/// Plane: Triangle Constructor
NOALIAS __forceinline const Plane MakePlane( const Triangle& triangle );

/// Get Plane normal
NOALIAS __forceinline const Normal3 Normal( const Plane PCREF plane );

/// Get Plane distance to origin
NOALIAS __forceinline const VF32 DistanceToOrigin( const Plane PCREF plane );

/// Get Plane point (closest point on plane to origin)
NOALIAS __forceinline const Point3 Point( const Plane PCREF plane );

/// Plane negation: returns plane in same position but with negated normal
NOALIAS __forceinline const Plane operator - ( const Plane PCREF plane );

/// Plane translation: Returns parallel plane offset at distance 'd' forwards along the normal
NOALIAS __forceinline const Plane operator + ( const Plane PCREF plane, const VF32 VECREF d );

/// Plane translation: Returns parallel plane offset at distance 'd' backwards along the normal
NOALIAS __forceinline const Plane operator - ( const Plane PCREF plane, const VF32 VECREF d );

/// Plane translation: translates plane by distance 'd' forwards along the normal
NOALIAS __forceinline void operator += ( Plane& plane, const VF32 VECREF d );

/// Plane translation: translates plane by distance 'd' backwards along the normal
NOALIAS __forceinline void operator -= ( Plane& plane, const VF32 VECREF d );

/// Plane transformation by 3x3 matrix
NOALIAS __forceinline const Plane operator * ( const Plane PCREF plane, const Mat33 MATREF matrix33 );

/// Plane transformation by 4x3 matrix
NOALIAS __forceinline const Plane operator * ( const Plane PCREF plane, const Mat43 MATREF matrix43 );

/// Plane transformation by 4x4 matrix
NOALIAS __forceinline const Plane operator * ( const Plane PCREF plane, const Mat44 MATREF matrix44 );

/// Plane-Line Intersection: returns t value of the line intersect point
//NOALIAS __forceinline const VF32 IntersectLine( const Plane PCREF plane, const Point3 PCREF start, const Point3 PCREF end );

/// Plane CoPlanar: Determines if the given planes are coplanar within the given tolerances
NOALIAS __forceinline const VBool CoPlanar( const Plane PCREF a, const Plane PCREF b, const VF32 VECREF normalTolerance, const VF32 VECREF offsetTolerance );

/// Project Point3 onto Plane
NOALIAS __forceinline const Point3 Project( const Point3 VECREF point, const Plane VECREF plane );

/// Plane Serialise to stream
template < typename T_Stream >
__forceinline void Serialise( const Plane PCREF plane, T_Stream& stream );

/// Plane Deserialise from stream
template < typename T_Stream >
__forceinline void Deserialise( Plane& plane, T_Stream& stream );




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

