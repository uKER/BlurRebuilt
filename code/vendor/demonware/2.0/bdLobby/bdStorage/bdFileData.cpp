// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdStorage/bdFileData.h>

bdFileData::bdFileData()
:m_fileData(BD_NULL),
 m_fileSize(0)
{
}

bdFileData::bdFileData(void* fileData, bdUInt fileSize)
:m_fileData(fileData),
 m_fileSize(fileSize)
{
}

bdFileData::~bdFileData()
{
}

bdBool bdFileData::deserialize(bdByteBufferRef buffer)
{
	bdBool ok;
	ok = buffer->readBlob(static_cast<bdUByte8*>(m_fileData), m_fileSize);
	return ok;
}

bdUInt bdFileData::sizeOf()
{
	return sizeof(bdFileData);
}


