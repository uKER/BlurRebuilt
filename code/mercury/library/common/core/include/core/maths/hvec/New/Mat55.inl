#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT55_INL
#define SOFT_MATHS_HVEC_NEW_MAT55_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT55_INL
#define MATHS_HVEC_NEW_MAT55_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat55
/// @{

/// Make Mat55 from 5 Vector5 rows
NOALIAS __forceinline const Mat55 MakeMat55( const Vector5& row0, const Vector5& row1, const Vector5& row2, const Vector5& row3,
                                             const Vector5& row4 )
{
    Mat55 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    result.m_Row[4] = row4;
    return result;
}


NOALIAS __forceinline const Mat55 Const< Mat55 >::Zero() ///< Constant Mat55 with all elements zeroed
{
    Mat55 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] =
    result.m_Row[4] = Const< Vector5 >::Zero();
    return result;
}


/// Mat55 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat55& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat55::m_Rows ) && ( C < Mat55::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat55 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat55* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat55::m_Rows ) && ( C < Mat55::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat55 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector5 Row( const Mat55& src )
{
    BIZ_STATIC_ASSERT( R < Mat55::m_Rows );
    return src.m_Row[R];
}

/// Mat55 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat55* const __restrict pDst, const Vector5& row )
{
    BIZ_STATIC_ASSERT( R < Mat55::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat55 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector5 Col( const Mat55& src )
{
    BIZ_STATIC_ASSERT( C < Mat55::m_Cols );
    return MakeVector5( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ),
                        Element<3,C>( src ),
                        Element<4,C>( src ) );
}


/// Mat55 Negation Operator
NOALIAS __forceinline const Mat55 operator - ( const Mat55& a )
{
    Mat55 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    result.m_Row[4] = -a.m_Row[4];
    return result;
}

/// Mat55 Addition Operator
NOALIAS __forceinline const Mat55 operator + ( const Mat55& a, const Mat55& b )
{
    Mat55 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] + b.m_Row[4];
    return result;
}

/// Mat55 Subtraction Operator
NOALIAS __forceinline const Mat55 operator - ( const Mat55& a, const Mat55& b )
{
    Mat55 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] - b.m_Row[4];
    return result;
}

/// Mat55 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat55& a, const Mat55& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
    a.m_Row[4] += b.m_Row[4];
}

/// Mat55 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat55& a, const Mat55& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
    a.m_Row[4] -= b.m_Row[4];
}


/// Mat55 * Vector5 Multiplication Operator
NOALIAS __forceinline const Vector5 operator * ( const Mat55& m55, const Vector5& v5 )
{
    return MakeVector5( Dot( Row<0>( m55 ), v5 ),
                        Dot( Row<1>( m55 ), v5 ),
                        Dot( Row<2>( m55 ), v5 ),
                        Dot( Row<3>( m55 ), v5 ),
                        Dot( Row<4>( m55 ), v5 ) );
}

/// Vector5 * Mat55 Multiplication Operator
NOALIAS __forceinline const Vector5 operator * ( const Vector5& v5, const Mat55& m55 )
{
    return MakeVector5( Dot( v5, Col<0>( m55 ) ),
                        Dot( v5, Col<1>( m55 ) ),
                        Dot( v5, Col<2>( m55 ) ),
                        Dot( v5, Col<3>( m55 ) ),
                        Dot( v5, Col<4>( m55 ) ) );
}


/// Mat55 Scalar Multiplication
NOALIAS __forceinline const Mat55 operator * ( const Mat55& m, const VF32 VECREF f )
{
    Mat55 result;
	result.m_Row[0] = Row<0>( m ) * f;
	result.m_Row[1] = Row<1>( m ) * f;
	result.m_Row[2] = Row<2>( m ) * f;
	result.m_Row[3] = Row<3>( m ) * f;
	result.m_Row[4] = Row<4>( m ) * f;
	return result;
}


/// Mat55 * Mat55 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector5 DiagonalMultiply( const Mat55& a, const Mat55& b )
{
    return MakeVector5( Dot( Row<0>(a), Col<0>(b) ),
                        Dot( Row<1>(a), Col<1>(b) ),
                        Dot( Row<2>(a), Col<2>(b) ),
                        Dot( Row<3>(a), Col<3>(b) ),
                        Dot( Row<4>(a), Col<4>(b) ) );
}

