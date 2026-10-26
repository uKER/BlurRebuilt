#include "stdafx.h"
#include "renderingdata/lods.h"

#ifdef CFG_CONVERTER
#include <texture_export/texture.h>
#include <system/serialise/C_MemoryStream.h>
#include "texture_export/targets/xetex.h"
#include "texture_export/targets/ps3tex.h"
#include "texture_export/targets/pctex.h"
#endif

//#pragma optimize("", off)

namespace RenderingData {
	
#ifdef CFG_CONVERTER
namespace Build {extern void ChangeNumMipLevels(ActTools::Texture& texture, S32 lowest, S32 highest);}
#endif

namespace LodCalculator
{

S32 LodForTextureSize(float sizeU, float sizeV, float scaleU, float scaleV)
{
	S32 lod = (S32) (log(Maths::Max(sizeU / scaleU, sizeV / scaleV)) / log(2.f));
	return lod;
}

float LodDrawDistance(S32 lod, float screenSize, float fov)
{
	assert(lod > -1 && lod < GetNumLods());
	
	if (lod == 0)
		return FLT_MAX;

	float d = screenSize / (fov * (float)(1 << (lod)));
	return d;
}

S32 GetNumLods()
{
	return 10;
}

#ifdef CFG_CONVERTER

Btl::Pair<S32, S32> GetMipLevelsForLod(S32 lod, const ActTools::Texture& texture, float scaleU, float scaleV)
{
	// Get mip level for this lod.
	S32 lodForFullSize = LodCalculator::LodForTextureSize((float)texture.GetWidth(), (float)texture.GetHeight(), scaleU, scaleV);
	S32 levelForLod = lodForFullSize - lod;

	// Find the first level to write in lod 0
	S32 firstLevel = texture.GetNumMipMaps() - 1;

	// Can only separate mip levels when they are bigger than 16 on 360,
	// plus it is better not to have lots of small texture reads.
	for (int i = firstLevel; i >= 0; i--) 
		if (texture.GetMipmap(0, i)->GetWidth() <= 16
			|| texture.GetMipmap(0, i)->GetHeight() <= 16)
			firstLevel = i;

	S32 startLevel = -1;
	S32 endLevel = -1;

	if (lod == 0) // write levels from first to max
	{
		startLevel = Maths::Clamped(levelForLod, 0, firstLevel);
		endLevel = texture.GetNumMipMaps() - 1;
	}
	else if (levelForLod < firstLevel) // write one level
	{
		startLevel = endLevel = levelForLod;
	}

	return Btl::MakePair(startLevel, endLevel);
}

S32 GetSize(S32 lod, const ActTools::Texture& texture, float scaleU, float scaleV, PlatformType platformType)
{
	Btl::Pair<S32, S32> startEnd = GetMipLevelsForLod(lod, texture, scaleU, scaleV);

	if (startEnd.first < 0)
		return 0;

	ActTools::Texture tempTexture(texture);
	Build::ChangeNumMipLevels(tempTexture, startEnd.first, startEnd.second);
	Sys::C_MemoryStream memoryStream(1024 * 1024);

	switch (platformType)
	{
		case PLATFORM_TYPE_360:
		{
			NglXeTex texWriter;
			texWriter.SerialOutTexture(&memoryStream, &tempTexture, NglBaseTex::JUST_DATA);
			break;
		}
		case PLATFORM_TYPE_PS3:
		{
			NglPs3Tex texWriter;
			texWriter.SerialOutTexture(&memoryStream, &tempTexture, NglBaseTex::JUST_DATA);
			break;
		}
		case PLATFORM_TYPE_PC:
		{
			NglPcTex texWriter;
			texWriter.SerialOutTexture(&memoryStream, &tempTexture, NglBaseTex::JUST_DATA);
			break;
		}
		default:
			assert(0); // unknown platform
	}

	return memoryStream.m_pCurBuffer - memoryStream.GetBufferPtr();
}

#endif

}}
