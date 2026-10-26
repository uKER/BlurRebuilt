// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The interface for bitdemon to the client instance of a game.

#ifndef BD_CLIENT_H
#define BD_CLIENT_H

#include <bdCore/bdCore.h>

#include <bdConnection/bdMessage/bdMessage.h>

#include <bdConnection/bdDispatch/bdDispatchInterceptor.h>

#include <bdClientServer/bdReplica/bdReplicaTable.h>
#include <bdClientServer/bdReplica/bdClientReplicaManager.h>
#include <bdClientServer/bdCommand/bdCommandWindow.h>
#include <bdClientServer/bdPlayer/bdPlayer.h>
#include <bdClientServer/bdClientSettings.h>

class bdClientPlayerListener;
class bdUpdateListener;
class bdCommandGenerator;

BD_REFERENCE(bdClient);
BD_REFERENCE(bdGameInfo);

/// The interface for bitdemon to the client instance of a game.
/// The functions described in this class are used by bitdemon to interact with
/// the game instance.
class bdClient : public bdReferencable,
				 public bdDispatchInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdClient();

		/// Destructor.
		virtual ~bdClient();


		/// Connects to a particular game.
		/// \param game[in] The game we want to connect to.
		void connect(bdGameInfoRef game);

		/// Attempt to join the game with 'player'.
		/// This will start the join sequence which involves an exchange of
		/// messages with the server.
		/// \param player[in]	The player for whom the join should be
		///						attempted.
		/// \todo Add a check so that the join request is not sent if
		///  BD_MAX_CLIENT_PLAYERS have already joined the game.
		void join(const bdPlayerRef player);

		/// Start a controlled departure from the game for 'player'.
		/// This will start the leave sequence which involves an exchange of
		/// messages with the server.
		/// \param player[in]	The player for whom the leave should be
		///						attempted.
		void leave(const bdPlayerRef player);

		/// Register a client player listener object with this class.
		/// All the registered listeners will be notified whenever any
		/// player related events occur. See the bdClientPlayerListener
		/// interface for a full list of player related events.
		/// \param listener[in]	The listener to be registered.
		void registerPlayerListener(bdClientPlayerListener *const listener);

		/// Unregister a previously registered client player listener.
		/// \param listener[in]	The listener to be unregistered.
		void unregisterPlayerListener(bdClientPlayerListener *const listener);

		/// Register an update listener object with this class.
		/// All the registered listeners will be notified whenever
		/// an update is received.
		/// \param listener[in]	The listener to be registered.
		void registerUpdateListener(bdUpdateListener *const listener);

		/// Unregister a previously registered update listener.
		/// \param listener[in]	The listener to be unregistered.
		void unregisterUpdateListener(bdUpdateListener *const listener);

		/// Associates a command generator with the client.
		/// \param commandGenerator[in] A pointer to a user defined command generator.
		void setCommandGenerator(bdCommandGenerator *const commandGenerator);

		/// Sets the time interval by which commands are sampled for a player.
		/// \param interval[in] The time interval in seconds by which getCommand() from
		/// the command generator will be called.
		void setCommandInterval(const bdFloat32 interval);

		/// Gets the current command interval.
		/// \return The command interval in seconds.
		bdFloat32 getCommandInterval() const;

		/// Advances the simulation by the given amount of time.
		/// This function should be called each client frame. elapsedTime
		/// should contain the time elapsed since the function was called last.
		/// \param elapsedTime[in] The amount of time the simulation needs to advance.
		void step(const bdFloat32 elapsedTime);

		/// Get a reference to the player identified by id.
		/// \param playerId[in] The id of the player to get a reference to.
		/// \return
		///  - A reference to the player if one was found.
		///  - BD_NULL otherwise.
		bdPlayerRef getPlayer(const bdPlayerId playerId) const;

		/// Retrieves a pointer to the client replica manager.
		/// \return A pointer to the client replica manager.
		bdClientReplicaManager *getReplicaManager();

		/// Called by the dispatcher to see if this class can handle the
		/// message to be dispatched.
		/// Implemented to conform to the bdDispatchInterceptor interface.
		/// \param receivedMsg[in]	The message to be handled.
		/// \return
		///  - True if this class can, and will handle the message.
		///  - False otherwise.
		virtual bdBool accept(const bdReceivedMessage& receivedMsg);

		/// Specifies the amount of time over which interpolation should occur.
		/// \param interpolationInterval[in] The interpolation interval in seconds
		void setInterpolationInterval(bdFloat32 interpolationInterval);

		/// Returns the connection to the server
		bdConnectionRef getConnection() const;

	protected:

		/// Handle a reply to a join request message that was sent.
		/// \param replyMsg[in] The reply message.
		void handleJoinReply(bdMessageRef replyMsg);

		/// Handle an update from the server.
		/// \param updateMsg[in] The message containing an update.
		void handleUpdate(bdMessageRef updateMsg);

		///
		/// \param replica[in]
		/// \param buffer[in]
		void replay(bdReplicaRef replica,
					bdBitBufferRef buffer);

		/// Handle a player message from the server.
		/// \param playerMsg[in] The player message to handle.
		void handlePlayerMessage(bdMessageRef playerMsg);

	protected:

		/// A table of all the replicas in this instance of the game.
		bdReplicaTable m_replicaTable;

		/// An instance of the client replica manager.
		bdClientReplicaManager m_replicaManager;

		/// All the outstanding join sequences which have not yet completed.
		bdLinkedList<bdPlayerRef> m_pendingJoins;

		/// The next available player id to use.
		bdPlayerId m_pendingId;

		/// A container of all the registered client player listeners.
		bdFastArray<bdClientPlayerListener*> m_playerListeners;

		/// A container of all the registered update listeners.
		bdFastArray<bdUpdateListener*> m_updateListeners;

		/// A container with references to all the players on the client.
		bdPlayerRef m_players[BD_MAX_CLIENT_PLAYERS];

		/// The time period between the sampling of commands.
		bdFloat32 m_commandInterval;

		/// Timer to keep track of the elapsed time.
		bdFloat32 m_commandTimer;

		/// Auxiliary time for command rate locking.
		/// Stores any extra time that was stepped over the
		/// \a m_commandInterval.
		bdFloat32 m_extraStepTime;

		/// A pointer to the user defined command generator.
		bdCommandGenerator *m_commandGenerator;

		/// The connection to the server
		bdConnectionRef m_connection;

};

#endif // BD_CLIENT_H
