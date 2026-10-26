#ifndef UT_OPEN_HASH_TABLE_H
#define UT_OPEN_HASH_TABLE_H

#include "utility.h"
#include "initialiser.h"

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
		inline static unsigned int Probe(
			const OpenHashTableEntry<Key, ItemType>* pHashTable, 
			unsigned int hashTableActiveElements, 
			unsigned int hashTableCapacity, 
			const Key& key)
		{
			unsigned int hashIndex = HashKeyOp::GenerateHashIndex(key, hashTableCapacity);
			if (hashTableActiveElements == hashTableCapacity)
				//Hash table is full.
				return -1;

			//We must have at least one free entry in our hash table.
			while (pHashTable[hashIndex].state != OpenHashTableEntry<Key, ItemType>::EMPTY
				&& !HashKeyOp::Equal(key, pHashTable[hashIndex].key))
			{
				//Resolve collisions...
				hashIndex++;
				if (hashIndex == hashTableCapacity)
					//Wrap index around to the beginning.
					hashIndex = 0;
			}
			return hashIndex;
		}
	};

	//\brief An implementation of linear probing.
	template <typename Key, typename ItemType, typename HashKeyOp>
	class OpenHashTableQuadraticProbe
	{
	public:
		inline static unsigned int Probe(
			const OpenHashTableEntry<Key, ItemType>* pHashTable, 
			unsigned int hashTableActiveElements, 
			unsigned int hashTableCapacity,
			const Key& key)
		{
			unsigned int hashIndex = HashKeyOp::GetHash(key);
			hashIndex = hashIndex % hashTableCapacity;
			if (hashTableActiveElements == hashTableCapacity)
				//Hash table is full.
				return -1;

			int i = 0;
			//We must have at least one free entry in our hash table.
			while (pHashTable[hashIndex].state != OpenHashTableEntry<Key, ItemType>::EMPTY
				&& !HashKeyOp::Equal(key, pHashTable[hashIndex].element.GetKey()))
			{
				//Resolve collisions...
				hashIndex += 2 * ++i - 1;
				if (hashIndex >= hashTableCapacity)
					//Wrap index around to the beginning.
					hashIndex -= hashTableCapacity;
			}
			return hashIndex;
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
		///%Const Iterator for OpenHashTable
		class ConstIterator
		{
		public:
			// constructors
			ConstIterator() {}

			ConstIterator(
				const OpenHashTableEntry<Key, ItemType>* pEntry,
				const UtVector<DynamicVector<OpenHashTableEntry<Key, ItemType>, Allocator> >* pVector) 
				
			{
				m_pEntry = const_cast<OpenHashTableEntry<Key, ItemType>* >(pEntry);
				m_pVector = pVector;
			} ///< @a

			ConstIterator(const ConstIterator& c) : m_pEntry(c.m_pEntry), m_pVector(c.m_pVector) {} ///< @a

			ConstIterator& operator = (const ConstIterator& c) 
			{
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

			ConstIterator& operator ++()
			{
				if (m_pEntry == (&((*m_pVector)[0]) + m_pVector->Size()))
					return *this;

				++m_pEntry;
				OpenHashTableEntry<Key, ItemType>* lastEndPos = m_pEntry;
				while(
					m_pEntry->state != OpenHashTableEntry<Key, ItemType>::ACTIVE && 
					m_pEntry < (&(((*m_pVector)[m_pVector->Size()-1]))))
				{
					++m_pEntry;
				}
				if (m_pEntry != (&((*m_pVector)[0]) + m_pVector->Size()))
					if (m_pEntry->state != OpenHashTableEntry<Key, ItemType>::ACTIVE)
						m_pEntry = lastEndPos;

				return *this;
			} ///< @a

			ConstIterator operator ++(int) 
			{
				if (m_pEntry == (&((*m_pVector)[0]) + m_pVector->Size()))
					return *this;

				++m_pEntry;
				OpenHashTableEntry<Key, ItemType>* lastEndPos = m_pEntry;
				ConstIterator p(*this); 
				while(
					m_pEntry->state != OpenHashTableEntry<Key, ItemType>::ACTIVE && 
					m_pEntry < (&(((*m_pVector)[m_pVector->Size()-1]))))
				{
					++m_pEntry;
				}
				if (m_pEntry != (&((*m_pVector)[0]) + m_pVector->Size()))
					if (m_pEntry->state != OpenHashTableEntry<Key, ItemType>::ACTIVE)
						m_pEntry = lastEndPos;

				return p;
			} ///< @a

			bool operator < (const ConstIterator& other) const
			{
				assert(m_pVector == other.m_pVector); 
				return m_pEntry < other.m_pEntry;
			} 

			unsigned int GetIndex() const
			{
				#pragma warning(push)
					#pragma warning(disable: 4311)
#ifdef __64BIT__
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
				#pragma warning(pop)
			} ///< @a

		protected:
			OpenHashTableEntry<Key, ItemType>* m_pEntry;

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
				const UtVector<DynamicVector<OpenHashTableEntry<Key, ItemType>, Allocator> >* pVector) 
			{
				m_pEntry = pEntry;
				m_pVector = pVector;
			} ///< @a

			Iterator(const ConstIterator& c) : ConstIterator(c) {} ///< @a

			Iterator& operator = (const Iterator c)
			{
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
		void Optimise();

		///\brief Clears the contents of the hash table.
		///
		void Clear();

		///\brief Returns true if the table is empty.
		///
		///@return True if empty, otherwise false.
		bool Empty();

	private:
		FORCE_INLINE Key& GetNonConstKey(const Key& key)
		{
			return const_cast<Key&>(key);
		}

		inline void Rehash();

		void Rebuild(unsigned int capacity);

		inline unsigned int NextPrime(unsigned int thisNumber);

		UtVector<DynamicVector<OpenHashTableEntry<Key, ItemType>, Allocator> > m_hashTable;
		unsigned int m_nActiveElements;
		unsigned int m_nOccupiedElements;
	};

	/// @} end group

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::OpenHashTable()
	{
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::OpenHashTable(unsigned int initialSize)
	{
		m_nActiveElements = 0;
		m_hashTable.Resize(initialSize);
		Clear();
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashTable<Key, ItemType, HashKeyOp ,Probe, Allocator>::OpenHashTable(const OpenHashTable& rhs)
	{
		*this = rhs;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::~OpenHashTable()
	{
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline const OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>& OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::operator=(
		const OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>& rhs)
	{
		m_hashTable = rhs.m_hashTable;
		m_nActiveElements = rhs.m_nActiveElements;
		m_nOccupiedElements = rhs.m_nOccupiedElements;
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
	inline void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Reserve(unsigned int size)
	{
		m_hashTable.Resize(size);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::InsertionReturnType OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Insert(const Key& key)
	{
		int probedHashIndex = -1;
		if (m_hashTable.Size() > 0)
		{
			 probedHashIndex = Probe::Probe(
				&(m_hashTable[0]), 
				m_nActiveElements, 
				(unsigned int)m_hashTable.Size(),
				key);
			if (m_hashTable[probedHashIndex].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
			{
				//Duplicate entry - just return.
				return InsertionReturnType(ConstIterator(&m_hashTable[probedHashIndex], &m_hashTable), false);
			}
		}

		m_nOccupiedElements++;
		if (m_nOccupiedElements > (unsigned int)m_hashTable.Size() / 2)
		{
			Rehash();
			//Attempt insertion again.
			return Insert(key);
		}
		assert(probedHashIndex != -1);
		m_nActiveElements++;

		//Insert new record into our hash table.
		//m_hashTable[probedHashIndex].element.m_key = key;
		GetNonConstKey(m_hashTable[probedHashIndex].element.GetKey()) = key;
		m_hashTable[probedHashIndex].state = OpenHashTableEntry<Key, ItemType>::ACTIVE;

		//We didn't need to rehash.
		return InsertionReturnType(ConstIterator(&m_hashTable[probedHashIndex], &m_hashTable), true);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::InsertionReturnType OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Insert(const ItemType& item)
	{
		return Insert(item.GetKey());
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	unsigned int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Erase(Iterator& it)
	{
		int probedHashIndex = FindIndex(it.first);
		if (probedHashIndex < 0)
			return 0;
		if (m_hashTable[probedHashIndex].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//Flag as deleted.
			m_hashTable[probedHashIndex].state = OpenHashTableEntry<Key, ItemType>::DELETED;
			m_nActiveElements--;
			return 1;
		}
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	unsigned int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::EraseByIndex(unsigned int hashIndex)
	{
		if (m_hashTable[hashIndex].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//Flag as deleted.
			m_hashTable[hashIndex].state = OpenHashTableEntry<Key, ItemType>::DELETED;
			m_nActiveElements--;
			return 1;
		}
		return 0;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Iterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Find(const Key& key)
	{
		//Empty hash table!
		if (m_hashTable.Size() == 0)
			return End();

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//We found what we were looking for.
			return Iterator(const_cast<OpenHashTableEntry<Key, ItemType>*>(&m_hashTable[probedHashIndex]), &m_hashTable);
		}

		//We didn't find anything.
		return Iterator(const_cast<OpenHashTableEntry<Key, ItemType>*>(&m_hashTable[m_hashTable.Size()]), &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Find(const Key& key) const
	{
		OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>* pNonConstThis = 
			const_cast<OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>* >(this);

		return pNonConstThis->Find(key);
	}


	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::FindIndex(const Key& key) const
	{
		//Empty hash table!
		if (m_hashTable.Size() == 0)
			return -1;

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
		{
			//We found what we were looking for.
			return probedHashIndex;
		}

		//We didn't find anything.
		return -1;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Begin() const
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
		return ConstIterator(const_cast<OpenHashTableEntry<Key, ItemType>*>(pEntry), &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	FORCE_INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::End() const
	{
		const OpenHashTableEntry<Key, ItemType>* pEntry;
		if (m_hashTable.Size() > 0)
		{
			pEntry = &m_hashTable[m_hashTable.Size()-1];

			for (int i = m_hashTable.Size()-1; i >= 0; i--)
			{
				if (m_hashTable[i].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
				{
					pEntry = &m_hashTable[i];
					break;
				}
			}
			pEntry++;
		}
		else
			pEntry = 0;
		return ConstIterator(const_cast<OpenHashTableEntry<Key, ItemType>*>(pEntry), &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Iterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Begin()
	{
		OpenHashTableEntry<Key, ItemType>* pEntry = 0;
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
		return ConstIterator(pEntry, &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	FORCE_INLINE typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Iterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::End()
	{
		//Avoid assertions from UtVector bounds checking.
		if (m_hashTable.Size() == 0)
			return ConstIterator(0, &m_hashTable);

		OpenHashTableEntry<Key, ItemType>* pEntry = &m_hashTable[m_hashTable.Size()-1];
		for (int i = m_hashTable.Size()-1; i >= 0; i--)
		{
			if (m_hashTable[i].state == OpenHashTableEntry<Key, ItemType>::ACTIVE)
			{
				pEntry = &m_hashTable[i];
				break;
			}
		}
		
		pEntry++;
		return ConstIterator(pEntry, &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Iterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::FindByIndex(unsigned int hashKey)
	{
		return Iterator(&(m_hashTable[hashKey]), &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::FindByIndex(unsigned int hashKey) const
	{
		return ConstIterator(&(m_hashTable[hashKey]), &m_hashTable);
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Clear()
	{
		//Call destructors and then placement new upon contained objects.
		m_hashTable.Clear();
		m_hashTable.Resize(m_hashTable.Capacity());
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;

		//Reinitialise hash table.
		for (int i = 0; i < m_hashTable.Size(); i++)
			m_hashTable[i].state = OpenHashTableEntry<Key, ItemType>::EMPTY;
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline unsigned int OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::NextPrime(unsigned int thisNumber)
	{
		static const unsigned int POW2_32MINUS1 = 4294967295;
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
	inline void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Rebuild(unsigned int capacity)
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
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Rehash()
	{
		Rebuild(NextPrime(m_hashTable.Capacity()*2));
	}

	template <typename Key, typename ItemType, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashTable<Key, ItemType, HashKeyOp, Probe, Allocator>::Optimise()
	{
		Rebuild(NextPrime(m_hashTable.Capacity()));
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
		typename HashKeyOp,
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
		typename HashKeyOp,
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
