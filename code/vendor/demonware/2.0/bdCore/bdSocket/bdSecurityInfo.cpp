// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdSocket/bdSecurityInfo.h>

#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#define BD_LOG_LEVEL "bdCore/bdSocket/bdSecurityInfo"

bdUInt BD_CALL bdSecurityInfo::toString(const bdSecurityID &id, bdNChar8* buffer, bdUInt len)
{
	return bufferToString(id.ab, BD_SECURITY_ID_LENGTH, buffer, len);
}

bdUInt BD_CALL bdSecurityInfo::toString(const bdSecurityKey &key, bdNChar8* buffer, bdUInt len)
{
	return bufferToString(key.ab, BD_SECURITY_KEY_LENGTH, buffer, len);
}

bdUInt BD_CALL bdSecurityInfo::bufferToString(	const bdUByte8* buffer,
												const bdUInt bufferLen,
												bdNChar8* outBuffer,
												const bdUInt outBufferLen)
{
	bdNChar8* cur = outBuffer;
	for (bdUInt i=0; i < bufferLen; i++)
	{
		if (i % 4 == 0 && i != 0)
		{
			cur += bdSnprintf(cur, outBufferLen - (cur - outBuffer), " ");
		}

		if (bdSnprintf(cur, outBufferLen - (cur - outBuffer), "%02hhx", buffer[i]) == 2)
		{
			cur += 2;
		}
		else
		{
			// Run out of space.
			return 0;
		}

	}

	return static_cast<bdUInt>(cur - outBuffer);
}
