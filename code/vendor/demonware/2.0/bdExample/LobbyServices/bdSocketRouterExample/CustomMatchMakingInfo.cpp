// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32 UNIX OSX IPHONE PS2 PS3 PSP-INFRA WII
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "CustomMatchMakingInfo.h"

CustomMatchMakingInfo::CustomMatchMakingInfo()
: m_levelNum(0)
{
}

CustomMatchMakingInfo::~CustomMatchMakingInfo()
{
}

CustomMatchMakingInfo::CustomMatchMakingInfo(const bdNChar8 *const sessionName,
											 const bdInt levelNum,
											 const bdInt gameType)									 
											 : m_levelNum(levelNum)
{
	m_gameType = gameType;
	setSessionName(sessionName);

}

bdUWord CustomMatchMakingInfo::getSessionName(bdNChar8 *const str) const
{
	const bdUWord val = bdStrlcpy(str, m_sessionName, BD_MAX_SESSIONNAME_LENGTH+1);
	if (val >BD_MAX_SESSIONNAME_LENGTH+1)
	{
		bdPrintf("Error");
	}
	return val;
}

bdUWord CustomMatchMakingInfo::setSessionName(const bdNChar8 *const sessionName)
{
	bdUWord val = 0;
	if (sessionName != BD_NULL)
	{
		val = bdStrlcpy(m_sessionName, sessionName, BD_MAX_SESSIONNAME_LENGTH+1);
		if (val > BD_MAX_SESSIONNAME_LENGTH+1)
		{
			bdPrintf("CustomMatchMakingInfo::setSessionName, sessionName too big.");	
			bdPrintf("UserInfo::setPassword, password too big.");
		}
	}
	return val;
}

bdInt CustomMatchMakingInfo::getLevelNum() const
{
	return m_levelNum;
}

void CustomMatchMakingInfo::setLevelNum(bdInt levelNum)
{
	m_levelNum = levelNum;
}

bdInt CustomMatchMakingInfo::getGameType() const
{
	return m_gameType;
}

void CustomMatchMakingInfo::setGameType(bdInt gameType)
{
	m_gameType = gameType;
}

void CustomMatchMakingInfo::serialize(bdByteBuffer& buffer)const
{
	// Call up to the base class to serialize the standard matchmaking info.
	bdMatchMakingInfo::serialize(buffer);

	// Serialize the custom attributes
	buffer.writeString(m_sessionName, BD_MAX_SESSIONNAME_LENGTH);
	buffer.writeInt32(m_levelNum);
	buffer.writeBlob(m_secKey.ab, BD_SECURITY_KEY_LENGTH);
}

bdBool CustomMatchMakingInfo::deserialize(bdByteBufferRef buffer)
{
	// Call up to the base class to deserialize the standard matchmaking info.
	bdBool ok = bdMatchMakingInfo::deserialize(buffer);

	ok = ok && buffer->readString(m_sessionName, BD_MAX_SESSIONNAME_LENGTH);
	ok = ok && buffer->readInt32(m_levelNum);
	bdUInt maxLen = BD_SECURITY_KEY_LENGTH;
	ok = ok && buffer->readBlob(m_secKey.ab, maxLen);
	return ok;
}

bdUInt CustomMatchMakingInfo::sizeOf()
{
	return sizeof(CustomMatchMakingInfo);
}
