#ifndef MATHS_HVEC_VECTORINT_INL
#define MATHS_HVEC_VECTORINT_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Maths
{

///////////////////////////////////////////////////////////////////
//
// Scalar_U32 & Scalar_S32 Non-Member Friend Functions
//

template <>
NOALIAS __forceinline const Scalar_U32 Load( U32 const * const  __restrict ptr )
{
    Scalar_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( *ptr );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( (vector unsigned int)vec_lvlx( 0x0, ptr ), VX );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( ptr, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_epi32( *ptr );
    #else
    result.m_V = *ptr; // TODO: SSE VERSION!
    #endif
    return result;
}

template <>
NOALIAS __forceinline const Scalar_S32 Load( S32 const * const  __restrict ptr )
{
    Scalar_S32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( *ptr );
    #elif defined( _PPU_ )
    result.m_V = vec_splat( (vector signed int)vec_lvlx( 0x0, ptr ), VX );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( ptr, 0x0 ), VX );
    #elif defined( _SSE_ )
    result.m_V = _mm_set1_epi32( *ptr );
    #else
    result.m_V = *ptr; // TODO: SSE VERSION!
    #endif
    return result;
}

template <>
NOALIAS __forceinline const Vector3_U32 Load( U32 const * const __restrict ptr )
{
	using namespace Maths;
    Vector3_U32 result;
    #if defined( _VMX128_ )
    result.m_V = __vrlimi( __loadunalignedvector( ptr ), __vzero(), VMX_MASK_INSERT_W, 0 );
    #elif defined( _SSE_ )
    //result.m_V = _mm_setr_epi32( ptr[0], ptr[1], ptr[2], 0 );
    result.m_V = _mm_and_si128( _mm_loadu_si128( (__m128i*)ptr ), (__m128i&)SIMD_Mask<1,1,1,0>() );
    #else
	result.m_V[0] = ptr[0]; result.m_V[1] = ptr[1]; result.m_V[2] = ptr[2];
    #endif
    return result;
}

/// Store HVec3 into a Float3
//template <> 
NOALIAS __forceinline void Store( const Maths::Vector3_U32 PCREF src, U32* const __restrict pDst )
{
    using namespace Maths;
    #if defined( _VMX128_ )
    ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VX ), pDst, 0x00 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VY ), pDst, 0x04 ); ReadWriteBarrier();
    __stvewx( __vspltw( src.m_V, VZ ), pDst, 0x08 ); ReadWriteBarrier();
    #elif defined ( _SSE_ )
    //_mm_store_ss( (float*)&pDst[0], (__m128&)src.m_V );
    //_mm_store_ss( (float*)&pDst[1], (__m128&)sse_splat_int32<VY>( src.m_V ) );
    //_mm_store_ss( (float*)&pDst[2], (__m128&)sse_splat_int32<VZ>( src.m_V ) );
    _mm_maskmoveu_si128( src.m_V, (__m128i&)SIMD_Mask<1,1,1,0>(), (char*)pDst );
    _mm_mfence();
    #else
    pDst[0] = src.m_V[0];
    pDst[1] = src.m_V[1];
    pDst[2] = src.m_V[2];
    #endif
}

//template <>
NOALIAS __forceinline void Store( const Scalar_U32 PCREF scalar, U32* const __restrict ptr )
{
    #if defined( _SPU_ )
    *ptr = spu_extract( scalar.m_V, VX );
    #elif defined( _PPU_ )
    vec_ste( scalar.m_V, 0x0, ptr );
    #elif defined( _VMX128_ )
    __stvewx( scalar.m_V, ptr, 0x0 );
    #elif defined ( _SSE_ )
    _mm_store_ss( (float*)ptr, (__m128&)scalar.m_V );
    #else
    *ptr = scalar.m_V; // TODO: SSE VERSION!
    #endif
}

