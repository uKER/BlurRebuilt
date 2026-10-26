// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "TeamInfo.h"

void PublicTeamProfile::serialize(bdByteBuffer& buffer)const
{
	bdTeamProfile::serialize(buffer);
	buffer.writeInt32(m_int1);
	buffer.writeInt32(m_int2);
	buffer.writeInt32(m_int3);
	buffer.writeInt32(m_int4);
	buffer.writeInt32(m_int5);
	buffer.writeInt32(m_int6);

	buffer.writeString(m_string1, 100);
	buffer.writeString(m_string2, 100);
}

bdBool PublicTeamProfile::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdTeamProfile::deserialize(buffer);

	ok = ok && buffer->readInt32(m_int1);
	ok = ok && buffer->readInt32(m_int2);
	ok = ok && buffer->readInt32(m_int3);
	ok = ok && buffer->readInt32(m_int4);
	ok = ok && buffer->readInt32(m_int5);
	ok = ok && buffer->readInt32(m_int6);

	ok = ok && buffer->readString(m_string1, 100);
	ok = ok && buffer->readString(m_string2, 100);

	return ok;
}

bdUInt PublicTeamProfile::sizeOf()
{
	return sizeof(PublicTeamProfile);
}


void PrivateTeamProfile::serialize(bdByteBuffer& buffer)const
{
	bdTeamProfile::serialize(buffer);
	buffer.writeString(m_string1, 100);
	buffer.writeString(m_string2, 100);
}

bdBool PrivateTeamProfile::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdTeamProfile::deserialize(buffer);
	ok = ok && buffer->readString(m_string1, 100);
	ok = ok && buffer->readString(m_string2, 100);

	return ok;
}

bdUInt PrivateTeamProfile::sizeOf()
{
	return sizeof(PrivateTeamProfile);
}

// Team Member Info's

void PublicTeamMemberInfo::serialize(bdByteBuffer& buffer)const
{
	bdProfileInfo::serialize(buffer);
	buffer.writeInt64(m_int1);
	buffer.writeInt32(m_int2);
	buffer.writeInt32(m_int3);
	buffer.writeString(m_string1, 100);
	buffer.writeString(m_string2, 100);
}

bdBool PublicTeamMemberInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdProfileInfo::deserialize(buffer);
	bdUInt64 userID;
	ok = ok && buffer->readUInt64(userID);
	ok = ok && buffer->readInt64(m_int1);
	ok = ok && buffer->readInt32(m_int2);
	ok = ok && buffer->readInt32(m_int3);
	ok = ok && buffer->readString(m_string1, 100);
	ok = ok && buffer->readString(m_string2, 100);

	return ok;
}

bdUInt PublicTeamMemberInfo::sizeOf()
{
	return sizeof(PublicTeamMemberInfo);
}

void PrivateTeamMemberInfo::serialize(bdByteBuffer& buffer)const
{
	bdProfileInfo::serialize(buffer);
	buffer.writeInt32(m_int1);

	buffer.writeString(m_string1, 100);
	buffer.writeString(m_string2, 100);
}

bdBool PrivateTeamMemberInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdProfileInfo::deserialize(buffer);
	bdUInt64 userID;
	ok = ok && buffer->readUInt64(userID);
	ok = ok && buffer->readInt32(m_int1);
	ok = ok && buffer->readString(m_string1, 100);
	ok = ok && buffer->readString(m_string2, 100);

	return ok;
}

bdUInt PrivateTeamMemberInfo::sizeOf()
{
	return sizeof(PrivateTeamMemberInfo);
}

