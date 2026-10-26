// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include "InterestLevels.h"
#include "ReplicaAdaptor.h"
#include <bdClientServer/bdReplica/bdReplicaTable.h>

#define NUM_LEVELS 3
#define MAX_UPDATE_INTERVAL 50


InterestLevels::InterestLevels()
: m_replicaTable(BD_NULL),
  m_counter(0)
{
}

InterestLevels::InterestLevels(bdReplicaTable *table)
: m_counter(0)
{
	m_replicaTable = table;
}

InterestLevels::~InterestLevels()
{
}

void InterestLevels::registerReplicaTable(bdReplicaTable *table)
{
	m_replicaTable = table;
}

void InterestLevels::beginUpdate()
{
	m_counter++;
}

void InterestLevels::getInterestSet(bdPlayerRef player,
									bdSet<bdReplicaRef>& interestSet)
{
	
	bdReplicaRef playersReplica = player->getControlledReplica();
	
	if(playersReplica != BD_NULL)
	{
		// we iterate through every object
		// and see how interested the passed in player's controlled object 
		// is in them.

		bdReplicaRef otherReplica;

		bdReplicaTable::Iterator iter = m_replicaTable->getIterator();

		while(iter != BD_NULL)
		{
			otherReplica = m_replicaTable->next(iter);

			// levelOfInterest is a float between 0 & 1
			// where 1.0 is most interested
			//   and 0.0 is least interested
			bdFloat32 levelOfInterest = getInterest(playersReplica, otherReplica);
			
			bdUInt32 level;

			// assuming 2 levels.
			// this means 0.0  -> 0.5  = update every 10th frame
			// this means 0.5  -> 1    = update every frame

			if(levelOfInterest > 0.5)
			{
				level = 1; 
			}
			else
			{
				level = 10;
			}

			if(m_counter%level == 0)
			{
				// add this replica to the list.
				interestSet.insert(otherReplica);
			}
		}
	}
	else
	{
		// do nothing.
	}
}
//end_of_code

bdFloat32 InterestLevels::getInterest(bdReplicaRef interestingReplica,
								      bdReplicaRef interestedReplica)
{
	ReplicaAdaptorRef replica1 = reference_cast<ReplicaAdaptor>(interestingReplica);
	ReplicaAdaptorRef replica2 = reference_cast<ReplicaAdaptor>(interestedReplica);

	bdFloat32 pos1[2];
	bdFloat32 pos2[2];
	replica1->getSphere().getPos(pos1);
	replica2->getSphere().getPos(pos2);
	const bdFloat32 diffX = pos1[0] - pos2[0];
	const bdFloat32 diffY = pos1[1] - pos2[1];
	const bdFloat32 dist = bdSqrtf32((diffX*diffX) + (diffY*diffY));

	if(dist < MAX_INTEREST_DIST)
	{
		const bdFloat32 normalisedDist = dist/MAX_INTEREST_DIST;
		return 1.0f - normalisedDist;
	}

	// indicates least interest
	return 0;
}


