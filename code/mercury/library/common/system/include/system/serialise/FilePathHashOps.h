#ifndef __SYSTEM_SERIALISE_FILEPATHHASHOPS_H__
#define __SYSTEM_SERIALISE_FILEPATHHASHOPS_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <Btl/Utility/Hash.h>

namespace Sys
{
	class HashKeyOpFilePath
	{
	public:
		//Hashing function.
		static U32 GetHash(const C_FilePath& key)
		{
			return Btl::HashRaw((U8*)key.Chars(), (U32)key.Length(), 0);
		}

		static bool Equal(const C_FilePath& key1, const C_FilePath& key2)
		{
			if (key1 == key2)
				return true;

			return false;
		}
	};
}

#endif
