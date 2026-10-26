#ifndef CROWD_TEXTURE_CACHE_H
#define CROWD_TEXTURE_CACHE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <crowd_system/CrowdDefines.h>

#include <textureResource/TextureResource.h>
#include <btl/containers/OpenHashMap.h>

#include <resource_utils/ResourceUtilsDeclare.h>

namespace AnimatedCrowd
{
	class C_XtCrowdTextureList;
};

namespace AnimatedCrowd
{

//! C_CrowdTexture
class C_CrowdTexture : public Resource::C_Listener
{
public:
	C_CrowdTexture();
	~C_CrowdTexture();

	void			Create(const Util::C_String& filename);

	virtual void	ResourceNotify(Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData);

	const bool		IsValid() const;

	const TextureResource::C_TextureResource*
					GetTextureResource() const;

private:
	TextureResource::C_TextureHandle	m_TextureResourceHandle;

	U32									m_Valid : 1;
};

//! C_CrowdTextureCache

class C_CrowdTextureCache
{
public:
	C_CrowdTextureCache();
	~C_CrowdTextureCache();

	void Destroy();

	void BeginBuild();
	void AddTextureList(const C_XtCrowdTextureList* pTextureList);
	void EndBuild();
	
	const S32					GetTextureIndex(const char* pTextureName) const;
	const U32					GetNumTextures() const;
	const Display::C_Texture2D*	GetTexture(const U32 index) const;

private:
	Btl::OpenHashMap<const char*, U32>	m_TextureMap;
	Btl::Vector<C_CrowdTexture>			m_Textures;

	Btl::Vector<const char*>			m_TextureCreationBuffer;
};

}; // namespace AnimatedCrowd

#endif // CROWD_TEXTURE_CACHE_H
