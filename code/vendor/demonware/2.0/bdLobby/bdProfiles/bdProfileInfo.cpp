// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS


#include <bdLobby/bdProfiles/bdProfileInfo.h>

#define BD_LOG_LEVEL "profilesInfo"

bdProfileInfo::bdProfileInfo()
: m_entityID(0)
{
}

bdProfileInfo::~bdProfileInfo()
{
}

void bdProfileInfo::serialize(bdByteBuffer&)const
{
}

bdBool bdProfileInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = buffer->readUInt64(m_entityID);
	if (!ok)
	{
		BD_ERR(BD_LOG_LEVEL, "Deserialization failed");
	}
	return ok;
}

bdUInt bdProfileInfo::sizeOf()
{
	return sizeof(bdProfileInfo);
}

bdEntityID bdProfileInfo::getEntityID() const
{
	return m_entityID;
}

