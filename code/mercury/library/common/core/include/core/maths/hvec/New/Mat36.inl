#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT36_INL
#define SOFT_MATHS_HVEC_NEW_MAT36_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT36_INL
#define MATHS_HVEC_NEW_MAT36_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat36
/// @{

/// Make Mat36 from 3 Vector6 rows
NOALIAS __forceinline const Mat36 MakeMat36( const Vector6& row0, const Vector6& row1, const Vector6& row2 )
{
    Mat36 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    return result;
}


/// Make Mat36 from 2 concatenated Mat33's
NOALIAS __forceinline const Mat36 MakeMat36( const Mat33 MATREF a, const Mat33 MATREF b )
{
    Mat36 result;
    result.m_Row[0] = MakeVector6( RowX( a ), RowX( b ) );
    result.m_Row[1] = MakeVector6( RowY( a ), RowY( b ) );
    result.m_Row[2] = MakeVector6( RowZ( a ), RowZ( b ) );
    return result;
}


NOALIAS __forceinline const Mat36 Const< Mat36 >::Zero() ///< Constant Mat36 with all elements zeroed
{
    Mat36 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] = Const< Vector6 >::Zero();
    return result;
}


/// Mat36 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat36& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat36::m_Rows ) && ( C < Mat36::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat36 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat36* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat36::m_Rows ) && ( C < Mat36::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat36 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat36& src )
{
    BIZ_STATIC_ASSERT( R < Mat36::m_Rows );
    return src.m_Row[R];
}

