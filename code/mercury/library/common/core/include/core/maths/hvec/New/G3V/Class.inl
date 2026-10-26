#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_G3V_CLASS_INL
#define SOFT_MATHS_HVEC_NEW_G3V_CLASS_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_G3V_CLASS_INL
#define MATHS_HVEC_NEW_G3V_CLASS_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
namespace G3V
{
/// @addtogroup G3V
/// @{

NOALIAS __forceinline MultiVector::MultiVector()
{
}

NOALIAS __forceinline MultiVector::MultiVector( const MultiVector& copy )
    : m_VS( copy.m_VS )
    , m_BT( copy.m_BT )
{
}

NOALIAS __forceinline const MultiVector& MultiVector::operator = ( const MultiVector& copy )
{
    m_VS = copy.m_VS;
    m_BT = copy.m_BT;
    return copy;
}


NOALIAS __forceinline MultiVecVB::MultiVecVB()
{
}

NOALIAS __forceinline MultiVecVB::MultiVecVB( const MultiVecVB& copy )
    : m_V( copy.m_V )
    , m_B( copy.m_B )
{
}

NOALIAS __forceinline const MultiVecVB& MultiVecVB::operator = ( const MultiVecVB& copy )
{
    m_V = copy.m_V;
    m_B = copy.m_B;
    return copy;
}



/// @}
}; // namespace G3V
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


