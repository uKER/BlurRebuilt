// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An interface for notifications about player manager related events.

#ifndef BD_PLAYER_MANAGER_LISTENER_H
#define BD_PLAYER_MANAGER_LISTENER_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdPlayer);

/// This class defines a listener interface for events that may occur in a
/// bdPlayerManager. Classes derived from bdPlayerManagerListener can be
/// registered using bdPlayerManager::registerListener.
class bdPlayerManagerListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdPlayerManagerListener();

		/// Virtual destructor.
		virtual ~bdPlayerManagerListener();

		/// Called when a new player is added.
		/// \param player[in]	The player new player.
		virtual void onPlayerJoin(const bdPlayerRef player);

		/// Called when a player is removed.
		/// \param player[in]	The player that has left.
		virtual void onPlayerLeave(const bdPlayerRef player);
};

#endif // BD_PLAYER_MANAGER_LISTENER_H
