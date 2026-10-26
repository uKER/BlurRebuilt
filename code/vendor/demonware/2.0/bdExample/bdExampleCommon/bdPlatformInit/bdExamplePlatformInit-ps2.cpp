// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: PS2
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit.h>

#ifdef BD_PS2_USE_LIBINSOCK
#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit-ps2-libinsock.inl>
#elif defined BD_PS2_USE_LIBEENET
#include <bdExample/bdExampleCommon/bdPlatformInit/bdExamplePlatformInit-ps2-eenet.inl>
#else "No network stack defined."
#endif

