// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class that uses operating system specific libraries to generate 
//			truly random numbers.

#ifndef BD_PLATFORM_TRULYRANDOM_H
#define BD_PLATFORM_TRULYRANDOM_H

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>

/// Fill an array of bytes of a specified size with random bytes.
/// \param	in[out]	The byte array to fill in.
/// \param	length[in]	The size of the array.
/// \todo (win32): 258000 - Memory Leak When You Use the Microsoft 
///  Base Cryptographic Provider. See implementation.
/// \todo (unix): every call means a read from /dev/random - see if this is slow
/// and if so implement read-ahead cache.
/// \todo (ps2, ps3, wii): No cryptographically strong random number generator on PS2. Weak
/// PRNG used instead. This should be fixed.
void BD_CALL bdGetRandomUChar8(bdUByte8 in[], 
							   const bdUInt length);				

#endif // BD_PLATFORM_TRULYRANDOM_H
