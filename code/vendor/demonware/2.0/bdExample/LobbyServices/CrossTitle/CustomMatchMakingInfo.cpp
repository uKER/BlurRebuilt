// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY		: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "CustomMatchMakingInfo.h"

#include <bdCore/bdSocket/bdCommonAddrInfo.h>

#define BD_LOG_LEVEL "MM Info"

CustomMatchMakingInfo::CustomMatchMakingInfo()
: m_levelNum(0)
{
	bdMemset(m_sessionName, '\0', sizeof(m_sessionName));
}

CustomMatchMakingInfo::~CustomMatchMakingInfo()
{
}

bdUWord CustomMatchMakingInfo::setSessionName(const bdNChar8 *const sessionName)
{
	bdUWord sessionNameLength = 0;
	if (sessionName != BD_NULL)
	{
		sessionNameLength = bdStrlcpy(m_sessionName, sessionName, sizeof(m_sessionName));
		if (sessionNameLength >= sizeof(m_sessionName))
		{
			BD_WARN(BD_LOG_LEVEL, "Session name truncated.");
		}
	}
 	return sessionNameLength;
}

bdUWord CustomMatchMakingInfo::getSessionName(bdNChar8 *const buf,
											  const bdUInt bufSize) const
{
	const bdUWord sessionNameLength = bdStrlcpy(buf, m_sessionName, bufSize);
	if(sessionNameLength >= bufSize)
	{
		BD_WARN(BD_LOG_LEVEL, "Session name truncated");
	}
	return sessionNameLength;
}

void CustomMatchMakingInfo::setLevelNum(const bdInt levelNum)
{
	m_levelNum = levelNum;
}

bdInt CustomMatchMakingInfo::getLevelNum() const
{
	return m_levelNum;
}


void CustomMatchMakingInfo::serialize(bdByteBuffer& buffer) const
{
	bdMatchMakingInfo::serialize(buffer);

	buffer.writeString(m_sessionName, sizeof(m_sessionName));
	buffer.writeInt32(m_levelNum);
	buffer.writeBlob(m_securityKey, BD_SECURITY_KEY_LENGTH);
}

bdBool CustomMatchMakingInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdMatchMakingInfo::deserialize(buffer);

	ok = ok && buffer->readString(m_sessionName, sizeof(m_sessionName));
	ok = ok && buffer->readInt32(m_levelNum);
	bdUInt len = BD_SECURITY_KEY_LENGTH;
	ok = ok && buffer->readBlob(m_securityKey, len);
	return ok;
}

bdUInt CustomMatchMakingInfo::sizeOf()
{
	return sizeof(CustomMatchMakingInfo);
}

void BD_CALL printInfo(const CustomMatchMakingInfo& sessionData,
					   bdNChar8 *buf,
					   const bdUInt size)
{
	if(size > 0)
	{
		bdNChar8 addrStr[BD_COMMON_ADDR_INFO_SIZE];
		bdCommonAddrInfo::getBriefInfo(sessionData.getHostAddrAsCommonAddr(), addrStr, sizeof(addrStr));

		bdNChar8 sessionName[BD_MAX_SESSIONNAME_LENGTH];
		sessionData.getSessionName(sessionName, sizeof(sessionName));

		const bdInt gameType = sessionData.m_gameType;

		bdSnprintf(buf, size, "%s: GameType %u\n %s", sessionName, gameType, addrStr);
	}
}