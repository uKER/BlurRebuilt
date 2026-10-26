// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An interface that can be registered to handle application specific
// operations when a replica has been created or is about to be deleted.

#ifndef BD_CLIENT_REPLICA_LISTENER_H
#define BD_CLIENT_REPLICA_LISTENER_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdReplica.h>

/// This abstract class declares an interface that can be implemented to handle
/// application specific operations when bdClientReplicaManager creates a 
/// replica, or is about to delete a replica. For example, it could be used 
/// to maintain a list of current replicas for rendering. An instance of this 
/// interface should be registered with the replica manager with the 
/// bdClientReplicaManager::registerListener() method.
class bdClientReplicaListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		virtual ~bdClientReplicaListener();

		/// Replica create notification.
		/// This method is called immediately after a replica has been created
		/// and before it is initialized with the initial update from the server.
		/// This will only be in response to a 
		/// bdReplicaManagerOperation::BD_CREATE message from the server.
		///
		/// Typically this is a good place to add any assests (eg meshes) that 
		/// the \a replica requires.
		/// \param replica[in] A reference to the replica just created.
		virtual void onClientCreate(bdReplicaRef replica);

		/// Replica initialization notification.
		/// This method is called after it has been created and has received
		/// some initial state.
		/// \param replica[in] A reference to the replica just initialized.
		virtual void onClientInitialize(bdReplicaRef replica);

		/// Replica deletion notification.
		/// This method is envoked immediately before a replica is
		/// removed by the client. It is called in response to a BD_DESTROY
		/// message from the server and should clean up any references held by
		/// the application to \a replica.
		/// \param replica[in] A reference to the replica that will be deleted.
		virtual void onClientDelete(bdReplicaRef replica);

};

#endif // BD_CLIENT_REPLICA_LISTENER_H
