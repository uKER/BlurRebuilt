// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdContentStreaming/bdFileID.h>

bdFileID::bdFileID()
:	m_fileID(0)
{
}

bdFileID::~bdFileID()
{
}


void bdFileID::serialize(bdByteBuffer &buffer)const
{
	buffer.writeUInt64(m_fileID);
}

bdBool bdFileID::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt64(m_fileID);

	return ok;
}

bdUInt bdFileID::sizeOf()
{
	return sizeof(bdFileID);
}

void BD_CALL printInfo(const bdFileID& fileID,
					   bdNChar8 *buf,
					   const bdUInt length)
{
	if(length > 0)
	{
		bdSnprintf(buf, length, "FileID : %I64d", fileID.m_fileID);
	}
}
