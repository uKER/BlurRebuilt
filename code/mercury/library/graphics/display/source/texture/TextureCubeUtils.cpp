#include "stdafx.h"
#include "Display/Texture/TextureCubeUtils.h"
#include "Display/Device/Device.h"
#include "Display/Utils/File.h"
#include "Display/Texture/DDS/DDSTextureUtils.h"
#include <System/Misc/StringUtil.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Display
{


bool UploadTextureCube(C_TextureCube* pTexture, C_TextureParams* pTextureParams)
{

	BIZ_ASSERT(pTexture);

	if(!pTexture->IsCreated())
		return false;

	BIZ_ASSERT(pTextureParams);
	BIZ_ASSERT(pTextureParams->m_pMipData);
	BIZ_ASSERT(pTextureParams->m_MipSize > 0);
	BIZ_ASSERT(pTextureParams->m_MipOffset < pTextureParams->m_MipSize);

	U8* pData = &pTextureParams->m_pMipData[pTextureParams->m_MipOffset];

	const Display2::E_TextureFormat format = GetTextureFormatFromFormat(pTextureParams->m_Format);

	for(U32 face=0; face<E_CubeTextureFace_Count; face++)
	{
		U32 width = pTextureParams->m_Width;

		for(U32 level=pTextureParams->m_MipHead; level<=pTextureParams->m_MipTail; level++)
		{
			BIZ_ASSERT(pData <= pTextureParams->m_pMipData+pTextureParams->m_MipSize);

			C_LockedRect lockedRect = pTexture->LockRectCube((Display2::E_CubeMapFace)face, level);

			if(!lockedRect.GetBits())
			{
				LOGERROR(g_Log, "Failed to lock cube texture face %i level %i\n", face, level);
				return false;
			}

			U32 pitch = Display2::GetTexturePitchFromWidth(width, pTextureParams->m_Flags, format);
			U32 size = Display2::GetTexture2DSize(width, width, pTextureParams->m_Flags, format);

			BIZ_ASSERT(pData+size <= pTextureParams->m_pMipData+pTextureParams->m_MipSize);
			
			memcpy(lockedRect.GetBits(), pData, size);

			pData += size;

			pTexture->UnlockRectCube((Display2::E_CubeMapFace)face, level);
			
			if(width > 1)
				width >>= 1;
		}
	}

	BIZ_ASSERT(pData == pTextureParams->m_pMipData+pTextureParams->m_MipSize);

	return true;
}

U32 GetTextureCubeSize(const U32 width, const U32 flags, const E_Format format)
{
	return Display2::GetTextureCubeSize(width, flags, GetTextureFormatFromFormat(format));
}

U32 GetTextureCubeSize(U32 width, const U32 levels, const U32 flags, const E_Format format)
{
	return Display2::GetTextureCubeSize(width, levels, flags, GetTextureFormatFromFormat(format));
}


bool CreateTextureCubeFromFile(Util::C_String fileName, C_TextureCube* pTexture)
{
	if(fileName == "")
	{
		LOGERROR(g_Log, "No file name specified when creating cube texture from file\n");
		return false;
	}

	fileName.ConvertToLower();

#if 0
	return D3DXCreateTextureCubeFromFile(fileName, pTexture);
#endif

	C_TextureParams params;
	
	if(fileName.Search(".dds") != -1)
	{
		if(!DDS::LoadTextureFromFile(fileName, &params))
			return false;
	}
	else
	{
		LOGERROR(g_Log, "Unsupported cube texture file extension '%s'\n", fileName.Chars());
		return false;
	}

	params.m_Name = fileName;

	#if defined(PLATFORM_PS3) && !defined(CFG_GOLD)
	if(params.m_Width & (params.m_Width-1))
	{
		LOGERROR(g_Log, "Cube texture'%s' is not power of 2 (%i)\n", fileName.Chars(), params.m_Width);
	}
	#endif

	params.m_Flags |= E_Format_Linear;
	params.m_Flags |= E_Format_Raw;

	if(!Device()->CreateTextureCube(params.m_Width, params.m_Levels, params.m_Flags, params.m_Format, pTexture))
	{
		SAFE_DELETE_ARY(params.m_pMipData);
		return false;
	}

	params.m_MipHead = 0;
	params.m_MipTail = params.m_Levels-1;

	if(!UploadTextureCube(pTexture, &params))
	{
		Device()->DestroyTextureCube(pTexture);
		SAFE_DELETE_ARY(params.m_pMipData);
		LOGERROR(g_Log, "Failed to upload the cube texture '%s'\n", fileName.Chars());
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(pTexture, fileName);
	#endif
	
	SAFE_DELETE_ARY(params.m_pMipData);
	
	return true;
}

}; // namespace Display
