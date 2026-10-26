// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdPlatform/bdPlatformError/bdPlatformError.h>
#include <bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>

#include <bdCore/bdUtilities/bdBitOperations.h>

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::bdHashMap(const bdUInt initialCapacity, 
														  const bdFloat32 loadFactor)
#ifdef BD_DEBUG
:m_numIterators(0)
#endif // BD_DEBUG
{
	createMap(initialCapacity, loadFactor);
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::~bdHashMap()
{
	clear();
	bdDeallocate<Node*>(m_map);
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
bdBool bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::isEmpty() const
{
	return m_size == 0;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
bdInt bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getSize() const
{
	return m_size;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
void bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::clear()
{
#ifdef BD_DEBUG
	BD_ASSERT(m_numIterators == 0, "bdHashMap::clear, another iterator is being held while clearing "
		"the hashmap");
#endif //BD_DEBUG

	// iterate through buckets
	for(bdUInt i = 0; i < m_capacity; i++)
	{
		Node *n = m_map[i];
		// iterate through nodes
		while(n != BD_NULL)
		{
			Node *const last = n;
			n = n->m_next;
			delete last;
		}

		m_map[i] = BD_NULL;
	}
	m_size = 0;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
bdBool bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::get(const KEY_TYPE &key,
														   VALUE_TYPE &value) const
{
	const Iterator iterator = getIterator(key);
	if(iterator)
	{
		value = getValue(iterator);
#ifdef BD_DEBUG
		releaseIterator(iterator);
#endif // BD_DEBUG
		return true;
	}

	return false;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
bdBool bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::put(const KEY_TYPE &key, 
														   const VALUE_TYPE &value)
{
#ifdef BD_DEBUG
	BD_ASSERT(m_numIterators == 0, "bdHashMap::put, another iterator is being held while inserting to hashmap");
#endif // BD_DEBUG

	// get the map index to put this key in
	const bdUInt hash = m_hashClass.getHash(key);
	bdUInt i = getHashIndex(hash);
	const Node *n = m_map[i];

	// iterate through the bucket
	while (n != BD_NULL) 
	{
		//if the key already exists, return false
		if (key == n->m_key) 
		{
			return false;
		}
		n = n->m_next;
	}	

	// check if we should rehash
	if(m_size + 1 > m_threshold)
	{
		resize(m_capacity * 2);
		i = getHashIndex(hash);
	}
	
	m_size++;
	// create the new node
	m_map[i] = new Node(key, value, m_map[i]);

	return true;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS>
void bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::putAll(const bdHashMap<KEY_TYPE,VALUE_TYPE, HASHING_CLASS> &map)
{
	const bdUInt numNewKeys = map.getSize();
	if (numNewKeys == 0)
	{
		return;
	}

	/// If the potential number of new keys is greater than the threshold then
	/// resize the map.
	if ((numNewKeys + m_size) > m_threshold)
	{
		bdUInt newCapacity = static_cast<bdUInt>( (numNewKeys + m_size) / (m_loadFactor ) + 1);
		resize(newCapacity);
	}

	// now iterate through map and add each of the entries
	Iterator iterator = map.getIterator();
	while(iterator)
	{
		const KEY_TYPE& key = map.getKey(iterator);
		const VALUE_TYPE& value = map.getValue(iterator);
		put(key, value);
		map.next(iterator);
	}
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
bdBool bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::remove(const KEY_TYPE &key)
{
#ifdef BD_DEBUG
	BD_ASSERT(m_numIterators == 0, "bdHashMap::remove, another iterator is being held while removing "
		"from hashmap");
#endif // BD_DEBUG

	const bdUInt hash = m_hashClass.getHash(key);
	const bdUInt i = getHashIndex(hash);
	//bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::Node *n;
	//n = m_map[i];
	Node *n = m_map[i];
	Node *prevNode = BD_NULL;
	while (n != BD_NULL) 
	{
		if (key == n->m_key) 
		{
			// Patch up the pointer links in the bucket before deleting.
			if(prevNode)
			{
				// There is a node before us in the bucket so link the one 
				// before us with the one after us
				prevNode->m_next = n->m_next;
			}			
			else
			{
				// This is the first node in the bucket
				m_map[i] = n->m_next;
			}

			delete n;
			m_size--;
			return true;
		}

		prevNode = n;
		n = n->m_next;
	}

	return false;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
bdBool bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::remove(const KEY_TYPE &key,
															  VALUE_TYPE& value)
{
	const bdUInt hash = m_hashClass.getHash(key);
	const bdUInt i = getHashIndex(hash);
	Node *n = m_map[i];
	Node *prevNode = BD_NULL;

#ifdef BD_DEBUG
	BD_ASSERT(m_numIterators == 0, "bdHashMap::remove, another iterator is being held while removing "
		"from hashmap");
#endif // BD_DEBUG

	while (n != BD_NULL) 
	{
		if (key == n->m_key) 
		{
			// Patch up the pointer links in the bucket before deleting.
			if(prevNode)
			{
				// There is a node before us in the bucket so link the one 
				// before us with the one after us
				prevNode->m_next = n->m_next;
			}			
			else
			{
				// This is the first node in the bucket
				m_map[i] = n->m_next;
			}

			value = n->m_data;
			delete n;
			m_size--;
			return true;
		}

		prevNode = n;
		n = n->m_next;
	}

	return false;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS>
bdBool bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::remove(Iterator &iterator)
{
#ifdef BD_DEBUG
	BD_ASSERT(m_numIterators == 1, "bdHashMap::remove, more than one iterator held  while removing "
		"from hashmap");
#endif // #ifdef BD_DEBUG

	Iterator iter = iterator;
	next(iter);
	Node *n = reinterpret_cast<Node*>(iterator);
	iterator = iter;
#ifdef BD_DEBUG
	bdUInt numIterators = m_numIterators;
	bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
	(*numIter) = 0;
	//static_cast<bdUInt>(m_numIterators) = 0;
#endif // BD_DEBUG
	bdBool result =  remove(n->m_key);
#ifdef BD_DEBUG
	//bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
	(*numIter) = numIterators;
	//static_cast<bdUInt>(m_numIterators) = numIterators;
#endif // BD_DEBUG
	return result;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
bdBool bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::containsKey(const KEY_TYPE &key) const
{
	Iterator iterator = getIterator(key);
#ifdef BD_DEBUG
	releaseIterator(iterator);
#endif // BD_DEBUG
	return iterator != BD_NULL;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
bdBool bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::containsValue(const VALUE_TYPE &value) const
{
	// iterate through buckets
	for(bdUInt i = 0; i < m_capacity; i++)
	{
		const Node *n = m_map[i];
		// iterate through nodes
		while(n != BD_NULL)
		{
			if(n->m_data == value)
			{
				return true;
			}
			n = n->m_next;
		}
	}

	return false;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
typename bdHashMap<KEY_TYPE,VALUE_TYPE, HASHING_CLASS>::Iterator bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getIterator() const
{
	if(m_size == 0)
	{
		return BD_NULL;
	}

	// Skip any leading null buckets
	bdUInt i = 0;
	while((i < m_capacity) && (m_map[i] == BD_NULL))
	{
		i++;
	}
#ifdef BD_DEBUG
	if (m_map[i] != BD_NULL)
	{
		bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
		(*numIter)++;
		//static_cast<bdUInt>(m_numIterators)++;
	}
#endif // BD_DEBUG
	return reinterpret_cast<Iterator>(m_map[i]);
}


template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
typename bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::Iterator bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getIterator(const KEY_TYPE &key) const
{
	if(m_size == 0)
	{
		return BD_NULL;
	}

	const bdUInt hash = m_hashClass.getHash(key);
	const bdUInt i = getHashIndex(hash);
	Node *n = m_map[i];
	// iterate through nodes
	while (n != BD_NULL) 
	{
		if (key == n->m_key) 
		{
#ifdef BD_DEBUG
			bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
			(*numIter)++;
			//static_cast<bdUInt>(m_numIterators)++;
#endif // BD_DEBUG
			return reinterpret_cast<Iterator>(n);
		}
		n = n->m_next;
	}
	return BD_NULL;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS>
inline
typename bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::Iterator bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getIterator(const Iterator &other) const
{
#ifdef BD_DEBUG
	if (other != BD_NULL)
	{
		bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
		(*numIter)++;
	}
#endif //BD_DEBUG
	return other;
}

#ifdef BD_DEBUG
template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
void bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::releaseIterator(Iterator iterator) const
{
	if (iterator != BD_NULL)
	{
		BD_ASSERT(m_numIterators != 0,"bdHashMap::releaseIterator"
			"Iterator count reached 0, can't release iterator");

		bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
		(*numIter)--;
		//static_cast<bdUInt>(m_numIterators)--;

	}
}
#else
template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
void bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::releaseIterator(Iterator) const
{
}
#endif // BD_DEBUG

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
void bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::next(Iterator &iterator) const
{	
	const Node *const n = reinterpret_cast<Node*>(iterator);
	if(n->m_next)
	{
		iterator = reinterpret_cast<Iterator>(n->m_next);
	}
	else
	{
		//get an index for this Iterator
		const bdUInt hash = m_hashClass.getHash(n->m_key);
		bdUInt i = getHashIndex(hash) + 1;	
		while(i < m_capacity)
		{
			if(m_map[i] != BD_NULL)
			{
				iterator = reinterpret_cast<Iterator>(m_map[i]);
				return;
			}
			i++;
		}

		iterator = BD_NULL;
#ifdef BD_DEBUG
		{
			bdUInt *numIter = const_cast<bdUInt*>(&m_numIterators);
			(*numIter)--;
			//static_cast<bdUInt>(m_numIterators)--;
		}
#endif // BD_DEBUG
	}
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
const KEY_TYPE& bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getKey(const Iterator iterator) const
{
	const Node *const node = reinterpret_cast<Node*>(iterator);
	return node->m_key;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
KEY_TYPE& bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getKey(const Iterator iterator)
{
	Node *const node = reinterpret_cast<Node*>(iterator);
	return node->m_key;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
const VALUE_TYPE& bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getValue(const Iterator iterator) const
{
	const Node *const node = reinterpret_cast<Node*>(iterator);
	return node->m_data;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
VALUE_TYPE& bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getValue(const Iterator iterator)
{
	Node *const node = reinterpret_cast<Node*>(iterator);
	return node->m_data;
}

#ifdef BD_DEBUG
template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS>  
void bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::printStats() const
{
	bdUInt i;
	bdUInt bucketSize;
	BD_INFO("hashmap", "bdHashMap key distribution statistics\n------------------------------------");
	//iterate through buckets
	for(i = 0; i < m_capacity; i++)
	{
		bucketSize = 0;
		Node *n = m_map[i];
		//iterate through nodes
		while(n != BD_NULL)
		{
			bucketSize++;
			n = n->m_next;
		}
		BD_INFO("hashmap", "Bucket %u has \t%u entries", i, bucketSize);
	}
}
#endif // BD_DEBUG

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> 
void bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::createMap(const bdUInt initialCapacity, 
															   const bdFloat32 loadFactor)
{
	
	if (loadFactor <= 0.0f || loadFactor > 1.0f)
	{
		BD_WARN("hashmap","Illegal loadFactor. Using default value.");
		m_loadFactor = BD_HASHMAP_DEFAULT_LOAD_FACTOR;
	}

	m_size = 0;
	m_capacity = getNextCapacity(initialCapacity);
	m_loadFactor = loadFactor;
	m_threshold = static_cast<bdUInt>(m_capacity * m_loadFactor);
	m_map = bdAllocate<Node*>(m_capacity);

	//initialise the m_map array to BD_NULL
	bdMemset(m_map, BD_NULL, m_capacity * sizeof(Node*));
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS>
bdUInt BD_CALL bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getNextCapacity(const bdUInt targetCapacity)
{	
#ifdef BD_HASHMAP_USE_PRIME_CAPACITY
	bdUInt nextCapacity;
	// Find a prime >= currentCapacity
	nextCapacity = bdPrimeTable::m_primes[0];
	bdUInt i = 0;
	while (nextCapacity < targetCapacity)
	{
		//check that we did not overrun the m_primeCapacities array
		if(i >= bdPrimeTable::m_numPrimes - 1)
		{
			BD_WARN("hashmap","Cannot resize hash map above max size.");
			break;
		}
		nextCapacity = bdPrimeTable::m_primes[++i];
	}
#else
	// Find a power of 2 >= currentCapacity
	const bdUInt nextCapacity = bdBitOperations::nextPowerOf2(targetCapacity);

#endif //BD_HASHMAP_USE_PRIME_CAPACITY
	return nextCapacity;
}

template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS>
void bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::resize(const bdUInt newSize)
{
	Node **oldmap;
	oldmap = m_map;
	const bdUInt oldCapacity = m_capacity;
	
	//recalculate capacity
	const bdUInt targetCapacity = getNextCapacity(newSize);

	//we can only resize the map upwards. Return if there is no work to do
	if(targetCapacity <= m_capacity)
	{
		return;
	}

	m_capacity = targetCapacity;
	m_threshold = static_cast<bdUInt>(m_capacity * m_loadFactor);
	m_map = bdAllocate<Node*>(m_capacity);
	m_size = 0;

	//initialise the m_map array to BD_NULL
	bdMemset(m_map, BD_NULL, m_capacity * sizeof(Node*));

	//now iterate through oldmap and add its entries to m_map
	//We delete bucket entries along the way
	{
		for(bdUInt i = 0; i < oldCapacity; i++)
		{
			Node *n = oldmap[i];
			while(n != BD_NULL)
			{
				put(n->m_key, n->m_data);
				Node *const prev = n;
				n = n->m_next;
				delete prev;
			}
		}
	}
	//and deallocate oldmap
	bdDeallocate<Node*>(oldmap);
}
template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS> inline 
bdUInt bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS>::getHashIndex(const bdUInt hash) const
{
#ifdef	BD_HASHMAP_USE_PRIME_CAPACITY
	return hash % m_capacity;
#else
	return hash & (m_capacity-1);
#endif //BD_HASHMAP_USE_PRIME_CAPACITY
}




