// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdClientServer/bdReplica/bdReplicaTable.h>

bdReplicaRef bdReplicaTable::get(const bdReplicaId id) const
{
	bdReplicaMap::Iterator iterator;
	iterator = m_table.getIterator(id);
	if(iterator)
	{
#ifdef BD_DEBUG
		m_table.releaseIterator(iterator);
#endif // BD_DEBUG
		return m_table.getValue(iterator);
	}
	else
	{
		return BD_NULL;
	}
}

#ifdef BD_DEBUG
void bdReplicaTable::print(const bdByte8 *const tableName) const
{
	const bdUInt32 size = m_table.getSize();
	bdPrintf("%s Data\n", tableName);
    bdPrintf("%u entries\n\n", size);

	bdReplicaMap::Iterator iter =  m_table.getIterator();

	if(size)
	{
		bdPrintf("\tbdReplicaId\tbdReplicaRef\tbdReplicaRef->getObjectID()\n");

		bdUInt32 i = 1;
		while(iter)
		{
			const bdReplicaId& key = m_table.getKey(iter);
			bdReplicaRef replica = m_table.getValue(iter);
			const bdReplicaId& replicaId = replica->getId();

			bdPrintf("%u\t%u\t\t%p\t%u\n",i, key, (replica.operator ->()), replicaId);

			m_table.next(iter);
			i++;
		}

		bdPrintf("\n");

	}
}
#endif // BD_DEBUG
