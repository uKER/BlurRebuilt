// dxerr.h — shim for the DirectX SDK's dxerr library (not part of the Windows
// SDK any more). The engine only calls DXGetErrorString(hr) for log messages.
#ifndef BLUR2PC_DXERR_H
#define BLUR2PC_DXERR_H

#include <windows.h>
#include <stdio.h>

inline const char* DXGetErrorStringA(HRESULT hr)
{
	static char buf[4][32];
	static int next = 0;
	char* b = buf[next++ & 3];
	_snprintf_s(b, 32, _TRUNCATE, "HRESULT 0x%08lX", (unsigned long)hr);
	return b;
}

inline const char* DXGetErrorDescriptionA(HRESULT hr)
{
	return DXGetErrorStringA(hr);
}

#define DXGetErrorString DXGetErrorStringA
#define DXGetErrorDescription DXGetErrorDescriptionA

#endif