//template <>
NOALIAS __forceinline void Store( const Scalar_S32 PCREF scalar, S32* const __restrict ptr )
{
    #if defined( _SPU_ )
    *ptr = spu_extract( scalar.m_V, VX );
    #elif defined( _PPU_ )
    vec_ste( scalar.m_V, 0x0, ptr );
    #elif defined( _VMX128_ )
    __stvewx( scalar.m_V, ptr, 0x0 );
    #elif defined ( _SSE_ )
    _mm_store_ss( (float*)ptr, (__m128&)scalar.m_V );
    #else
    *ptr = scalar.m_V; // TODO: SSE VERSION!
    #endif
}

/// Make Scalar_U32 from 1 U32 splatted into all elements
NOALIAS __forceinline const Scalar_U32 Make_Scalar_U32( const U32& i )
{
    Scalar_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( i );
    #elif defined( _PPU_ )
    result.m_V = vec_splat_u32( i );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( &i, 0x0 ), VX );
    #elif defined ( _SSE_ )
    result.m_V = _mm_set1_epi32( i );
    #else
    result.m_V = i; // TODO: SSE VERSION!
    #endif
    return result;
}

/// Make Scalar_S32 from 1 S32 splatted into all elements
NOALIAS __forceinline const Scalar_S32 Make_Scalar_S32( const S32& i )
{
    Scalar_S32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( i );
    #elif defined( _PPU_ )
    result.m_V = vec_splat_s32( i );
    #elif defined( _VMX128_ )
    result.m_V = __vspltw( __lvlx( &i, 0x0 ), VX );
    #elif defined ( _SSE_ )
    result.m_V = _mm_set1_epi32( i );
    #else
    result.m_V = i; // TODO: SSE VERSION!
    #endif
    return result;
}

/// Make Scalar_U32 from 1 U32 splatted into all elements
template < U32 I > NOALIAS __forceinline const Scalar_U32 Make_Scalar_U32()
{
    Scalar_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( I );
    #elif defined( _PPU_ )
    result.m_V = vec_splat_u32( I );
    #elif defined( _VMX128_ )
    if ( I <= 15 )
    {
        result.m_V = __vspltisw( I );
    }
    else
    {
        result.m_V = __vspltw( __lvlx( &Const_U32<I>::m_Value, 0x0 ), VX );
    }
    #else
    result.m_V = I; // TODO: SSE VERSION!
    #endif
    return result;
}

