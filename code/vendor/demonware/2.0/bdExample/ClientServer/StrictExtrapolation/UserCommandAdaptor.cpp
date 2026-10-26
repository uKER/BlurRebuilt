// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "UserCommandAdaptor.h"

void UserCommandAdaptor::setUserInput(const PlayerInput& input)
{
	m_input = input;
}

const PlayerInput& UserCommandAdaptor::getUserInput() const
{
	return m_input;
}

void UserCommandAdaptor::clear()
{
	bdUserCommand::clear();
	m_input.clear();
}

void UserCommandAdaptor::serialize(bdBitBufferRef buffer, 
								   bdUserCommand *previousCommand)
{
	bdUserCommand::serialize(buffer, previousCommand);
	buffer->writeBool(m_input.m_keys[0]);
	buffer->writeBool(m_input.m_keys[1]);
	buffer->writeBool(m_input.m_keys[2]);
	buffer->writeBool(m_input.m_keys[3]);
}

bdBool UserCommandAdaptor::deserialize(bdBitBufferRef buffer)
{
	if (bdUserCommand::deserialize(buffer))
	{
		buffer->readBool(m_input.m_keys[0]);
		buffer->readBool(m_input.m_keys[1]);
		buffer->readBool(m_input.m_keys[2]);
		buffer->readBool(m_input.m_keys[3]);
	}
	return (!buffer->failedRead());
}
