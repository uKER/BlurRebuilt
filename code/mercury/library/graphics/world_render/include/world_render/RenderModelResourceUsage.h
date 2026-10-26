#ifndef WORLD_RENDER_RENDER_RESOURCE_USAGE_INFO_H
#define WORLD_RENDER_RENDER_RESOURCE_USAGE_INFO_H


#include <btl/containers/set.h>
#include <btl/containers/vector.h>
#include <core/containers/String.h>
#include <display.h>


namespace RenderingData
{
	namespace Target
	{
		struct TextureInfo;
	}
}


namespace WorldRender
{

struct RenderModelTextureInfo
{
	enum E_Flag
	{
		E_Flag_Shared = 1 << 0, 
	};

	U32					flags;
	U32					uSizeInBytes;
	U32					uWidth;
	U32					uHeight;
	U32					uLevels;
	Display::E_Format	eFormat;
	Util::C_String		textureName;
	const Display::C_Texture2D*	pDisplayTexture;

	inline bool operator < (const RenderModelTextureInfo& rhs) const;
};

inline bool RenderModelTextureInfo::operator < (const RenderModelTextureInfo& rhs) const 
{
	if (uSizeInBytes < rhs.uSizeInBytes)
		return true;

	return textureName < rhs.textureName;
}

typedef Btl::Set<RenderModelTextureInfo> RenderModelTextureInfoList;

struct RenderModelResourceUsage
{
	RenderModelTextureInfoList	textureInfoList;

	void AccumulateRenderingDataTextures(const Btl::Vector<RenderingData::Target::TextureInfo>& renderingData);
};

} // namespace WorldRender


#endif // WORLD_RENDER_RENDER_RESOURCE_USAGE_INFO_H
