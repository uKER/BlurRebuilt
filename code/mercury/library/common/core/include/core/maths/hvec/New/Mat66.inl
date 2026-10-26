#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT66_INL
#define SOFT_MATHS_HVEC_NEW_MAT66_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT66_INL
#define MATHS_HVEC_NEW_MAT66_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat66
/// @{

/// Make Mat66 from 6 Vector6 rows
NOALIAS __forceinline const Mat66 MakeMat66( const Vector6& row0, const Vector6& row1, const Vector6& row2, const Vector6& row3,
                                             const Vector6& row4, const Vector6& row5 )
{
    Mat66 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    result.m_Row[4] = row4;
    result.m_Row[5] = row5;
    return result;
}



/// Mat66 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat66& src )
{
    #if defined( CFG_DEBUG )
    BIZ_STATIC_ASSERT( ( R < Mat66::m_Rows ) && ( C < Mat66::m_Cols ) );
    #endif
    
    return Element<C>( src.m_Row[R] );
}

/// Mat66 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat66* const __restrict pDst, const VF32 VECREF e )
{
    #if defined( CFG_DEBUG )
    BIZ_STATIC_ASSERT( ( R < Mat66::m_Rows ) && ( C < Mat66::m_Cols ) );
    #endif
    
    SetElement<C>( &pDst->m_Row[R], e );
}


NOALIAS __forceinline const Mat66 Const< Mat66 >::Zero() ///< Constant Mat66 with all elements 0
{
    Mat66 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] =
    result.m_Row[4] =
    result.m_Row[5] = Const< Vector6 >::Zero();
    return result;
}

NOALIAS __forceinline const Mat66 Const< Mat66 >::Identity() ///< Constant Mat66 with diagonal elements 1 & all other elements 0
{
    const VF32 one( Const<VF32>::One() );
    Mat66 result( Const< Mat66 >::Zero() );
    SetElement<0,0>( &result, one );
    SetElement<1,1>( &result, one );
    SetElement<2,2>( &result, one );
    SetElement<3,3>( &result, one );
    SetElement<4,4>( &result, one );
    SetElement<5,5>( &result, one );
    return result;
}


/// Mat66 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector6 Row( const Mat66& src )
{
    BIZ_STATIC_ASSERT( R < Mat66::m_Rows );
    return src.m_Row[R];
}

