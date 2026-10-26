#include "stdafx.h"
#include "renderingdata/reloadabletextures.h"

#include "renderingdata/target.h"
#include "renderingdata/streaming.h"
#include "system/Serialise.h"
#include "system/misc/StringUtil.h"

//#pragma optimize ("", off)

namespace RenderingData {
namespace Target
{

static Debug::Logger &g_Log = Debug::GetLogger("graphics.renderingdata");
ReloadableTextureManager ReloadableTextureManager::s_instance;

ReloadableTextureManager::ReloadableTextureManager() : m_reloadCount(0) 
{
}

U32 Hash(const ReloadableTextureManager::TextureKey& textureKey)
{
	U32 hash = Btl::Hash(textureKey.textureName.Chars());
	return Btl::Hash(textureKey.hash, hash);
}

bool operator == (const ReloadableTextureManager::TextureKey& a, const ReloadableTextureManager::TextureKey& b)
{
	return (a.hash == b.hash) 
		&& (a.textureName == b.textureName);
}

void ReloadableTextureManager::LoadTexture(ReloadableTextureManager::TextureData& textureData, const Sys::C_FileInfo& fileInfo)
{
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(fileInfo.m_FileName, Sys::E_FileAccess_Read);

	if (pFile) // see if the file can be opened
	{
		delete pFile;

		textureData.fileTime = fileInfo.m_FileTime;
		const char* fileName = fileInfo.m_FileName.Chars();

		if (textureData.pTexture != 0)
			m_oldTextures.PushBack(textureData.pTexture);

		Allocate(textureData.pTexture);

		if (!textureData.pTexture->Create(fileName))
			Free(textureData.pTexture);
	}
	else
	{
		textureData.fileTime = 0;
	}
}

void ReloadableTextureManager::RefreshTextures(const Btl::String& dir, const bool refreshTextures)
{
	m_oldTextures.Clear();

	if (!refreshTextures)
	{
		m_reloadCount = 0;

		for (TextureMap::Iterator i = m_currentTextures.Begin(); i != m_currentTextures.End(); ++i)
			m_oldTextures.PushBack(i->GetValue().pTexture);

		m_currentTextures.Clear();

		return;
	}

	RefreshTexturesRecurseDirectories(dir, refreshTextures);
}

void ReloadableTextureManager::RefreshTexturesRecurseDirectories(const Btl::String& dir, const bool refreshTextures)
{
	PERF_EVENT("ReloadableTextureManager::RefreshTextures");

	Sys::C_DirectoryInfo dirInfo;

	bool change = false;
	const bool success = Display::gFileSystem->DirectoryInfo(dir.Chars(), &dirInfo);

	if(success)
	{
		for (U32 fileIndex = 0; fileIndex < dirInfo.NumFiles(); fileIndex++)
		{
			Sys::C_FileInfo& fileInfo = dirInfo.FileInfo(fileIndex);

			if(fileInfo.m_FileType == Sys::E_FileType_Directory)
			{
				RefreshTexturesRecurseDirectories(fileInfo.m_FileName.Chars(), refreshTextures);
			}
			
			if(fileInfo.m_FileType == Sys::E_FileType_File)
			{
				TextureKey key;
				key.textureName = (Sys::RemoveFileExtension(Sys::ExtractFilename(fileInfo.m_FileName))).Chars();
				key.hash = 0;
				TextureMap::InsertionReturnType r = m_currentTextures.Insert(key);

				if (r.inserted) // new texture
				{
					LoadTexture(r.iterator->GetValue(), fileInfo);
					change = true;
				}
				else // texture exists
				{
					if (r.iterator->GetValue().fileTime != fileInfo.m_FileTime)
					{
						LoadTexture(r.iterator->GetValue(), fileInfo);
						change = true;
					}
				}
			}
		}
	}

	if (change)
		m_reloadCount++;
}

U32 ReloadableTextureManager::GetReloadCount() const
{
	return m_reloadCount;
}

const Texture* ReloadableTextureManager::GetTexturePtr(const char* name)
{
	if (m_currentTextures.Empty())
		return 0;

	TextureKey key;
	key.textureName = name;
	key.hash = 0;
	TextureMap::Iterator i = m_currentTextures.Find(key);

	if (i == m_currentTextures.End())
		return 0;
	else
		return Pointer(i->GetValue().pTexture);
}

void ReloadableTextureManager::CopyFlags(Texture* pDst, const Texture* pSrc)
{
}

}}
