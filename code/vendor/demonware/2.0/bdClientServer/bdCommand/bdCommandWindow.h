// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A container for a sequence of commands.

#ifndef BD_COMMAND_WINDOW_H
#define BD_COMMAND_WINDOW_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdCommand/bdCommandSettings.h>
#include <bdClientServer/bdCommand/bdUserCommand.h>
#include <bdClientServer/bdCommand/bdCommandGenerator.h>

//class bdCommandGenerator;

/// A container to store a sequence of commands.
class bdCommandWindow
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdCommandWindow();

		/// Destructor.
		~bdCommandWindow();

		bdUserCommand* getSpareCommand() const;

		void addSpareCommand();

		void writeCommands(bdBitBufferRef buffer) const;

		bdBool inWindow(const bdCommandSeqNum commandNum) const;

		bdCommandSeqNum getCommandsSince(const bdCommandSeqNum commandNum) const;

		bdUserCommand* get(const bdCommandSeqNum commandNum) const;

		/// Returns the sequence number pf the last command in the window.
		/// \return The sequence of the most recent command.
		bdCommandSeqNum getCommandNum() const;

		/// Reset the command window.
		/// This will clear all the entries and reset the appropriate counter.
		void reset();

	protected:
		
		/// A container for the commands in the window.
		bdUserCommand *m_commands[BD_COMMAND_HISTORY_SIZE];

		bdUserCommand *m_nullCommand;

		/// The index of where the next expected command will be placed.
		bdCommandSeqNum m_nextCommandNum;
};

#endif // BD_COMMAND_WINDOW_H
