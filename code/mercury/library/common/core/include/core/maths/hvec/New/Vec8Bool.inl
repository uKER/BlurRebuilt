#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC8BOOL_INL
#define SOFT_MATHS_HVEC_NEW_VEC8BOOL_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC8BOOL_INL
#define MATHS_HVEC_NEW_VEC8BOOL_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean
/// @{

/// Make Vec8Bool from a set of 8 template bools ( compile time constants )
template < bool b0, bool b1, bool b2, bool b3, bool b4, bool b5, bool b6, bool b7 >
NOALIAS __forceinline const Vec8Bool MakeVec8Bool()
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned short >::Bool<b0,b1,b2,b3,b4,b5,b6,b7>();
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool short >::Bool<b0,b1,b2,b3,b4,b5,b6,b7>();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool<b0,b1,b2,b3,b4,b5,b6,b7>();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128i >::Bool<b0,b1,b2,b3,b4,b5,b6,b7>();
    #else
    result.m_V[0] = b0;
    result.m_V[1] = b1;
    result.m_V[2] = b2;
    result.m_V[3] = b3;
    result.m_V[4] = b4;
    result.m_V[5] = b5;
    result.m_V[6] = b6;
    result.m_V[7] = b7;
    #endif
    return result;
}

/// Make Vec8Bool from 2 Vec4Bool's
NOALIAS __forceinline const Vec8Bool MakeVec8Bool( const Vec4Bool PCREF a, const Vec4Bool PCREF b )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_pack_mod( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vpkuwum( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vpkuwum( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_packs_epi32( (__m128i&)a.m_V, (__m128i&)b.m_V );
    #else
    result.m_V[0] = a.m_V[0];
    result.m_V[1] = a.m_V[1];
    result.m_V[2] = a.m_V[2];
    result.m_V[3] = a.m_V[3];
    result.m_V[4] = b.m_V[0];
    result.m_V[5] = b.m_V[1];
    result.m_V[6] = b.m_V[2];
    result.m_V[7] = b.m_V[3];
    #endif
    return result;
}

/// Split Vec8Bool into 2 Vec4Bool's
NOALIAS __forceinline void Split( const Vec8Bool PCREF src, Vec4Bool* const __restrict pA, Vec4Bool* const __restrict pB )
{
    #if defined( _SPU_ )
    pA->m_V = (vector unsigned int)spu_merge_hi( src.m_V, src.m_V );
    pB->m_V = (vector unsigned int)spu_merge_lo( src.m_V, src.m_V );
    #elif defined( _PPU_ )
    pA->m_V = (vector bool int)vec_vmrghh( src.m_V, src.m_V );
    pB->m_V = (vector bool int)vec_vmrglh( src.m_V, src.m_V );
    #elif defined( _VMX128_ )
    pA->m_V = __vmrghh( src.m_V, src.m_V );
    pB->m_V = __vmrglh( src.m_V, src.m_V );
    #elif defined( _SSE_ )
    pA->m_V = (__m128&)_mm_unpacklo_epi16( src.m_V, src.m_V );
    pB->m_V = (__m128&)_mm_unpackhi_epi16( src.m_V, src.m_V );
    #else
    pA->m_V[0] = src.m_V[0];
    pA->m_V[1] = src.m_V[1];
    pA->m_V[2] = src.m_V[2];
    pA->m_V[3] = src.m_V[3];
    pB->m_V[0] = src.m_V[4];
    pB->m_V[1] = src.m_V[5];
    pB->m_V[2] = src.m_V[6];
    pB->m_V[3] = src.m_V[7];
    #endif
}


/// Vec8Bool accessor: Get element
template < int I >
NOALIAS __forceinline const VBool Element( const Vec8Bool PCREF v )
{
    BIZ_STATIC_ASSERT( I < 8 );
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<I>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, I );
    #elif defined( _VMX128_ )
    result.m_V = __vsplth( v.m_V, I );
    #elif defined( _SSE_ )
    result.m_V = sse_splat_int16<I>( v.m_V );
    #else
    result.m_V = v.m_V[I];
    #endif
    return result;
}

/// Vec8Bool accessor: Set element
template < int I >
NOALIAS __forceinline void SetElement( Vec8Bool* const __restrict pDst, const VBool PCREF src )
{
    BIZ_STATIC_ASSERT( I < 8 );
    #if defined( _SPU_ )
    pDst->m_V = spu_sel( pDst->m_V, src.m_V, Const< vector unsigned short >::Bool<I==0,I==1,I==2,I==3,I==4,I==5,I==6,I==7>() );
    #elif defined( _PPU_ )
    pDst->m_V = vec_sel( pDst->m_V, src.m_V, Const< vector bool short >::Bool<I==0,I==1,I==2,I==3,I==4,I==5,I==6,I==7>() );
    #elif defined( _VMX128_ )
    pDst->m_V = __vsel( pDst->m_V, src.m_V, Const< __vector4 >::Bool<I==0,I==1,I==2,I==3,I==4,I==5,I==6,I==7>() );
    #elif defined( _SSE_ )
    pDst->m_V = sse_select( pDst->m_V, src.m_V, Const< __m128i >::Bool<I==0,I==1,I==2,I==3,I==4,I==5,I==6,I==7>() );
    #else
    pDst->m_V[I] = src.m_V;
    #endif
}





/// Vec8Bool negation operator
NOALIAS __forceinline const Vec8Bool operator ~ ( const Vec8Bool PCREF x )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_not( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_nor( x.m_V, x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vnor( x.m_V, x.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_si128( x.m_V, Const< __m128i >::Bool<1,1,1,1,1,1,1,1>() );
    #else
    result.m_V[0] = !x.m_V[0];
    result.m_V[1] = !x.m_V[1];
    result.m_V[2] = !x.m_V[2];
    result.m_V[3] = !x.m_V[3];
    result.m_V[4] = !x.m_V[4];
    result.m_V[5] = !x.m_V[5];
    result.m_V[6] = !x.m_V[6];
    result.m_V[7] = !x.m_V[7];
    #endif
    return result;
}

/// Vec8Bool logical and operator
NOALIAS __forceinline const Vec8Bool operator & ( const Vec8Bool PCREF x, const Vec8Bool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_si128( x.m_V, y.m_V );
    #else
    result.m_V[0] = x.m_V[0] && y.m_V[0];
    result.m_V[1] = x.m_V[1] && y.m_V[1];
    result.m_V[2] = x.m_V[2] && y.m_V[2];
    result.m_V[3] = x.m_V[3] && y.m_V[3];
    result.m_V[4] = x.m_V[4] && y.m_V[4];
    result.m_V[5] = x.m_V[5] && y.m_V[5];
    result.m_V[6] = x.m_V[6] && y.m_V[6];
    result.m_V[7] = x.m_V[7] && y.m_V[7];
    #endif
    return result;
}

/// Vec8Bool logical or operator
NOALIAS __forceinline const Vec8Bool operator | ( const Vec8Bool PCREF x, const Vec8Bool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_si128( x.m_V, y.m_V );
    #else
    result.m_V[0] = x.m_V[0] || y.m_V[0];
    result.m_V[1] = x.m_V[1] || y.m_V[1];
    result.m_V[2] = x.m_V[2] || y.m_V[2];
    result.m_V[3] = x.m_V[3] || y.m_V[3];
    result.m_V[4] = x.m_V[4] || y.m_V[4];
    result.m_V[5] = x.m_V[5] || y.m_V[5];
    result.m_V[6] = x.m_V[6] || y.m_V[6];
    result.m_V[7] = x.m_V[7] || y.m_V[7];
    #endif
    return result;
}

/// Vec8Bool logical xor operator
NOALIAS __forceinline const Vec8Bool operator ^ ( const Vec8Bool PCREF x, const Vec8Bool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_si128( x.m_V, y.m_V );
    #else
    result.m_V[0] = x.m_V[0] != y.m_V[0];
    result.m_V[1] = x.m_V[1] != y.m_V[1];
    result.m_V[2] = x.m_V[2] != y.m_V[2];
    result.m_V[3] = x.m_V[3] != y.m_V[3];
    result.m_V[4] = x.m_V[4] != y.m_V[4];
    result.m_V[5] = x.m_V[5] != y.m_V[5];
    result.m_V[6] = x.m_V[6] != y.m_V[6];
    result.m_V[7] = x.m_V[7] != y.m_V[7];
    #endif
    return result;
}


/// Vec8Bool equal operator
NOALIAS __forceinline const VBool operator == ( const Vec8Bool PCREF x, const Vec8Bool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_and( spu_cmpeq( x.m_V, y.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_and( vec_cmpeq( (vector unsigned short)x.m_V, (vector unsigned short)y.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_and_int16( __vcmpequh( x.m_V, y.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = (__m128&)sse_horizontal_and_int16( _mm_cmpeq_epi16( x.m_V, y.m_V ) );
    #else
    result.m_V = x.m_V[0] == y.m_V[0] &&
                 x.m_V[1] == y.m_V[1] &&
                 x.m_V[2] == y.m_V[2] &&
                 x.m_V[3] == y.m_V[3] &&
                 x.m_V[4] == y.m_V[4] &&
                 x.m_V[5] == y.m_V[5] &&
                 x.m_V[6] == y.m_V[6] &&
                 x.m_V[7] == y.m_V[7];
    #endif
    return result;
}

/// Vec8Bool not-equal operator
NOALIAS __forceinline const VBool operator != ( const Vec8Bool PCREF x, const Vec8Bool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_or( spu_xor( x.m_V, y.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_or( vec_xor( x.m_V, y.m_V ) );
    #elif defined( _VMX128_ )
    const __vector4 ne( __vxor( x.m_V, y.m_V ) );
    result.m_V = vmx_horizontal_or_int16( ne );
    #elif defined( _SSE_ )
    const __m128i ne( _mm_xor_si128( x.m_V, y.m_V ) );
    result.m_V = (__m128&)sse_horizontal_or_int16( ne );
    #else
    result.m_V = x.m_V[0] != y.m_V[0] ||
                 x.m_V[1] != y.m_V[1] ||
                 x.m_V[2] != y.m_V[2] ||
                 x.m_V[3] != y.m_V[3] ||
                 x.m_V[4] != y.m_V[4] ||
                 x.m_V[5] != y.m_V[5] ||
                 x.m_V[6] != y.m_V[6] ||
                 x.m_V[7] != y.m_V[7];
    #endif
    return result;
}

/// Vec8Bool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec8Bool& x, const Vec8Bool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_and_si128( x.m_V, y.m_V );
    #else
    x.m_V[0] = x.m_V[0] && y.m_V[0];
    x.m_V[1] = x.m_V[1] && y.m_V[1];
    x.m_V[2] = x.m_V[2] && y.m_V[2];
    x.m_V[3] = x.m_V[3] && y.m_V[3];
    x.m_V[4] = x.m_V[4] && y.m_V[4];
    x.m_V[5] = x.m_V[5] && y.m_V[5];
    x.m_V[6] = x.m_V[6] && y.m_V[6];
    x.m_V[7] = x.m_V[7] && y.m_V[7];
    #endif
}

/// Vec8Bool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec8Bool& x, const Vec8Bool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_or_si128( x.m_V, y.m_V );
    #else
    x.m_V[0] = x.m_V[0] || y.m_V[0];
    x.m_V[1] = x.m_V[1] || y.m_V[1];
    x.m_V[2] = x.m_V[2] || y.m_V[2];
    x.m_V[3] = x.m_V[3] || y.m_V[3];
    x.m_V[4] = x.m_V[4] || y.m_V[4];
    x.m_V[5] = x.m_V[5] || y.m_V[5];
    x.m_V[6] = x.m_V[6] || y.m_V[6];
    x.m_V[7] = x.m_V[7] || y.m_V[7];
    #endif
}
    




/// Vec8Bool & VBool logical and operator
NOALIAS __forceinline const Vec8Bool operator & ( const Vec8Bool PCREF x, const VBool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( x.m_V, (vector unsigned short)y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( x.m_V, (vector bool short)y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_si128( x.m_V, (__m128i&)y.m_V );
    #else
    result.m_V[0] = x.m_V[0] && y.m_V;
    result.m_V[1] = x.m_V[1] && y.m_V;
    result.m_V[2] = x.m_V[2] && y.m_V;
    result.m_V[3] = x.m_V[3] && y.m_V;
    result.m_V[4] = x.m_V[4] && y.m_V;
    result.m_V[5] = x.m_V[5] && y.m_V;
    result.m_V[6] = x.m_V[6] && y.m_V;
    result.m_V[7] = x.m_V[7] && y.m_V;
    #endif
    return result;
}

/// VBool & Vec8Bool logical and operator
NOALIAS __forceinline const Vec8Bool operator & ( const VBool PCREF x, const Vec8Bool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( (vector unsigned short)x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( (vector bool short)x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_si128( (__m128i&)x.m_V, y.m_V );
    #else
    result.m_V[0] = x.m_V && y.m_V[0];
    result.m_V[1] = x.m_V && y.m_V[1];
    result.m_V[2] = x.m_V && y.m_V[2];
    result.m_V[3] = x.m_V && y.m_V[3];
    result.m_V[4] = x.m_V && y.m_V[4];
    result.m_V[5] = x.m_V && y.m_V[5];
    result.m_V[6] = x.m_V && y.m_V[6];
    result.m_V[7] = x.m_V && y.m_V[7];
    #endif
    return result;
}

/// Vec8Bool & VBool logical or operator
NOALIAS __forceinline const Vec8Bool operator | ( const Vec8Bool PCREF x, const VBool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( x.m_V, (vector unsigned short)y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( x.m_V, (vector bool short)y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_si128( x.m_V, (__m128i&)y.m_V );
    #else
    result.m_V[0] = x.m_V[0] || y.m_V;
    result.m_V[1] = x.m_V[1] || y.m_V;
    result.m_V[2] = x.m_V[2] || y.m_V;
    result.m_V[3] = x.m_V[3] || y.m_V;
    result.m_V[4] = x.m_V[4] || y.m_V;
    result.m_V[5] = x.m_V[5] || y.m_V;
    result.m_V[6] = x.m_V[6] || y.m_V;
    result.m_V[7] = x.m_V[7] || y.m_V;
    #endif
    return result;
}

/// VBool & Vec8Bool logical or operator
NOALIAS __forceinline const Vec8Bool operator | ( const VBool PCREF x, const Vec8Bool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( (vector unsigned short)x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( (vector bool short)x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_si128( (__m128i&)x.m_V, y.m_V );
    #else
    result.m_V[0] = x.m_V || y.m_V[0];
    result.m_V[1] = x.m_V || y.m_V[1];
    result.m_V[2] = x.m_V || y.m_V[2];
    result.m_V[3] = x.m_V || y.m_V[3];
    result.m_V[4] = x.m_V || y.m_V[4];
    result.m_V[5] = x.m_V || y.m_V[5];
    result.m_V[6] = x.m_V || y.m_V[6];
    result.m_V[7] = x.m_V || y.m_V[7];
    #endif
    return result;
}

/// Vec8Bool & VBool logical xor operator
NOALIAS __forceinline const Vec8Bool operator ^ ( const Vec8Bool PCREF x, const VBool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, (vector unsigned short)y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, (vector bool short)y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_si128( x.m_V, (__m128i&)y.m_V );
    #else
    result.m_V[0] = x.m_V[0] != y.m_V;
    result.m_V[1] = x.m_V[1] != y.m_V;
    result.m_V[2] = x.m_V[2] != y.m_V;
    result.m_V[3] = x.m_V[3] != y.m_V;
    result.m_V[4] = x.m_V[4] != y.m_V;
    result.m_V[5] = x.m_V[5] != y.m_V;
    result.m_V[6] = x.m_V[6] != y.m_V;
    result.m_V[7] = x.m_V[7] != y.m_V;
    #endif
    return result;
}

/// VBool & Vec8Bool logical xor operator
NOALIAS __forceinline const Vec8Bool operator ^ ( const VBool PCREF x, const Vec8Bool PCREF y )
{
    Vec8Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( (vector unsigned short)x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( (vector bool short)x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_si128( (__m128i&)x.m_V, y.m_V );
    #else
    result.m_V[0] = x.m_V != y.m_V[0];
    result.m_V[1] = x.m_V != y.m_V[1];
    result.m_V[2] = x.m_V != y.m_V[2];
    result.m_V[3] = x.m_V != y.m_V[3];
    result.m_V[4] = x.m_V != y.m_V[4];
    result.m_V[5] = x.m_V != y.m_V[5];
    result.m_V[6] = x.m_V != y.m_V[6];
    result.m_V[7] = x.m_V != y.m_V[7];
    #endif
    return result;
}

/// Vec8Bool & VBool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec8Bool& x, const VBool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_and( x.m_V, (vector unsigned short)y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_and( x.m_V, (vector bool short)y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_and_si128( x.m_V, (__m128i&)y.m_V );
    #else
    x.m_V[0] = x.m_V[0] && y.m_V;
    x.m_V[1] = x.m_V[1] && y.m_V;
    x.m_V[2] = x.m_V[2] && y.m_V;
    x.m_V[3] = x.m_V[3] && y.m_V;
    x.m_V[4] = x.m_V[4] && y.m_V;
    x.m_V[5] = x.m_V[5] && y.m_V;
    x.m_V[6] = x.m_V[6] && y.m_V;
    x.m_V[7] = x.m_V[7] && y.m_V;
    #endif
}

/// Vec8Bool & VBool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec8Bool& x, const VBool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_or( x.m_V, (vector unsigned short)y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_or( x.m_V, (vector bool short)y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_or_si128( x.m_V, (__m128i&)y.m_V );
    #else
    x.m_V[0] = x.m_V[0] || y.m_V;
    x.m_V[1] = x.m_V[1] || y.m_V;
    x.m_V[2] = x.m_V[2] || y.m_V;
    x.m_V[3] = x.m_V[3] || y.m_V;
    x.m_V[4] = x.m_V[4] || y.m_V;
    x.m_V[5] = x.m_V[5] || y.m_V;
    x.m_V[6] = x.m_V[6] || y.m_V;
    x.m_V[7] = x.m_V[7] || y.m_V;
    #endif
}
    
/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


