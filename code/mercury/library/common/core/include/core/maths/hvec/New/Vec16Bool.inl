#if defined( SOFT_MATHS )
#ifndef SOFT_MATHS_HVEC_NEW_VEC16BOOL_INL
#define SOFT_MATHS_HVEC_NEW_VEC16BOOL_INL
#define INCLUDE_THIS
#endif
#else
#ifndef MATHS_HVEC_NEW_VEC16BOOL_INL
#define MATHS_HVEC_NEW_VEC16BOOL_INL
#define INCLUDE_THIS
#endif
#endif

#if defined( INCLUDE_THIS )
#undef INCLUDE_THIS

namespace Maths
{
/// @addtogroup Boolean
/// @{

/// Make Vec16Bool from a set of 16 template bools ( compile time constants )
template < bool b0,  bool b1,  bool b2,  bool b3,
           bool b4,  bool b5,  bool b6,  bool b7,
           bool b8,  bool b9,  bool b10, bool b11,
           bool b12, bool b13, bool b14, bool b15 >
NOALIAS __forceinline const Vec16Bool MakeVec16Bool()
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = Const< vector unsigned short >::Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>();
    #elif defined( _PPU_ )
    result.m_V = Const< vector bool short >::Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>();
    #elif defined( _VMX128_ )
    result.m_V = Const< __vector4 >::Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>();
    #elif defined( _SSE_ )
    result.m_V = Const< __m128i >::Bool<b0,b1,b2,b3,b4,b5,b6,b7,b8,b9,b10,b11,b12,b13,b14,b15>();
    #else
    result.m_V[0]  = b0;  result.m_V[1]  = b1;  result.m_V[2]  = b2;  result.m_V[3]  = b3;
    result.m_V[4]  = b4;  result.m_V[5]  = b5;  result.m_V[6]  = b6;  result.m_V[7]  = b7;
    result.m_V[8]  = b8;  result.m_V[9]  = b9;  result.m_V[10] = b10; result.m_V[11] = b11;
    result.m_V[12] = b12; result.m_V[13] = b13; result.m_V[14] = b14; result.m_V[15] = b15;
    #endif
    return result;
}

