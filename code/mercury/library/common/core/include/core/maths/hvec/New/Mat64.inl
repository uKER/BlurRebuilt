#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT64_INL
#define SOFT_MATHS_HVEC_NEW_MAT64_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT64_INL
#define MATHS_HVEC_NEW_MAT64_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat64
/// @{

/// Make Mat64 from 6 Vector4 rows
NOALIAS __forceinline const Mat64 MakeMat64( const Vector4 PCREF row0, const Vector4 PCREF row1, const Vector4 PCREF row2, const Vector4 PCREF row3,
                                             const Vector4 PCREF row4, const Vector4 PCREF row5 )
{
    Mat64 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    result.m_Row[4] = row4;
    result.m_Row[5] = row5;
    return result;
}


NOALIAS __forceinline const Mat64 Const< Mat64 >::Zero() ///< Constant Mat64 with all elements zeroed
{
    Mat64 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] =
    result.m_Row[4] =
    result.m_Row[5] = Const< Vector4 >::Zero();
    return result;
}


/// Mat64 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat64& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat64::m_Rows ) && ( C < Mat64::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat64 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat64* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat64::m_Rows ) && ( C < Mat64::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat64 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector4 Row( const Mat64& src )
{
    BIZ_STATIC_ASSERT( R < Mat64::m_Rows );
    return src.m_Row[R];
}

/// Mat64 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat64* const __restrict pDst, const Vector4 PCREF row )
{
    BIZ_STATIC_ASSERT( R < Mat64::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat64 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat64& src )
{
    BIZ_STATIC_ASSERT( C < Mat64::m_Cols );
    return MakeVector6( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ),
                        Element<3,C>( src ),
                        Element<4,C>( src ),
                        Element<5,C>( src ) );
}


