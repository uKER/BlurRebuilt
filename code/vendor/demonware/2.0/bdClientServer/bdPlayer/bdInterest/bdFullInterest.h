// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A bdInterestInterface implemenataion where everything is
//			interested in everything else.

#ifndef BD_FULL_INTEREST_H
#define BD_FULL_INTEREST_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdPlayer/bdInterest/bdInterestInterface.h>

class bdServer;
class bdReplicaTable;

/// The default bdInterestInterface implementation used by bdServer.
/// In this implementation every bdReplica is interested in every
/// other bdReplica on the bdServer.
class bdFullInterest : public bdInterestInterface
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Constructor.
		/// \param replicaTable[in] A pointer to the bdReplica table that
		///							should contain a bdReplicaRef of all
		///							bdReplica on the server.
		bdFullInterest(bdReplicaTable *const replicaTable);

		/// Adds a bdReplicaRef for all replica in \a m_replicaTable
		/// to \a interestSet.
		/// \param player[in] The player whose interest set is required.
		/// \param interestSet[in, out]
		virtual void getInterestSet(bdPlayerRef player,
									bdSet<bdReplicaRef>& interestSet);

	protected:

		/// A pointer to the bdReplica table.
		///	This should contain a bdReplicaRef of all bdReplica on the server.
		bdReplicaTable *m_replicaTable;
};

#endif // BD_FULL_INTEREST_H
