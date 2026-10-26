// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdContentStreaming/bdFileMetaData.h>

bdFileMetaData::bdFileMetaData()
: m_fileID(0),
  m_createTime(0),
  m_modifedTime(0),
  m_fileSize(0),
  m_ownerID(0),
  m_fileSlot(0),
  m_numTags(0),
  m_thumbNailFileID(0),
  m_thumbNailDataSize(0)
{
	resetArrays();
}

bdFileMetaData::~bdFileMetaData()
{
}

void bdFileMetaData::reset()
{
	m_fileID = 0;
	m_fileSlot = 0;
	m_createTime = 0;
	m_modifedTime = 0;
	m_fileSize = 0;
	m_ownerID = 0;
	m_numTags = 0;
	resetArrays();
}

void bdFileMetaData::resetArrays()
{
	bdMemset(m_fileName, 0, BD_MAX_FILE_NAME_LENGTH);
	bdMemset(m_url, 0, BD_MAX_URL_SIZE);
	bdMemset(m_ownerName, 0, BD_MAX_USERNAME_LENGTH);

	for(bdUInt i=0; i<m_numTags; i++)
	{
		m_tags[i].set(0,0);
	}

}

bdBool bdFileMetaData::deserialize(bdByteBufferRef buffer)
{
	reset();

	bdBool ok = true;
	ok = ok && buffer->readUInt64(m_fileID);
	ok = ok && buffer->readUInt32(m_createTime);
	ok = ok && buffer->readUInt32(m_modifedTime);
	ok = ok && buffer->readUInt32(m_fileSize);
	ok = ok && buffer->readUInt64(m_ownerID);
	ok = ok && buffer->readString(m_ownerName, BD_MAX_USERNAME_LENGTH);
	ok = ok && buffer->readUInt16(m_fileSlot);
	ok = ok && buffer->readString(m_fileName, BD_MAX_FILE_NAME_LENGTH);
	ok = ok && buffer->readString(m_url, BD_MAX_URL_SIZE);
	ok = ok && buffer->readUInt16(m_category);
	ok = ok && buffer->readUInt64(m_thumbNailFileID);
	ok = ok && buffer->readUInt32(m_thumbNailDataSize);
	
	ok = ok && buffer->readArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, m_numTags);

	if (m_numTags != 0)
	{
		m_numTags /= 2;
	}

	for (bdUInt i(0); i < m_numTags && ok; i++)
	{
		ok = ok && m_tags[i].deserialize(buffer);
	}
	buffer->readArrayEnd();

	return ok;
}

bdUInt bdFileMetaData::sizeOf()
{
	return sizeof(bdFileMetaData);
}

