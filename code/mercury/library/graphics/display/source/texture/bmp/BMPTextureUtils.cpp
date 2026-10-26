#include "stdafx.h"
#include "Display/Texture/BMP/BMPTextureUtils.h"
#include "Display/Utils/File.h"

namespace Display
{
namespace BMP
{

#pragma pack(push, bitmap_data, 1)

struct C_BitmapFileHeader
{
	unsigned short	bfType;
	unsigned int	bfSize;
	unsigned short	bfReserved1;
	unsigned short	bfReserved2;
	unsigned int	bfOffBits;
};

struct C_BitmapInfoHeader
{
	unsigned int	biSize;
	int 			biWidth;
	int 			biHeight;
	unsigned short	biPlanes;
	unsigned short	biBitCount;
	unsigned int	biCompression;
	unsigned int	biSizeImage;
	int 			biXPelsPerMeter;
	int 			biYPelsPerMeter;
	unsigned int	biClrUsed;
	unsigned int	biClrImportant;
};

#pragma pack(pop, bitmap_data)

bool LoadTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams)
{
	BIZ_ASSERT(pParams);

	U32 size = 0;
	U8* pData = NULL;
	if(!GetFileData(fileName.Chars(), size, &pData))
		return false;

	U32 i = 0;

	C_BitmapFileHeader* pFileHeader = (C_BitmapFileHeader*)&pData[i];
	i += sizeof(C_BitmapFileHeader);

	C_BitmapInfoHeader* pInfoHeader = (C_BitmapInfoHeader*)&pData[i];
	i += sizeof(C_BitmapInfoHeader);

#if PLATFORM_PS3
	{
		U32* pPtr = (U32*)pFileHeader;
		for(U32 i=0; i<(sizeof(C_BitmapFileHeader)-4)/4; i++)
		{
			*pPtr = ((*pPtr >> 24) & 0x000000ff) | ((*pPtr >>  8) & 0x0000ff00) | ((*pPtr <<  8) & 0x00ff0000) | ((*pPtr << 24) & 0xff000000);
			pPtr++;
		}
	}
	{
		U32* pPtr = (U32*)pInfoHeader;
		for(U32 i=0; i<(sizeof(C_BitmapInfoHeader)-4)/4; i++)
		{
			*pPtr = ((*pPtr >> 24) & 0x000000ff) | ((*pPtr >>  8) & 0x0000ff00) | ((*pPtr <<  8) & 0x00ff0000) | ((*pPtr << 24) & 0xff000000);
			pPtr++;
		}
	}
#endif

	BIZ_ASSERT(pInfoHeader->biBitCount == 1);
	
	pParams->m_Width		= pInfoHeader->biWidth;
	pParams->m_Height		= abs(pInfoHeader->biHeight);
	pParams->m_Depth		= 1;
	pParams->m_Levels		= 1;
	pParams->m_Format		= E_Format_A8R8G8B8;

	pParams->m_MipSize		= pParams->m_Width*pParams->m_Height*4;
	pParams->m_MipOffset	= 0;
	pParams->m_pMipData		= new U8[pParams->m_MipSize];

	if(pInfoHeader->biPlanes == 32)
	{
		for(U32 h=0; h<pParams->m_Height; h++)
		{
			for(U32 w=0; w<pParams->m_Width; w++)
			{
				const U32 src = i+((h*4*pParams->m_Width)+(w*4));
				const U32 h1 = pInfoHeader->biHeight > 0 ? (pParams->m_Height-h-1) : h ;
				const U32 dst = ((h1*4*pParams->m_Width)+(w*4));

				#if PLATFORM_PS3
				pParams->m_pMipData[dst+0] = pData[src+3];
				pParams->m_pMipData[dst+1] = pData[src+2];
				pParams->m_pMipData[dst+2] = pData[src+1];
				pParams->m_pMipData[dst+3] = pData[src+0];
				#else
				pParams->m_pMipData[dst+0] = pData[src+0];
				pParams->m_pMipData[dst+1] = pData[src+1];
				pParams->m_pMipData[dst+2] = pData[src+2];
				pParams->m_pMipData[dst+3] = pData[src+3];
				#endif
			}
		}
	}
	else
	if(pInfoHeader->biPlanes == 24)
	{
		for(U32 h=0; h<pParams->m_Height; h++)
		{
			for(U32 w=0; w<pParams->m_Width; w++)
			{
				const U32 src = i+((h*3*pParams->m_Width)+(w*3));
				const U32 h1 = pInfoHeader->biHeight > 0 ? (pParams->m_Height-h-1) : h ;
				const U32 dst = ((h1*4*pParams->m_Width)+(w*4));

				#if PLATFORM_PS3
				pParams->m_pMipData[dst+0] = UCHAR_MAX;
				pParams->m_pMipData[dst+1] = pData[src+2];
				pParams->m_pMipData[dst+2] = pData[src+1];
				pParams->m_pMipData[dst+3] = pData[src+0];
				#else
				pParams->m_pMipData[dst+0] = pData[src+0];
				pParams->m_pMipData[dst+1] = pData[src+1];
				pParams->m_pMipData[dst+2] = pData[src+2];
				pParams->m_pMipData[dst+3] = UCHAR_MAX;
				#endif
			}
		}
	}
	else
	{
		LOGERROR(g_Log, "Unsupported bit planes for texture '%s'\n", fileName.Chars());
		SAFE_DELETE_ARY(pData);
		return false;
	}

	SAFE_DELETE_ARY(pData);
	
	return true;
}

}; // namespace BMP
}; // namespace Display
