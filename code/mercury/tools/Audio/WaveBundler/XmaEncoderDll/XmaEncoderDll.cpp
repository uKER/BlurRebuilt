// XmaEncoderDll.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include <stdio.h>
#include "XmaEncoderDll.h"

#ifdef _MANAGED
#pragma managed(push, off)
#endif

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
    return TRUE;
}

#ifdef _MANAGED
#pragma managed(pop)
#endif

//--------------------------------------------------------------------------------------
// Name: XMAEncode
// Desc: wrapper function which calls XDK function XMAInMemoryEncoder() from xmaencoder.lib
//--------------------------------------------------------------------------------------
XMAENCODERDLLWRAPPER_API HRESULT XMAEncode(DWORD InputStreamCount, 
                          const XMAENCODERSTREAM* pStreams, 
                          DWORD Compression, 
                          DWORD Flags,
						  //DWORD BlockSizeInKbytes,	// for xma2
                          VOID** ppEncodedBuffer, 
                          DWORD* pEncodedBufferSize, 
                         // XMA2WAVEFORMAT** ppEncodedBufferFormat, 
						  XMAWAVEFORMAT** ppEncodedBufferFormat, 
                          DWORD* pEncodedBufferFormatSize,
                          DWORD** ppSeekTable,
                          DWORD* pSeekTableSize)
{
	//OutputDebugString("DllWrapper: XmaInMemoryEncode()\n");

	return XMAInMemoryEncoder(InputStreamCount, 
                      pStreams, 
                      Compression, 
                      Flags, 
					  //BlockSizeInKbytes,
                      ppEncodedBuffer, 
                      pEncodedBufferSize, 
                      ppEncodedBufferFormat, 
                      pEncodedBufferFormatSize,
                      ppSeekTable,
                      pSeekTableSize);
}

//--------------------------------------------------------------------------------------
// Name: XMA2Encode
// Desc: wrapper function which calls XDK function XMAInMemoryEncoder() from xmaencoder.lib
//--------------------------------------------------------------------------------------
XMAENCODERDLLWRAPPER_API HRESULT XMA2Encode(DWORD InputStreamCount, 
										   const XMAENCODERSTREAM* pStreams, 
										   DWORD Compression, 
										   DWORD Flags,
										   DWORD BlockSizeInKbytes,	// for xma2
										   VOID** ppEncodedBuffer, 
										   DWORD* pEncodedBufferSize, 
										   XMA2WAVEFORMAT** ppEncodedBufferFormat, 
										   DWORD* pEncodedBufferFormatSize,
										   DWORD** ppSeekTable,
										   DWORD* pSeekTableSize)
{
	//OutputDebugString("DllWrapper: Xma2InMemoryEncode()\n");

	return XMA2InMemoryEncoder(InputStreamCount, 
		pStreams, 
		Compression, 
		Flags, 
		BlockSizeInKbytes,
		ppEncodedBuffer, 
		pEncodedBufferSize, 
		ppEncodedBufferFormat, 
		pEncodedBufferFormatSize,
		ppSeekTable,
		pSeekTableSize);
}

//--------------------------------------------------------------------------------------
// Name: DllAlloc
// Desc: alloc function which allocs from the dll's heap
//--------------------------------------------------------------------------------------
void* DllAlloc(size_t sz)
{
	//printf("DllAlloc(%d)\n", sz);
	return malloc(sz);
}

//-----------------------------------------------------------------------------
// Name: DllFree()
// Desc: free function which frees from the dll's heap
//-----------------------------------------------------------------------------
void DllFree(void* ptr)
{
	//printf("DllFree(%x)\n", (DWORD)ptr);
	free(ptr);
}



