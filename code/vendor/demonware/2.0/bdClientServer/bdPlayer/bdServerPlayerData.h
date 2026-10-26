// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: This class contains additional information about a player that is needed
// on the server.

#ifndef BD_SERVER_PLAYER_DATA_H
#define BD_SERVER_PLAYER_DATA_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdCommand/bdCommandSettings.h>
#include <bdClientServer/bdCommand/bdUserCommand.h>
#include <bdClientServer/bdReplica/bdUpdate/bdUpdateWindow.h>

BD_REFERENCE(bdConnection);

/// This class contains additional information about a player that is needed
/// on the server. The information includes
/// - The host on which the player is located
/// - A sliding window of updates about the replica that is controlled by the player
/// - A list of commands that have been issued by the player
/// - The amount of simulation time that has been performed on the replica that is
///   controlled by the player
class bdServerPlayerData
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdServerPlayerData();

		/// Destructor.
		~bdServerPlayerData();

		/// Specifies the connection from which the player has connected.
		/// \param connection[in] The player's connection
		void setConnection(bdConnectionRef connection);

		/// Retrieves the connection from which the player has connected.
		/// \return The player's connection
		const bdConnectionRef getConnection() const;

		/// Sets the sequence number of the last command that has been
		/// received from the player.
		/// \param commandSeqNum[in] An 8-bit sequence number
		void setCommandSeqNum(const bdCommandSeqNum commandSeqNum);

		/// Gets the sequence number of the last command that has been
		/// received from the player.
		/// \return The last see command sequence number.
		bdCommandSeqNum getCommandSeqNum() const;

		/// Accessor function for the array of commands for this player.
		/// \return A pointer to the array of commands.
		bdUserCommand** getCommands();

		/// Reset all the commands associated with this player.
		/// This will call clear on all the commands in the command array.
		void resetCommands();

	protected:

		/// The player's connection
		bdConnectionRef m_connection;

		/// The last command sequence number received.
		bdCommandSeqNum m_commandSeqNum;

		/// The last set of commands we received from the player.
		/// We may reapply the newest of these commands if we detect any
		/// packets have been dropped. In case commands are delta compressed
		/// against each other we store the entire command received from
		/// the client.
		bdUserCommand *m_commands[BD_COMMAND_REDUNDANCY+1];
};

#endif // BD_SERVER_PLAYER_DATA_H
