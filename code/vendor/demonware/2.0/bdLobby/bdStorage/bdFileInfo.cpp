// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdStorage/bdFileInfo.h>

bdFileInfo::bdFileInfo()
:	m_fileID(0),
	m_createTime(0),
	m_modifedTime(0),
	m_visibility(BD_MAX_VISIBILITY_TYPE),
	m_ownerID(0),
	m_fileSize(0)
{
	bdMemset(m_fileName, 0, BD_MAX_FILE_NAME_LENGTH);
}

bdFileInfo::~bdFileInfo()
{
}

bdBool bdFileInfo::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_fileSize);
	ok = ok && buffer->readUInt64(m_fileID);
	ok = ok && buffer->readUInt32(m_createTime);
	ok = ok && buffer->readUInt32(m_modifedTime);

	bdBool visibilityPrivate = false;
	ok = ok && buffer->readBool(visibilityPrivate);
	if(visibilityPrivate)
	{
		m_visibility = bdFileInfo::BD_VISIBLE_PRIVATE;
	}
	else
	{
		m_visibility = bdFileInfo::BD_VISIBLE_PUBLIC;
	}

	ok = ok && buffer->readUInt64(m_ownerID);
	ok = ok && buffer->readString(m_fileName, BD_MAX_FILE_NAME_LENGTH);
	return ok;
}

bdUInt bdFileInfo::sizeOf()
{
	return sizeof(bdFileInfo);
}