/// Make Vec16Bool from 4 Vec4Bool's
NOALIAS __forceinline const Vec16Bool MakeVec16Bool( const Vec4Bool PCREF a, const Vec4Bool PCREF b, const Vec4Bool PCREF c, const Vec4Bool PCREF d )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_pack_mod( spu_pack_mod( a.m_V, b.m_V ),
                               spu_pack_mod( c.m_V, d.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_vpkuhum( vec_vpkuwum( a.m_V, b.m_V ),
                              vec_vpkuwum( c.m_V, d.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = __vpkuhum( __vpkuwum( a.m_V, b.m_V ),
                            __vpkuwum( c.m_V, d.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = _mm_packs_epi16( _mm_packs_epi32( (__m128i&)a.m_V, (__m128i&)b.m_V ),
                                  _mm_packs_epi32( (__m128i&)c.m_V, (__m128i&)d.m_V ) );
    #else
    result.m_V[0]  = a.m_V[0]; result.m_V[1]  = a.m_V[1]; result.m_V[2]  = a.m_V[2]; result.m_V[3]  = a.m_V[3];
    result.m_V[4]  = b.m_V[0]; result.m_V[5]  = b.m_V[1]; result.m_V[6]  = b.m_V[2]; result.m_V[7]  = b.m_V[3];
    result.m_V[8]  = c.m_V[0]; result.m_V[9]  = c.m_V[1]; result.m_V[10] = c.m_V[2]; result.m_V[11] = c.m_V[3];
    result.m_V[12] = d.m_V[0]; result.m_V[13] = d.m_V[1]; result.m_V[14] = d.m_V[2]; result.m_V[15] = d.m_V[3];
    #endif
    return result;
}

/// Split Vec16Bool into 4 Vec4Bool's
NOALIAS __forceinline void Split( const Vec16Bool PCREF src,
                                  Vec4Bool* const __restrict pA,
                                  Vec4Bool* const __restrict pB,
                                  Vec4Bool* const __restrict pC,
                                  Vec4Bool* const __restrict pD )
{
    #if defined( _SPU_ )
    const vector unsigned short ab( (vector unsigned short)spu_merge_hi( src.m_V, src.m_V ) );
    const vector unsigned short cd( (vector unsigned short)spu_merge_lo( src.m_V, src.m_V ) );
    pA->m_V = (vector unsigned int)spu_merge_hi( ab, ab );
    pB->m_V = (vector unsigned int)spu_merge_lo( ab, ab );
    pC->m_V = (vector unsigned int)spu_merge_hi( cd, cd );
    pD->m_V = (vector unsigned int)spu_merge_lo( cd, cd );
    #elif defined( _PPU_ )
    const vector bool short ab( (vector bool short)vec_vmrghb( src.m_V, src.m_V ) );
    const vector bool short cd( (vector bool short)vec_vmrglb( src.m_V, src.m_V ) );
    pA->m_V = (vector bool int)vec_vmrghh( ab, ab );
    pB->m_V = (vector bool int)vec_vmrglh( ab, ab );
    pC->m_V = (vector bool int)vec_vmrghh( cd, cd );
    pD->m_V = (vector bool int)vec_vmrglh( cd, cd );
    #elif defined( _VMX128_ )
    const __vector4 ab( __vmrghb( src.m_V, src.m_V ) );
    const __vector4 cd( __vmrglb( src.m_V, src.m_V ) );
    pA->m_V = __vmrghh( ab, ab );
    pB->m_V = __vmrglh( ab, ab );
    pC->m_V = __vmrghh( cd, cd );
    pD->m_V = __vmrglh( cd, cd );
    #elif defined( _SSE_ )
    const __m128i ab( _mm_unpacklo_epi8( src.m_V, src.m_V ) );
    const __m128i cd( _mm_unpackhi_epi8( src.m_V, src.m_V ) );
    pA->m_V = (__m128&)_mm_unpacklo_epi16( ab, ab );
    pB->m_V = (__m128&)_mm_unpackhi_epi16( ab, ab );
    pC->m_V = (__m128&)_mm_unpacklo_epi16( cd, cd );
    pD->m_V = (__m128&)_mm_unpackhi_epi16( cd, cd );
    #else
    pA->m_V[0] = src.m_V[0];  pA->m_V[1] = src.m_V[1];  pA->m_V[2] = src.m_V[2];  pA->m_V[3] = src.m_V[3];
    pB->m_V[0] = src.m_V[4];  pB->m_V[1] = src.m_V[5];  pB->m_V[2] = src.m_V[6];  pB->m_V[3] = src.m_V[7];
    pC->m_V[0] = src.m_V[8];  pC->m_V[1] = src.m_V[9];  pC->m_V[2] = src.m_V[10]; pC->m_V[3] = src.m_V[11];
    pD->m_V[0] = src.m_V[12]; pD->m_V[1] = src.m_V[13]; pD->m_V[2] = src.m_V[14]; pD->m_V[3] = src.m_V[15];
    #endif
}


/// Make Vec16Bool from 2 Vec8Bool's
NOALIAS __forceinline const Vec16Bool MakeVec16Bool( const Vec8Bool PCREF a, const Vec8Bool PCREF b )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_pack_mod( a.m_V, b.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_vpkuhum( a.m_V, b.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vpkuhum( a.m_V, b.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_packs_epi16( a.m_V, b.m_V );
    #else
    result.m_V[0]  = a.m_V[0]; result.m_V[1]  = a.m_V[1]; result.m_V[2]  = a.m_V[2]; result.m_V[3]  = a.m_V[3];
    result.m_V[4]  = a.m_V[4]; result.m_V[5]  = a.m_V[5]; result.m_V[6]  = a.m_V[6]; result.m_V[7]  = a.m_V[7];
    result.m_V[8]  = b.m_V[0]; result.m_V[9]  = b.m_V[1]; result.m_V[10] = b.m_V[2]; result.m_V[11] = b.m_V[3];
    result.m_V[12] = b.m_V[4]; result.m_V[13] = b.m_V[5]; result.m_V[14] = b.m_V[6]; result.m_V[15] = b.m_V[7];
    #endif
    return result;
}

/// Split Vec16Bool into 2 Vec8Bool's
NOALIAS __forceinline void Split( const Vec16Bool PCREF src,
                                  Vec8Bool* const __restrict pA,
                                  Vec8Bool* const __restrict pB )
{
    #if defined( _SPU_ )
    pA->m_V = (vector unsigned short)spu_merge_hi( src.m_V, src.m_V );
    pB->m_V = (vector unsigned short)spu_merge_lo( src.m_V, src.m_V );
    #elif defined( _PPU_ )
    pA->m_V = (vector bool short)vec_vmrghb( src.m_V, src.m_V );
    pB->m_V = (vector bool short)vec_vmrglb( src.m_V, src.m_V );
    #elif defined( _VMX128_ )
    pA->m_V = __vmrghb( src.m_V, src.m_V );
    pB->m_V = __vmrglb( src.m_V, src.m_V );
    #elif defined( _SSE_ )
    pA->m_V = _mm_unpacklo_epi8( src.m_V, src.m_V );
    pB->m_V = _mm_unpackhi_epi8( src.m_V, src.m_V );
    #else
    pA->m_V[0] = src.m_V[0];  pA->m_V[1] = src.m_V[1];  pA->m_V[2] = src.m_V[2];  pA->m_V[3] = src.m_V[3];
    pA->m_V[4] = src.m_V[4];  pA->m_V[5] = src.m_V[5];  pA->m_V[6] = src.m_V[6];  pA->m_V[7] = src.m_V[7];
    pB->m_V[0] = src.m_V[8];  pB->m_V[1] = src.m_V[9];  pB->m_V[2] = src.m_V[10]; pB->m_V[3] = src.m_V[11];
    pB->m_V[4] = src.m_V[12]; pB->m_V[5] = src.m_V[13]; pB->m_V[6] = src.m_V[14]; pB->m_V[7] = src.m_V[15];
    #endif
}


/// Vec16Bool accessor: Get element
template < int I >
NOALIAS __forceinline const VBool Element( const Vec16Bool PCREF v )
{
    BIZ_STATIC_ASSERT( I < 16 );
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_splat<I>( v.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( v.m_V, I );
    #elif defined( _VMX128_ )
    result.m_V = __vspltb( v.m_V, I );
    #elif defined( _SSE_ )
    result.m_V = sse_splat_8<I>( v.m_V );
    #else
    result.m_V = v.m_V[I];
    #endif
    return result;
}

/// Vec16Bool accessor: Set element
template < int I >
NOALIAS __forceinline void SetElement( Vec16Bool* const __restrict pDst, const VBool PCREF src )
{
    BIZ_STATIC_ASSERT( I < 16 );
    #if defined( _SPU_ )
    pDst->m_V = spu_sel( pDst->m_V, src.m_V, Const< vector unsigned char >::Bool<I==0,I==1,I==2,I==3,I==4,I==5,I==6,I==7,I==8,I==9,I==10,I==11,I==12,I==13,I==14,I==15>() );
    #elif defined( _PPU_ )
    pDst->m_V = vec_sel( pDst->m_V, src.m_V, Const< vector bool char >::Bool<I==0,I==1,I==2,I==3,I==4,I==5,I==6,I==7,I==8,I==9,I==10,I==11,I==12,I==13,I==14,I==15>() );
    #elif defined( _VMX128_ )
    pDst->m_V = __vsel( pDst->m_V, src.m_V, Const< __vector4 >::Bool<I==0,I==1,I==2,I==3,I==4,I==5,I==6,I==7,I==8,I==9,I==10,I==11,I==12,I==13,I==14,I==15>() );
    #elif defined( _SSE_ )
    pDst->m_V = sse_select( pDst->m_V, src.m_V, Const< __m128i >::Bool<I==0,I==1,I==2,I==3,I==4,I==5,I==6,I==7,I==8,I==9,I==10,I==11,I==12,I==13,I==14,I==15>() );
    #else
    pDst->m_V[I] = src.m_V;
    #endif
}


/// Vec16Bool negation operator
NOALIAS __forceinline const Vec16Bool operator ~ ( const Vec16Bool PCREF x )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_not( x.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_nor( x.m_V, x.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vnor( x.m_V, x.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_si128( x.m_V, Const< __m128i >::Bool<true>() );
    #else
    result.m_V[0]  = !x.m_V[0];  result.m_V[1]  = !x.m_V[1];  result.m_V[2]  = !x.m_V[2];  result.m_V[3]  = !x.m_V[3];
    result.m_V[4]  = !x.m_V[4];  result.m_V[5]  = !x.m_V[5];  result.m_V[6]  = !x.m_V[6];  result.m_V[7]  = !x.m_V[7];
    result.m_V[8]  = !x.m_V[8];  result.m_V[9]  = !x.m_V[9];  result.m_V[10] = !x.m_V[10]; result.m_V[11] = !x.m_V[11];
    result.m_V[12] = !x.m_V[12]; result.m_V[13] = !x.m_V[13]; result.m_V[14] = !x.m_V[14]; result.m_V[15] = !x.m_V[15];
    #endif
    return result;
}

/// Vec16Bool logical and operator
NOALIAS __forceinline const Vec16Bool operator & ( const Vec16Bool PCREF x, const Vec16Bool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_si128( x.m_V, y.m_V );
    #else
    result.m_V[0]  = x.m_V[0]  && y.m_V[0];
    result.m_V[1]  = x.m_V[1]  && y.m_V[1];
    result.m_V[2]  = x.m_V[2]  && y.m_V[2];
    result.m_V[3]  = x.m_V[3]  && y.m_V[3];
    result.m_V[4]  = x.m_V[4]  && y.m_V[4];
    result.m_V[5]  = x.m_V[5]  && y.m_V[5];
    result.m_V[6]  = x.m_V[6]  && y.m_V[6];
    result.m_V[7]  = x.m_V[7]  && y.m_V[7];
    result.m_V[8]  = x.m_V[8]  && y.m_V[8];
    result.m_V[9]  = x.m_V[9]  && y.m_V[9];
    result.m_V[10] = x.m_V[10] && y.m_V[10];
    result.m_V[11] = x.m_V[11] && y.m_V[11];
    result.m_V[12] = x.m_V[12] && y.m_V[12];
    result.m_V[13] = x.m_V[13] && y.m_V[13];
    result.m_V[14] = x.m_V[14] && y.m_V[14];
    result.m_V[15] = x.m_V[15] && y.m_V[15];
    #endif
    return result;
}

/// Vec16Bool logical or operator
NOALIAS __forceinline const Vec16Bool operator | ( const Vec16Bool PCREF x, const Vec16Bool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_si128( x.m_V, y.m_V );
    #else
    result.m_V[0]  = x.m_V[0]  || y.m_V[0];
    result.m_V[1]  = x.m_V[1]  || y.m_V[1];
    result.m_V[2]  = x.m_V[2]  || y.m_V[2];
    result.m_V[3]  = x.m_V[3]  || y.m_V[3];
    result.m_V[4]  = x.m_V[4]  || y.m_V[4];
    result.m_V[5]  = x.m_V[5]  || y.m_V[5];
    result.m_V[6]  = x.m_V[6]  || y.m_V[6];
    result.m_V[7]  = x.m_V[7]  || y.m_V[7];
    result.m_V[8]  = x.m_V[8]  || y.m_V[8];
    result.m_V[9]  = x.m_V[9]  || y.m_V[9];
    result.m_V[10] = x.m_V[10] || y.m_V[10];
    result.m_V[11] = x.m_V[11] || y.m_V[11];
    result.m_V[12] = x.m_V[12] || y.m_V[12];
    result.m_V[13] = x.m_V[13] || y.m_V[13];
    result.m_V[14] = x.m_V[14] || y.m_V[14];
    result.m_V[15] = x.m_V[15] || y.m_V[15];
    #endif
    return result;
}

/// Vec16Bool logical xor operator
NOALIAS __forceinline const Vec16Bool operator ^ ( const Vec16Bool PCREF x, const Vec16Bool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_si128( x.m_V, y.m_V );
    #else
    result.m_V[0]  = x.m_V[0]  != y.m_V[0];
    result.m_V[1]  = x.m_V[1]  != y.m_V[1];
    result.m_V[2]  = x.m_V[2]  != y.m_V[2];
    result.m_V[3]  = x.m_V[3]  != y.m_V[3];
    result.m_V[4]  = x.m_V[4]  != y.m_V[4];
    result.m_V[5]  = x.m_V[5]  != y.m_V[5];
    result.m_V[6]  = x.m_V[6]  != y.m_V[6];
    result.m_V[7]  = x.m_V[7]  != y.m_V[7];
    result.m_V[8]  = x.m_V[8]  != y.m_V[8];
    result.m_V[9]  = x.m_V[9]  != y.m_V[9];
    result.m_V[10] = x.m_V[10] != y.m_V[10];
    result.m_V[11] = x.m_V[11] != y.m_V[11];
    result.m_V[12] = x.m_V[12] != y.m_V[12];
    result.m_V[13] = x.m_V[13] != y.m_V[13];
    result.m_V[14] = x.m_V[14] != y.m_V[14];
    result.m_V[15] = x.m_V[15] != y.m_V[15];
    #endif
    return result;
}


/// Vec16Bool equal operator
NOALIAS __forceinline const VBool operator == ( const Vec16Bool PCREF x, const Vec16Bool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_and( spu_cmpeq( x.m_V, y.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_and( vec_cmpeq( (vector unsigned short)x.m_V, (vector unsigned short)y.m_V ) );
    #elif defined( _VMX128_ )
    result.m_V = vmx_horizontal_and_int8( __vcmpequh( x.m_V, y.m_V ) );
    #elif defined( _SSE_ )
    result.m_V = (__m128&)sse_horizontal_and_int8( _mm_cmpeq_epi16( x.m_V, y.m_V ) );
    #else
    result.m_V = x.m_V[0]  == y.m_V[0]  &&
                 x.m_V[1]  == y.m_V[1]  &&
                 x.m_V[2]  == y.m_V[2]  &&
                 x.m_V[3]  == y.m_V[3]  &&
                 x.m_V[4]  == y.m_V[4]  &&
                 x.m_V[5]  == y.m_V[5]  &&
                 x.m_V[6]  == y.m_V[6]  &&
                 x.m_V[7]  == y.m_V[7]  &&
                 x.m_V[8]  == y.m_V[8]  &&
                 x.m_V[9]  == y.m_V[9]  &&
                 x.m_V[10] == y.m_V[10] &&
                 x.m_V[11] == y.m_V[11] &&
                 x.m_V[12] == y.m_V[12] &&
                 x.m_V[13] == y.m_V[13] &&
                 x.m_V[14] == y.m_V[14] &&
                 x.m_V[15] == y.m_V[15];
    #endif
    return result;
}

/// Vec16Bool not-equal operator
NOALIAS __forceinline const VBool operator != ( const Vec16Bool PCREF x, const Vec16Bool PCREF y )
{
    VBool result;
    #if defined( _SPU_ )
    result.m_V = spu_horizontal_or( spu_xor( x.m_V, y.m_V ) );
    #elif defined( _PPU_ )
    result.m_V = vec_horizontal_or( vec_xor( x.m_V, y.m_V ) );
    #elif defined( _VMX128_ )
    const __vector4 ne( __vxor( x.m_V, y.m_V ) );
    result.m_V = vmx_horizontal_or_int8( ne );
    #elif defined( _SSE_ )
    const __m128i ne( _mm_xor_si128( x.m_V, y.m_V ) );
    result.m_V = (__m128&)sse_horizontal_or_int8( ne );
    #else
    result.m_V = x.m_V[0]  != y.m_V[0]  ||
                 x.m_V[1]  != y.m_V[1]  ||
                 x.m_V[2]  != y.m_V[2]  ||
                 x.m_V[3]  != y.m_V[3]  ||
                 x.m_V[4]  != y.m_V[4]  ||
                 x.m_V[5]  != y.m_V[5]  ||
                 x.m_V[6]  != y.m_V[6]  ||
                 x.m_V[7]  != y.m_V[7]  ||
                 x.m_V[8]  != y.m_V[8]  ||
                 x.m_V[9]  != y.m_V[9]  ||
                 x.m_V[10] != y.m_V[10] ||
                 x.m_V[11] != y.m_V[11] ||
                 x.m_V[12] != y.m_V[12] ||
                 x.m_V[13] != y.m_V[13] ||
                 x.m_V[14] != y.m_V[14] ||
                 x.m_V[15] != y.m_V[15];
    #endif
    return result;
}

/// Vec16Bool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec16Bool& x, const Vec16Bool PCREF y )
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
    x.m_V[0]  = x.m_V[0]  && y.m_V[0];
    x.m_V[1]  = x.m_V[1]  && y.m_V[1];
    x.m_V[2]  = x.m_V[2]  && y.m_V[2];
    x.m_V[3]  = x.m_V[3]  && y.m_V[3];
    x.m_V[4]  = x.m_V[4]  && y.m_V[4];
    x.m_V[5]  = x.m_V[5]  && y.m_V[5];
    x.m_V[6]  = x.m_V[6]  && y.m_V[6];
    x.m_V[7]  = x.m_V[7]  && y.m_V[7];
    x.m_V[8]  = x.m_V[8]  && y.m_V[8];
    x.m_V[9]  = x.m_V[9]  && y.m_V[9];
    x.m_V[10] = x.m_V[10] && y.m_V[10];
    x.m_V[11] = x.m_V[11] && y.m_V[11];
    x.m_V[12] = x.m_V[12] && y.m_V[12];
    x.m_V[13] = x.m_V[13] && y.m_V[13];
    x.m_V[14] = x.m_V[14] && y.m_V[14];
    x.m_V[15] = x.m_V[15] && y.m_V[15];
    #endif
}

/// Vec16Bool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec16Bool& x, const Vec16Bool PCREF y )
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
    x.m_V[0]  = x.m_V[0]  || y.m_V[0];
    x.m_V[1]  = x.m_V[1]  || y.m_V[1];
    x.m_V[2]  = x.m_V[2]  || y.m_V[2];
    x.m_V[3]  = x.m_V[3]  || y.m_V[3];
    x.m_V[4]  = x.m_V[4]  || y.m_V[4];
    x.m_V[5]  = x.m_V[5]  || y.m_V[5];
    x.m_V[6]  = x.m_V[6]  || y.m_V[6];
    x.m_V[7]  = x.m_V[7]  || y.m_V[7];
    x.m_V[8]  = x.m_V[8]  || y.m_V[8];
    x.m_V[9]  = x.m_V[9]  || y.m_V[9];
    x.m_V[10] = x.m_V[10] || y.m_V[10];
    x.m_V[11] = x.m_V[11] || y.m_V[11];
    x.m_V[12] = x.m_V[12] || y.m_V[12];
    x.m_V[13] = x.m_V[13] || y.m_V[13];
    x.m_V[14] = x.m_V[14] || y.m_V[14];
    x.m_V[15] = x.m_V[15] || y.m_V[15];
    #endif
}
    




/// Vec16Bool & VBool logical and operator
NOALIAS __forceinline const Vec16Bool operator & ( const Vec16Bool PCREF x, const VBool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( x.m_V, (vector unsigned char)y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( x.m_V, (vector bool char)y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_si128( x.m_V, (__m128i&)y.m_V );
    #else
    result.m_V[0]  = x.m_V[0]  && y.m_V;
    result.m_V[1]  = x.m_V[1]  && y.m_V;
    result.m_V[2]  = x.m_V[2]  && y.m_V;
    result.m_V[3]  = x.m_V[3]  && y.m_V;
    result.m_V[4]  = x.m_V[4]  && y.m_V;
    result.m_V[5]  = x.m_V[5]  && y.m_V;
    result.m_V[6]  = x.m_V[6]  && y.m_V;
    result.m_V[7]  = x.m_V[7]  && y.m_V;
    result.m_V[8]  = x.m_V[8]  && y.m_V;
    result.m_V[9]  = x.m_V[9]  && y.m_V;
    result.m_V[10] = x.m_V[10] && y.m_V;
    result.m_V[11] = x.m_V[11] && y.m_V;
    result.m_V[12] = x.m_V[12] && y.m_V;
    result.m_V[13] = x.m_V[13] && y.m_V;
    result.m_V[14] = x.m_V[14] && y.m_V;
    result.m_V[15] = x.m_V[15] && y.m_V;
    #endif
    return result;
}

/// VBool & Vec16Bool logical and operator
NOALIAS __forceinline const Vec16Bool operator & ( const VBool PCREF x, const Vec16Bool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_and( (vector unsigned char)x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_and( (vector bool char)x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_and_si128( (__m128i&)x.m_V, y.m_V );
    #else
    result.m_V[0]  = x.m_V && y.m_V[0];
    result.m_V[1]  = x.m_V && y.m_V[1];
    result.m_V[2]  = x.m_V && y.m_V[2];
    result.m_V[3]  = x.m_V && y.m_V[3];
    result.m_V[4]  = x.m_V && y.m_V[4];
    result.m_V[5]  = x.m_V && y.m_V[5];
    result.m_V[6]  = x.m_V && y.m_V[6];
    result.m_V[7]  = x.m_V && y.m_V[7];
    result.m_V[8]  = x.m_V && y.m_V[8];
    result.m_V[9]  = x.m_V && y.m_V[9];
    result.m_V[10] = x.m_V && y.m_V[10];
    result.m_V[11] = x.m_V && y.m_V[11];
    result.m_V[12] = x.m_V && y.m_V[12];
    result.m_V[13] = x.m_V && y.m_V[13];
    result.m_V[14] = x.m_V && y.m_V[14];
    result.m_V[15] = x.m_V && y.m_V[15];
    #endif
    return result;
}

/// Vec16Bool & VBool logical or operator
NOALIAS __forceinline const Vec16Bool operator | ( const Vec16Bool PCREF x, const VBool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( x.m_V, (vector unsigned char)y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( x.m_V, (vector bool char)y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_si128( x.m_V, (__m128i&)y.m_V );
    #else
    result.m_V[0]  = x.m_V[0]  || y.m_V;
    result.m_V[1]  = x.m_V[1]  || y.m_V;
    result.m_V[2]  = x.m_V[2]  || y.m_V;
    result.m_V[3]  = x.m_V[3]  || y.m_V;
    result.m_V[4]  = x.m_V[4]  || y.m_V;
    result.m_V[5]  = x.m_V[5]  || y.m_V;
    result.m_V[6]  = x.m_V[6]  || y.m_V;
    result.m_V[7]  = x.m_V[7]  || y.m_V;
    result.m_V[8]  = x.m_V[8]  || y.m_V;
    result.m_V[9]  = x.m_V[9]  || y.m_V;
    result.m_V[10] = x.m_V[10] || y.m_V;
    result.m_V[11] = x.m_V[11] || y.m_V;
    result.m_V[12] = x.m_V[12] || y.m_V;
    result.m_V[13] = x.m_V[13] || y.m_V;
    result.m_V[14] = x.m_V[14] || y.m_V;
    result.m_V[15] = x.m_V[15] || y.m_V;
    #endif
    return result;
}

/// VBool & Vec16Bool logical or operator
NOALIAS __forceinline const Vec16Bool operator | ( const VBool PCREF x, const Vec16Bool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_or( (vector unsigned char)x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_or( (vector bool char)x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_or_si128( (__m128i&)x.m_V, y.m_V );
    #else
    result.m_V[0]  = x.m_V || y.m_V[0];
    result.m_V[1]  = x.m_V || y.m_V[1];
    result.m_V[2]  = x.m_V || y.m_V[2];
    result.m_V[3]  = x.m_V || y.m_V[3];
    result.m_V[4]  = x.m_V || y.m_V[4];
    result.m_V[5]  = x.m_V || y.m_V[5];
    result.m_V[6]  = x.m_V || y.m_V[6];
    result.m_V[7]  = x.m_V || y.m_V[7];
    result.m_V[8]  = x.m_V || y.m_V[8];
    result.m_V[9]  = x.m_V || y.m_V[9];
    result.m_V[10] = x.m_V || y.m_V[10];
    result.m_V[11] = x.m_V || y.m_V[11];
    result.m_V[12] = x.m_V || y.m_V[12];
    result.m_V[13] = x.m_V || y.m_V[13];
    result.m_V[14] = x.m_V || y.m_V[14];
    result.m_V[15] = x.m_V || y.m_V[15];
    #endif
    return result;
}

/// Vec16Bool & VBool logical xor operator
NOALIAS __forceinline const Vec16Bool operator ^ ( const Vec16Bool PCREF x, const VBool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( x.m_V, (vector unsigned char)y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( x.m_V, (vector bool char)y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_si128( x.m_V, (__m128i&)y.m_V );
    #else
    result.m_V[0]  = x.m_V[0]  != y.m_V;
    result.m_V[1]  = x.m_V[1]  != y.m_V;
    result.m_V[2]  = x.m_V[2]  != y.m_V;
    result.m_V[3]  = x.m_V[3]  != y.m_V;
    result.m_V[4]  = x.m_V[4]  != y.m_V;
    result.m_V[5]  = x.m_V[5]  != y.m_V;
    result.m_V[6]  = x.m_V[6]  != y.m_V;
    result.m_V[7]  = x.m_V[7]  != y.m_V;
    result.m_V[8]  = x.m_V[8]  != y.m_V;
    result.m_V[9]  = x.m_V[9]  != y.m_V;
    result.m_V[10] = x.m_V[10] != y.m_V;
    result.m_V[11] = x.m_V[11] != y.m_V;
    result.m_V[12] = x.m_V[12] != y.m_V;
    result.m_V[13] = x.m_V[13] != y.m_V;
    result.m_V[14] = x.m_V[14] != y.m_V;
    result.m_V[15] = x.m_V[15] != y.m_V;
    #endif
    return result;
}

/// VBool & Vec16Bool logical xor operator
NOALIAS __forceinline const Vec16Bool operator ^ ( const VBool PCREF x, const Vec16Bool PCREF y )
{
    Vec16Bool result;
    #if defined( _SPU_ )
    result.m_V = spu_xor( (vector unsigned char)x.m_V, y.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_xor( (vector bool char)x.m_V, y.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vxor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_xor_si128( (__m128i&)x.m_V, y.m_V );
    #else
    result.m_V[0]  = x.m_V != y.m_V[0];
    result.m_V[1]  = x.m_V != y.m_V[1];
    result.m_V[2]  = x.m_V != y.m_V[2];
    result.m_V[3]  = x.m_V != y.m_V[3];
    result.m_V[4]  = x.m_V != y.m_V[4];
    result.m_V[5]  = x.m_V != y.m_V[5];
    result.m_V[6]  = x.m_V != y.m_V[6];
    result.m_V[7]  = x.m_V != y.m_V[7];
    result.m_V[8]  = x.m_V != y.m_V[8];
    result.m_V[9]  = x.m_V != y.m_V[9];
    result.m_V[10] = x.m_V != y.m_V[10];
    result.m_V[11] = x.m_V != y.m_V[11];
    result.m_V[12] = x.m_V != y.m_V[12];
    result.m_V[13] = x.m_V != y.m_V[13];
    result.m_V[14] = x.m_V != y.m_V[14];
    result.m_V[15] = x.m_V != y.m_V[15];
    #endif
    return result;
}

/// Vec16Bool & VBool logical and assignment operator
NOALIAS __forceinline void operator &= ( Vec16Bool& x, const VBool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_and( x.m_V, (vector unsigned char)y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_and( x.m_V, (vector bool char)y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vand( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_and_si128( x.m_V, (__m128i&)y.m_V );
    #else
    x.m_V[0]  = x.m_V[0]  && y.m_V;
    x.m_V[1]  = x.m_V[1]  && y.m_V;
    x.m_V[2]  = x.m_V[2]  && y.m_V;
    x.m_V[3]  = x.m_V[3]  && y.m_V;
    x.m_V[4]  = x.m_V[4]  && y.m_V;
    x.m_V[5]  = x.m_V[5]  && y.m_V;
    x.m_V[6]  = x.m_V[6]  && y.m_V;
    x.m_V[7]  = x.m_V[7]  && y.m_V;
    x.m_V[8]  = x.m_V[8]  && y.m_V;
    x.m_V[9]  = x.m_V[9]  && y.m_V;
    x.m_V[10] = x.m_V[10] && y.m_V;
    x.m_V[11] = x.m_V[11] && y.m_V;
    x.m_V[12] = x.m_V[12] && y.m_V;
    x.m_V[13] = x.m_V[13] && y.m_V;
    x.m_V[14] = x.m_V[14] && y.m_V;
    x.m_V[15] = x.m_V[15] && y.m_V;
    #endif
}

/// Vec16Bool & VBool logical or assignment operator
NOALIAS __forceinline void operator |= ( Vec16Bool& x, const VBool PCREF y )
{
    #if defined( _SPU_ )
    x.m_V = spu_or( x.m_V, (vector unsigned char)y.m_V );
    #elif defined( _PPU_ )
    x.m_V = vec_or( x.m_V, (vector bool char)y.m_V );
    #elif defined( _VMX128_ )
    x.m_V = __vor( x.m_V, y.m_V );
    #elif defined( _SSE_ )
    x.m_V = _mm_or_si128( x.m_V, (__m128i&)y.m_V );
    #else
    x.m_V[0]  = x.m_V[0]  || y.m_V;
    x.m_V[1]  = x.m_V[1]  || y.m_V;
    x.m_V[2]  = x.m_V[2]  || y.m_V;
    x.m_V[3]  = x.m_V[3]  || y.m_V;
    x.m_V[4]  = x.m_V[4]  || y.m_V;
    x.m_V[5]  = x.m_V[5]  || y.m_V;
    x.m_V[6]  = x.m_V[6]  || y.m_V;
    x.m_V[7]  = x.m_V[7]  || y.m_V;
    x.m_V[8]  = x.m_V[8]  || y.m_V;
    x.m_V[9]  = x.m_V[9]  || y.m_V;
    x.m_V[10] = x.m_V[10] || y.m_V;
    x.m_V[11] = x.m_V[11] || y.m_V;
    x.m_V[12] = x.m_V[12] || y.m_V;
    x.m_V[13] = x.m_V[13] || y.m_V;
    x.m_V[14] = x.m_V[14] || y.m_V;
    x.m_V[15] = x.m_V[15] || y.m_V;
    #endif
}


/// @}
}; // namespace Maths

#endif // #if defined( INCLUDE_THIS )


