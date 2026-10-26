#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT65_INL
#define SOFT_MATHS_HVEC_NEW_MAT65_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT65_INL
#define MATHS_HVEC_NEW_MAT65_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat65
/// @{

/// Make Mat65 from 6 Vector5 rows
NOALIAS __forceinline const Mat65 MakeMat65( const Vector5& row0, const Vector5& row1, const Vector5& row2, const Vector5& row3,
                                             const Vector5& row4, const Vector5& row5 )
{
    Mat65 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    result.m_Row[4] = row4;
    result.m_Row[5] = row5;
    return result;
}


NOALIAS __forceinline const Mat65 Const< Mat65 >::Zero() ///< Constant Mat65 with all elements zeroed
{
    Mat65 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] =
    result.m_Row[4] =
    result.m_Row[5] = Const< Vector5 >::Zero();
    return result;
}


/// Mat65 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat65& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat65::m_Rows ) && ( C < Mat65::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat65 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat65* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat65::m_Rows ) && ( C < Mat65::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat65 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector5 Row( const Mat65& src )
{
    BIZ_STATIC_ASSERT( R < Mat65::m_Rows );
    return src.m_Row[R];
}

/// Mat65 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat65* const __restrict pDst, const Vector5& row )
{
    BIZ_STATIC_ASSERT( R < Mat65::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat65 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat65& src )
{
    BIZ_STATIC_ASSERT( C < Mat65::m_Cols );
    return MakeVector6( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ),
                        Element<3,C>( src ),
                        Element<4,C>( src ),
                        Element<5,C>( src ) );
}


/// Mat65 Negation Operator
NOALIAS __forceinline const Mat65 operator - ( const Mat65& a )
{
    Mat65 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    result.m_Row[4] = -a.m_Row[4];
    result.m_Row[5] = -a.m_Row[5];
    return result;
}

/// Mat65 Addition Operator
NOALIAS __forceinline const Mat65 operator + ( const Mat65& a, const Mat65& b )
{
    Mat65 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] + b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] + b.m_Row[5];
    return result;
}

/// Mat65 Subtraction Operator
NOALIAS __forceinline const Mat65 operator - ( const Mat65& a, const Mat65& b )
{
    Mat65 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] - b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] - b.m_Row[5];
    return result;
}

/// Mat65 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat65& a, const Mat65& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
    a.m_Row[4] += b.m_Row[4];
    a.m_Row[5] += b.m_Row[5];
}

/// Mat65 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat65& a, const Mat65& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
    a.m_Row[4] -= b.m_Row[4];
    a.m_Row[5] -= b.m_Row[5];
}


/// Mat65 * Vector5 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat65& m, const Vector5& v )
{
    return MakeVector6( Dot( Row<0>( m ), v ),
                        Dot( Row<1>( m ), v ),
                        Dot( Row<2>( m ), v ),
                        Dot( Row<3>( m ), v ),
                        Dot( Row<4>( m ), v ),
                        Dot( Row<5>( m ), v ) );
}


/// Mat65 * Mat56 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat65& a, const Mat56& b )
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


