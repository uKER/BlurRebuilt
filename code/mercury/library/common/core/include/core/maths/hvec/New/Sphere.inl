#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_SPHERE_INL
#define SOFT_MATHS_HVEC_NEW_SPHERE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_SPHERE_INL
#define MATHS_HVEC_NEW_SPHERE_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Sphere
/// @{

/// Make Sphere from centre & radius
NOALIAS __forceinline const Sphere MakeSphere( const Point3 PCREF centre, const VF32 VECREF radius )
{
    Sphere result;
    const VF32 zero( Const<VF32>::Zero() );
    result.m_V = MakeVec4F32( centre, Max( radius, zero ) );
    return result;
}

/// Make Sphere from AxisBox (returns the bounding sphere of the box)
NOALIAS __forceinline const Sphere MakeSphere( const AxisBox& box )
{
    return BoundingSphere( box );
}

/// Sphere get Centre
NOALIAS __forceinline const Point3 Centre( const Sphere PCREF sphere )
{
    return MakePoint3( sphere.m_V );
}

/// Sphere get Radius
NOALIAS __forceinline const VF32 Radius( const Sphere PCREF sphere )
{
    return W( sphere.m_V );
}

/// Sphere get bounding box
NOALIAS __forceinline const AxisBox BoundingBox( const Sphere PCREF sphere )
{
    AxisBox result;
    const Point3  centre( Centre( sphere ) );
    const Vector3 delta( MakeVector3( Radius( sphere ) ) );
    result.m_Min = MakeVector3( centre - delta );
    result.m_Max = MakeVector3( centre + delta );
    return result;
}

/// Sphere + Vector3
NOALIAS __forceinline const Sphere operator + ( const Sphere PCREF sphere, const Vector3 PCREF vector )
{
    return MakeSphere( Centre( sphere ) + vector, Radius( sphere ) );
}

/// Sphere - Vector3
NOALIAS __forceinline const Sphere operator - ( const Sphere PCREF sphere, const Vector3 PCREF vector )
{
    return MakeSphere( Centre( sphere ) - vector, Radius( sphere ) );
}

/// Sphere * Mat33 (NOTE: the matrix must be orthonormalised!)
NOALIAS __forceinline const Sphere operator * ( const Sphere PCREF sphere, const Mat33 MATREF matrix33 )
{
    return MakeSphere( Centre( sphere ) * matrix33, Radius( sphere ) );
}

/// Sphere * Mat43 (NOTE: the rotation part of matrix43 must be orthonormalised!)
NOALIAS __forceinline const Sphere operator * ( const Sphere PCREF sphere, const Mat43 MATREF matrix43 )
{
    return MakeSphere( Centre( sphere ) * matrix43, Radius( sphere ) );
}

/// Sphere Transform: sphere * quaternion + vector
NOALIAS __forceinline const Sphere Transform( const Sphere PCREF sphere, const Quaternion PCREF quaternion, const Vector3 PCREF vector )
{
	return MakeSphere( Centre( sphere ) * quaternion + vector, Radius( sphere ) );
}

/// Sphere Serialise to stream
template < typename T_Stream >
__forceinline void Serialise( const Sphere PCREF sphere, T_Stream& stream )
{
    stream << sphere.m_V;
}    

/// Sphere Deserialise from stream
template < typename T_Stream >
__forceinline void Deserialise( Sphere& sphere, T_Stream& stream )
{
    stream >> sphere.m_V;
}






/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

