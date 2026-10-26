#ifndef UT_OPEN_HASH_TABLE_H
#define UT_OPEN_HASH_TABLE_H

#include <core/utility/hashops.h>
#include <core/containers/initialiser.h>

namespace Ut
{
	template <typename Key, typename ItemType>
	struct OpenHashTableEntry
	{
		enum State
		{
			ACTIVE,
			EMPTY,
			DELETED
		};

		OpenHashTableEntry(State s = EMPTY) : state(s) {}

		State state;
		
		//This is our map/set item.
		ItemType element;
	};

	//\brief An implementation of linear probing.
	template <typename Key, typename ItemType, typename HashKeyOp>
	class OpenHashTableLinearProbe
	{
	public:
		INLINE static unsigned int Probe(
			const OpenHashTableEntry<Key, ItemType>* pHashTable, 
			unsigned int hashTableActiveElements, 
			unsigned int hashTableCapacity,
			const Key& key)
		{
			unsigned int hashIndex = HashKeyOp::GetHash(key);
			hashIndex = hashIndex % hashTableCapacity;

			while(true)
			{
				if (pHashTable[hashIndex].state == OpenHashTableEntry<Key, ItemType>::EMPTY)
					return hashIndex;

				if (pHashTable[hashIndex].state != OpenHashTableEntry<Key, ItemType>::DELETED)
				{
					//Must be active
					if (HashKeyOp::Equal(key, pHashTable[hashIndex].element.GetKey()))
						return hashIndex;
				}

					//Resolve collisions...
				hashIndex++;
				if (hashIndex >= hashTableCapacity)
					//Wrap index around to the beginning.
					hashIndex = 0;
			}
		}
	};

	//\brief An implementation of quadratic probing.
	template <typename Key, typename ItemType, typename HashKeyOp>
	class OpenHashTableQuadraticProbe
	{
	public:
		INLINE static unsigned int Probe(
			const OpenHashTableEntry<Key, ItemType>* pHashTable, 
			unsigned int hashTableActiveElements, 
			unsigned int hashTableCapacity,
			const Key& key)
		{
			unsigned int hashIndex = HashKeyOp::GetHash(key);
			hashIndex = hashIndex % hashTableCapacity;

			int i = 0;
			
			//Exit our loop if our slot is not occupied or is equal to our key.

			while(true)
			{
				if (pHashTable[hashIndex].state == OpenHashTableEntry<Key, ItemType>::EMPTY)
					return hashIndex;

				if (pHashTable[hashIndex].state != OpenHashTableEntry<Key, ItemType>::DELETED)
				{
					//Must be active
					if (HashKeyOp::Equal(key, pHashTable[hashIndex].element.GetKey()))
						return hashIndex;
				}

				//Resolve collisions...
				hashIndex += 2 * ++i - 1;
				if (hashIndex >= hashTableCapacity)
					//Wrap index around to the beginning.
					hashIndex -= hashTableCapacity;

				if (hashIndex >= hashTableCapacity)
				{
					//We should only get here if our hash table capacity is not a prime number.
					//Perform a linear probe.
					return OpenHashTableLinearProbe<Key, ItemType, HashKeyOp>::Probe(
						pHashTable, 
						hashTableActiveElements, 
						hashTableCapacity,
						key);
				}
			}
		}
	};

	/// @addtogroup Containers
	/// @{
	///\brief Hash Table implementation using open addressing and thus lazy deletion. 
	///
	///The use of `open addressing' means that if two keys hash to the same position in the table we have to 
	///find a new position in the table for the value. We do this by `probing' positions in the table to determine
	///if they're free. The type of probe used is specified as a template argument. An alternative approach to
	///open addressing would be `chaining', where a list of keys can be stored per position in the hash table.
	///This has not been used in the interests of memory efficiency, we want to avoid the heap fragmentation caused
	///by many d
	///
	///Data items are unique, i.e. one copy of each key.
	///
	///Template parameters:
	///Key is the type used for comparisons, which may be the same or different to data stored if a HashMap is
	///being created.
	///ItemType is a container for the Key (HashSet) or Key Value pair (HashMap) stored at each slot in the
	///hash table.
	///HashKeyOp is a class that defines how the key is hashed and how keys are tested for equivalence.
	///
	///See HashSet and HashMap below.
	
