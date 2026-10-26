#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVECTOR_INL
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVECTOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVECTOR_INL
#define MATHS_HVEC_NEW_G3V_MULTIVECTOR_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVector
/// @{

/// Make MultiVector from Scalar, Vector, BiVector & TriVector
NOALIAS __forceinline const MultiVector MakeMultiVector( const VF32 VECREF s, const Vector PCREF v, const BiVector PCREF b, const TriVector PCREF t )
{
    MultiVector result;
    result.m_VS = MakeVector4( v.m_V, s );
    result.m_BT = MakeVector4( b.m_B, t.m_T );
    return result;
}

/// Make MultiVector from a Vector8
NOALIAS __forceinline const MultiVector MakeMultiVector( const Vector8& v8 )
{
    MultiVector result;
    result.m_VS.m_V = v8.m_V.m_V0;
    result.m_BT.m_V = v8.m_V.m_V1;
    return result;
}

/// Get Vector8 from MultiVector
NOALIAS __forceinline const Vector8 GetVector8( const MultiVector& mv )
{
    Vector8 result;
    result.m_V.m_V0 = mv.m_VS.m_V;
    result.m_V.m_V1 = mv.m_BT.m_V;
    return result;
}

/// Get Scalar from MultiVector
NOALIAS __forceinline const VF32 GetScalar( const MultiVector& mv )
{
    return W( mv.m_VS );
}

/// Get Vector from MultiVector
NOALIAS __forceinline const Vector GetVector( const MultiVector& mv )
{
    Vector result;
    result.m_V = MakeVector3( mv.m_VS );
    return result;
}

/// Get BiVector from MultiVector
NOALIAS __forceinline const BiVector GetBiVector( const MultiVector& mv )
{
    BiVector result;
    result.m_B = MakeVector3( mv.m_BT );
    return result;
}

/// Get TriVector from MultiVector
NOALIAS __forceinline const TriVector GetTriVector( const MultiVector& mv )
{
    TriVector result;
    result.m_T = W( mv.m_BT );
    return result;
}


/// MultiVector Get Element
template < MultiVectorElementIndex I >
NOALIAS __forceinline const VF32 Element( const MultiVector& mv )
{
    Vector8 v8;
    v8.m_V.m_V0 = mv.m_VS.m_V;
    v8.m_V.m_V1 = mv.m_BT.m_V;
    return Maths::Element<I>( v8 );
}


/// MultiVector Permute : Indices select 8 result elements [I0,I1,I2,I3,I4,I5,I6,I7] from [ 0:e1, 1:e2, 2:e3, 3:s, 4:e12, 5:e23, 6:e31, 7:e123 ]
template < MultiVectorElementIndex I0,
           MultiVectorElementIndex I1,
           MultiVectorElementIndex I2,
           MultiVectorElementIndex I3,
           MultiVectorElementIndex I4,
           MultiVectorElementIndex I5,
           MultiVectorElementIndex I6,
           MultiVectorElementIndex I7 >
NOALIAS __forceinline const Vector8 MV_Permute( const MultiVector& mv )
{
    Vector8 v8;
    v8.m_V.m_V0 = mv.m_VS.m_V;
    v8.m_V.m_V1 = mv.m_BT.m_V;
    return Permute<I0,I1,I2,I3,I4,I5,I6,I7>( v8 );
}






/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


