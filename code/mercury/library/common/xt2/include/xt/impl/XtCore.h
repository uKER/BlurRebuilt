#ifndef XT_IMPL_XTCORE_H
#define XT_IMPL_XTCORE_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Xt/XtCoreTypes.h"

//
// XtEndian
//

enum XtEndian
{
	kXtLittleEndian,
	kXtBigEndian
};

#if defined(PLATFORM_PS3) || defined(PLATFORM_WII) || defined(PLATFORM_SPU)
	inline XtEndian XtGetEndian() { return kXtBigEndian; }
#elif defined(PLATFORM_PC) || defined(PLATFORM_LINUX)
	inline XtEndian XtGetEndian() { return kXtLittleEndian; }
#else
	inline XtEndian XtGetEndian()
	{
		static const XtU16 test = 0x00FF;

		return (*((XtU8*)&test) != 0) ? kXtLittleEndian : kXtBigEndian;
	}
#endif

//
// Prototypes
//

namespace XtUt
{
	template<typename Type> Type* Construct(void* where);
	template<typename Type> Type* Construct(void* where, const Type& toCopy);
	template<typename Type> Type* ConstructN(void* where, size_t count);

	template<typename Type> void Destruct(void* where);
	template<typename Type> void DestructN(void* where, size_t count);

	template<typename Type> Type* AllocateAsBytes(size_t count);
	template<typename Type> void FreeAsBytes(Type* array);

	template<typename Type> struct LessThan;
	template<typename Type> struct GreaterThan;
};

#endif // XT_IMPL_XTCORE_H

