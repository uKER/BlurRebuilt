#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT26_INL
#define SOFT_MATHS_HVEC_NEW_MAT26_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT26_INL
#define MATHS_HVEC_NEW_MAT26_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat26
/// @{

/// Make Mat26 from 3 Vector6 rows
NOALIAS __forceinline const Mat26 MakeMat26( const Vector6& row0, const Vector6& row1 )
{
    Mat26 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    return result;
}


NOALIAS __forceinline const Mat26 Const< Mat26 >::Zero() ///< Constant Mat26 with all elements zeroed
{
    Mat26 result;
    result.m_Row[0] =
    result.m_Row[1] = Const< Vector6 >::Zero();
    return result;
}


/// Mat26 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat26& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat26::m_Rows ) && ( C < Mat26::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat26 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat26* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat26::m_Rows ) && ( C < Mat26::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat26 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat26& src )
{
    BIZ_STATIC_ASSERT( R < Mat26::m_Rows );
    return src.m_Row[R];
}

/// Mat26 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat26* const __restrict pDst, const Vector6& row )
{
    BIZ_STATIC_ASSERT( R < Mat26::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat26 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector2 Col( const Mat26& src )
{
    BIZ_STATIC_ASSERT( C < Mat26::m_Cols );
    return MakeVector2( Element<0,C>( src ),
                        Element<1,C>( src ) );
}

/// Mat26 Accessor: Set column
template < unsigned int C >
NOALIAS __forceinline void SetCol( Mat26* const __restrict pDst, const Vector2 PCREF col )
{
    BIZ_STATIC_ASSERT( C < Mat26::m_Cols );
    SetElement<C>( &pDst->m_Row[0], Element<0>( col ) );
    SetElement<C>( &pDst->m_Row[1], Element<1>( col ) );
}


/// Mat26 Transpose
NOALIAS __forceinline const Mat62 Transpose( const Mat26& src )
{
    Mat62 result;
    
    #if defined( _SPU_ )
    
    // transpose top-left 2x4
    const vector float tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const vector float tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const vector float tl_x0_x1_y0_y1( spu_merge_xy( tl_x0_y0_z0_w0, tl_x1_y1_z1_w1 ) ); // [ x0 x1 y0 y1 ]
    const vector float tl_z0_z1_w0_w1( spu_merge_zw( tl_x0_y0_z0_w0, tl_x1_y1_z1_w1 ) ); // [ z0 z1 w0 w1 ]
    const vector float tl_y0_y1_x0_x1( spu_rotate_left_qw_words<2>( tl_x0_x1_y0_y1 ) );  // [ y0 y1 x0 x1 ]
    const vector float tl_w0_w1_z0_z1( spu_rotate_left_qw_words<2>( tl_z0_z1_w0_w1 ) );  // [ w0 w1 z0 z1 ]
    result.m_Row[0].m_V.m_V = tl_x0_x1_y0_y1;                                            // [ x0 x1 ]
    result.m_Row[1].m_V.m_V = tl_y0_y1_x0_x1;                                            // [ y0 y1 ]
    result.m_Row[2].m_V.m_V = tl_z0_z1_w0_w1;                                            // [ z0 z1 ]
    result.m_Row[3].m_V.m_V = tl_w0_w1_z0_z1;                                            // [ w0 w1 ]
    
    // transpose top-right 2x2 to bottom-left 2x2
    const vector float tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                           // [ x0 y0 ]
    const vector float tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                           // [ x1 y1 ]
    const vector float tr_x0_x1_y0_y1( spu_merge_xy( tr_x0_y0, tr_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    const vector float tr_y0_y1_x0_x1( spu_rotate_left_qw_words<2>( tr_x0_x1_y0_y1 ) ); // [ x1 00 y1 00 ]
    result.m_Row[4].m_V.m_V = tr_x0_x1_y0_y1;                                           // [ x0 x1 ]
    result.m_Row[5].m_V.m_V = tr_y0_y1_x0_x1;                                           // [ y0 y1 ]
    
    #elif defined( _PPU_ )                                                                                                
    
    // transpose top-left 2x4
    const vector float tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const vector float tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const vector float tl_x0_x1_y0_y1( vec_merge_xy( tl_x0_y0_z0_w0, tl_x1_y1_z1_w1 ) ); // [ x0 x1 y0 y1 ]
    const vector float tl_z0_z1_w0_w1( vec_merge_zw( tl_x0_y0_z0_w0, tl_x1_y1_z1_w1 ) ); // [ z0 z1 w0 w1 ]
    const vector float tl_y0_y1_x0_x1( vec_rotate_left_qw_words<2>( tl_x0_x1_y0_y1 ) );  // [ y0 y1 x0 x1 ]
    const vector float tl_w0_w1_z0_z1( vec_rotate_left_qw_words<2>( tl_z0_z1_w0_w1 ) );  // [ w0 w1 z0 z1 ]
    result.m_Row[0].m_V.m_V = tl_x0_x1_y0_y1;                                            // [ x0 x1 ]
    result.m_Row[1].m_V.m_V = tl_y0_y1_x0_x1;                                            // [ y0 y1 ]
    result.m_Row[2].m_V.m_V = tl_z0_z1_w0_w1;                                            // [ z0 z1 ]
    result.m_Row[3].m_V.m_V = tl_w0_w1_z0_z1;                                            // [ w0 w1 ]
    
    // transpose top-right 2x2 to bottom-left 2x2
    const vector float tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                           // [ x0 y0 ]
    const vector float tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                           // [ x1 y1 ]
    const vector float tr_x0_x1_y0_y1( vec_merge_xy( tr_x0_y0, tr_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    const vector float tr_y0_y1_x0_x1( vec_rotate_left_qw_words<2>( tr_x0_x1_y0_y1 ) ); // [ x1 00 y1 00 ]
    result.m_Row[4].m_V.m_V = tr_x0_x1_y0_y1;                                           // [ x0 x1 ]
    result.m_Row[5].m_V.m_V = tr_y0_y1_x0_x1;                                           // [ y0 y1 ]
    
    #elif defined( _VMX128_ )                                                                                             
    
    // transpose top-left 2x4
    const __vector4 tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const __vector4 tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const __vector4 tl_x0_x1_y0_y1( vmx_merge_xy( tl_x0_y0_z0_w0, tl_x1_y1_z1_w1 ) ); // [ x0 x1 y0 y1 ]
    const __vector4 tl_z0_z1_w0_w1( vmx_merge_zw( tl_x0_y0_z0_w0, tl_x1_y1_z1_w1 ) ); // [ z0 z1 w0 w1 ]
    const __vector4 tl_y0_y1_x0_x1( vmx_rotate_left_qw_words<2>( tl_x0_x1_y0_y1 ) );  // [ y0 y1 x0 x1 ]
    const __vector4 tl_w0_w1_z0_z1( vmx_rotate_left_qw_words<2>( tl_z0_z1_w0_w1 ) );  // [ w0 w1 z0 z1 ]
    result.m_Row[0].m_V.m_V = tl_x0_x1_y0_y1;                                         // [ x0 x1 ]
    result.m_Row[1].m_V.m_V = tl_y0_y1_x0_x1;                                         // [ y0 y1 ]
    result.m_Row[2].m_V.m_V = tl_z0_z1_w0_w1;                                         // [ z0 z1 ]
    result.m_Row[3].m_V.m_V = tl_w0_w1_z0_z1;                                         // [ w0 w1 ]
    
    // transpose top-right 2x2 to bottom-left 2x2
    const __vector4 tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                           // [ x0 y0 ]
    const __vector4 tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                           // [ x1 y1 ]
    const __vector4 tr_x0_x1_y0_y1( vmx_merge_xy( tr_x0_y0, tr_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    const __vector4 tr_y0_y1_x0_x1( vmx_rotate_left_qw_words<2>( tr_x0_x1_y0_y1 ) ); // [ x1 00 y1 00 ]
    result.m_Row[4].m_V.m_V = tr_x0_x1_y0_y1;                                        // [ x0 x1 ]
    result.m_Row[5].m_V.m_V = tr_y0_y1_x0_x1;                                        // [ y0 y1 ]
    
    #elif defined( _SSE_ )                                                                                                
    
    // transpose top-left 2x4
    const __m128 tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const __m128 tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const __m128 tl_x0_x1_y0_y1( sse_merge_xy( tl_x0_y0_z0_w0, tl_x1_y1_z1_w1 ) ); // [ x0 x1 y0 y1 ]
    const __m128 tl_z0_z1_w0_w1( sse_merge_zw( tl_x0_y0_z0_w0, tl_x1_y1_z1_w1 ) ); // [ z0 z1 w0 w1 ]
    const __m128 tl_y0_y1_x0_x1( sse_rotate_left_qw_words<2>( tl_x0_x1_y0_y1 ) );  // [ y0 y1 x0 x1 ]
    const __m128 tl_w0_w1_z0_z1( sse_rotate_left_qw_words<2>( tl_z0_z1_w0_w1 ) );  // [ w0 w1 z0 z1 ]
    result.m_Row[0].m_V.m_V = tl_x0_x1_y0_y1;                                      // [ x0 x1 ]
    result.m_Row[1].m_V.m_V = tl_y0_y1_x0_x1;                                      // [ y0 y1 ]
    result.m_Row[2].m_V.m_V = tl_z0_z1_w0_w1;                                      // [ z0 z1 ]
    result.m_Row[3].m_V.m_V = tl_w0_w1_z0_z1;                                      // [ w0 w1 ]
    
    // transpose top-right 2x2 to bottom-left 2x2
    const __m128 tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                           // [ x0 y0 ]
    const __m128 tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                           // [ x1 y1 ]
    const __m128 tr_x0_x1_y0_y1( sse_merge_xy( tr_x0_y0, tr_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    const __m128 tr_y0_y1_x0_x1( sse_rotate_left_qw_words<2>( tr_x0_x1_y0_y1 ) ); // [ x1 00 y1 00 ]
    result.m_Row[4].m_V.m_V = tr_x0_x1_y0_y1;                                     // [ x0 x1 ]
    result.m_Row[5].m_V.m_V = tr_y0_y1_x0_x1;                                     // [ y0 y1 ]
    
    #else
    
    for ( U32 r=0; r<src.m_Rows; r++ )
    {
        for ( U32 c=0; c<src.m_Cols; c++ )
        {
            result.m_Row[c].m_V.m_V[r] = src.m_Row[r].m_V[c];
        }
    }
    
    #endif
    
    return result;
}


/// Mat26 Negation Operator
NOALIAS __forceinline const Mat26 operator - ( const Mat26& a )
{
    Mat26 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    return result;
}

/// Mat26 Addition Operator
NOALIAS __forceinline const Mat26 operator + ( const Mat26& a, const Mat26& b )
{
    Mat26 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    return result;
}

/// Mat26 Subtraction Operator
NOALIAS __forceinline const Mat26 operator - ( const Mat26& a, const Mat26& b )
{
    Mat26 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    return result;
}

/// Mat26 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat26& a, const Mat26& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
}

/// Mat26 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat26& a, const Mat26& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
}


/// Mat26 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector2 operator * ( const Mat26& a, const Vector6& b )
{
    #if defined( _VMX128_ )
    
    return MakeVector2( Dot( Row<0>( a ), b ),
                        Dot( Row<1>( a ), b ) );
    
    #else
    
    const Mat62 at( Transpose( a ) );
    
	return MulAdd( Row<0>( at ), Element<0>( b ), Row<1>( at ) * Element<1>( b ) ) +
	       MulAdd( Row<2>( at ), Element<2>( b ), Row<3>( at ) * Element<3>( b ) ) +
	       MulAdd( Row<4>( at ), Element<4>( b ), Row<5>( at ) * Element<5>( b ) );
    
    #endif
}

/// Vector2 * Mat26 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector2 PCREF a, const Mat26& b )
{
    #if defined( _VMX128_ )
    
    const Mat62 bt( Transpose( b ) );
    
	return MakeVector6( Dot( a, Row<0>( bt ) ), 
						Dot( a, Row<1>( bt ) ),
						Dot( a, Row<2>( bt ) ),
						Dot( a, Row<3>( bt ) ),
						Dot( a, Row<4>( bt ) ),
						Dot( a, Row<5>( bt ) ) );
    
    #else
    
	return MulAdd( Element<0>( a ), Row<0>( b ), Element<1>( a ) * Row<1>( b ) );
    
    #endif
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


