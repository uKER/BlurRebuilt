#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_SCALEMAT3_INL
#define SOFT_MATHS_HVEC_NEW_SCALEMAT3_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_SCALEMAT3_INL
#define MATHS_HVEC_NEW_SCALEMAT3_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup ScaleMat3
/// @{

/// Make ScaleMat3 from 3 float elements
NOALIAS __forceinline const ScaleMat3 MakeScaleMat3( const F32& sx, const F32& sy, const F32& sz )
{
    ScaleMat3 result;
    result.m_Scale = MakeVector3( sx, sy, sz );
    return result;
}

/// Make ScaleMat3 from 3 VF32 elements
NOALIAS __forceinline const ScaleMat3 MakeScaleMat3( const VF32 VECREF sx, const VF32 VECREF sy, const VF32 VECREF sz )
{
    ScaleMat3 result;
    result.m_Scale = MakeVector3( sx, sy, sz );
    return result;
}

/// Make ScaleMat3 from a diagonal vector (3 element)
NOALIAS __forceinline const ScaleMat3 MakeScaleMat3( const Vector3 PCREF scale )
{
    ScaleMat3 result;
    result.m_Scale = scale;
    return result;
}

/// Constant ScaleMat3(0,0,0)
NOALIAS __forceinline const ScaleMat3 Const< ScaleMat3 >::Zero()
{
    return MakeScaleMat3( Const<Vector3>::Zero() );
}

/// Constant ScaleMat3(1,1,1)
NOALIAS __forceinline const ScaleMat3 Const< ScaleMat3 >::Identity()
{
    return MakeScaleMat3( Const<Vector3>::One() );
}

/// Get Vector3 from a ScaleMat3
NOALIAS __forceinline const Vector3 GetScale( const ScaleMat3 PCREF m )
{
    return m.m_Scale;
}

/// Set Diagonal vector of a ScaleMat3
NOALIAS __forceinline void SetScale( ScaleMat3* pDst, const Vector3 PCREF scale )
{
    pDst->m_Scale = scale;
}

/// Get SX from a ScaleMat3
NOALIAS __forceinline const VF32 GetSX( const ScaleMat3 PCREF m )
{
    return X( m.m_Scale );
}

/// Get SY from a ScaleMat3
NOALIAS __forceinline const VF32 GetSY( const ScaleMat3 PCREF m )
{
    return Y( m.m_Scale );
}

/// Get SZ from a ScaleMat3
NOALIAS __forceinline const VF32 GetSZ( const ScaleMat3 PCREF m )
{
    return Z( m.m_Scale );
}

/// Get SX from a ScaleMat3
NOALIAS __forceinline const VF32 X( const ScaleMat3 PCREF m )
{
    return X( m.m_Scale );
}

/// Get SY from a ScaleMat3
NOALIAS __forceinline const VF32 Y( const ScaleMat3 PCREF m )
{
    return Y( m.m_Scale );
}

/// Get SZ from a ScaleMat3
NOALIAS __forceinline const VF32 Z( const ScaleMat3 PCREF m )
{
    return Z( m.m_Scale );
}

/// ScaleMat3 Inverse (faster but only 22 bit precision)
NOALIAS __forceinline const ScaleMat3 Inverse( const ScaleMat3 PCREF m )
{
    ScaleMat3 result;
    result.m_Scale = Rcp( m.m_Scale );
    return result;
}

/// ScaleMat3 Inverse (slower but full 24 bit precision)
NOALIAS __forceinline const ScaleMat3 Inverse24( const ScaleMat3 PCREF m )
{
    ScaleMat3 result;
    result.m_Scale = Rcp24( m.m_Scale );
    return result;
}


/// ScaleMat3 Finite test
NOALIAS __forceinline const VBool Finite( const ScaleMat3 PCREF m )
{
    return Finite( m.m_Scale );
}


/// ScaleMat3 * VF32 Right Multiplication
NOALIAS __forceinline const ScaleMat3 operator * ( const ScaleMat3 PCREF a, const VF32 VECREF b )
{
    ScaleMat3 result;
    result.m_Scale = a.m_Scale * b;
    return result;
}

/// ScaleMat3 * VF32 Left Multiplication
NOALIAS __forceinline const ScaleMat3 operator * ( const VF32 VECREF a, const ScaleMat3 PCREF b )
{
    ScaleMat3 result;
    result.m_Scale = a * b.m_Scale;
    return result;
}


/// ScaleMat3 * Mat33 Right Multiplication
NOALIAS __forceinline const Mat33 operator * ( const ScaleMat3 PCREF a, const Mat33 MATREF b )
{
    return MakeMat33( a ) * b;
}

/// ScaleMat3 * Mat33 Left Multiplication
NOALIAS __forceinline const Mat33 operator * ( const Mat33 MATREF a, const ScaleMat3 PCREF b )
{
    return a * MakeMat33( b );
}


/// ScaleMat3 * ShearMat3 Right Multiplication
NOALIAS __forceinline const Mat33 operator * ( const ScaleMat3 PCREF a, const ShearMat3& b )
{
    return MakeMat33( a ) * MakeMat33( b );
}

/// ScaleMat3 * ShearMat3 Left Multiplication
NOALIAS __forceinline const Mat33 operator * ( const ShearMat3& a, const ScaleMat3 PCREF b )
{
    return MakeMat33( a ) * MakeMat33( b );
}






/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


