#ifndef __SYSTEM_ENDIAN_H__
#define __SYSTEM_ENDIAN_H__

#pragma once

namespace Ut
{


// Don't use this, use typedefs below.
struct DoEndianSwap
{
	static void EndianSwapQ(void* out, const void* in);
	static void EndianSwapD(void* out, const void* in);
	static void EndianSwapW(void* out, const void* in);
	static void EndianSwapQ(void* ptr);
	static void EndianSwapD(void* ptr);
	static void EndianSwapW(void* ptr);

#if defined(COMPILER_VC)
	FORCE_INLINE static void EndianSwap(U64& data) { data = _byteswap_uint64(data); }
	FORCE_INLINE static void EndianSwap(U32& data) { data = _byteswap_ulong(data); }
	FORCE_INLINE static void EndianSwap(U16& data) { data = _byteswap_ushort(data); }
	FORCE_INLINE static void EndianSwap(S64& data) { data = _byteswap_uint64(data); }
	FORCE_INLINE static void EndianSwap(S32& data) { data = _byteswap_ulong(data); }
	FORCE_INLINE static void EndianSwap(S16& data) { data = _byteswap_ushort(data); }
	FORCE_INLINE static void EndianSwap(float& data) { (U32&)data = _byteswap_ulong((U32&)data); }
	FORCE_INLINE static void EndianSwap(double& data) { (U64&)data = _byteswap_uint64((U64&)data); }
#else
	FORCE_INLINE static void EndianSwap(U64& data) {EndianSwapQ(&data);}
	FORCE_INLINE static void EndianSwap(U32& data) {EndianSwapD(&data);}
	FORCE_INLINE static void EndianSwap(U16& data) {EndianSwapW(&data);}
	FORCE_INLINE static void EndianSwap(S64& data) {EndianSwapQ(&data);}
	FORCE_INLINE static void EndianSwap(S32& data) {EndianSwapD(&data);}
	FORCE_INLINE static void EndianSwap(S16& data) {EndianSwapW(&data);}
	FORCE_INLINE static void EndianSwap(float& data) {EndianSwapD(&data);}
	FORCE_INLINE static void EndianSwap(double& data) {EndianSwapD(&data);}
#endif
	static void EndianSwap(U8& data) {}

	static void EndianSwap(void *ptr, const char *pszFormat, DWORD dwNumItems=1);
};

struct NoEndianSwap
{
	FORCE_INLINE static void EndianSwapQ(void* out, const void* in) {*(U64*) out = *(U64*) in;}
	FORCE_INLINE static void EndianSwapD(void* out, const void* in) {*(U32*) out = *(U32*) in;}
	FORCE_INLINE static void EndianSwapW(void* out, const void* in) {*(U16*) out = *(U16*) in;}
	FORCE_INLINE static void EndianSwapQ(void* ptr) {}
	FORCE_INLINE static void EndianSwapD(void* ptr) {}
	FORCE_INLINE static void EndianSwapW(void* ptr) {}

	FORCE_INLINE static void EndianSwap(U64& data) {}
	FORCE_INLINE static void EndianSwap(S64& data) {}
	FORCE_INLINE static void EndianSwap(U32& data) {}
	FORCE_INLINE static void EndianSwap(U16& data) {}
	FORCE_INLINE static void EndianSwap(S32& data) {}
	FORCE_INLINE static void EndianSwap(S16& data) {}
	FORCE_INLINE static void EndianSwap(U8& data) {}
	FORCE_INLINE static void EndianSwap(float& data) {}

	static void EndianSwap(void *ptr, const char *pszFormat, DWORD dwNumItems=1) {}
};

////////////////////////////////////////////////


#ifdef PLATFORM_XENON // Endian swapping for XENON

typedef DoEndianSwap LittleEndianToNative;
typedef DoEndianSwap NativeToLittleEndian;

typedef NoEndianSwap BigEndianToNative;
typedef NoEndianSwap NativeToBigEndian;

#else // Endian swapping for PC

typedef NoEndianSwap LittleEndianToNative;
typedef NoEndianSwap NativeToLittleEndian;

typedef DoEndianSwap BigEndianToNative;
typedef DoEndianSwap NativeToBigEndian;

#endif

}

#endif