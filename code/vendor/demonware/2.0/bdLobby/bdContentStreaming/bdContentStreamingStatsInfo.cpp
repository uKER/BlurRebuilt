// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "bdContentStreamingStatsInfo.h"

bdContentStreamingStatsInfo::bdContentStreamingStatsInfo()
: m_fileOwnerID(0)
{
	bdMemset(m_fileOwnerName, 0, BD_MAX_USERNAME_LENGTH+1);
}

bdContentStreamingStatsInfo::~bdContentStreamingStatsInfo()
{
}

bdUInt bdContentStreamingStatsInfo::sizeOf()
{
	return sizeof(bdContentStreamingStatsInfo);
}
		 
void bdContentStreamingStatsInfo::serialize(bdByteBuffer &buffer)const
{
	// Call up to the base class to serialize the standard statistics info.
	bdStatsInfo::serialize(buffer);

	// Serialize the custom attributes.
	buffer.writeUInt64(m_fileOwnerID);
	buffer.writeString(m_fileOwnerName, BD_MAX_USERNAME_LENGTH+1);
}

bdBool bdContentStreamingStatsInfo::deserialize(bdByteBufferRef buffer)
{
	// Call up to the base class to deserialize the standard matchmaking info.
	bdBool ok = bdStatsInfo::deserialize(buffer);
	ok = ok && buffer->readUInt64(m_fileOwnerID);	
	ok = ok && buffer->readString(m_fileOwnerName, BD_MAX_USERNAME_LENGTH+1);
	return ok;
}
