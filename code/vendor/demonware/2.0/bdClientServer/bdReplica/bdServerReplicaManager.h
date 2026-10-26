// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A class to organise & co-ordinate replication on the server.

#ifndef BD_SERVER_REPLICA_MANAGER_H
#define BD_SERVER_REPLICA_MANAGER_H

#include <bdCore/bdCore.h>

#include <bdConnection/bdMessage/bdMessage.h>

#include <bdClientServer/bdReplica/bdReplica.h>
#include <bdClientServer/bdReplica/bdIdManager/bdIdManager.h>

class bdReplicaTable;

/// A class to organise & co-ordinate replication on the server.
class bdServerReplicaManager
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default Constructor
		bdServerReplicaManager();

		/// Initialize the replica manager and tell it which replica table
		/// to use.
		/// \param replicaTable[in] A pointer to the replica table to use.
		/// \param connections[in]  A pointer to a list of current connections
		void init(bdReplicaTable *const replicaTable, bdArray<bdConnectionRef> *const connections);

		/// Will cause the replica specified to be replicated
		/// This function is usually called by a server who needs
		/// to replicate the objects it owns on other hosts
		/// \param replica[in] The replica to replicate.
		void add(bdReplicaRef replica);

		/// Removes all copies of \a replica from all hosts where they exist.
		/// \param replica The replica to delete.
		void remove(bdReplicaRef replica);

		/// Utility function to remove a host from all replicated objects.
		/// Used to handle disconnections from a host
		/// \param connection[in]  The host that disconnected.
		void removeConnection(bdConnectionRef connection);

		/// Checks whether any new replicas should be created, or any
		/// existing replicas should be removed. It does this by
		/// retrieving the potentially relevant set (PRS) for each replica and
		/// determining if it has changed.
		/// It then deletes copies of replicas on hosts that are no longer
		/// interested in them, and creates replicas on hosts that have recently
		/// become interested.
		void refresh(bdReplicaRef replica,
					 bdHashMap<bdConnectionRef, bdMessageRef> &replicationMsg);

		/// Traverses all replicas in the replica table and checks whether
		/// the relevant set has changed.
		void refresh();

	protected:

		/// Adds a replication message for replica to msg
		/// \param replica[in] 		The replica to be created.
		/// \param connection[in] 	The host on which the replica is to be created
		/// \param msg[in|out] 		The message in which this replication message is inserted.
		void replicate(bdReplicaRef replica,
					   bdConnectionRef connection,
					   bdMessageRef msg) const;

		/// Adds a dereplication message for replica to msg.
		/// \param replica[in] 		The replica to be removed.
		/// \param connection[in] 	The host on which the replica is to be removed from
		/// \param msg[in|out] 		The message in which this replication message is inserted.
		void dereplicate(bdReplicaRef replica,
						 bdConnectionRef connection,
						 bdMessageRef msg) const;

		/// Transmits a dereplicate message concerning a specific replica
		/// to a host.
		/// \param replica[in]	The replica to be removed.
		/// \param msg[out]		The message to be created.
		/// \todo Check this explanation.
		void createDereplicateMessage(bdReplicaRef replica,
									  bdMessageRef msg) const;

	protected:

		/// A pointer to the table of all replicas managed by the server.
		bdReplicaTable *m_replicaTable;

		/// A list of all current connections. This list is owned and maintained by
		/// bdServer.
		bdArray<bdConnectionRef> *m_connections;

		/// An instance of the id manager.
		bdIdManager m_idManager;

};

#endif // BD_SERVER_REPLICA_MANAGER_H
