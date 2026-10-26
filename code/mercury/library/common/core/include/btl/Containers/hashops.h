#ifndef BTL_HASH_OPS_H
#define BTL_HASH_OPS_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Hash Ops
//////////////////////////////////////////////////////

#include "btl/utility/hash.h"

namespace Btl
{
	class HashKeyOpString
	{
	public:
		//Hashing function.
		static U32 GetHash(const char* key)
		{
			return HashRaw((U8*) key, (U32) strlen(key), 0);
		}

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
		static U32 GetHash(const Key& key)
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
		static U32 GetHash(const Key& key)
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
		static U32 GetHash(const void* p)
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
		static U32 GetHash(const Key& key)
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
