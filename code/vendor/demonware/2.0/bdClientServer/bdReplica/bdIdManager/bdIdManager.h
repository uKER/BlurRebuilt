// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: This class implements a A simple sever bdReplicaId manager.

#ifndef BD_ID_MANAGER_H
#define BD_ID_MANAGER_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdReplicaTypes.h>

/// A simple sever bdReplicaId manager.
/// An unbound bdReplicaId will be reused on the next bind.
class bdIdManager
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		bdIdManager();

		/// Get an unused bdReplicaId from the pool.
		/// The returned bdReplicaId is removed from the pool of available
		/// IDs until it is passed to bdIdManager::unbind.
		/// \return A currently unsued bdReplicaId.
		bdReplicaId bind();

		/// Return a bdReplicaId to the pool.
		void unbind(const bdReplicaId& id);

	protected:

		/// Stores ids that have been freed through calls to unbind
		/// until they are rebound.
		bdFastArray<bdReplicaId> m_freedIds;

		/// The highest allocated ID so far.
		bdUInt16 m_idCounter;
};

#endif // BD_ID_MANAGER_H