/// Mat36 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat36* const __restrict pDst, const Vector6& row )
{
    BIZ_STATIC_ASSERT( R < Mat36::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat36 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector3 Col( const Mat36& src )
{
    BIZ_STATIC_ASSERT( C < Mat36::m_Cols );
    return MakeVector3( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ) );
}


/// Mat36 Transpose
NOALIAS __forceinline const Mat63 Transpose( const Mat36& src )
{
    Mat63 result;
    
    #if defined( _SPU_ )
    
    const vector float zero( Const< vector float >::Zero() );
    
    // transpose top-left 3x4
    const vector float tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const vector float tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const vector float tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V0.m_V );                      // [ x2 y2 z2 w2 ]
    const vector float tl_x0_x2_y0_y2( spu_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const vector float tl_x1_00_y1_00( spu_merge_xy( tl_x1_y1_z1_w1, zero           ) ); // [ x1 00 y1 00 ]
    const vector float tl_z0_z2_w0_w2( spu_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const vector float tl_z1_00_w1_00( spu_merge_zw( tl_x1_y1_z1_w1, zero           ) ); // [ z1 00 w1 00 ]
    result.m_Row[0].m_V.m_V = spu_merge_xy( tl_x0_x2_y0_y2, tl_x1_00_y1_00 );            // [ x0 x1 x2 ]
    result.m_Row[1].m_V.m_V = spu_merge_zw( tl_x0_x2_y0_y2, tl_x1_00_y1_00 );            // [ y0 y1 y2 ]
    result.m_Row[2].m_V.m_V = spu_merge_xy( tl_z0_z2_w0_w2, tl_z1_00_w1_00 );            // [ z0 z1 z2 ]
    result.m_Row[3].m_V.m_V = spu_merge_zw( tl_z0_z2_w0_w2, tl_z1_00_w1_00 );            // [ w0 w1 w2 ]
    
    // transpose top-right 3x2 to bottom-left 2x3
    const vector float tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                 // [ x0 y0 ]
    const vector float tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                 // [ x1 y1 ]
    const vector float tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                 // [ x2 y2 ]
    const vector float tr_x0_x2_y0_y2( spu_merge_xy( tr_x0_y0, tr_x2_y2 ) );  // [ x0 x2 y0 y2 ]
    const vector float tr_x1_00_y1_00( spu_merge_xy( tr_x1_y1, zero     ) );  // [ x1 00 y1 00 ]
    result.m_Row[4].m_V.m_V = spu_merge_xy( tr_x0_x2_y0_y2, tr_x1_00_y1_00 ); // [ x0 x1 x2 ]
    result.m_Row[5].m_V.m_V = spu_merge_zw( tr_x0_x2_y0_y2, tr_x1_00_y1_00 ); // [ y0 y1 y2 ]
    
    #elif defined( _PPU_ )                                                                                                
    
    const vector float zero( Const< vector float >::Zero() );
    
    // transpose top-left 3x4
    const vector float tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const vector float tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const vector float tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V0.m_V );                      // [ x2 y2 z2 w2 ]
    const vector float tl_x0_x2_y0_y2( vec_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const vector float tl_x1_00_y1_00( vec_merge_xy( tl_x1_y1_z1_w1, zero           ) ); // [ x1 00 y1 00 ]
    const vector float tl_z0_z2_w0_w2( vec_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const vector float tl_z1_00_w1_00( vec_merge_zw( tl_x1_y1_z1_w1, zero           ) ); // [ z1 00 w1 00 ]
    result.m_Row[0].m_V.m_V = vec_merge_xy( tl_x0_x2_y0_y2, tl_x1_00_y1_00 );            // [ x0 x1 x2 ]
    result.m_Row[1].m_V.m_V = vec_merge_zw( tl_x0_x2_y0_y2, tl_x1_00_y1_00 );            // [ y0 y1 y2 ]
    result.m_Row[2].m_V.m_V = vec_merge_xy( tl_z0_z2_w0_w2, tl_z1_00_w1_00 );            // [ z0 z1 z2 ]
    result.m_Row[3].m_V.m_V = vec_merge_zw( tl_z0_z2_w0_w2, tl_z1_00_w1_00 );            // [ w0 w1 w2 ]
    
    // transpose top-right 3x2 to bottom-left 2x3
    const vector float tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                 // [ x0 y0 ]
    const vector float tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                 // [ x1 y1 ]
    const vector float tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                 // [ x2 y2 ]
    const vector float tr_x0_x2_y0_y2( vec_merge_xy( tr_x0_y0, tr_x2_y2 ) );  // [ x0 x2 y0 y2 ]
    const vector float tr_x1_00_y1_00( vec_merge_xy( tr_x1_y1, zero     ) );  // [ x1 00 y1 00 ]
    result.m_Row[4].m_V.m_V = vec_merge_xy( tr_x0_x2_y0_y2, tr_x1_00_y1_00 ); // [ x0 x1 x2 ]
    result.m_Row[5].m_V.m_V = vec_merge_zw( tr_x0_x2_y0_y2, tr_x1_00_y1_00 ); // [ y0 y1 y2 ]
    
    #elif defined( _VMX128_ )                                                                                             
    
    const __vector4 zero( Const< __vector4 >::Zero() );
    
    // transpose top-left 3x4
    const __vector4 tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const __vector4 tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const __vector4 tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V0.m_V );                      // [ x2 y2 z2 w2 ]
    const __vector4 tl_x0_x2_y0_y2( vmx_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const __vector4 tl_x1_00_y1_00( vmx_merge_xy( tl_x1_y1_z1_w1, zero           ) ); // [ x1 00 y1 00 ]
    const __vector4 tl_z0_z2_w0_w2( vmx_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const __vector4 tl_z1_00_w1_00( vmx_merge_zw( tl_x1_y1_z1_w1, zero           ) ); // [ z1 00 w1 00 ]
    result.m_Row[0].m_V.m_V = vmx_merge_xy( tl_x0_x2_y0_y2, tl_x1_00_y1_00 );         // [ x0 x1 x2 ]
    result.m_Row[1].m_V.m_V = vmx_merge_zw( tl_x0_x2_y0_y2, tl_x1_00_y1_00 );         // [ y0 y1 y2 ]
    result.m_Row[2].m_V.m_V = vmx_merge_xy( tl_z0_z2_w0_w2, tl_z1_00_w1_00 );         // [ z0 z1 z2 ]
    result.m_Row[3].m_V.m_V = vmx_merge_zw( tl_z0_z2_w0_w2, tl_z1_00_w1_00 );         // [ w0 w1 w2 ]
    
    // transpose top-right 3x2 to bottom-left 2x3
    const __vector4 tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                    // [ x0 y0 ]
    const __vector4 tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                    // [ x1 y1 ]
    const __vector4 tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                    // [ x2 y2 ]
    const __vector4 tr_x0_x2_y0_y2( vmx_merge_xy( tr_x0_y0, tr_x2_y2 ) );     // [ x0 x2 y0 y2 ]
    const __vector4 tr_x1_00_y1_00( vmx_merge_xy( tr_x1_y1, zero     ) );     // [ x1 00 y1 00 ]
    result.m_Row[4].m_V.m_V = vmx_merge_xy( tr_x0_x2_y0_y2, tr_x1_00_y1_00 ); // [ x0 x1 x2 ]
    result.m_Row[5].m_V.m_V = vmx_merge_zw( tr_x0_x2_y0_y2, tr_x1_00_y1_00 ); // [ y0 y1 y2 ]
    
    #elif defined( _SSE_ )                                                                                                
    
    const __m128 zero( Const< __m128 >::Zero() );
    
    // transpose top-left 3x4
    const __m128 tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const __m128 tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const __m128 tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V0.m_V );                      // [ x2 y2 z2 w2 ]
    const __m128 tl_x0_x2_y0_y2( sse_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const __m128 tl_x1_00_y1_00( sse_merge_xy( tl_x1_y1_z1_w1, zero           ) ); // [ x1 00 y1 00 ]
    const __m128 tl_z0_z2_w0_w2( sse_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const __m128 tl_z1_00_w1_00( sse_merge_zw( tl_x1_y1_z1_w1, zero           ) ); // [ z1 00 w1 00 ]
    result.m_Row[0].m_V.m_V = sse_merge_xy( tl_x0_x2_y0_y2, tl_x1_00_y1_00 );      // [ x0 x1 x2 ]
    result.m_Row[1].m_V.m_V = sse_merge_zw( tl_x0_x2_y0_y2, tl_x1_00_y1_00 );      // [ y0 y1 y2 ]
    result.m_Row[2].m_V.m_V = sse_merge_xy( tl_z0_z2_w0_w2, tl_z1_00_w1_00 );      // [ z0 z1 z2 ]
    result.m_Row[3].m_V.m_V = sse_merge_zw( tl_z0_z2_w0_w2, tl_z1_00_w1_00 );      // [ w0 w1 w2 ]
    
    // transpose top-right 3x2 to bottom-left 2x3
    const __m128 tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                       // [ x0 y0 ]
    const __m128 tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                       // [ x1 y1 ]
    const __m128 tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                       // [ x2 y2 ]
    const __m128 tr_x0_x2_y0_y2( sse_merge_xy( tr_x0_y0, tr_x2_y2 ) );        // [ x0 x2 y0 y2 ]
    const __m128 tr_x1_00_y1_00( sse_merge_xy( tr_x1_y1, zero     ) );        // [ x1 00 y1 00 ]
    result.m_Row[4].m_V.m_V = sse_merge_xy( tr_x0_x2_y0_y2, tr_x1_00_y1_00 ); // [ x0 x1 x2 ]
    result.m_Row[5].m_V.m_V = sse_merge_zw( tr_x0_x2_y0_y2, tr_x1_00_y1_00 ); // [ y0 y1 y2 ]
    
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


/// Mat36 Negation Operator
NOALIAS __forceinline const Mat36 operator - ( const Mat36& a )
{
    Mat36 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    return result;
}

/// Mat36 Addition Operator
NOALIAS __forceinline const Mat36 operator + ( const Mat36& a, const Mat36& b )
{
    Mat36 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    return result;
}

/// Mat36 Subtraction Operator
NOALIAS __forceinline const Mat36 operator - ( const Mat36& a, const Mat36& b )
{
    Mat36 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    return result;
}

/// Mat36 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat36& a, const Mat36& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
}

/// Mat36 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat36& a, const Mat36& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
}


/// Mat36 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector3 operator * ( const Mat36& a, const Vector6& b )
{
    #if defined( _VMX128_ )
    
    return MakeVector3( Dot( Row<0>( a ), b ),
                        Dot( Row<1>( a ), b ),
                        Dot( Row<2>( a ), b ) );
    
    #else
    
    const Mat63 at( Transpose( a ) );
    
	return MulAdd( Row<0>( at ), Element<0>( b ), Row<1>( at ) * Element<1>( b ) ) +
	       MulAdd( Row<2>( at ), Element<2>( b ), Row<3>( at ) * Element<3>( b ) ) +
	       MulAdd( Row<4>( at ), Element<4>( b ), Row<5>( at ) * Element<5>( b ) );
    
    #endif
}

/// Vector3 * Mat36 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector3 PCREF a, const Mat36& b )
{
    #if defined( _VMX128_ )
    
    const Mat63 bt( Transpose( b ) );
    
	return MakeVector6( Dot( a, Row<0>( bt ) ), 
						Dot( a, Row<1>( bt ) ),
						Dot( a, Row<2>( bt ) ),
						Dot( a, Row<3>( bt ) ),
						Dot( a, Row<4>( bt ) ),
						Dot( a, Row<5>( bt ) ) );
    
    #else
    
	return MulAdd( Element<0>( a ), Row<0>( b ), Element<1>( a ) * Row<1>( b ) ) +
	               Element<2>( a ) * Row<2>( b );
    
    #endif
}




/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


