#include "MiscSharedPch.h"

#include <system/debuglog.h>
#include <core/utility/hash.h>
#include <system/serialise.h>
#include "misc_shared\cache\cache.h"
#include "misc_shared\cache\cachestream.h"

namespace DiskCache
{

/// protected constructor
CacheWriteStream::CacheWriteStream(CacheEntry *pParent, const char *pszType) :
	m_pParent(pParent),
	m_sDataType(pszType),
	Sys::C_MemoryStream(128*1024)
{	
}

///	destructor
CacheWriteStream::~CacheWriteStream()
{
	m_pParent->SetDataInternal(m_sDataType, Sys::C_MemoryStream::GetBufferPtr(), (int)Sys::C_MemoryStream::GetBufferSize());
}

};


