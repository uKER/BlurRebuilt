#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_ORIENTED_BOX_INL
#define SOFT_MATHS_HVEC_NEW_ORIENTED_BOX_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_ORIENTED_BOX_INL
#define MATHS_HVEC_NEW_ORIENTED_BOX_INL
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
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Vector3 PCREF halfExtent )
{
    OrientedBox result;
    result.m_HalfExtent = halfExtent;
    result.m_Matrix = matrix;
    return result;
}

/// Make OrientedBox from Mat43 & AxisBox (in the space of the matrix)
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const AxisBox& box )
{
    const VF32    half( Const<VF32>::Half() );
    const Point3  centre( Centre( box ) );
    const Vector3 extent( Extent( box ) );
    return MakeOrientedBox( MakeMat43( Rotation( matrix ), centre * matrix ), extent * half );
}

/// Make OrientedBox from Mat43 & min & max vectors
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Vector3 PCREF min, const Vector3 PCREF max )
{
    return MakeOrientedBox( matrix, MakeAxisBox( min, max ) );
}

/// Make OrientedBox from Mat43 & point pair
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Point3 PCREF a, const Point3 PCREF b )
{
    return MakeOrientedBox( matrix, MakeAxisBox( a, b ) );
}

/// Make OrientedBox from Mat43 & point triple
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Point3 PCREF a, const Point3 PCREF b, const Point3 PCREF c )
{
    return MakeOrientedBox( matrix, MakeAxisBox( a, b, c ) );
}

/// Make OrientedBox from Mat43 & point quad
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Point3 PCREF a, const Point3 PCREF b, const Point3 PCREF c, const Point3 PCREF d )
{
    return MakeOrientedBox( matrix, MakeAxisBox( a, b, c, d ) );
}

/// Make OrientedBox from Mat43 & containing 2 input boxes
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const AxisBox& a, const AxisBox& b )
{
    return MakeOrientedBox( matrix, MakeAxisBox( a, b ) );
}

/// Make OrientedBox from Sphere
NOALIAS __forceinline const OrientedBox MakeOrientedBox( const Mat43& matrix, const Sphere PCREF sphere )
{
    return MakeOrientedBox( matrix, BoundingBox( sphere ) );
}

/// OrientedBox get matrix
NOALIAS __forceinline const Mat43 Matrix( const OrientedBox& obb )
{
    return obb.m_Matrix;
}

/// OrientedBox get centre (obb's matrix translation)
NOALIAS __forceinline const Point3 Centre( const OrientedBox& obb )
{
    return MakePoint3( Translation( obb.m_Matrix ) );
}

/// OrientedBox get half extent (in space of obb's matrix)
NOALIAS __forceinline const Vector3 HalfExtent( const OrientedBox& obb )
{
    return obb.m_HalfExtent;
}

/// OrientedBox get extent (in space of obb's matrix)
NOALIAS __forceinline const Vector3 Extent( const OrientedBox& obb )
{
    return obb.m_HalfExtent + obb.m_HalfExtent;
}

/// OrientedBox get AxisBox (in space of obb's matrix)
NOALIAS __forceinline const AxisBox GetAxisBox( const OrientedBox& obb )
{
    return MakeAxisBox( -obb.m_HalfExtent, obb.m_HalfExtent );
}




/// AxisBox get minimum vector
//NOALIAS __forceinline const Vector3 Min( const AxisBox& box )
//{
//    return box.m_Min;
//}

/// AxisBox get maximum vector
//NOALIAS __forceinline const Vector3 Max( const AxisBox& box )
//{
//    return box.m_Max;
//}

/// AxisBox set minimum vector
//NOALIAS __forceinline void SetMin( AxisBox* const __restrict pBox, const Vector3 PCREF min )
//{
//    pBox->m_Min = min;
//}

/// AxisBox set maximum vector
//NOALIAS __forceinline void SetMax( AxisBox* const __restrict pBox, const Vector3 PCREF max )
//{
//    pBox->m_Max = max;
//}

/// AxisBox get centre
//NOALIAS __forceinline const Point3 Centre( const AxisBox& box )
//{
//    const VF32 half( Const<VF32>::Half() );
//    return MakePoint3( box.m_Min + box.m_Max ) * half;
//}

/// AxisBox get extent
//NOALIAS __forceinline const Vector3 Extent( const AxisBox& box )
//{
//    return box.m_Max - box.m_Min;
//}

/// AxisBox get bounding sphere
//NOALIAS __forceinline const Sphere BoundingSphere( const AxisBox& box )
//{
//    const VF32 half( Const<VF32>::Half() );
//    return MakeSphere( Centre( box ), Length( Extent( box ) ) * half );
//}

/// AxisBox set zero extent
//NOALIAS __forceinline void SetZeroExtent( AxisBox* const __restrict pBox )
//{
//    const Vector3 v_zero( Const<Vector3>::Zero() );
//    pBox->m_Min = v_zero;
//    pBox->m_Max = v_zero;
//}

/// AxisBox set invalid extent ( min > max )
//NOALIAS __forceinline void SetInvalidExtent( AxisBox* const __restrict pBox )
//{
//    const Vector3 v_max( Const<Vector3>::Max() );
//    pBox->m_Min =  v_max;
//    pBox->m_Max = -v_max;
//}

/// AxisBox update extent with Point3
//NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const Point3 PCREF point )
//{
//    pBox->m_Min.m_V = Min( pBox->m_Min.m_V, point.m_V );
//    pBox->m_Max.m_V = Max( pBox->m_Max.m_V, point.m_V );
//}

/// AxisBox update extent with another AxisBox
//NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const AxisBox PCREF box2 )
//{
//    pBox->m_Min.m_V = Min( pBox->m_Min.m_V, box2.m_Min.m_V );
//    pBox->m_Max.m_V = Max( pBox->m_Max.m_V, box2.m_Max.m_V );
//}

/// AxisBox update extent with Sphere
//NOALIAS __forceinline void UpdateExtent( AxisBox* const __restrict pBox, const Sphere PCREF sphere )
//{
//    UpdateExtent( pBox, BoundingBox( sphere ) );
//}

/// AxisBox valid test ( min < max, for all elements )
//NOALIAS __forceinline const VBool Valid( const AxisBox& box )
//{
//    return AllTrue3( box.m_Min < box.m_Max );
//}

/// AxisBox surface area
//NOALIAS __forceinline const VF32 SurfaceArea( const AxisBox& box )
//{
//    const Vector3 dimensionsXYZ( box.m_Max - box.m_Min );
//    const Vector3 dimensionsYZX( Permute<VY,VZ,VX>( dimensionsXYZ ) );
//    const VF32    halfSurfaceArea( dimensionsXYZ | dimensionsYZX );
//    return halfSurfaceArea + halfSurfaceArea;
//}

/// AxisBox volume
//NOALIAS __forceinline const VF32 Volume( const AxisBox& box )
//{
//    const Vector3 dimensions( box.m_Max - box.m_Min );
//    const VF32    x( X( dimensions ) );
//    const VF32    y( Y( dimensions ) );
//    const VF32    z( Z( dimensions ) );
//    return x * y * z;
//}


/// AxisBox Serialise to Stream
//template < typename T_Stream >
//NOALIAS __forceinline void Serialise( const AxisBox& box, T_Stream& stream )
//{
//	stream << box.m_Min;
//	stream << box.m_Max;
//}

/// AxisBox Deserialise from Stream
//template < typename T_Stream >
//NOALIAS __forceinline void Deserialise( AxisBox& box, T_Stream& stream )
//{
//    stream >> box.m_Min;
//    stream >> box.m_Max;
//}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