	///Base class for OpenHashSet and OpenHashMap.
	template <
		typename Key, 
		typename ItemType,
		typename HashKeyOp,
		typename Probe,
		typename Allocator>
	class OpenHashTable
	{
	public:
		typedef ItemType DataType; // same as utmap / utset

		///%Const Iterator for OpenHashTable
		class ConstIterator
		{
		public:
			// constructors
			ConstIterator() {}

			ConstIterator(
				const OpenHashTableEntry<Key, ItemType>* pEntry,
				const OpenHashTableEntry<Key, ItemType>* pEnd,
				const UtVector<DynamicVector<OpenHashTableEntry<Key, ItemType>, Allocator> >* pVector) 
				
			{
				m_pEntry = const_cast<OpenHashTableEntry<Key, ItemType>* >(pEntry);
				m_pEnd = const_cast<OpenHashTableEntry<Key, ItemType>* >(pEnd);
				m_pVector = pVector;
			} ///< @a

			ConstIterator(const ConstIterator& c)
			:
			m_pEntry(c.m_pEntry), 
			m_pEnd(c.m_pEnd), 
			m_pVector(c.m_pVector) {} ///< @a

			ConstIterator& operator = (const ConstIterator& c) 
			{
				m_pEnd = c.m_pEnd;
				m_pEntry = c.m_pEntry; 
				m_pVector = c.m_pVector;
				return *this;
			} ///< @a
			
			// comparison
			bool operator == (const ConstIterator& c) const 
			{
				return m_pEntry == c.m_pEntry;
			} ///< @a

			bool operator == (const OpenHashTableEntry<Key, ItemType>* p) const 
			{
				return m_pEntry == p;
			} ///< @a

			bool operator != (const ConstIterator& c) const 
			{
				return m_pEntry != c.m_pEntry;
			} ///< @a

			bool operator != (const OpenHashTableEntry<Key, ItemType>* p) const
			{
				return m_pEntry != p;
			} ///< @a

			const ItemType& operator*() const 
			{
				return m_pEntry->element;
			} ///< @a
		
			const ItemType* operator->() const 
			{
				return &m_pEntry->element;
			} ///< @a

			//Pre-increment
			ConstIterator& operator ++()
			{
				++m_pEntry;
				if (m_pEntry >= m_pEnd)
				{
					m_pEntry = m_pEnd;
					return *this;
				}

				//While we're inactive, but within the bounds of our hash-table.
				if (m_pEntry->state != OpenHashTableEntry<Key, ItemType>::ACTIVE)
				{
					while(
						(m_pEntry < m_pEnd) &&
						(m_pEntry->state != OpenHashTableEntry<Key, ItemType>::ACTIVE))
					{
						++m_pEntry;
					}
				}
				return *this;
		
			} ///< @a
			//Post-increment iterator
			ConstIterator operator ++(int) 
			{
				ConstIterator tmp(*this);
				++(*this);
				return tmp;
			} ///< @a

			bool operator < (const ConstIterator& other) const
			{
				assert(m_pVector == other.m_pVector); 
				return m_pEntry < other.m_pEntry;
			} 

			unsigned int GetIndex() const
			{
#ifdef COMPILER_VC
				#pragma warning(push)
					#pragma warning(disable: 4311)
#endif
#if WORD_SIZE == 64
					U64 size = sizeof(OpenHashTableEntry<Key, ItemType>);
					U64 ptr = (U64)m_pEntry;
					U64 arrayStart = (U64)&((*m_pVector)[0]);
					return (int)((ptr - arrayStart) / size);
#else
					unsigned int size = sizeof(OpenHashTableEntry<Key, ItemType>);
					unsigned int ptr = (unsigned int)m_pEntry;
					unsigned int arrayStart = (unsigned int)&((*m_pVector)[0]);
					return (ptr - arrayStart) / size;
#endif
#ifdef COMPILER_VC
				#pragma warning(pop)
#endif
			} ///< @a

		protected:
			OpenHashTableEntry<Key, ItemType>* m_pEntry;
			OpenHashTableEntry<Key, ItemType>* m_pEnd;

			const UtVector<DynamicVector<OpenHashTableEntry<Key, ItemType>, Allocator> >* m_pVector;
		};

		///%Iterator for OpenHashTable
		class Iterator : public ConstIterator
		{
		public:
			using ConstIterator::m_pEntry;
			using ConstIterator::m_pVector;

			// constructors
			Iterator() {} ///< @a

