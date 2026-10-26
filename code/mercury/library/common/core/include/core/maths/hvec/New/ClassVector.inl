#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_VECTOR_INL
#define SOFT_MATHS_HVEC_NEW_CLASS_VECTOR_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_VECTOR_INL
#define MATHS_HVEC_NEW_CLASS_VECTOR_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Vector_Classes
/// @{

#if !defined( PASSINREG_ENABLED )

NOALIAS __forceinline Vec4F32::Vec4F32( const Vec4F32& copy )
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

NOALIAS __forceinline const Vec4F32& Vec4F32::operator = ( const Vec4F32& copy )
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

#endif


NOALIAS __forceinline Vec8F32::Vec8F32( const Vec8F32& copy )
    : m_V0( copy.m_V0 )
    , m_V1( copy.m_V1 )
{
}

NOALIAS __forceinline const Vec8F32& Vec8F32::operator = ( const Vec8F32& copy )
{
    m_V0 = copy.m_V0;
    m_V1 = copy.m_V1;
    return copy;
}


NOALIAS __forceinline Vec12F32::Vec12F32( const Vec12F32& copy )
    : m_V0( copy.m_V0 )
    , m_V1( copy.m_V1 )
{
}

NOALIAS __forceinline const Vec12F32& Vec12F32::operator = ( const Vec12F32& copy )
{
    m_V0 = copy.m_V0;
    m_V1 = copy.m_V1;
    return copy;
}


NOALIAS __forceinline Vec16F32::Vec16F32( const Vec16F32& copy )
    : m_V0( copy.m_V0 )
    , m_V1( copy.m_V1 )
{
}

NOALIAS __forceinline const Vec16F32& Vec16F32::operator = ( const Vec16F32& copy )
{
    m_V0 = copy.m_V0;
    m_V1 = copy.m_V1;
    return copy;
}


NOALIAS __forceinline Vector5::Vector5( const Vector5& copy )
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
    #endif
}

NOALIAS __forceinline const Vector5& Vector5::operator = ( const Vector5& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[0] = copy.m_V[0];
    m_V[1] = copy.m_V[1];
    m_V[2] = copy.m_V[2];
    m_V[3] = copy.m_V[3];
    m_V[4] = copy.m_V[4];
    #endif
    return copy;
}


NOALIAS __forceinline Vector6::Vector6( const Vector6& copy )
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
    #endif
}

NOALIAS __forceinline const Vector6& Vector6::operator = ( const Vector6& copy )
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
    #endif
    return copy;
}


NOALIAS __forceinline Vector7::Vector7( const Vector7& copy )
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
    #endif
}

NOALIAS __forceinline const Vector7& Vector7::operator = ( const Vector7& copy )
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
    #endif
    return copy;
}


NOALIAS __forceinline Vector8::Vector8( const Vector8& copy )
    : m_V( copy.m_V )
{
}

NOALIAS __forceinline const Vector8& Vector8::operator = ( const Vector8& copy )
{
    m_V = copy.m_V;
    return copy;
}




#if !defined( PASSINREG_ENABLED )

NOALIAS __forceinline Vec16S8::Vec16S8( const Vec16S8& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[0x0] = copy.m_V[0x0]; m_V[0x1] = copy.m_V[0x1]; m_V[0x2] = copy.m_V[0x2]; m_V[0x3] = copy.m_V[0x3];
    m_V[0x4] = copy.m_V[0x4]; m_V[0x5] = copy.m_V[0x5]; m_V[0x6] = copy.m_V[0x6]; m_V[0x7] = copy.m_V[0x7];
    m_V[0x8] = copy.m_V[0x8]; m_V[0x9] = copy.m_V[0x9]; m_V[0xA] = copy.m_V[0xA]; m_V[0xB] = copy.m_V[0xB];
    m_V[0xC] = copy.m_V[0xC]; m_V[0xD] = copy.m_V[0xD]; m_V[0xE] = copy.m_V[0xE]; m_V[0xF] = copy.m_V[0xF];
    #endif
}

NOALIAS __forceinline const Vec16S8& Vec16S8::operator = ( const Vec16S8& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[0x0] = copy.m_V[0x0]; m_V[0x1] = copy.m_V[0x1]; m_V[0x2] = copy.m_V[0x2]; m_V[0x3] = copy.m_V[0x3];
    m_V[0x4] = copy.m_V[0x4]; m_V[0x5] = copy.m_V[0x5]; m_V[0x6] = copy.m_V[0x6]; m_V[0x7] = copy.m_V[0x7];
    m_V[0x8] = copy.m_V[0x8]; m_V[0x9] = copy.m_V[0x9]; m_V[0xA] = copy.m_V[0xA]; m_V[0xB] = copy.m_V[0xB];
    m_V[0xC] = copy.m_V[0xC]; m_V[0xD] = copy.m_V[0xD]; m_V[0xE] = copy.m_V[0xE]; m_V[0xF] = copy.m_V[0xF];
    #endif
    return copy;
}

