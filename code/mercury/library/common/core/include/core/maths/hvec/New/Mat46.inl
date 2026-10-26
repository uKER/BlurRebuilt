#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT46_INL
#define SOFT_MATHS_HVEC_NEW_MAT46_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT46_INL
#define MATHS_HVEC_NEW_MAT46_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat46
/// @{

/// Make Mat46 from 4 Vector6 rows
NOALIAS __forceinline const Mat46 MakeMat46( const Vector6& row0, const Vector6& row1, const Vector6& row2, const Vector6& row3 )
{
    Mat46 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    return result;
}


NOALIAS __forceinline const Mat46 Const< Mat46 >::Zero() ///< Constant Mat46 with all elements zeroed
{
    Mat46 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] = Const< Vector6 >::Zero();
    return result;
}


/// Mat46 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat46& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat46::m_Rows ) && ( C < Mat46::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat46 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat46* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat46::m_Rows ) && ( C < Mat46::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat46 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat46& src )
{
    BIZ_STATIC_ASSERT( R < Mat46::m_Rows );
    return src.m_Row[R];
}

/// Mat46 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat46* const __restrict pDst, const Vector6& row )
{
    BIZ_STATIC_ASSERT( R < Mat46::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat46 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector4 Col( const Mat46& src )
{
    BIZ_STATIC_ASSERT( C < Mat46::m_Cols );
    return MakeVector4( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ),
                        Element<3,C>( src ) );
}


/// Mat46 Transpose
NOALIAS __forceinline const Mat64 Transpose( const Mat46& src )
{
    Mat64 result;
    
    #if defined( _SPU_ )
    
    // transpose top-left 4x4
    const vector float tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const vector float tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const vector float tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V0.m_V );                      // [ x2 y2 z2 w2 ]
    const vector float tl_x3_y3_z3_w3( src.m_Row[3].m_V.m_V0.m_V );                      // [ x3 y3 z3 w3 ]
    const vector float tl_x0_x2_y0_y2( spu_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const vector float tl_x1_x3_y1_y3( spu_merge_xy( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const vector float tl_z0_z2_w0_w2( spu_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const vector float tl_z1_z3_w1_w3( spu_merge_zw( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    result.m_Row[0].m_V.m_V = spu_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );            // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V = spu_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );            // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V = spu_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );            // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V = spu_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );            // [ w0 w1 w2 w3 ]
    
    // transpose top-right 4x2 to bottom-left 2x4
    const vector float tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                 // [ x0 y0 ]
    const vector float tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                 // [ x1 y1 ]
    const vector float tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                 // [ x2 y2 ]
    const vector float tr_x3_y3( src.m_Row[3].m_V.m_V1.m_V );                 // [ x3 y3 ]
    const vector float tr_x0_x2_y0_y2( spu_merge_xy( tr_x0_y0, tr_x2_y2 ) );  // [ x0 x2 y0 y2 ]
    const vector float tr_x1_x3_y1_y3( spu_merge_xy( tr_x1_y1, tr_x3_y3 ) );  // [ x1 x3 y1 y3 ]
    result.m_Row[4].m_V.m_V = spu_merge_xy( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[5].m_V.m_V = spu_merge_zw( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    #elif defined( _PPU_ )                                                                                                
    
    // transpose top-left 4x4
    const vector float tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const vector float tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const vector float tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V0.m_V );                      // [ x2 y2 z2 w2 ]
    const vector float tl_x3_y3_z3_w3( src.m_Row[3].m_V.m_V0.m_V );                      // [ x3 y3 z3 w3 ]
    const vector float tl_x0_x2_y0_y2( vec_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const vector float tl_x1_x3_y1_y3( vec_merge_xy( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const vector float tl_z0_z2_w0_w2( vec_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const vector float tl_z1_z3_w1_w3( vec_merge_zw( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    result.m_Row[0].m_V.m_V = vec_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );            // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V = vec_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );            // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V = vec_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );            // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V = vec_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );            // [ w0 w1 w2 w3 ]
    
    // transpose top-right 4x2 to bottom-left 2x4
    const vector float tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                 // [ x0 y0 ]
    const vector float tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                 // [ x1 y1 ]
    const vector float tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                 // [ x2 y2 ]
    const vector float tr_x3_y3( src.m_Row[3].m_V.m_V1.m_V );                 // [ x3 y3 ]
    const vector float tr_x0_x2_y0_y2( vec_merge_xy( tr_x0_y0, tr_x2_y2 ) );  // [ x0 x2 y0 y2 ]
    const vector float tr_x1_x3_y1_y3( vec_merge_xy( tr_x1_y1, tr_x3_y3 ) );  // [ x1 x3 y1 y3 ]
    result.m_Row[4].m_V.m_V = vec_merge_xy( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[5].m_V.m_V = vec_merge_zw( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    #elif defined( _VMX128_ )                                                                                             
    
    // transpose top-left 4x4
    const __vector4 tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const __vector4 tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const __vector4 tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V0.m_V );                      // [ x2 y2 z2 w2 ]
    const __vector4 tl_x3_y3_z3_w3( src.m_Row[3].m_V.m_V0.m_V );                      // [ x3 y3 z3 w3 ]
    const __vector4 tl_x0_x2_y0_y2( vmx_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const __vector4 tl_x1_x3_y1_y3( vmx_merge_xy( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const __vector4 tl_z0_z2_w0_w2( vmx_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const __vector4 tl_z1_z3_w1_w3( vmx_merge_zw( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    result.m_Row[0].m_V.m_V = vmx_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );         // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V = vmx_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );         // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V = vmx_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );         // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V = vmx_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );         // [ w0 w1 w2 w3 ]
    
    // transpose top-right 4x2 to bottom-left 2x4
    const __vector4 tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                    // [ x0 y0 ]
    const __vector4 tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                    // [ x1 y1 ]
    const __vector4 tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                    // [ x2 y2 ]
    const __vector4 tr_x3_y3( src.m_Row[3].m_V.m_V1.m_V );                    // [ x3 y3 ]
    const __vector4 tr_x0_x2_y0_y2( vmx_merge_xy( tr_x0_y0, tr_x2_y2 ) );     // [ x0 x2 y0 y2 ]
    const __vector4 tr_x1_x3_y1_y3( vmx_merge_xy( tr_x1_y1, tr_x3_y3 ) );     // [ x1 x3 y1 y3 ]
    result.m_Row[4].m_V.m_V = vmx_merge_xy( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[5].m_V.m_V = vmx_merge_zw( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    #elif defined( _SSE_ )                                                                                                
    
    // transpose top-left 4x4
    const __m128 tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const __m128 tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
    const __m128 tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V0.m_V );                      // [ x2 y2 z2 w2 ]
    const __m128 tl_x3_y3_z3_w3( src.m_Row[3].m_V.m_V0.m_V );                      // [ x3 y3 z3 w3 ]
    const __m128 tl_x0_x2_y0_y2( sse_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const __m128 tl_x1_x3_y1_y3( sse_merge_xy( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const __m128 tl_z0_z2_w0_w2( sse_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const __m128 tl_z1_z3_w1_w3( sse_merge_zw( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    result.m_Row[0].m_V.m_V = sse_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );      // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V = sse_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );      // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V = sse_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );      // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V = sse_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );      // [ w0 w1 w2 w3 ]
    
    // transpose top-right 4x2 to bottom-left 2x4
    const __m128 tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                       // [ x0 y0 ]
    const __m128 tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                       // [ x1 y1 ]
    const __m128 tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                       // [ x2 y2 ]
    const __m128 tr_x3_y3( src.m_Row[3].m_V.m_V1.m_V );                       // [ x3 y3 ]
    const __m128 tr_x0_x2_y0_y2( sse_merge_xy( tr_x0_y0, tr_x2_y2 ) );        // [ x0 x2 y0 y2 ]
    const __m128 tr_x1_x3_y1_y3( sse_merge_xy( tr_x1_y1, tr_x3_y3 ) );        // [ x1 x3 y1 y3 ]
    result.m_Row[4].m_V.m_V = sse_merge_xy( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[5].m_V.m_V = sse_merge_zw( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
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


/// Mat46 Negation Operator
NOALIAS __forceinline const Mat46 operator - ( const Mat46& a )
{
    Mat46 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    return result;
}

/// Mat46 Addition Operator
NOALIAS __forceinline const Mat46 operator + ( const Mat46& a, const Mat46& b )
{
    Mat46 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    return result;
}

/// Mat46 Subtraction Operator
NOALIAS __forceinline const Mat46 operator - ( const Mat46& a, const Mat46& b )
{
    Mat46 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    return result;
}

/// Mat46 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat46& a, const Mat46& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
}

/// Mat46 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat46& a, const Mat46& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
}


/// Mat46 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector4 operator * ( const Mat46& a, const Vector6& b )
{
    #if defined( _VMX128_ )
    
    return MakeVector4( Dot( Row<0>( a ), b ),
                        Dot( Row<1>( a ), b ),
                        Dot( Row<2>( a ), b ),
                        Dot( Row<3>( a ), b ) );
    
    #else
    
    const Mat64 at( Transpose( a ) );
    
	return MulAdd( Row<0>( at ), Element<0>( b ), Row<1>( at ) * Element<1>( b ) ) +
	       MulAdd( Row<2>( at ), Element<2>( b ), Row<3>( at ) * Element<3>( b ) ) +
	       MulAdd( Row<4>( at ), Element<4>( b ), Row<5>( at ) * Element<5>( b ) );
    
    #endif
}

/// Vector4 * Mat46 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector4 PCREF a, const Mat46& b )
{
    #if defined( _VMX128_ )
    
    const Mat64 bt( Transpose( b ) );
    
	return MakeVector6( Dot( a, Row<0>( bt ) ), 
						Dot( a, Row<1>( bt ) ),
						Dot( a, Row<2>( bt ) ),
						Dot( a, Row<3>( bt ) ),
						Dot( a, Row<4>( bt ) ),
						Dot( a, Row<5>( bt ) ) );
    
    #else
    
	return MulAdd( Element<0>( a ), Row<0>( b ), Element<1>( a ) * Row<1>( b ) ) +
	       MulAdd( Element<2>( a ), Row<2>( b ), Element<3>( a ) * Row<3>( b ) );
    
    #endif
}





/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


