// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdMatchMaking/bdMatchMakingInfo.h>

#define BD_LOG_LEVEL "match making info"

bdMatchMakingInfo::bdMatchMakingInfo()
: m_hostAddrSize(0),
  m_gameType(0),
  m_maxPlayers(0)
{
	bdMemset(m_hostAddr, 0, sizeof(m_hostAddr));
}

bdMatchMakingInfo::bdMatchMakingInfo(const bdCommonAddrRef hostAddr,
									 const bdInt32  gameType,
									 const bdUInt32 maxNumPlayers)
: m_gameType(gameType),
  m_maxPlayers(maxNumPlayers)
{
	hostAddr->serialize(m_hostAddr);
	m_hostAddrSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
}

bdMatchMakingInfo::bdMatchMakingInfo(const bdUByte8 *hostAddr,
									 const bdUInt32 hostAddrSize,
									 const bdInt32  gameType,
									 const bdUInt32 maxNumPlayers)
: m_gameType(gameType),
  m_maxPlayers(maxNumPlayers)
{
	BD_ASSERT(hostAddr != BD_NULL, "No valid host address specified");
	BD_ASSERT(hostAddrSize <= BD_MAX_HOST_ADDR_SIZE, "Size of host address exceeds BD_MAX_HOST_ADDR_SIZE");
	bdMemcpy(m_hostAddr, hostAddr, hostAddrSize);
	m_hostAddrSize = hostAddrSize;

}

bdMatchMakingInfo::~bdMatchMakingInfo()
{
}

void bdMatchMakingInfo::setHostAddr(bdCommonAddrRef localCommonAddr)
{
	localCommonAddr->serialize(m_hostAddr);
	m_hostAddrSize = BD_COMMON_ADDR_SERIALIZED_SIZE;
}

bdCommonAddrRef bdMatchMakingInfo::getHostAddrAsCommonAddr(bdCommonAddrRef localCommonAddr) const
{
	bdCommonAddrRef commonAddr;
	bdBool ok = true;
	// Sanity check: make sure that the length of the m_hostAddr array
	// matches the size of a bdCommonAddr
	if (m_hostAddrSize == BD_COMMON_ADDR_SERIALIZED_SIZE)
	{
		commonAddr = new bdCommonAddr();
		ok = ok && commonAddr->deserialize(localCommonAddr, m_hostAddr);
		if(localCommonAddr == BD_NULL)
		{
			/// See note in documentation for this function.
			BD_WARN(BD_LOG_LEVEL, "localCommonAddr not set. ");
		}
	}
	else 
	{
		BD_ERR(BD_LOG_LEVEL, "m_hostAddrSize != BD_COMMON_ADDR_SERIALIZED_SIZE, ");
		ok = false;
	}

	return commonAddr;
}

void bdMatchMakingInfo::serialize(bdByteBuffer &buffer)const
{
	buffer.writeBlob(m_hostAddr, m_hostAddrSize);
	buffer.writeUInt32(m_gameType);
	buffer.writeUInt32(m_maxPlayers);
}

bdBool bdMatchMakingInfo::deserialize(bdByteBufferRef buffer)
{

	bdUInt length = BD_MAX_HOST_ADDR_SIZE;
	bdBool ok = buffer->readBlob(m_hostAddr, length);
	m_hostAddrSize = length;

	length = BD_SECURITY_ID_LENGTH;
	ok = ok && buffer->readBlob(m_sessionID.ab, length);

	bdUInt32 intValue = 0;
	ok = ok && buffer->readUInt32(intValue);
	m_gameType = intValue;

	bdUInt32 uintValue = 0;
	ok = ok && buffer->readUInt32(uintValue);
	m_maxPlayers = uintValue;
	
	ok = ok && buffer->readUInt32(uintValue);
	m_numPlayers = uintValue;

	if (!ok)
	{
		BD_ERR(BD_LOG_LEVEL, "Deserialization failed");
	}
	return ok;
}

bdUInt bdMatchMakingInfo::sizeOf()
{
	return sizeof(bdMatchMakingInfo);
}

