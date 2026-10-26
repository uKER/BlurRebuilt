// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

inline 
bdBool bdReplicaTable::add(const bdReplicaRef replica)
{
	return m_table.put(replica->getId(), replica);
}

inline 
bdBool bdReplicaTable::remove(const bdReplicaRef replica)
{
	return m_table.remove(replica->getId());
}

inline 
bdBool bdReplicaTable::remove(const bdReplicaId id)
{
	return m_table.remove(id);
}

inline
void bdReplicaTable::clear()
{
	m_table.clear();
}

inline 
bdReplicaTable::Iterator bdReplicaTable::getIterator() const
{
	return m_table.getIterator();
}

inline 
bdReplicaTable::Iterator bdReplicaTable::getIterator(const bdReplicaId &id) const
{
	return m_table.getIterator(id);
}

inline
bdReplicaTable::Iterator bdReplicaTable::getIterator(const Iterator &other) const
{
	return m_table.getIterator(other);
}

inline
void bdReplicaTable::releaseIterator(Iterator iterator) const
{
	m_table.releaseIterator(iterator);
}

inline 
bdReplicaRef bdReplicaTable::next(bdReplicaTable::Iterator &iterator) const 
{
	bdReplicaRef returnVal = m_table.getValue(iterator);
	m_table.next(iterator);
	return returnVal;
}
