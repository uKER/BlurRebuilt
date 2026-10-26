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

	// get the input from the previous command
	const UserCommandAdaptor *const lastCommand = static_cast<UserCommandAdaptor*>(previousCommand);
	const PlayerInput& previousInput = lastCommand->getUserInput();

	// only write the command if it has changed from the previous input
	for(bdUInt32 i = 0; i < 4; i++)
	{
		if(buffer->writeBool(m_input.m_keys[i] != previousInput.m_keys[i]))
		{
			buffer->writeBool(m_input.m_keys[i]);
		}
	}
}
//end_of_code

bdBool UserCommandAdaptor::deserialize(bdBitBufferRef buffer)
{
	if (bdUserCommand::deserialize(buffer))
	{
		for(bdUInt32 i = 0; i < 4; i++)
		{
			if(buffer->testBool())
			{
				buffer->readBool(m_input.m_keys[i]);
			}
		}
	}

	return !buffer->failedRead();
}
//end_of_code
