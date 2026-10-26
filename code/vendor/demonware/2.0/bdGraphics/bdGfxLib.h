// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE		: When running on Win32, this file can be used to switch between Direct3D and OpenGL

#ifndef BD_GFX_LIB_H
#define BD_GFX_LIB_H

#ifdef BD_PLATFORM_WIN32
#	if 1
#		define BD_WIN32_DIRECTX
#	else
#		define BD_WIN32_GL
#	endif
#endif // BD_PLATFORM_WIN32

#endif // BD_GFX_LIB_H