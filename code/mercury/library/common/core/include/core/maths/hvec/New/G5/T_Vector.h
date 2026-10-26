#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_T_VECTOR_H
#define SOFT_MATHS_HVEC_NEW_GA_T_VECTOR_H
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_T_VECTOR_H
#define MATHS_HVEC_NEW_GA_T_VECTOR_H
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace GA /// 5d Geometric Algebra
{
/// @addtogroup GA
/// @{

#if defined( USER_JDENTON )
#define MATHS_GA_CODEGEN
#endif

#if !defined( GA_FUNCTION_INLINE )
//#define GA_FUNCTION_INLINE __forceinline
#define GA_FUNCTION_INLINE inline
#endif

#if !defined( GA_OPERATOR_INLINE )
//#define GA_OPERATOR_INLINE __forceinline
#define GA_OPERATOR_INLINE inline
#endif

#if !defined( GA_CODEGEN_INLINE )
//#define GA_CODEGEN_INLINE __forceinline
#define GA_CODEGEN_INLINE inline
#endif


/// T_Vector Get Basis Element (by ElementBits index)
template < BasisBits t_Basis, ElementBits t_ElementBitfield, unsigned int t_Dimensions, typename t_Metric >
NOALIAS __forceinline const VF32 Element( const T_Vector< t_ElementBitfield, t_Dimensions, t_Metric >& v );

/// T_Vector Set Basis Element (by ElementBits index)
template < BasisBits t_Basis, ElementBits t_ElementBitfield, unsigned int t_Dimensions, typename t_Metric >
NOALIAS __forceinline void SetElement( T_Vector< t_ElementBitfield, t_Dimensions, t_Metric >* const __restrict pV, const VF32 VECREF e );


/// T_Vector Print
template < ElementBits t_ElementBitfield, unsigned int t_Dimensions, typename t_Metric >
NOALIAS __forceinline void Print( const T_Vector< t_ElementBitfield, t_Dimensions, t_Metric >& v );





/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


