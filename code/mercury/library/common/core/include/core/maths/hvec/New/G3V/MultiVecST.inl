#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_ST_INL
#define SOFT_MATHS_HVEC_NEW_G3V_MULTIVEC_ST_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_MULTIVEC_ST_INL
#define MATHS_HVEC_NEW_G3V_MULTIVEC_ST_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V_MultiVecST
/// @{

/// Make MultiVecST from Scalar & TriVector
NOALIAS __forceinline const MultiVecST MakeMultiVecST( const VF32 VECREF s, const TriVector PCREF t )
{
    MultiVecST result;
    result.m_ST = MakeVec4F32( s, t.m_T );
    return result;
}

/// Get Scalar from MultiVecST
NOALIAS __forceinline const VF32 GetScalar( const MultiVecST PCREF st )
{
    return X( st.m_ST );
}

/// Get TriVector from MultiVecST
NOALIAS __forceinline const TriVector GetTriVector( const MultiVecST PCREF st )
{
    return MakeTriVector( Y( st.m_ST ) );
}




/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


