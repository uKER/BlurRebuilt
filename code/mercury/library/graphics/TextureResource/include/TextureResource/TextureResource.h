#ifndef GAME_RESOURCE_TEXTURE_RESOURCE_H
#define GAME_RESOURCE_TEXTURE_RESOURCE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <btl/containers/vector.h>
#include <resource/Resource.h>
#include <display/texture/Texture.h>


namespace TextureResource
{

class C_TextureResource;

typedef Resource::C_Handle<C_TextureResource> C_TextureHandle;

//! C_TextureResource 
class C_TextureResource : public Resource::C_Resource<C_TextureResource>
{
public:
	C_TextureResource();

	~C_TextureResource();

	// Inherited from C_Resource
	// |------------------------
	bool LoadResourceData();
	bool FreeResourceData();
	S32 GetEvictDelay();
	static Util::C_String StaticGetResourceTypeName();
	// |------------------------

	const Display::C_Texture* GetDisplayTexture() const
	{
		return m_pDisplayTexture;
	}
	
	// TRUE: SRGB texture, it is degamma corrected when bind. lighting, 3D. Need postprocess gamma correction.
	// FALSE: Standard, with not post process gamma correction
	bool GetSRGBFlag()
	{
		return m_SRGBFlag;
	}

private:
	bool LoadTextureData( Sys::I_File* pFile );

private:
	Display::C_Texture* m_pDisplayTexture;
	bool m_SRGBFlag;
};

} // namespace GameResource

#endif // GAME_RESOURCE_MODEL_H
