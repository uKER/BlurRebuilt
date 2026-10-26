// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Handles management of bdPlayers in a peer-to-peer system.

#ifndef BD_PLAYER_MANAGER_H
#define BD_PLAYER_MANAGER_H

#include <bdCore/bdCore.h>

#include <bdPeer/bdSession/bdSessionInterceptor.h>

#include <bdPeer/bdDelivery/bdDeliveryManagerListener.h>
#include <bdPeer/bdDelivery/bdDeliveryInterceptor.h>
#include <bdPeer/bdDelivery/bdDeliveryInitializer.h>

BD_REFERENCE(bdPlayer);

class bdDeliveryManager;
class bdPlayerManagerListener;

/// Manages the addition and removal of players within a peer-to-peer system.
/// bdPlayerManager uses the bdPlayerFactory singleton class to create
/// bdPlayer instances. Use the BD_REGISTER_PLAYER_CLASS macro to register
/// a player class, derived from bdPlayer, with the factory.
class bdPlayerManager : public bdDeliveryManagerListener,
						public bdDeliveryInterceptor,
						public bdDeliveryInitializer,
						public bdSessionInterceptor
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor.
		/// \param deliveryManager[in] A pointer to the bdDeliveryManager to
		///		use for all communication.
		bdPlayerManager();

		/// Virtual destructor.
		virtual ~bdPlayerManager();

		void init(bdDeliveryManager *const deliveryManager);

		void quit();

		/// Add a player to the system.
		/// Adding players is A-synchronous, register a
		/// bdPlayerManagerListener to be informed of new players.
		/// Upon addition of the first player the underlying bdDeliveryManager
		/// will be activated, if required.
		/// \param player[in] A reference to a bdPlayer or derived class that
		///		is to be added.
		/// \return Adding a player requires adding and sending a message
		///		through the underlying bdDeliveryManager. In order to send a
		///		message the bdDeliveryManager must be 'active' see
		///		bdDeliveryManager::activate for more details. If the
		///		bdDeliveryManager is not active an activation request will be
		///		added to the bdDeliveryManager with the add player message.
		///		The return value is:
		///		- True if the add player message was added to the
		///		bdDeliveryManager.
		///		- False otherwise. The add player message was not added to the
		///		bdDeliveryManager.
		bdBool add(bdPlayer& player);

		/// Remove a player to the system.
		/// Removing players is A-synchronous, register a
		/// bdPlayerManagerListener to be informed of player removals.
		/// Upon removal of the last local player the underlying
		/// bdDeliveryManager will be deactivated.
		/// \param player[in] A reference to a bdPlayer or derived class that
		///		is to be removed.
		/// \return Removing a player requires adding and sending a message
		///		through the underlying bdDeliveryManager. In order to send a
		///		message the bdDeliveryManager must be 'active' see
		///		bdDeliveryManager::activate for more details. If the
		///		bdDeliveryManager is not active an activation request will be
		///		added to the bdDeliveryManager with the remove player message.
		///		The return value is:
		///		- True if the remove player message was added to the
		///		bdDeliveryManager.
		///		- False otherwise. The remove player message was not added to the
		///		bdDeliveryManager.
		bdBool remove(const bdPlayerRef player) const;

		/// Get the total number of players.
		/// \return The current number of players.
		bdUInt getNumPlayers() const;

		/// Player accessor.
		/// \note The order of players within a bdPlayerManager can change
		///		as players are added and removed. It is not advisable to
		///		store the index of a player within the bdPlayerManager. Instead
		///		store the bdPlayerRef returned by bdPlayerManager::getPlayer.
		/// \param i[in] The index of the required player.
		/// \return a bdPlayerRef pointing to the \a i'th player.
		bdPlayerRef getPlayer(const bdUInt i) const;

		/// Register a bdPlayerManagerListener. Listeners are used to inform other
		/// classes of bdPlayerManager related events.
		/// \param listener[in] A pointer to the listener to be registered.
		void registerListener(bdPlayerManagerListener *const listener);

		/// Unregister a particular bdPlayerManager.
		/// \param listener[in] A pointer to the interceptor to unregister.
		void unregisterListener(bdPlayerManagerListener *const listener);

		/// Write the state of the bdPlayerManager to a bdBitBuffer.
		/// \param buffer[in,out] The bdBitBuffer into which to write our state.
		void serialize(bdBitBufferRef buffer);

		/// Set the state of this bdPlayerManager from a previously
		/// serialized state.
		/// \param buffer[in] The bdBitBuffer from which to read our state.
		bdBool deserialize(bdBitBufferRef buffer);

	protected:

		virtual void onDeliveryDeactivate(const bdCommonAddrRef addr);

		virtual bdBool acceptDelivery(const bdDeliveryMessage& receivedMsg);

		void handleAdd(const bdDeliveryMessage& addMsg);

		void handleRemove(const bdDeliveryMessage& removeMsg);

		virtual void sendInitData(const bdConnectionRef connection);

		virtual bdBool acceptSession(const bdReceivedMessage& receivedMsg);

		void handleInitData(const bdReceivedMessage& initData);

	protected:

		friend class bdGameLog;

	protected:

		bdDeliveryManager *m_deliveryManager;

		bdArray<bdPlayerRef> m_players;

		bdFastArray<bdPlayerManagerListener*> m_listeners;
};

#endif // BD_PLAYER_MANAGER_H
