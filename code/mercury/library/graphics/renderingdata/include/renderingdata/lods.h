#ifndef RD_LODS_H
#define RD_LODS_H
#ifdef _MSC_VER
#pragma once
#endif

#ifdef CFG_CONVERTER
#include <texture_export/texture.h>
#include "renderingdata.h"
#include "scene.h"
#endif

namespace RenderingData {
namespace LodCalculator
{
	// Size = pixels.
	// Scale = size in world space of one unit in texture space.
	S32 LodForTextureSize(float sizeU, float sizeV, float scaleU, float scaleV);

	// Get the maximum draw distance for a lod.
	float LodDrawDistance(S32 lod, float screenSize, float fov);

	// Number of lods that we use (constant).
	// Lod 0 = lowest, containing vertex and index buffers and lower mip levels of textures.
	// Other lods contain higher mip levels.
	S32 GetNumLods();

#ifdef CFG_CONVERTER
	bool IsPackedMip(const ActTools::Texture& texture, S32 i, PlatformType platformType);

	// Get indices (first, last) of the mip levels included in a lod.
	Btl::Pair<S32, S32> GetMipLevelsForLod(S32 lod, const ActTools::Texture& texture, float scaleU, float scaleV);

	// Get the size in bytes of the mip levels included in a lod.
	S32 GetSize(S32 lod, const ActTools::Texture& texture, float scaleU, float scaleV, PlatformType platformType);
#endif

}}

#endif
