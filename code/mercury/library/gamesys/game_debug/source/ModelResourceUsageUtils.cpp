#include "GameDebugPch.h"
#include <game_debug/ModelResourceUsageUtils.h>


namespace ModelResourceUsage
{

const char* const g_TextureTypeNames [] = 
{
	"Diffuse", 
	"Normal",
	"IPR",
	"Alpha",
	"Lightmap",
	"Misc",
};

const char* GetTextureTypeName(const E_TextureType textureType)
{
	BIZ_ASSERT((textureType >= 0) && (textureType < E_TextureType_Count));
	StaticAssert(lengthof(g_TextureTypeNames) == E_TextureType_Count, array_size_must_match_enum_count);
	return g_TextureTypeNames[textureType];
}

void AccumulateTotals(C_TextureTotals& ioTotals, const Util::C_String& name, const U32 sizeInBytes, const Display::E_Format format)
{
	E_TextureType primaryType = E_TextureType_Misc;
	bool hasSecondaryAlpha = false;

	if (name.ISearch("lightmap_") == 0)
	{
		primaryType = E_TextureType_Lightmap;
	}
	else
	{
		if (name.ISearch("_DIFF") == (name.Length() - 5))
		{
			primaryType = E_TextureType_Diffuse;
		}
		else if (name.ISearch("_NORM") == (name.Length() - 5))
		{
			primaryType = E_TextureType_Normal;
		}
		else if (name.ISearch("_IPR0") == (name.Length() - 5))
		{
			primaryType = E_TextureType_IPR;
		}

		if (name.ISearch("_ALPHA") > 0)
		{
			if (primaryType == E_TextureType_Misc)
			{
				primaryType = E_TextureType_Alpha;
			}
			else
			{
				hasSecondaryAlpha = true;
			}
		}
	}

	if (hasSecondaryAlpha)
	{
		U32 secondarySize = 0;

		switch (format)
		{
		case Display::E_Format_DXT3:
		case Display::E_Format_DXT5:
#if !PLATFORM_PS3
		case Display::E_Format_A8L8:
#endif
		case Display::E_Format_Unknown:
			secondarySize = sizeInBytes / 2;
			break;

		case Display::E_Format_A8R8G8B8:
#if !PLATFORM_PS3
		case Display::E_Format_X8R8G8B8:
		case Display::E_Format_G16R16:
		case Display::E_Format_G16R16F:
		case Display::E_Format_A16B16G16R16:
#endif
		case Display::E_Format_A16B16G16R16F:
		case Display::E_Format_A32B32G32R32F:
			secondarySize = sizeInBytes / 4;
			break;

#if !PLATFORM_PS3
		case Display::E_Format_A2R10G10B10:
#endif
		case Display::E_Format_A2B10G10R10F:
			secondarySize = sizeInBytes / 16;
			break;
		}

		ioTotals[primaryType] += sizeInBytes - secondarySize;
		ioTotals[E_TextureType_Alpha] += secondarySize;
	}
	else
	{
		ioTotals[primaryType] += sizeInBytes;
	}
}

} // namespace ModelResourceUsage
