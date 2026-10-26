#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_MAT62_INL
#define SOFT_MATHS_HVEC_NEW_MAT62_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_MAT62_INL
#define MATHS_HVEC_NEW_MAT62_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Mat62
/// @{

/// Make Mat62 from 6 Vector2 rows
NOALIAS __forceinline const Mat62 MakeMat62( const Vector2 PCREF row0, const Vector2 PCREF row1, const Vector2 PCREF row2, const Vector2 PCREF row3,
                                             const Vector2 PCREF row4, const Vector2 PCREF row5 )
{
    Mat62 result;
    result.m_Row[0] = row0;
    result.m_Row[1] = row1;
    result.m_Row[2] = row2;
    result.m_Row[3] = row3;
    result.m_Row[4] = row4;
    result.m_Row[5] = row5;
    return result;
}


NOALIAS __forceinline const Mat62 Const< Mat62 >::Zero() ///< Constant Mat62 with all elements zeroed
{
    Mat62 result;
    result.m_Row[0] =
    result.m_Row[1] =
    result.m_Row[2] =
    result.m_Row[3] =
    result.m_Row[4] =
    result.m_Row[5] = Const< Vector2 >::Zero();
    return result;
}


/// Mat62 Accessor: Get element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline const VF32 Element( const Mat62& src )
{
    BIZ_STATIC_ASSERT( ( R < Mat62::m_Rows ) && ( C < Mat62::m_Cols ) );
    return Element<C>( src.m_Row[R] );
}

/// Mat62 Accessor: Set element
template < unsigned int R, unsigned int C >
NOALIAS __forceinline void SetElement( Mat62* const __restrict pDst, const VF32 VECREF e )
{
    BIZ_STATIC_ASSERT( ( R < Mat62::m_Rows ) && ( C < Mat62::m_Cols ) );
    SetElement<C>( &pDst->m_Row[R], e );
}


/// Mat62 Accessor: Get row
template < unsigned int R >
NOALIAS __forceinline const Vector2 Row( const Mat62& src )
{
    BIZ_STATIC_ASSERT( R < Mat62::m_Rows );
    return src.m_Row[R];
}

/// Mat62 Accessor: Set row
template < unsigned int R >
NOALIAS __forceinline void SetRow( Mat62* const __restrict pDst, const Vector2 PCREF row )
{
    BIZ_STATIC_ASSERT( R < Mat62::m_Rows );
    pDst->m_Row[R] = row;
}


/// Mat62 Accessor: Get column
template < unsigned int C >
NOALIAS __forceinline const Vector6 Col( const Mat62& src )
{
    BIZ_STATIC_ASSERT( C < Mat62::m_Cols );
    return MakeVector6( Element<(int)0,(int)C>( src ),
                        Element<(int)1,(int)C>( src ),
                        Element<(int)2,(int)C>( src ),
                        Element<(int)3,(int)C>( src ),
                        Element<(int)4,(int)C>( src ),
                        Element<(int)5,(int)C>( src ) );
}

/// Mat62 Accessor: Set column
template < unsigned int C >
NOALIAS __forceinline void SetCol( Mat62* const __restrict pDst, const Vector6 PCREF col )
{
    BIZ_STATIC_ASSERT( C < Mat62::m_Cols );
    SetElement<C>( &pDst->m_Row[0], Element<0>( col ) );
    SetElement<C>( &pDst->m_Row[1], Element<1>( col ) );
    SetElement<C>( &pDst->m_Row[2], Element<2>( col ) );
    SetElement<C>( &pDst->m_Row[3], Element<3>( col ) );
    SetElement<C>( &pDst->m_Row[4], Element<4>( col ) );
    SetElement<C>( &pDst->m_Row[5], Element<5>( col ) );
}


