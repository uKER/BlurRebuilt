// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A simple implementation of the bdInterestInterface class.

#ifndef INTEREST_LEVELS_H
#define INTEREST_LEVELS_H

#include <bdExample/bdExampleCommonClientServer/bdExampleCommonCS.h>

class bdReplicaTable;

#define MAX_INTEREST_DIST (10.0f)

/// An implementation of the bdInterestInterface class.
/// It provides two different interest levels based on the distance between
/// two replicas.
class InterestLevels : public bdInterestInterface
{

	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// Default constructor.
		InterestLevels();

		/// Constructor taking a net object table pointer.
		InterestLevels(bdReplicaTable *table);

		/// Destructor.
		virtual ~InterestLevels();

		/// Set replica table to use.
		/// This function must be called before any use can be made
		/// of this class.
		void registerReplicaTable(bdReplicaTable *table);

		virtual void beginUpdate();

		/// Populates the interest set for a given player.
		virtual void getInterestSet(bdPlayerRef player,
									bdSet<bdReplicaRef>& interestSet);

		/// Calculates the interest between any to replicas.
		bdFloat32 getInterest(bdReplicaRef interestingReplica,
							  bdReplicaRef interestedReplica);


	protected:

		/// A pointer to the table containing all the replicas in the game.
		bdReplicaTable *m_replicaTable;

		/// An internal counter so that we can keep track of the number of
		/// updates sent. We will use this to enforce an update to be sent to 
		/// everyone every so often.
		bdUInt32 m_counter;

};

#endif // INTEREST_LEVELS_H

