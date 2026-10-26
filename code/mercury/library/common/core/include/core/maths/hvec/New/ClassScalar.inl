#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_SCALAR_INL
#define SOFT_MATHS_HVEC_NEW_CLASS_SCALAR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_SCALAR_INL
#define MATHS_HVEC_NEW_CLASS_SCALAR_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Scalar_Classes
/// @{

NOALIAS __forceinline VF64::VF64( const VF64& copy )
    : m_V( copy.m_V )
{
}

NOALIAS __forceinline const VF64& VF64::operator = ( const VF64& copy )
{
    m_V = copy.m_V;
    return copy;
}

/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

