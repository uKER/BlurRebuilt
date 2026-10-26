// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: ALL
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

// PURPOSE: This class is a templated, easy-to-use, hash map that associates
// keys to values.

#ifndef BD_HASHMAP_H
#define BD_HASHMAP_H

#include <bdCore/bdMemory/bdMemory.h>
#include <bdCore/bdString/bdString.h>

/// The default load factor at which the map will rehash itself.
#define BD_HASHMAP_DEFAULT_LOAD_FACTOR (0.75f)

/// The default starting capacity of all bdHashMaps.
#define BD_HASHMAP_DEFAULT_CAPACITY (4)

/// This specifies whether to use a prime number for the map capacity.
/// Using prime capacities causes the getHashIndex() method to be more
/// expensive but gives a better distribution. If this is commented out, the
/// class will default to using capacities that are powers of two. This makes
/// getHashIndex() extremely fast, but creates more "funnels" in the
/// distribution and hence longer buckets.
///  #define BD_HASHMAP_USE_PRIME_CAPACITY

/// This class implements the hashing mechanism.
/// It works for bdString, simple data types and classes that do not contain
/// references or pointers.
class bdHashingClass
{
	public:
		/// Templated function calculates the 32-bit hash for a key.
		/// \param key[in]	The \c KEY_TYPE to calculate a hash code for.
		/// \return			The hash code that was calculated.
		template <typename KEY_TYPE>
		inline bdUInt getHash(const KEY_TYPE &key) const
		{
			bdUInt h = 0;
			bdUInt off = 0;
			const bdUByte8 *val = reinterpret_cast<const bdUByte8*>(&key);
			bdUInt len = sizeof(key);

			for (bdUInt i = 0; i < len; i++)
			{
				h = 31*h + *(val+off++);
			}

			return h;
		}

	#ifdef BD_COMPILER_MSVC
		// Microsft Visual Studio version.
		/// Explicit template specialization for hashing a bdString
		/// \param key[in]	The \c KEY_TYPE to calculate a hash code for.
		/// \return			The hash code that was calculated.
		template<>
		inline bdUInt getHash(const bdString &key) const
		{
			bdUInt h = 0;
			bdUInt off = 0;
			bdUWord len = key.getLength();
			const bdNChar8* val = key.getBuffer();

			for(bdUInt i = 0; i<len; i++)
			{
				h = 31*h + *(val+off++);
			}

			return h;
		}
	#endif // BD_COMPILER_MSVC

};

#ifndef BD_COMPILER_MSVC
// GNU version.
/// Explicit template specialization for hashing a bdString.
/// \param key[in]	The \c KEY_TYPE to calculate a hash code for.
/// \return			The hash code that was calculated.
template<>
inline bdUInt bdHashingClass::getHash<bdString>(const bdString &key) const
{
	bdUInt h = 0;
	bdUInt off = 0;
	bdUInt len = key.getLength();
	const bdNChar8* val = key.getBuffer();

	for(bdUInt i = 0; i<len; i++)
	{
		h = 31*h + *(val+off++);
	}

	return h;
}
#endif // BD_COMPILER_MSVC

