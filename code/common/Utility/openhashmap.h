#pragma once

#ifndef UT_OPEN_HASH_MAP_H
#define UT_OPEN_HASH_MAP_H

//#include "allocator.h"
//#include "vector.h"

namespace Ut
{
	template<typename Key, typename Value>
	struct OpenHashMapEntry
	{
		enum State
		{
			ACTIVE,
			EMPTY,
			DELETED
		};

		Key key;
		Value value;

		State state;

		OpenHashMapEntry(const Key& k = Key(), State s = EMPTY) 
			: key(k), state(s) {}
	};

	template <typename Key, typename Value, typename HashKeyOp>
	class OpenHashMapLinearProbe
	{
	public:
//		inline unsigned int operator()(
		inline static unsigned int Probe(
			const OpenHashMapEntry<Key, Value>* pHashTable, unsigned int hashTableActiveElements, unsigned int hashTableCapacity, const Key& key)
		{
			unsigned int hashIndex = HashKeyOp::GenerateHashIndex(key, hashTableCapacity);
			if (hashTableActiveElements == hashTableCapacity)
				//Hash table is full.
				return -1;

			//We must have at least one free entry in our hash table.
			while (pHashTable[hashIndex].state != OpenHashMapEntry<Key>::EMPTY
				&& !HashKeyOp.Equal(key, pHashTable[hashIndex].key))
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

	template <typename Key, typename Value, typename HashKeyOp>
	class OpenHashMapQuadraticProbe
	{
	public:
		inline static unsigned int Probe(
			const OpenHashMapEntry<Key, Value>* pHashTable, 
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
			while (pHashTable[hashIndex].state != OpenHashMapEntry<Key, Value>::EMPTY
				&& !HashKeyOp::Equal(key, pHashTable[hashIndex].key))
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


	//An implementation of a Hash set using open addressing and thus lazy deletion.
	template <
		typename Key, 
		typename Value,
		typename HashKeyOp,
		typename Probe = OpenHashMapQuadraticProbe<Key, Value, HashKeyOp>,
		typename Allocator = ArrayAllocator<OpenHashMapEntry<Key, Value> > >
	class OpenHashMap
	{
	public:
		typedef const OpenHashMapEntry<Key, Value>* ConstPointerType;
		typedef const OpenHashMapEntry<Key, Value>& ConstReferenceType;
		typedef OpenHashMapEntry<Key, Value>* PointerType;
		typedef OpenHashMapEntry<Key, Value>& ReferenceType;

		class ConstCheckedIterator
		{
		public:

			// constructors
			ConstCheckedIterator() {}
			ConstCheckedIterator(
				PointerType pointer,
				const UtVector<DynamicVector<OpenHashMapEntry<Key, Value>, Allocator> >* pVector) 
				: m_pointer(pointer), m_pVector(pVector) {}
			ConstCheckedIterator(const ConstCheckedIterator& c) : m_pointer(c.m_pointer), m_pVector(c.m_pVector) {}
			ConstCheckedIterator& operator = (const ConstCheckedIterator& c) 
			{
				m_pointer = c.m_pointer; 
				m_pVector = c.m_pVector;
				return *this;
			}
			
			// comparison
			bool operator == (const ConstCheckedIterator& c) const 
			{
				return m_pointer == c.m_pointer;
			}
			bool operator == (ConstPointerType p) const 
			{
				return m_pointer == p;
			}
			bool operator != (const ConstCheckedIterator& c) const 
			{
				return m_pointer != c.m_pointer;
			}
			bool operator != (ConstPointerType p) const
			{
				return m_pointer != p;
			}

			// return T
			ConstReferenceType operator*() const 
			{
				m_pVector->Validate(m_pointer);
				return *m_pointer;
			}
			ConstPointerType operator->() const 
			{
				m_pVector->Validate(m_pointer); 
				return m_pointer;
			}

			// arithmetic
			ConstCheckedIterator operator + (int n) const 
			{
				return ConstCheckedIterator(m_pointer + n, m_pVector);
			}
			ConstCheckedIterator operator - (int n) const
			{
				return ConstCheckedIterator(m_pointer - n, m_pVector);
			}
			int operator - (const ConstCheckedIterator& c) const 
			{
				return m_pointer - c.m_pointer;
			}

			ConstCheckedIterator& operator ++()
			{
				if (m_pointer == (&((*m_pVector)[0]) + m_pVector->Size()))
					return *this;

				++m_pointer;
				PointerType lastEndPos = m_pointer;
				while(
					m_pointer->state != OpenHashMapEntry<Key>::ACTIVE && 
					m_pointer < (&(((*m_pVector)[m_pVector->Size()-1]))))
				{
					++m_pointer;
				}
				if (m_pointer != (&((*m_pVector)[0]) + m_pVector->Size()))
					if (m_pointer->state != OpenHashMapEntry<Key>::ACTIVE)
						m_pointer = lastEndPos;

				return *this;
			}

			ConstCheckedIterator operator ++(int) 
			{
				if (m_pointer == (&((*m_pVector)[0]) + m_pVector->Size()))
					return *this;

				++m_pointer;
				PointerType lastEndPos = m_pointer;
				ConstCheckedIterator p(*this); 
				while(
					m_pointer->state != OpenHashMapEntry<Key>::ACTIVE && 
					m_pointer < (&(((*m_pVector)[m_pVector->Size()-1]))))
				{
					++m_pointer;
				}
				if (m_pointer != (&((*m_pVector)[0]) + m_pVector->Size()))
					if (m_pointer->state != OpenHashMapEntry<Key>::ACTIVE)
						m_pointer = lastEndPos;

				return p;
			}

			bool operator < (const ConstCheckedIterator& other) const
			{
				assert(m_pVector == other.m_pVector); 
				return m_pointer < other.m_pointer;
			}

			unsigned int Index() const
			{
				#pragma warning(push)
					#pragma warning(disable: 4311)
#ifdef WIN64
					unsigned __int64 size = sizeof(OpenHashMapEntry<Key>);
					unsigned __int64 ptr = (unsigned __int64)m_pointer;
					unsigned __int64 arrayStart = (unsigned __int64)&((*m_pVector)[0]);
					return (unsigned int)((ptr - arrayStart) / size);
#else
					unsigned int size = sizeof(OpenHashMapEntry<Key>);
					unsigned int ptr = (unsigned int)m_pointer;
					unsigned int arrayStart = (unsigned int)&((*m_pVector)[0]);
					return (ptr - arrayStart) / size;
#endif
				#pragma warning(pop)
			}

		protected:
			PointerType m_pointer;
			const UtVector<DynamicVector<OpenHashMapEntry<Key, Value>, Allocator> >* m_pVector;
		};

		class CheckedIterator : public ConstCheckedIterator
		{
		public:
			typedef PointerType pointer;
			typedef ReferenceType reference;
			
			// constructors
			CheckedIterator() {}
			CheckedIterator(const ConstCheckedIterator& c) : ConstCheckedIterator(c) {}
			CheckedIterator& operator = (const CheckedIterator c)
			{
				ConstCheckedIterator::m_pointer = c.ConstCheckedIterator::m_pointer;
				ConstCheckedIterator::m_pVector = c.ConstCheckedIterator::m_pVector;
				return *this;
			}
			// return T
			ReferenceType operator*() const 
			{
				m_pVector->Validate(m_pointer);
				return *m_pointer;
			}
			PointerType operator->() const 
			{
				m_pVector->Validate(m_pointer); 
				return m_pointer;
			}

			// arithmetic
			CheckedIterator operator + (int n) const 
			{
				return CheckedIterator(m_pointer + n, m_pVector);
			}
			CheckedIterator operator - (int n) const
			{
				return CheckedIterator(m_pointer - n, m_pVector);
			}
			int operator - (const CheckedIterator& c) const 
			{
				return m_pointer - c.m_pointer;
			}

			CheckedIterator& operator ++()
			{
				if (m_pointer == (&((*m_pVector)[0]) + m_pVector->Size()))
					return *this;

				++m_pointer;
				PointerType lastEndPos = m_pointer;
				while(
					m_pointer->state != OpenHashMapEntry<Key>::ACTIVE && 
					m_pointer < (&(((*m_pVector)[m_pVector->Size()-1]))))
				{
					++m_pointer;
				}
				if (m_pointer != (&((*m_pVector)[0]) + m_pVector->Size()))
					if (m_pointer->state != OpenHashMapEntry<Key>::ACTIVE)
						m_pointer = lastEndPos;

				return *this;
			}

			CheckedIterator operator ++(int) 
			{
				if (m_pointer == (&((*m_pVector)[0]) + m_pVector->Size()))
					return *this;

				++m_pointer;
				pointer lastEndPos = m_pointer;
				CheckedIterator p(*this); 
				while(
					m_pointer->state != OpenHashMapEntry<Key>::ACTIVE && 
					m_pointer < (&(((*m_pVector)[m_pVector->Size()-1]))))
				{
					++m_pointer;
				}
				if (m_pointer != (&((*m_pVector)[0]) + m_pVector->Size()))
					if (m_pointer->state != OpenHashMapEntry<Key>::ACTIVE)
						m_pointer = lastEndPos;

				return p;
			}

		};

	typedef CheckedIterator Iterator; ///< Iterator for Vector elements.
	typedef ConstCheckedIterator ConstIterator; ///< Iterator for Vector elements.


	public:
		OpenHashMap();

		OpenHashMap(unsigned int initialSize);

		OpenHashMap(const OpenHashMap& rhs);

		~OpenHashMap();

		const OpenHashMap& operator=(const OpenHashMap& rhs);

		unsigned int Capacity() const;

		unsigned int NumActiveElements() const;

		void Reserve(unsigned int size);

		Iterator Insert(const Key& key, const Value& value);

		void Erase(const Key& key);

		void EraseByIndex(unsigned int hashKey);

		Iterator Find(const Key& key);

		int FindIndex(const Key& key) const;

		Iterator Begin();

		Iterator End();

		ConstIterator Begin() const;

		ConstIterator End() const;

		const OpenHashMapEntry<Key, Value>& operator[](unsigned int hashIndex) const;

		OpenHashMapEntry<Key, Value>& operator[](unsigned int hashIndex);

		void Optimise();

		void Clear();

	private:
		inline void Rehash();

		void Rebuild(unsigned int capacity);

		inline unsigned int NextPrime(unsigned int thisNumber);

		UtVector<DynamicVector<OpenHashMapEntry<Key, Value>, Allocator> > m_hashTable;
		unsigned int m_nActiveElements;
		unsigned int m_nOccupiedElements;
	};

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::OpenHashMap()
	{
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::OpenHashMap(unsigned int initialSize)
	{
		m_nActiveElements = 0;
		m_hashTable.Resize(initialSize);
		Clear();
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashMap<Key, Value, HashKeyOp ,Probe, Allocator>::OpenHashMap(const OpenHashMap& rhs)
	{
		*this = rhs;
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::~OpenHashMap()
	{
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline const OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>& OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::operator=(
		const OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>& rhs)
	{
		m_hashTable = rhs.m_hashTable;
		m_nActiveElements = rhs.m_nActiveElements;
		m_nOccupiedElements = rhs.m_nOccupiedElements;
		return *this;
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	__forceinline unsigned int OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::NumActiveElements() const
	{
		return m_nActiveElements;
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	__forceinline unsigned int OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Capacity() const
	{
		return m_hashTable.Size();
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Reserve(unsigned int size)
	{
		m_hashTable.Resize(size);
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Iterator OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Insert(const Key& key, const Value& value)
	{
		m_nOccupiedElements++;
		if (m_nOccupiedElements > (unsigned int)m_hashTable.Size() / 2)
		{
			Rehash();
			//Attempt insertion again.
			return Insert(key, value);
		}

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashMapEntry<Key, Value>::ACTIVE)
		{
			//Duplicate entry - just return.
			return ConstIterator(&m_hashTable[probedHashIndex], &m_hashTable);
		}

		m_nActiveElements++;

		//Insert new record into our hash table.
		m_hashTable[probedHashIndex].key = key;
		m_hashTable[probedHashIndex].value = value;
		m_hashTable[probedHashIndex].state = OpenHashMapEntry<Key, Value>::ACTIVE;

		//We didn't need to rehash.
		return ConstIterator(&m_hashTable[probedHashIndex], &m_hashTable);
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	void OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Erase(const Key& key)
	{
		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashMapEntry<Key, Value>::ACTIVE)
		{
			//Flag as deleted.
			m_hashTable[probedHashIndex].state = OpenHashMapEntry<Key, Value>::DELETED;
			m_nActiveElements--;
		}
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	void OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::EraseByIndex(unsigned int hashIndex)
	{
		if (m_hashTable[hashIndex].state == OpenHashMapEntry<Key>::ACTIVE)
		{
			//Flag as deleted.
			m_hashTable[hashIndex].state = OpenHashMapEntry<Key>::DELETED;
			m_nActiveElements--;
		}
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Iterator OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Find(const Key& key)
	{
		//Empty hash table!
		if (m_hashTable.Size() == 0)
			return End();

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashMapEntry<Key, Value>::ACTIVE)
		{
			//We found what we were looking for.
			return ConstIterator(const_cast<OpenHashMapEntry<Key, Value>*>(&m_hashTable[probedHashIndex]), &m_hashTable);
		}

		//We didn't find anything.
		return ConstIterator(const_cast<OpenHashMapEntry<Key, Value>*>(&m_hashTable[m_hashTable.Size()]), &m_hashTable);
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline int OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::FindIndex(const Key& key) const
	{
		//Empty hash table!
		if (m_hashTable.Size() == 0)
			return -1;

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashMapEntry<Key>::ACTIVE)
		{
			//We found what we were looking for.
			return probedHashIndex;
		}

		//We didn't find anything.
		return -1;
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Begin() const
	{
		const OpenHashMapEntry<Key, Value>* pEntry = 0;
		for (int i = 0; i < m_hashTable.Size(); i++)
		{
			if (m_hashTable[i].state == OpenHashMapEntry<Key>::ACTIVE)
			{
				pEntry = &m_hashTable[i];
				break;
			}
		}

		if (!pEntry)
			return End();
		return ConstIterator(const_cast<OpenHashMapEntry<Key, Value>*>(pEntry), &m_hashTable);
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	__forceinline typename OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::End() const
	{
		const OpenHashMapEntry<Key, Value>* pEntry;
		if (m_hashTable.Size() > 0)
		{
			pEntry = &m_hashTable[m_hashTable.Size()-1];

			for (int i = m_hashTable.Size()-1; i >= 0; i--)
			{
				if (m_hashTable[i].state == OpenHashMapEntry<Key>::ACTIVE)
				{
					pEntry = &m_hashTable[i];
					break;
				}
			}
			pEntry++;
		}
		else
			pEntry = 0;
		return ConstIterator(const_cast<OpenHashMapEntry<Key, Value>*>(pEntry), &m_hashTable);
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Iterator OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Begin()
	{
		OpenHashMapEntry<Key, Value>* pEntry = 0;
		for (int i = 0; i < m_hashTable.Size(); i++)
		{
			if (m_hashTable[i].state == OpenHashMapEntry<Key>::ACTIVE)
			{
				pEntry = &m_hashTable[i];
				break;
			}
		}

		if (!pEntry)
			return End();
		return ConstIterator(pEntry, &m_hashTable);
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	__forceinline typename OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Iterator OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::End()
	{
		OpenHashMapEntry<Key, Value>* pEntry = &m_hashTable[m_hashTable.Size()-1];
		for (int i = m_hashTable.Size()-1; i >= 0; i--)
		{
			if (m_hashTable[i].state == OpenHashMapEntry<Key, Value>::ACTIVE)
			{
				pEntry = &m_hashTable[i];
				break;
			}
		}
		
		pEntry++;
		return ConstIterator(pEntry, &m_hashTable);
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	__forceinline OpenHashMapEntry<Key, Value>& OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::operator[](unsigned int hashIndex) 
	{
		return m_hashTable[hashIndex];
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	__forceinline const OpenHashMapEntry<Key, Value>& OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::operator[](unsigned int hashIndex) const
	{
		return m_hashTable[hashIndex];
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Clear()
	{
		//Call destructors and then placement new upon contained objects.
		m_hashTable.Clear();
		m_hashTable.Resize(m_hashTable.Capacity());
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;

		//Reinitialise hash table.
		for (int i = 0; i < m_hashTable.Size(); i++)
			m_hashTable[i].state = OpenHashMapEntry<Key, Value>::EMPTY;
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline unsigned int OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::NextPrime(unsigned int thisNumber)
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

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Rebuild(unsigned int capacity)
	{
		UtVector<DynamicVector<OpenHashMapEntry<Key, Value>, Allocator> > oldTable = m_hashTable;
		//Call destructors and then placement new upon contained objects.
		m_hashTable.Clear();
		m_hashTable.Resize(capacity);
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;

		//Initialise new hash table.
		for (int i = 0; i < m_hashTable.Size(); i++)
			m_hashTable[i].state = OpenHashMapEntry<Key, Value>::EMPTY;

		//Insert elements into table.
		for (int i = 0; i < oldTable.Size(); i++)
			if (oldTable[i].state == OpenHashMapEntry<Key, Value>::ACTIVE)
			{
				Insert(oldTable[i].key, oldTable[i].value);
			}
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Rehash()
	{
		Rebuild(NextPrime(m_hashTable.Capacity()*2));
	}

	template <typename Key, typename Value, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashMap<Key, Value, HashKeyOp, Probe, Allocator>::Optimise()
	{
		Rebuild(NextPrime(m_hashTable.Capacity()));
	}
}

#endif
