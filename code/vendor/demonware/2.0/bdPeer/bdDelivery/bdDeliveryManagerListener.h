// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A listener interface for events associated with a delivery manager.

#ifndef BD_DELIVERY_MANAGER_LISTENER_H
#define BD_DELIVERY_MANAGER_LISTENER_H

#include <bdCore/bdCore.h>

BD_REFERENCE(bdCommonAddr);

/// This class defines a listener interface for events that may occur in a 
/// bdDeliveryManager. Classes derived from bdDeliveryManagerListener can be 
/// registered using bdDeliveryManager::registerListener.
class bdDeliveryManagerListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor.
		bdDeliveryManagerListener();

		/// Virtual destructor.
		virtual ~bdDeliveryManagerListener();

		/// Called when a bdDeliveryManager has activated.
		/// \note listeners will receive onDeliveryActivate notifications
		///		for all activations, not just their own.
		/// \param addr[in] The address of the activating peer.
		virtual void onDeliveryActivate(const bdCommonAddrRef addr);

		/// Called when a bdDeliveryManager has deactivated.
		/// \note listeners will receive onDeliveryDeactivate notifications
		///		for all activations, not just their own.
		/// \param addr[in] The address of the deactivating peer.
		virtual void onDeliveryDeactivate(const bdCommonAddrRef addr);

		/// Called before the bdDeliveryManager dispatches messages 
		/// received from all peers (including itself).
		virtual void onDeliveryStart();

		/// Called after the bdDeliveryManager has dispatched all messages 
		/// received from all peers (including itself).
		virtual void onDeliveryEnd();

		virtual void onDeliveryHostMigrationFailed();

		virtual void onDeliveryInitCompleted();

};

#endif // BD_DELIVERY_MANAGER_LISTENER_H
