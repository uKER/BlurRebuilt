// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "UserCommand.h"

UserCommand::UserCommand()
: m_time(0)
{
	m_keys[0] = false;
	m_keys[1] = false;
	m_keys[2] = false;
	m_keys[3] = false;
}

void UserCommand::serialize(bdBitBufferRef buffer)
{
	bdCommand::serialize(buffer);
	buffer->writeBool(m_keys[0]);
	buffer->writeBool(m_keys[1]);
	buffer->writeBool(m_keys[2]);
	buffer->writeBool(m_keys[3]);
	buffer->writeUByte8(m_time);
}

bdBool UserCommand::deserialize(bdBitBufferRef buffer)
{
	bdCommand::deserialize(buffer);
	buffer->readBool(m_keys[0]);
	buffer->readBool(m_keys[1]);
	buffer->readBool(m_keys[2]);
	buffer->readBool(m_keys[3]);
	buffer->readUByte8(m_time);

	return (!buffer->failedRead());
}
