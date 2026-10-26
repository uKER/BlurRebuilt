// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdLobby/bdTags/bdTag.h>

bdTag::bdTag()
: m_priTag(0),
  m_secTag(0)
{
}

bdTag::bdTag(bdInt priTag, bdInt secTag)
: m_priTag(priTag),
  m_secTag(secTag)
{
}

bdTag::~bdTag()
{
}

void bdTag::serialize(bdByteBuffer &buffer) const
{	
	buffer.writeUInt32(m_priTag);
	buffer.writeUInt32(m_secTag);
}

bdBool bdTag::deserialize(bdByteBufferRef buffer)
{
	bdBool ok = true;
	ok = ok && buffer->readUInt32(m_priTag);
	ok = ok && buffer->readUInt32(m_secTag);
	return ok;
}

bdUInt bdTag::sizeOf()
{
	return sizeof(bdTag);
}

void bdTag::set(bdUInt32 priTag, bdUInt32 secTag)
{
	m_priTag = priTag;
	m_secTag = secTag;
}
