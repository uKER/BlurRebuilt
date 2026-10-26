// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdCommand/bdCommandWindow.h>

#include <bdClientServer/bdCommand/bdCommandFactory.h>

bdCommandWindow::bdCommandWindow()
: m_nextCommandNum(1)
{
	bdCheckCommandSettings();

	for(bdCommandSeqNum i = 0; i < BD_COMMAND_HISTORY_SIZE; i++)
	{		
		m_commands[i] = bdCommandFactory::getInstance()->create();
		m_commands[i]->clear();
	}

	m_nullCommand = bdCommandFactory::getInstance()->create();
	m_nullCommand->clear();
}

bdCommandWindow::~bdCommandWindow()
{
	for(bdCommandSeqNum i = 0; i < BD_COMMAND_HISTORY_SIZE; i++)
	{
		bdUserCommand *const command = m_commands[i];
		if(command)
		{
			delete command;
		}
	}

	if(m_nullCommand)
	{
		delete m_nullCommand;
	}
}

bdUserCommand* bdCommandWindow::getSpareCommand() const
{
	// it not really necessary to clear the command, but it's nice and maybe 
	// expected.
	m_nullCommand->clear();
	return m_nullCommand;
}

void bdCommandWindow::addSpareCommand()
{
	// add the command to the window
	const bdCommandSeqNum nextIndex = m_nextCommandNum & (BD_COMMAND_HISTORY_SIZE-1);
	bdUserCommand *command = m_commands[nextIndex];

	// just swap the current command with the spare command
	bdUserCommand *const backup = command;
	m_commands[nextIndex] = m_nullCommand;
	m_nullCommand = backup;

	m_nextCommandNum++;
}

void bdCommandWindow::writeCommands(bdBitBufferRef buffer) const
{
	const bdCommandSeqNum cmdNum = m_nextCommandNum-1;

	// write a sequence number to the message
	buffer->BD_COMMAND_SEQ_NUM_WRITE(cmdNum);
	
	// get the index of the command BD_COMMAND_REDUNDANCY commands ago 
	bdCommandSeqNum index = (cmdNum - (BD_COMMAND_REDUNDANCY)) & (BD_COMMAND_HISTORY_SIZE-1);
	
	// Use the nullCommand to combine the commands together
	bdUserCommand *previousCommand = m_nullCommand;
	previousCommand->clear();

	const bdUInt32 savedReadPos = buffer->getReadPosition();
	// Set read position so subsequent reads read only what's written in here.
	buffer->setReadPosition(buffer->getWritePosition());

	// Iterate through previous commands an serialize them
	for(bdUInt32 i = 0; i <= BD_COMMAND_REDUNDANCY; i++)
	{
		bdUserCommand *const command = m_commands[index];
		command->serialize(buffer, previousCommand);

		previousCommand->deserialize(buffer);

		index = (index + 1) & (BD_COMMAND_HISTORY_SIZE-1);		
	}	

	buffer->setReadPosition(savedReadPos);
}

bdBool bdCommandWindow::inWindow(const bdCommandSeqNum commandNum) const
{
	return static_cast<bdCommandSeqNum>((m_nextCommandNum - 1) - commandNum) < BD_COMMAND_HISTORY_SIZE;
}

bdCommandSeqNum bdCommandWindow::getCommandsSince(const bdCommandSeqNum commandNum) const
{
	bdCommandSeqNum commandsSince = 0;
	if(inWindow(commandNum))
	{
		commandsSince = static_cast<bdCommandSeqNum>((m_nextCommandNum - 1) - commandNum);
	}

	return commandsSince;
}

bdUserCommand* bdCommandWindow::get(const bdCommandSeqNum commandNum) const
{
	bdUserCommand *command = BD_NULL;
	if (inWindow(commandNum))
	{
		const bdCommandSeqNum index = commandNum & (BD_COMMAND_HISTORY_SIZE-1);
		command = m_commands[index];
	}
	return command;
}

bdCommandSeqNum bdCommandWindow::getCommandNum() const
{
	return m_nextCommandNum - 1;
}

void bdCommandWindow::reset()
{
//	m_nextCommandNum = 1;
	for(bdCommandSeqNum i = 0; i < BD_COMMAND_HISTORY_SIZE; i++)
	{
		m_commands[i]->clear();
	}

	// clear the null command just to be sure
	if(m_nullCommand)
	{
		m_nullCommand->clear();
	}
}
