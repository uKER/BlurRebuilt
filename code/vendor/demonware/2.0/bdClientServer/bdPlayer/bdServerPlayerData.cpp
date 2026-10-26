// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdPlayer/bdServerPlayerData.h>

#include <bdClientServer/bdCommand/bdCommandFactory.h>
#include <bdConnection/bdConnection.h>

bdServerPlayerData::bdServerPlayerData()
: m_commandSeqNum(0)
{
	for(bdUInt32 i = 0; i < BD_COMMAND_REDUNDANCY + 1; i++)
	{
		m_commands[i] = bdCommandFactory::getInstance()->create();
	}
}

bdServerPlayerData::~bdServerPlayerData()
{
	for(bdUInt32 i = 0; i < BD_COMMAND_REDUNDANCY + 1; i++)
	{
		delete m_commands[i];
	}
}

void bdServerPlayerData::setConnection(bdConnectionRef connection)
{
	m_connection = connection;
}

const bdConnectionRef bdServerPlayerData::getConnection() const
{
	return m_connection;
}

void bdServerPlayerData::setCommandSeqNum(const bdCommandSeqNum commandSeqNum)
{
	m_commandSeqNum = commandSeqNum;
}

bdCommandSeqNum bdServerPlayerData::getCommandSeqNum() const
{
	return m_commandSeqNum;
}

bdUserCommand** bdServerPlayerData::getCommands()
{
	return m_commands;
}

void bdServerPlayerData::resetCommands()
{
	for(bdUInt32 i = 0; i < BD_COMMAND_REDUNDANCY + 1; i++)
	{
		m_commands[i]->clear();
	}
}
