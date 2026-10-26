#ifndef RD_RELOADABLE_TEXTURES_H
#define RD_RELOADABLE_TEXTURES_H
#ifdef _MSC_VER
#pragma once
#endif

#include "btl/containers/chainedhashmap.h"
#include "btl/utility/autopointer.h"
#include "system/time/time.h"

namespace Sys {struct C_FileInfo;}

namespace RenderingData {
namespace Target
{

class Texture;

class ReloadableTextureManager
{
public:
	static ReloadableTextureManager& Instance() {return s_instance;}

	void RefreshTextures(const Btl::String& dir, const bool refreshTextures);
	
	U32 GetReloadCount() const;
	const Texture* GetTexturePtr(const char* name);

	void CopyFlags(Texture* pDst, const Texture* pSrc);

private:
	ReloadableTextureManager();
	void RefreshTexturesRecurseDirectories(const Btl::String& dir, const bool refreshTextures);

	static ReloadableTextureManager s_instance;

	struct TextureKey
	{
		Btl::String textureName;
		U32 hash;
	};

	struct TextureData
	{
		Btl::AutoPointer<Texture> pTexture;
		Time::C_Time fileTime;
	};

	friend U32 Hash(const TextureKey& textureKey);
	friend bool operator == (const TextureKey& a, const TextureKey& b);
	void LoadTexture(TextureData& textureData, const Sys::C_FileInfo& fileInfo);

	typedef Btl::ChainedHashMap<TextureKey, TextureData> TextureMap;
	TextureMap m_currentTextures;
	Btl::Vector<Btl::AutoPointer<Texture> > m_oldTextures;
	U32 m_reloadCount;
};

}}

#endif