/// Mat66 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat66* const __restrict pDst, const Vector6& row )
{
    BIZ_STATIC_ASSERT( R < Mat66::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat66 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat66& src )
{
    BIZ_STATIC_ASSERT( C < Mat66::m_Cols );
    return MakeVector6( Element<0,C>( src ),
                        Element<1,C>( src ),
                        Element<2,C>( src ),
                        Element<3,C>( src ),
                        Element<4,C>( src ),
                        Element<5,C>( src ) );
}

/// Mat66 Accessor: Set column
template < unsigned int C >
NOALIAS __forceinline void SetCol( Mat66* const __restrict pDst, const Vector6& col )
{
    BIZ_STATIC_ASSERT( C < Mat66::m_Cols );
    SetElement<0,C>( pDst, Element<0>( col ) );
    SetElement<1,C>( pDst, Element<1>( col ) );
    SetElement<2,C>( pDst, Element<2>( col ) );
    SetElement<3,C>( pDst, Element<3>( col ) );
    SetElement<4,C>( pDst, Element<4>( col ) );
    SetElement<5,C>( pDst, Element<5>( col ) );
}


/// Mat66 Accessor: Assign column from another Mat66
template < unsigned int DstCol, unsigned int SrcCol >
#if defined( CFG_SPEED ) || defined( CFG_GOLD ) // JASOND: The 360 compiler takes forever if this is forceinlined in release builds...
NOALIAS __forceinline void ReplaceColumn( Mat66* const __restrict pDst, const Mat66& src )
#else
NOALIAS inline void ReplaceColumn( Mat66* const __restrict pDst, const Mat66& src )
#endif
{
    #if defined( CFG_DEBUG )
    BIZ_STATIC_ASSERT( ( DstCol < Mat66::m_Cols ) && ( SrcCol < Mat66::m_Cols ) );
    #endif
    
    SetElement<0,DstCol>( pDst, Element<0,SrcCol>( src ) );
    SetElement<1,DstCol>( pDst, Element<1,SrcCol>( src ) );
    SetElement<2,DstCol>( pDst, Element<2,SrcCol>( src ) );
    SetElement<3,DstCol>( pDst, Element<3,SrcCol>( src ) );
    SetElement<4,DstCol>( pDst, Element<4,SrcCol>( src ) );
    SetElement<5,DstCol>( pDst, Element<5,SrcCol>( src ) );
}


/// Mat66 Transpose
NOALIAS __forceinline const Mat66 Transpose( const Mat66& src )
{
    Mat66 result;
    
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
    result.m_Row[0].m_V.m_V0.m_V = spu_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );       // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = spu_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );       // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V0.m_V = spu_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );       // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V0.m_V = spu_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );       // [ w0 w1 w2 w3 ]
    
    // transpose bottom-left 2x4 to top-right 4x2
    const vector float bl_x0_y0_z0_w0( src.m_Row[4].m_V.m_V0.m_V );                     // [ x0 y0 z0 w0 ]
    const vector float bl_x1_y1_z1_w1( src.m_Row[5].m_V.m_V0.m_V );                     // [ x1 y1 z1 w1 ]
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
    
    // transpose top-right 4x2 to bottom-left 2x4
    const vector float tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                      // [ x0 y0 ]
    const vector float tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                      // [ x1 y1 ]
    const vector float tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                      // [ x2 y2 ]
    const vector float tr_x3_y3( src.m_Row[3].m_V.m_V1.m_V );                      // [ x3 y3 ]
    const vector float tr_x0_x2_y0_y2( spu_merge_xy( tr_x0_y0, tr_x2_y2 ) );       // [ x0 x2 y0 y2 ]
    const vector float tr_x1_x3_y1_y3( spu_merge_xy( tr_x1_y1, tr_x3_y3 ) );       // [ x1 x3 y1 y3 ]
    result.m_Row[4].m_V.m_V0.m_V = spu_merge_xy( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[5].m_V.m_V0.m_V = spu_merge_zw( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    // transpose bottom-right 2x2
    const vector float br_x0_y0( src.m_Row[4].m_V.m_V1.m_V );                     // [ x0 y0 ]
    const vector float br_x1_y1( src.m_Row[5].m_V.m_V1.m_V );                     // [ x1 y1 ]
    const vector float br_x0_x1_y0_y1( spu_merge_xy( br_x0_y0, br_x1_y1 ) );      // [ x0 x1 y0 y1 ]
    result.m_Row[4].m_V.m_V1.m_V = br_x0_x1_y0_y1;                                // [ x0 x1 ]
    result.m_Row[5].m_V.m_V1.m_V = spu_rotate_left_qw_words<2>( br_x0_x1_y0_y1 ); // [ y0 y1 ]
    
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
    result.m_Row[0].m_V.m_V0.m_V = vec_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );       // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = vec_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );       // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V0.m_V = vec_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );       // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V0.m_V = vec_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );       // [ w0 w1 w2 w3 ]
    
    // transpose bottom-left 2x4 to top-right 4x2
    const vector float bl_x0_y0_z0_w0( src.m_Row[4].m_V.m_V0.m_V );                     // [ x0 y0 z0 w0 ]
    const vector float bl_x1_y1_z1_w1( src.m_Row[5].m_V.m_V0.m_V );                     // [ x1 y1 z1 w1 ]
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
    
    // transpose top-right 4x2 to bottom-left 2x4
    const vector float tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                      // [ x0 y0 ]
    const vector float tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                      // [ x1 y1 ]
    const vector float tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                      // [ x2 y2 ]
    const vector float tr_x3_y3( src.m_Row[3].m_V.m_V1.m_V );                      // [ x3 y3 ]
    const vector float tr_x0_x2_y0_y2( vec_merge_xy( tr_x0_y0, tr_x2_y2 ) );       // [ x0 x2 y0 y2 ]
    const vector float tr_x1_x3_y1_y3( vec_merge_xy( tr_x1_y1, tr_x3_y3 ) );       // [ x1 x3 y1 y3 ]
    result.m_Row[4].m_V.m_V0.m_V = vec_merge_xy( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[5].m_V.m_V0.m_V = vec_merge_zw( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    // transpose bottom-right 2x2
    const vector float br_x0_y0( src.m_Row[4].m_V.m_V1.m_V );                     // [ x0 y0 ]
    const vector float br_x1_y1( src.m_Row[5].m_V.m_V1.m_V );                     // [ x1 y1 ]
    const vector float br_x0_x1_y0_y1( vec_merge_xy( br_x0_y0, br_x1_y1 ) );      // [ x0 x1 y0 y1 ]
    result.m_Row[4].m_V.m_V1.m_V = br_x0_x1_y0_y1;                                // [ x0 x1 ]
    result.m_Row[5].m_V.m_V1.m_V = vec_rotate_left_qw_words<2>( br_x0_x1_y0_y1 ); // [ y0 y1 ]
    
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
    result.m_Row[0].m_V.m_V0.m_V = vmx_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );    // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = vmx_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 );    // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V0.m_V = vmx_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );    // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V0.m_V = vmx_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 );    // [ w0 w1 w2 w3 ]
    
    // transpose bottom-left 2x4 to top-right 4x2
    const __vector4 bl_x0_y0_z0_w0( src.m_Row[4].m_V.m_V0.m_V );                     // [ x0 y0 z0 w0 ]
    const __vector4 bl_x1_y1_z1_w1( src.m_Row[5].m_V.m_V0.m_V );                     // [ x1 y1 z1 w1 ]
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
    
    // transpose top-right 4x2 to bottom-left 2x4
    const __vector4 tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                         // [ x0 y0 ]
    const __vector4 tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                         // [ x1 y1 ]
    const __vector4 tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                         // [ x2 y2 ]
    const __vector4 tr_x3_y3( src.m_Row[3].m_V.m_V1.m_V );                         // [ x3 y3 ]
    const __vector4 tr_x0_x2_y0_y2( vmx_merge_xy( tr_x0_y0, tr_x2_y2 ) );          // [ x0 x2 y0 y2 ]
    const __vector4 tr_x1_x3_y1_y3( vmx_merge_xy( tr_x1_y1, tr_x3_y3 ) );          // [ x1 x3 y1 y3 ]
    result.m_Row[4].m_V.m_V0.m_V = vmx_merge_xy( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[5].m_V.m_V0.m_V = vmx_merge_zw( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    // transpose bottom-right 2x2
    const __vector4 br_x0_y0( src.m_Row[4].m_V.m_V1.m_V );                        // [ x0 y0 ]
    const __vector4 br_x1_y1( src.m_Row[5].m_V.m_V1.m_V );                        // [ x1 y1 ]
    const __vector4 br_x0_x1_y0_y1( vmx_merge_xy( br_x0_y0, br_x1_y1 ) );         // [ x0 x1 y0 y1 ]
    result.m_Row[4].m_V.m_V1.m_V = br_x0_x1_y0_y1;                                // [ x0 x1 ]
    result.m_Row[5].m_V.m_V1.m_V = vmx_rotate_left_qw_words<2>( br_x0_x1_y0_y1 ); // [ y0 y1 ]
    
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
    result.m_Row[0].m_V.m_V0.m_V = sse_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = sse_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    result.m_Row[2].m_V.m_V0.m_V = sse_merge_xy( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 ); // [ z0 z1 z2 z3 ]
    result.m_Row[3].m_V.m_V0.m_V = sse_merge_zw( tl_z0_z2_w0_w2, tl_z1_z3_w1_w3 ); // [ w0 w1 w2 w3 ]
    
    // transpose bottom-left 2x4 to top-right 4x2
    const __m128 bl_x0_y0_z0_w0( src.m_Row[4].m_V.m_V0.m_V );                      // [ x0 y0 z0 w0 ]
    const __m128 bl_x1_y1_z1_w1( src.m_Row[5].m_V.m_V0.m_V );                      // [ x1 y1 z1 w1 ]
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
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                    // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                    // [ y0 y1 ]
    result.m_Row[2].m_V.m_V1.m_V = bl_z0_z1_w0_w1;                                    // [ z0 z1 ]
    result.m_Row[3].m_V.m_V1.m_V = bl_w0_w1_z0_z1;                                    // [ w0 w1 ]
    
    // transpose top-right 4x2 to bottom-left 2x4
    const __m128 tr_x0_y0( src.m_Row[0].m_V.m_V1.m_V );                            // [ x0 y0 ]
    const __m128 tr_x1_y1( src.m_Row[1].m_V.m_V1.m_V );                            // [ x1 y1 ]
    const __m128 tr_x2_y2( src.m_Row[2].m_V.m_V1.m_V );                            // [ x2 y2 ]
    const __m128 tr_x3_y3( src.m_Row[3].m_V.m_V1.m_V );                            // [ x3 y3 ]
    const __m128 tr_x0_x2_y0_y2( sse_merge_xy( tr_x0_y0, tr_x2_y2 ) );             // [ x0 x2 y0 y2 ]
    const __m128 tr_x1_x3_y1_y3( sse_merge_xy( tr_x1_y1, tr_x3_y3 ) );             // [ x1 x3 y1 y3 ]
    result.m_Row[4].m_V.m_V0.m_V = sse_merge_xy( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[5].m_V.m_V0.m_V = sse_merge_zw( tr_x0_x2_y0_y2, tr_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    // transpose bottom-right 2x2
    const __m128 br_x0_y0( src.m_Row[4].m_V.m_V1.m_V );                           // [ x0 y0 ]
    const __m128 br_x1_y1( src.m_Row[5].m_V.m_V1.m_V );                           // [ x1 y1 ]
    const __m128 br_x0_x1_y0_y1( sse_merge_xy( br_x0_y0, br_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    result.m_Row[4].m_V.m_V1.m_V = br_x0_x1_y0_y1;                                // [ x0 x1 ]
    result.m_Row[5].m_V.m_V1.m_V = sse_rotate_left_qw_words<2>( br_x0_x1_y0_y1 ); // [ y0 y1 ]
    
    #else
    
    for ( U32 r=0; r<src.m_Rows; r++ )
    {
        for ( U32 c=0; c<src.m_Cols; c++ )
        {
            result.m_Row[c].m_V[r] = src.m_Row[r].m_V[c];
        }
    }
    
    #endif
    
    return result;
}


/// Mat66 Negation Operator
NOALIAS __forceinline const Mat66 operator - ( const Mat66& a )
{
    Mat66 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    result.m_Row[4] = -a.m_Row[4];
    result.m_Row[5] = -a.m_Row[5];
    return result;
}

/// Mat66 Addition Operator
NOALIAS __forceinline const Mat66 operator + ( const Mat66& a, const Mat66& b )
{
    Mat66 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] + b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] + b.m_Row[5];
    return result;
}

/// Mat66 Subtraction Operator
NOALIAS __forceinline const Mat66 operator - ( const Mat66& a, const Mat66& b )
{
    Mat66 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] - b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] - b.m_Row[5];
    return result;
}

/// Mat66 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat66& a, const Mat66& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
    a.m_Row[4] += b.m_Row[4];
    a.m_Row[5] += b.m_Row[5];
}

/// Mat66 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat66& a, const Mat66& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
    a.m_Row[4] -= b.m_Row[4];
    a.m_Row[5] -= b.m_Row[5];
}


/// Mat66 Scalar Multiplication Operator
NOALIAS __forceinline const Mat66 operator * ( const Mat66& m, const VF32 VECREF f )
{
    Mat66 result;
	result.m_Row[0] = Row<0>( m ) * f;
	result.m_Row[1] = Row<1>( m ) * f;
	result.m_Row[2] = Row<2>( m ) * f;
	result.m_Row[3] = Row<3>( m ) * f;
	result.m_Row[4] = Row<4>( m ) * f;
	result.m_Row[5] = Row<5>( m ) * f;
	return result;
}


/// Mat66 * Vector6 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat66& a, const Vector6& b )
{
    #if defined( _VMX128_ )
    
    return MakeVector6( Dot( Row<0>( a ), b ),
                        Dot( Row<1>( a ), b ),
                        Dot( Row<2>( a ), b ),
                        Dot( Row<3>( a ), b ),
                        Dot( Row<4>( a ), b ),
                        Dot( Row<5>( a ), b ) );
    
    #else
    
    const Mat66 at( Transpose( a ) );
    
	return MulAdd( Row<0>( at ), Element<0>( b ), Row<1>( at ) * Element<1>( b ) ) +
	       MulAdd( Row<2>( at ), Element<2>( b ), Row<3>( at ) * Element<3>( b ) ) +
	       MulAdd( Row<4>( at ), Element<4>( b ), Row<5>( at ) * Element<5>( b ) );
    
    #endif
}

/// Vector6 * Mat66 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Vector6& a, const Mat66& b )
{
    #if defined( _VMX128_ )
    
    const Mat66 bt( Transpose( b ) );
    
	return MakeVector6( Dot( a, Row<0>( bt ) ), 
						Dot( a, Row<1>( bt ) ),
						Dot( a, Row<2>( bt ) ),
						Dot( a, Row<3>( bt ) ),
						Dot( a, Row<4>( bt ) ),
                        Dot( a, Row<5>( bt ) ) );
    
    #else
    
	return MulAdd( Element<0>( a ), Row<0>( b ), Element<1>( a ) * Row<1>( b ) ) +
	       MulAdd( Element<2>( a ), Row<2>( b ), Element<3>( a ) * Row<3>( b ) ) +
	       MulAdd( Element<4>( a ), Row<4>( b ), Element<5>( a ) * Row<5>( b ) );
    
    #endif
}


/// Mat66 * Mat66 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat66& a, const Mat66& b )
{
    #if defined( _VMX128_ )
    
    const Mat66 bt( Transpose( b ) );
    
    return MakeVector6( Dot( Row<0>( a ), Row<0>( bt ) ),
                        Dot( Row<1>( a ), Row<1>( bt ) ),
                        Dot( Row<2>( a ), Row<2>( bt ) ),
                        Dot( Row<3>( a ), Row<3>( bt ) ),
                        Dot( Row<4>( a ), Row<4>( bt ) ),
                        Dot( Row<5>( a ), Row<5>( bt ) ) );
    
    #else
    
    const Mat66 at( Transpose( a ) );
    
    return MulAdd( Row<0>( at ), Row<0>( b ), Row<1>( at ) * Row<1>( b ) ) +
           MulAdd( Row<2>( at ), Row<2>( b ), Row<3>( at ) * Row<3>( b ) ) +
           MulAdd( Row<4>( at ), Row<4>( b ), Row<5>( at ) * Row<5>( b ) );
    
    #endif
}

/// Mat66 Matrix Multiply Operator
NOALIAS __forceinline const Mat66 operator * ( const Mat66& a, const Mat66& b )
{
    #if defined( _VMX128_ )
    
	const Vector6 rowA0( Row<0>( a ) );
	const Vector6 rowA1( Row<1>( a ) );
	const Vector6 rowA2( Row<2>( a ) );
	const Vector6 rowA3( Row<3>( a ) );
	const Vector6 rowA4( Row<4>( a ) );
	const Vector6 rowA5( Row<5>( a ) );
                                   
    const Mat66 bt( Transpose( b ) );
	const Vector6 colB0( Row<0>( bt ) );
	const Vector6 colB1( Row<1>( bt ) );
	const Vector6 colB2( Row<2>( bt ) );
	const Vector6 colB3( Row<3>( bt ) );
	const Vector6 colB4( Row<4>( bt ) );
	const Vector6 colB5( Row<5>( bt ) );
    
	Mat66 result;
	result.m_Row[0] = MakeVector6( Dot( rowA0, colB0 ), Dot( rowA0, colB1 ), Dot( rowA0, colB2 ), Dot( rowA0, colB3 ), Dot( rowA0, colB4 ), Dot( rowA0, colB5 ) );
	result.m_Row[1] = MakeVector6( Dot( rowA1, colB0 ), Dot( rowA1, colB1 ), Dot( rowA1, colB2 ), Dot( rowA1, colB3 ), Dot( rowA1, colB4 ), Dot( rowA1, colB5 ) );
	result.m_Row[2] = MakeVector6( Dot( rowA2, colB0 ), Dot( rowA2, colB1 ), Dot( rowA2, colB2 ), Dot( rowA2, colB3 ), Dot( rowA2, colB4 ), Dot( rowA2, colB5 ) );
	result.m_Row[3] = MakeVector6( Dot( rowA3, colB0 ), Dot( rowA3, colB1 ), Dot( rowA3, colB2 ), Dot( rowA3, colB3 ), Dot( rowA3, colB4 ), Dot( rowA3, colB5 ) );
	result.m_Row[4] = MakeVector6( Dot( rowA4, colB0 ), Dot( rowA4, colB1 ), Dot( rowA4, colB2 ), Dot( rowA4, colB3 ), Dot( rowA4, colB4 ), Dot( rowA4, colB5 ) );
	result.m_Row[5] = MakeVector6( Dot( rowA5, colB0 ), Dot( rowA5, colB1 ), Dot( rowA5, colB2 ), Dot( rowA5, colB3 ), Dot( rowA5, colB4 ), Dot( rowA5, colB5 ) );
	return result;
    
    #else
    
	const Vector6 b0( Row<0>( b ) );
	const Vector6 b1( Row<1>( b ) );
	const Vector6 b2( Row<2>( b ) );
	const Vector6 b3( Row<3>( b ) );
	const Vector6 b4( Row<4>( b ) );
	const Vector6 b5( Row<5>( b ) );
	
	Mat66 result;
	
	result.m_Row[0] = MulAdd( Element<0,0>( a ), b0, Element<0,1>( a ) * b1 ) +
	                  MulAdd( Element<0,2>( a ), b2, Element<0,3>( a ) * b3 ) +
	                  MulAdd( Element<0,4>( a ), b4, Element<0,5>( a ) * b5 );
	                                               
	result.m_Row[1] = MulAdd( Element<1,0>( a ), b0, Element<1,1>( a ) * b1 ) +
	                  MulAdd( Element<1,2>( a ), b2, Element<1,3>( a ) * b3 ) +
	                  MulAdd( Element<1,4>( a ), b4, Element<1,5>( a ) * b5 );
	                                               
	result.m_Row[2] = MulAdd( Element<2,0>( a ), b0, Element<2,1>( a ) * b1 ) +
	                  MulAdd( Element<2,2>( a ), b2, Element<2,3>( a ) * b3 ) +
	                  MulAdd( Element<2,4>( a ), b4, Element<2,5>( a ) * b5 );
	                                               
	result.m_Row[3] = MulAdd( Element<3,0>( a ), b0, Element<3,1>( a ) * b1 ) +
	                  MulAdd( Element<3,2>( a ), b2, Element<3,3>( a ) * b3 ) +
	                  MulAdd( Element<3,4>( a ), b4, Element<3,5>( a ) * b5 );
	                                               
	result.m_Row[4] = MulAdd( Element<4,0>( a ), b0, Element<4,1>( a ) * b1 ) +
	                  MulAdd( Element<4,2>( a ), b2, Element<4,3>( a ) * b3 ) +
	                  MulAdd( Element<4,4>( a ), b4, Element<4,5>( a ) * b5 );
	                                               
	result.m_Row[5] = MulAdd( Element<5,0>( a ), b0, Element<5,1>( a ) * b1 ) +
	                  MulAdd( Element<5,2>( a ), b2, Element<5,3>( a ) * b3 ) +
	                  MulAdd( Element<5,4>( a ), b4, Element<5,5>( a ) * b5 );
	
	return result;
    
    #endif
}


/// Mat66 Split into four Mat33's
NOALIAS __forceinline void Split( const Mat66& m,
                                  Mat33* const __restrict pTL,
                                  Mat33* const __restrict pTR,
                                  Mat33* const __restrict pBL,
                                  Mat33* const __restrict pBR )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    Vector3 tl_RowX, tl_RowY, tl_RowZ;
    Vector3 tr_RowX, tr_RowY, tr_RowZ;
    Vector3 bl_RowX, bl_RowY, bl_RowZ;
    Vector3 br_RowX, br_RowY, br_RowZ;
    Split( m.m_Row[0], &tl_RowX, &tr_RowX );
    Split( m.m_Row[1], &tl_RowY, &tr_RowY );
    Split( m.m_Row[2], &tl_RowZ, &tr_RowZ );
    Split( m.m_Row[3], &bl_RowX, &br_RowX );
    Split( m.m_Row[4], &bl_RowY, &br_RowY );
    Split( m.m_Row[5], &bl_RowZ, &br_RowZ );
    *pTL = MakeMat33( tl_RowX, tl_RowY, tl_RowZ );
    *pTR = MakeMat33( tr_RowX, tr_RowY, tr_RowZ );
    *pBL = MakeMat33( bl_RowX, bl_RowY, bl_RowZ );
    *pBR = MakeMat33( br_RowX, br_RowY, br_RowZ );
}

/// Make Mat66 from four Mat33's
NOALIAS __forceinline const Mat66 MakeMat66( const Mat33 MATREF tl, const Mat33 MATREF tr,
                                             const Mat33 MATREF bl, const Mat33 MATREF br )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    Mat66 result;
    result.m_Row[0] = MakeVector6( RowX( tl ), RowX( tr ) );
    result.m_Row[1] = MakeVector6( RowY( tl ), RowY( tr ) );
    result.m_Row[2] = MakeVector6( RowZ( tl ), RowZ( tr ) );
    result.m_Row[3] = MakeVector6( RowX( bl ), RowX( br ) );
    result.m_Row[4] = MakeVector6( RowY( bl ), RowY( br ) );
    result.m_Row[5] = MakeVector6( RowZ( bl ), RowZ( br ) );
    return result;
}


