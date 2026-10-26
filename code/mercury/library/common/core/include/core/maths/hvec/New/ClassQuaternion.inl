#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_QUATERNION_INL
#define SOFT_MATHS_HVEC_NEW_CLASS_QUATERNION_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_QUATERNION_INL
#define MATHS_HVEC_NEW_CLASS_QUATERNION_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Quaternion_Classes
/// @{

/// QuatTrans Copy Constructor
NOALIAS __forceinline QuatTrans::QuatTrans( const QuatTrans& copy )
    : m_Q( copy.m_Q )
    , m_T( copy.m_T )
{
}

/// QuatTrans Copy Assignment Operator
NOALIAS __forceinline const QuatTrans& QuatTrans::operator = ( const QuatTrans& copy )
{
    m_Q = copy.m_Q;
    m_T = copy.m_T;
    return copy;
}

/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


