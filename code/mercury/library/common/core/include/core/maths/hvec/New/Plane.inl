#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_PLANE_INL
#define SOFT_MATHS_HVEC_NEW_PLANE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_PLANE_INL
#define MATHS_HVEC_NEW_PLANE_INL
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
NOALIAS __forceinline const Plane MakePlane( const Normal3 PCREF normal, const VF32 VECREF d )
{
    Plane result;
    result.m_V = MakeVec4F32( normal, d );
    return result;
}

/// Plane: Point & Normal Constructor
NOALIAS __forceinline const Plane MakePlane( const Point3 PCREF point, const Normal3 PCREF normal )
{
    Plane result;
    result.m_V = MakeVec4F32( normal, -normal | point );
    return result;
}

/// Plane: Normal & Point Constructor
NOALIAS __forceinline const Plane MakePlane( const Normal3 PCREF normal, const Point3 PCREF point )
{
    Plane result;
    result.m_V = MakeVec4F32( normal, -normal | point );
    return result;
}

/// Plane: Point Triple Constructor (faster but only 22 bit precision)
NOALIAS __forceinline const Plane MakePlane( const Point3 PCREF pointA, const Point3 PCREF pointB, const Point3 PCREF pointC )
{
    Plane result;
    const Normal3 normal( Normalise( ( pointB - pointA ) ^ ( pointC - pointA ) ) );
    result.m_V = MakeVec4F32( normal, -normal | pointA );
    return result;
}

/// Plane: Point Triple Constructor (slower but full 24 bit precision)
NOALIAS __forceinline const Plane MakePlane24( const Point3 PCREF pointA, const Point3 PCREF pointB, const Point3 PCREF pointC )
{
    Plane result;
    const Normal3 normal( Normalise24( ( pointB - pointA ) ^ ( pointC - pointA ) ) );
    result.m_V = MakeVec4F32( normal, -normal | pointA );
    return result;
}

/// Plane: Triangle Constructor
NOALIAS __forceinline const Plane MakePlane( const Triangle& triangle )
{
    return MakePlane( PointA( triangle ), PointB( triangle ), PointC( triangle ) );
}

/// Get Plane normal
NOALIAS __forceinline const Normal3 Normal( const Plane PCREF plane )
{
    return MakeNormal3( plane.m_V );
}

/// Get Plane distance to origin
NOALIAS __forceinline const VF32 DistanceToOrigin( const Plane PCREF plane )
{
    return W( plane.m_V );
}

/// Get Plane point (closest point on plane to origin)
NOALIAS __forceinline const Point3 Point( const Plane PCREF plane )
{
    return MakePoint3( Normal( plane ) * -DistanceToOrigin( plane ) );
}

/// Plane negation: returns plane in same position but with negated normal
NOALIAS __forceinline const Plane operator - ( const Plane PCREF plane )
{
    Plane result;
    result.m_V = -plane.m_V;
    return result;
}

/// Plane translation: Returns parallel plane offset at distance 'd' forwards along the normal
NOALIAS __forceinline const Plane operator + ( const Plane PCREF plane, const VF32 VECREF d )
{
    Plane result;
    result.m_V = MakeVec4F32( Normal( plane ), DistanceToOrigin( plane ) - d );
    return result;
}

/// Plane translation: Returns parallel plane offset at distance 'd' backwards along the normal
NOALIAS __forceinline const Plane operator - ( const Plane PCREF plane, const VF32 VECREF d )
{
    Plane result;
    result.m_V = MakeVec4F32( Normal( plane ), DistanceToOrigin( plane ) + d );
    return result;
}

/// Plane translation: translates plane by distance 'd' forwards along the normal
NOALIAS __forceinline void operator += ( Plane& plane, const VF32 VECREF d )
{
    plane.m_V = MakeVec4F32( Normal( plane ), DistanceToOrigin( plane ) - d );
}

/// Plane translation: translates plane by distance 'd' backwards along the normal
NOALIAS __forceinline void operator -= ( Plane& plane, const VF32 VECREF d )
{
    plane.m_V = MakeVec4F32( Normal( plane ), DistanceToOrigin( plane ) + d );
}