/// Mat66 Split into top-left Mat55, top-right column Vector5, bottom-left row Vector5 & bottom-right VF32
NOALIAS __forceinline void Split( const Mat66& m,
                                  Mat55*   const __restrict pTL,
                                  Vector5* const __restrict pTR,
                                  Vector5* const __restrict pBL,
                                  VF32*    const __restrict pBR )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    
    const Vector6 row0( Row<0>( m ) );
    const Vector6 row1( Row<1>( m ) );
    const Vector6 row2( Row<2>( m ) );
    const Vector6 row3( Row<3>( m ) );
    const Vector6 row4( Row<4>( m ) );
    const Vector6 row5( Row<5>( m ) );
    
    *pTL = MakeMat55( MakeVector5( row0 ),
                      MakeVector5( row1 ),
                      MakeVector5( row2 ),
                      MakeVector5( row3 ),
                      MakeVector5( row4 ) );
    
    *pTR = MakeVector5( Element<5>( row0 ),
                        Element<5>( row1 ),
                        Element<5>( row2 ),
                        Element<5>( row3 ),
                        Element<5>( row4 ) );
    
    *pBL = MakeVector5( row5 );
    
    *pBR = Element<5>( row5 );
}

/// Make Mat66 from top-left Mat55, top-right column Vector5, bottom-left row Vector5 & bottom-right VF32
NOALIAS __forceinline const Mat66 MakeMat66( const Mat55&   tl, const Vector5&   tr,
                                             const Vector5& bl, const VF32 VECREF br )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    Mat66 result;
    result.m_Row[0] = MakeVector6( Row<0>( tl ), Element<0>( tr ) );
    result.m_Row[1] = MakeVector6( Row<1>( tl ), Element<1>( tr ) );
    result.m_Row[2] = MakeVector6( Row<2>( tl ), Element<2>( tr ) );
    result.m_Row[3] = MakeVector6( Row<3>( tl ), Element<3>( tr ) );
    result.m_Row[4] = MakeVector6( Row<4>( tl ), Element<4>( tr ) );
    result.m_Row[5] = MakeVector6( bl, br );
    return result;
}


