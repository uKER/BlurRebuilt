#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_DUAL_INL
#define SOFT_MATHS_HVEC_NEW_CLASS_DUAL_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_DUAL_INL
#define MATHS_HVEC_NEW_CLASS_DUAL_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup DualScalar
/// @{

/// DualVector3 Copy Constructor
//NOALIAS __forceinline DualVector3::DualVector3( const DualVector3& copy )
//    : m_V0( copy.m_V0 )
//    , m_Ve( copy.m_Ve )
//{
//}

/// DualVector3 Copy Assignment Operator
//NOALIAS __forceinline const DualVector3& DualVector3::operator = ( const DualVector3& copy )
//{
//    m_V0 = copy.m_V0;
//    m_Ve = copy.m_Ve;
//    return copy;
//}


/// DualQuaternion Copy Constructor
NOALIAS __forceinline DualQuaternion::DualQuaternion( const DualQuaternion& copy )
    : m_V( copy.m_V )
{
}

/// DualQuaternion Copy Assignment Operator
NOALIAS __forceinline const DualQuaternion& DualQuaternion::operator = ( const DualQuaternion& copy )
{
    m_V = copy.m_V;
    return copy;
}





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


