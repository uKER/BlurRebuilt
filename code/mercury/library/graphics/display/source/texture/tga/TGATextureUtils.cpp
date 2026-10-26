#include "stdafx.h"
#include "Display/Texture/TGA/TGATextureUtils.h"
#include "Display/Utils/File.h"

namespace Display
{
namespace TGA
{

bool LoadTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams)
{
	BIZ_ASSERT(pParams);

	U32 size = 0;
	U8* pData = NULL;
	if(!GetFileData(fileName.Chars(), size, &pData))
		return false;

	if(pData[2] != 2)
	{
		LOGERROR(g_Log, "File '%s' is not uncompressed\n", fileName.Chars());
		SAFE_DELETE_ARY(pData);
		return false;
	}

	if(pData[16] != 32 && pData[16] != 24)
	{
		LOGERROR(g_Log, "File '%s' is not 32 or 24 bit\n", fileName.Chars());
		SAFE_DELETE_ARY(pData);
		return false;
	}

	pParams->m_Width		= pData[13] * 256 + pData[12];
	pParams->m_Height		= pData[15] * 256 + pData[14];
	pParams->m_Depth		= 1;
	pParams->m_Levels		= 1;
	pParams->m_MipSize		= size;
	pParams->m_MipOffset	= 18;
	pParams->m_pMipData		= pData;
	pParams->m_Format		= E_Format_A8R8G8B8;

	if(pData[16] == 24)
	{
		U8* pSrc = &pParams->m_pMipData[pParams->m_MipOffset];
		U8* pDst = new U8[pParams->m_Width*pParams->m_Height*4];

		for(U32 h=0; h<pParams->m_Height; h++)
		{
			for(U32 w=0; w<pParams->m_Width; w++)
			{
				pDst[(((pParams->m_Height-h-1)*4*pParams->m_Width) + (w*4)) + 0] = pSrc[((h*3*pParams->m_Width) + (w*3)) + 0];
				pDst[(((pParams->m_Height-h-1)*4*pParams->m_Width) + (w*4)) + 1] = pSrc[((h*3*pParams->m_Width) + (w*3)) + 1];
				pDst[(((pParams->m_Height-h-1)*4*pParams->m_Width) + (w*4)) + 2] = pSrc[((h*3*pParams->m_Width) + (w*3)) + 2];
				pDst[(((pParams->m_Height-h-1)*4*pParams->m_Width) + (w*4)) + 3] = 255;
			}
		}

		SAFE_DELETE_ARY(pData);

		pParams->m_MipSize = pParams->m_Width*pParams->m_Height*4;
		pParams->m_MipOffset = 0;
		pParams->m_pMipData = pDst;
	}

	if(pParams->m_Format != E_Format_Unknown)
	{
		return true;
	}
	else
	{
		SAFE_DELETE_ARY(pData);
		return false;
	}
}

}; // namespace TGA
}; // namespace Display
