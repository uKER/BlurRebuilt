#ifndef __MISC_SHARED_CACHESTREAM_H__
#define __MISC_SHARED_CACHESTREAM_H__

#include <system/serialise/C_MemoryStream.h>

namespace DiskCache
{

class CacheWriteStream : public Sys::C_MemoryStream
{
	friend class CacheEntry;
protected:
	/// protected constructor
			CacheWriteStream(CacheEntry *pParent, const char *pszDataType);
public:
	///	destructor
			~CacheWriteStream();
protected:
	CacheEntry		*m_pParent;
	Util::C_String	m_sDataType;
};

}

#endif
