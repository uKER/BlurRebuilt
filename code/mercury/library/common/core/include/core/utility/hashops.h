#ifndef UT_HASH_OPS_H
#define UT_HASH_OPS_H

#if defined(PLATFORM_PS3) || defined(PLATFORM_PC)
#include <string.h>
#endif

#include "utility.h"
#include "hash.h"

namespace Ut
{
	class HashKeyOpString
	{
	public:
#ifdef _STRING_
		//Hashing function.
		static unsigned int GetHash(const std::string& key)
		{
			//Generate a hash from this key
			/*unsigned int hashVal = 0;
			for (unsigned int i = 0; i < key.length(); i++)
			{
					hashVal = 37 * hashVal + key[i];
			}
			return hashVal;*/

			return HashRaw((U8*) key.c_str(), (U32) key.length(), 0);
		}
#endif

		//Hashing function.
		static unsigned int GetHash(const char* key)
		{
			//Generate a hash from this key
			/*unsigned int hashVal = 0;
			for (unsigned int i = 0; i < strlen(key); i++)
			{
					hashVal = 37 * hashVal + key[i];
			}
			return hashVal;*/

			return HashRaw((U8*) key, (U32) strlen(key), 0);
		}

#ifdef _STRING_
		static bool Equal(const std::string& key1, const std::string& key2)
		{
			if (key1 == key2)
				return true;

			return false;
		}
#endif
		static bool Equal(const char* key1, const char* key2)
		{
			if (!strcmp(key1, key2))
				return true;

			return false;
		}
	};

	template<class Key>
	class HashKeyOpByteArray
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const Key& key)
		{
			return HashRaw((U8*) &key, sizeof(key), 0);
		}

		static bool Equal(const Key& key1, const Key& key2)
		{
			if (key1 == key2)
				return true;

			return false;
		}
	};

	// Hash small objects (up to 8 bytes)
	template<class Key>
	class HashKeyOpSmall
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const Key& key)
		{
			return HashSmall(key, 0);
		}

		static bool Equal(const Key& key1, const Key& key2)
		{
			if (key1 == key2)
				return true;

			return false;
		}

		int operator()(const Key& key) const
		{	
			return GetHash(key);
		}

		bool operator()(const Key& key1, const Key& key2) const
		{	
			return key1 < key2;
		}

		enum
		{	// parameters for hash table
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N
	};

	class HashKeyOpPtr
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const void* p)
		{
			return HashSmall(p, 0);
		}

		static bool Equal(const void* p1, const void* p2)
		{
			if (p1 == p2)
				return true;

			return false;
		}
	};

	template<class Key>
	class HashKeyOpDefault
	{
	public:
		//Hashing function.
		static unsigned int GetHash(const Key& key)
		{
			return Hash(key);
		}

		static bool Equal(const Key& key1, const Key& key2)
		{
			if (key1 == key2)
				return true;

			return false;
		}
	};
}

#endif
