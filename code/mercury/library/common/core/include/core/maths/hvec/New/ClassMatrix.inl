#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_MATRIX_INL
#define SOFT_MATHS_HVEC_NEW_CLASS_MATRIX_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_MATRIX_INL
#define MATHS_HVEC_NEW_CLASS_MATRIX_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Matrix_Classes
/// @{

NOALIAS __forceinline Mat33::Mat33( const Mat33& copy )
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    : m_RowX( copy.m_RowX )
    , m_RowY( copy.m_RowY )
    , m_RowZ( copy.m_RowZ )
    #elif defined( _VMX128_ )
    : m_ColX( copy.m_ColX )
    , m_ColY( copy.m_ColY )
    , m_ColZ( copy.m_ColZ )
    #endif
{
    #if !defined( _SIMD_ )
    m[0][0] = copy.m[0][0]; m[0][1] = copy.m[0][1]; m[0][2] = copy.m[0][2];
    m[1][0] = copy.m[1][0]; m[1][1] = copy.m[1][1]; m[1][2] = copy.m[1][2];
    m[2][0] = copy.m[2][0]; m[2][1] = copy.m[2][1]; m[2][2] = copy.m[2][2];
    #endif
}

NOALIAS __forceinline const Mat33& Mat33::operator = ( const Mat33& copy )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    m_RowX = copy.m_RowX;
    m_RowY = copy.m_RowY;
    m_RowZ = copy.m_RowZ;
    #elif defined( _VMX128_ )
    m_ColX = copy.m_ColX;
    m_ColY = copy.m_ColY;
    m_ColZ = copy.m_ColZ;
    #else
    m[0][0] = copy.m[0][0]; m[0][1] = copy.m[0][1]; m[0][2] = copy.m[0][2];
    m[1][0] = copy.m[1][0]; m[1][1] = copy.m[1][1]; m[1][2] = copy.m[1][2];
    m[2][0] = copy.m[2][0]; m[2][1] = copy.m[2][1]; m[2][2] = copy.m[2][2];
    #endif
    return copy;
}


NOALIAS __forceinline Mat43::Mat43( const Mat43& copy )
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    : m_RowX( copy.m_RowX )
    , m_RowY( copy.m_RowY )
    , m_RowZ( copy.m_RowZ )
    , m_RowW( copy.m_RowW )
    #elif defined( _VMX128_ )
    : m_ColX( copy.m_ColX )
    , m_ColY( copy.m_ColY )
    , m_ColZ( copy.m_ColZ )
    #endif
{
    #if !defined( _SIMD_ )
    m[0][0] = copy.m[0][0]; m[0][1] = copy.m[0][1]; m[0][2] = copy.m[0][2];
    m[1][0] = copy.m[1][0]; m[1][1] = copy.m[1][1]; m[1][2] = copy.m[1][2];
    m[2][0] = copy.m[2][0]; m[2][1] = copy.m[2][1]; m[2][2] = copy.m[2][2];
    m[3][0] = copy.m[3][0]; m[3][1] = copy.m[3][1]; m[3][2] = copy.m[3][2];
    #endif
}

NOALIAS __forceinline const Mat43& Mat43::operator = ( const Mat43& copy )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    m_RowX = copy.m_RowX;
    m_RowY = copy.m_RowY;
    m_RowZ = copy.m_RowZ;
    m_RowW = copy.m_RowW;
    #elif defined( _VMX128_ )
    m_ColX = copy.m_ColX;
    m_ColY = copy.m_ColY;
    m_ColZ = copy.m_ColZ;
    #else
    m[0][0] = copy.m[0][0]; m[0][1] = copy.m[0][1]; m[0][2] = copy.m[0][2];
    m[1][0] = copy.m[1][0]; m[1][1] = copy.m[1][1]; m[1][2] = copy.m[1][2];
    m[2][0] = copy.m[2][0]; m[2][1] = copy.m[2][1]; m[2][2] = copy.m[2][2];
    m[3][0] = copy.m[3][0]; m[3][1] = copy.m[3][1]; m[3][2] = copy.m[3][2];
    #endif
    return copy;
}


