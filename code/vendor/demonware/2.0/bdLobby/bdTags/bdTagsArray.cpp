// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTags/bdTagsArray.h>

bdTagsArray::bdTagsArray()
: m_entityID(0),
  m_numTags(0) 
{
}

bdTagsArray::~bdTagsArray()
{
}

void bdTagsArray::serialize(bdByteBuffer &buffer)const
{
	buffer.writeArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, m_numTags*2, sizeof(bdUInt32)); // 2 ints per bdTag(pri/sec)
	for(bdUInt i=0; i<m_numTags; i++)
	{
		m_tags[i].serialize(buffer);
	}
	buffer.writeArrayEnd();
}

bdBool bdTagsArray::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;

	ok = ok && buffer->readUInt64(m_entityID);

	ok = ok && buffer->readArrayStart(BD_BB_UNSIGNED_INTEGER32_TYPE, m_numTags);

	// numtags returned from server is total tags elements, m_tags actually has half that (priTag,secTag)
	m_numTags = m_numTags/2;
	
	for(bdUInt i=0; i<m_numTags && ok; i++)
	{
		ok = ok && m_tags[i].deserialize(buffer);
	}
	buffer->readArrayEnd();
	return ok;
}

bdUInt bdTagsArray::sizeOf()
{
	return sizeof(bdTagsArray);
}

void BD_CALL printInfo(const bdTagsArray& tags,
					   bdNChar8 *buf,
					   const bdUInt length)
{
	if(length > 0)
	{
		bdUInt32 written(0);
		written += bdSnprintf(buf, length, "FileID[%I64u] : Tags{", tags.m_entityID);
		for(bdUInt i=0; i<tags.m_numTags; i++)
		{
			written += bdSnprintf(buf + written, length - written, "[%d,%d] ", tags.m_tags[i].m_priTag, tags.m_tags[i].m_secTag);
		}
		written += bdSnprintf(buf+written, length-written, "}");
	}
}
