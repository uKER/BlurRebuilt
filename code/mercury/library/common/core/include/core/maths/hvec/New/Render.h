#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_RENDER_H
#define SOFT_MATHS_HVEC_NEW_RENDER_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_RENDER_H
#define MATHS_HVEC_NEW_RENDER_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Render
/// @{

#if 0

/// Draw arrow
void DrawArrow( const Vector3 PCREF vec, const Point3 PCREF point, const RGBA colour );

/// Draw Mat33
void DrawMatrix( const Mat33 MATREF matrix, const Point3 PCREF point );

#endif

/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

