#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_CONST_INL
#define SOFT_MATHS_HVEC_NEW_GA_CONST_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_CONST_INL
#define MATHS_HVEC_NEW_GA_CONST_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup GA
/// @{

NOALIAS __forceinline const GA::Origin Const< GA::Origin >::One()      { return m_One; }
NOALIAS __forceinline const GA::Origin Const< GA::Origin >::MinusOne() { return m_MinusOne; }

NOALIAS __forceinline const GA::Infinity Const< GA::Infinity >::One()      { return m_One; }
NOALIAS __forceinline const GA::Infinity Const< GA::Infinity >::MinusOne() { return m_MinusOne; }

NOALIAS __forceinline const GA::PseudoScalar Const< GA::PseudoScalar >::One()     { return m_One; }
NOALIAS __forceinline const GA::PseudoScalar Const< GA::PseudoScalar >::Inverse() { return m_Inverse; }

NOALIAS __forceinline const GA::PseudoScalarE3 Const< GA::PseudoScalarE3 >::One()     { return m_One; }
NOALIAS __forceinline const GA::PseudoScalarE3 Const< GA::PseudoScalarE3 >::Inverse() { return m_Inverse; }

/// @}

namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{


/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