NOALIAS __forceinline Vec16U8::Vec16U8( const Vec16U8& copy )
    #if defined( _SIMD_ )
    : m_V( copy.m_V )
    #endif
{
    #if !defined( _SIMD_ )
    m_V[0x0] = copy.m_V[0x0]; m_V[0x1] = copy.m_V[0x1]; m_V[0x2] = copy.m_V[0x2]; m_V[0x3] = copy.m_V[0x3];
    m_V[0x4] = copy.m_V[0x4]; m_V[0x5] = copy.m_V[0x5]; m_V[0x6] = copy.m_V[0x6]; m_V[0x7] = copy.m_V[0x7];
    m_V[0x8] = copy.m_V[0x8]; m_V[0x9] = copy.m_V[0x9]; m_V[0xA] = copy.m_V[0xA]; m_V[0xB] = copy.m_V[0xB];
    m_V[0xC] = copy.m_V[0xC]; m_V[0xD] = copy.m_V[0xD]; m_V[0xE] = copy.m_V[0xE]; m_V[0xF] = copy.m_V[0xF];
    #endif
}

NOALIAS __forceinline const Vec16U8& Vec16U8::operator = ( const Vec16U8& copy )
{
    #if defined( _SIMD_ )
    m_V = copy.m_V;
    #else
    m_V[0x0] = copy.m_V[0x0]; m_V[0x1] = copy.m_V[0x1]; m_V[0x2] = copy.m_V[0x2]; m_V[0x3] = copy.m_V[0x3];
    m_V[0x4] = copy.m_V[0x4]; m_V[0x5] = copy.m_V[0x5]; m_V[0x6] = copy.m_V[0x6]; m_V[0x7] = copy.m_V[0x7];
    m_V[0x8] = copy.m_V[0x8]; m_V[0x9] = copy.m_V[0x9]; m_V[0xA] = copy.m_V[0xA]; m_V[0xB] = copy.m_V[0xB];
    m_V[0xC] = copy.m_V[0xC]; m_V[0xD] = copy.m_V[0xD]; m_V[0xE] = copy.m_V[0xE]; m_V[0xF] = copy.m_V[0xF];
    #endif
    return copy;
}



NOALIAS __forceinline Vec8S16::Vec8S16( const Vec8S16& copy )
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

NOALIAS __forceinline const Vec8S16& Vec8S16::operator = ( const Vec8S16& copy )
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

NOALIAS __forceinline Vec8U16::Vec8U16( const Vec8U16& copy )
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

NOALIAS __forceinline const Vec8U16& Vec8U16::operator = ( const Vec8U16& copy )
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




NOALIAS __forceinline Vec4S32::Vec4S32( const Vec4S32& copy )
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

NOALIAS __forceinline const Vec4S32& Vec4S32::operator = ( const Vec4S32& copy )
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

NOALIAS __forceinline Vec4U32::Vec4U32( const Vec4U32& copy )
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

NOALIAS __forceinline const Vec4U32& Vec4U32::operator = ( const Vec4U32& copy )
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





//NOALIAS __forceinline Vec2S64::Vec2S64( const Vec2S64& copy )
//    #if defined( _SPU_ ) || defined( _SSE_ )
//    : m_V( copy.m_V )
//    #endif
//{
//    #if !defined( _SPU_ ) && !defined( _SSE_ )
//    m_V[0] = copy.m_V[0];
//    m_V[1] = copy.m_V[1];
//    #endif
//}
//
//NOALIAS __forceinline const Vec2S64& Vec2S64::operator = ( const Vec2S64& copy )
//{
//    #if defined( _SPU_ ) || defined( _SSE_ )
//    m_V = copy.m_V;
//    #else
//    m_V[0] = copy.m_V[0];
//    m_V[1] = copy.m_V[1];
//    #endif
//    return copy;
//}
//
//NOALIAS __forceinline Vec2U64::Vec2U64( const Vec2U64& copy )
//    #if defined( _SPU_ ) || defined( _SSE_ )
//    : m_V( copy.m_V )
//    #endif
//{
//    #if !defined( _SPU_ ) && !defined( _SSE_ )
//    m_V[0] = copy.m_V[0];
//    m_V[1] = copy.m_V[1];
//    #endif
//}
//
//NOALIAS __forceinline const Vec2U64& Vec2U64::operator = ( const Vec2U64& copy )
//{
//    #if defined( _SPU_ ) || defined( _SSE_ )
//    m_V = copy.m_V;
//    #else
//    m_V[0] = copy.m_V[0];
//    m_V[1] = copy.m_V[1];
//    #endif
//    return copy;
//}

#endif // #if !defined( PASSINREG_ENABLED )

/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

