#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_CLASS_GEOMETRY_INL
#define SOFT_MATHS_HVEC_NEW_CLASS_GEOMETRY_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_CLASS_GEOMETRY_INL
#define MATHS_HVEC_NEW_CLASS_GEOMETRY_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Geometry_Classes
/// @{

/// QuadraticResult Copy Constructor
NOALIAS __forceinline QuadraticResult::QuadraticResult( const QuadraticResult& copy )
    : m_X1( copy.m_X1 )
    , m_X2( copy.m_X2 )
{
}

/// QuadraticResult Copy Assignment
NOALIAS __forceinline const QuadraticResult& QuadraticResult::operator = ( const QuadraticResult& copy )
{
    m_X1 = copy.m_X1;
    m_X2 = copy.m_X2;
    return copy;
}


/// Line Copy Constructor
NOALIAS __forceinline Line::Line( const Line& copy )
    : m_StartLength( copy.m_StartLength )
    , m_EndRcpLength( copy.m_EndRcpLength )
{
}

/// Line Copy Assignment
NOALIAS __forceinline const Line& Line::operator = ( const Line& copy )
{
    m_StartLength  = copy.m_StartLength;
    m_EndRcpLength = copy.m_EndRcpLength;
    return copy;
}


/// Triangle Copy Constructor
NOALIAS __forceinline Triangle::Triangle( const Triangle& copy )
    : m_PointA      ( copy.m_PointA )
    , m_EdgeVectorAB( copy.m_EdgeVectorAB )
    , m_EdgeVectorAC( copy.m_EdgeVectorAC )
{
}

/// Triangle Copy Assignment
NOALIAS __forceinline const Triangle& Triangle::operator = ( const Triangle& copy )
{
    m_PointA       = copy.m_PointA;
    m_EdgeVectorAB = copy.m_EdgeVectorAB;
    m_EdgeVectorAC = copy.m_EdgeVectorAC;
    return copy;
}


/// AxisBox Copy Constructor
NOALIAS __forceinline AxisBox::AxisBox( const AxisBox& copy )
    : m_Min( copy.m_Min )
    , m_Max( copy.m_Max )
{
}

/// AxisBox Copy Assignment
NOALIAS __forceinline const AxisBox& AxisBox::operator = ( const AxisBox& copy )
{
    m_Min = copy.m_Min;
    m_Max = copy.m_Max;
    return copy;
}


/// OrientedBox Copy Constructor
NOALIAS __forceinline OrientedBox::OrientedBox( const OrientedBox& copy )
    : m_HalfExtent( copy.m_HalfExtent )
    , m_Matrix( copy.m_Matrix )
{
}

/// OrientedBox Copy Assignment
NOALIAS __forceinline const OrientedBox& OrientedBox::operator = ( const OrientedBox& copy )
{
    m_HalfExtent = copy.m_HalfExtent;
    m_Matrix = copy.m_Matrix;
    return copy;
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


