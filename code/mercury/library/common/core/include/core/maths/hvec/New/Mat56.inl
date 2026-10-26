#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT56_INL
#define SOFT_MATHS_HVEC_NEW_MAT56_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT56_INL
#define MATHS_HVEC_NEW_MAT56_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat56
/// @{

/// Make Mat56 from 5 Vector6 rows
NOALIAS __forceinline const Mat56 MakeMat56( const Vector6& row0, const Vector6& row1, const Vector6& row2, const Vector6& row3,
                                             const Vector6& row4 )
{
    Mat56 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    result.m_Row[4] = row4;
    return result;
}


NOALIAS __forceinline const Mat56 Const< Mat56 >::Zero() ///< Constant Mat56 with all elements zeroed
{
    Mat56 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] =
    result.m_Row[4] = Const< Vector6 >::Zero();
    return result;
}


/// Mat56 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat56& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat56::m_Rows ) && ( C < Mat56::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat56 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat56* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat56::m_Rows ) && ( C < Mat56::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat56 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat56& src )
{
    BIZ_STATIC_ASSERT( R < Mat56::m_Rows );
    return src.m_Row[R];
}

/// Mat56 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat56* const __restrict pDst, const Vector6& row )
{
    BIZ_STATIC_ASSERT( R < Mat56::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat56 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector5 Col( const Mat56& src )
{
    BIZ_STATIC_ASSERT( C < Mat56::m_Cols );
    return MakeVector5( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ),
                        Element<3,C>( src ),
                        Element<4,C>( src ) );
}


/// Mat56 Negation Operator
NOALIAS __forceinline const Mat56 operator - ( const Mat56& a )
{
    Mat56 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    result.m_Row[4] = -a.m_Row[4];
    return result;
}

/// Mat56 Addition Operator
NOALIAS __forceinline const Mat56 operator + ( const Mat56& a, const Mat56& b )
{
    Mat56 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] + b.m_Row[4];
    return result;
}

/// Mat56 Subtraction Operator
NOALIAS __forceinline const Mat56 operator - ( const Mat56& a, const Mat56& b )
{
    Mat56 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] - b.m_Row[4];
    return result;
}

/// Mat56 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat56& a, const Mat56& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
    a.m_Row[4] += b.m_Row[4];
}

/// Mat56 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat56& a, const Mat56& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
    a.m_Row[4] -= b.m_Row[4];
}


/// Mat56 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector5 operator * ( const Mat56& m, const Vector6& v )
{
    return MakeVector5( Dot( Row<0>( m ), v ),
                        Dot( Row<1>( m ), v ),
                        Dot( Row<2>( m ), v ),
                        Dot( Row<3>( m ), v ),
                        Dot( Row<4>( m ), v ) );
}


/// Mat56 * Mat65 Diagonal Multiply: Returns the diagonal of the resulting 5x5 matrix
NOALIAS __forceinline const Vector5 DiagonalMultiply( const Mat56& a, const Mat65& b )
{
    return MakeVector5( Dot( Row<0>(a), Col<0>(b) ),
                        Dot( Row<1>(a), Col<1>(b) ),
                        Dot( Row<2>(a), Col<2>(b) ),
                        Dot( Row<3>(a), Col<3>(b) ),
                        Dot( Row<4>(a), Col<4>(b) ) );
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