/// Mat55 Matrix Multiply
NOALIAS __forceinline const Mat55 operator * ( const Mat55& a, const Mat55& b )
{
	const Vector5 rowA0( Row<0>( a ) );
	const Vector5 rowA1( Row<1>( a ) );
	const Vector5 rowA2( Row<2>( a ) );
	const Vector5 rowA3( Row<3>( a ) );
	const Vector5 rowA4( Row<4>( a ) );
                                   
	const Vector5 colB0( Col<0>( b ) );
	const Vector5 colB1( Col<1>( b ) );
	const Vector5 colB2( Col<2>( b ) );
	const Vector5 colB3( Col<3>( b ) );
	const Vector5 colB4( Col<4>( b ) );
    
	Mat55 result;
	result.m_Row[0] = MakeVector5( Dot( rowA0, colB0 ), Dot( rowA0, colB1 ), Dot( rowA0, colB2 ), Dot( rowA0, colB3 ), Dot( rowA0, colB4 ) );
	result.m_Row[1] = MakeVector5( Dot( rowA1, colB0 ), Dot( rowA1, colB1 ), Dot( rowA1, colB2 ), Dot( rowA1, colB3 ), Dot( rowA1, colB4 ) );
	result.m_Row[2] = MakeVector5( Dot( rowA2, colB0 ), Dot( rowA2, colB1 ), Dot( rowA2, colB2 ), Dot( rowA2, colB3 ), Dot( rowA2, colB4 ) );
	result.m_Row[3] = MakeVector5( Dot( rowA3, colB0 ), Dot( rowA3, colB1 ), Dot( rowA3, colB2 ), Dot( rowA3, colB3 ), Dot( rowA3, colB4 ) );
	result.m_Row[4] = MakeVector5( Dot( rowA4, colB0 ), Dot( rowA4, colB1 ), Dot( rowA4, colB2 ), Dot( rowA4, colB3 ), Dot( rowA4, colB4 ) );
	return result;

}


/// Make Mat55 from Mat44 top-left matrix, Vector4 top-right column vector, Vector4 bottom-left row vector & bottom-right scalar
NOALIAS __forceinline const Mat55 MakeMat55( const Mat44 MATREF        tl, const Vector4 PCREF tr,
                                             const Vector4 PCREF bl, const VF32 VECREF    br )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    Mat55 result;
    result.m_Row[0] = MakeVector5( RowX( tl ), X( tr ) );
    result.m_Row[1] = MakeVector5( RowY( tl ), Y( tr ) );
    result.m_Row[2] = MakeVector5( RowZ( tl ), Z( tr ) );
    result.m_Row[3] = MakeVector5( RowW( tl ), W( tr ) );
    result.m_Row[4] = MakeVector5( bl, br );
    return result;
}

/// Make Mat55 from top-left column Vector4, top-right Mat44, bottom-left scalar & bottom-right row Vector4
NOALIAS __forceinline const Mat55 MakeMat55( const Vector4 PCREF tl, const Mat44 MATREF        tr,
                                             const VF32 VECREF    bl, const Vector4 PCREF br )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    Mat55 result;
    result.m_Row[0] = MakeVector5( X( tl ), RowX( tr ) );
    result.m_Row[1] = MakeVector5( Y( tl ), RowY( tr ) );
    result.m_Row[2] = MakeVector5( Z( tl ), RowZ( tr ) );
    result.m_Row[3] = MakeVector5( W( tl ), RowW( tr ) );
    result.m_Row[4] = MakeVector5( bl, br );
    return result;
}


/// Mat55 Split into Mat44 top-left matrix, Vector4 top-right column vector, Vector4 bottom-left row vector & bottom-right scalar
NOALIAS __forceinline void Split( const Mat55& m,
                                  Mat44*   const __restrict pTL,
                                  Vector4* const __restrict pTR,
                                  Vector4* const __restrict pBL,
                                  VF32*    const __restrict pBR )
{
    // TL = top left,    TR = top right
    // BL = bottom left, BR = bottom right
    
    const Vector5 row0( Row<0>( m ) );
    const Vector5 row1( Row<1>( m ) );
    const Vector5 row2( Row<2>( m ) );
    const Vector5 row3( Row<3>( m ) );
    const Vector5 row4( Row<4>( m ) );
    
    *pTL = MakeMat44( MakeVector4( row0 ),
                      MakeVector4( row1 ),
                      MakeVector4( row2 ),
                      MakeVector4( row3 ) );
    
    *pTR = MakeVector4( Element<4>( row0 ),
                        Element<4>( row1 ),
                        Element<4>( row2 ),
                        Element<4>( row3 ) );
    
    *pBL = MakeVector4( row4 );
    
    *pBR = Element<4>( row4 );
}

/// Mat55 Split into top-left row Vector4, top-right scalar, bottom-left Mat44 & bottom-right column Vector4
NOALIAS __forceinline void Split( const Mat55& m,
                                  Vector4* const __restrict pTL,
                                  VF32*    const __restrict pTR,
                                  Mat44*   const __restrict pBL,
                                  Vector4* const __restrict pBR )
{
    // TL = top left,    TR = top right
    // BL = bottom left, BR = bottom right
    
    const Vector5 row0( Row<0>( m ) );
    const Vector5 row1( Row<1>( m ) );
    const Vector5 row2( Row<2>( m ) );
    const Vector5 row3( Row<3>( m ) );
    const Vector5 row4( Row<4>( m ) );
    
    *pTL = MakeVector4( row0 );
    
    *pTR = Element<4>( row0 );
    
    *pBL = MakeMat44( MakeVector4( row1 ),
                      MakeVector4( row2 ),
                      MakeVector4( row3 ),
                      MakeVector4( row4 ) );
    
    *pBR = MakeVector4( Element<4>( row1 ),
                        Element<4>( row2 ),
                        Element<4>( row3 ),
                        Element<4>( row4 ) );
}



