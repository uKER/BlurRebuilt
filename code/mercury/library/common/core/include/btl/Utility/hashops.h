#ifndef BTL_UTIL_HASH_OPS_H
#define BTL_UTIL_HASH_OPS_H
#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Hash Ops
//////////////////////////////////////////////////////

#include "btl/utility/hash.h"

namespace Btl
{
	class HashKeyOpBtlString
	{
	public:
		//Hashing function.
		static U32 GetHash(const Btl::String& key)
		{
			return HashRaw((U8*)key.Chars(), (U32)key.Length(), 0);
		}

		static bool Equal(const Btl::String& key1, const Btl::String& key2)
		{
			if (key1 == key2)
				return true;

			return false;
		}
	};
}

#endif