/// Mat62 Transpose
NOALIAS __forceinline const Mat26 Transpose( const Mat62& src )
{
    Mat26 result;
    
    #if defined( _SPU_ )
    
    // transpose top-left 4x2
    const vector float tl_x0_y0( src.m_Row[0].m_V.m_V );                           // [ x0 y0 ]
    const vector float tl_x1_y1( src.m_Row[1].m_V.m_V );                           // [ x1 y1 ]
    const vector float tl_x2_y2( src.m_Row[2].m_V.m_V );                           // [ x2 y2 ]
    const vector float tl_x3_y3( src.m_Row[3].m_V.m_V );                           // [ x3 y3 ]
    const vector float tl_x0_x2_y0_y2( spu_merge_xy( tl_x0_y0, tl_x2_y2 ) );       // [ x0 x2 y0 y2 ]
    const vector float tl_x1_x3_y1_y3( spu_merge_xy( tl_x1_y1, tl_x3_y3 ) );       // [ x1 x3 y1 y3 ]
    result.m_Row[0].m_V.m_V0.m_V = spu_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = spu_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    // transpose bottom-left 2x2 to top-right 2x2
    const vector float bl_x0_y0( src.m_Row[4].m_V.m_V );                 // [ x0 y0 ]
    const vector float bl_x1_y1( src.m_Row[5].m_V.m_V );                 // [ x1 y1 ]
    const vector float bl_y0( spu_rotate_left_qw_words<1>( bl_x0_y0 ) ); // [ y0 ]
    //const vector float bl_y1( spu_rotate_left_qw_words<1>( bl_x1_y1 ) ); // [ y1 ]
    //result.m_Row[0].m_V.m_V1.m_V = spu_merge_xy( bl_x0_y0, bl_x1_y1 );   // [ x0 x1 ]
    //result.m_Row[1].m_V.m_V1.m_V = spu_merge_xy( bl_y0, bl_y1 );         // [ y0 y1 ]
    const vector float bl_x0_x1_y0_y1( spu_merge_xy( bl_x0_y0, bl_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    const vector float bl_y0_y1_x0_x1( spu_rotate_left_qw_words<2>( bl_x0_x1_y0_y1 ) ); // [ y0 y1 x0 x1 ]
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                      // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                      // [ y0 y1 ]
    
    #elif defined( _PPU_ )                                                                                                
    
    // transpose top-left 4x2
    const vector float tl_x0_y0( src.m_Row[0].m_V.m_V );                           // [ x0 y0 z0 ]
    const vector float tl_x1_y1( src.m_Row[1].m_V.m_V );                           // [ x1 y1 z1 ]
    const vector float tl_x2_y2( src.m_Row[2].m_V.m_V );                           // [ x2 y2 z2 ]
    const vector float tl_x3_y3( src.m_Row[3].m_V.m_V );                           // [ x3 y3 z3 ]
    const vector float tl_x0_x2_y0_y2( vec_merge_xy( tl_x0_y0, tl_x2_y2 ) );       // [ x0 x2 y0 y2 ]
    const vector float tl_x1_x3_y1_y3( vec_merge_xy( tl_x1_y1, tl_x3_y3 ) );       // [ x1 x3 y1 y3 ]
    result.m_Row[0].m_V.m_V0.m_V = vec_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = vec_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    // transpose bottom-left 2x2 to top-right 2x2
    const vector float bl_x0_y0( src.m_Row[4].m_V.m_V );                 // [ x0 y0 ]
    const vector float bl_x1_y1( src.m_Row[5].m_V.m_V );                 // [ x1 y1 ]
    const vector float bl_y0( vec_rotate_left_qw_words<1>( bl_x0_y0 ) ); // [ y0 ]
    //const vector float bl_y1( vec_rotate_left_qw_words<1>( bl_x1_y1 ) ); // [ y1 ]
    //result.m_Row[0].m_V.m_V1.m_V = vec_merge_xy( bl_x0_y0, bl_x1_y1 );   // [ x0 x1 ]
    //result.m_Row[1].m_V.m_V1.m_V = vec_merge_xy( bl_y0, bl_y1 );         // [ y0 y1 ]
    const vector float bl_x0_x1_y0_y1( vec_merge_xy( bl_x0_y0, bl_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    const vector float bl_y0_y1_x0_x1( vec_rotate_left_qw_words<2>( bl_x0_x1_y0_y1 ) ); // [ y0 y1 x0 x1 ]
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                      // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                      // [ y0 y1 ]
    
    #elif defined( _VMX128_ )                                                                                             
    
    // transpose top-left 4x2
    const __vector4 tl_x0_y0( src.m_Row[0].m_V.m_V );                              // [ x0 y0 ]
    const __vector4 tl_x1_y1( src.m_Row[1].m_V.m_V );                              // [ x1 y1 ]
    const __vector4 tl_x2_y2( src.m_Row[2].m_V.m_V );                              // [ x2 y2 ]
    const __vector4 tl_x3_y3( src.m_Row[3].m_V.m_V );                              // [ x3 y3 ]
    const __vector4 tl_x0_x2_y0_y2( vmx_merge_xy( tl_x0_y0, tl_x2_y2 ) );          // [ x0 x2 y0 y2 ]
    const __vector4 tl_x1_x3_y1_y3( vmx_merge_xy( tl_x1_y1, tl_x3_y3 ) );          // [ x1 x3 y1 y3 ]
    result.m_Row[0].m_V.m_V0.m_V = vmx_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = vmx_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    // transpose bottom-left 2x2 to top-right 2x2
    const __vector4 bl_x0_y0( src.m_Row[4].m_V.m_V );                  // [ x0 y0 ]
    const __vector4 bl_x1_y1( src.m_Row[5].m_V.m_V );                  // [ x1 y1 ]
    //const __vector4 bl_y0( vmx_rotate_left_qw_words<1>( bl_x0_y0 ) );  // [ y0 ]
    //const __vector4 bl_y1( vmx_rotate_left_qw_words<1>( bl_x1_y1 ) );  // [ y1 ]
    //result.m_Row[0].m_V.m_V1.m_V = vmx_merge_xy( bl_x0_y0, bl_x1_y1 ); // [ x0 x1 ]
    //result.m_Row[1].m_V.m_V1.m_V = vmx_merge_xy( bl_y0, bl_y1 );       // [ y0 y1 ]
    const __vector4 bl_x0_x1_y0_y1( vmx_merge_xy( bl_x0_y0, bl_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    const __vector4 bl_y0_y1_x0_x1( vmx_rotate_left_qw_words<2>( bl_x0_x1_y0_y1 ) ); // [ y0 y1 x0 x1 ]
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                   // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                   // [ y0 y1 ]
    
    #elif defined( _SSE_ )                                                                                                
    
    // transpose top-left 4x2
    const __m128 tl_x0_y0( src.m_Row[0].m_V.m_V );                                 // [ x0 y0 z0 ]
    const __m128 tl_x1_y1( src.m_Row[1].m_V.m_V );                                 // [ x1 y1 z1 ]
    const __m128 tl_x2_y2( src.m_Row[2].m_V.m_V );                                 // [ x2 y2 z2 ]
    const __m128 tl_x3_y3( src.m_Row[3].m_V.m_V );                                 // [ x3 y3 z3 ]
    const __m128 tl_x0_x2_y0_y2( sse_merge_xy( tl_x0_y0, tl_x2_y2 ) );             // [ x0 x2 y0 y2 ]
    const __m128 tl_x1_x3_y1_y3( sse_merge_xy( tl_x1_y1, tl_x3_y3 ) );             // [ x1 x3 y1 y3 ]
    result.m_Row[0].m_V.m_V0.m_V = sse_merge_xy( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ x0 x1 x2 x3 ]
    result.m_Row[1].m_V.m_V0.m_V = sse_merge_zw( tl_x0_x2_y0_y2, tl_x1_x3_y1_y3 ); // [ y0 y1 y2 y3 ]
    
    // transpose bottom-left 2x2 to top-right 2x2
    const __m128 bl_x0_y0( src.m_Row[4].m_V.m_V );                     // [ x0 y0 z0 ]
    const __m128 bl_x1_y1( src.m_Row[5].m_V.m_V );                     // [ x1 y1 z1 ]
    //const __m128 bl_y0( sse_rotate_left_qw_words<1>( bl_x0_y0 ) );     // [ y0 ]
    //const __m128 bl_y1( sse_rotate_left_qw_words<1>( bl_x1_y1 ) );     // [ y1 ]
    //result.m_Row[0].m_V.m_V1.m_V = sse_merge_xy( bl_x0_y0, bl_x1_y1 ); // [ x0 x1 ]
    //result.m_Row[1].m_V.m_V1.m_V = sse_merge_xy( bl_y0, bl_y1 );       // [ y0 y1 ]
    const __m128 bl_x0_x1_y0_y1( sse_merge_xy( bl_x0_y0, bl_x1_y1 ) );            // [ x0 x1 y0 y1 ]
    const __m128 bl_y0_y1_x0_x1( sse_rotate_left_qw_words<2>( bl_x0_x1_y0_y1 ) ); // [ y0 y1 x0 x1 ]
    result.m_Row[0].m_V.m_V1.m_V = bl_x0_x1_y0_y1;                                // [ x0 x1 ]
    result.m_Row[1].m_V.m_V1.m_V = bl_y0_y1_x0_x1;                                // [ y0 y1 ]
    
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


/// Mat62 Negation Operator
NOALIAS __forceinline const Mat62 operator - ( const Mat62& a )
{
    Mat62 result;
    result.m_Row[0] = -a.m_Row[0];
    result.m_Row[1] = -a.m_Row[1];
    result.m_Row[2] = -a.m_Row[2];
    result.m_Row[3] = -a.m_Row[3];
    result.m_Row[4] = -a.m_Row[4];
    result.m_Row[5] = -a.m_Row[5];
    return result;
}

/// Mat62 Addition Operator
NOALIAS __forceinline const Mat62 operator + ( const Mat62& a, const Mat62& b )
{
    Mat62 result;
    result.m_Row[0] = a.m_Row[0] + b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] + b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] + b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] + b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] + b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] + b.m_Row[5];
    return result;
}

/// Mat62 Subtraction Operator
NOALIAS __forceinline const Mat62 operator - ( const Mat62& a, const Mat62& b )
{
    Mat62 result;
    result.m_Row[0] = a.m_Row[0] - b.m_Row[0];
    result.m_Row[1] = a.m_Row[1] - b.m_Row[1];
    result.m_Row[2] = a.m_Row[2] - b.m_Row[2];
    result.m_Row[3] = a.m_Row[3] - b.m_Row[3];
    result.m_Row[4] = a.m_Row[4] - b.m_Row[4];
    result.m_Row[5] = a.m_Row[5] - b.m_Row[5];
    return result;
}

/// Mat62 Addition Update Operator
NOALIAS __forceinline void operator += ( Mat62& a, const Mat62& b )
{
    a.m_Row[0] += b.m_Row[0];
    a.m_Row[1] += b.m_Row[1];
    a.m_Row[2] += b.m_Row[2];
    a.m_Row[3] += b.m_Row[3];
    a.m_Row[4] += b.m_Row[4];
    a.m_Row[5] += b.m_Row[5];
}

/// Mat62 Subtraction Update Operator
NOALIAS __forceinline void operator -= ( Mat62& a, const Mat62& b )
{
    a.m_Row[0] -= b.m_Row[0];
    a.m_Row[1] -= b.m_Row[1];
    a.m_Row[2] -= b.m_Row[2];
    a.m_Row[3] -= b.m_Row[3];
    a.m_Row[4] -= b.m_Row[4];
    a.m_Row[5] -= b.m_Row[5];
}


/// Mat62 * Vector2 Multiplication Operator
NOALIAS __forceinline const Vector6 operator * ( const Mat62& a, const Vector2& b )
{
    #if defined( _VMX128_ )
    
    return MakeVector6( Dot( Row<0>( a ), b ),
                        Dot( Row<1>( a ), b ),
                        Dot( Row<2>( a ), b ),
                        Dot( Row<3>( a ), b ),
                        Dot( Row<4>( a ), b ),
                        Dot( Row<5>( a ), b ) );
    
    #else
    
    const Mat26 at( Transpose( a ) );
    
	return MulAdd( Row<0>( at ), Element<0>( b ), Row<1>( at ) * Element<1>( b ) );
    
    #endif
}

/// Vector6 * Mat62 Multiplication Operator
NOALIAS __forceinline const Vector2 operator * ( const Vector6& a, const Mat62& b )
{
    #if defined( _VMX128_ )
    
    const Mat26 bt( Transpose( b ) );
    
	return MakeVector2( Dot( a, Row<0>( bt ) ), 
						Dot( a, Row<1>( bt ) ) );
    
    #else
    
	return MulAdd( Element<0>( a ), Row<0>( b ), Element<1>( a ) * Row<1>( b ) ) +
	       MulAdd( Element<2>( a ), Row<2>( b ), Element<3>( a ) * Row<3>( b ) ) +
	       MulAdd( Element<4>( a ), Row<4>( b ), Element<5>( a ) * Row<5>( b ) );
    
    #endif
}


/// Mat62 * Mat26 Diagonal Multiply: Returns the diagonal of the resulting 6x6 matrix
NOALIAS __forceinline const Vector6 DiagonalMultiply( const Mat62& a, const Mat26& b )
{
    #if defined( _VMX128_ )
    
    const Mat62 bt( Transpose( b ) );
    
    return MakeVector6( Dot( Row<0>( a ), Row<0>( bt ) ),
                        Dot( Row<1>( a ), Row<1>( bt ) ),
                        Dot( Row<2>( a ), Row<2>( bt ) ),
                        Dot( Row<3>( a ), Row<3>( bt ) ),
                        Dot( Row<4>( a ), Row<4>( bt ) ),
                        Dot( Row<5>( a ), Row<5>( bt ) ) );
    
    #else
    
    const Mat26 at( Transpose( a ) );
    
    return MulAdd( Row<0>( at ), Row<0>( b ), Row<1>( at ) * Row<1>( b ) );
    
    #endif
}

/// Mat26 * Mat62 Diagonal Multiply: Returns the diagonal of the resulting 2x2 matrix
NOALIAS __forceinline const Vector2 DiagonalMultiply( const Mat26& a, const Mat62& b )
{
    #if defined( _VMX128_ )
    
    const Mat26 bt( Transpose( b ) );
    
    return MakeVector2( Dot( Row<0>( a ), Row<0>( bt ) ),
                        Dot( Row<1>( a ), Row<1>( bt ) ) );
    
    #else
    
    const Mat62 at( Transpose( a ) );
    
    return MulAdd( Row<0>( at ), Row<0>( b ), Row<1>( at ) * Row<1>( b ) ) +
           MulAdd( Row<2>( at ), Row<2>( b ), Row<3>( at ) * Row<3>( b ) ) +
           MulAdd( Row<4>( at ), Row<4>( b ), Row<5>( at ) * Row<5>( b ) );
    
    #endif
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