			Iterator(
				OpenHashTableEntry<Key, ItemType>* pEntry,
				OpenHashTableEntry<Key, ItemType>* pEnd,
				const UtVector<DynamicVector<OpenHashTableEntry<Key, ItemType>, Allocator> >* pVector) 
			{
				m_pEntry = pEntry;
				ConstIterator::m_pEnd = pEnd;
				m_pVector = pVector;
			} ///< @a

			Iterator(const ConstIterator& c) : ConstIterator(c) {} ///< @a

			Iterator& operator = (const Iterator c)
			{
				ConstIterator::m_pEnd = c.m_pEnd;
				m_pEntry = c.m_pEntry;
				m_pVector = c.m_pVector;
				return *this;
			} ///< @a

			ItemType& operator*() 
			{
				return m_pEntry->element;
			} ///< @a
		
			ItemType* operator->() 
			{
				return &m_pEntry->element;
			} ///< @a
		};

		typedef Pair<typename OpenHashTable::Iterator, bool> InsertionReturnType;

		///\brief Constructor
		///
		OpenHashTable();

		///\brief Construct a hash table with a given initial table size.
		///
		///@param initialSize The initial table size.
		OpenHashTable(unsigned int initialSize);

		///\brief Copy constructor.
		///
		///@param rhs The hash table to copy.
		OpenHashTable(const OpenHashTable& rhs);

		///\brief Destructor.
		///
		~OpenHashTable();

		///\brief Asignment operator.
		///
		///@param rhs The hash table to copy.
		///@return A reference to this class.
		const OpenHashTable& operator=(const OpenHashTable& rhs);

		///\brief Returns the capacity of the hash table (unused + used entries).
		///
		///@return Hash table capacity.
		unsigned int Capacity() const;

		///\brief Returns the number of active entries in the hash table.
		///
		///@return The number of active hash table entries.
		unsigned int Size() const;

		///\brief Allocates memory to store the given number of hash table entries.
		///
		///@param size The number of hash table entries to reserve memory for.
		void Reserve(unsigned int size);

		///\brief Inserts a key into the hash table.
		///
		///@param key The key to insert.
		///@return Returns a pair: The first element is an iterator pointing to the key position. The second element is a boolean indicating whether a new key was inserted (true) or not (false).
		InsertionReturnType Insert(const Key& key);

		///\brief Inserts an item into the hash table.
		///
		///An item is a pair of values comprising a key and, optionally, a value.
		///@param item The item to insert.
		///@return Returns a pair: The first element is an iterator pointing to the key position. The second element is a boolean indicating whether a new key was inserted (true) or not (false).
		InsertionReturnType Insert(const ItemType& item);

		///\brief Erases a hash table entry.
		///
		///Note that lazy deletion is used here. The capacity of the hash table remains constant.
		///@param it Iterator pointing at the entry to be erased.
		///@return The number of elements erased.
		unsigned int Erase(Iterator& it);

		///\brief Erases a hash table entry.
		///
		///Note that lazy deletion is used here. The capacity of the hash table remains constant.
		///@param hashKey An index into the hash table.
		///@return The numberof elements erased.
		unsigned int EraseByIndex(unsigned int hashKey);

		///\brief Returns an iterator to the given key.
		///
		///If the key is not found then the iterator returned is equal to End().
		///
		///@param key The key to find
		///@return Iterator to the given key, if found.
		Iterator Find(const Key& key);

		///\brief Returns an iterator to the given key.
		///
		///If the key is not found then the iterator returned is equal to End().
		///
		///@param key The key to find.
		///@return Iterator to the given key, if found.
		ConstIterator Find(const Key& key) const;

		///\brief Returns an iterator to the given key.
		///
		///If the key is not found then the iterator returned is equal to End().
		///
		///@param hashKey An index into the hash table.
		///@return Iterator to the given key, if found.
		Iterator FindByIndex(unsigned int hashKey);

		///\brief Returns an iterator to the given key.
		///
		///If the key is not found then the iterator returned is equal to End().
		///
		///@param hashKey An index into the hash table.
		///@return Iterator to the given key, if found.
		ConstIterator FindByIndex(unsigned int hashKey) const;

		///\brief Returns the hash table index of the given key.
		///
		///If the key is not found then the hash index found is equal to -1.
		///
		///@param key The key to find.
		///@return The hash index for the given key.
		int FindIndex(const Key& key) const;

