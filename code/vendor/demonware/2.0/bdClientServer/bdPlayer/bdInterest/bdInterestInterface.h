// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: Provides an interface for implementing a selective updating schemes.

#ifndef BD_INTEREST_INTERFACE_H
#define BD_INTEREST_INTERFACE_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdPlayer/bdPlayer.h>
#include <bdClientServer/bdReplica/bdReplica.h>

/// An abstract base class for selective updating.
/// A derived instance of bdInterestInterface may be registered with
/// bdServer::setInterest, such a class will typically use a games
/// potentially visible set (PVS) and potentially audible set (PAS)
/// systems, to ensure that only the necessary updates are sent to
/// each player. BitDemon combines the interest set of
/// all bdPlayers on the same host so that no duplicate data is sent.
class bdInterestInterface
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Virtual destructor.
		virtual ~bdInterestInterface();

		/// Indicates the start of a server update loop.
		/// bdInterestInterface::beginUpdate is called just before the server
		/// enters a cycle of collating updates for its clients. This is to
		/// allow the interest set to increment or reset any internal counters
		/// or timers.
		virtual void beginUpdate();

		/// Get the interest set for a given bdPlayer.
		/// Overloaded implementations will typically look at the bdReplica
		/// \a player is controlling and add any bdReplicas within it's
		/// PVS/PAS to \a interestSet. The \a players controlled replica
		/// should also be added to \a interestSet. The \a player will only
		/// be updated about bdReplica in its \a interestSet.
		/// \param player[in]	The player whose interest set is required.
		/// \param interestSet[in, out]	A set of bdReplicaRef to which any
		///								bdReplica the player is interested in
		///								should be added. This set may not be
		///								empty as the same set if used for all
		///								bdPlayer on the same host. Hence
		///								insertions may fail due to the
		///								bdReplicaRef already being in the set.
		virtual void getInterestSet(bdPlayerRef player,
									bdSet<bdReplicaRef>& interestSet) = 0;

};

#endif // BD_INTEREST_INTERFACE_H

