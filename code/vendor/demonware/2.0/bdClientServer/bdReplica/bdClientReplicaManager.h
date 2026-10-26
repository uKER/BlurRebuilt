// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to organise & co-ordinate replication on the client.

#ifndef BD_CLIENT_REPLICA_MANAGER_H
#define BD_CLIENT_REPLICA_MANAGER_H

#include <bdCore/bdCore.h>


class bdReplicaTable;
class bdClientReplicaListener;
BD_REFERENCE(bdMessage);

/// A class to organise & co-ordinate replication on the client.
class bdClientReplicaManager
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		bdClientReplicaManager();

		void init(bdReplicaTable *const replicaTable);

		/// Registers a bdReplicaListener to inform the application about
		/// creation or deletion of SECONDARY replicas.
		/// \param listener[in] The bdReplicaListener instance that will
		/// perform application-specific handling of creation and deletion
		/// events.
		void registerReplicaListener(bdClientReplicaListener *const listener);

		/// Unregisters a previously registered bdReplicaListener.
		/// \param listener[in] The bdReplicaListener to be removed.
		void unregisterReplicaListener(bdClientReplicaListener *const listener);

		/// Handler for remote calls to this service
		void handle(bdMessageRef msg);

	protected:

		/// Creates a secondary replica on this host.
		/// \param msg The message that contains the type and id of the object
		/// to be replicated.
		void handleCreateSecondary(bdMessageRef msg);

		/// Destroys a secondary replica on this host.
		/// \param msg The message that contains the id of the object
		/// to be destroyed.
		void handleDestroySecondary(bdMessageRef msg);

	protected:

		/// Array of bdReplicaListeners that inform the application of creation or
		/// deletion events.
		bdFastArray<bdClientReplicaListener*> m_listeners;

		//bdAddr m_serverHost;

		bdReplicaTable *m_replicaTable;
};

#endif //BD_CLIENT_REPLICA_MANAGER_H
