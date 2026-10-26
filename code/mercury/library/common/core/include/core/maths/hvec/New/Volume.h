#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VOLUME_H
#define SOFT_MATHS_HVEC_NEW_VOLUME_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VOLUME_H
#define MATHS_HVEC_NEW_VOLUME_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Volume Tests
/// @{

/// Test if Point3 inside Sphere
NOALIAS __forceinline const VBool Inside( const Point3 PCREF p, const Sphere PCREF sphere );

/// Test if point inside box
NOALIAS __forceinline const VBool Inside( const Point3 PCREF p, const AxisBox& box );



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

