#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_AXIS_BOX_INL
#define SOFT_MATHS_HVEC_NEW_AXIS_BOX_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_AXIS_BOX_INL
#define MATHS_HVEC_NEW_AXIS_BOX_INL
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
NOALIAS __forceinline const AxisBox MakeAxisBox( const Vector3 PCREF min, const Vector3 PCREF max )
{
    AxisBox result;
    result.m_Min = min;
    result.m_Max = max;
    return result;
}

/// Make AxisBox from point pair
NOALIAS __forceinline const AxisBox MakeAxisBox( const Point3 PCREF a, const Point3 PCREF b )
{
    AxisBox result;
    result.m_Min.m_V = Min( a.m_V, b.m_V );
    result.m_Max.m_V = Max( a.m_V, b.m_V );
    return result;
}

/// Make AxisBox from point triple
NOALIAS __forceinline const AxisBox MakeAxisBox( const Point3 PCREF a, const Point3 PCREF b, const Point3 PCREF c )
{
    AxisBox result;
    result.m_Min.m_V = Min( Min( a.m_V, b.m_V ), c.m_V );
    result.m_Max.m_V = Max( Max( a.m_V, b.m_V ), c.m_V );
    return result;
}

/// Make AxisBox from point quad
NOALIAS __forceinline const AxisBox MakeAxisBox( const Point3 PCREF a, const Point3 PCREF b, const Point3 PCREF c, const Point3 PCREF d )
{
    AxisBox result;
    result.m_Min.m_V = Min( Min( a.m_V, b.m_V ), Min( c.m_V, d.m_V ) );
    result.m_Max.m_V = Max( Max( a.m_V, b.m_V ), Max( c.m_V, d.m_V ) );
    return result;
}

/// Make AxisBox containing 2 input boxes
NOALIAS __forceinline const AxisBox MakeAxisBox( const AxisBox& a, const AxisBox& b )
{
    AxisBox result;
    result.m_Min = Min( a.m_Min, b.m_Min );
    result.m_Max = Max( a.m_Max, b.m_Max );
    return result;
}

/// Make AxisBox from Sphere (returns the bounding box of the sphere)
NOALIAS __forceinline const AxisBox MakeAxisBox( const Sphere PCREF sphere )
{
    //AxisBox result;
    //const Point3  centre( Centre( sphere ) );
    //const Vector3 delta( MakeVector3( Radius( sphere ) ) );
    //result.m_Min = MakeVector3( centre - delta );
    //result.m_Max = MakeVector3( centre + delta );
    //return result;
    return BoundingBox( sphere );
}

/// AxisBox get minimum vector
NOALIAS __forceinline const Vector3 Min( const AxisBox& box )
{
    return box.m_Min;
}

/// AxisBox get maximum vector
NOALIAS __forceinline const Vector3 Max( const AxisBox& box )
{
    return box.m_Max;
}

/// AxisBox set minimum vector
NOALIAS __forceinline void SetMin( AxisBox* const __restrict pBox, const Vector3 PCREF min )
{
    pBox->m_Min = min;
}

/// AxisBox set maximum vector
NOALIAS __forceinline void SetMax( AxisBox* const __restrict pBox, const Vector3 PCREF max )
{
    pBox->m_Max = max;
}

/// AxisBox get centre
NOALIAS __forceinline const Point3 Centre( const AxisBox& box )
{
    const VF32 half( Const<VF32>::Half() );
    return MakePoint3( box.m_Min + box.m_Max ) * half;
}

/// AxisBox get extent
NOALIAS __forceinline const Vector3 Extent( const AxisBox& box )
{
    return box.m_Max - box.m_Min;
}

/// AxisBox get bounding sphere
NOALIAS __forceinline const Sphere BoundingSphere( const AxisBox& box )
{
    const VF32 half( Const<VF32>::Half() );
    return MakeSphere( Centre( box ), Length( Extent( box ) ) * half );
}

/// AxisBox set zero extent
NOALIAS __forceinline void SetZeroExtent( AxisBox* const __restrict pBox )
{
    const Vector3 v_zero( Const<Vector3>::Zero() );
    pBox->m_Min = v_zero;
    pBox->m_Max = v_zero;
}

/// AxisBox set invalid extent ( min > max )
NOALIAS __forceinline void SetInvalidExtent( AxisBox* const __restrict pBox )
{
    const Vector3 v_max( Const<Vector3>::Max() );
    pBox->m_Min =  v_max;
    pBox->m_Max = -v_max;
}

/// AxisBox update extent with Point3
NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const Point3 PCREF point )
{
    pBox->m_Min.m_V = Min( pBox->m_Min.m_V, point.m_V );
    pBox->m_Max.m_V = Max( pBox->m_Max.m_V, point.m_V );
}

/// AxisBox update extent with another AxisBox
NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const AxisBox PCREF box2 )
{
    pBox->m_Min.m_V = Min( pBox->m_Min.m_V, box2.m_Min.m_V );
    pBox->m_Max.m_V = Max( pBox->m_Max.m_V, box2.m_Max.m_V );
}

/// AxisBox update extent with Sphere
NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const Sphere PCREF sphere )
{
    UpdateExtent( pBox, BoundingBox( sphere ) );
}

/// AxisBox valid test ( min < max, for all elements )
NOALIAS __forceinline const VBool Valid( const AxisBox& box )
{
    return AllTrue3( box.m_Min < box.m_Max );
}

/// AxisBox surface area
NOALIAS __forceinline const VF32 SurfaceArea( const AxisBox& box )
{
    const Vector3 dimensionsXYZ( box.m_Max - box.m_Min );
    const Vector3 dimensionsYZX( Permute<VY,VZ,VX>( dimensionsXYZ ) );
    const VF32    halfSurfaceArea( dimensionsXYZ | dimensionsYZX );
    return halfSurfaceArea + halfSurfaceArea;
}

/// AxisBox volume
NOALIAS __forceinline const VF32 Volume( const AxisBox& box )
{
    const Vector3 dimensions( box.m_Max - box.m_Min );
    const VF32    x( X( dimensions ) );
    const VF32    y( Y( dimensions ) );
    const VF32    z( Z( dimensions ) );
    return x * y * z;
}


/// AxisBox Serialise to Stream
template < typename T_Stream >
NOALIAS __forceinline void Serialise( const AxisBox& box, T_Stream& stream )
{
	stream << box.m_Min;
	stream << box.m_Max;
}

/// AxisBox Deserialise from Stream
template < typename T_Stream >
NOALIAS __forceinline void Deserialise( AxisBox& box, T_Stream& stream )
{
    stream >> box.m_Min;
    stream >> box.m_Max;
}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

