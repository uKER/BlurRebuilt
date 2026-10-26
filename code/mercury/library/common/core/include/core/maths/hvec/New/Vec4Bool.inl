#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC4BOOL_INL
#define SOFT_MATHS_HVEC_NEW_VEC4BOOL_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC4BOOL_INL
#define MATHS_HVEC_NEW_VEC4BOOL_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean
/// @{

/// Make Vec4Bool from a set of 4 template bools ( compile time constants )
template < bool x, bool y, bool z, bool w >
NOALIAS __forceinline const Vec4Bool MakeVec4Bool()
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::Bool<x,y,z,w>();
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool int >::Bool<x,y,z,w>();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool<x,y,z,w>();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::Bool<x,y,z,w>();
    #else
    result.m_V[VX] = x;
    result.m_V[VY] = y;
    result.m_V[VZ] = z;
    result.m_V[VW] = w;
    #endif
    return result;
}

/// Make Vec4Bool from a set of 4 bools ( runtime variables )
NOALIAS __forceinline const Vec4Bool MakeVec4Bool( const bool x, const bool y, const bool z, const bool w )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned int >::Bool( x, y, z, w );
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool int >::Bool( x, y, z, w );
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool( x, y, z, w );
    #elif defined( _SSE_ )
    result.m_V = Const< __m128 >::Bool( x, y, z, w );
    #else
    result.m_V[VX] = x;
    result.m_V[VY] = y;
    result.m_V[VZ] = z;
    result.m_V[VW] = w;
    #endif
    return result;
}

/// Make Vec4Bool from a set of 4 VBool's
NOALIAS __forceinline const Vec4Bool MakeVec4Bool( const VBool PCREF x, const VBool PCREF y, const VBool PCREF z, const VBool PCREF w )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_insert( spu_extract( x.m_V, VX ),
                 spu_insert( spu_extract( y.m_V, VY ),
                 spu_insert( spu_extract( z.m_V, VZ ), w.m_V, VZ ), VY ), VX );
    #elif defined( _PPU_ )
    result.m_V = vec_merge_xy( vec_merge_xy( x.m_V, z.m_V ),
                               vec_merge_xy( y.m_V, w.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_merge_xy( vmx_merge_xy( x.m_V, z.m_V ),
                               vmx_merge_xy( y.m_V, w.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_unpacklo_ps( _mm_unpacklo_ps( x.m_V, z.m_V ),
                                  _mm_unpacklo_ps( y.m_V, w.m_V ) );
    #else
    result.m_V[VX] = x.m_V;
    result.m_V[VY] = y.m_V;
    result.m_V[VZ] = z.m_V;
    result.m_V[VW] = w.m_V;
    #endif
    return result;
}


/// Vec4Bool accessor: Get X element
NOALIAS __forceinline const VBool X( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<VX>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, VX );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( v.m_V, VX );
    #elif defined( _SSE_ )
    //result.m_V = _mm_shuffle_ps( v.m_V, v.m_V, _MM_SHUFFLE( VX, VX, VX, VX );
    result.m_V = sse_splat<VX>( v.m_V );
    #else
    result.m_V = v.m_V[VX];
    #endif
    return result;
}

/// Vec4Bool accessor: Get Y element
NOALIAS __forceinline const VBool Y( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<VY>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, VY );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( v.m_V, VY );
    #elif defined( _SSE_ )
    //result.m_V = _mm_shuffle_ps( v.m_V, v.m_V, _MM_SHUFFLE( VY, VY, VY, VY );
    result.m_V = sse_splat<VY>( v.m_V );
    #else
    result.m_V = v.m_V[VY];
    #endif
    return result;
}

/// Vec4Bool accessor: Get Z element
NOALIAS __forceinline const VBool Z( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<VZ>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, VZ );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( v.m_V, VZ );
    #elif defined( _SSE_ )
    //result.m_V = _mm_shuffle_ps( v.m_V, v.m_V, _MM_SHUFFLE( VZ, VZ, VZ, VZ );
    result.m_V = sse_splat<VZ>( v.m_V );
    #else
    result.m_V = v.m_V[VZ];
    #endif
    return result;
}

