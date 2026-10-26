#ifndef MATHS_HVEC_SCALARINT_INL
#define MATHS_HVEC_SCALARINT_INL

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include "SIMD.h"
#endif

namespace Maths {

template<typename T>
NOALIAS __forceinline
IntInVec<T>::IntInVec(const T& src)

	#if defined( _PPU_ )
    : m_Vec( vec_vcfsx( vec_vspltw( vec_load_left_aligned( &src ), VX ), 0 ) )

   #elif defined( _VMX128_ ) || defined( _SSE_ )
    : m_Vec( SIMD_Scalar( src ) )

   #elif !defined( _SIMD_ )
    : m_Value( (T)src )

   #endif
{
}


template<typename T>
NOALIAS __forceinline IntInVec<T>::operator const T() const __restrict
{
    #if defined( _PPU_ )
    T value;
    vec_stvewx( m_F, 0x0, &value );
    return value;

    #elif defined( _VMX128_ )
    T value;
    __stvewx( m_Vec, &value, 0x0 );
    return value;

    #elif defined( _SIMD_ )
    return SIMD_ScalarToF32( m_Vec );

    #else
    return m_Value;

    #endif
}


template<typename T>
NOALIAS __forceinline 
IntInVec<T> operator & (const IntInVec<T>& src1, const IntInVec<T>& src2)
{
	#if defined(_VMX128_) 
	return	IntInVec<T>( __vand(src1.m_Vec, src2.m_Vec)); 

	#elif defined(PLATFORM_PPU) 

	return	IntInVec<T>( vec_and(src1.m_Vec, src2.m_Vec));

	#elif defined(_SIMD_)

	return	IntInVec<T>(_mm_and_ps(src1.m_Vec, src2.m_Vec) );

	#else
	return	IntInVec(src1.m_Value & src2.m_Value);
	#endif
}

// this is crying out for a Hash define :)
template<typename T>
NOALIAS __forceinline 
IntInVec<T> operator | (const IntInVec<T>& src1, const IntInVec<T>& src2)
{
	#if defined(_VMX128_) 
	return	IntInVec<T>( __vor(src1.m_Vec, src2.m_Vec) );

	#elif defined(PLATFORM_PPU) 

	return	IntInVec<T>( vec_or(src1.m_Vec, src2.m_Vec) );

	#elif defined(_SIMD_)

	return	IntInVec<T>(_mm_or_ps(src1.m_Vec, src2.m_Vec) );

	#else
	return	IntInVec<T>(src1.m_Value | src2.m_Value);
	#endif
}


template<typename T>
NOALIAS __forceinline 
IntInVec<T> operator + (const IntInVec<T>& src1, const IntInVec<T>& src2)
{
	#if defined(_VMX128_) 
	return	IntInVec<T>( __vadduwm(src1.m_Vec, src2.m_Vec));

	#elif defined(PLATFORM_PPU) 

	return	IntInVec<T>( vec_adduwm(src1.m_Vec, src2.m_Vec));

	#elif defined(_SIMD_)

	return	IntInVec<T>(_mm_add_epi32(src1.m_Vec, src2.m_Vec) );

	#else
	return	IntInVec(src1.m_Value + src2.m_Value);
	#endif
}

// this is crying out for a Hash define :)
template<typename T>
NOALIAS __forceinline 
IntInVec<T> operator - (const IntInVec<T>& src1, const IntInVec<T>& src2)
{
	#if defined(_VMX128_) 
	return	IntInVec( __vsubuwm(src1.m_Vec, src2.m_Vec));

	#elif defined(PLATFORM_PPU) 

	return	IntInVec<T>( vec_subuwm(src1.m_Vec, src2.m_Vec));

	#elif defined(_SIMD_)

	return	IntInVec<T>(_mm_sub_epi32(src1.m_Vec, src2.m_Vec) );

	#else
	return	IntInVec<T>(src1.m_Value - src2.m_Value);
	#endif
}


////////////////////////////////////////////////////////////////
//
// assignment operators
//


template<typename T>
NOALIAS __forceinline 
IntInVec<T> operator &= (IntInVec<T>& src1, const IntInVec<T>& src2)
{
	#if defined(_VMX128_) 				
	src1.m_Vec = __vand(src1.m_Vec, src2.m_Vec);

	#elif defined(PLATFORM_PPU) 

	src1.m_Vec= vec_and(src1.m_Vec, src2.m_Vec);

	#elif defined(_SIMD_)

	src1.m_Vec = _mm_and_ps(src1.m_Vec, src2.m_Vec);

	#else
	src1.m_Value & src2.m_Value;
	#endif

	return	src1;
}


// this is crying out for a Hash define :)
template<typename T>
NOALIAS __forceinline 
IntInVec<T> operator |= (IntInVec<T>& src1, const IntInVec<T>& src2)
{
	#if defined(_VMX128_) 
	src1.m_Vec= __vor(src1.m_Vec, src2.m_Vec);

	#elif defined(PLATFORM_PPU) 

	src1.m_Vec =vec_or(src1.m_Vec, src2.m_Vec);

	#elif defined(_SIMD_)

	src1.m_Vec = _mm_or_ps(src1.m_Vec, src2.m_Vec);

	#else
	src1.m_Value |= src2.m_Value);
	#endif
	return	src1;
}					 


////////////////////////////////////////////////////////////////
//
// int comparisons
//

template<typename T>
NOALIAS __forceinline 
IntInVecBool operator > (const IntInVec<T>& src1, const IntInVec<T>& src2)
{
	#if defined(_VMX128_) 
	return	IntInVec( __vcmpgtuw(src1.m_Vec, src2.m_Vec));

	#elif defined(PLATFORM_PPU) 
	return	IntInVec( vec_cmpgtuw(src1.m_Vec, src2.m_Vec));

	#elif defined(_SIMD_)
	return	IntInVec(_mm_cmpgt_epi32(src1.m_Vec, src2.m_Vec) );

	#else
		return	src1.m_Value > src2.m_Value;
	#endif
}

template<typename T>
NOALIAS __forceinline 
IntInVecBool operator == (const IntInVec<T>& src1, const IntInVec<T>& src2)
{
	#if defined(_VMX128_) 
	return	IntInVec( __vcmpequw(src1.m_Vec, src2.m_Vec));

	#elif defined(PLATFORM_PPU) 
	return	IntInVec( vec_cmpequw(src1.m_Vec, src2.m_Vec));

	#elif defined(_SIMD_)
	return	IntInVec(_mm_cmpeq_epi32(src1.m_Vec, src2.m_Vec) );

	#else
		return	src1.m_Value == src2.m_Value;
	#endif
}








}

#endif
