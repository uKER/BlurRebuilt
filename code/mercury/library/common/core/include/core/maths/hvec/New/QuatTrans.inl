#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_QUAT_TRANS_INL
#define SOFT_MATHS_HVEC_NEW_QUAT_TRANS_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_QUAT_TRANS_INL
#define MATHS_HVEC_NEW_QUAT_TRANS_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup QuatTrans
/// @{

/// QuatTrans Get Quaternion
NOALIAS __forceinline const Quaternion Quat( const QuatTrans& qt )
{
    return qt.m_Q;
}

/// QuatTrans Get Translation
NOALIAS __forceinline const Vector3 Trans( const QuatTrans& qt )
{
    return qt.m_T;
}

/// QuatTrans Set Quaternion
NOALIAS __forceinline void SetQuat( QuatTrans* const __restrict pQT, const Quaternion PCREF q )
{
    pQT->m_Q = q;
}

/// QuatTrans Set Translation
NOALIAS __forceinline void SetTrans( QuatTrans* const __restrict pQT, const Vector3 PCREF t )
{
    pQT->m_T = t;
}

/// Make QuatTrans from Mat43
NOALIAS __forceinline const QuatTrans MakeQuatTrans( const Mat43 MATREF m43 )
{
    QuatTrans result;
    result.m_Q = MakeUnitQuaternion( MakeMat33( m43 ) );
    result.m_T = RowW( m43 );
    return result;
}

/// Make QuatTrans from Quaternion and Vector3
NOALIAS __forceinline const QuatTrans MakeQuatTrans( const Quaternion PCREF q, const Vector3 PCREF t )
{
    QuatTrans result;
    result.m_Q = q;
    result.m_T = t;
    return result;
}

/// Make QuatTrans from a DualQuaternion
NOALIAS __forceinline const QuatTrans MakeQuatTrans( const DualQuaternion& dq )
{
    QuatTrans result;
    result.m_Q = Quat( dq );
    result.m_T = Trans( dq );
    return result;
}


/// Make Mat43 from QuatTrans
NOALIAS __forceinline const Mat43 MakeMat43( const QuatTrans& qt )
{
    return MakeMat43( MakeMat33( qt.m_Q ), qt.m_T );
}

/// QuatTrans Nearest: Returns whichever one of the two equivalent destinations is nearest to the source
NOALIAS __forceinline const QuatTrans Nearest( const QuatTrans& src, const QuatTrans& dst )
{
    QuatTrans result;
    result.m_Q = Nearest( src.m_Q, dst.m_Q );
    result.m_T = dst.m_T;
    return result;
}

/// QuatTrans Multiplication Operator: Vector3 * QuatTrans
NOALIAS __forceinline const Vector3 operator * ( const Vector3 PCREF v, const QuatTrans& qt )
{
    return v * qt.m_Q;
}

/// QuatTrans Multiplication Operator: Point3 * QuatTrans
NOALIAS __forceinline const Point3 operator * ( const Point3 PCREF v, const QuatTrans& qt )
{
    return ( v * qt.m_Q ) + qt.m_T;
}

/// QuatTrans Multiplication Operator: QuatTrans * QuatTrans
NOALIAS __forceinline const QuatTrans operator * ( const QuatTrans& a, const QuatTrans& b )
{
    QuatTrans result;
    result.m_Q = a.m_Q * b.m_Q;
    result.m_T = ( a.m_T * b.m_Q ) + b.m_T;
    return result;
}

/// QuatTrans Inverse
NOALIAS __forceinline const QuatTrans Inverse( const QuatTrans& qt )
{
    const Quaternion qConjugate( !qt.m_Q );
    QuatTrans result;
    result.m_Q = qConjugate;
    result.m_T = -qt.m_T * qConjugate;
    return result;
}

/// QuatTrans Normalise
NOALIAS __forceinline const QuatTrans Normalise( const QuatTrans& a )
{
    QuatTrans result;
    result.m_Q = Normalise( a.m_Q );
    result.m_T = a.m_T;
    return result;
}


/// QuatTrans Lerp: Linear Interpolation
NOALIAS __forceinline const QuatTrans Lerp( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t )
{
    /// @par NOTE: t must be in the range [ 0.0f, 1.0f ]
    QuatTrans result;
    result.m_Q = Lerp( src.m_Q, dst.m_Q, t );
    result.m_T = src.m_T + ( ( dst.m_T - src.m_T ) * t );
    return result;
}

/// QuatTrans Lerp: Linear Interpolation with normalisation
NOALIAS __forceinline const QuatTrans LerpNormalised( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t )
{
    /// @par NOTE: t must be in the range [ 0.0f, 1.0f ]
    QuatTrans result;
    result.m_Q = LerpNormalised( src.m_Q, dst.m_Q, t );
    result.m_T = src.m_T + ( ( dst.m_T - src.m_T ) * t );
    return result;
}

/// QuatTrans Nearest Lerp: Nearest Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const QuatTrans NearestLerp( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t )
{
    /// @par NOTE: t must be in the range [ 0.0f, 1.0f ]
    QuatTrans result;
    result.m_Q = NearestLerp( src.m_Q, dst.m_Q, t );
    result.m_T = src.m_T + ( ( dst.m_T - src.m_T ) * t );
    return result;
}

/// QuatTrans Nearest Lerp: Nearest Linear Interpolation - Negates dst if necessary - with normalisation
NOALIAS __forceinline const QuatTrans NearestLerpNormalised( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t )
{
    /// @par NOTE: t must be in the range [ 0.0f, 1.0f ]
    QuatTrans result;
    result.m_Q = NearestLerpNormalised( src.m_Q, dst.m_Q, t );
    result.m_T = src.m_T + ( ( dst.m_T - src.m_T ) * t );
    return result;
}

/// QuatTrans Slerp: Spherical Linear Interpolation
NOALIAS __forceinline const QuatTrans Slerp( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t )
{
    /// @par NOTE: t must be in the range [ 0.0f, 1.0f ]
    QuatTrans result;
    result.m_Q = Slerp( src.m_Q, dst.m_Q, t );
    result.m_T = src.m_T + ( ( dst.m_T - src.m_T ) * t );
    return result;
}

/// QuatTrans Nearest Slerp: Spherical Linear Interpolation - Negates dst if necessary
NOALIAS __forceinline const QuatTrans NearestSlerp( const QuatTrans& src, const QuatTrans& dst, const VF32 VECREF t )
{
    /// @par NOTE: t must be in the range [ 0.0f, 1.0f ]
    QuatTrans result;
    result.m_Q = NearestSlerp( src.m_Q, dst.m_Q, t );
    result.m_T = src.m_T + ( ( dst.m_T - src.m_T ) * t );
    return result;
}



/// QuatTrans Serialise to Stream
template < typename T_Stream > void Serialise( const QuatTrans& src, T_Stream& stream )
{
    Serialise( src.m_Q, stream );
    Serialise( src.m_T, stream );
}

/// QuatTrans Deserialise from Stream
template < typename T_Stream > void Deserialise( T_Stream& stream, QuatTrans* const __restrict pDst )
{
    Deserialise( stream, &pDst->m_Q );
    Deserialise( stream, &pDst->m_T );
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

