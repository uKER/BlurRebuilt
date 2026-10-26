#include "stdafx.h"
#include "Display/Texture/DDS/DDSTextureUtils.h"
#include "Display/Utils/File.h"

namespace Display
{
namespace DDS
{

const U32 DDSF_CAPS       			= 0x00000001l;
const U32 DDSF_HEIGHT     			= 0x00000002l;
const U32 DDSF_WIDTH      			= 0x00000004l;
const U32 DDSF_PITCH      			= 0x00000008l;
const U32 DDSF_PIXELFORMAT			= 0x00001000l;
const U32 DDSF_MIPMAPCOUNT			= 0x00020000l;
const U32 DDSF_LINEARSIZE 			= 0x00080000l;
const U32 DDSF_DEPTH      			= 0x00800000l;

const U32 DDSF_ALPHAPIXELS   		= 0x00000001l;
const U32 DDSF_FOURCC        		= 0x00000004l;
const U32 DDSF_RGB           		= 0x00000040l;
const U32 DDSF_RGBA          		= 0x00000041l;

const U32 DDSF_COMPLEX         		= 0x00000008l;
const U32 DDSF_TEXTURE         		= 0x00001000l;
const U32 DDSF_MIPMAP          		= 0x00400000l;

const U32 DDSF_CUBEMAP				= 0x00000200l;
const U32 DDSF_CUBEMAP_POSITIVEX 	= 0x00000400l;
const U32 DDSF_CUBEMAP_NEGATIVEX 	= 0x00000800l;
const U32 DDSF_CUBEMAP_POSITIVEY 	= 0x00001000l;
const U32 DDSF_CUBEMAP_NEGATIVEY 	= 0x00002000l;
const U32 DDSF_CUBEMAP_POSITIVEZ 	= 0x00004000l;
const U32 DDSF_CUBEMAP_NEGATIVEZ 	= 0x00008000l;
const U32 DDSF_CUBEMAP_ALL_FACES 	= 0x0000FC00l;
const U32 DDSF_VOLUME				= 0x00200000l;

const U32 FOURCC_UNKNOWN			= 0;

#ifndef MAKEFOURCC
#define MAKEFOURCC(c0,c1,c2,c3) \
	((U32)(U8)(c0)| \
	((U32)(U8)(c1) << 8)| \
	((U32)(U8)(c2) << 16)| \
	((U32)(U8)(c3) << 24))
#endif

const U32 FOURCC_R8G8B8        		= 20;
const U32 FOURCC_A8R8G8B8      		= 21;
const U32 FOURCC_X8R8G8B8      		= 22;
const U32 FOURCC_R5G6B5        		= 23;
const U32 FOURCC_X1R5G5B5      		= 24;
const U32 FOURCC_A1R5G5B5      		= 25;
const U32 FOURCC_A4R4G4B4      		= 26;
const U32 FOURCC_R3G3B2        		= 27;
const U32 FOURCC_A8            		= 28;
const U32 FOURCC_A8R3G3B2      		= 29;
const U32 FOURCC_X4R4G4B4      		= 30;
const U32 FOURCC_A2B10G10R10   		= 31;
const U32 FOURCC_A8B8G8R8      		= 32;
const U32 FOURCC_X8B8G8R8      		= 33;
const U32 FOURCC_G16R16        		= 34;
const U32 FOURCC_A2R10G10B10   		= 35;
const U32 FOURCC_A16B16G16R16  		= 36;

const U32 FOURCC_L8            		= 50;
const U32 FOURCC_A8L8          		= 51;
const U32 FOURCC_A4L4          		= 52;
const U32 FOURCC_DXT1          		= 0x31545844l; //(MAKEFOURCC('D','X','T','1'))
const U32 FOURCC_DXT2          		= 0x32545844l; //(MAKEFOURCC('D','X','T','1'))
const U32 FOURCC_DXT3          		= 0x33545844l; //(MAKEFOURCC('D','X','T','3'))
const U32 FOURCC_DXT4          		= 0x34545844l; //(MAKEFOURCC('D','X','T','3'))
const U32 FOURCC_DXT5          		= 0x35545844l; //(MAKEFOURCC('D','X','T','5'))
const U32 FOURCC_ATI1          		= MAKEFOURCC('A','T','I','1');
const U32 FOURCC_ATI2          		= MAKEFOURCC('A','T','I','2');

const U32 FOURCC_D16_LOCKABLE  		= 70;
const U32 FOURCC_D32           		= 71;
const U32 FOURCC_D24X8         		= 77;
const U32 FOURCC_D16           		= 80;

const U32 FOURCC_D32F_LOCKABLE 		= 82;

const U32 FOURCC_L16           		= 81;

const U32 FOURCC_R16F          		= 111;
const U32 FOURCC_G16R16F       		= 112;
const U32 FOURCC_A16B16G16R16F 		= 113;

const U32 FOURCC_R32F          		= 114;
const U32 FOURCC_G32R32F       		= 115;
const U32 FOURCC_A32B32G32R32F 		= 116;

struct DDS_PIXELFORMAT
{
	U32					m_Size;
    U32					m_Flags;
    U32					m_FourCC;
    U32					m_RGBBitCount;
    U32					m_RBitMask;
    U32					m_GBitMask;
    U32					m_BBitMask;
    U32					m_ABitMask;
};

struct DDS_TEXTURE
{
	char				m_Type[4];
	U32					m_Size;
    U32					m_Flags;
    U32					m_Height;
    U32					m_Width;
    U32					m_PitchOrLinearSize;
    U32					m_Depth;
    U32					m_Levels;
	U32					m_Reserved1[11];
	DDS_PIXELFORMAT		m_PixelFormat;
    U32					m_Caps1;
    U32					m_Caps2;
    U32					m_Reserved2[3];
};

const Util::C_String GetFourCCName(const U32 fourCC)
{
	switch(fourCC)
	{
	case FOURCC_DXT1:					return "FOURCC_DXT1";
	case FOURCC_DXT2:					return "FOURCC_DXT2";
	case FOURCC_DXT3:					return "FOURCC_DXT3";
	case FOURCC_DXT4:					return "FOURCC_DXT4";
	case FOURCC_DXT5:					return "FOURCC_DXT5";
	case FOURCC_ATI1:					return "FOURCC_ATI1";
	case FOURCC_ATI2:					return "FOURCC_ATI2";
    case FOURCC_R8G8B8:					return "FOURCC_R8G8B8";
    case FOURCC_A8R8G8B8:				return "FOURCC_A8R8G8B8";
    case FOURCC_X8R8G8B8:				return "FOURCC_X8R8G8B8";
    case FOURCC_R5G6B5:					return "FOURCC_R5G6B5";
    case FOURCC_A8:						return "FOURCC_A8";
    case FOURCC_A2B10G10R10:			return "FOURCC_A2B10G10R10";
    case FOURCC_A8B8G8R8:				return "FOURCC_A8B8G8R8";
    case FOURCC_X8B8G8R8:				return "FOURCC_X8B8G8R8";
    case FOURCC_A2R10G10B10:			return "FOURCC_A2R10G10B10";
    case FOURCC_A16B16G16R16:			return "FOURCC_A16B16G16R16";
    case FOURCC_L8:						return "FOURCC_L8";
    case FOURCC_A8L8:					return "FOURCC_A8L8";
    case FOURCC_L16:					return "FOURCC_L16";
    case FOURCC_R16F:					return "FOURCC_R16F";
    case FOURCC_A16B16G16R16F:			return "FOURCC_A16B16G16R16F";
    case FOURCC_R32F:					return "FOURCC_R32F";
    case FOURCC_A32B32G32R32F:			return "FOURCC_A32B32G32R32F";
    case FOURCC_UNKNOWN:				return "FOURCC_UNKNOWN";
    case FOURCC_X1R5G5B5:				return "FOURCC_X1R5G5B5";
    case FOURCC_A1R5G5B5:				return "FOURCC_A1R5G5B5";
    case FOURCC_A4R4G4B4:				return "FOURCC_A4R4G4B4";
    case FOURCC_R3G3B2:					return "FOURCC_R3G3B2";
    case FOURCC_A8R3G3B2:				return "FOURCC_A8R3G3B2";
    case FOURCC_X4R4G4B4:				return "FOURCC_X4R4G4B4";
    case FOURCC_A4L4:					return "FOURCC_A4L4";
    case FOURCC_D16_LOCKABLE:			return "FOURCC_D16_LOCKABLE";
    case FOURCC_D32:					return "FOURCC_D32";
    case FOURCC_D24X8:					return "FOURCC_D24X8";
    case FOURCC_D16:					return "FOURCC_D16";
    case FOURCC_D32F_LOCKABLE:			return "FOURCC_D32F_LOCKABLE";
    case FOURCC_G16R16:					return "FOURCC_G16R16";
    case FOURCC_G16R16F:				return "FOURCC_G16R16F";
    case FOURCC_G32R32F:				return "FOURCC_G32R32F";
	default:							return "FOURCC_UNSUPPORTED";
	}
}

bool LoadTextureFromMemory(U8* pData, const size_t size, C_TextureParams* pParams, const char* fileName)
{
	BIZ_ASSERT(pParams);
	BIZ_ASSERT(size > sizeof(DDS_TEXTURE));

	DDS_TEXTURE* pTexture = (DDS_TEXTURE*)pData;

	if (strncmp(pTexture->m_Type, "DDS ", 4) != 0)
	{
		if (fileName)
			LOGFATAL(g_Log, "Texture '%s' not a dds file", fileName);
		else
			LOGFATAL(g_Log, "Texture buffer not a dds file");

		return false;
	}

#if PLATFORM_PS3
	U32* pPtr = &pTexture->m_Size;
	for(U32 i=0; i<(sizeof(DDS_TEXTURE)-4)/4; i++)
	{
		*pPtr = ((*pPtr >> 24) & 0x000000ff) | ((*pPtr >>  8) & 0x0000ff00) | ((*pPtr <<  8) & 0x00ff0000) | ((*pPtr << 24) & 0xff000000);
		pPtr++;
	}
#endif

	pParams->m_Width		= pTexture->m_Width;
	pParams->m_Height		= pTexture->m_Height;
	pParams->m_Depth		= pTexture->m_Depth;
	pParams->m_Levels		= pTexture->m_Levels > 0 ? pTexture->m_Levels : 1;
	pParams->m_MipSize		= size;
	pParams->m_MipOffset	= sizeof(DDS_TEXTURE);
	pParams->m_pMipData		= pData;
	pParams->m_Format		= E_Format_Unknown;

	if(pTexture->m_PixelFormat.m_Flags & DDSF_FOURCC)
	{
		switch(pTexture->m_PixelFormat.m_FourCC)
		{
		case FOURCC_DXT1:
			pParams->m_Format = E_Format_DXT1;
			break;
		case FOURCC_DXT2:
        case FOURCC_DXT3:
			pParams->m_Format = E_Format_DXT3;
			break;
		case FOURCC_DXT4:
		case FOURCC_DXT5:
			pParams->m_Format = E_Format_DXT5;
			break;
		default:
			if (fileName)
				LOGERROR(g_Log, "Unsupported compressed texture format %s in file '%s'", GetFourCCName(pTexture->m_PixelFormat.m_FourCC).Chars(), fileName);
			else
				LOGERROR(g_Log, "Unsupported compressed texture format %s in buffer", GetFourCCName(pTexture->m_PixelFormat.m_FourCC).Chars());
			break;
		}
	}
	else
	if(pTexture->m_PixelFormat.m_Flags == DDSF_RGBA && pTexture->m_PixelFormat.m_RGBBitCount == 32)
	{
		pParams->m_Format = E_Format_A8R8G8B8;
	}
	else
	if(pTexture->m_PixelFormat.m_Flags == DDSF_RGB && pTexture->m_PixelFormat.m_RGBBitCount == 32)
	{
		pParams->m_Format = E_Format_A8R8G8B8; // nom?
		//LOGERROR(g_Log, "Unsupported uncompressed 32 bit texture format DDSF_RGB in file '%s'\n", fileName.Chars());
	}
	else
	if(pTexture->m_PixelFormat.m_Flags == DDSF_RGB && pTexture->m_PixelFormat.m_RGBBitCount == 24)
	{
		if (fileName)
			LOGERROR(g_Log, "Unsupported uncompressed 24 bit texture format DDSF_RGB in file '%s'", fileName);
		else
			LOGERROR(g_Log, "Unsupported uncompressed 24 bit texture format DDSF_RGB in buffer");
	}
	else
	if(pTexture->m_PixelFormat.m_RGBBitCount == 8)
	{
		pParams->m_Format = E_Format_L8;
	}
	else 
	{
		if (fileName)
			LOGERROR(g_Log, "Unsupported texture format in file '%s'", fileName);
		else
			LOGERROR(g_Log, "Unsupported texture format in file buffer");
	}

	if(pParams->m_Format != E_Format_Unknown)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool LoadTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams)
{
	BIZ_ASSERT(pParams);

	U32 size = 0;
	U8* pData = NULL;
	if(!GetFileData(fileName.Chars(), size, &pData))
		return false;

	bool result = LoadTextureFromMemory(pData, size, pParams, fileName.Chars());

	if (!result)
	{
		SAFE_DELETE_ARY(pData);
	}

	return result;
}

}; // namespace DDS
}; // namespace Display