NOALIAS __forceinline Mat44::Mat44( const Mat44& copy )
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    : m_RowX( copy.m_RowX )
    , m_RowY( copy.m_RowY )
    , m_RowZ( copy.m_RowZ )
    , m_RowW( copy.m_RowW )
    #elif defined( _VMX128_ )
    : m_ColX( copy.m_ColX )
    , m_ColY( copy.m_ColY )
    , m_ColZ( copy.m_ColZ )
    , m_ColW( copy.m_ColW )
    #endif
{
    #if !defined( _SIMD_ )
    m[0][0] = copy.m[0][0]; m[0][1] = copy.m[0][1]; m[0][2] = copy.m[0][2]; m[0][3] = copy.m[0][3];
    m[1][0] = copy.m[1][0]; m[1][1] = copy.m[1][1]; m[1][2] = copy.m[1][2]; m[1][3] = copy.m[1][3];
    m[2][0] = copy.m[2][0]; m[2][1] = copy.m[2][1]; m[2][2] = copy.m[2][2]; m[2][3] = copy.m[2][3];
    m[3][0] = copy.m[3][0]; m[3][1] = copy.m[3][1]; m[3][2] = copy.m[3][2]; m[3][3] = copy.m[3][3];
    #endif
}

NOALIAS __forceinline const Mat44& Mat44::operator = ( const Mat44& copy )
{
    #if defined( _SPU_ ) || defined( _PPU_ ) || defined( _SSE_ )
    m_RowX = copy.m_RowX;
    m_RowY = copy.m_RowY;
    m_RowZ = copy.m_RowZ;
    m_RowW = copy.m_RowW;
    #elif defined( _VMX128_ )
    m_ColX = copy.m_ColX;
    m_ColY = copy.m_ColY;
    m_ColZ = copy.m_ColZ;
    m_ColW = copy.m_ColW;
    #else
    m[0][0] = copy.m[0][0]; m[0][1] = copy.m[0][1]; m[0][2] = copy.m[0][2]; m[0][3] = copy.m[0][3];
    m[1][0] = copy.m[1][0]; m[1][1] = copy.m[1][1]; m[1][2] = copy.m[1][2]; m[1][3] = copy.m[1][3];
    m[2][0] = copy.m[2][0]; m[2][1] = copy.m[2][1]; m[2][2] = copy.m[2][2]; m[2][3] = copy.m[2][3];
    m[3][0] = copy.m[3][0]; m[3][1] = copy.m[3][1]; m[3][2] = copy.m[3][2]; m[3][3] = copy.m[3][3];
    #endif
    return copy;
}


#if !defined( PASSINREG_ENABLED )

NOALIAS __forceinline ScaleMat3::ScaleMat3( const ScaleMat3& copy )
    : m_Scale( copy.m_Scale )
{
}

NOALIAS __forceinline const ScaleMat3& ScaleMat3::operator = ( const ScaleMat3& copy )
{
    m_Scale = copy.m_Scale;
    return copy;
}


NOALIAS __forceinline ShearMat3::ShearMat3( const ShearMat3& copy )
    : m_Shear( copy.m_Shear )
{
}

NOALIAS __forceinline const ShearMat3& ShearMat3::operator = ( const ShearMat3& copy )
{
    m_Shear = copy.m_Shear;
    return copy;
}

#endif


NOALIAS __forceinline OrthoMat33::OrthoMat33( const OrthoMat33& copy )
    : m( copy.m )
{
}

NOALIAS __forceinline const OrthoMat33& OrthoMat33::operator = ( const OrthoMat33& copy )
{
    m = copy.m;
    return copy;
}


NOALIAS __forceinline OrthoMat43::OrthoMat43( const OrthoMat43& copy )
    : m( copy.m )
{
}

NOALIAS __forceinline const OrthoMat43& OrthoMat43::operator = ( const OrthoMat43& copy )
{
    m = copy.m;
    return copy;
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )

