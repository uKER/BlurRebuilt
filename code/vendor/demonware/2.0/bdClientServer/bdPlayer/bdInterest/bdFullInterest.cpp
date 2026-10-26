// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdPlayer/bdInterest/bdFullInterest.h>

#include <bdClientServer/bdReplica/bdReplicaTable.h>

bdFullInterest::bdFullInterest(bdReplicaTable *const replicaTable)
: m_replicaTable(replicaTable)
{
}

void bdFullInterest::getInterestSet(bdPlayerRef,
									bdSet<bdReplicaRef>& interestSet)
{
	if(m_replicaTable)
	{
		bdReplicaTable::Iterator iter = m_replicaTable->getIterator();

		while(iter)
		{
			bdReplicaRef replica = m_replicaTable->next(iter);
			interestSet.insert(replica);
		}
	}
}

