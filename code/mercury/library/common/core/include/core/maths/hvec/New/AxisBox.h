#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_AXIS_BOX_H
#define SOFT_MATHS_HVEC_NEW_AXIS_BOX_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_AXIS_BOX_H
#define MATHS_HVEC_NEW_AXIS_BOX_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup AxisBox
/// @{

/// Make AxisBox from min & max vectors
NOALIAS __forceinline const AxisBox MakeAxisBox( const Vector3 PCREF min, const Vector3 PCREF max );

/// Make AxisBox from point pair
NOALIAS __forceinline const AxisBox MakeAxisBox( const Point3 PCREF a, const Point3 PCREF b );

/// Make AxisBox from point triple
NOALIAS __forceinline const AxisBox MakeAxisBox( const Point3 PCREF a, const Point3 PCREF b, const Point3 PCREF c );

/// Make AxisBox from point quad
NOALIAS __forceinline const AxisBox MakeAxisBox( const Point3 PCREF a, const Point3 PCREF b, const Point3 PCREF c, const Point3 PCREF d );

/// Make AxisBox containing 2 input boxes
NOALIAS __forceinline const AxisBox MakeAxisBox( const AxisBox& a, const AxisBox& b );

/// Make AxisBox from Sphere (returns the bounding box of the sphere)
NOALIAS __forceinline const AxisBox MakeAxisBox( const Sphere PCREF sphere );

/// AxisBox get minimum vector
NOALIAS __forceinline const Vector3 Min( const AxisBox& box );

/// AxisBox get maximum vector
NOALIAS __forceinline const Vector3 Max( const AxisBox& box );

/// AxisBox set minimum vector
NOALIAS __forceinline void SetMin( AxisBox* const __restrict pBox, const Vector3 PCREF min );

/// AxisBox set maximum vector
NOALIAS __forceinline void SetMax( AxisBox* const __restrict pBox, const Vector3 PCREF max );

/// AxisBox get centre
NOALIAS __forceinline const Point3 Centre( const AxisBox& box );

/// AxisBox get extent
NOALIAS __forceinline const Vector3 Extent( const AxisBox& box );

/// AxisBox get bounding sphere
NOALIAS __forceinline const Sphere BoundingSphere( const AxisBox& box );

/// AxisBox set zero extent
NOALIAS __forceinline void SetZeroExtent( AxisBox* const __restrict pBox );

/// AxisBox set invalid extent ( min > max )
NOALIAS __forceinline void SetInvalidExtent( AxisBox* const __restrict pBox );

/// AxisBox update extent with Point3
NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const Point3 PCREF point );

/// AxisBox update extent with another AxisBox
NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const AxisBox PCREF box2 );

/// AxisBox update extent with Sphere
NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const Sphere PCREF sphere );

/// AxisBox valid test ( min < max, for all elements )
NOALIAS __forceinline const VBool Valid( const AxisBox& box );

/// AxisBox surface area
NOALIAS __forceinline const VF32 SurfaceArea( const AxisBox& box );

/// AxisBox volume
NOALIAS __forceinline const VF32 Volume( const AxisBox& box );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

