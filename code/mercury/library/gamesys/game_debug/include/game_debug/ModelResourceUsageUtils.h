#ifndef SHAREDACTORS_MODELRESOURCEUSAGEUTILS_H
#define SHAREDACTORS_MODELRESOURCEUSAGEUTILS_H

#ifdef _MSC_VER
#pragma once
#endif

#include <display/device/DeviceTypesEnums.h>


namespace ModelResourceUsage
{

enum E_TextureType
{
	E_TextureType_Diffuse, 
	E_TextureType_Normal, 
	E_TextureType_IPR, 
	E_TextureType_Alpha,
	E_TextureType_Lightmap,
	E_TextureType_Misc,
	E_TextureType_Count
};

extern const char* GetTextureTypeName(const E_TextureType textureType);

typedef U32 C_TextureTotals [E_TextureType_Count];

extern void AccumulateTotals(C_TextureTotals& ioTotals, const Util::C_String& name, const U32 sizeInBytes, const Display::E_Format format);

} // namespace ModelResourceUsage


#endif // SHAREDACTORS_MODELRESOURCEUSAGEUTILS_H
