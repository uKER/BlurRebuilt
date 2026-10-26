// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the XMAENCODERDLLWRAPPER_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// XMAENCODERDLLWRAPPER_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.
#ifdef XMAENCODERDLLWRAPPER_EXPORTS
#define XMAENCODERDLLWRAPPER_API __declspec(dllexport)
#else
#define XMAENCODERDLLWRAPPER_API __declspec(dllimport)
#endif

#include "XmaEncoder.h"

extern XMAENCODERDLLWRAPPER_API HRESULT XMAEncode(DWORD InputStreamCount, 
                          const XMAENCODERSTREAM* pStreams, 
                          DWORD Compression, 
                          DWORD Flags, 
						  //DWORD BlockSizeInKbytes, // for xma2
                          VOID** ppEncodedBuffer, 
                          DWORD* pEncodedBufferSize, 
                          XMAWAVEFORMAT** ppEncodedBufferFormat, 
                          DWORD* pEncodedBufferFormatSize,
                          DWORD** ppSeekTable,
                          DWORD* pSeekTableSize);

extern XMAENCODERDLLWRAPPER_API HRESULT XMA2Encode(DWORD InputStreamCount, 
												  const XMAENCODERSTREAM* pStreams, 
												  DWORD Compression, 
												  DWORD Flags, 
												  DWORD BlockSizeInKbytes, // for xma2
												  VOID** ppEncodedBuffer, 
												  DWORD* pEncodedBufferSize, 
												  XMA2WAVEFORMAT** ppEncodedBufferFormat, 
												  DWORD* pEncodedBufferFormatSize,
												  DWORD** ppSeekTable,
												  DWORD* pSeekTableSize);

extern XMAENCODERDLLWRAPPER_API void* DllAlloc(size_t sz);
extern XMAENCODERDLLWRAPPER_API void DllFree(void* ptr);