/// Mat66 Split into top-left row Vector5, top-right VF32, bottom-left Mat55 & bottom-right column Vector5
NOALIAS __forceinline void Split( const Mat66& m55,
                                  Vector5* const __restrict pTL,
                                  VF32*    const __restrict pTR,
                                  Mat55*   const __restrict pBL,
                                  Vector5* const __restrict pBR )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    
    const Vector6 row0( Row<0>( m55 ) );
    const Vector6 row1( Row<1>( m55 ) );
    const Vector6 row2( Row<2>( m55 ) );
    const Vector6 row3( Row<3>( m55 ) );
    const Vector6 row4( Row<4>( m55 ) );
    const Vector6 row5( Row<5>( m55 ) );
    
    *pTL = MakeVector5( row0 );
    
    *pTR = Element<5>( row0 );
    
    *pBL = MakeMat55( MakeVector5( row1 ),
                      MakeVector5( row2 ),
                      MakeVector5( row3 ),
                      MakeVector5( row4 ),
                      MakeVector5( row5 ) );
    
    *pBR = MakeVector5( Element<5>( row1 ),
                        Element<5>( row2 ),
                        Element<5>( row3 ),
                        Element<5>( row4 ),
                        Element<5>( row5 ) );
}

/// Make Mat66 from top-left row Vector5, top-right VF32, bottom-left Mat55 & bottom-right column Vector5
NOALIAS __forceinline const Mat66 MakeMat66( const Vector5& tl, const VF32 VECREF tr,
                                             const Mat55&   bl, const Vector5&   br )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    Mat66 result;
    result.m_Row[0] = MakeVector6( tl, tr );
    result.m_Row[1] = MakeVector6( Row<0>( bl ), Element<0>( br ) );
    result.m_Row[2] = MakeVector6( Row<1>( bl ), Element<1>( br ) );
    result.m_Row[3] = MakeVector6( Row<2>( bl ), Element<2>( br ) );
    result.m_Row[4] = MakeVector6( Row<3>( bl ), Element<3>( br ) );
    result.m_Row[5] = MakeVector6( Row<4>( bl ), Element<4>( br ) );
    return result;
}


