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

	buffer.writeString(m_string1, TEAM_STRING_SIZE);
}

bdBool PublicTeamProfile::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdTeamProfile::deserialize(buffer);

	ok = ok && buffer->readInt32(m_int1);
	ok = ok && buffer->readString(m_string1, TEAM_STRING_SIZE);
	return ok;
}

bdUInt PublicTeamProfile::sizeOf()
{
	return sizeof(PublicTeamProfile);
}


void PrivateTeamProfile::serialize(bdByteBuffer& buffer)const
{
	bdTeamProfile::serialize(buffer);
	buffer.writeString(m_string1, TEAM_STRING_SIZE);
	buffer.writeString(m_string2, TEAM_STRING_SIZE);
}

bdBool PrivateTeamProfile::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdTeamProfile::deserialize(buffer);
	ok = ok && buffer->readString(m_string1, TEAM_STRING_SIZE);
	ok = ok && buffer->readString(m_string2, TEAM_STRING_SIZE);

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
	buffer.writeString(m_string1, TEAM_STRING_SIZE);
}

bdBool PublicTeamMemberInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdProfileInfo::deserialize(buffer);
	bdUInt64 userID;
	ok = ok && buffer->readUInt64(userID);
	ok = ok && buffer->readInt64(m_int1);
	ok = ok && buffer->readString(m_string1, TEAM_STRING_SIZE);
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

	buffer.writeString(m_string1, TEAM_STRING_SIZE);
}

bdBool PrivateTeamMemberInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = bdProfileInfo::deserialize(buffer);
	bdUInt64 userID;
	ok = ok && buffer->readUInt64(userID);
	ok = ok && buffer->readInt32(m_int1);
	ok = ok && buffer->readString(m_string1, TEAM_STRING_SIZE);
	return ok;
}

bdUInt PrivateTeamMemberInfo::sizeOf()
{
	return sizeof(PrivateTeamMemberInfo);
}