/// Vec4Bool accessor: Get W element
NOALIAS __forceinline const VBool W( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<VW>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, VW );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( v.m_V, VW );
    #elif defined( _SSE_ )
    //result.m_V = _mm_shuffle_ps( v.m_V, v.m_V, _MM_SHUFFLE( VW, VW, VW, VW );
    result.m_V = sse_splat<VW>( v.m_V );
    #else
    result.m_V = v.m_V[VW];
    #endif
    return result;
}


/// Vec4Bool accessor: Set X element
NOALIAS __forceinline void SetX( Vec4Bool* const __restrict pDst, const VBool PCREF x )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_select<1,0,0,0>( pDst->m_V, x.m_V );
    #elif defined( _PPU_ )
    pDst->m_V = vec_select<1,0,0,0>( pDst->m_V, x.m_V );
    #elif defined( _VMX128_ )
    pDst->m_V = __vrlimi( pDst->m_V, x.m_V, VMX_MASK_INSERT_X, 0 );
    #elif defined( _SSE_ )
    pDst->m_V = sse_select<1,0,0,0>( pDst->m_V, x.m_V );
    #else
    pDst->m_V[VX] = x.m_V;
    #endif
}

/// Vec4Bool accessor: Set Y element
NOALIAS __forceinline void SetY( Vec4Bool* const __restrict pDst, const VBool PCREF y )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_select<0,1,0,0>( pDst->m_V, y.m_V );
    #elif defined( _PPU_ )
    pDst->m_V = vec_select<0,1,0,0>( pDst->m_V, y.m_V );
    #elif defined( _VMX128_ )
    pDst->m_V = __vrlimi( pDst->m_V, y.m_V, VMX_MASK_INSERT_Y, 0 );
    #elif defined( _SSE_ )
    pDst->m_V = sse_select<0,1,0,0>( pDst->m_V, y.m_V );
    #else
    pDst->m_V[VY] = y.m_V;
    #endif
}

/// Vec4Bool accessor: Set Z element
NOALIAS __forceinline void SetZ( Vec4Bool* const __restrict pDst, const VBool PCREF z )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_select<0,0,1,0>( pDst->m_V, z.m_V );
    #elif defined( _PPU_ )
    pDst->m_V = vec_select<0,0,1,0>( pDst->m_V, z.m_V );
    #elif defined( _VMX128_ )
    pDst->m_V = __vrlimi( pDst->m_V, z.m_V, VMX_MASK_INSERT_Z, 0 );
    #elif defined( _SSE_ )
    pDst->m_V = sse_select<0,0,1,0>( pDst->m_V, z.m_V );
    #else
    pDst->m_V[VZ] = z.m_V;
    #endif
}

