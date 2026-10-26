#ifndef __MISC_SHARED_CACHE_H__
#define __MISC_SHARED_CACHE_H__

#include <system/serialise/I_File.h>
#include <system/serialise/C_MemoryStream.h>
#include "misc_shared/cache/cachekey.h"
#include "misc_shared/cache/cacheentry.h"

namespace DiskCache
{

/// cache arbitrary binary data in a disk cache
/// the cache is automatically saved and can be restored by reloading it from disk
class Cache
{
	friend class CacheEntry;
public:
	/// construct cache from a folder on disk
				Cache(const char *pszFolderName, int iVersion, const char *pszCacheName, bool bDeleteOldCache);

	/// destructor
				~Cache();

	/// create a new cache entry, deletes existing one if present
	CacheEntry	*CreateEntry(CacheKey key);

	/// lookup cache entry, returns NULL if it doesn't exist
	CacheEntry	*GetEntry(CacheKey key);

protected:
	/// write a new cache entry into the cache - remove any existing entry
	void		Write(CacheEntry *pEntry);

private:
	/// calculates a cache filename given a cache key
	Util::C_String	GetFilenameFromKey(CacheKey key);

	/// returns the cache directory name
	Util::C_String	GetCacheDirectoryPath();

	/// create the cache directory if it doesn't already exist, also deletes old version cache directories if found
	void			CreateCacheDirectory(bool bDeleteOldCache);

	Util::C_String		m_sCacheFolder;
	Util::C_String		m_sCacheName;

	int					m_iDataVersion;
	const static int	m_iCacheVersion = 1;
};

}

#endif
