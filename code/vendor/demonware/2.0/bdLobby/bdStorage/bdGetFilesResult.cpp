// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdStorage/bdGetFilesResult.h>

bdGetFilesResult::bdGetFilesResult()
: m_ID(0)
{
}

bdGetFilesResult::~bdGetFilesResult()
{
}

bdGetFilesResult::bdGetFilesResult(void* fileData, bdUInt fileSize)
{
	setFileData(fileData, fileSize);
}

void bdGetFilesResult::setFileData(void* fileData, bdUInt fileSize)
{
	m_data.m_fileData = fileData;
	m_data.m_fileSize = fileSize;
}

bdBool bdGetFilesResult::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt64(m_ID);
	ok = ok && m_data.deserialize(buffer);

	return ok;
}

bdUInt bdGetFilesResult::sizeOf()
{
	return sizeof(bdGetFilesResult);
}

void BD_CALL printInfo(const bdGetFilesResult& fileResult,
					   bdNChar8 *buf,
					   const bdUInt length)
{
	if(length > 0)
	{
		bdSnprintf(buf, length, "FileID : %I64d", fileResult.m_ID);
	}
}