/// Make Mat66 from top-left column Vector5, top-right Mat55, bottom-left VF32 & bottom-right row Vector5
NOALIAS __forceinline const Mat66 MakeMat66( const Vector5&   tl, const Mat55&   tr,
                                             const VF32 VECREF bl, const Vector5& br )
{
    /// tl = top left
    /// tr = top right
    /// bl = bottom left
    /// br = bottom right
    Mat66 result;
    result.m_Row[0] = MakeVector6( Element<0>( tl ), Row<0>( tr ) );
    result.m_Row[1] = MakeVector6( Element<1>( tl ), Row<1>( tr ) );
    result.m_Row[2] = MakeVector6( Element<2>( tl ), Row<2>( tr ) );
    result.m_Row[3] = MakeVector6( Element<3>( tl ), Row<3>( tr ) );
    result.m_Row[4] = MakeVector6( Element<4>( tl ), Row<4>( tr ) );
    result.m_Row[5] = MakeVector6( bl, br );
    return result;
}


/// Create a Mat55 sub-matrix from a Mat66 by deleting Row and Col
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const Mat55 DeleteRowCol( const Mat66& src )
{
    const Vector5 row0( MakeVector5( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, C<=4?5:4, 5 >( Row< R<=0?1:0 >( src ) ) ) );
    const Vector5 row1( MakeVector5( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, C<=4?5:4, 5 >( Row< R<=1?2:1 >( src ) ) ) );
    const Vector5 row2( MakeVector5( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, C<=4?5:4, 5 >( Row< R<=2?3:2 >( src ) ) ) );
    const Vector5 row3( MakeVector5( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, C<=4?5:4, 5 >( Row< R<=3?4:3 >( src ) ) ) );
    const Vector5 row4( MakeVector5( Permute< C<=0?1:0, C<=1?2:1, C<=2?3:2, C<=3?4:3, C<=4?5:4, 5 >( Row< R<=4?5:4 >( src ) ) ) );
    return MakeMat55( row0, row1, row2, row3, row4 );
}

