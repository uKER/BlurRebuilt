// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines a number of notification functions that deal with player 
//			related events on a client.

#ifndef BD_CLIENT_PLAYER_LISTENER_H
#define BD_CLIENT_PLAYER_LISTENER_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdPlayer/bdPlayer.h>

/// This listener class defines a number of notification functions that
/// deal with player related events on a client.
class bdClientPlayerListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor.
		virtual ~bdClientPlayerListener();

		/// Notifies the client of any new players that have joined the game.
		/// \param player[in]	The new player that has joined.
		virtual void onClientPlayerJoin(bdPlayerRef player);

		/// Notifies the client of any players that have left the game.
		/// \param playerId[in]	The ID of player that has left.
		virtual void onClientPlayerLeave(const bdPlayerId playerId);

		/// The response from the server about a join request for a player.
		/// \param player[in]	The player that requested to join
		/// \param success[in]	A Flag to indicate whether the player was 
		///						allowed to join or not.
		virtual void onClientJoinReply(bdPlayerRef player,
									   const bdBool success);

		/// Notifies a client that control over a replica has been granted 
		/// for one of its players.
		/// \param player[in]	The player in question.
		/// \param prevReplica[in]	The replica that the player was in control of previously.
		/// \note: to get the current replica that the player controls call
		/// bdPlayer::getControlledReplica()
		virtual void onClientSetControl(bdPlayerRef player, 
										bdReplicaRef prevReplica);
};

#endif // BD_CLIENT_PLAYER_LISTENER_H
