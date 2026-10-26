// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformCoreTypes/bdPlatformCoreTypes.h>

/// Macro to convert the number of bytes required to hold data in binary to
/// to number of bytes required for equivalent data in base64 encoding.
#define BD_BIN_TO_B64_SIZE(n) (((((n)+2)/3) *4) + 1)
/// Macro to convert the number of bytes required to hold data encoded in base
/// 64 to the number of bytes required for equivalent data in raw binary.
#define BD_B64_TO_BIN_SIZE(n) (((n)*3)/4)

/// This macro evaluates to a safe size for a binary buffer that base64 data
/// is to be decoded to. This is necessary because a given number of base64 bytes
//  can decode to different lengths, depending on how much padding is present.
#define BD_B64_SAFE_BUFFER_SIZE(n) ((n)+2)

class bdBase64
{
	public:

		static void BD_CALL encode(const bdByte8* src, 
						   bdNChar8* dest, 
						   bdUInt maxLen);

		static bdUInt BD_CALL decode(const bdNChar8* src, 
							 bdByte8* dest, 
							 bdUInt maxLen);

};
