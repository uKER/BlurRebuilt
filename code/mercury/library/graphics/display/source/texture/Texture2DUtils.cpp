#include "stdafx.h"
#include "Display/Texture/Texture2DUtils.h"
#include "Display/Device/Device.h"
#include "Display/Utils/File.h"
#include "Display/Texture/DDS/DDSTextureUtils.h"
#include "Display/Texture/TGA/TGATextureUtils.h"
#include "Display/Texture/BMP/BMPTextureUtils.h"
#include <System/Misc/StringUtil.h>

#if !PLATFORM_PC
#include <Core/Utility/MemCpy.h>
#endif

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Display
{


#if PLATFORM_PS3

bool UploadTexture2D_Tiled(C_Texture2D* pTexture, C_TextureParams* pTextureParams)
{
	BIZ_ASSERT(pTexture);
	
	if(!pTexture->IsCreated())
		return false;

	assert(pTextureParams);

	const U32 size							= pTexture->GetSize();
	const U32 flags							= pTexture->GetFlags();
	U8* pAddress							= (U8*)pTexture->GetAddress();

	const U32 width							= pTexture->GetWidth();
	const U32 height						= pTexture->GetHeight();
	const U32 levels						= pTexture->GetLevels();
	const Display2::E_TextureFormat format	= pTexture->GetFormat();

	const U32 offset = Display2::GetTexture2DLevelOffset(pTextureParams->m_MipHead, width, height, levels, flags, format);

	const U32 srcSize = pTextureParams->m_MipSize-pTextureParams->m_MipOffset;
	assert(srcSize > 0 && srcSize <= size);

	U8* pSrcData = &pTextureParams->m_pMipData[pTextureParams->m_MipOffset];
	assert(pSrcData);

	U8* pDstData = pAddress+offset;
	assert(pDstData);

	switch(pTextureParams->m_TransferMode)
	{
	//case CELL_GCM_TRANSFER_MAIN_TO_LOCAL:
	case CELL_GCM_TRANSFER_LOCAL_TO_LOCAL:
		{
			sys_ppu_thread_t threadID;
			int ret = sys_ppu_thread_get_id(&threadID);
			assert(ret == CELL_OK);
			if(threadID == g_MainThreadID)
			{
				U32 srcOffset = 0, dstOffset = 0;
				cellGcmAddressToOffset(pSrcData, &srcOffset);
				cellGcmAddressToOffset(pDstData, &dstOffset);
				cellGcmSetTransferImage(pTextureParams->m_TransferMode, dstOffset, 128, 0,0, srcOffset, 128, 0,0, 64, srcSize>>7, 2);
				cellGcmFlush();
			}
			else
			{
				FastMemCpy(pDstData, pSrcData, srcSize);
			}
		}
		break;
	default:
		{
			FastMemCpy(pDstData, pSrcData, srcSize);
		}
		break;
	}

	return true;
}

#endif

bool UploadTexture2D(C_Texture2D* pTexture, C_TextureParams* pTextureParams)
{
#if PLATFORM_PS3
	if(!(pTextureParams->m_Flags & E_Format_Linear))
		return UploadTexture2D_Tiled(pTexture, pTextureParams);
#endif

	BIZ_ASSERT(pTexture);

	if(!pTexture->IsCreated())
		return false;

	BIZ_ASSERT(pTextureParams);
	BIZ_ASSERT(pTextureParams->m_pMipData);
	BIZ_ASSERT(pTextureParams->m_MipSize > 0);
	BIZ_ASSERT(pTextureParams->m_MipOffset < pTextureParams->m_MipSize);

	U8* pData	= &pTextureParams->m_pMipData[pTextureParams->m_MipOffset];
	U32 width	= pTextureParams->m_Width;
	U32 height	= pTextureParams->m_Height;
	
	const Display2::E_TextureFormat format = GetTextureFormatFromFormat(pTextureParams->m_Format);

	for(U32 level=pTextureParams->m_MipHead; level<=pTextureParams->m_MipTail; level++)
	{
		BIZ_ASSERT(pData <= pTextureParams->m_pMipData+pTextureParams->m_MipSize);

		C_LockedRect lockedRect = pTexture->LockRect2D(level);

		if(!lockedRect.GetBits())
		{
			LOGERROR(g_Log, "Failed to lock 2d texture level %i\n", level);
			return false;
		}

		U32 pitch = Display2::GetTexturePitchFromWidth(width, pTextureParams->m_Flags, format);
		U32 size = Display2::GetTexture2DSize(width, height, pTextureParams->m_Flags, format);

		BIZ_ASSERT(pData+size <= pTextureParams->m_pMipData+pTextureParams->m_MipSize);
			
		memcpy(lockedRect.GetBits(), pData, size);
		
		pData += size;

		pTexture->UnlockRect2D(level);
			
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
	}
	
	BIZ_ASSERT(pData <= pTextureParams->m_pMipData+pTextureParams->m_MipSize); // TGA loading only supports one mip at mo...
	
	return true;
}

U32 GetTexture2DSize(const U32 width, const U32 height, const U32 flags, const E_Format format)
{
	return Display2::GetTexture2DSize(width, height, flags, GetTextureFormatFromFormat(format));
}

U32 GetTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_Format format)
{
	return Display2::GetTexture2DSize(width, height, levels, flags, GetTextureFormatFromFormat(format));
}