/// Vec4Bool accessor: Set W element
NOALIAS __forceinline void SetW( Vec4Bool* const __restrict pDst, const VBool PCREF w )
{
    #if defined( _SPU_ )
    pDst->m_V = spu_select<0,0,0,1>( pDst->m_V, w.m_V );
    #elif defined( _PPU_ )
    pDst->m_V = vec_select<0,0,0,1>( pDst->m_V, w.m_V );
    #elif defined( _VMX128_ )
    pDst->m_V = __vrlimi( pDst->m_V, w.m_V, VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    pDst->m_V = sse_select<0,0,0,1>( pDst->m_V, w.m_V );
    #else
    pDst->m_V[VW] = w.m_V;
    #endif
}



/// Vec4Bool negation operator
NOALIAS __forceinline const Vec4Bool operator ~ ( const Vec4Bool PCREF x )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_not( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_nor( x.m_V, x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vnor( x.m_V, x.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_ps( x.m_V, Const< __m128 >::Mask<1,1,1,1>() );
    #else
    result.m_V[VX] = !x.m_V[VX];
    result.m_V[VY] = !x.m_V[VY];
    result.m_V[VZ] = !x.m_V[VZ];
    result.m_V[VW] = !x.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool logical and operator
NOALIAS __forceinline const Vec4Bool operator & ( const Vec4Bool PCREF x, const Vec4Bool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] && y.m_V[VX];
    result.m_V[VY] = x.m_V[VY] && y.m_V[VY];
    result.m_V[VZ] = x.m_V[VZ] && y.m_V[VZ];
    result.m_V[VW] = x.m_V[VW] && y.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool logical or operator
NOALIAS __forceinline const Vec4Bool operator | ( const Vec4Bool PCREF x, const Vec4Bool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] || y.m_V[VX];
    result.m_V[VY] = x.m_V[VY] || y.m_V[VY];
    result.m_V[VZ] = x.m_V[VZ] || y.m_V[VZ];
    result.m_V[VW] = x.m_V[VW] || y.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool logical xor operator
NOALIAS __forceinline const Vec4Bool operator ^ ( const Vec4Bool PCREF x, const Vec4Bool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] != y.m_V[VX];
    result.m_V[VY] = x.m_V[VY] != y.m_V[VY];
    result.m_V[VZ] = x.m_V[VZ] != y.m_V[VZ];
    result.m_V[VW] = x.m_V[VW] != y.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool equal operator
NOALIAS __forceinline const VBool operator == ( const Vec4Bool PCREF x, const Vec4Bool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    //const vector unsigned int eq( spu_cmpeq( x.m_V, y.m_V ) );
    //result.m_V = spu_and( spu_and( spu_splat<VX>( eq ), spu_splat<VY>( eq ) ),
    //                      spu_and( spu_splat<VZ>( eq ), spu_splat<VW>( eq ) ) );
    result.m_V = spu_horizontal_and( spu_cmpeq( x.m_V, y.m_V ) );
    #elif defined( _PPU_ )
    //const vector bool int eq( vec_cmpeq( x.m_V, y.m_V ) );
    //result.m_V = vec_and( vec_and( vec_splat( eq, VX ), vec_splat( eq, VY ) ),
    //                      vec_and( vec_splat( eq, VZ ), vec_splat( eq, VW ) ) );
    result.m_V = vec_horizontal_and( vec_cmpeq( (vector unsigned int)x.m_V, (vector unsigned int)y.m_V ) );
    #elif defined( _VMX128_ )
    const __vector4 eq( __vcmpequw( x.m_V, y.m_V ) );
    //result.m_V = __vand( __vand( __vspltw( eq, VX ), __vspltw( eq, VY ) ),
    //                     __vand( __vspltw( eq, VZ ), __vspltw( eq, VW ) ) );
    result.m_V = vmx_horizontal_and_int32( eq );
    #elif defined( _SSE_ )
    const __m128 eq( (__m128&)_mm_cmpeq_epi32( (__m128i&)x.m_V, (__m128i&)y.m_V ) );
    //result.m_V = _mm_and_ps( _mm_and_ps( sse_splat<VX>( eq ), sse_splat<VY>( eq ) ),
    //                         _mm_and_ps( sse_splat<VZ>( eq ), sse_splat<VW>( eq ) ) );
    result.m_V = sse_horizontal_and_int32( eq );
    #else
    result.m_V = x.m_V[VX] == y.m_V[VX] &&
                 x.m_V[VY] == y.m_V[VY] &&
                 x.m_V[VZ] == y.m_V[VZ] &&
                 x.m_V[VW] == y.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool not-equal operator
NOALIAS __forceinline const VBool operator != ( const Vec4Bool PCREF x, const Vec4Bool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    //const vector unsigned int ne( spu_xor( x.m_V, y.m_V ) );
    //result.m_V = spu_or( spu_or( spu_splat<VX>( ne ), spu_splat<VY>( ne ) ),
    //                     spu_or( spu_splat<VZ>( ne ), spu_splat<VW>( ne ) ) );
    result.m_V = spu_horizontal_or( spu_xor( x.m_V, y.m_V ) );
    #elif defined( _PPU_ )
    //const vector bool int ne( vec_xor( x.m_V, y.m_V ) );
    //result.m_V = vec_or( vec_or( vec_splat( ne, VX ), vec_splat( ne, VY ) ),
    //                     vec_or( vec_splat( ne, VZ ), vec_splat( ne, VW ) ) ); 
    result.m_V = vec_horizontal_or( vec_xor( x.m_V, y.m_V ) );
    #elif defined( _VMX128_ )
    const __vector4 ne( __vxor( x.m_V, y.m_V ) );
    //result.m_V = __vor( __vor( __vspltw( ne, VX ), __vspltw( ne, VY ) ),
    //                    __vor( __vspltw( ne, VZ ), __vspltw( ne, VW ) ) );
    result.m_V = vmx_horizontal_or_int32( ne );
    #elif defined( _SSE_ )
    const __m128 ne( _mm_xor_ps( x.m_V, y.m_V ) );
    //result.m_V = _mm_or_ps( _mm_or_ps( sse_splat<VX>( ne ), sse_splat<VY>( ne ) ),
    //                        _mm_or_ps( sse_splat<VZ>( ne ), sse_splat<VW>( ne ) ) );
    result.m_V = sse_horizontal_or_int32( ne );
    #else
    result.m_V = x.m_V[VX] != y.m_V[VX] ||
                 x.m_V[VY] != y.m_V[VY] ||
                 x.m_V[VZ] != y.m_V[VZ] ||
                 x.m_V[VW] != y.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec4Bool& x, const Vec4Bool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_and_ps( x.m_V, y.m_V );
    #else
    x.m_V[VX] = x.m_V[VX] && y.m_V[VX];
    x.m_V[VY] = x.m_V[VY] && y.m_V[VY];
    x.m_V[VZ] = x.m_V[VZ] && y.m_V[VZ];
    x.m_V[VW] = x.m_V[VW] && y.m_V[VW];
    #endif
}

/// Vec4Bool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec4Bool& x, const Vec4Bool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_or_ps( x.m_V, y.m_V );
    #else
    x.m_V[VX] = x.m_V[VX] || y.m_V[VX];
    x.m_V[VY] = x.m_V[VY] || y.m_V[VY];
    x.m_V[VZ] = x.m_V[VZ] || y.m_V[VZ];
    x.m_V[VW] = x.m_V[VW] || y.m_V[VW];
    #endif
}
    



/// Vec4Bool & VBool logical and operator
NOALIAS __forceinline const Vec4Bool operator & ( const Vec4Bool PCREF x, const VBool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] && y.m_V;
    result.m_V[VY] = x.m_V[VY] && y.m_V;
    result.m_V[VZ] = x.m_V[VZ] && y.m_V;
    result.m_V[VW] = x.m_V[VW] && y.m_V;
    #endif
    return result;
}

/// VBool & Vec4Bool logical and operator
NOALIAS __forceinline const Vec4Bool operator & ( const VBool PCREF x, const Vec4Bool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V && y.m_V[VX];
    result.m_V[VY] = x.m_V && y.m_V[VY];
    result.m_V[VZ] = x.m_V && y.m_V[VZ];
    result.m_V[VW] = x.m_V && y.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool & VBool logical or operator
NOALIAS __forceinline const Vec4Bool operator | ( const Vec4Bool PCREF x, const VBool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] || y.m_V;
    result.m_V[VY] = x.m_V[VY] || y.m_V;
    result.m_V[VZ] = x.m_V[VZ] || y.m_V;
    result.m_V[VW] = x.m_V[VW] || y.m_V;
    #endif
    return result;
}

/// VBool & Vec4Bool logical or operator
NOALIAS __forceinline const Vec4Bool operator | ( const VBool PCREF x, const Vec4Bool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V || y.m_V[VX];
    result.m_V[VY] = x.m_V || y.m_V[VY];
    result.m_V[VZ] = x.m_V || y.m_V[VZ];
    result.m_V[VW] = x.m_V || y.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool & VBool logical xor operator
NOALIAS __forceinline const Vec4Bool operator ^ ( const Vec4Bool PCREF x, const VBool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V[VX] != y.m_V;
    result.m_V[VY] = x.m_V[VY] != y.m_V;
    result.m_V[VZ] = x.m_V[VZ] != y.m_V;
    result.m_V[VW] = x.m_V[VW] != y.m_V;
    #endif
    return result;
}

/// VBool & Vec4Bool logical xor operator
NOALIAS __forceinline const Vec4Bool operator ^ ( const VBool PCREF x, const Vec4Bool PCREF y )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_ps( x.m_V, y.m_V );
    #else
    result.m_V[VX] = x.m_V != y.m_V[VX];
    result.m_V[VY] = x.m_V != y.m_V[VY];
    result.m_V[VZ] = x.m_V != y.m_V[VZ];
    result.m_V[VW] = x.m_V != y.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool & VBool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec4Bool& x, const VBool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_and_ps( x.m_V, y.m_V );
    #else
    x.m_V[VX] = x.m_V[VX] && y.m_V;
    x.m_V[VY] = x.m_V[VY] && y.m_V;
    x.m_V[VZ] = x.m_V[VZ] && y.m_V;
    x.m_V[VW] = x.m_V[VW] && y.m_V;
    #endif
}

/// Vec4Bool & VBool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec4Bool& x, const VBool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_or_ps( x.m_V, y.m_V );
    #else
    x.m_V[VX] = x.m_V[VX] || y.m_V;
    x.m_V[VY] = x.m_V[VY] || y.m_V;
    x.m_V[VZ] = x.m_V[VZ] || y.m_V;
    x.m_V[VW] = x.m_V[VW] || y.m_V;
    #endif
}


/// Vec4Bool Bitwise NOT: ~a
NOALIAS __forceinline const Vec4Bool NOT( const Vec4Bool PCREF a )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_not( a.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_not( a.m_V );
    #elif defined( _VMX128_ )                    
    //result.m_V = __vnor( a.m_V, a.m_V );
    result.m_V = vmx_not( a.m_V );
    #elif defined( _SSE_ )                      
    //result.m_V = _mm_xor_ps( a.m_V, Const< __m128 >::Mask<1,1,1,1>() );
    result.m_V = sse_not( a.m_V );
    #else
    result.m_V[VX] = !a.m_V[VX];
    result.m_V[VY] = !a.m_V[VY];
    result.m_V[VZ] = !a.m_V[VZ];
    result.m_V[VW] = !a.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool Bitwise OR: a | b
NOALIAS __forceinline const Vec4Bool OR( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_or( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_or( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_or( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] || b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] || b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] || b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] || b.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool Bitwise AND: a & b
NOALIAS __forceinline const Vec4Bool AND( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_and( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_and( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_and( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] && b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] && b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] && b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] && b.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool Bitwise XOR: a ^ b : a != b : (a & ~b) | (~a & b)
NOALIAS __forceinline const Vec4Bool XOR( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_xor( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_xor( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_xor( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] != b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] != b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] != b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] != b.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool Bitwise NOR: ~(a | b)
NOALIAS __forceinline const Vec4Bool NOR( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_nor( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_nor( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_nor( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_nor( a.m_V, b.m_V );
    #else
    result.m_V[VX] = !( a.m_V[VX] || b.m_V[VX] );
    result.m_V[VY] = !( a.m_V[VY] || b.m_V[VY] );
    result.m_V[VZ] = !( a.m_V[VZ] || b.m_V[VZ] );
    result.m_V[VW] = !( a.m_V[VW] || b.m_V[VW] );
    #endif
    return result;
}

/// Vec4Bool Bitwise NAND: ~(a & b)
NOALIAS __forceinline const Vec4Bool NAND( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_nand( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_nand( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_nand( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_nand( a.m_V, b.m_V );
    #else
    result.m_V[VX] = !( a.m_V[VX] && b.m_V[VX] );
    result.m_V[VY] = !( a.m_V[VY] && b.m_V[VY] );
    result.m_V[VZ] = !( a.m_V[VZ] && b.m_V[VZ] );
    result.m_V[VW] = !( a.m_V[VW] && b.m_V[VW] );
    #endif
    return result;
}

/// Vec4Bool Bitwise EQV: ~(a ^ b) : a == b : (a & b) | (~a & ~b)
NOALIAS __forceinline const Vec4Bool EQV( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_eqv( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_eqv( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_eqv( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_eqv( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] == b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] == b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] == b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] == b.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool Bitwise ORC: a | ~b
NOALIAS __forceinline const Vec4Bool ORC( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_orc( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_orc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_orc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_orc( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] || !b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] || !b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] || !b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] || !b.m_V[VW];
    #endif
    return result;
}

/// Vec4Bool Bitwise ANDC: a & ~b
NOALIAS __forceinline const Vec4Bool ANDC( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec4Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_andc( a.m_V, b.m_V );
    #elif defined( _PPU_ )                       
    result.m_V = vec_andc( a.m_V, b.m_V );
    #elif defined( _VMX128_ )                    
    result.m_V = vmx_andc( a.m_V, b.m_V );
    #elif defined( _SSE_ )                      
    result.m_V = sse_andc( a.m_V, b.m_V );
    #else
    result.m_V[VX] = a.m_V[VX] && !b.m_V[VX];
    result.m_V[VY] = a.m_V[VY] && !b.m_V[VY];
    result.m_V[VZ] = a.m_V[VZ] && !b.m_V[VZ];
    result.m_V[VW] = a.m_V[VW] && !b.m_V[VW];
    #endif
    return result;
}


/// Vec4Bool Horizontal AND 3 element
NOALIAS __forceinline const VBool HorizontalAND3( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SIMD_ )
    Vec4Bool xyzt( v );
    SetW( &xyzt, Const<VBool>::True() );
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_and( xyzt.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_and( xyzt.m_V );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_and_int32( xyzt.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_horizontal_and_int32( xyzt.m_V );
    #endif
    #else
    result = X(v) & Y(v) & Z(v);
    #endif
    return result;
}
    
/// Vec4Bool Horizontal AND 4 element
NOALIAS __forceinline const VBool HorizontalAND4( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_and( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_and( v.m_V );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_and_int32( v.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_horizontal_and_int32( v.m_V );
    #else
    result = X(v) & Y(v) & Z(v) & W(v);
    #endif
    return result;
}


/// Vec4Bool Horizontal OR 3 element
NOALIAS __forceinline const VBool HorizontalOR3( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SIMD_ )
    Vec4Bool xyzf( v );
    SetW( &xyzf, Const<VBool>::False() );
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_or( xyzf.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_or( xyzf.m_V );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_or_int32( xyzf.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_horizontal_or_int32( xyzf.m_V );
    #endif
    #else
    result = X(v) | Y(v) | Z(v);
    #endif
    return result;
}
    
/// Vec4Bool Horizontal OR 4 element
NOALIAS __forceinline const VBool HorizontalOR4( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_or( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_or( v.m_V );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_or_int32( v.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_horizontal_or_int32( v.m_V );
    #else
    result = X(v) | Y(v) | Z(v) | W(v);
    #endif
    return result;
}


/// Vec4Bool Horizontal AND NOT 3 element : ~X & ~Y & ~Z
NOALIAS __forceinline const VBool HorizontalANDNOT3( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SIMD_ )
    Vec4Bool xyzt( v );
    SetW( &xyzt, Const<VBool>::True() );
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_andnot( xyzt.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_and( vec_not( xyzt.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_and_int32( vmx_not( xyzt.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = sse_horizontal_and_int32( sse_not( xyzt.m_V ) );
    #endif
    #else
    const Vec4Bool not_v( NOT(v) );
    result = X(not_v) & Y(not_v) & Z(not_v);
    #endif
    return result;
}
    
/// Vec4Bool Horizontal AND NOT 4 element : ~X & ~Y & ~Z & ~W
NOALIAS __forceinline const VBool HorizontalANDNOT4( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_andnot( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_and( vec_not( v.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_and_int32( vmx_not( v.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = sse_horizontal_and_int32( sse_not( v.m_V ) );
    #else
    const Vec4Bool not_v( NOT(v) );
    result = X(not_v) & Y(not_v) & Z(not_v) & W(not_v);
    #endif
    return result;
}


/// Vec4Bool Horizontal OR NOT 3 element : ~X | ~Y | ~Z
NOALIAS __forceinline const VBool HorizontalORNOT3( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SIMD_ )
    Vec4Bool xyzf( v );
    SetW( &xyzf, Const<VBool>::False() );
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_or( spu_not( xyzf.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_or( vec_not( xyzf.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_or_int32( vmx_not( xyzf.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = sse_horizontal_or_int32( sse_not( xyzf.m_V ) );
    #endif
    #else
    const Vec4Bool not_v( NOT(v) );
    result = X(not_v) | Y(not_v) | Z(not_v);
    #endif
    return result;
}
    
/// Vec4Bool Horizontal OR NOT 4 element : ~X | ~Y | ~Z | ~W
NOALIAS __forceinline const VBool HorizontalORNOT4( const Vec4Bool PCREF v )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_or( spu_not( v.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_or( vec_not( v.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_or_int32( vmx_not( v.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = sse_horizontal_or_int32( sse_not( v.m_V ) );
    #else
    const Vec4Bool not_v( NOT(v) );
    result = X(not_v) | Y(not_v) | Z(not_v) | W(not_v);
    #endif
    return result;
}


/// Vec4Bool All True 3 element
NOALIAS __forceinline const VBool AllTrue3( const Vec4Bool PCREF v )
{
    return HorizontalAND3( v );
}

/// Vec4Bool All False 3 element
NOALIAS __forceinline const VBool AllFalse3( const Vec4Bool PCREF v )
{
    //return HorizontalAND3( NOT( v ) );
    return HorizontalANDNOT3( v );
}

/// Vec4Bool Any True 3 element
NOALIAS __forceinline const VBool AnyTrue3( const Vec4Bool PCREF v )
{
    return HorizontalOR3( v );
}

/// Vec4Bool Any False 3 element
NOALIAS __forceinline const VBool AnyFalse3( const Vec4Bool PCREF v )
{
    //return HorizontalOR3( NOT( v ) );
    return HorizontalORNOT3( v );
}


/// Vec4Bool All True 4 element
NOALIAS __forceinline const VBool AllTrue4( const Vec4Bool PCREF v )
{
    return HorizontalAND4( v );
}

/// Vec4Bool All False 4 element
NOALIAS __forceinline const VBool AllFalse4( const Vec4Bool PCREF v )
{
    return HorizontalANDNOT4( v );
}

/// Vec4Bool Any True 4 element
NOALIAS __forceinline const VBool AnyTrue4( const Vec4Bool PCREF v )
{
    return HorizontalOR4( v );
}

/// Vec4Bool Any False 4 element
NOALIAS __forceinline const VBool AnyFalse4( const Vec4Bool PCREF v )
{
    return HorizontalORNOT4( v );
}



/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