#if 0

/// Create a Mat55 sub-matrix from a Mat66 by deleting Row 0 and Col 0
template <> NOALIAS __forceinline const Mat55 DeleteRowCol< 0, 0 >( const Mat66& src )
{
    const Vector5 row0( MakeVector5( Permute<1,2,3,4,5,5>( Row<1>( src ) ) ) );
    const Vector5 row1( MakeVector5( Permute<1,2,3,4,5,5>( Row<2>( src ) ) ) );
    const Vector5 row2( MakeVector5( Permute<1,2,3,4,5,5>( Row<3>( src ) ) ) );
    const Vector5 row3( MakeVector5( Permute<1,2,3,4,5,5>( Row<4>( src ) ) ) );
    const Vector5 row4( MakeVector5( Permute<1,2,3,4,5,5>( Row<5>( src ) ) ) );
    return MakeMat55( row0, row1, row2, row3, row4 );
}

/// Create a Mat55 sub-matrix from a Mat66 by deleting Row 0 and Col 1
template <> NOALIAS __forceinline const Mat55 DeleteRowCol< 0, 1 >( const Mat66& src )
{
    const Vector5 row0( MakeVector5( Permute<0,2,3,4,5,5>( Row<1>( src ) ) ) );
    const Vector5 row1( MakeVector5( Permute<0,2,3,4,5,5>( Row<2>( src ) ) ) );
    const Vector5 row2( MakeVector5( Permute<0,2,3,4,5,5>( Row<3>( src ) ) ) );
    const Vector5 row3( MakeVector5( Permute<0,2,3,4,5,5>( Row<4>( src ) ) ) );
    const Vector5 row4( MakeVector5( Permute<0,2,3,4,5,5>( Row<5>( src ) ) ) );
    return MakeMat55( row0, row1, row2, row3, row4 );
}

