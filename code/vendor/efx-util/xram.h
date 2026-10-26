// xram.h — shim for Creative's X-RAM extension header from the OpenAL 1.1 SDK.
// The engine queries the extension at runtime through alGetProcAddress /
// alGetEnumValue and only needs the function-pointer types from this header.
#ifndef BLUR2PC_XRAM_H
#define BLUR2PC_XRAM_H

#include "al.h"

typedef ALboolean (__cdecl *EAXSetBufferMode)(ALsizei n, ALuint* buffers, ALint value);
typedef ALenum    (__cdecl *EAXGetBufferMode)(ALuint buffer, ALint* value);

#endif
