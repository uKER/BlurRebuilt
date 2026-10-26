#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT76_INL
#define SOFT_MATHS_HVEC_NEW_MAT76_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT76_INL
#define MATHS_HVEC_NEW_MAT76_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat76
/// @{

/// Make Mat76 from 7 Vector6 rows
NOALIAS __forceinline const Mat76 MakeMat76( const Vector6& row0, const Vector6& row1, const Vector6& row2, const Vector6& row3,
                                             const Vector6& row4, const Vector6& row5, const Vector6& row6 )
{
    Mat76 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    result.m_Row[4] = row4;
    result.m_Row[5] = row5;
    result.m_Row[6] = row6;
    return result;
}


NOALIAS __forceinline const Mat76 Const< Mat76 >::Zero() ///< Constant Mat76 with all elements zeroed
{
    Mat76 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] =
    result.m_Row[4] =
    result.m_Row[5] =
    result.m_Row[6] = Const< Vector6 >::Zero();
    return result;
}


/// Mat76 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat76& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat76::m_Rows ) && ( C < Mat76::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat76 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat76* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat76::m_Rows ) && ( C < Mat76::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat76 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat76& src )
{
    BIZ_STATIC_ASSERT( R < Mat76::m_Rows );
    return src.m_Row[R];
}

/// Mat76 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat76* const __restrict pDst, const Vector6& row )
{
    BIZ_STATIC_ASSERT( R < Mat76::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat76 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector7 Col( const Mat76& src )
{
    BIZ_STATIC_ASSERT( C < Mat76::m_Cols );
    return MakeVector7( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ),
                        Element<3,C>( src ),
                        Element<4,C>( src ),
                        Element<5,C>( src ),
                        Element<6,C>( src ) );
}


/// Mat76 Negation Operator
NOALIAS __forceinline const Mat76 operator - ( const Mat76& a )
{
    Mat76 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    result.m_Row[4] = -a.m_Row[4];
    result.m_Row[5] = -a.m_Row[5];
    result.m_Row[6] = -a.m_Row[6];
    return result;
}

/// Mat76 Addition Operator
NOALIAS __forceinline const Mat76 operator + ( const Mat76& a, const Mat76& b )
{
    Mat76 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] + b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] + b.m_Row[5];
    result.m_Row[6] = a.m_Row[6] + b.m_Row[6];
    return result;
}

/// Mat76 Subtraction Operator
NOALIAS __forceinline const Mat76 operator - ( const Mat76& a, const Mat76& b )
{
    Mat76 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] - b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] - b.m_Row[5];
    result.m_Row[6] = a.m_Row[6] - b.m_Row[6];
    return result;
}

/// Mat76 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat76& a, const Mat76& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
    a.m_Row[4] += b.m_Row[4];
    a.m_Row[5] += b.m_Row[5];
    a.m_Row[6] += b.m_Row[6];
}

/// Mat76 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat76& a, const Mat76& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
    a.m_Row[4] -= b.m_Row[4];
    a.m_Row[5] -= b.m_Row[5];
    a.m_Row[6] -= b.m_Row[6];
}


/// Mat76 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector7 operator * ( const Mat76& m, const Vector6& v )
{
    return MakeVector7( Dot( Row<0>( m ), v ),
                        Dot( Row<1>( m ), v ),
                        Dot( Row<2>( m ), v ),
                        Dot( Row<3>( m ), v ),
                        Dot( Row<4>( m ), v ),
                        Dot( Row<5>( m ), v ),
                        Dot( Row<6>( m ), v ) );
}


/// Mat76 * Mat67 Diagonal Multiply: Returns the diagonal of the resulting 7x7 matrix
NOALIAS __forceinline const Vector7 DiagonalMultiply( const Mat76& a, const Mat67& b )
{
    return MakeVector7( Dot( Row<0>(a), Col<0>(b) ),
                        Dot( Row<1>(a), Col<1>(b) ),
                        Dot( Row<2>(a), Col<2>(b) ),
                        Dot( Row<3>(a), Col<3>(b) ),
                        Dot( Row<4>(a), Col<4>(b) ),
                        Dot( Row<5>(a), Col<5>(b) ),
                        Dot( Row<6>(a), Col<6>(b) ) );
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


