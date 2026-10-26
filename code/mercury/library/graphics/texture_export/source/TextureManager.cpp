#include "stdafx.h"

#include "texture_export/TextureManager.h"

#include <misc_shared/cache/cache.h>
#include <system/serialise.h>
#include "texture_export/targets/xetex.h"
#include "texture_export/textureprocessing.h"

namespace TextureExport
{

/// check that all the dependencies (input files) are older than the given timestamp
bool TextureInfoBase::DependenciesOlderThan(Time::C_Time timeStamp)
{
	for (int i=0; i<m_InputFilenames.Size(); i++)
	{
		// if the file exists and it's newer than the given timestamp, return false
		Sys::C_FileInfo info;
		if (Sys::C_FileSystem::Instance().FileInfo(m_InputFilenames[i].sName, &info) && info.m_FileTime > timeStamp)
			return false;
	}
	return true;
}

/// generate user-defined dependency data which has to match for the cache entry to be valid
/// in this case we check whether each input file exists and store this in a bitfield
U32 TextureInfoBase::UserDependencyData()
{
	U32 iMask = 1;
	U32 iResult = 0;
	for (int i=0; i<m_InputFilenames.Size(); i++)
	{
		Util::C_String name = m_InputFilenames[i].sName;
		if (name == "")
			continue;
		if (Sys::C_FileSystem::Instance().FileExists(name))
		{
			iResult |= iMask;
		}
		else
		{
			if (m_InputFilenames[i].bRequired)
			{
				// if a required texture is missing, print out a warning
				// only warn once for each texture
				static Btl::Set<Util::C_String> filenameSet;
				Btl::Set<Util::C_String>::Iterator iter = filenameSet.Find(name);
				if (iter == filenameSet.End())
				{
					filenameSet.Insert(name);
					LOGWARN( g_Log, "Required texture '%s' not found.\n", name.Chars());
				}
			}
		}

		iMask = iMask<<1;
	}

	return iResult;
}

void TextureInfoBase::Read(Sys::I_BinaryStream *pStream)
{
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pStream);
	ar >> m_InputFilenames;
}

void TextureInfoBase::Write(Sys::I_BinaryStream *pStream)
{
	Sys::BinarySerialiser<Ut::NoEndianSwap> ar(*pStream);
	ar << m_InputFilenames;
}

/// constructor
TextureManager::TextureManager(const char *pszCacheFolderName, int iVersion, bool bDeleteOldTextureCache)
{
	m_pDiskCache = new DiskCache::Cache(pszCacheFolderName, iVersion, "texturecache", bDeleteOldTextureCache);
	m_pMap = new Ut::UtMap<DiskCache::CacheKey, int>();
	m_pList = new Ut::Vector<DiskCache::CacheKey>();
	m_shared = true;
	for (int i=0; i<m_Cache.Size(); i++)
	{
		m_Cache[i].iIdx = -1;
		m_Cache[i].iLastUsed = 0;
	}
	m_iCurrentUsed = 0;
}

/// destructor
TextureManager::~TextureManager()
{
	delete m_pDiskCache;
	delete m_pMap;
	delete m_pList;
}

void TextureManager::SetShared(bool b)
{
	m_shared = b;
}

bool TextureManager::GetShared() const
{
	return m_shared;
}

/// retrieves the name of the texture to be used instead of missing textures
Util::C_String TextureManager::GetMissingTextureFilename()
{
	return m_sMissingTextureFilename;
}

/// set the missing texture name
void TextureManager::SetMissingTextureFilename(const char *psz)
{
	m_sMissingTextureFilename = psz;
}

/// export a texture, returns the handle of the exported texture, checks for duplicates
int TextureManager::GetTextureIndex(TextureInfoBase *pInfo)
{
	DiskCache::CacheKey key = DiskCache::GenerateCacheKey(*pInfo);

	Ut::UtMap<DiskCache::CacheKey, int>::Iterator iter = m_pMap->Find(key);
	if (iter != m_pMap->End())
		return iter->GetValue();

	// TODO: potentially start converting the texture by calling 'GenerateTextureData' on another thread

	m_pList->PushBack(key);
	return (*m_pMap)[key] = m_pList->Size()-1;
}

/// retrieves the texture info for a previously acquired handle
void TextureManager::GetTextureInfo(int idx, TextureInfoBase &texinfo)
{
	DiskCache::CacheKeyObj *pKeyObj = m_pList->At(idx).RawPtr();
	Sys::C_MemoryStream stream(pKeyObj->pData, pKeyObj->iSize);
	texinfo.Read(&stream);
}

// retrieves the exported texture data for a previously acquired handle
DiskCache::CacheEntry *TextureManager::GetTextureData(int idx, TextureInfoBase &texinfo)
{
	GetTextureInfo(idx, texinfo);

	while (true)
	{
		// see if this entry already exists in the cache
		DiskCache::CacheKey key = m_pList->At(idx);
		DiskCache::CacheEntry *pEntry = m_pDiskCache->GetEntry(key);
		U32 userData = texinfo.UserDependencyData();
		if (pEntry)
		{
			// check that the same files exist as when the entry was created
			if (userData == pEntry->GetUserData() &&
			// found it - check that the input files are all older than the cache entry
			     texinfo.DependenciesOlderThan(pEntry->GetTime()))
				return pEntry;
			delete pEntry;
		}

		// cache entry does not exist or is invalid - run exporter to generate data
		pEntry = m_pDiskCache->CreateEntry(key);
		pEntry->SetUserData(userData);
		bool bResult = GenerateTextureData(&texinfo, pEntry);
		delete pEntry;
		if (!bResult)
			return NULL;
	}
}

/// retrieve a texture from the cache
ActTools::Texture *TextureManager::GetTexture(int iIdx, TextureExport::TextureInfo &info)
{
	m_iCurrentUsed++;
	if (m_iCurrentUsed == 0)
		for (int i=0; i<m_Cache.Size(); i++)
			m_Cache[i].iLastUsed = 0;

	int iOldestIdx = 0;
	U32 iOldestTime = 0xffffffff;
	for (int i=0; i<m_Cache.Size(); i++)
	{
		CacheItem *p = &m_Cache[i];
		if (p->iIdx == iIdx)
		{
			// found in cache
			p->iLastUsed = m_iCurrentUsed;
			return &p->tex;
		}

		if (p->iLastUsed < iOldestTime)
		{
			iOldestTime = p->iLastUsed;
			iOldestIdx = i;
		}
	}

	// not found - make new
	CacheItem *p = &m_Cache[iOldestIdx];
	p->iLastUsed = m_iCurrentUsed;
	p->iIdx = iIdx;
	GetTextureData(iIdx, info, "TEX", p->tex);
	return &p->tex;
}

}
