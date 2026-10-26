#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_GA_TRANSLATOR_INL
#define SOFT_MATHS_HVEC_NEW_GA_TRANSLATOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_GA_TRANSLATOR_INL
#define MATHS_HVEC_NEW_GA_TRANSLATOR_INL
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

NOALIAS __forceinline const Translator MakeTranslator( const Vector& t )
{
    const Scalar one( Const<VF32>::One() );
    const Infinity halfInfinity( Const<VF32>::Half() );
    return one - ( t * halfInfinity );
}




/// @}
}; // namespace GA
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


