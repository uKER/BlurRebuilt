#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_BOOL_INL
#define SOFT_MATHS_HVEC_NEW_CLASS_BOOL_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_BOOL_INL
#define MATHS_HVEC_NEW_CLASS_BOOL_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean_Classes
/// @{

#if !defined( PASSINREG_ENABLED )

NOALIAS __forceinline Vec4Bool::Vec4Bool( const Vec4Bool& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[0] = copy.m_V[0];
    m_V[1] = copy.m_V[1];
    m_V[2] = copy.m_V[2];
    m_V[3] = copy.m_V[3];
    #endif
}

NOALIAS __forceinline const Vec4Bool& Vec4Bool::operator = ( const Vec4Bool& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[0] = copy.m_V[0];
    m_V[1] = copy.m_V[1];
    m_V[2] = copy.m_V[2];
    m_V[3] = copy.m_V[3];
    #endif
    return copy;
}


NOALIAS __forceinline Vec8Bool::Vec8Bool( const Vec8Bool& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[0] = copy.m_V[0];
    m_V[1] = copy.m_V[1];
    m_V[2] = copy.m_V[2];
    m_V[3] = copy.m_V[3];
    m_V[4] = copy.m_V[4];
    m_V[5] = copy.m_V[5];
    m_V[6] = copy.m_V[6];
    m_V[7] = copy.m_V[7];
    #endif
}

NOALIAS __forceinline const Vec8Bool& Vec8Bool::operator = ( const Vec8Bool& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[0] = copy.m_V[0];
    m_V[1] = copy.m_V[1];
    m_V[2] = copy.m_V[2];
    m_V[3] = copy.m_V[3];
    m_V[4] = copy.m_V[4];
    m_V[5] = copy.m_V[5];
    m_V[6] = copy.m_V[6];
    m_V[7] = copy.m_V[7];
    #endif
    return copy;
}


NOALIAS __forceinline Vec16Bool::Vec16Bool( const Vec16Bool& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[0x0] = copy.m_V[0x0];
    m_V[0x1] = copy.m_V[0x1];
    m_V[0x2] = copy.m_V[0x2];
    m_V[0x3] = copy.m_V[0x3];
    m_V[0x4] = copy.m_V[0x4];
    m_V[0x5] = copy.m_V[0x5];
    m_V[0x6] = copy.m_V[0x6];
    m_V[0x7] = copy.m_V[0x7];
    m_V[0x8] = copy.m_V[0x8];
    m_V[0x9] = copy.m_V[0x9];
    m_V[0xA] = copy.m_V[0xA];
    m_V[0xB] = copy.m_V[0xB];
    m_V[0xC] = copy.m_V[0xC];
    m_V[0xD] = copy.m_V[0xD];
    m_V[0xE] = copy.m_V[0xE];
    m_V[0xF] = copy.m_V[0xF];
    #endif
}

NOALIAS __forceinline const Vec16Bool& Vec16Bool::operator = ( const Vec16Bool& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[0x0] = copy.m_V[0x0];
    m_V[0x1] = copy.m_V[0x1];
    m_V[0x2] = copy.m_V[0x2];
    m_V[0x3] = copy.m_V[0x3];
    m_V[0x4] = copy.m_V[0x4];
    m_V[0x5] = copy.m_V[0x5];
    m_V[0x6] = copy.m_V[0x6];
    m_V[0x7] = copy.m_V[0x7];
    m_V[0x8] = copy.m_V[0x8];
    m_V[0x9] = copy.m_V[0x9];
    m_V[0xA] = copy.m_V[0xA];
    m_V[0xB] = copy.m_V[0xB];
    m_V[0xC] = copy.m_V[0xC];
    m_V[0xD] = copy.m_V[0xD];
    m_V[0xE] = copy.m_V[0xE];
    m_V[0xF] = copy.m_V[0xF];
    #endif
    return copy;
}

#endif

/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

