#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VOLUME_INL
#define SOFT_MATHS_HVEC_NEW_VOLUME_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VOLUME_INL
#define MATHS_HVEC_NEW_VOLUME_INL
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
NOALIAS __forceinline const VBool Inside( const Point3 PCREF p, const Sphere PCREF sphere )
{
    const Point3  c( Centre( sphere ) );
    const VF32    r( Radius( sphere ) );
    const Vector3 cp( p - c );
    const VF32    r_squared( r * r );
    const VF32    d_squared( LengthSquared( cp ) );
    return d_squared <= r_squared;
}

/// Test if Point3 inside AxisBox
NOALIAS __forceinline const VBool Inside( const Point3 PCREF p, const AxisBox& box )
{
    //return AllTrue3( p.m_V >= box.m_Min.m_V ) &
    //       AllTrue3( p.m_V <= box.m_Max.m_V );
    return AllTrue3( ( p.m_V >= box.m_Min.m_V ) & ( p.m_V <= box.m_Max.m_V ) );
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

