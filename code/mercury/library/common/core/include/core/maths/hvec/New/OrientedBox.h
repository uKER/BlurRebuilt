#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_ORIENTED_BOX_H
#define SOFT_MATHS_HVEC_NEW_ORIENTED_BOX_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_ORIENTED_BOX_H
#define MATHS_HVEC_NEW_ORIENTED_BOX_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup OrientedBox
/// @{

/// Make OrientedBox from Mat43 & Vector3 half extents
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Vector3 PCREF halfExtent );

/// Make OrientedBox from Mat43 & AxisBox (in the space of the matrix)
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const AxisBox& box );

/// Make OrientedBox from Mat43 & min & max vectors
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Vector3 PCREF min, const Vector3 PCREF max );

/// Make OrientedBox from Mat43 & point pair
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Point3 PCREF a, const Point3 PCREF b );

/// Make OrientedBox from Mat43 & point triple
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Point3 PCREF a, const Point3 PCREF b, const Point3 PCREF c );

/// Make OrientedBox from Mat43 & point quad
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Point3 PCREF a, const Point3 PCREF b, const Point3 PCREF c, const Point3 PCREF d );

/// Make OrientedBox from Mat43 & containing 2 input boxes
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const AxisBox& a, const AxisBox& b );

/// Make OrientedBox from Sphere
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Sphere PCREF sphere );

/// OrientedBox get matrix
NOALIAS __forceinline const Mat43 Matrix( const OrientedBox& obb );

/// OrientedBox get centre (obb's matrix translation)
NOALIAS __forceinline const Point3 Centre( const OrientedBox& obb );

/// OrientedBox get half extent (in space of obb's matrix)
NOALIAS __forceinline const Vector3 HalfExtent( const OrientedBox& obb );

/// OrientedBox get extent (in space of obb's matrix)
NOALIAS __forceinline const Vector3 Extent( const OrientedBox& obb );

/// OrientedBox get AxisBox (in space of obb's matrix)
NOALIAS __forceinline const AxisBox GetAxisBox( const OrientedBox& obb );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

