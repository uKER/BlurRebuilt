#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_JACOBIAN_INL
#define SOFT_MATHS_HVEC_NEW_CLASS_JACOBIAN_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_JACOBIAN_INL
#define MATHS_HVEC_NEW_CLASS_JACOBIAN_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Jacobian_Classes
/// @{

NOALIAS __forceinline Mat26::Mat26( const Mat26& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
}

NOALIAS __forceinline const Mat26& Mat26::operator = ( const Mat26& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    return copy;
}


NOALIAS __forceinline Mat36::Mat36( const Mat36& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
}

NOALIAS __forceinline const Mat36& Mat36::operator = ( const Mat36& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    return copy;
}


NOALIAS __forceinline Mat46::Mat46( const Mat46& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
}

NOALIAS __forceinline const Mat46& Mat46::operator = ( const Mat46& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    return copy;
}


NOALIAS __forceinline Mat55::Mat55( const Mat55& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
}

NOALIAS __forceinline const Mat55& Mat55::operator = ( const Mat55& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    return copy;
}


NOALIAS __forceinline Mat56::Mat56( const Mat56& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
}

NOALIAS __forceinline const Mat56& Mat56::operator = ( const Mat56& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    return copy;
}


NOALIAS __forceinline Mat66::Mat66( const Mat66& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
}

NOALIAS __forceinline const Mat66& Mat66::operator = ( const Mat66& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
    return copy;
}


NOALIAS __forceinline Mat76::Mat76( const Mat76& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
    m_Row[6] = copy.m_Row[6];
}

NOALIAS __forceinline const Mat76& Mat76::operator = ( const Mat76& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
    m_Row[6] = copy.m_Row[6];
    return copy;
}


NOALIAS __forceinline Mat62::Mat62( const Mat62& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
}

NOALIAS __forceinline const Mat62& Mat62::operator = ( const Mat62& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
    return copy;
}


NOALIAS __forceinline Mat63::Mat63( const Mat63& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
}

NOALIAS __forceinline const Mat63& Mat63::operator = ( const Mat63& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
    return copy;
}


NOALIAS __forceinline Mat64::Mat64( const Mat64& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
}

NOALIAS __forceinline const Mat64& Mat64::operator = ( const Mat64& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
    return copy;
}


NOALIAS __forceinline Mat65::Mat65( const Mat65& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
}

NOALIAS __forceinline const Mat65& Mat65::operator = ( const Mat65& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
    return copy;
}


NOALIAS __forceinline Mat67::Mat67( const Mat67& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
}

NOALIAS __forceinline const Mat67& Mat67::operator = ( const Mat67& copy )
{
    m_Row[0] = copy.m_Row[0];
    m_Row[1] = copy.m_Row[1];
    m_Row[2] = copy.m_Row[2];
    m_Row[3] = copy.m_Row[3];
    m_Row[4] = copy.m_Row[4];
    m_Row[5] = copy.m_Row[5];
    return copy;
}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


