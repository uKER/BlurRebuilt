#ifndef MATHS_HVEC_SCALARINT_H
#define MATHS_HVEC_SCALARINT_H

#ifdef _MSC_VER
#pragma once
#endif

//#ifdef PLATFORM_PS3
#include "SIMD.h"
//#endif

namespace Maths
{

// [TODO - largely cut/paste from HF32 class.]
// [cleanup would involve creating "ScalarInVec"
// [from which derived "IntInVec", "BoolInVec" etc etc ..
//
// goes in vec reg on Xenon,PPU
// stays in scalar reg on SPU
// on SSE, jury's out...
// (following behaviour of F32 for the moment ....)
//

// [codegen on templated types verified.]
// [declspec passinreg works now ? ]
//
//
//
template<typename T>
PASSINREG class ALIGN16 IntInVec
{
public:
    
    /// Default Constructor
    IntInVec()	{		};

    IntInVec( const T&	src );
	
	
	operator const T() const;
   
//private:
public: // For __declspec( passinreg )
    
	#if defined(_VMX128_) || defined(PLATFORM_PPU) || defined(_SIMD_)
	IntInVec( V128 src ) 			{ m_Vec	=	src;	}
    IntInVec( Scalar_Bool& src )	{ m_Vec	=	src.m_V; 	}
    V128                    m_Vec;
	typedef	Scalar_Bool	T_Bool;

	#else
	T	 					m_Value;
	typedef	bool	T_Bool;

	#endif
};
#if defined(_VMX128_) || defined(PLATFORM_PPU) || defined(_SIMD_)
	typedef	Scalar_Bool IntInVecBool;
#else
	typedef	bool	IntInVecBool;
#endif

typedef	IntInVec<U32> HU32;
typedef	IntInVec<S32> HS32;
typedef C_SelectorScalar	HBOOL;

// Operators: bitwise with ScalarBool to allow 
// updating flags etc from vector results
// i.e. (HU32)flags |= (C_ScalarBool)... & E_SomeFlag

//todo , does simd version need a argument constref type

// TODO - add bitshifts, i.e. for building/testing bitfields..
//

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator & (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator | (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator &= (IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator |= (IntInVec<T>& src1, const IntInVec<T>& src2);


template<typename T> NOALIAS __forceinline 
IntInVec<T> operator + (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator - (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator && (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator || (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator & (const IntInVec<T>& src1, const T&	value) {
	return	src1 & IntInVec<T>(value);
}

HU32	operator& (const Scalar_Bool& src1, const U32& src2)
{	return	HU32(src1) & HU32(src2);
};
	

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator | (const IntInVec<T>& src1, const T&	value) {
	return	src1 | IntInVec<T>(value);
}

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator |= (IntInVec<T>& src1, const T&	value) {
	return	src1 |= IntInVec<T>(value);
}

template<typename T> NOALIAS __forceinline 
IntInVec<T> operator &= (IntInVec<T>& src1, const T&	value) {
	return	src1 &= IntInVec<T>(value);
}

template<typename T> NOALIAS __forceinline 
IntInVecBool operator > (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVecBool operator < (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVecBool operator == (const IntInVec<T>& src1, const IntInVec<T>& src2);

template<typename T> NOALIAS __forceinline 
IntInVecBool  operator != (const IntInVec<T>& src1, const IntInVec<T>& src2);













}; // namespace Maths

#include "scalarInt.inl"

#endif
