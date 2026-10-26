// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Defines a number of notification functions that deal with player
//			related events on a server.

#ifndef BD_SERVER_PLAYER_LISTENER_H
#define BD_SERVER_PLAYER_LISTENER_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdPlayer/bdPlayer.h>

class bdServer;

BD_REFERENCE(bdConnection);

/// This listener class defines a number of notification functions that
/// deal with player related events on a server.
class bdServerPlayerListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor.
		virtual ~bdServerPlayerListener();

		/// Notifies the server about a player requesting to join the game.
		/// \param server[in]		A pointer to the server interface object
		///							that handles the request
		/// \param connection[in]	The host from where the request came
		/// \return
		///	  - True  : the player is allowed to join
		///	  - False : the player was denied to join
		virtual bdBool onServerPlayerJoinRequest(bdServer *const server,
												 bdConnectionRef connection);

		/// Allows the server to setup a player
		/// \param server[in]	A pointer to the server interface object
		///						that handles the setup
		/// \param player[in]	The player in question
		virtual void onServerSetupPlayer(bdServer *const server,
										 bdPlayerRef player);

		/// Notifies the server about a new player who has joined
		/// \param server[in]	A pointer to the server interface object
		///						that handles the setup
		/// \param  player[in]	The player in question
		virtual void onServerPlayerJoin(bdServer *const server,
										bdPlayerRef player);

		/// Notifies the server about a player who has left the game
		/// \param server[in]	A pointer to the server interface object
		///						that handles the player
		/// \param player[in]	The player in question
		virtual void onServerPlayerLeave(bdServer *const server,
										 bdPlayerRef player);
};

#endif // BD_SERVER_PLAYER_LISTENER_H