/// Mat64 Transpose
NOALIAS __forceinline const Mat46 Transpose( const Mat64& src )
{
    Mat46 result;
    
    #if defined( _SPU_ )
    
    // transpose top-left 4x4
    const vector float tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V );                           // [ x0 y0 z0 w0 ]
    const vector float tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V );                           // [ x1 y1 z1 w1 ]
    const vector float tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V );                           // [ x2 y2 z2 w2 ]
    const vector float tl_x3_y3_z3_w3( src.m_Row[3].m_V.m_V );                           // [ x3 y3 z3 w3 ]
    const vector float tl_x0_x2_y0_y2( spu_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const vector float tl_x1_x3_y1_y3( spu_merge_xy( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const vector float tl_z0_z2_w0_w2( spu_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const vector float tl_z1_z3_w1_w3( spu_merge_zw( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    result.m_Row[0].m_V.m_V0.m_V = spu_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );       // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = spu_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );       // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V0.m_V = spu_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );       // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V0.m_V = spu_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );       // [ w0 w1 w2 w3 ]
    
    // transpose bottom-left 2x4 to top-right 4x2
    const vector float bl_x0_y0_z0_w0( src.m_Row[4].m_V.m_V );                          // [ x0 y0 z0 w0 ]
    const vector float bl_x1_y1_z1_w1( src.m_Row[5].m_V.m_V );                          // [ x1 y1 z1 w1 ]
    //const vector float bl_y0_z0_w0_x0( spu_rotate_left_qw_words<1>( bl_x0_y0_z0_w0 ) ); // [ y0 z0 w0 x0 ]
    //const vector float bl_y1_z1_w1_x1( spu_rotate_left_qw_words<1>( bl_x1_y1_z1_w1 ) ); // [ y1 z1 w1 x1 ]
    //result.m_Row[0].m_V.m_V1.m_V = spu_merge_xy( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 );      // [ x0 x1 ]
    //result.m_Row[1].m_V.m_V1.m_V = spu_merge_xy( bl_y0_z0_w0_x0, bl_y1_z1_w1_x1 );      // [ y0 y1 ]
    //result.m_Row[2].m_V.m_V1.m_V = spu_merge_zw( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 );      // [ z0 z1 ]
    //result.m_Row[3].m_V.m_V1.m_V = spu_merge_zw( bl_y0_z0_w0_x0, bl_y1_z1_w1_x1 );      // [ w0 w1 ]
    const vector float bl_x0_x1_y0_y1( spu_merge_xy( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ) ); // [ x0 x1 y0 y1 ]
    const vector float bl_z0_z1_w0_w1( spu_merge_zw( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ) ); // [ z0 z1 w0 w1 ]
    const vector float bl_y0_y1_x0_x1( spu_rotate_left_qw_words<2>( bl_x0_x1_y0_y1 ) );  // [ x0 x1 y0 y1 ]
    const vector float bl_w0_w1_z0_z1( spu_rotate_left_qw_words<2>( bl_z0_z1_w0_w1 ) );  // [ z0 z1 w0 w1 ]
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                       // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                       // [ y0 y1 ]
    result.m_Row[2].m_V.m_V1.m_V = bl_z0_z1_w0_w1;                                       // [ z0 z1 ]
    result.m_Row[3].m_V.m_V1.m_V = bl_w0_w1_z0_z1;                                       // [ w0 w1 ]
    
    #elif defined( _PPU_ )                                                                                                
    
    // transpose top-left 4x4
    const vector float tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V );                           // [ x0 y0 z0 w0 ]
    const vector float tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V );                           // [ x1 y1 z1 w1 ]
    const vector float tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V );                           // [ x2 y2 z2 w2 ]
    const vector float tl_x3_y3_z3_w3( src.m_Row[3].m_V.m_V );                           // [ x3 y3 z3 w3 ]
    const vector float tl_x0_x2_y0_y2( vec_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const vector float tl_x1_x3_y1_y3( vec_merge_xy( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const vector float tl_z0_z2_w0_w2( vec_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const vector float tl_z1_z3_w1_w3( vec_merge_zw( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    result.m_Row[0].m_V.m_V0.m_V = vec_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );       // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = vec_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );       // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V0.m_V = vec_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );       // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V0.m_V = vec_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );       // [ w0 w1 w2 w3 ]
    
    // transpose bottom-left 2x4 to top-right 4x2
    const vector float bl_x0_y0_z0_w0( src.m_Row[4].m_V.m_V );                          // [ x0 y0 z0 w0 ]
    const vector float bl_x1_y1_z1_w1( src.m_Row[5].m_V.m_V );                          // [ x1 y1 z1 w1 ]
    //const vector float bl_y0_z0_w0_x0( vec_rotate_left_qw_words<1>( bl_x0_y0_z0_w0 ) ); // [ y0 z0 w0 x0 ]
    //const vector float bl_y1_z1_w1_x1( vec_rotate_left_qw_words<1>( bl_x1_y1_z1_w1 ) ); // [ y1 z1 w1 x1 ]
    //result.m_Row[0].m_V.m_V1.m_V = vec_merge_xy( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 );      // [ x0 x1 ]
    //result.m_Row[1].m_V.m_V1.m_V = vec_merge_xy( bl_y0_z0_w0_x0, bl_y1_z1_w1_x1 );      // [ y0 y1 ]
    //result.m_Row[2].m_V.m_V1.m_V = vec_merge_zw( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 );      // [ z0 z1 ]
    //result.m_Row[3].m_V.m_V1.m_V = vec_merge_zw( bl_y0_z0_w0_x0, bl_y1_z1_w1_x1 );      // [ w0 w1 ]
    const vector float bl_x0_x1_y0_y1( vec_merge_xy( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ) ); // [ x0 x1 y0 y1 ]
    const vector float bl_z0_z1_w0_w1( vec_merge_zw( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ) ); // [ z0 z1 w0 w1 ]
    const vector float bl_y0_y1_x0_x1( vec_rotate_left_qw_words<2>( bl_x0_x1_y0_y1 ) );  // [ x0 x1 y0 y1 ]
    const vector float bl_w0_w1_z0_z1( vec_rotate_left_qw_words<2>( bl_z0_z1_w0_w1 ) );  // [ z0 z1 w0 w1 ]
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                       // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                       // [ y0 y1 ]
    result.m_Row[2].m_V.m_V1.m_V = bl_z0_z1_w0_w1;                                       // [ z0 z1 ]
    result.m_Row[3].m_V.m_V1.m_V = bl_w0_w1_z0_z1;                                       // [ w0 w1 ]
    
    #elif defined( _VMX128_ )                                                                                             
    
    // transpose top-left 4x4
    const __vector4 tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V );                           // [ x0 y0 z0 w0 ]
    const __vector4 tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V );                           // [ x1 y1 z1 w1 ]
    const __vector4 tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V );                           // [ x2 y2 z2 w2 ]
    const __vector4 tl_x3_y3_z3_w3( src.m_Row[3].m_V.m_V );                           // [ x3 y3 z3 w3 ]
    const __vector4 tl_x0_x2_y0_y2( vmx_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const __vector4 tl_x1_x3_y1_y3( vmx_merge_xy( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const __vector4 tl_z0_z2_w0_w2( vmx_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const __vector4 tl_z1_z3_w1_w3( vmx_merge_zw( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    result.m_Row[0].m_V.m_V0.m_V = vmx_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );    // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = vmx_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );    // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V0.m_V = vmx_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );    // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V0.m_V = vmx_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );    // [ w0 w1 w2 w3 ]
    
    // transpose bottom-left 2x4 to top-right 4x2
    const __vector4 bl_x0_y0_z0_w0( src.m_Row[4].m_V.m_V );                          // [ x0 y0 z0 w0 ]
    const __vector4 bl_x1_y1_z1_w1( src.m_Row[5].m_V.m_V );                          // [ x1 y1 z1 w1 ]
    //const __vector4 bl_y0_z0_w0_x0( vmx_rotate_left_qw_words<1>( bl_x0_y0_z0_w0 ) ); // [ y0 z0 w0 x0 ]
    //const __vector4 bl_y1_z1_w1_x1( vmx_rotate_left_qw_words<1>( bl_x1_y1_z1_w1 ) ); // [ y1 z1 w1 x1 ]
    //result.m_Row[0].m_V.m_V1.m_V = vmx_merge_xy( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 );   // [ x0 x1 ]
    //result.m_Row[1].m_V.m_V1.m_V = vmx_merge_xy( bl_y0_z0_w0_x0, bl_y1_z1_w1_x1 );   // [ y0 y1 ]
    //result.m_Row[2].m_V.m_V1.m_V = vmx_merge_zw( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 );   // [ z0 z1 ]
    //result.m_Row[3].m_V.m_V1.m_V = vmx_merge_zw( bl_y0_z0_w0_x0, bl_y1_z1_w1_x1 );   // [ w0 w1 ]
    const __vector4 bl_x0_x1_y0_y1( vmx_merge_xy( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ) ); // [ x0 x1 y0 y1 ]
    const __vector4 bl_z0_z1_w0_w1( vmx_merge_zw( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ) ); // [ z0 z1 w0 w1 ]
    const __vector4 bl_y0_y1_x0_x1( vmx_rotate_left_qw_words<2>( bl_x0_x1_y0_y1 ) );  // [ x0 x1 y0 y1 ]
    const __vector4 bl_w0_w1_z0_z1( vmx_rotate_left_qw_words<2>( bl_z0_z1_w0_w1 ) );  // [ z0 z1 w0 w1 ]
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                    // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                    // [ y0 y1 ]
    result.m_Row[2].m_V.m_V1.m_V = bl_z0_z1_w0_w1;                                    // [ z0 z1 ]
    result.m_Row[3].m_V.m_V1.m_V = bl_w0_w1_z0_z1;                                    // [ w0 w1 ]
    
    #elif defined( _SSE_ )                                                                                                
    
    // transpose top-left 4x4
    const __m128 tl_x0_y0_z0_w0( src.m_Row[0].m_V.m_V );                           // [ x0 y0 z0 w0 ]
    const __m128 tl_x1_y1_z1_w1( src.m_Row[1].m_V.m_V );                           // [ x1 y1 z1 w1 ]
    const __m128 tl_x2_y2_z2_w2( src.m_Row[2].m_V.m_V );                           // [ x2 y2 z2 w2 ]
    const __m128 tl_x3_y3_z3_w3( src.m_Row[3].m_V.m_V );                           // [ x3 y3 z3 w3 ]
    const __m128 tl_x0_x2_y0_y2( sse_merge_xy( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ x0 x2 y0 y2 ]
    const __m128 tl_x1_x3_y1_y3( sse_merge_xy( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ x1 x3 y1 y3 ]
    const __m128 tl_z0_z2_w0_w2( sse_merge_zw( tl_x0_y0_z0_w0, tl_x2_y2_z2_w2 ) ); // [ z0 z2 w0 w2 ]
    const __m128 tl_z1_z3_w1_w3( sse_merge_zw( tl_x1_y1_z1_w1, tl_x3_y3_z3_w3 ) ); // [ z1 z3 w1 w3 ]
    result.m_Row[0].m_V.m_V0.m_V = sse_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = sse_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V0.m_V = sse_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 ); // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V0.m_V = sse_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 ); // [ w0 w1 w2 w3 ]
    
    // transpose bottom-left 2x4 to top-right 4x2
    const __m128 bl_x0_y0_z0_w0( src.m_Row[4].m_V.m_V );                           // [ x0 y0 z0 w0 ]
    const __m128 bl_x1_y1_z1_w1( src.m_Row[5].m_V.m_V );                           // [ x1 y1 z1 w1 ]
    //const __m128 bl_y0_z0_w0_x0( sse_rotate_left_qw_words<1>( bl_x0_y0_z0_w0 ) );  // [ y0 z0 w0 x0 ]
    //const __m128 bl_y1_z1_w1_x1( sse_rotate_left_qw_words<1>( bl_x1_y1_z1_w1 ) );  // [ y1 z1 w1 x1 ]
    //result.m_Row[0].m_V.m_V1.m_V = sse_merge_xy( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ); // [ x0 x1 ]
    //result.m_Row[1].m_V.m_V1.m_V = sse_merge_xy( bl_y0_z0_w0_x0, bl_y1_z1_w1_x1 ); // [ y0 y1 ]
    //result.m_Row[2].m_V.m_V1.m_V = sse_merge_zw( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ); // [ z0 z1 ]
    //result.m_Row[3].m_V.m_V1.m_V = sse_merge_zw( bl_y0_z0_w0_x0, bl_y1_z1_w1_x1 ); // [ w0 w1 ]
    const __m128 bl_x0_x1_y0_y1( sse_merge_xy( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ) ); // [ x0 x1 y0 y1 ]
    const __m128 bl_z0_z1_w0_w1( sse_merge_zw( bl_x0_y0_z0_w0, bl_x1_y1_z1_w1 ) ); // [ z0 z1 w0 w1 ]
    const __m128 bl_y0_y1_x0_x1( sse_rotate_left_qw_words<2>( bl_x0_x1_y0_y1 ) );  // [ x0 x1 y0 y1 ]
    const __m128 bl_w0_w1_z0_z1( sse_rotate_left_qw_words<2>( bl_z0_z1_w0_w1 ) );  // [ z0 z1 w0 w1 ]
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                 // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                 // [ y0 y1 ]
    result.m_Row[2].m_V.m_V1.m_V = bl_z0_z1_w0_w1;                                 // [ z0 z1 ]
    result.m_Row[3].m_V.m_V1.m_V = bl_w0_w1_z0_z1;                                 // [ w0 w1 ]
    
    #else
    
    for ( U32 r=0; r<src.m_Rows; r++ )
    {
        for ( U32 c=0; c<src.m_Cols; c++ )
        {
            result.m_Row[c].m_V[r] = src.m_Row[r].m_V.m_V[c];
        }
    }
    
    #endif
    
    return result;
}


/// Mat64 Negation Operator
NOALIAS __forceinline const Mat64 operator - ( const Mat64& a )
{
    Mat64 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    result.m_Row[4] = -a.m_Row[4];
    result.m_Row[5] = -a.m_Row[5];
    return result;
}

/// Mat64 Addition Operator
NOALIAS __forceinline const Mat64 operator + ( const Mat64& a, const Mat64& b )
{
    Mat64 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] + b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] + b.m_Row[5];
    return result;
}

/// Mat64 Subtraction Operator
NOALIAS __forceinline const Mat64 operator - ( const Mat64& a, const Mat64& b )
{
    Mat64 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] - b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] - b.m_Row[5];
    return result;
}

/// Mat64 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat64& a, const Mat64& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
    a.m_Row[4] += b.m_Row[4];
    a.m_Row[5] += b.m_Row[5];
}

/// Mat64 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat64& a, const Mat64& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
    a.m_Row[4] -= b.m_Row[4];
    a.m_Row[5] -= b.m_Row[5];
}


/// Mat64 * Vector4 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat64& a, const Vector4 PCREF b )
{
    #if defined( _VMX128_ )
    
    return MakeVector6( Dot( Row<0>( a ), b ),
                        Dot( Row<1>( a ), b ),
                        Dot( Row<2>( a ), b ),
                        Dot( Row<3>( a ), b ),
                        Dot( Row<4>( a ), b ),
                        Dot( Row<5>( a ), b ) );
    
    #else
    
    const Mat46 at( Transpose( a ) );
    
	return MulAdd( Row<0>( at ), Element<0>( b ), Row<1>( at ) * Element<1>( b ) ) +
	       MulAdd( Row<2>( at ), Element<2>( b ), Row<3>( at ) * Element<3>( b ) );
    
    #endif
}


/// Vector6 * Mat64 Multiplication Operator
NOALIAS __forceinline const Vector4 operator * ( const Vector6& a, const Mat64& b )
{
    #if defined( _VMX128_ )
    
    const Mat46 bt( Transpose( b ) );
    
	return MakeVector4( Dot( a, Row<0>( bt ) ), 
						Dot( a, Row<1>( bt ) ),
						Dot( a, Row<2>( bt ) ),
						Dot( a, Row<3>( bt ) ) );
    
    #else
    
	return MulAdd( Element<0>( a ), Row<0>( b ), Element<1>( a ) * Row<1>( b ) ) +
	       MulAdd( Element<2>( a ), Row<2>( b ), Element<3>( a ) * Row<3>( b ) ) +
	       MulAdd( Element<4>( a ), Row<4>( b ), Element<5>( a ) * Row<5>( b ) );
    
    #endif
}


/// Mat64 * Mat46 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat64& a, const Mat46& b )
{
    #if defined( _VMX128_ )
    
    const Mat64 bt( Transpose( b ) );
    
    return MakeVector6( Dot( Row<0>( a ), Row<0>( bt ) ),
                        Dot( Row<1>( a ), Row<1>( bt ) ),
                        Dot( Row<2>( a ), Row<2>( bt ) ),
                        Dot( Row<3>( a ), Row<3>( bt ) ),
                        Dot( Row<4>( a ), Row<4>( bt ) ),
                        Dot( Row<5>( a ), Row<5>( bt ) ) );
    
    #else
    
    const Mat46 at( Transpose( a ) );
    
    return MulAdd( Row<0>( at ), Row<0>( b ), Row<1>( at ) * Row<1>( b ) ) +
           MulAdd( Row<2>( at ), Row<2>( b ), Row<3>( at ) * Row<3>( b ) );
    
    #endif
}

/// Mat46 * Mat64 Diagonal Multiply: Returns the diagonal of the resulting 4x4 matrix
NOALIAS __forceinline const Vector4 DiagonalMultiply( const Mat46& a, const Mat64& b )
{
    #if defined( _VMX128_ )
    
    const Mat46 bt( Transpose( b ) );
    
    return MakeVector4( Dot( Row<0>( a ), Row<0>( bt ) ),
                        Dot( Row<1>( a ), Row<1>( bt ) ),
                        Dot( Row<2>( a ), Row<2>( bt ) ),
                        Dot( Row<3>( a ), Row<3>( bt ) ) );
    
    #else
    
    const Mat64 at( Transpose( a ) );
    
    return MulAdd( Row<0>( at ), Row<0>( b ), Row<1>( at ) * Row<1>( b ) ) +
           MulAdd( Row<2>( at ), Row<2>( b ), Row<3>( at ) * Row<3>( b ) ) +
           MulAdd( Row<4>( at ), Row<4>( b ), Row<5>( at ) * Row<5>( b ) );
    
    #endif
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


