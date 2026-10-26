// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: An interface that can be registered to notify the application 
//			when a specific bdReplica receives an update.

#ifndef BD_UPDATE_LISTENER_H
#define BD_UPDATE_LISTENER_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdReplica.h>

/// This abstract class declares an interface that can be implemented to notify
/// the application about a replica that has received an update.
/// An instance of this interface should be registered with the update  
/// manager's registerListener() method.
class bdUpdateListener
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor.
		virtual ~bdUpdateListener() {};

		/// This method will be called immediately after \a replica 
		/// receives an update.
		/// \param replica[in]	A reference to the replica that has been 
		///						updated.
		virtual void onUpdate(bdReplicaRef replica) = 0;
};

#endif // BD_UPDATE_LISTENER_H