/// Create a Mat55 sub-matrix from a Mat66 by deleting Row 0 and Col 2
template <> NOALIAS __forceinline const Mat55 DeleteRowCol< 0, 2 >( const Mat66& src )
{
    const Vector5 row0( MakeVector5( Permute<0,1,3,4,5,5>( Row<1>( src ) ) ) );
    const Vector5 row1( MakeVector5( Permute<0,1,3,4,5,5>( Row<2>( src ) ) ) );
    const Vector5 row2( MakeVector5( Permute<0,1,3,4,5,5>( Row<3>( src ) ) ) );
    const Vector5 row3( MakeVector5( Permute<0,1,3,4,5,5>( Row<4>( src ) ) ) );
    const Vector5 row4( MakeVector5( Permute<0,1,3,4,5,5>( Row<5>( src ) ) ) );
    return MakeMat55( row0, row1, row2, row3, row4 );
}

/// Create a Mat55 sub-matrix from a Mat66 by deleting Row 0 and Col 3
template <> NOALIAS __forceinline const Mat55 DeleteRowCol< 0, 3 >( const Mat66& src )
{
    const Vector5 row0( MakeVector5( Permute<0,1,2,4,5,5>( Row<1>( src ) ) ) );
    const Vector5 row1( MakeVector5( Permute<0,1,2,4,5,5>( Row<2>( src ) ) ) );
    const Vector5 row2( MakeVector5( Permute<0,1,2,4,5,5>( Row<3>( src ) ) ) );
    const Vector5 row3( MakeVector5( Permute<0,1,2,4,5,5>( Row<4>( src ) ) ) );
    const Vector5 row4( MakeVector5( Permute<0,1,2,4,5,5>( Row<5>( src ) ) ) );
    return MakeMat55( row0, row1, row2, row3, row4 );
}

