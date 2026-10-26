// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: A container for fast lookup of bdReplica from bdReplicaId.

#ifndef BD_REPLICA_TABLE_H
#define BD_REPLICA_TABLE_H

#include <bdCore/bdCore.h>

#include <bdClientServer/bdReplica/bdReplica.h>

/// A container for all the currently existing bdReplicas.
/// Used this class to quickly find bdReplica whose bdReplicaId is known.
class bdReplicaTable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// The type of map that is used to relate object references to ids.
		typedef bdHashMap<bdReplicaId, bdReplicaRef> bdReplicaMap;
		/// The iterator to use when iterating \a this table.
		typedef bdReplicaMap::Iterator Iterator;

		/// Adds a bdReplica to bdReplicaId association into the table.
		/// Adds \a replica into the table, associating \a replica with
		/// its bdReplicaId retrieved via bdReplica::getId().
		/// \param replica[in]	The bdReplica to add into the table.
		/// \return 
		///  - True if successful.
		///  - False if \a replicas id already exists in the table.
		inline bdBool add(const bdReplicaRef replica);

		/// Removes a bdReplica to bdReplicaId association from the table.
		/// Removes the association between \a id and a bdReplica.
		/// \param id[in] The bdReplicaId of the bdReplica to remove.
		/// \return 
		///  - True if successful.
		///  - False if the \a id does not exist in the table.
		inline bdBool remove(const bdReplicaId id);

		/// Removes a bdReplica to bdReplicaId association from the table.
		/// Removes the association between \a replica and a bdReplicaId.
		/// \param replica[in]	A bdReplicaRef to the \a replica to remove.
		/// \return 
		///  - True if successful.
		///  - False if the \a replica does not exist in the table.
		inline bdBool remove(const bdReplicaRef replica);

		/// Clears the table.
		inline void clear();

		/// Finds a bdReplica given a bdReplicaId in the table.
		/// \param id[in] The bdReplicaId of the bdReplica to locate.
		/// \return 
		///  - A reference to the bdReplica if successful.
		///  - BD_NULL if the bdReplica does not exist.
		bdReplicaRef get(const bdReplicaId id) const;

		/// Gets the bdReplicaTable::Iterator of the first bdReplica in the
		/// table.
		/// \return The iterator corresponding to the first association.
		/// \note This does not correspond to the first bdReplica inserted 
		///  chronologically.
		inline Iterator getIterator() const;

		/// Gets an iterator that corresponds to the specified bdReplicaId.
		/// \param id[in] The bdReplicaId to get the iterator for.
		/// \return The iterator for \a id.
		inline Iterator getIterator(const bdReplicaId &id) const;

		/// Creates a safe copy of an iterator.
		/// \param other[in] The iterator to make a copy from.
		/// \return A new Iterator pointing to the same position as \a other.
		inline Iterator getIterator(const Iterator &other) const;

		/// Advances the given iterator to the next bdReplica in the table.
		/// \param iterator[in, out] The iterator to advance.
		/// \return The bdReplica corresponding to \a iterator prior to \a
		/// iterator being advanced.
		inline bdReplicaRef next(Iterator &iterator) const;

		/// Releases an iterator. This is used for debugging purposes in
		/// order to detect simultaneous iteration and modification bugs.
		/// \param iterator[in] The iterator to be released.
		inline void releaseIterator(Iterator iterator) const;

#ifdef BD_DEBUG
		/// Used to print the contents of the table to the screen for 
		/// debugging purposes.
		/// This function should only be used for debugging purposes.
		void print(const bdByte8 *const tableName) const;
#endif // BD_DEBUG

	protected:

		/// A map of bdReplicaId to bdReplica.
		bdReplicaMap m_table;
};

#include <bdClientServer/bdReplica/bdReplicaTable.inl>

#endif //BD_REPLICA_TABLE_H
