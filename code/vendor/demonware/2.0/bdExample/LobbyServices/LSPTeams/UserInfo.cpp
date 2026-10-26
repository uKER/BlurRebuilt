// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "UserInfo.h"

PrivateProfileInfo::~PrivateProfileInfo()
{
}

void PrivateProfileInfo::serialize(bdByteBuffer& buffer)const
{
	bdProfileInfo::serialize(buffer);
	buffer.writeString(m_email, 200);
}

bdBool PrivateProfileInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdProfileInfo::deserialize(buffer);
	ok = ok && buffer->readString(m_email, 200);
	return ok;
}


bdUInt PrivateProfileInfo::sizeOf()
{
	return sizeof(PrivateProfileInfo);
}

PublicProfileInfo::~PublicProfileInfo()
{
}

void PublicProfileInfo::serialize(bdByteBuffer& buffer)const
{
	bdProfileInfo::serialize(buffer);
	buffer.writeInt32(m_experiencePoints);
}

bdBool PublicProfileInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdProfileInfo::deserialize(buffer);
	ok = ok && buffer->readInt32(m_experiencePoints);
	return ok;
}

bdUInt PublicProfileInfo::sizeOf()
{
	return sizeof(PublicProfileInfo);
}