/// Create a Mat55 sub-matrix from a Mat66 by deleting Row 0 and Col 4
template <> NOALIAS __forceinline const Mat55 DeleteRowCol< 0, 4 >( const Mat66& src )
{
    const Vector5 row0( MakeVector5( Permute<0,1,2,3,5,5>( Row<1>( src ) ) ) );
    const Vector5 row1( MakeVector5( Permute<0,1,2,3,5,5>( Row<2>( src ) ) ) );
    const Vector5 row2( MakeVector5( Permute<0,1,2,3,5,5>( Row<3>( src ) ) ) );
    const Vector5 row3( MakeVector5( Permute<0,1,2,3,5,5>( Row<4>( src ) ) ) );
    const Vector5 row4( MakeVector5( Permute<0,1,2,3,5,5>( Row<5>( src ) ) ) );
    return MakeMat55( row0, row1, row2, row3, row4 );
}

/// Create a Mat55 sub-matrix from a Mat66 by deleting Row 0 and Col 5
template <> NOALIAS __forceinline const Mat55 DeleteRowCol< 0, 5 >( const Mat66& src )
{
    const Vector5 row0( MakeVector5( Permute<0,1,2,3,4,4>( Row<1>( src ) ) ) );
    const Vector5 row1( MakeVector5( Permute<0,1,2,3,4,4>( Row<2>( src ) ) ) );
    const Vector5 row2( MakeVector5( Permute<0,1,2,3,4,4>( Row<3>( src ) ) ) );
    const Vector5 row3( MakeVector5( Permute<0,1,2,3,4,4>( Row<4>( src ) ) ) );
    const Vector5 row4( MakeVector5( Permute<0,1,2,3,4,4>( Row<5>( src ) ) ) );
    return MakeMat55( row0, row1, row2, row3, row4 );
}

#endif


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