/// Make Scalar_U32 from 1 U32 splatted into all elements
template < S32 I > NOALIAS __forceinline const Scalar_S32 Make_Scalar_S32()
{
    Scalar_S32 result;
    #if defined( _SPU_ )
    result.m_V = spu_splats( I );
    #elif defined( _PPU_ )
    result.m_V = vec_splat_s32( I );
    #elif defined( _VMX128_ )
    if ( ( I >= -16 ) && ( I <= 15 ) )
    {
        result.m_V = __vspltisw( I );
    }
    else
    {
        result.m_V = __vspltw( __lvlx( &Const_S32<I>::m_Value, 0x0 ), VX );
    }
    #else
    result.m_V = I; // TODO: SSE VERSION!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_U32 operator ++ ( Scalar_U32 & scalar ) // ++Prefix
{
    #if defined( _SPU_ )
    scalar.m_V = spu_add( scalar.m_V, spu_splats( (U32)1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_add( scalar.m_V, vec_splat_u32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vadduwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_add_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V++; // TODO: SPU & SSE VERSIONS!
    #endif
    const Scalar_U32 result = scalar;
    return result;
}

NOALIAS __forceinline const Scalar_S32 operator ++ ( Scalar_S32 & scalar ) // ++Prefix
{
    #if defined( _SPU_ )
    scalar.m_V = spu_add( scalar.m_V, spu_splats( 1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_add( scalar.m_V, vec_splat_s32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vadduwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_add_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V++; // TODO: SPU & SSE VERSIONS!
    #endif
    const Scalar_S32 result = scalar;
    return result;
}

NOALIAS __forceinline const Vector3_U32 operator ++ ( Vector3_U32 & scalar ) // ++Prefix
{
    #if defined( _SPU_ )
    scalar.m_V = spu_add( scalar.m_V, spu_splats( 1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_add( scalar.m_V, vec_splat_s32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vadduwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_add_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V[0]++; // TODO: SPU & SSE VERSIONS!
	scalar.m_V[1]++; // TODO: SPU & SSE VERSIONS!
	scalar.m_V[2]++; // TODO: SPU & SSE VERSIONS!
    #endif
    const Vector3_U32 result = scalar;
    return result;
}

NOALIAS __forceinline const Scalar_U32 operator ++ ( Scalar_U32 & scalar, int ) // Postfix++
{
    const Scalar_U32 result = scalar;
    #if defined( _SPU_ )
    scalar.m_V = spu_add( scalar.m_V, spu_splats( (U32)1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_add( scalar.m_V, vec_splat_u32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vadduwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_add_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V++; // TODO: SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_S32 operator ++ ( Scalar_S32 & scalar, int ) // Postfix++
{
    const Scalar_S32 result = scalar;
    #if defined( _SPU_ )
    scalar.m_V = spu_add( scalar.m_V, spu_splats( 1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_add( scalar.m_V, vec_splat_s32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vadduwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_add_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V++; // TODO: SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Vector3_U32 operator ++ ( Vector3_U32 & scalar, int ) // Postfix++
{
    const Vector3_U32 result = scalar;
    #if defined( _SPU_ )
    scalar.m_V = spu_add( scalar.m_V, spu_splats( 1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_add( scalar.m_V, vec_splat_s32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vadduwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_add_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V[0]++; // TODO: SPU & SSE VERSIONS!
	scalar.m_V[1]++; // TODO: SPU & SSE VERSIONS!
	scalar.m_V[2]++; // TODO: SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_U32 operator -- ( Scalar_U32 & scalar ) // --Prefix
{
    #if defined( _SPU_ )
    scalar.m_V = spu_sub( scalar.m_V, spu_splats( (U32)1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_sub( scalar.m_V, vec_splat_u32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vsubuwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_sub_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V--; // TODO: SPU & SSE VERSIONS!
    #endif
    const Scalar_U32 result = scalar;
    return result;
}

NOALIAS __forceinline const Scalar_S32 operator -- ( Scalar_S32 & scalar ) // --Prefix
{
    #if defined( _SPU_ )
    scalar.m_V = spu_sub( scalar.m_V, spu_splats( 1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_sub( scalar.m_V, vec_splat_s32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vsubuwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_sub_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V--; // TODO: SPU & SSE VERSIONS!
    #endif
    const Scalar_S32 result = scalar;
    return result;
}

NOALIAS __forceinline const Vector3_U32 operator -- ( Vector3_U32 & scalar ) // --Prefix
{
    #if defined( _SPU_ )
    scalar.m_V = spu_sub( scalar.m_V, spu_splats( 1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_sub( scalar.m_V, vec_splat_s32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vsubuwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_sub_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V[0]--; // TODO: SPU & SSE VERSIONS!
	scalar.m_V[1]--; // TODO: SPU & SSE VERSIONS!
	scalar.m_V[2]--; // TODO: SPU & SSE VERSIONS!
    #endif
    const Vector3_U32 result = scalar;
    return result;
}

NOALIAS __forceinline const Scalar_U32 operator -- ( Scalar_U32 & scalar, int ) // Postfix--
{
    const Scalar_U32 result = scalar;
    #if defined( _SPU_ )
    scalar.m_V = spu_sub( scalar.m_V, spu_splats( (U32)1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_sub( scalar.m_V, vec_splat_u32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vsubuwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_sub_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V--; // TODO: SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_S32 operator -- ( Scalar_S32 & scalar, int ) // Postfix--
{
    const Scalar_S32 result = scalar;
    #if defined( _SPU_ )
    scalar.m_V = spu_sub( scalar.m_V, spu_splats( 1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_sub( scalar.m_V, vec_splat_s32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vsubuwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_sub_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V--; // TODO: SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Vector3_U32 operator -- ( Vector3_U32 & scalar, int ) // Postfix--
{
    const Vector3_U32 result = scalar;
    #if defined( _SPU_ )
    scalar.m_V = spu_sub( scalar.m_V, spu_splats( 1 ) );
    #elif defined( _PPU_ )
    scalar.m_V = vec_sub( scalar.m_V, vec_splat_s32( 1 ) );
    #elif defined( _VMX128_ )
    scalar.m_V = __vsubuwm( scalar.m_V, __vspltisw( 1 ) );
    #elif defined( _SSE_ )
    scalar.m_V = _mm_sub_epi32( scalar.m_V, _mm_set1_epi32( 1 ) );
    #else
    scalar.m_V[0]--; // TODO: SPU & SSE VERSIONS!
	scalar.m_V[1]--; // TODO: SPU & SSE VERSIONS!
	scalar.m_V[2]--; // TODO: SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_U32 operator += ( Scalar_U32 & src0, const Scalar_U32 PCREF src1 )
{
    #if defined( _SPU_ )
    src0.m_V = spu_add( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    src0.m_V = vec_add( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    src0.m_V = __vadduwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    src0.m_V = _mm_add_epi32( src0.m_V, src1.m_V );
    #else
    src0.m_V = src0.m_V + src1.m_V; // TODO: SPU & SSE VERSIONS!
    #endif
    return src0;
}

NOALIAS __forceinline const Scalar_S32 operator += ( Scalar_S32 & src0, const Scalar_S32 PCREF src1 )
{
    #if defined( _SPU_ )
    src0.m_V = spu_add( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    src0.m_V = vec_add( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    src0.m_V = __vadduwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    src0.m_V = _mm_add_epi32( src0.m_V, src1.m_V );
    #else
    src0.m_V = src0.m_V + src1.m_V; // TODO: SPU & SSE VERSIONS!
    #endif
    return src0;
}

NOALIAS __forceinline const Vector3_U32 operator += ( Vector3_U32 & src0, const Vector3_U32 PCREF src1 )
{
    #if defined( _SPU_ )
    src0.m_V = spu_add( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    src0.m_V = vec_add( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    src0.m_V = __vadduwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    src0.m_V = _mm_add_epi32( src0.m_V, src1.m_V );
    #else
    src0.m_V [0]= src0.m_V[0] + src1.m_V[0]; // TODO: SPU & SSE VERSIONS!
	src0.m_V [1]= src0.m_V[1] + src1.m_V[1]; // TODO: SPU & SSE VERSIONS!
	src0.m_V [2]= src0.m_V[2] + src1.m_V[2]; // TODO: SPU & SSE VERSIONS!
    #endif
    return src0;
}

NOALIAS __forceinline const Scalar_U32 operator -= ( Scalar_U32 & src0, const Scalar_U32 PCREF src1 )
{
    #if defined( _SPU_ )
    src0.m_V = spu_sub( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    src0.m_V = vec_sub( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    src0.m_V = __vsubuwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    src0.m_V = _mm_sub_epi32( src0.m_V, src1.m_V );
    #else
    src0.m_V = src0.m_V - src1.m_V; // TODO: SPU & SSE VERSIONS!
    #endif
    return src0;
}

NOALIAS __forceinline const Scalar_S32 operator -= ( Scalar_S32 & src0, const Scalar_S32 PCREF src1 )
{
    #if defined( _SPU_ )
    src0.m_V = spu_sub( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    src0.m_V = vec_sub( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    src0.m_V = __vsubuwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    src0.m_V = _mm_sub_epi32( src0.m_V, src1.m_V );
    #else
    src0.m_V = src0.m_V - src1.m_V; // TODO: SPU & SSE VERSIONS!
    #endif
    return src0;
}

NOALIAS __forceinline const Vector3_U32 operator -= ( Vector3_U32 & src0, const Vector3_U32 PCREF src1 )
{
    #if defined( _SPU_ )
    src0.m_V = spu_sub( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    src0.m_V = vec_sub( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    src0.m_V = __vsubuwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    src0.m_V = _mm_sub_epi32( src0.m_V, src1.m_V );
    #else
    src0.m_V[0] = src0.m_V[0] - src1.m_V[0]; // TODO: SPU & SSE VERSIONS!
	src0.m_V[1] = src0.m_V[1] - src1.m_V[1]; // TODO: SPU & SSE VERSIONS!
	src0.m_V[2] = src0.m_V[2] - src1.m_V[2]; // TODO: SPU & SSE VERSIONS!
    #endif
    return src0;
}

NOALIAS __forceinline const Scalar_U32 operator + ( const Scalar_U32 PCREF src0, const Scalar_U32 PCREF src1 )
{
    Scalar_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_add( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_add( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vadduwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_epi32( src0.m_V, src1.m_V );
    #else
    result.m_V = src0.m_V + src1.m_V; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_S32 operator + ( const Scalar_S32 PCREF src0, const Scalar_S32 PCREF src1 )
{
    Scalar_S32 result;
    #if defined( _SPU_ )
    result.m_V = spu_add( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_add( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vadduwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_epi32( src0.m_V, src1.m_V );
    #else
    result.m_V = src0.m_V + src1.m_V; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Vector3_U32 operator + ( const Vector3_U32 PCREF src0, const Vector3_U32 PCREF src1 )
{
    Vector3_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_add( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_add( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vadduwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_add_epi32( src0.m_V, src1.m_V );
    #else
    result.m_V[0] = src0.m_V[0] + src1.m_V[0]; // TODO: PPU, SPU & SSE VERSIONS!
	result.m_V[1] = src0.m_V[1] + src1.m_V[1]; // TODO: PPU, SPU & SSE VERSIONS!
	result.m_V[2] = src0.m_V[2] + src1.m_V[2]; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_U32 operator - ( const Scalar_U32 PCREF src0, const Scalar_U32 PCREF src1 )
{
    Scalar_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sub( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sub( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsubuwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_epi32( src0.m_V, src1.m_V );
    #else
    result.m_V = src0.m_V - src1.m_V; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_S32 operator - ( const Scalar_S32 PCREF src0, const Scalar_S32 PCREF src1 )
{
    Scalar_S32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sub( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sub( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsubuwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_epi32( src0.m_V, src1.m_V );
    #else
    result.m_V = src0.m_V - src1.m_V; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Vector3_U32 operator - ( const Vector3_U32 PCREF src0, const Vector3_U32 PCREF src1 )
{
    Vector3_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sub( src0.m_V, src1.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sub( src0.m_V, src1.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsubuwm( src0.m_V, src1.m_V );
    #elif defined( _SSE_ )
    result.m_V = _mm_sub_epi32( src0.m_V, src1.m_V );
    #else
    result.m_V[0] = src0.m_V[0] - src1.m_V[0]; // TODO: PPU, SPU & SSE VERSIONS!
	result.m_V[1] = src0.m_V[1] - src1.m_V[1]; // TODO: PPU, SPU & SSE VERSIONS!
	result.m_V[2] = src0.m_V[2] - src1.m_V[2]; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_U32 Select( const Scalar_Bool PCREF condition, const Scalar_U32 PCREF resultIfTrue, const Scalar_U32 PCREF resultIfFalse )
{
    Scalar_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_select( resultIfFalse.m_V, resultIfTrue.m_V, (__m128i&)condition.m_V );
    #else
    result.m_V = condition.m_F ? resultIfTrue.m_V : resultIfFalse.m_V; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_S32 Select( const Scalar_Bool PCREF condition, const Scalar_S32 PCREF resultIfTrue, const Scalar_S32 PCREF resultIfFalse )
{
    Scalar_S32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_select( resultIfFalse.m_V, resultIfTrue.m_V, (__m128i&)condition.m_V );
    #else
    result.m_V = condition.m_F ? resultIfTrue.m_V : resultIfFalse.m_V; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Vector3_U32 Select( const Vector3_Bool PCREF condition, const Vector3_U32 PCREF resultIfTrue, const Vector3_U32 PCREF resultIfFalse )
{
    Vector3_U32 result;
    #if defined( _SPU_ )
    result.m_V = spu_sel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _PPU_ )
    result.m_V = vec_sel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _VMX128_ )
    result.m_V = __vsel( resultIfFalse.m_V, resultIfTrue.m_V, condition.m_V );
    #elif defined( _SSE_ )
    result.m_V = sse_select( resultIfFalse.m_V, resultIfTrue.m_V, (__m128i&)condition.m_V );
    #else
    result.m_V[0] = condition.m_X ? resultIfTrue.m_V[0] : resultIfFalse.m_V[0]; // TODO: PPU, SPU & SSE VERSIONS!
	result.m_V[1] = condition.m_Y ? resultIfTrue.m_V[1] : resultIfFalse.m_V[1]; // TODO: PPU, SPU & SSE VERSIONS!
	result.m_V[2] = condition.m_Z ? resultIfTrue.m_V[2] : resultIfFalse.m_V[2]; // TODO: PPU, SPU & SSE VERSIONS!
    #endif
    return result;
}

NOALIAS __forceinline const Scalar_U32 X(const Vector3_U32 PCREF vector)
{
	#if defined( _SPU_ )
    Scalar_U32 result;
    result.m_V = ( vector unsigned int )spu_splat<VX>( vector.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_U32 result;
    result.m_V = (vector unsigned int)vec_splat( vector.m_V, VX );
    return result;
    #elif defined( _VMX128_ )
    Scalar_U32 result;
    result.m_V = __vspltw( vector.m_V, VX );
    return result;
    #elif defined( _SSE_ )
    Scalar_U32 result;
    result.m_V = sse_splat_int32<VX>( vector.m_V );
    return result;
    #else
    Scalar_U32 val;
	val.m_V = vector.m_V[VX];
	return val;
    #endif
}

NOALIAS __forceinline const Scalar_U32 Y(const Vector3_U32 PCREF vector)
{
	#if defined( _SPU_ )
    Scalar_U32 result;
    result.m_V = (vector unsigned int)spu_splat<VY>( vector.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_U32 result;
    result.m_V = (vector unsigned int)vec_splat( vector.m_V, VY );
    return result;
    #elif defined( _VMX128_ )
    Scalar_U32 result;
    result.m_V = __vspltw( vector.m_V, VY );
    return result;
    #elif defined( _SSE_ )
    Scalar_U32 result;
    result.m_V = sse_splat_int32<VY>( vector.m_V );
    return result;
    #else
    Scalar_U32 val;
	val.m_V = vector.m_V[VY];
	return val;
    #endif
}

NOALIAS __forceinline const Scalar_U32 Z(const Vector3_U32 PCREF vector)
{
	#if defined( _SPU_ )
    Scalar_U32 result;
    result.m_V = (vector unsigned int)spu_splat<VZ>( vector.m_V );
    return result;
    #elif defined( _PPU_ )
    Scalar_U32 result;
    result.m_V = (vector unsigned int)vec_splat( vector.m_V, VZ );
    return result;
    #elif defined( _VMX128_ )
    Scalar_U32 result;
    result.m_V = __vspltw( vector.m_V, VZ );
    return result;
    #elif defined( _SSE_ )
    Scalar_U32 result;
    result.m_V = sse_splat_int32<VZ>( vector.m_V );
    return result;
    #else
	Scalar_U32 val;
	val.m_V = vector.m_V[VZ];
	return val;
    #endif
}


}; // namespace Maths

#endif // #ifndef MATHS_HVEC_VECTORINT_INL

