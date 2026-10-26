// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Data required for replicas on a server.

#ifndef BD_SERVER_REPLICA_DATA_H
#define BD_SERVER_REPLICA_DATA_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdUpdate/bdUpdateSettings.h>
#include <bdConnection/bdConnection.h>

BD_REFERENCE(bdConnection);

class bdUpdateWindow;

/// Data required for replicas on a server.
class bdServerReplicaData
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Define a last ack map container.
		typedef bdHashMap<bdConnectionRef, bdUpdateSeqNum> bdLastAckMap;

		/// Default constructor.
		bdServerReplicaData();

		/// Destructor.
		~bdServerReplicaData();

		/// Create an update window for this class.
		/// \return A pointer to the newly created update window.
		bdUpdateWindow* createUpdateWindow();

		/// Accessor function for the update window for this class.
		/// \return A pointer to the newly created update window.
		bdUpdateWindow* getUpdateWindow();

		/// Record the fact that an update has been 'done'.
		/// \param dirtyFlags[in]	Flags to indicate what was dirty in the
		///							update.
		void addUpdate(const bdUInt32 dirtyFlags);

		/// Accessor function for the highest update sequence number that has
		/// been received.
		/// \return The highest update number.
		bdUpdateSeqNum getUpdateNum() const;

		/// Adds a new host for a replica.
		/// Adds an entry to the set of hosts that hold a copy of the replica,
		/// and to the bdLastAckMap.
		/// \param connection[in] 	The new host that holds a copy of this replica.
		/// \param firstAck[in] 	The ack to add to the last ack map.
		void addConnection(bdConnectionRef connection,
						   const bdUpdateSeqNum firstAck);

		/// Adds a host host for a replica.
		/// Removes a host from the set of hosts that hold a copy of the replica,
		/// and from the bdLastAckMap.
		/// \param connection[in] The host from which the replica has been removed.
		void removeConnection(bdConnectionRef connection);

		/// Updates the host to update acknowledgment map.
		/// \param connection[in]	The host that has acknowledged an update.
		/// \param lastAck[in]	 	The sequence number of the acknowledged update.
		void updateAckMap(bdConnectionRef connection,
						  const bdUpdateSeqNum lastAck);

		/// Find the last update that a given host acknowledged.
		/// \param connection[in]	The host whose last acknowledgement is required.
		/// \param lastAck[out] 	The sequence number of the last update acknowledged
		///						 	by \a host, if it exists.
		/// \return
		/// - True, if a host has acknowledged an update.
		/// - False, otherwise.
		bdBool getLastAck(bdConnectionRef connection,
						  bdUpdateSeqNum& lastAck) const;

		/// Get const access to the set of hosts.
		/// \return A const reference to m_hosts.
		const bdSet<bdConnectionRef>& getConnectionSet() const;

	protected:

		/// The data required to create delta compressed update messages.
		/// This data is only created if required, otherwise this pointer
		/// remains set to BD_NULL.
		bdUpdateWindow *m_updateWindow;

		/// The update sequence number of the replica.
		/// If the replica uses delta compressed updates, this member is
		/// a duplicate of bdUpdateWindow::m_nextUpdateNum. However to
		/// remove the duplication would cost more.
		bdUpdateSeqNum m_nextUpdateNum;

		/// Map of connections to last acknowledged updates.
		bdLastAckMap m_lastAckMap;

		/// The set of connections to which the replica is replicated.
		bdSet<bdConnectionRef> m_connections;
};

#endif // BD_SERVER_REPLICA_DATA_H