		///\brief Returns an iterator to the first element in the hash table.
		///
		///@return Iterator.
		Iterator Begin();

		///\brief Returns an iterator pointing at one entry past the end of the hash table.
		///
		///@return Iterator.
		Iterator End();

		///\brief Returns an iterator to the first element in the hash table.
		///
		///@return Iterator.
		ConstIterator Begin() const;

		///\brief Returns an iterator pointing at one entry past the end of the hash table.
		///
		///@return Iterator.
		ConstIterator End() const;

		///\brief Optimises the current memory usage of the hash table.
		///
		///This function attempts to reduce the capacity of the hash table to be closer to its current size.
		DEPRECATED void Optimise();

		void TrimCapacity();

		///\brief Clears the contents of the hash table.
		///
		void Clear();

		///\brief Returns true if the table is empty.
		///
		///@return True if empty, otherwise false.
		bool Empty();

	private:
		FORCE_INLINE Key& _GetNonConstKey(const Key& key)
		{
			return const_cast<Key&>(key);
		}

		FORCE_INLINE void _UpdateBeginCache();

		FORCE_INLINE void _UpdateEndCache();

		INLINE InsertionReturnType _InsertNoResize(const Key& key);

		INLINE void _Rehash();

		void _Rebuild(unsigned int capacity);

		INLINE unsigned int _NextPrime(unsigned int thisNumber);


		UtVector<DynamicVector<OpenHashTableEntry<Key, ItemType>, Allocator> > m_hashTable;
		OpenHashTableEntry<Key, ItemType>* m_pBeginCache;
		OpenHashTableEntry<Key, ItemType>* m_pEndCache;
		unsigned int m_nActiveElements;
		unsigned int m_nOccupiedElements;
	};

