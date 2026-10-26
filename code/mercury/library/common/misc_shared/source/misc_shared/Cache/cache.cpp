#include "MiscSharedPch.h"

#include <system/debuglog.h>
#include <core/utility/hash.h>
#include <system/serialise.h>
#include "misc_shared\cache\cache.h"
#include <system/misc/StringUtil.h>
#include <Winbase.h>

namespace DiskCache
{

// utility function that performs a DEL
	void DeleteMatchingFiles(const Util::C_String folder, const Util::C_String filter)
{
	WIN32_FIND_DATA	wfd;
	HANDLE			hFind	= FindFirstFile(folder + filter, &wfd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((strcmp(wfd.cFileName, ".")) && (strcmp(wfd.cFileName, "..")))
			{
				Util::C_String fullPath = folder + wfd.cFileName;
				DeleteFile(fullPath);
			}
		} while (FindNextFile(hFind, &wfd));

		FindClose(hFind);
	}
}

Debug::Logger &g_Log = Debug::GetLogger("common.diskcache");

/// construct cache from a folder on disk
Cache::Cache(const char *pszFolder, int iVersion, const char *pszCacheName, bool bDeleteOldCache) :
	m_sCacheFolder(pszFolder),
	m_sCacheName(pszCacheName),
	m_iDataVersion(iVersion)
{
	CreateCacheDirectory(bDeleteOldCache);
}

/// destructor
Cache::~Cache()
{
}

/// returns the cache directory name
Util::C_String Cache::GetCacheDirectoryPath()
{
	Util::C_String path;
	path.Printf("%s\\%s%d.%d", m_sCacheFolder.Chars(), m_sCacheName.Chars(), m_iCacheVersion, m_iDataVersion);
	return path;
}

/// create the cache directory if it doesn't already exist, also deletes old version cache directories if found
void Cache::CreateCacheDirectory(bool bDeleteOldCache)
{
	Sys::C_FilePath cacheDir = GetCacheDirectoryPath();
	Sys::C_FilePath cacheFilename = Sys::ExtractFilename(cacheDir);
	Sys::GetDefaultFileSystem()->CreateDirectory(cacheDir + "\\");

	// Scan the folder... look for old version cache directories and delete them
	WIN32_FIND_DATA	wfd;
	HANDLE			hFind	= FindFirstFile((cacheDir + "\\..\\*").Chars(), &wfd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((strcmp(wfd.cFileName, ".")) && (strcmp(wfd.cFileName, "..")) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				if (!strncmp(m_sCacheName.Chars(), wfd.cFileName, m_sCacheName.Length()))
				{
					// delete texture cache folders that are older than our current one
					const char *p1 = strrchr(cacheFilename.Chars(), '.');
					const char *p2 = strrchr(wfd.cFileName, '.');
					if (p1 && p2 && atoi(p1+1) > atoi(p2+1))
					{
						const Util::C_String folderName = m_sCacheFolder + "\\" + wfd.cFileName;

						if (bDeleteOldCache)
						{
							LOGDEBUG( g_Log, "Deleting old texture cache folder '%s'...\n", folderName);
							
							// Delete all files in the folder... then delete folder
							DeleteMatchingFiles(folderName + "\\", "*");

							if (!RemoveDirectory(folderName + "\\"))
							{
								LOGERROR( g_Log, "%s: %s", folderName.Chars(), _strerror(NULL));
							}
						}
						else
						{
							LOGWARN( g_Log, "Old texture cache folder '%s' exists. Is the scene converter using an older texture cache version number from the PVS tool (which uses version %d)? If so then we probably want to re-publish the scene converter, or each export will have to -convert every texture twice.", folderName.Chars(), m_iCacheVersion);
						}
					}
				}
			}
		} while (FindNextFile(hFind, &wfd));
		FindClose(hFind);
	}
}

/// create a new cache entry, deletes existing one if present
CacheEntry *Cache::CreateEntry(CacheKey key)
{
	return new CacheEntry(this, true, key);
}

/// calculates a cache filename given a cache key
Util::C_String Cache::GetFilenameFromKey(CacheKey key)
{
	U32 hash = Ut::HashRaw(key->pData, key->iSize);
	Util::C_String name;
	name.Printf("%s\\%x.bin", GetCacheDirectoryPath().Chars(), hash);
	return name;
}

/// write a new cache entry into the cache - remove any existing entry
void Cache::Write(CacheEntry *pEntry)
{
	Util::C_String sName = GetFilenameFromKey(pEntry->m_Key);
	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open(sName, Sys::E_FileAccess_Write);
	if (!pFile)
	{
		LOGERROR( g_Log, "TextureCache: Failed to write cache file '%s'.\n", sName.Chars());
	}

	Sys::BinarySerialiser<Ut::NoEndianSwap> ar(*pFile);
	ar << m_iCacheVersion;
	ar << *pEntry;

	delete pFile;
}

/// lookup cache entry, returns NULL if it doesn't exist
CacheEntry *Cache::GetEntry(CacheKey key)
{
	// open file
	Util::C_String sFilename = GetFilenameFromKey(key);
	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open(sFilename);
	if (!pFile)
		return NULL;

	// read version number
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);
	int iCacheVersion;
	ar >> iCacheVersion;
	if (iCacheVersion != m_iCacheVersion)
	{
		// version number doesn't match; delete file and retur NULL
		Sys::GetDefaultFileSystem()->DeleteFile(sFilename);
		delete pFile;
		return NULL;
	}

	// read cache entry
	CacheEntry *pEntry = new CacheEntry(this, false, CacheKey());
	ar >> *pEntry;
	delete pFile;
	// verify that the key matches
	if (key.RawPtr()->Compare(*pEntry->m_Key.RawPtr()))
	{
		delete pEntry;
		return NULL;
	}
	return pEntry;
}

}
