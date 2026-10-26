#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT67_INL
#define SOFT_MATHS_HVEC_NEW_MAT67_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT67_INL
#define MATHS_HVEC_NEW_MAT67_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat67
/// @{

/// Make Mat67 from 6 Vector7 rows
NOALIAS __forceinline const Mat67 MakeMat67( const Vector7& row0, const Vector7& row1, const Vector7& row2, const Vector7& row3,
                                             const Vector7& row4, const Vector7& row5 )
{
    Mat67 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    result.m_Row[4] = row4;
    result.m_Row[5] = row5;
    return result;
}


NOALIAS __forceinline const Mat67 Const< Mat67 >::Zero() ///< Constant Mat67 with all elements zeroed
{
    Mat67 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] =
    result.m_Row[4] =
    result.m_Row[5] = Const< Vector7 >::Zero();
    return result;
}


/// Mat67 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat67& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat67::m_Rows ) && ( C < Mat67::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat67 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat67* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat67::m_Rows ) && ( C < Mat67::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat67 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector7 Row( const Mat67& src )
{
    BIZ_STATIC_ASSERT( R < Mat67::m_Rows );
    return src.m_Row[R];
}

/// Mat67 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat67* const __restrict pDst, const Vector7& row )
{
    BIZ_STATIC_ASSERT( R < Mat67::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat67 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat67& src )
{
    BIZ_STATIC_ASSERT( C < Mat67::m_Cols );
    return MakeVector6( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ),
                        Element<3,C>( src ),
                        Element<4,C>( src ),
                        Element<5,C>( src ) );
}


/// Mat67 Negation Operator
NOALIAS __forceinline const Mat67 operator - ( const Mat67& a )
{
    Mat67 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    result.m_Row[4] = -a.m_Row[4];
    result.m_Row[5] = -a.m_Row[5];
    return result;
}

/// Mat67 Addition Operator
NOALIAS __forceinline const Mat67 operator + ( const Mat67& a, const Mat67& b )
{
    Mat67 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] + b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] + b.m_Row[5];
    return result;
}

/// Mat67 Subtraction Operator
NOALIAS __forceinline const Mat67 operator - ( const Mat67& a, const Mat67& b )
{
    Mat67 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] - b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] - b.m_Row[5];
    return result;
}

/// Mat67 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat67& a, const Mat67& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
    a.m_Row[4] += b.m_Row[4];
    a.m_Row[5] += b.m_Row[5];
}

/// Mat67 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat67& a, const Mat67& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
    a.m_Row[4] -= b.m_Row[4];
    a.m_Row[5] -= b.m_Row[5];
}


/// Mat67 * Vector7 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat67& m, const Vector7& v )
{
    return MakeVector6( Dot( Row<0>( m ), v ),
                        Dot( Row<1>( m ), v ),
                        Dot( Row<2>( m ), v ),
                        Dot( Row<3>( m ), v ),
                        Dot( Row<4>( m ), v ),
                        Dot( Row<5>( m ), v ) );
}


/// Mat67 * Mat76 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat67& a, const Mat76& b )
{
    return MakeVector6( Dot( Row<0>(a), Col<0>(b) ),
                        Dot( Row<1>(a), Col<1>(b) ),
                        Dot( Row<2>(a), Col<2>(b) ),
                        Dot( Row<3>(a), Col<3>(b) ),
                        Dot( Row<4>(a), Col<4>(b) ),
                        Dot( Row<5>(a), Col<5>(b) ) );
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


