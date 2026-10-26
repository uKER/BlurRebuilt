#ifndef __TEXTURE_EXPORT_TEXTURE_MANAGER_H__
#define __TEXTURE_EXPORT_TEXTURE_MANAGER_H__

#include <core/containers.h>
#include <system/time/time.h>
#include <system/serialise/i_binarystream.h>
#include <misc_shared/cache/cachekey.h>
#include <misc_shared/cache/cacheentry.h>
#include <misc_shared/archive/archive.h>
#include <misc_shared/archive/btl.h>
#include <system/serialise.h>
#include "texture_export/Texture.h"

// forward declarations
namespace Sys
{
class I_BinaryStream;
}
namespace ActTools
{
class Texture;
}

//
namespace TextureExport
{

class TextureInfo;

struct InputFile
{
	InputFile() : bRequired(false) {}
	InputFile(const Util::C_String& name, U8 required = true) : sName(name), bRequired(required) {}

	Util::C_String sName;		// the name of the input texture
	U8 bRequired;				// if a texture doesn't exist and this is set to true, a warning will be printed out
};

template<typename Archive>
void Serialise(Archive &ar, InputFile &t)
{
	ar << t.sName << t.bRequired;
}

template<typename Archive>
void Deserialise(Archive &ar, InputFile &t)
{
	ar >> t.sName >> t.bRequired;
}

/// base class for a texture info structure
class TextureInfoBase
{
public:
	/// destructor
	virtual ~TextureInfoBase() {}

	/// write contents of this class to a binary stream
	virtual void Write(Sys::I_BinaryStream *pStream) = 0;
	/// read contents of this class from a binary stream
	virtual void Read(Sys::I_BinaryStream *pStream) = 0;
	/// check that all the dependencies (input files) are older than the given timestamp
	virtual bool DependenciesOlderThan(Time::C_Time timeStamp);
	/// generate user-defined dependency data which has to match for the cache entry to be valid
	virtual U32 UserDependencyData();

	/// input filenames are to be added to this vector so they can be dependency checked
	Ut::Vector<InputFile> m_InputFilenames;
};

template<typename Archive>
void Serialise(Archive &ar, TextureInfoBase &t)
{
	t.Write(&ar.m_stream);
}
template<typename Archive>
void Deserialise(Archive &ar, TextureInfoBase &t)
{
	t.Read(&ar.m_stream);
}

/// manages the exporting of texture data, checking for duplicate textures etc.
class TextureManager
{
public:
	/// constructor
	TextureManager(const char *pszCacheFolderName, int iVersion, bool bDeleteOldTextureCache);

	/// destructor
	~TextureManager();

	/// export a texture, returns the handle of the exported texture, checks for duplicates
	int					GetTextureIndex(TextureInfoBase *pInfo);

	/// retrieves the texture info for a previously acquired handle
	void				GetTextureInfo(int idx, TextureInfoBase &texinfo);

	/// retrieves the name of the texture to be used instead of missing textures
	Util::C_String		GetMissingTextureFilename();

	/// set the missing texture name
	void				SetMissingTextureFilename(const char *psz);

	// retrieves the exported texture data for a previously acquired handle
	DiskCache::CacheEntry *GetTextureData(int idx, TextureInfoBase &texinfo);

	// as above, but templated to serialise directly into the target structure
	template<typename T>
	void GetTextureData(int idx, TextureInfoBase &texInfo, const char *pszDataType, T &data)
	{
		DiskCache::CacheEntry *pEntry = GetTextureData(idx, texInfo);
		Sys::I_BinaryStream *pStream = pEntry->GetData(pszDataType);
		Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pStream);
		ar >> data;
		delete pEntry;
		delete pStream;
	}

	/// as above but specific to ActTools::Texture data. uses a local cache to speed up accesses
	ActTools::Texture *GetTexture(int iIdx, TextureExport::TextureInfo &info);

	void SetShared(bool b);
	bool GetShared() const;

private:
	DiskCache::Cache					*m_pDiskCache;
	Ut::UtMap<DiskCache::CacheKey, int> *m_pMap;
	Ut::Vector<DiskCache::CacheKey>		*m_pList;
	bool								m_shared;
	Util::C_String						m_sMissingTextureFilename;

	struct CacheItem
	{
		int iIdx;
		U32 iLastUsed;
		ActTools::Texture tex;
	};

	Btl::Vector<CacheItem, Btl::VectorParameters_Array<16> > m_Cache;
	U32 m_iCurrentUsed;
};

bool GenerateTextureData(TextureInfoBase *pTexInfo, DiskCache::CacheEntry *pEntry);

};

#endif
