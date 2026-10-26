#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_INVERSE_INL
#define SOFT_MATHS_HVEC_NEW_GA_INVERSE_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_INVERSE_INL
#define MATHS_HVEC_NEW_GA_INVERSE_INL
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

/// T_Vector Inverse
template < ElementBits  t_Element,
           unsigned int t_Dimensions,
           typename     t_Metric >
NOALIAS GA_FUNCTION_INLINE
    const T_Vector< t_Element, t_Dimensions, t_Metric >
    Inverse( const T_Vector< t_Element, t_Dimensions, t_Metric >& src )
{
    const T_Vector< t_Element, t_Dimensions, t_Metric > reverse( Reverse( src ) );
    return reverse * Rcp( src % reverse );
}









/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