/// This class implements a templated hash map.
/// It implements an array (\a m_map) of pointers to "buckets", which are
/// singly-linked lists of nodes that associate a key with a value. The index
/// into the array is the remainder of key hash divided by the current array
/// size. The map is also self-balancing. It automatically resizes itself when
/// a load threshold has been reached. All keys and values in this container
/// are copied, so the correct constructors and destructors are called
/// automatically.
/// \warning This class uses bdHashingClass to transform \c KEY_TYPE objects
/// into indexes into the map. Currently this class is designed to work only
/// on simple types, bdString, and any other classes that can be
/// equated with each other using their binary representation.
template <typename KEY_TYPE, typename VALUE_TYPE, typename HASHING_CLASS = bdHashingClass>
class bdHashMap
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

	protected:

		/// A node that represents a key-value binding, and can be linked to
		/// form a bucket.
		class Node
		{
			public:

				BD_DECLARE_NEW_AND_DELETE_OPERATORS

				/// Constructor.
				/// \param key[in]		The key to insert into this node.
				/// \param value[in]	The value to insert into this node.
				/// \param next[in]		The next node in the bucket.
				Node(const KEY_TYPE& key,
					 const VALUE_TYPE& value,
					 Node *const next)
				: m_data(value),
				  m_key(key),
				  m_next(next)
				{
				};

				/// The value stored at this node.
				VALUE_TYPE m_data;

				/// The key under which the value was inserted
				KEY_TYPE m_key;

				/// A pointer to the next node in the list.
				/// This will be equal to \c BD_NULL if this is the last node
				/// in the list.
				Node *m_next;
		};

	public:

		/// Provides an iterator for use with the getIterator() and
		/// next() methods.
		typedef void* Iterator;

		//
		// Construction and destruction.
		//

		/// Constructor.
		/// Create a hash map with space to store \a initialCapacity elements,
		/// and with the load factor set to \a loadFactor.
		/// \param initialCapacity[in]	The initial capacity this map
		///								should be able to contain. The capacity should
		///								be set to the expected number of
		///								elements, so that unnecessary resizes
		///								are avoided.
		/// \param loadFactor[in]		The permissible ratio of used buckets
		///								to unused buckets before a resize is
		///								caused.
		inline bdHashMap(const bdUInt initialCapacity = BD_HASHMAP_DEFAULT_CAPACITY,
						 const bdFloat32 loadFactor = BD_HASHMAP_DEFAULT_LOAD_FACTOR);

		/// Destructor, cleans up the memory associated with the map.
		inline ~bdHashMap();

		//
		// Methods.
		//

		/// Check if the map is empty.
		/// \return
		/// -	True, if the map is empty.
		/// -	False, if the map is not empty.
		inline bdBool isEmpty() const;

		/// Get the number of key-value associations in the map.
		/// \return	The number of associations in the map.
		inline bdInt getSize() const;

		/// Remove all associations from this map.
		void clear();

		/// Get the value associated with the specified key.
		/// \param key[in]		The key to retrieve the value for.
		/// \param value[out]	Receives the found value, if it exists.
		/// \return
		/// -	True, if the key exists in the map and \a value was set.
		/// -	False, if the key does not exist in the map and \a value was not
		///		set.
		bdBool get(const KEY_TYPE &key,
				   VALUE_TYPE &value) const;

		/// Associate a copy of the specified value with a copy of the
		/// specified key.
		/// \param key[in]		The key to associate with \a value.
		/// \param value[in]	The value to associate.
		/// \return
		/// -	True, if successful.
		/// -	False, if \a key is already associated with a value in this map.
		bdBool put(const KEY_TYPE &key,
				   const VALUE_TYPE &value);

		/// Copy all of the associations from the specified map to this one.
		/// \param map[in]	The map to copy into this map.
		void putAll(const bdHashMap<KEY_TYPE, VALUE_TYPE, HASHING_CLASS> &map);

		/// Remove the association corresponding to \a key from this map.
		/// \param key[in]	The key corresponding to the association to be
		///					removed.
		/// \return
		/// -	True, if \a key was found and its association removed.
		/// -	False, if \a key was not found.
		bdBool remove(const KEY_TYPE &key);

		/// Remove the association corresponding to this key from this map.
		/// \param key[in]		The key corresponding to the association to be
		///						removed.
		/// \param value[out]	Receives the value that was deleted.
		/// \return
		/// -	True, if \a key was found, its association deleted and \a value
		///		set.
		/// -	False, if \a key was not found, and \a value left unchanged.
		bdBool remove(const KEY_TYPE &key,
					  VALUE_TYPE& value);

		/// Removes the entry in the map specified by iterator.
		/// After the element has been removed, the iterator is advanced to
		/// point to the next element in the map.
		/// \param iterator[in,out]	The iterator pointing to the entry to be
		///							removed.
		/// \return
		/// -	True, if the entry was removed.
		/// -	False, if the entry could not be removed.
		bdBool remove(Iterator &iterator);

		/// Determine whether the map contains an association for the specified
		/// key.
		/// \param key[in]	The key to check for.
		/// \return
		/// -	True, if \a key exists in the map.
		/// -	False, if \a key does not exist in the map.
		inline bdBool containsKey(const KEY_TYPE &key) const;

		/// Determine whether the map associates one or more keys to the
		/// specified value.
		/// \warning This function is O(n).
		/// \param value[in]	The value to check for.
		/// \return
		/// -	True, if \a value exists in the map.
		/// -	False, if \a value does not exist in the map.
		bdBool containsValue(const VALUE_TYPE &value) const;

		/// Get a bdHashMap::Iterator for the first association in the map.
		/// \note	This does not correspond to the first association inserted
		///			chronologically.
		/// \return	A bdHashMap::Iterator corresponding to the first
		///			association.
		Iterator getIterator() const;

		/// Get a bdHashMap::Iterator for the association identified by the
		/// specified key.
		/// \param key[in]	The key to retrieve an iterator for.
		/// \return	A bdHashMap::Iterator for \a key.
		Iterator getIterator(const KEY_TYPE &key) const;

		/// Creates a safe copy of an iterator.
		/// \param other[in] The iterator to make a copy from.
		/// \return A new Iterator pointing to the same position as \a other.
		inline Iterator getIterator(const Iterator &other) const;

		/// Releases an iterator. This is used for debugging purposes in
		/// order to detect simultaneous iteration and modification bugs.
		/// \param iterator[in] The iterator to be released.
		void releaseIterator(Iterator iterator) const;

		/// Advances the given bdHashMap::Iterator to the next association in
		/// the map.
		/// \param iterator[in,out]	The iterator to advance.
		void next(Iterator &iterator) const;

		/// Get a const reference to the key at the given bdHashMap::Iterator.
		/// \param iterator[in]	The bdHashMap::Iterator to get the key of.
		/// \return	A const reference to the key at \a iterator.
		inline const KEY_TYPE& getKey(const Iterator iterator) const;

		/// Get a reference to the key at the given bdHashMap::Iterator.
		/// \param iterator[in]	The bdHashMap::Iterator to get the key of.
		/// \return A reference to the key at \a iterator.
		inline KEY_TYPE& getKey(const Iterator iterator);

		/// Get a constant reference to the value at the given
		/// bdHashMap::Iterator.
		/// \param iterator[in]	The bdHashMap::Iterator to get the value of.
		/// \return	A const reference to the value at the \a iterator.
		inline const VALUE_TYPE& getValue(const Iterator iterator) const;

		/// Get a reference to the value at the given bdHashMap::Iterator.
		/// \param iterator[in]	The bdHashMap::Iterator to get the value of.
		/// \return	A reference to the value at \a iterator.
		inline VALUE_TYPE& getValue(const Iterator iterator);

