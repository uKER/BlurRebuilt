// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The bitdemon representation of a player in a game.

#ifndef BD_PLAYER_H
#define BD_PLAYER_H

#include <bdCore/bdCore.h>


#include <bdClientServer/bdReplica/bdReplica.h>
#include <bdClientServer/bdPlayer/bdServerPlayerData.h>
#include <bdClientServer/bdCommand/bdCommandSettings.h>
#include <bdClientServer/bdCommand/bdCommandWindow.h>

/// The data type to use for bdPlayer numbering.
/// This should be an unsigned type.
typedef bdUByte8 bdPlayerId;
#define BD_PLAYER_ID_READ readUByte8
#define BD_PLAYER_ID_WRITE writeUByte8

BD_REFERENCE(bdPlayer);

/// The bitdemon representation of a player in a game.
class bdPlayer : public bdReferencable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS
		
		/// Bit flags to indicate what behaviour the player has.
		enum bdPlayerSettings
		{
			BD_DO_CLIENT_SIDE_PREDICTION	= (1 << 0),			
			BD_MAX_PLAYER_SETTING			= (1 << 1)
		};

		/// Default constructor
		bdPlayer();

		/// Virtual destructor.
		virtual ~bdPlayer();

		/// Specify the replica that this player is currently controlling.
		/// \param replica[in]	The replica that is being controlled.
		void setControlledReplica(bdReplicaRef replica);

		/// Accessor function for the replica currently under the control of 
		/// this player.
		/// \return The currently controlled replica.
		bdReplicaRef getControlledReplica() const;

		/// Set the id of this player.
		/// \param id[in] The new identifier of this player.
		void setId(const bdPlayerId id);

		/// Accessor function for the identity of this player.
		/// \return The id of this player.
		bdPlayerId getId() const;

		/// Serializes the object's state into a bitbuffer.
		/// \param	buffer[in]	The bdBitBuffer into which the state is 
		///						serialized.
		virtual void serialize(bdBitBufferRef buffer);


		/// Deserializes the object's state from a bitbuffer.
		/// \param	buffer[in]	The bdBitBuffer from which the state is 
		///						deserialized.
		/// \return 
		/// - True if the state could be read succesfully.
		/// - False otherwise.
		virtual bdBool deserialize(bdBitBufferRef buffer);

		/// Create a client command window for the player.
		/// \return A pointer to the newly created command window.
		bdCommandWindow* createClientCommandWindow();

		/// Accessor function for the client command window of this player.
		/// \return A pointer to the command window.
		bdCommandWindow* getClientCommandWindow();

		/// Create an instance of the server data for this player.
		/// \return A pointer to the newly created server player data.
		bdServerPlayerData* createServerData();

		/// Accessor function for the server data for this player.
		/// \return A pointer to the server player data.
		bdServerPlayerData* getServerData();

		/// Replay a number of commands.
		/// This will replay all commands between 'fromCommand' to the most
		/// recent one that has been seen.
		/// \param fromCommand[in]	Sequence number of the command to start 
		///							the replay from.
		void replay(const bdCommandSeqNum fromCommand);

		/// Set settings for this player.
		/// \param settings[in] Bitfield flag representing the settings.
		void setSettings(const bdUInt32 settings);

		/// Clear settings for this player.
		/// \param settings[in] Bitfield flag representing the settings.
		void clearSettings(const bdUInt32 settings);

		/// Check if the specified setting is set/active.
		/// \param settings[in] Bitfield flag representing the settings to
		///						test for.
		/// \return
		///   - True if the setting is active.
		///   - False otherwise.
		bdBool testSettings(const bdUInt32 settings) const;

	protected:

		/// An integer to store the player settings.
		bdUInt32 m_settings;

		/// The identifier of this player.
		bdPlayerId m_id;

		/// The replica that this player is currently controlling.		
		bdReplicaRef m_controlledReplica;
		
		/// A pointer to the players command window if it exists.
		/// Only players hosted on the local machine have command windows.
		bdCommandWindow *m_clientCommandWindow;

		/// A pointer to the player server data for this object if it exists.
		bdServerPlayerData *m_serverData;
};


#endif // BD_PLAYER_H
