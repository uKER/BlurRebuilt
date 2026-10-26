#pragma once

#ifndef UT_OPEN_HASH_SET_H
#define UT_OPEN_HASH_SET_H

#include "utility.h"
//#include "allocator.h"
//#include "vector.h"

namespace Ut
{
	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	class OpenHashSet;

	template<typename Key>
	struct OpenHashSetEntry
	{
		enum State
		{
			ACTIVE,
			EMPTY,
			DELETED
		};

		Key key;
		State state;

		OpenHashSetEntry(const Key& k = Key(), State s = EMPTY) 
			: key(k), state(s) {}
	};

	template <typename Key, typename HashKeyOp>
	class OpenHashLinearProbe
	{
	public:
//		inline unsigned int operator()(
		inline static unsigned int Probe(
			const OpenHashSetEntry<Key>* pHashTable, unsigned int hashTableActiveElements, unsigned int hashTableCapacity, const Key& key)
		{
			unsigned int hashIndex = HashKeyOp::GenerateHashIndex(key, hashTableCapacity);
			if (hashTableActiveElements == hashTableCapacity)
				//Hash table is full.
				return -1;

			//We must have at least one free entry in our hash table.
			while (pHashTable[hashIndex].state != OpenHashSetEntry<Key>::EMPTY
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

	template <typename Key, typename HashKeyOp>
	class OpenHashSetQuadraticProbe
	{
	public:
		inline static unsigned int Probe(
			const OpenHashSetEntry<Key>* pHashTable, unsigned int hashTableActiveElements, unsigned int hashTableCapacity, const Key& key)
		{
			unsigned int hashIndex = HashKeyOp::GetHash(key);
			hashIndex = hashIndex % hashTableCapacity;
			if (hashTableActiveElements == hashTableCapacity)
				//Hash table is full.
				return -1;

			int i = 0;
			//We must have at least one free entry in our hash table.
			while (pHashTable[hashIndex].state != OpenHashSetEntry<Key>::EMPTY
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
	template <typename Key, typename HashKeyOp, typename Probe = OpenHashSetQuadraticProbe<Key, HashKeyOp>, typename Allocator = ArrayAllocator<OpenHashSetEntry<Key> > >
	class OpenHashSet
	{
	public:
		typedef const OpenHashSetEntry<Key>* ConstPointerType;
		typedef const OpenHashSetEntry<Key>& ConstReferenceType;
		typedef OpenHashSetEntry<Key>* PointerType;
		typedef OpenHashSetEntry<Key>& ReferenceType;

		class ConstCheckedIterator
		{
		public:

			// constructors
			ConstCheckedIterator() {}
			ConstCheckedIterator(
				PointerType pointer,
				const UtVector<DynamicVector<OpenHashSetEntry<Key>, Allocator> >* pVector) 
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
					m_pointer->state != OpenHashSetEntry<Key>::ACTIVE && 
					m_pointer < (&(((*m_pVector)[m_pVector->Size()-1]))))
				{
					++m_pointer;
				}
				if (m_pointer != (&((*m_pVector)[0]) + m_pVector->Size()))
					if (m_pointer->state != OpenHashSetEntry<Key>::ACTIVE)
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
					m_pointer->state != OpenHashSetEntry<Key>::ACTIVE && 
					m_pointer < (&(((*m_pVector)[m_pVector->Size()-1]))))
				{
					++m_pointer;
				}
				if (m_pointer != (&((*m_pVector)[0]) + m_pVector->Size()))
					if (m_pointer->state != OpenHashSetEntry<Key>::ACTIVE)
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
					unsigned __int64 size = sizeof(OpenHashSetEntry<Key>);
					unsigned __int64 ptr = (unsigned __int64)m_pointer;
					unsigned __int64 arrayStart = (unsigned __int64)&((*m_pVector)[0]);
					return (unsigned int)((ptr - arrayStart) / size);
#elif defined(YDL)
					unsigned int size = sizeof(OpenHashSetEntry<Key>);
					unsigned int ptr = (long long)m_pointer;
					unsigned int arrayStart = (long long)&((*m_pVector)[0]);
					return (ptr - arrayStart) / size;
#else
					unsigned int size = sizeof(OpenHashSetEntry<Key>);
					unsigned int ptr = (unsigned int)m_pointer;
					unsigned int arrayStart = (unsigned int)&((*m_pVector)[0]);
					return (ptr - arrayStart) / size;
#endif
				#pragma warning(pop)
			}

		protected:
			PointerType m_pointer;
			const UtVector<DynamicVector<OpenHashSetEntry<Key>, Allocator> >* m_pVector;
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
				ConstCheckedIterator::m_pVector->Validate(ConstCheckedIterator::m_pointer);
				return *ConstCheckedIterator::m_pointer;
			}
			PointerType operator->() const 
			{
				ConstCheckedIterator::m_pVector->Validate(ConstCheckedIterator::m_pointer); 
				return ConstCheckedIterator::m_pointer;
			}

			// arithmetic
			CheckedIterator operator + (int n) const 
			{
				return CheckedIterator(ConstCheckedIterator::m_pointer + n, ConstCheckedIterator::m_pVector);
			}
			CheckedIterator operator - (int n) const
			{
				return CheckedIterator(ConstCheckedIterator::m_pointer - n, ConstCheckedIterator::m_pVector);
			}
			int operator - (const CheckedIterator& c) const 
			{
				return ConstCheckedIterator::m_pointer - c.m_pointer;
			}

			CheckedIterator& operator ++()
			{
				if (ConstCheckedIterator::m_pointer == (&((*ConstCheckedIterator::m_pVector)[0]) + ConstCheckedIterator::m_pVector->Size()))
					return *this;

				++ConstCheckedIterator::m_pointer;
				PointerType lastEndPos = ConstCheckedIterator::m_pointer;
				while(
					ConstCheckedIterator::m_pointer->state != OpenHashSetEntry<Key>::ACTIVE && 
					ConstCheckedIterator::m_pointer < (&(((*ConstCheckedIterator::m_pVector)[ConstCheckedIterator::m_pVector->Size()-1]))))
				{
					++ConstCheckedIterator::m_pointer;
				}
				if (ConstCheckedIterator::m_pointer != (&((*ConstCheckedIterator::m_pVector)[0]) + ConstCheckedIterator::m_pVector->Size()))
					if (ConstCheckedIterator::m_pointer->state != OpenHashSetEntry<Key>::ACTIVE)
						ConstCheckedIterator::m_pointer = lastEndPos;

				return *this;
			}

			CheckedIterator operator ++(int) 
			{
				if (ConstCheckedIterator::m_pointer == (&((*ConstCheckedIterator::m_pVector)[0]) + ConstCheckedIterator::m_pVector->Size()))
					return *this;

				++ConstCheckedIterator::m_pointer;
				pointer lastEndPos = ConstCheckedIterator::m_pointer;
				CheckedIterator p(*this); 
				while(
					ConstCheckedIterator::m_pointer->state != OpenHashSetEntry<Key>::ACTIVE && 
					ConstCheckedIterator::m_pointer < (&(((*ConstCheckedIterator::m_pVector)[ConstCheckedIterator::m_pVector->Size()-1]))))
				{
					++ConstCheckedIterator::m_pointer;
				}
				if (ConstCheckedIterator::m_pointer != (&((*ConstCheckedIterator::m_pVector)[0]) + ConstCheckedIterator::m_pVector->Size()))
					if (ConstCheckedIterator::m_pointer->state != OpenHashSetEntry<Key>::ACTIVE)
						ConstCheckedIterator::m_pointer = lastEndPos;

				return p;
			}

		};

	typedef CheckedIterator Iterator; ///< Iterator for Vector elements.
	typedef ConstCheckedIterator ConstIterator; ///< Iterator for Vector elements.


	public:
		OpenHashSet();

		OpenHashSet(unsigned int initialSize);

		OpenHashSet(const OpenHashSet& rhs);

		~OpenHashSet();

		const OpenHashSet& operator=(const OpenHashSet& rhs);

		unsigned int Capacity() const;

		unsigned int NumActiveElements() const;

		void Reserve(unsigned int size);

		Iterator Insert(const Key& key);

		void Erase(const Key& key);

		void EraseByIndex(unsigned int hashKey);

		Iterator Find(const Key& key);

		int FindIndex(const Key& key) const;

		Iterator Begin();

		Iterator End();

		ConstIterator Begin() const;

		ConstIterator End() const;

		const OpenHashSetEntry<Key>& operator[](unsigned int hashIndex) const;

		OpenHashSetEntry<Key>& operator[](unsigned int hashIndex);

		void Optimise();

		void Clear();

	private:
		inline void Rehash();

		void Rebuild(unsigned int capacity);

		inline unsigned int NextPrime(unsigned int thisNumber);

		UtVector<DynamicVector<OpenHashSetEntry<Key>, Allocator> > m_hashTable;
		unsigned int m_nActiveElements;
		unsigned int m_nOccupiedElements;
	};

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashSet<Key, HashKeyOp, Probe, Allocator>::OpenHashSet()
	{
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashSet<Key, HashKeyOp, Probe, Allocator>::OpenHashSet(unsigned int initialSize)
	{
		m_nActiveElements = 0;

		m_hashTable.Resize(initialSize);
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;

		//Reinitialise hash table.
		for (int i = 0; i < m_hashTable.Size(); i++)
			m_hashTable[i].state = OpenHashSetEntry<Key>::EMPTY;
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashSet<Key, HashKeyOp ,Probe, Allocator>::OpenHashSet(const OpenHashSet& rhs)
	{
		*this = rhs;
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline OpenHashSet<Key, HashKeyOp, Probe, Allocator>::~OpenHashSet()
	{
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline const OpenHashSet<Key, HashKeyOp, Probe, Allocator>& OpenHashSet<Key, HashKeyOp, Probe, Allocator>::operator=(const OpenHashSet<Key, HashKeyOp, Probe, Allocator>& rhs)
	{
		m_hashTable = rhs.m_hashTable;
		m_nActiveElements = rhs.m_nActiveElements;
		m_nOccupiedElements = rhs.m_nOccupiedElements;
		return *this;
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	UT_FORCE_INLINE unsigned int OpenHashSet<Key, HashKeyOp, Probe, Allocator>::NumActiveElements() const
	{
		return m_nActiveElements;
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	UT_FORCE_INLINE unsigned int OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Capacity() const
	{
		return m_hashTable.Size();
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Reserve(unsigned int size)
	{
		m_hashTable.Resize(size);
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Iterator OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Insert(const Key& key)
	{
		m_nOccupiedElements++;
		if (m_nOccupiedElements > (unsigned int)m_hashTable.Size() / 2)
		{
			Rehash();
			//Attempt insertion again.
			return Insert(key);
		}

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashSetEntry<Key>::ACTIVE)
		{
			//Duplicate entry - just return.
			return ConstIterator(&m_hashTable[probedHashIndex], &m_hashTable);
		}

		m_nActiveElements++;

		//Insert new record into our hash table.
		m_hashTable[probedHashIndex].key = key;
		m_hashTable[probedHashIndex].state = OpenHashSetEntry<Key>::ACTIVE;

		//We didn't need to rehash.
		return ConstIterator(&m_hashTable[probedHashIndex], &m_hashTable);
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	void OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Erase(const Key& key)
	{
		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashSetEntry<Key>::ACTIVE)
		{
			//Flag as deleted.
			m_hashTable[probedHashIndex].state = OpenHashSetEntry<Key>::DELETED;
			m_nActiveElements--;
		}
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	void OpenHashSet<Key, HashKeyOp, Probe, Allocator>::EraseByIndex(unsigned int hashIndex)
	{
		if (m_hashTable[hashIndex].state == OpenHashSetEntry<Key>::ACTIVE)
		{
			//Flag as deleted.
			m_hashTable[hashIndex].state = OpenHashSetEntry<Key>::DELETED;
			m_nActiveElements--;
		}
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Iterator OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Find(const Key& key)
	{
		//Empty hash table!
		if (m_hashTable.Size() == 0)
			return End();

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashSetEntry<Key>::ACTIVE)
		{
			//We found what we were looking for.
			return ConstIterator(const_cast<OpenHashSetEntry<Key>*>(&m_hashTable[probedHashIndex]), &m_hashTable);
		}

		//We didn't find anything.
		return End();
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline int OpenHashSet<Key, HashKeyOp, Probe, Allocator>::FindIndex(const Key& key) const
	{
		//Empty hash table!
		if (m_hashTable.Size() == 0)
			return -1;

		unsigned int probedHashIndex = Probe::Probe(
			&(m_hashTable[0]), 
			m_nActiveElements, 
			(unsigned int)m_hashTable.Size(),
			key);
		if (m_hashTable[probedHashIndex].state == OpenHashSetEntry<Key>::ACTIVE)
		{
			//We found what we were looking for.
			return probedHashIndex;
		}

		//We didn't find anything.
		return -1;
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashSet<Key, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Begin() const
	{
		const OpenHashSetEntry<Key>* pEntry = 0;
		for (int i = 0; i < m_hashTable.Size(); i++)
		{
			if (m_hashTable[i].state == OpenHashSetEntry<Key>::ACTIVE)
			{
				pEntry = &m_hashTable[i];
				break;
			}
		}

		if (!pEntry)
			return End();
		return ConstIterator(const_cast<OpenHashSetEntry<Key>*>(pEntry), &m_hashTable);
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	UT_FORCE_INLINE typename OpenHashSet<Key, HashKeyOp, Probe, Allocator>::ConstIterator OpenHashSet<Key, HashKeyOp, Probe, Allocator>::End() const
	{
		if (m_hashTable.Size() == 0)
		{
			const OpenHashSetEntry<Key>* pEntry = &m_hashTable[0];
			return ConstIterator(const_cast<OpenHashSetEntry<Key>*>(pEntry), &m_hashTable);
		}

		const OpenHashSetEntry<Key>* pEntry = &m_hashTable[m_hashTable.Size()-1];
		for (int i = m_hashTable.Size()-1; i >= 0; i--)
		{
			if (m_hashTable[i].state == OpenHashSetEntry<Key>::ACTIVE)
			{
				pEntry = &m_hashTable[i];
				break;
			}
		}
		
		pEntry++;
		return ConstIterator(const_cast<OpenHashSetEntry<Key>*>(pEntry), &m_hashTable);
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline typename OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Iterator OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Begin()
	{
		OpenHashSetEntry<Key>* pEntry = 0;
		for (int i = 0; i < m_hashTable.Size(); i++)
		{
			if (m_hashTable[i].state == OpenHashSetEntry<Key>::ACTIVE)
			{
				pEntry = &m_hashTable[i];
				break;
			}
		}

		if (!pEntry)
			return End();
		return ConstIterator(pEntry, &m_hashTable);
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	UT_FORCE_INLINE typename OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Iterator OpenHashSet<Key, HashKeyOp, Probe, Allocator>::End()
	{
		OpenHashSetEntry<Key>* pEntry; 
		if (m_hashTable.Size() > 0)
		{
			pEntry = &m_hashTable[m_hashTable.Size()-1];
			for (int i = m_hashTable.Size()-1; i >= 0; i--)
			{
				if (m_hashTable[i].state == OpenHashSetEntry<Key>::ACTIVE)
				{
					pEntry = &m_hashTable[i];
					break;
				}
			}
			pEntry++;
		}
		else
		{
			//No hash table!
			pEntry = 0;
		}
		return ConstIterator(pEntry, &m_hashTable);
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	UT_FORCE_INLINE OpenHashSetEntry<Key>& OpenHashSet<Key, HashKeyOp, Probe, Allocator>::operator[](unsigned int hashIndex) 
	{
		return m_hashTable[hashIndex];
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	UT_FORCE_INLINE const OpenHashSetEntry<Key>& OpenHashSet<Key, HashKeyOp, Probe, Allocator>::operator[](unsigned int hashIndex) const
	{
		return m_hashTable[hashIndex];
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Clear()
	{
		//Call destructors and then placement new upon contained objects.
		m_hashTable.Clear();
		m_hashTable.Resize(m_hashTable.Capacity());
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;

		//Reinitialise hash table.
		for (int i = 0; i < m_hashTable.Size(); i++)
			m_hashTable[i].state = OpenHashSetEntry<Key>::EMPTY;
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline unsigned int OpenHashSet<Key, HashKeyOp, Probe, Allocator>::NextPrime(unsigned int thisNumber)
	{
		static const unsigned int POW2_32MINUS1 = 0xffffff;
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

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Rebuild(unsigned int capacity)
	{
		UtVector<DynamicVector<OpenHashSetEntry<Key>, Allocator> > oldTable = m_hashTable;
		//Call destructors and then placement new upon contained objects.
		m_hashTable.Clear();
		m_hashTable.Resize(capacity);
		m_nActiveElements = 0;
		m_nOccupiedElements = 0;

		//Initialise new hash table.
		for (int i = 0; i < m_hashTable.Size(); i++)
			m_hashTable[i].state = OpenHashSetEntry<Key>::EMPTY;

		//Insert elements into table.
		for (int i = 0; i < oldTable.Size(); i++)
			if (oldTable[i].state == OpenHashSetEntry<Key>::ACTIVE)
				Insert(oldTable[i].key);
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Rehash()
	{
		Rebuild(NextPrime(m_hashTable.Capacity()*2));
	}

	template <typename Key, typename HashKeyOp, typename Probe, typename Allocator>
	inline void OpenHashSet<Key, HashKeyOp, Probe, Allocator>::Optimise()
	{
		Rebuild(NextPrime(m_hashTable.Capacity()));
	}
}

#endif
