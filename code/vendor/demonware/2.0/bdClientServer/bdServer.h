// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: The interface for bitdemon to the server instance of a game.

#ifndef BD_SERVER_H
#define BD_SERVER_H

#include <bdCore/bdCore.h>

#include <bdConnection/bdMessage/bdMessage.h>
#include <bdConnection/bdConnectionListener.h>

#include <bdConnection/bdDispatch/bdDispatchInterceptor.h>

#include <bdClientServer/bdReplica/bdReplicaTable.h>
#include <bdClientServer/bdReplica/bdReplica.h>
#include <bdClientServer/bdReplica/bdServerReplicaManager.h>
#include <bdClientServer/bdPlayer/bdPlayer.h>


class bdInterestInterface;
class bdServerPlayerListener;

BD_REFERENCE(bdServer);
BD_REFERENCE(bdGameInfo);

/// The interface for bitdemon to the server instance of a game.
/// The functions described in this class are used by bitdemon to interact with
/// the server game instance.
class bdServer : public bdReferencable,
				 public bdDispatchInterceptor,
				 public bdConnectionListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdServer();

		/// Destructor.
		virtual ~bdServer();

		/// Initializes the server with the specified game info
		/// \param gameInfo[in] : The game info to be used by the server.
		/// This essentially registers the security key/id pair that clients
		/// will need in order to be able to connect
		void init(bdGameInfoRef gameInfo);

		/// Register a server player listener object with this class.
		/// All registered listeners will be notified whenever any
		/// player related events occur. See the bdServerPlayerListener
		/// interface for a full list of player related events.
		/// \param listener[in]	The listener to be registered.
		void registerPlayerListener(bdServerPlayerListener *const listener);

		/// Unregister a previously registered server player listener.
		/// \param listener[in]	The listener to be unregistered.
		void unregisterPlayerListener(bdServerPlayerListener *const listener);

		/// Set the server's update interval.
		/// The update interval specifies the time interval at which the server
		/// sends updates.
		/// \param secs[in]	The update interval in seconds.
		void setUpdateInterval(const bdFloat32 secs);

		/// Accessor function for the replica manager that is used by the
		/// server.
		/// \return A pointer to the replica manager.
		const bdServerReplicaManager* getReplicaManager() const;

		/// Accessor function for the replica manager that is used by the
		/// server.
		/// \return A pointer to the replica manager.
		bdServerReplicaManager* getReplicaManager();

		/// Set the 'replica' that is controlled by 'player'.
		/// \param player[in]	The player that controls the replica.
		/// \param replica[in]	The replica that is being controlled.
		void setControlledReplica(bdPlayerRef player,
								  bdReplicaRef replica);

		/// Gets the player that controls the given replica.
		/// \param replica[in] The replica that is controlled
		/// \return The player that controls this replica, if any
		bdPlayerRef getControllingPlayer(bdReplicaRef replica);

		/// Advances the simulation by the given amount of time.
		/// This function should be called each server frame. elapsedTime
		/// should contain the time elapsed since the function was called last.
		/// If the update interval has expired (as specified by the
		/// setUpdateInterval() function), this function will check for dirty
		/// replicas and send out updates to all interested hosts.
		/// \param elapsedTime[in]	The amount of time, in seconds, the
		///							simulation needs to advance.
		void step(const bdFloat32 elapsedTime);

		/// Same as above, but used for simulations that have no concept of
		/// time.
		void step();

		/// Accessor function for a player object identified by 'id'.
		/// \param playerId[in]	The id of the player that is being sought.
		/// \return
		///  - A reference to the player being sought if it was found.
		///  - BD_NULL otherwise.
		/// \todo Make this lookup O(1), rather than order O(n) as it
		///       currently is.
		bdPlayerRef getPlayer(const bdPlayerId playerId) const;

		/// Called by the dispatcher to see if this class can handle the
		/// message to be dispatched.
		/// Implemented to conform to the bdDispatchInterceptor interface.
		/// \param receivedMsg[in]	The message to be handled.
		/// \return
		///  - True if this class can, and will handle the message.
		///  - False otherwise.
		virtual bdBool accept(const bdReceivedMessage& receivedMsg);

		/// Called when a connection is disconnected.
		/// Implemented to conform to the bdConnectionListener interface.
		/// \param connection[in] The disconnected connection.
		virtual void onDisconnect(bdConnectionRef connection);

		/// Sets the server's interest.
		/// Assuming interest is valid (ie not null) the server will
		/// switch to use the given interest interface. The server assumes
		/// ownership of \a interest, so it should be heap allocated and
		/// you do not need to delete it. The bdServer will delete
		/// \a interest when it is destroyed or when setInterest is called
		/// again with a non-null pointer.
		/// \param interest[in] The interest to be used by the server.
		void setInterest(bdInterestInterface *const interest);

		/// Accessor for the replica table.
		bdReplicaTable* getReplicaTable();

	protected:

		/// Handle a player message that has been received.
		/// \param receivedMsg[in]	The message to handle.
		void handlePlayerMessage(const bdReceivedMessage& receivedMsg);

		/// Handle a join request message from a player.
		/// \param receivedMsg[in]
		/// \todo This should use a proper id manager
		void handlePlayerJoin(const bdReceivedMessage& receivedMsg);

		/// Handle a leave notification message from a player.
		/// \param leaveMsg[in]
		void handlePlayerLeave(bdMessageRef leaveMsg);

		/// Handle an acknowledgement for an update that was sent to a client.
		/// \param receivedMsg[in]
		void handleUpdateAck(const bdReceivedMessage& receivedMsg);

		/// Handle a command message from a player.
		/// \param player[in]		The player to which the command pertains.
		/// \param commandMsg[in]	The message containing the command.
		void handleUserCommandMessage(bdPlayerRef player,
									  bdMessageRef commandMsg);

		/// Update all the replicas. This function iterates through all current
		/// replicas and checks if they are flagged as dirty. This function is
		/// called as part of the server's step() function.
		/// \param time[in] : The elapsed time, in seconds, since this function
		///					  was last called.
		/// \param sendUpdates[in] : Flag to indicate whether updates are to be sent.
		///		If set to false, updateReplicas() will not send updates, but still
		///     might extrapolate replicas for the given amount of time.
		void updateReplicas(const bdFloat32 time,
							const bdBool sendUpdates);

		/// Remove a player from the game/server.
		/// This will remove all the player related data and inform all
		/// clients that the player has left the game.
		/// \param player[in] The player to be removed.
		void removePlayer(bdPlayerRef player);

		/// Create a 'join' message to send to all hosts.
		/// \param player[in]	The newly joined player.
		/// \return	A reference to the message.
		bdMessageRef createJoinMessage(bdPlayerRef player) const;

		/// Create a 'leave' message to send to all hosts.
		/// \param player[in]	The departed/departing player.
		/// \return	A reference to the message.
		bdMessageRef createLeaveMessage(bdPlayerRef player) const;



	protected:

		/// The time interval at which the server sends update.
		bdFloat32 m_updateInterval;

		/// A table of all replicas managed by the server..
		bdReplicaTable m_replicaTable;

		/// An instance of the server replica manager.
		bdServerReplicaManager m_replicaManager;

		/// A record of how long since an update was last sent.
		bdFloat32 m_processTime;

		/// A pointer to the interest object that this class uses.
		bdInterestInterface *m_interest;

		/// A simple implementation of an id manager.
		/// \todo Remove this, and replace with a proper id manager.
		static bdPlayerId m_maxPlayerId;

		/// A container for all the players on the server.
		bdArray<bdPlayerRef> m_players;

		/// A list of all current connections.
		bdArray<bdConnectionRef> m_connections;

		/// A map of bdReplica to their controlling players.
		/// Is used to achieve an O(1) lookup.
		bdHashMap<bdReplicaRef, bdPlayerRef> m_controlMap;

		/// A container for all the registered server player listeners.
		bdFastArray<bdServerPlayerListener*> m_playerListeners;
};

#endif // BD_SERVER_H