/// Plane transformation by 3x3 matrix
NOALIAS __forceinline const Plane operator * ( const Plane PCREF plane, const Mat33 MATREF matrix33 )
{
    const Normal3 normal( Normal( plane ) );
    const Point3  point( MakePoint3( normal ) * -DistanceToOrigin( plane ) );
    return MakePlane( point * matrix33, Normalise( normal * matrix33 ) );
}

/// Plane transformation by 4x3 matrix
NOALIAS __forceinline const Plane operator * ( const Plane PCREF plane, const Mat43 MATREF matrix43 )
{
    const Normal3 normal( Normal( plane ) );
    const Point3  point( MakePoint3( normal ) * -DistanceToOrigin( plane ) );
    return MakePlane( point * matrix43, Normalise( normal * matrix43 ) );
}

/// Plane transformation by 4x4 matrix
NOALIAS __forceinline const Plane operator * ( const Plane PCREF plane, const Mat44 MATREF matrix44 )
{
    const Normal3 normal( Normal( plane ) );
    const Point3  point( MakePoint3( normal * -DistanceToOrigin( plane ) ) );
    const Vector3 transformed_normal( normal * InverseTranspose( MakeMat33( matrix44 ) ) );
    //const Vector4 transformed_point( point * matrix44 );
    //return MakePlane( Normalise( transformed_normal ),
    //                  MakePoint3( transformed_point ) /* / W( transformed_point ) */ ); // NOTE: Division by W now implicit in Vector4 -> Point3 version of MakePoint3()
    const HPoint3 transformed_point( point * matrix44 );
    return MakePlane( Normalise( transformed_normal ),
                      Normalise( transformed_point ) ); // NOTE: Division by W performed in normalisation from HPoint3 -> Point3
}

/// Plane-Line Intersection: returns t value of the line intersect point
//NOALIAS __forceinline const VF32 IntersectLine( const Plane PCREF plane, const Point3 PCREF start, const Point3 PCREF end )
//{
//    const Normal3 n( Normal( plane ) );
//    const Point3  p( MakePoint3( n * -DistanceToOrigin( plane ) ) );
//    const VF32    denom( n | ( end - start ) );
//    const VF32    nearlyZero( Const<VF32>::Min() );
//    const VF32    errorResult( -Const<VF32>::Max() );
//    const VF32    num( n | ( p - start ) );
//    const VF32    rcpDenom( Rcp( denom ) );
//    const VF32    result( num * rcpDenom );
//    return Select( Abs( denom ) > nearlyZero, result, errorResult );
//}

/// Plane CoPlanar: Determines if the given planes are coplanar within the given tolerances
NOALIAS __forceinline const VBool CoPlanar( const Plane PCREF a, const Plane PCREF b, const VF32 VECREF normalTolerance, const VF32 VECREF offsetTolerance )
{
    return ( ( Normal(a) | Normal(b) ) > normalTolerance ) &
           ( ( DistanceToOrigin(a) - DistanceToOrigin(b) ) < offsetTolerance );
}

/// Translate Point onto Plane: Returns the given point translated along the plane's normal such that it lies on the plane
NOALIAS __forceinline const Point3 TranslateOntoPlane( const Point3 PCREF point, const Plane PCREF plane )
{
    //return point - ( Normal( plane ) * Distance( plane, point ) );
    return NegMulSub( Normal( plane ), Distance( plane, point ), point );
}

/// Project vector onto plane: Returns the given vector with any component in the direction of the plane's normal cancelled out
NOALIAS __forceinline const Vector3 ProjectOntoPlane( const Vector3 PCREF vector, const Plane PCREF plane )
{
    const Normal3 normal( Normal( plane ) );
    //return vector - ( normal * ( vector | normal ) );
    return NegMulSub( normal, vector | normal, vector );
}

/// Project Point3 onto Plane
NOALIAS __forceinline const Point3 Project( const Point3 VECREF point, const Plane VECREF plane )
{
    return point - ( Normal( plane ) * Distance( plane, point ) );
}

/// Plane Serialise to stream
template < typename T_Stream >
__forceinline void Serialise( const Plane PCREF plane, T_Stream& stream )
{
    stream << plane.m_V;
}    

/// Plane Deserialise from stream
template < typename T_Stream >
__forceinline void Deserialise( Plane& plane, T_Stream& stream )
{
    stream >> plane.m_V;
}








/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