/// Create a Mat44 sub-matrix from a Mat55 by deleting Row and Col
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const Mat44 DeleteRowCol( const Mat55& src )
{
    const Vector4 row0( MakeVector4( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, 4 >( Row< R<=0?1:0 >( src ) ) ) );
    const Vector4 row1( MakeVector4( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, 4 >( Row< R<=1?2:1 >( src ) ) ) );
    const Vector4 row2( MakeVector4( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, 4 >( Row< R<=2?3:2 >( src ) ) ) );
    const Vector4 row3( MakeVector4( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, 4 >( Row< R<=3?4:3 >( src ) ) ) );
    return MakeMat44( row0, row1, row2, row3 );
}

#if 0

/// Create a Mat44 sub-matrix from a Mat55 by deleting Row 0 and Col 0
template <> NOALIAS __forceinline const Mat44 DeleteRowCol< 0, 0 >( const Mat55& src )
{
    const Vector4 row0( MakeVector4( Permute<1,2,3,4,0>( Row<1>( src ) ) ) );
    const Vector4 row1( MakeVector4( Permute<1,2,3,4,0>( Row<2>( src ) ) ) );
    const Vector4 row2( MakeVector4( Permute<1,2,3,4,0>( Row<3>( src ) ) ) );
    const Vector4 row3( MakeVector4( Permute<1,2,3,4,0>( Row<4>( src ) ) ) );
    return MakeMat44( row0, row1, row2, row3 );
}

/// Create a Mat44 sub-matrix from a Mat55 by deleting Row 0 and Col 1
template <> NOALIAS __forceinline const Mat44 DeleteRowCol< 0, 1 >( const Mat55& src )
{
    const Vector4 row0( MakeVector4( Permute<0,2,3,4,0>( Row<1>( src ) ) ) );
    const Vector4 row1( MakeVector4( Permute<0,2,3,4,0>( Row<2>( src ) ) ) );
    const Vector4 row2( MakeVector4( Permute<0,2,3,4,0>( Row<3>( src ) ) ) );
    const Vector4 row3( MakeVector4( Permute<0,2,3,4,0>( Row<4>( src ) ) ) );
    return MakeMat44( row0, row1, row2, row3 );
}

/// Create a Mat44 sub-matrix from a Mat55 by deleting Row 0 and Col 2
template <> NOALIAS __forceinline const Mat44 DeleteRowCol< 0, 2 >( const Mat55& src )
{
    const Vector4 row0( MakeVector4( Permute<0,1,3,4,0>( Row<1>( src ) ) ) );
    const Vector4 row1( MakeVector4( Permute<0,1,3,4,0>( Row<2>( src ) ) ) );
    const Vector4 row2( MakeVector4( Permute<0,1,3,4,0>( Row<3>( src ) ) ) );
    const Vector4 row3( MakeVector4( Permute<0,1,3,4,0>( Row<4>( src ) ) ) );
    return MakeMat44( row0, row1, row2, row3 );
}

/// Create a Mat44 sub-matrix from a Mat55 by deleting Row 0 and Col 3
template <> NOALIAS __forceinline const Mat44 DeleteRowCol< 0, 3 >( const Mat55& src )
{
    const Vector4 row0( MakeVector4( Permute<0,1,2,4,0>( Row<1>( src ) ) ) );
    const Vector4 row1( MakeVector4( Permute<0,1,2,4,0>( Row<2>( src ) ) ) );
    const Vector4 row2( MakeVector4( Permute<0,1,2,4,0>( Row<3>( src ) ) ) );
    const Vector4 row3( MakeVector4( Permute<0,1,2,4,0>( Row<4>( src ) ) ) );
    return MakeMat44( row0, row1, row2, row3 );
}

/// Create a Mat44 sub-matrix from a Mat55 by deleting Row 0 and Col 4
template <> NOALIAS __forceinline const Mat44 DeleteRowCol< 0, 4 >( const Mat55& src )
{
    const Vector4 row0( MakeVector4( Permute<0,1,2,3,0>( Row<1>( src ) ) ) );
    const Vector4 row1( MakeVector4( Permute<0,1,2,3,0>( Row<2>( src ) ) ) );
    const Vector4 row2( MakeVector4( Permute<0,1,2,3,0>( Row<3>( src ) ) ) );
    const Vector4 row3( MakeVector4( Permute<0,1,2,3,0>( Row<4>( src ) ) ) );
    return MakeMat44( row0, row1, row2, row3 );
}

#endif



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