bool CreateTexture2DFromFile(Util::C_String fileName, C_Texture2D* pTexture)
{
	if(fileName == "")
	{
		LOGERROR(g_Log, "No file name specified when creating 2d texture from file\n");
		return false;
	}

	if (!Sys::GetDefaultFileSystem()->FileExists(fileName))
	{
		LOGERROR(g_Log, "Failed to find 2d texture file '%s'\n", fileName.Chars());
		return false;
	}
	
	fileName.ConvertToLower();

	//LOGINFO(g_Log, "Loading texture '%s'\n", fileName.Chars());

#if 0
	return D3DXCreateTexture2DFromFile(fileName, pTexture);
#endif

	C_TextureParams params;
	
	if(fileName.Search(".texture") != -1)
	{
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(fileName);
		if(!pFile)
		{
			LOGERROR(g_Log, "Failed to open 2d texture file '%s'\n", fileName.Chars());
			return false;
		}

		U32 flags = 0;

		if(!Display::LoadTexture(pTexture, pFile, flags))
		{
			LOGERROR(g_Log, "Failed to load 2d texture file '%s'\n", fileName.Chars());
			delete pFile;
			return false;
		}

		#if TRACK_DISPLAY2_RESOURCES
		Display2::Device()->SetTextureName(pTexture, fileName.Chars());
		#endif

		delete pFile;
		return true;
	}
	else
	if(fileName.Search(".dds") != -1)
	{
		if(!DDS::LoadTextureFromFile(fileName, &params))
			return false;
	}
	else
	if(fileName.Search(".tga") != -1)
	{
		if(!TGA::LoadTextureFromFile(fileName, &params))
			return false;
	}
	else
	if(fileName.Search(".bmp") != -1)
	{
		if(!BMP::LoadTextureFromFile(fileName, &params))
			return false;
	}
	else
	{
		LOGERROR(g_Log, "Unsupported 2d texture file extension '%s'\n", fileName.Chars());
		return false;
	}

	params.m_Name = fileName;

	#if defined(PLATFORM_PS3) && !defined(CFG_GOLD)
	if((params.m_Width & (params.m_Width-1)) || (params.m_Height & (params.m_Height-1)))
	{
		LOGERROR(g_Log, "Texture '%s' is not power of 2 (%ix%i)\n", fileName.Chars(), params.m_Width, params.m_Height);
	}
	#endif

	params.m_Flags |= E_Format_Linear;
	params.m_Flags |= E_Format_Raw;
	
	if(!Device()->CreateTexture2D(params.m_Width, params.m_Height, params.m_Levels, params.m_Flags, params.m_Format, pTexture))
	{
		SAFE_DELETE_ARY(params.m_pMipData);
		return false;
	}
	
	params.m_MipHead = 0;
	params.m_MipTail = params.m_Levels-1;

	if(!UploadTexture2D(pTexture, &params))
	{
		Device()->DestroyTexture2D(pTexture);
		SAFE_DELETE_ARY(params.m_pMipData);
		LOGERROR(g_Log, "Failed to upload the 2d texture '%s'\n", fileName.Chars());
		return false;
	}
	
	#if TRACK_DISPLAY2_RESOURCES
	Display2::Device()->SetTextureName(pTexture, fileName);
	#endif

	SAFE_DELETE_ARY(params.m_pMipData);

	return true;
}

U32 GetRenderTexture2DSize(U32 width, U32 height, const U32 levels, const U32 flags, const E_Format format, const E_Multisample multisample)
{
	U32 size = 0;
	for(U32 level=0; level<levels; level++)
	{
		size += GetSurfaceSize(width, height, flags, format, multisample);
		if(width > 1)
			width >>= 1;
		if(height > 1)
			height >>= 1;
	}
	return size;
}

U32 GetActiveDataSize(const C_Texture2D* pTexture)
{
	if (!pTexture)
		return 0;

	U32 size = 0;


	U32 width = pTexture->GetWidth();
	U32 height = pTexture->GetHeight();

	for (U32 level = 0; level < pTexture->GetLevels(); level++)
	{
		GetTexture2DSize(width, height, pTexture->GetFlags(), pTexture->GetFormat());
		width = Maths::Max(width / 2, (U32) 1);
		height = Maths::Max(height / 2, (U32) 1);
	}


	return size;
}

bool IsFloatFormat( const C_Texture2D* pTexture )
{
	if	(
		pTexture->GetFormat() == (Display2::E_TextureFormat)Display::E_Format_G16R16F ||
#if !PLATFORM_PS3
#if !PLATFORM_PC
		pTexture->GetFormat() == (Display2::E_TextureFormat)Display::E_Format_A2B10G10R10F ||
#endif
		pTexture->GetFormat() == (Display2::E_TextureFormat)Display::E_Format_R16F ||
#endif
		pTexture->GetFormat() == (Display2::E_TextureFormat)Display::E_Format_A16B16G16R16F ||
		pTexture->GetFormat() == (Display2::E_TextureFormat)Display::E_Format_R32F ||
		pTexture->GetFormat() == (Display2::E_TextureFormat)Display::E_Format_A32B32G32R32F
		)
		return true;
	else
		return false;
}

}; // namespace Display
