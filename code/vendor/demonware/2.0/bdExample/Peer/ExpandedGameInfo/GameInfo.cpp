// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "GameInfo.h"

GameInfo::GameInfo()
{
}

void GameInfo::serialize(bdBitBuffer& bitBuffer) const
{
	// Call the base class serialize
	bdGameInfo::serialize(bitBuffer);
	bitBuffer.writeBlob(m_gameName, GAME_NAME_SIZE);
}

bdBool GameInfo::deserialize(const bdCommonAddrRef me,
							 bdBitBuffer& bitBuffer)
{
	// Call the base class deserialize
	bdBool result = bdGameInfo::deserialize(me, bitBuffer);

	if(true == result)
	{
		bdNChar8 tempArray[GAME_NAME_SIZE];
		bdUInt tempAmountRead = GAME_NAME_SIZE;
		result = bitBuffer.readBlob(reinterpret_cast<bdUByte8*>(tempArray), tempAmountRead);

		if(	true == result &&
			false == bitBuffer.failedRead() &&
			GAME_NAME_SIZE == tempAmountRead)
		{
			bdMemcpy(m_gameName, tempArray, GAME_NAME_SIZE);
		}
		else
		{
			result = false;
		}
	}
	return result;
}