#ifdef BD_DEBUG
		/// Print out some debug stats about the distribution within the map.
		void printStats() const;
#endif // BD_DEBUG

	protected:
		/// Create an empty map of the given capacity and load factor.
		/// \param initialCapacity[in]	The initial capacity that this
		///								can contain without automatically
		///								resizing.
		/// \param loadFactor[in]		The permissible ratio of used buckets
		///								to unused buckets before a resize is
		///								caused.
		void createMap(const bdUInt initialCapacity,
					   const bdFloat32 loadFactor);

		/// Calculate the next highest appropriate size for the \a m_map
		/// array. If \c USE_PRIME_CAPACITY is defined this will be the first
		/// prime greater than the next power of 2. Otherwise it will be the
		/// next power of 2.
		/// \param currentCapacity[in]	The capacity to calculate the next
		///								capacity against.
		/// \return	The next highest capacity that should be used.
		static bdUInt BD_CALL getNextCapacity(const bdUInt currentCapacity);

		/// Re-size the map to \a newSize. This will cause the associations to
		/// be re-distributed.
		/// \param newSize[in]	The size that \a m_map should be resized to.
		void resize(const bdUInt newSize);

		/// Get an index into \a m_map using the given hash value.
		/// \param hash[in]	The 32-bit hash code to convert into an index for
		///					\a m_map.
		/// \return	An index corresponding to \a hash within the range of 0 to
		///			the size of \a m_map.
		inline bdUInt getHashIndex(const bdUInt hash) const;

	protected:

		/// The number of associations in the map.
		bdUInt m_size;

		/// The capacity of the map.
		bdUInt m_capacity;

		/// The load factor of the map.
		bdFloat32 m_loadFactor;

		/// Permissible threshold before rehashing.
		bdUInt m_threshold;

		/// The array of pointers to bdHashMap::Node's which map the
		/// key-value bindings.
		Node **m_map;

		/// The hash function for producing hash values from keys
		HASHING_CLASS m_hashClass;

#ifdef BD_DEBUG
		/// Counts how many iterators are currently being used to iterate
		/// this hashmap. We will prevent insertion/deletion as long as there
		/// is more than 1 iterator being held at the same time.
		bdUInt m_numIterators;
#endif // BD_DEBUG
};

#ifdef BD_HASHMAP_USE_PRIME_CAPACITY

/// Encapsulates a list of prime numbers to be used as sizes for the hash
/// index.
class bdPrimeTable
{
	public:

		BD_DECLARE_NEW_AND_DELETE_OPERATORS

		/// The number of elements in the \a m_primes array.
		static const bdUInt m_numPrimes;

		/// An array of prime numbers.
		/// Define \c BD_HASHMAP_USE_PRIME_CAPACITY to use prime capacities for
		/// bdHashMaps.
		/// The array values are the closest prime numbers larger than each of
		/// the powers of two.
		static const bdUInt m_primes[];

	protected:

		/// Protected default constructor.
		/// The constructor need never be called as everything in this
		/// class is static.
		bdPrimeTable();
};

#endif // BD_HASHMAP_USE_PRIME_CAPACITY

#include <bdCore/bdContainers/bdHashMap.inl>

#endif // BD_HASHMAP_H


