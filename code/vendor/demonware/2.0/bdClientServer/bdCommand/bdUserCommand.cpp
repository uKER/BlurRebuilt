// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdCommand/bdUserCommand.h>

#define BD_LOG_LEVEL "bdClientServer/usercommand"

bdUserCommand::bdUserCommand(const bdUByte8 msec)
: m_msec(msec)
{

}

bdUserCommand::~bdUserCommand()
{

}

void bdUserCommand::setTime(const bdUByte8 msec)
{
	m_msec = msec;
}

void bdUserCommand::setTimeInSeconds(const bdFloat32 sec)
{
	bdFloat32 tmp = sec * 1000.0f;
	if (tmp > BD_UBYTE8_MAX)
	{
		BD_ERR(BD_LOG_LEVEL, "command time > 255 ms");
	}
	m_msec = static_cast<bdUByte8>(tmp);
}

bdUByte8 bdUserCommand::getTime() const
{
	return m_msec;
}

bdFloat32 bdUserCommand::getTimeInSeconds() const
{
	return static_cast<bdFloat32>(m_msec) / 1000.0f;
}

void bdUserCommand::clear()
{
	m_msec = 0;
}

void bdUserCommand::serialize(bdBitBufferRef buffer, bdUserCommand *previousCommand)
{
	if(buffer->writeBool(previousCommand->m_msec != m_msec))
	{
		buffer->writeUByte8(m_msec);
	}
}

bdBool bdUserCommand::deserialize(bdBitBufferRef buffer)
{
	bdBool ok = true;
	if(buffer->testBool())
	{
		ok = buffer->readUByte8(m_msec);
	}

	return ok;
}