	/// @} end group

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::OpenHashTable()
	{
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;
		m_pBeginCache = 0;
		m_pEndCache = 0;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::OpenHashTable(unsigned int initialSize)
	{
		m_nActiveElements = 0;
		m_pBeginCache = 0;
		m_pEndCache = 0;
		m_hashTable.Resize(initialSize);
		Clear();
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE OpenHashTable<Key, ItemType, HashKeyOp ,Probe, Allocator>::OpenHashTable(const OpenHashTable& rhs)
	{
		*this = rhs;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::~OpenHashTable()
	{
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE const OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>& OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::operator=(
		const OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>& rhs)
	{
		if (rhs.m_hashTable.Size() == 0)
		{
			m_pBeginCache = 0;
			m_pEndCache = 0;
			m_hashTable = rhs.m_hashTable;
			m_nActiveElements = rhs.m_nActiveElements;
			m_nOccupiedElements = rhs.m_nOccupiedElements;
			return *this;
		}

		size_t rhsBeginCacheOffset;
		size_t rhsEndCacheOffset;

		if (rhs.m_pBeginCache)
			rhsBeginCacheOffset = (size_t)((U8*)rhs.m_pBeginCache - (U8*)&rhs.m_hashTable[0]);
		else
			rhsBeginCacheOffset = 0;
		
		if (rhs.m_pEndCache)
			rhsEndCacheOffset = (size_t)((U8*)rhs.m_pEndCache - (U8*)&rhs.m_hashTable[0]);
		else
			rhsEndCacheOffset = 0;

		m_hashTable = rhs.m_hashTable;
		m_nActiveElements = rhs.m_nActiveElements;
		m_nOccupiedElements = rhs.m_nOccupiedElements;
		m_pBeginCache = (OpenHashTableEntry<Key, ItemType>*)((U8*)&m_hashTable[0] + rhsBeginCacheOffset);
		m_pEndCache = (OpenHashTableEntry<Key, ItemType>*)((U8*)&m_hashTable[0] + rhsEndCacheOffset);
	
		return *this;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	UT_FORCE_INLINE unsigned int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Size() const
	{
		return m_nActiveElements;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	UT_FORCE_INLINE unsigned int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Capacity() const
	{
		return m_hashTable.Size();
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Reserve(unsigned int size)
	{
		m_hashTable.Resize(size);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::InsertionReturnType 
		OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Insert(const Key& key)
	{
		if (m_nOccupiedElements+1 > (unsigned int)m_hashTable.Size() / 2)
		{
			_Rehash();
			//Attempt insertion again.
			return Insert(key);
		}

		return _InsertNoResize(key);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::InsertionReturnType 
		OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Insert(const ItemType& item)
	{
		InsertionReturnType result = Insert(item.GetKey());
		if (result.second)
			*result.first = item;

		return result;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	unsigned int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Erase(Iterator& it)
	{
		int probedHashIndex = FindIndex(it->GetKey());
		if (probedHashIndex < 0)
			return 0;

		OpenHashTableEntry<Key, ItemType>* pDeletionAddr = &m_hashTable[probedHashIndex];
		if (pDeletionAddr->state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//Flag as deleted.
			pDeletionAddr->state = OpenHashTableEntry<Key, ItemType>::DELETED;
			m_nActiveElements--;

			//If we're removing our end pointer, get a new one.
			if (pDeletionAddr == m_pEndCache)
				_UpdateEndCache();
			if (pDeletionAddr == m_pBeginCache)
				_UpdateBeginCache();
			return 1;
		}

		return 0;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	unsigned int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::EraseByIndex(unsigned int hashIndex)
	{
		OpenHashTableEntry<Key, ItemType>* pDeletionAddr = &m_hashTable[hashIndex];
		if (pDeletionAddr->state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//Flag as deleted.
			pDeletionAddr->state = OpenHashTableEntry<Key, ItemType>::DELETED;
			m_nActiveElements--;

			//If we're removing our end pointer, get a new one.
			if (pDeletionAddr == m_pEndCache)
				_UpdateEndCache();
			if (pDeletionAddr == m_pBeginCache)
				_UpdateBeginCache();
			return 1;
		}
		return 0;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Iterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Find(const Key& key)
	{
		//Empty hash table!
		if (m_hashTable.Size() == 0)
			return End();

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		assert(probedHashIndex < (unsigned int)m_hashTable.Size());
		if (m_hashTable[probedHashIndex].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//We found what we were looking for.
			return Iterator(
				const_cast<OpenHashTableEntry<Key, ItemType>*>(&m_hashTable[probedHashIndex]), 
				m_pEndCache,
				&m_hashTable);
		}

		//We didn't find anything.
		return End();
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Find(const Key& key) const
	{
		OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>* pNonConstThis = 
			const_cast<OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>* >(this);

		return pNonConstThis->Find(key);
	}


	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::FindIndex(const Key& key) const
	{
		//Empty hash table!
		if (m_hashTable.Size() == 0)
			return -1;

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		assert(probedHashIndex < (unsigned int)m_hashTable.Size());
		if (m_hashTable[probedHashIndex].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//We found what we were looking for.
			return probedHashIndex;
		}

		//We didn't find anything.
		return -1;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Begin() const
	{
		const OpenHashTableEntry<Key, ItemType>* pEntry = 0;
		for (int i = 0; i < m_hashTable.Size(); i++)
		{
			if (m_hashTable[i].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
			{
				pEntry = &m_hashTable[i];
				break;
			}
		}

		if (!pEntry)
			return End();
		return ConstIterator(
			const_cast<OpenHashTableEntry<Key, ItemType>*>(pEntry), 
			m_pEndCache,
			&m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	FORCE_INLINE void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::_UpdateBeginCache()
	{
		for (int i = 0; i < m_hashTable.Size(); i++)
		{
			if (m_hashTable[i].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
			{
				m_pBeginCache = &m_hashTable[i];
				return;
			}
		}

		m_pBeginCache = m_pEndCache;		
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	FORCE_INLINE void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::_UpdateEndCache()
	{
		OpenHashTableEntry<Key, ItemType>* pEntry = 0;
		bool foundActive = false;
		if (m_hashTable.Size() > 0)
		{
			pEntry = &m_hashTable[m_hashTable.Size()-1];

			for (int i = m_hashTable.Size()-1; i >= 0; i--)
			{
				if (m_hashTable[i].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
				{
					foundActive = true;
					pEntry = &m_hashTable[i];
					break;
				}
			}
			pEntry++;
		}
		if (foundActive)
			m_pEndCache = pEntry;
		else
			m_pEndCache = 0;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::InsertionReturnType 
		OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::_InsertNoResize(const Key& key)
	{
		U32 probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		assert(probedHashIndex < (unsigned int)m_hashTable.Size());
		if (m_hashTable[probedHashIndex].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//Duplicate entry - just return.
			return InsertionReturnType(ConstIterator(&m_hashTable[probedHashIndex], m_pEndCache, &m_hashTable), false);
		}

		//Insert new record into our hash table.
		OpenHashTableEntry<Key, ItemType>* pInsertionAddr = &m_hashTable[probedHashIndex];
		_GetNonConstKey(pInsertionAddr->element.GetKey()) = key;
		if (pInsertionAddr->state != OpenHashTableEntry<Key, ItemType>::DELETED)
			m_nOccupiedElements++;

		pInsertionAddr->state = OpenHashTableEntry<Key, ItemType>::ACTIVE;
		m_nActiveElements++;

		if ((pInsertionAddr < m_pBeginCache) || (m_pBeginCache == 0))
			m_pBeginCache = pInsertionAddr;

		if (pInsertionAddr > m_pEndCache)
		{
			m_pEndCache = pInsertionAddr;
			m_pEndCache++;
		}
		else if (pInsertionAddr == m_pEndCache)
		{
			m_pEndCache++;
		}


		//We didn't need to rehash.
		return InsertionReturnType(ConstIterator(pInsertionAddr, m_pEndCache, &m_hashTable), true);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	FORCE_INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::End() const
	{
		return ConstIterator(
			const_cast<OpenHashTableEntry<Key, ItemType>*>(m_pEndCache), 
			m_pEndCache,
			&m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Iterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Begin()
	{
		return ConstIterator(m_pBeginCache, m_pEndCache, &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	FORCE_INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Iterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::End()
	{
		return ConstIterator(m_pEndCache, m_pEndCache, &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Iterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::FindByIndex(unsigned int hashKey)
	{
		return Iterator(&(m_hashTable[hashKey]), m_pEndCache, &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::FindByIndex(unsigned int hashKey) const
	{
		return ConstIterator(&(m_hashTable[hashKey]), m_pEndCache, &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Clear()
	{
		//Call destructors and then placement new upon contained objects.
		m_hashTable.Clear();
		m_hashTable.Resize(m_hashTable.Capacity());
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;
		m_pEndCache = 0;
		m_pBeginCache = 0;

		//Reinitialise hash table.
		for (int i = 0; i < m_hashTable.Size(); i++)
			m_hashTable[i].state = OpenHashTableEntry<Key, ItemType>::EMPTY;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE unsigned int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::_NextPrime(unsigned int thisNumber)
	{
		static const unsigned int POW2_32MINUS1 = 4294967295UL;
		int divisor;
		bool notPrime;

		if (thisNumber==0)
			thisNumber++;

		while(thisNumber < POW2_32MINUS1)
		{
			divisor = thisNumber / 2;
			notPrime = 0;
			while(divisor > 1)
			{
				if(thisNumber % divisor == 0)
				{
					notPrime = true;
					divisor = 0;
				}
				else
					divisor = divisor-1;
			}

			if (notPrime == false)
				return thisNumber;

			thisNumber = thisNumber + 1;
		}

		return POW2_32MINUS1;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::_Rebuild(unsigned int capacity)
	{
		UtVector<DynamicVector<OpenHashTableEntry<Key, ItemType>, Allocator> > oldTable = m_hashTable;
		//Call destructors and then placement new upon contained objects.
		m_hashTable.Clear();
		m_hashTable.Resize(capacity);
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;

		//Initialise new hash table.
		for (int i = 0; i < m_hashTable.Size(); i++)
			m_hashTable[i].state = OpenHashTableEntry<Key, ItemType>::EMPTY;

		//Insert elements into table.
		for (int i = 0; i < oldTable.Size(); i++)
			if (oldTable[i].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
			{
				InsertionReturnType r = Insert(oldTable[i].element.GetKey());
				r.first->CopyValue(oldTable[i].element);
			}

		//Invalidate cache.
		m_pBeginCache = 0;
		m_pEndCache = 0;
		//Update cache.
		_UpdateBeginCache();
		_UpdateEndCache();
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::_Rehash()
	{
		_Rebuild(_NextPrime(m_hashTable.Capacity()*2));
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	DEPRECATED INLINE void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Optimise()
	{
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	INLINE void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::TrimCapacity()
	{
		//Create new hashtable.
		OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator> newHashTable;
		newHashTable._Rebuild(_NextPrime(m_nActiveElements+1));
		for (Iterator it = Begin(); it != End(); ++it)
		{
			newHashTable._InsertNoResize(it->GetKey()).first->GetValue() = it->GetValue();
		}

		//Swap!
		assert(m_nActiveElements == newHashTable.m_nActiveElements);
		*this = newHashTable;
	}

	////////////////////////////////////////////////////////////
	// Set and Map
	// (which are just different interfaces to SortedBinaryTree)

	// transparent wrapper for Set elements
	template <typename Key>
	class OpenHashSetItem
	{
	public:
		OpenHashSetItem(const Key& k = Key()) : m_key(k) {}

		operator const Key& () const 
		{
			return m_key;
		} // can only get a const reference

		const Key& GetKey() const
		{
			return m_key;
		} ///< Returns a const reference to the key. The key cannot be modified after creation of the MapItem.

		//Dummy function.
		void CopyValue(const OpenHashSetItem& rhs)
		{
		}

	private:
		Key m_key;
	};

	//OpenHashTableEntry<Key, ItemType>
	/// Hash Set container class.
	/// Elements in a set are unique.
	/// @param Key The type stored in the set.
	/// @param Allocator The allocator type, e.g. Ut::StandardAllocator<Key>.
	template <
		typename Key, 
		typename HashKeyOp = HashKeyOpDefault<Key>,
		typename Probe = OpenHashTableQuadraticProbe<Key, OpenHashSetItem<Key>, HashKeyOp>,
		typename Allocator = ArrayAllocator<OpenHashTableEntry<Key, OpenHashSetItem<Key> > > >
	class OpenHashSet : public 
		OpenHashTable<
			Key,
			OpenHashSetItem<Key>, 
			HashKeyOp,
			Probe,
			Allocator>
	{
	public:
		OpenHashSet() {} ///<

		OpenHashSet(const OpenHashSet& set) : 
			OpenHashTable<
				Key,
				OpenHashSetItem<Key>, 
				HashKeyOp,
				Probe,
				Allocator> (set) {}
	};

	/// The type stored in a HashMap.
	/// @remarks
	/// The stl uses 'pair' for this. 
	template <typename Key, typename Value>
	class OpenHashMapItem
	{
	public:
		OpenHashMapItem(const Key& k = Key()) : m_key(k) {}
		OpenHashMapItem(const Key& k, const Value& v) : m_key(k), m_value(v) {} 
		OpenHashMapItem(const OpenHashMapItem& other) : m_key(other.m_key), m_value(other.m_value) {} 

		Value& GetValue() 
		{
			return m_value;
		} ///< Returns a reference to the value.
		
		const Key& GetKey() const
		{
			return m_key;
		} ///< Returns a const reference to the key. The key cannot be modified after creation of the MapItem.
		
		const Value& GetValue() const
		{
			return m_value;
		} ///< Return a reference to the value.

		void CopyValue(const OpenHashMapItem& rhs)
		{
			m_value = rhs.m_value;
		}

	private:
		Key m_key;
		Value m_value;
	};

	/// %Hash map container class.
	/// Elements in a map are unique and sorted according to the < operator on the keys.
	/// The type stored in the map is MapItem<Key, Value>.
	/// @param Key The sort key for items stored in the map.
	/// @param Value The value type for items stored in the map.
	/// @param Allocator The allocator type, e.g. Ut::StandardAllocator<MapItem>.
	template <
		typename Key, 
		typename Value, 
		typename HashKeyOp = HashKeyOpDefault<Key>,
		typename Probe = OpenHashTableQuadraticProbe<Key, OpenHashMapItem<Key, Value>, HashKeyOp>,
		typename Allocator = ArrayAllocator<OpenHashTableEntry<Key, OpenHashMapItem<Key, Value> > > >
	class OpenHashMap : public OpenHashTable<
		Key, 
		OpenHashMapItem<Key, Value>, 
		HashKeyOp, 
		Probe, 
		Allocator>
	{
	public:
		///\brief Constructor
		OpenHashMap() {}

		///\brief Constructor
		OpenHashMap(const OpenHashMap& map) : 
			OpenHashTable<
				Key,
				OpenHashMapItem<Key, Value>, 
				HashKeyOp,
				Probe,
				Allocator> (map) 
		{
		}

		///\brief Returns a reference to the value associated with the key, inserting a new item if necessary.
		Value& operator [] (const Key& key)
		{
			return Insert(OpenHashMapItem<Key, Value>(key)).first->GetValue();
		}
	};

} // Ut

#endif
