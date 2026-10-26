// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdNet/bdLANDiscovery/bdGameInfo.h>

#define BD_LOG_LEVEL "discovery/gameinfo"

/// \todo think about this a bit more..
#define BD_INVALID_TITLE_ID 666

bdGameInfo::bdGameInfo()
: m_titleId(BD_INVALID_TITLE_ID),
  m_hostAddr(BD_NULL)  
{
}

bdGameInfo::bdGameInfo(bdUInt32 titleID, 
		   const bdSecurityID& securityID, 
		   const bdSecurityKey& securityKey,
		   bdCommonAddrRef hostAddr)
		   :m_titleId(titleID),
		   m_secID(securityID),
		   m_secKey(securityKey),
		   m_hostAddr(hostAddr)
{
}

bdGameInfo::~bdGameInfo()
{
}

bdUInt32 bdGameInfo::getTitleID() const
{
	if(BD_INVALID_TITLE_ID == m_titleId)
	{
		BD_WARN(BD_LOG_LEVEL, "This object appears to be uninitialized.");
	}

	return m_titleId;
}

bdCommonAddrRef bdGameInfo::getHostAddr() const
{
	if(m_hostAddr.isNull())
	{
		BD_WARN(BD_LOG_LEVEL, "This object appears to be uninitialized.");
	}
	return m_hostAddr;
}

const bdSecurityID& bdGameInfo::getSecurityID() const
{
	return m_secID;
}

const bdSecurityKey& bdGameInfo::getSecurityKey() const
{
	return m_secKey;
}

void bdGameInfo::setTitleID(const bdUInt32 titleID)
{
	m_titleId = titleID;
}

void bdGameInfo::setHostAddr(const bdCommonAddrRef hostAddr)
{
	m_hostAddr = hostAddr;
}

void bdGameInfo::setSecurityID(const bdSecurityID& secID)
{
	m_secID = secID;
}

void bdGameInfo::setSecurityKey(const bdSecurityKey& secKey)
{
	m_secKey = secKey;
}

void bdGameInfo::serialize(bdBitBuffer& bitBuffer) const
{
	if(BD_INVALID_TITLE_ID == m_titleId)
	{
		BD_WARN(BD_LOG_LEVEL, "This object appears to be uninitialized.");
	}

	if(m_hostAddr.isNull())
	{
		BD_ERR(BD_LOG_LEVEL, "Serialized failed as data is uninitialized.");
	}
	else
	{
		bitBuffer.writeUInt32(m_titleId);
		bitBuffer.writeFull(m_secID.ab);
		bitBuffer.writeFull(m_secKey.ab);		

		bdUByte8 hostAddrBuffer[BD_COMMON_ADDR_SERIALIZED_SIZE];
		m_hostAddr->serialize(hostAddrBuffer);
		bitBuffer.writeBits(hostAddrBuffer, sizeof(hostAddrBuffer) << 3);
	}
}


bdBool bdGameInfo::deserialize(const bdCommonAddrRef localAddr, 
							   bdBitBuffer& bitBuffer)
{
	bdBool ok = true;

	// Deserialize to temporary variables first,
	// If all succeed then assign values to member variables.
	bdUInt32 tempTitleID = 0;
	bdSecurityID tempSecID;
	bdSecurityKey tempSecKey;
	bdCommonAddrRef tempCommonAddr;	

	ok = ok && bitBuffer.readUInt32(tempTitleID);
	ok = ok && bitBuffer.readFull(tempSecID.ab);
	ok = ok && bitBuffer.readFull(tempSecKey.ab);	

	bdUByte8 tempAddrBuffer[BD_COMMON_ADDR_SERIALIZED_SIZE];
	ok = ok && bitBuffer.readBits(tempAddrBuffer, sizeof(tempAddrBuffer) << 3);

	if(ok)
	{
		 tempCommonAddr = new bdCommonAddr();
		 ok = ok && tempCommonAddr->deserialize(localAddr, tempAddrBuffer);
	}

	if (!ok)
	{
		BD_ERR(BD_LOG_LEVEL, "Deserialization failed");
	}
	else
	{
		m_titleId = tempTitleID;
		bdMemcpy(m_secID.ab , tempSecID.ab, BD_SECURITY_ID_LENGTH);
		bdMemcpy(m_secKey.ab, tempSecKey.ab, BD_SECURITY_KEY_LENGTH);
		m_hostAddr = tempCommonAddr;
	}
	return ok;
}
