#ifndef CORE_STRING_HELPERS_H
#define CORE_STRING_HELPERS_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef USE_OLD_UTIL_CSTRING

namespace Util
{
	//generic template for converting integer based numbers to string. itoa limited to 32b numbers. 
	//Extensions are MS only, so a simple version for all types and signs is implemented here. 

	template <typename T>
	C_String unsignedIntegerToCString( T value);
		
	template <typename T>
	C_String signedIntegerToCString( T value);

	template <typename T>
	inline C_String& operator << (C_String& string, const T in);
    
    #if defined( __INTEL_COMPILER )
    
	template <typename T> // JASOND: Had to add this because the intel compiler doesn't allow specialisations of T with & or *
	inline C_String& operator << (C_String& string, const T& in);

	template <typename T> // JASOND: Had to add this because the intel compiler doesn't allow specialisations of T with & or *
	inline C_String& operator << (C_String& string, const T* in);
	
	#endif

	//specialisations for necessary types
	template <>
	inline C_String& operator << (C_String& string, const C_String& in);

	template <> 
	inline C_String& operator << (C_String& string, const C_String in);

	template <>
	inline C_String& operator << (C_String& string, const U8 in);

	template <>
	inline C_String& operator << (C_String& string, const S8 in);

	template <>
	inline C_String& operator << (C_String& string, const U16 in);

	template <>
	inline C_String& operator << (C_String& string, const S16 in);

	template <>
	inline C_String& operator << (C_String& string, const U32 in);

	template <>
	inline C_String& operator << (C_String& string, const S32 in);

	template <>
	inline C_String& operator << (C_String& string, const U64 in);

	template <>
	inline C_String& operator << (C_String& string, const S64 in);

	template <>
	inline C_String& operator << (C_String& string, const F32 in);

	template <>
	inline C_String& operator << (C_String& string, const F64 in);

	template <>
	inline C_String& operator << (C_String& string, const char* in);

	inline Util::C_String operator + (const char* p, const Util::C_String& s)
	{
		return Util::C_String(p) + s;
	}

	#include "StringHelpers.inl"
}

#endif //#ifdef USE_OLD_UTIL_CSTRING

#endif
