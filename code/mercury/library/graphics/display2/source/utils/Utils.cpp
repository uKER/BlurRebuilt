#include "StdAfx.h"
#include "Display2/Utils/Utils.h"
#include "Display2/Device/Device.h"
#include <System/Serialise.h>
#include <System/Time/Time.h>
#include <Btl/Utility/Hash.h>

#include <utility>

namespace Display2
{

Debug::Bool		g_debugDisplay2TimingsEnable	("Graphics.Display.Render.TimingRender.Enable2",	true);
Debug::Int		g_debugDisplay2TimingsDepth		("Graphics.Display.Render.TimingRender.Depth2", 	9, Debug::Range(0, 9));
Debug::Int		g_debugDisplay2TimingsLevel		("Graphics.Display.Render.TimingRender.Level2", 	9, Debug::Range(0, 9));

void* EndianSwap16(U16* data, S32 count)
{
	for(S32 i=0; i<count; i++)
	{
		unsigned char* pElement = (unsigned char*)data;
		std::swap(pElement[0], pElement[1]);
		data++;
	}

	return data;
}

void* EndianSwap32(U32* data, S32 count)
{
	for(S32 i=0; i<count; i++ )
	{
		unsigned char* pElement = (unsigned char*)data;
		std::swap(pElement[0], pElement[3]);
		std::swap(pElement[1], pElement[2]);
		data++;
	}

	return data;
}

void* EndianSwap64(U64* data, S32 count)
{
	for(S32 i=0; i<count; i++ )
	{
		unsigned char* pElement = (unsigned char*)data;
		std::swap(pElement[0], pElement[7]);
		std::swap(pElement[1], pElement[6]);
		std::swap(pElement[2], pElement[5]);
		std::swap(pElement[3], pElement[4]);
		data++;
	}

	return data;
}

U64 GetFileTime(const Util::C_String& fileName)
{
	const Sys::C_FilePath file = Sys::GetDefaultFileSystem()->GetWorkingDirectory() + "/" + fileName;
	
	Sys::C_FileInfo fileInfo;
	if (Sys::GetDefaultFileSystem()->FileInfo(file, &fileInfo))
	{
		return fileInfo.m_FileTime;
	}
	else
	{
		return 0;
	}
}

bool GetFileData(const Util::C_String& fileName, U32& fileSize, U8** ppFileData)
{
	fileSize	= 0;
	*ppFileData	= 0;

	bool result = false;

	if (Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(fileName))
	{
		if ((fileSize = (U32)pFile->Length()) > 0)
		{
			if ((*ppFileData = new U8[fileSize]) != 0)
			{
				if (pFile->Read(*ppFileData, fileSize))
				{
					result = true;
				}
				else
				{
					LOGWARN(g_Log, "Failed to read file '%s'\n", fileName.Chars());
				}
			}
			else
			{
				LOGWARN(g_Log, "Out of memory reading '%s'\n", fileName.Chars());
			}
		}
		else
		{
			LOGWARN(g_Log, "File is empty '%s'\n", fileName.Chars());
		}

		delete pFile;
	}
	else
	{
		LOGWARN(g_Log, "Failed to open file '%s'\n", fileName.Chars());
	}

	if (!result)
	{
		delete [] *ppFileData;

		fileSize		= 0;
		*ppFileData		= 0;
	}

	return result;
}

//------------------------------------------------------------------------------
// Shaders

Util::C_String GetVertexShaderFilePathFromName(const Util::C_String& fileName)
{
	return fileName + "." + g_VertexShaderFileExt;
}

Util::C_String GetPixelShaderFilePathFromName(const Util::C_String& fileName)
{
	return fileName + "." + g_PixelShaderFileExt;
}

U64 GetShaderFileTimeFromPath(const Util::C_String& filePath)
{
	return GetFileTime(filePath);
}

U64 GetVertexShaderFileTimeFromName(const Util::C_String& fileName)
{
	const Util::C_String filePath = GetVertexShaderFilePathFromName(fileName);
	return GetShaderFileTimeFromPath(filePath);
}

U64 GetPixelShaderFileTimeFromName(const Util::C_String& fileName)
{
	const Util::C_String filePath = GetPixelShaderFilePathFromName(fileName);
	return GetShaderFileTimeFromPath(filePath);
}

bool LoadShaderFromFilePath(const Util::C_String& filePath, U32& size, U8** ppData)
{
	if(!GetFileData(filePath, size, ppData))
	{
		LOGERROR(g_Log, "Failed to open file %s\n", filePath.Chars());
		return false;
	}

	return true;
}

bool LoadVertexShaderFromFileName(const Util::C_String& fileName, U32& size, U8** ppData)
{
	const Util::C_String filePath = GetVertexShaderFilePathFromName(fileName);
	return LoadShaderFromFilePath(filePath, size, ppData);
}

bool LoadPixelShaderFromFileName(const Util::C_String& fileName, U32& size, U8** ppData)
{
	const Util::C_String filePath = GetPixelShaderFilePathFromName(fileName);
	return LoadShaderFromFilePath(filePath, size, ppData);
}

bool CreateVertexShaderFromFunc(const Util::C_String& fileName, const U32 flags, C_VertexShader* pVertexShader, const U32 size, const U8* pData)
{
	assert(pVertexShader);

	if(!Device()->CreateVertexShader(size, pData, flags, pVertexShader))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	C_ResourceInfo* pResourceInfo = Device()->GetVertexShaderResourceInfo(pVertexShader);
	pResourceInfo->m_Name = fileName;
	pResourceInfo->m_Time = Time::GetCurrentTime();
	#endif

	return true;
}

bool CreatePixelShaderFromFunc(const Util::C_String& fileName, const U32 flags, C_PixelShader* pPixelShader, const U32 size, const U8* pData)
{
	assert(pPixelShader);

	if(!Device()->CreatePixelShader(size, pData, flags, pPixelShader))
		return false;

	#if TRACK_DISPLAY2_RESOURCES
	C_ResourceInfo* pResourceInfo = Device()->GetPixelShaderResourceInfo(pPixelShader);
	pResourceInfo->m_Name = fileName;
	pResourceInfo->m_Time= Time::GetCurrentTime();
	#endif

	return true;
}

bool CreateVertexShaderFromFile(const Util::C_String& fileName, const U32 flags, C_VertexShader* pVertexShader)
{
	assert(pVertexShader);

	const Util::C_String filePath = GetVertexShaderFilePathFromName(fileName);

	U32 fileSize = 0;
	U8* pFileData = NULL;
	if(!LoadShaderFromFilePath(filePath, fileSize, &pFileData))
	{
		LOGERROR(g_Log, "Failed to load the vertex shader from file '%s'\n", filePath.Chars());
		return false;
	}
	
	Device()->Destroy(pVertexShader);
	bool success = Device()->CreateVertexShader(fileSize, pFileData, flags, pVertexShader);
	
	#if DEVICE_GCM
	pVertexShader->m_pAddressInMainMemory = pFileData;
	#else
	SAFE_DELETE_ARY(pFileData);
	#endif

	if(!success)
	{
		LOGERROR(g_Log, "Failed to create the vertex shader from file '%s'\n", filePath.Chars());
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	C_ResourceInfo* pResourceInfo = Device()->GetVertexShaderResourceInfo(pVertexShader);
	pResourceInfo->m_Name = fileName;
	pResourceInfo->m_Time = Time::GetCurrentTime();
	#endif
	
	return true;
}

bool CreatePixelShaderFromFile(const Util::C_String& fileName, const U32 flags, C_PixelShader* pPixelShader)
{
	assert(pPixelShader);

	const Util::C_String filePath = GetPixelShaderFilePathFromName(fileName);

	U32 fileSize = 0;
	U8* pFileData = NULL;
	if(!LoadShaderFromFilePath(filePath, fileSize, &pFileData))
	{
		LOGERROR(g_Log, "Failed to load the pixel shader from file '%s'\n", filePath.Chars());
		return false;
	}
	
	#if !TRACK_DISPLAY2_RESOURCES && !PLATFORM_PS3
	Device()->Destroy(pPixelShader);
	#endif

	bool success = Device()->CreatePixelShader(fileSize, pFileData, flags, pPixelShader);
	
	#if DEVICE_GCM
	pPixelShader->m_pAddressInMainMemory = pFileData;
	#else
	SAFE_DELETE_ARY(pFileData);
	#endif
	
	if(!success)
	{
		LOGERROR(g_Log, "Failed to create the pixel shader from file '%s'\n", filePath.Chars());
		return false;
	}

	#if TRACK_DISPLAY2_RESOURCES
	C_ResourceInfo* pResourceInfo = Device()->GetPixelShaderResourceInfo(pPixelShader);
	pResourceInfo->m_Name = fileName;
	pResourceInfo->m_Time = Time::GetCurrentTime();
	#endif
	
	return true;
}

void RefreshShaders(bool refreshVertexShaders, bool refreshPixelShaders)
{
#if TRACK_DISPLAY2_RESOURCES
	Device()->LockMutex();

	Btl::Vector<C_ResourceListIt> vertexShaders;
	
	if(refreshVertexShaders)
	{
		C_ResourceListIt it = Device()->GetVertexShaderBegin();
		while(it != Device()->GetVertexShaderEnd())
		{
			C_VertexShader*	pVertexShader = (C_VertexShader*)it->GetKey();
			C_ResourceInfo* pResourceInfo = it->GetValue();

			if(pResourceInfo != NULL && pResourceInfo->m_Name != "")
			{
				const U64 fileTime = GetVertexShaderFileTimeFromName(pResourceInfo->m_Name.Chars());

				if(fileTime > pResourceInfo->m_Time)
				{
					pResourceInfo->m_Time = fileTime;

					vertexShaders.PushBack(it);
				}
				else
				if(fileTime == 0)
				{
					//LOGWARN(g_Log, "Failed to open vertex shader file '%s'\n", pResourceInfo->m_Path.Chars());
				}
			}

			it++;
		}
	}

	Btl::Vector<C_ResourceListIt> pixelShaders;

	if(refreshPixelShaders)
	{
		C_ResourceListIt it = Device()->GetPixelShaderBegin();
		while(it != Device()->GetPixelShaderEnd())
		{
			C_PixelShader*	pPixelShader = (C_PixelShader*)it->GetKey();
			C_ResourceInfo* pResourceInfo = it->GetValue();

			if(pResourceInfo != NULL && pResourceInfo->m_Name != "")
			{
				const U64 fileTime = GetPixelShaderFileTimeFromName(pResourceInfo->m_Name.Chars());

				if(fileTime > pResourceInfo->m_Time)
				{
					pResourceInfo->m_Time = fileTime;

					pixelShaders.PushBack(it);
				}
				else
				if(fileTime == 0)
				{
					//LOGWARN(g_Log, "Failed to open pixel shader file '%s'\n", pResourceInfo->m_Path.Chars());
				}
			}

			it++;

		}
	}

	if(vertexShaders.Size() > 0 || pixelShaders.Size() > 0)
	{
		#if PLATFORM_PS3
		Device()->BlockUntilIdle();
		#endif

		for(S32 i=0; i<vertexShaders.Size(); i++)
		{
			C_ResourceListIt it = vertexShaders[i];

			C_VertexShader*	pVertexShader = (C_VertexShader*)it->GetKey();
			C_ResourceInfo* pResourceInfo = it->GetValue();

			const U64 fileTime = pResourceInfo->m_Time;
			const Util::C_String fileName = pResourceInfo->m_Name;
			const Sys::C_FilePath filePath = fileName;

			if(CreateVertexShaderFromFile(fileName.Chars(), 0, pVertexShader))
			{
				C_ResourceInfo* pResourceInfo = Device()->GetVertexShaderResourceInfo(pVertexShader);

				pResourceInfo->m_Time = fileTime;
				pResourceInfo->m_Name = fileName;

				LOGINFO(g_Log, "Successfully refreshed vertex shader from file '%s'", fileName.Chars());
			}
			else
			{
				LOGWARN(g_Log, "Failed to refresh vertex shader from file '%s'\n", fileName.Chars());
			}
		}

		vertexShaders.Clear();

		for(S32 i=0; i<pixelShaders.Size(); i++)
		{
			C_ResourceListIt it = pixelShaders[i];

			C_PixelShader*	pPixelShader = (C_PixelShader*)it->GetKey();
			C_ResourceInfo* pResourceInfo = it->GetValue();

			const U64 fileTime = pResourceInfo->m_Time;
			const Util::C_String fileName = pResourceInfo->m_Name;
			const Sys::C_FilePath filePath = fileName;

			if(CreatePixelShaderFromFile(filePath.Chars(), 0, pPixelShader))
			{
				C_ResourceInfo* pResourceInfo = Device()->GetPixelShaderResourceInfo(pPixelShader);

				pResourceInfo->m_Time = fileTime;
				pResourceInfo->m_Name = fileName;

				LOGINFO(g_Log, "Successfully refreshed pixel shader from file '%s'", fileName.Chars());
			}
			else
			{
				LOGWARN(g_Log, "Failed to refresh pixel shader from file '%s'\n", fileName.Chars());
			}
		}
		
		pixelShaders.Clear();
	}

	Device()->UnlockMutex();
#endif
}

//------------------------------------------------------------------------------
// Textures

C_TextureParams::C_TextureParams() :
	m_Width(0),
	m_Height(0),
	m_Depth(0),
	m_Levels(0),
	m_Flags(0),
	m_Format(E_TextureFormat_Unknown),
	m_MipOffset(0),
	m_MipStart(0),
	m_MipEnd(0),
	m_FileSize(0),
	m_pFileData(NULL)
{
}

bool LoadDDSTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams)
{
	#define DDSF_FOURCC		0x00000004
	#define DDSF_RGBA		0x00000041
	#define FOURCC_DXT1		0x31545844
	#define FOURCC_DXT3		0x33545844
	#define FOURCC_DXT5		0x35545844

	struct DDS_TEXTURE
	{
		char	m_Type[4];
		U32		m_Size;
		U32		m_Flags;
		U32		m_Height;
		U32		m_Width;
		U32		m_PitchOrLinearSize;
		U32		m_Depth;
		U32		m_Levels;
		U32		m_Reserved1[11];
		U32		m_Size1;
		U32		m_Flags1;
		U32		m_FourCC;
		U32		m_RGBBitCount;
		U32		m_RBitMask;
		U32		m_GBitMask;
		U32		m_BBitMask;
		U32		m_ABitMask;
		U32		m_Caps1;
		U32		m_Caps2;
		U32		m_Reserved2[3];
	};

	assert(pParams);

	U32 size = 0;
	U8* pData = NULL;
	if(!GetFileData(fileName, size, &pData))
	{
		LOGERROR(g_Log, "Failed to open file %s\n", fileName.Chars());
		return false;
	}

	DDS_TEXTURE* pTexture = (DDS_TEXTURE*)pData;

	if(strncmp(pTexture->m_Type, "DDS ", 4) != 0)
	{
		SAFE_DELETE_ARY(pData);
		LOGERROR(g_Log, "%s is not a dds file\n", fileName.Chars());
		return false;
	}

	#if PLATFORM_PS3
	U32* pPtr = &pTexture->m_Size;
	for (U32 i = 0; i < (sizeof(DDS_TEXTURE) - 4) / 4; i++)
	{
		*pPtr = ((*pPtr >> 24) & 0x000000ff) | ((*pPtr >>  8) & 0x0000ff00) | ((*pPtr <<  8) & 0x00ff0000) | ((*pPtr << 24) & 0xff000000);
		pPtr++;
	}
	#endif

	pParams->m_Width		= pTexture->m_Width;
	pParams->m_Height		= pTexture->m_Height;
	pParams->m_Depth		= pTexture->m_Depth;
	pParams->m_Levels		= pTexture->m_Levels > 0 ? pTexture->m_Levels : 1;
	#if !DEVICE_GCM
	pParams->m_Flags		= E_Flags_Linear;
	#endif
	pParams->m_MipOffset	= sizeof(DDS_TEXTURE);
	pParams->m_MipStart		= 0;
	pParams->m_MipEnd		= pParams->m_Levels-1;
	pParams->m_FileSize		= size;
	pParams->m_pFileData	= pData;

	if(pTexture->m_Flags1 & DDSF_FOURCC)
	{
		switch (pTexture->m_FourCC)
		{
		case FOURCC_DXT1:
			pParams->m_Format = E_TextureFormat_DXT1;
			break;
		case FOURCC_DXT3:
			pParams->m_Format = E_TextureFormat_DXT3;
			break;
		case FOURCC_DXT5:
			pParams->m_Format = E_TextureFormat_DXT5;
			break;
		default:
			SAFE_DELETE_ARY(pData);
			LOGERROR(g_Log, "Unsupported compressed texture format %s\n", fileName.Chars());
			return false;
		}
	}
	else
	if(pTexture->m_Flags1 == DDSF_RGBA && pTexture->m_RGBBitCount == 32)
	{
		#if 1
		pParams->m_Format = E_TextureFormat_A8R8G8B8;
		#else
		SAFE_DELETE_ARY(pData);
		LOGERROR(g_Log, "Unsupported uncompressed texture format %s\n", fileName.Chars());
		return false;
		#endif
	}
	else 
	{
		SAFE_DELETE_ARY(pData);
		LOGERROR(g_Log, "Unsupported texture format %s\n", fileName.Chars());
		return false;
	}

	return true;
}

bool LoadTGATextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams)
{
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(fileName);
	if(!pFile)
	{
		LOGERROR(g_Log, "Failed to open file %s\n", fileName.Chars());
		return false;
	}

	U8 header[18];
	pFile->Read(header, sizeof(header));

	if(header[2] != 2)
	{
		SAFE_DELETE_PTR(pFile);
		LOGERROR(g_Log, "File %s is not uncompressed\n", fileName.Chars());
		return false;
	}

	if(header[16] != 32)
	{
		SAFE_DELETE_PTR(pFile);
		LOGERROR(g_Log, "File %s not 32 bit\n", fileName.Chars());
		return false;
	}

	pParams->m_Width		= header[13] * 256 + header[12];
	pParams->m_Height		= header[15] * 256 + header[14];
	pParams->m_Depth		= 1;
	pParams->m_Levels		= 1;
	pParams->m_Format		= E_TextureFormat_A8R8G8B8;
	pParams->m_MipOffset	= 0;
	pParams->m_MipStart		= 0;
	pParams->m_MipEnd		= pParams->m_Levels-1;
	pParams->m_FileSize		= pParams->m_Width*pParams->m_Height*4;
	pParams->m_pFileData	= new U8[pParams->m_FileSize];

	pFile->Read(pParams->m_pFileData, sizeof(U8)*pParams->m_FileSize);

	for(U32 h=0; h<pParams->m_Height/2; h++)
	{
		for(U32 w=0; w<pParams->m_Width; w++)
		{
			#define INDEX(_w, _h) ((((_h)*4)*pParams->m_Width)+((_w)*4))

			U8 r = pParams->m_pFileData[INDEX(w, h)+0];
			U8 g = pParams->m_pFileData[INDEX(w, h)+1];
			U8 b = pParams->m_pFileData[INDEX(w, h)+2];
			U8 a = pParams->m_pFileData[INDEX(w, h)+3];

			pParams->m_pFileData[INDEX(w, h)+0] = pParams->m_pFileData[INDEX(w, (pParams->m_Height-1-h))+0];
			pParams->m_pFileData[INDEX(w, h)+1] = pParams->m_pFileData[INDEX(w, (pParams->m_Height-1-h))+1];
			pParams->m_pFileData[INDEX(w, h)+2] = pParams->m_pFileData[INDEX(w, (pParams->m_Height-1-h))+2];
			pParams->m_pFileData[INDEX(w, h)+3] = pParams->m_pFileData[INDEX(w, (pParams->m_Height-1-h))+3];

			pParams->m_pFileData[INDEX(w, (pParams->m_Height-1-h))+0] = r;
			pParams->m_pFileData[INDEX(w, (pParams->m_Height-1-h))+1] = g;
			pParams->m_pFileData[INDEX(w, (pParams->m_Height-1-h))+2] = b;
			pParams->m_pFileData[INDEX(w, (pParams->m_Height-1-h))+3] = a;
		}
	}

	SAFE_DELETE_PTR(pFile);
	
	return true;
}

bool LoadBMPTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams)
{
	#define BITMAP_MAGIC_NUMBER 19778

	#pragma pack(push,bitmap_data,1)

	struct BitmapFileHeader
	{
		unsigned short bfType;
		unsigned int bfSize;
		unsigned short bfReserved1;
		unsigned short bfReserved2;
		unsigned int bfOffBits;

		void EndianSwap()
		{
			EndianSwap16((U16*)&bfType, 1);
			EndianSwap32((U32*)&bfSize, 1);
			EndianSwap16((U16*)&bfReserved1, 1);
			EndianSwap16((U16*)&bfReserved2, 1);
			EndianSwap32((U32*)&bfOffBits, 1);
		}
	};

	struct BitmapInfoHeader
	{
		unsigned int biSize;
		int biWidth;
		int biHeight;
		unsigned short biPlanes;
		unsigned short biBitCount;
		unsigned int biCompression;
		unsigned int biSizeImage;
		int biXPelsPerMeter;
		int biYPelsPerMeter;
		unsigned int biClrUsed;
		unsigned int biClrImportant;

		void EndianSwap()
		{
			EndianSwap32((U32*)&biSize, 1);
			EndianSwap32((U32*)&biWidth, 1);
			EndianSwap32((U32*)&biHeight, 1);
			EndianSwap32((U32*)&biPlanes, 1);
			EndianSwap32((U32*)&biBitCount, 1);
			EndianSwap32((U32*)&biCompression, 1);
			EndianSwap32((U32*)&biSizeImage, 1);
			EndianSwap32((U32*)&biXPelsPerMeter, 1);
			EndianSwap32((U32*)&biYPelsPerMeter, 1);
			EndianSwap32((U32*)&biClrUsed, 1);
			EndianSwap32((U32*)&biClrImportant, 1);
		}
	};

	#pragma pack(pop,bitmap_data)

	U32 size = 0;
	U8* pData = NULL;
	if(!GetFileData(fileName, size, &pData))
	{
		LOGERROR(g_Log, "Failed to open file %s\n", fileName.Chars());
		return false;
	}

	U32 i = 0;

	BitmapFileHeader* pHeader = (BitmapFileHeader*)&pData[i];
	i += sizeof(BitmapFileHeader);

	BitmapInfoHeader* pInfoHeader = (BitmapInfoHeader*)&pData[i];
	i += sizeof(BitmapInfoHeader);
	
	#if PLATFORM_PS3
	pHeader->EndianSwap();
	pInfoHeader->EndianSwap();
	#endif

	if(pHeader->bfType != BITMAP_MAGIC_NUMBER)
	{
		SAFE_DELETE_ARY(pData);
        LOGERROR(g_Log, "File not in DIB format %s\n", fileName.Chars());
		return false;
    }

	pParams->m_Width		= pInfoHeader->biWidth;
	pParams->m_Height		= pInfoHeader->biHeight;
	pParams->m_Depth		= 1;
	pParams->m_Levels		= 1;
	pParams->m_Format		= E_TextureFormat_A8R8G8B8;
	#if !DEVICE_GCM
	pParams->m_Flags		= E_Flags_Linear;
	#endif
	pParams->m_MipOffset	= 0;
	pParams->m_MipStart		= 0;
	pParams->m_MipEnd		= pParams->m_Levels-1;

	pParams->m_FileSize		= pParams->m_Width*pParams->m_Height*4;
	pParams->m_pFileData	= new U8[pParams->m_FileSize];

	for(U32 h=0; h<pParams->m_Height; h++)
	{
		for(U32 w=0; w<pParams->m_Width; w++)
		{
			#if PLATFORM_PS3
			pParams->m_pFileData[((h*4*pParams->m_Width)+(w*4))+0] = UCHAR_MAX;
			pParams->m_pFileData[((h*4*pParams->m_Width)+(w*4))+1] = pData[(((pParams->m_Height-h)*3*pParams->m_Width)+(w*3))+2];
			pParams->m_pFileData[((h*4*pParams->m_Width)+(w*4))+2] = pData[(((pParams->m_Height-h)*3*pParams->m_Width)+(w*3))+1];
			pParams->m_pFileData[((h*4*pParams->m_Width)+(w*4))+3] = pData[(((pParams->m_Height-h)*3*pParams->m_Width)+(w*3))+0];
			#else
			pParams->m_pFileData[((h*4*pParams->m_Width)+(w*4))+0] = pData[(((pParams->m_Height-h)*3*pParams->m_Width)+(w*3))+0];
			pParams->m_pFileData[((h*4*pParams->m_Width)+(w*4))+1] = pData[(((pParams->m_Height-h)*3*pParams->m_Width)+(w*3))+1];
			pParams->m_pFileData[((h*4*pParams->m_Width)+(w*4))+2] = pData[(((pParams->m_Height-h)*3*pParams->m_Width)+(w*3))+2];
			pParams->m_pFileData[((h*4*pParams->m_Width)+(w*4))+3] = UCHAR_MAX;
			#endif
		}
	}

	SAFE_DELETE_ARY(pData);

	return true;
}

bool LoadJPGTextureFromFile(const Util::C_String& fileName, C_TextureParams* pParams)
{
	return false;
}

bool CreateTexture2DFromFile(const Util::C_String& fileName, C_Texture* pTexture)
{
	C_TextureParams params;

	#if DEVICE_GCM
	bool bTextureRemap = false;
	U32 iTextureRemap = 0;
	#endif
	
	if(fileName.Length() > 3 && fileName[fileName.Length() - 3] == 'd' && fileName[fileName.Length() - 2] == 'd' && fileName[fileName.Length() - 1] == 's')
	{
		if(!LoadDDSTextureFromFile(fileName, &params))
			return false;
	}
	else
	if(fileName.Length() > 3 && fileName[fileName.Length() - 3] == 'b' && fileName[fileName.Length() - 2] == 'm' && fileName[fileName.Length() - 1] == 'p')
	{
		if(!LoadBMPTextureFromFile(fileName, &params))
			return false;
	}
	else
	if(fileName.Length() > 3 && fileName[fileName.Length() - 3] == 't' && fileName[fileName.Length() - 2] == 'g' && fileName[fileName.Length() - 1] == 'a')
	{
		if(!LoadTGATextureFromFile(fileName, &params))
			return false;

		#if DEVICE_GCM
		bTextureRemap = true;
		iTextureRemap = CELL_GCM_REMAP_MODE(
				CELL_GCM_TEXTURE_REMAP_ORDER_XYXY,
				CELL_GCM_TEXTURE_REMAP_FROM_B, CELL_GCM_TEXTURE_REMAP_FROM_G, CELL_GCM_TEXTURE_REMAP_FROM_R, CELL_GCM_TEXTURE_REMAP_FROM_A,
				CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP,  CELL_GCM_TEXTURE_REMAP_REMAP);
		#endif
	}
	else
	{
		LOGERROR(g_Log, "Unsupported 2d texture file extension %s\n", fileName.Chars());
		return false;
	}

	if(!Device()->CreateTexture2D(params.m_Width, params.m_Height, params.m_Levels, params.m_Flags, params.m_Format, pTexture))
	{
		SAFE_DELETE_ARY(params.m_pFileData);
		return false;
	}

	U32 width	= params.m_Width;
	U32 height	= params.m_Height;

	if(!params.m_pFileData)
	{
		Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(fileName);
		if(pFile)
		{
			pFile->SetPosition(params.m_MipOffset);

			for(U32 level=params.m_MipStart; level<=params.m_MipEnd; level++)
			{
				U32 size = 0;

				switch(params.m_Format)
				{
				case E_TextureFormat_DXT1:
					size = ((width+3)/4) * ((height+3)/4)*8;
					break;
				case E_TextureFormat_DXT3:
				case E_TextureFormat_DXT5:
					size = ((width+3)/4) * ((height+3)/4)*16;
					break;
				case E_TextureFormat_A8R8G8B8:
					size = width*height*4;         
					break;
				default:
					assert(0 && "Unsupported texture format");
					break;
				}

				C_LockedRect lockedRect = pTexture->LockRect2D(level);
				if(lockedRect.GetBits())
				{
					pFile->Read(lockedRect.GetBits(), sizeof(U8)*size);
				}
				else
				{
					LOGERROR(g_Log, "Failed to lock mip level %i of file %s\n", level, fileName.Chars());
					return false;
				}
				pTexture->UnlockRect2D(level);

				if(width > 1)
					width >>= 1;
				if(height > 1)
					height >>= 1;
			}

			#if DEVICE_GCM
			if(bTextureRemap)
				pTexture->m_gcmTexture.remap = iTextureRemap;
			#endif

			SAFE_DELETE_PTR(pFile);
		}
		else
		{
			LOGERROR(g_Log, "Failed to open file %s\n", fileName.Chars());
		}
	}
	else
	{
		U8* pData = &params.m_pFileData[params.m_MipOffset];

		for(U32 level=params.m_MipStart; level<=params.m_MipEnd; level++)
		{
			U32 size = 0;
			U8 pitchScale = 0;

			switch(params.m_Format)
			{
			case E_TextureFormat_DXT1:
				size = ((width+3)/4) * ((height+3)/4)*8;
				pitchScale = 4;
				break;
			case E_TextureFormat_DXT3:
			case E_TextureFormat_DXT5:
				size = ((width+3)/4) * ((height+3)/4)*16;
				pitchScale = 4;
				break;
			case E_TextureFormat_A8R8G8B8:
				size = width*height*4;         
				pitchScale = 1;
				break;
			default:
				assert(0 && "Unsupported texture format");
				break;
			}

			C_LockedRect lockedRect = ((C_Texture*)pTexture)->LockRect2D(level);
			if(lockedRect.GetBits())
			{
				memcpy(lockedRect.GetBits(), pData, size);
			}
			((C_Texture*)pTexture)->UnlockRect2D(level);

			pData += size;
			
			if(width > 1)
				width >>= 1;
			if(height > 1)
				height >>= 1;
		}

		#if DEVICE_GCM
		if(bTextureRemap)
			pTexture->m_gcmTexture.remap = iTextureRemap;
		#endif

		SAFE_DELETE_ARY(params.m_pFileData);
	}

	#if TRACK_DISPLAY2_RESOURCES
	C_ResourceInfo* pResourceInfo = Device()->GetTextureResourceInfo(pTexture);
	pResourceInfo->m_Name = fileName;
	pResourceInfo->m_Time = Time::GetCurrentTime();
	#endif

	return true;
}

bool CreateTexture3DFromFile(const Util::C_String& fileName, C_Texture* pTexture)
{
	return false;
}

bool CreateTextureCubeFromFile(const Util::C_String& fileName, C_Texture* pTexture)
{
	return false;
}

bool CloneTexture2D(C_Texture* pSrc, C_Texture* pDst)
{
	BIZ_ASSERT(pSrc);
	BIZ_ASSERT(pSrc->IsCreated());
	BIZ_ASSERT(pDst);
	BIZ_ASSERT(!pDst->IsCreated());

	if(!Device()->CreateTexture2D(pSrc->GetWidth(), pSrc->GetHeight(), pSrc->GetLevels(), pSrc->GetFlags(), pSrc->GetFormat(), pDst))
		return false;
	
	BIZ_ASSERT(pSrc->GetSize() == pDst->GetSize());
	//BIZ_ASSERT(pSrc->GetFlags() == pDst->GetFlags());

#if defined(DEVICE_D3D11)
	// [Blur1PCDX11] D3D11 textures have no CPU address; clone them on the GPU.
	ID3D11Texture2D* pSrcTexture = pSrc->GetGPUTexture2D();
	ID3D11Texture2D* pDstTexture = pDst->GetGPUTexture2D();
	if(!pSrcTexture || !pDstTexture)
		return false;
	ID3D11Device* pDevice = nullptr;
	pSrcTexture->GetDevice(&pDevice);
	ID3D11DeviceContext* pContext = nullptr;
	if(pDevice)
	{
		pDevice->GetImmediateContext(&pContext);
		pDevice->Release();
	}
	if(!pContext)
		return false;
	pContext->CopyResource(pDstTexture, pSrcTexture);
	D3D11ResourceWritten(pDstTexture, pContext); // [Blur1PCDX11]
	pContext->Release();
#else
	memcpy(pDst->GetAddress(), pSrc->GetAddress(), pDst->GetSize());

#endif
	
	return true;
}

//------------------------------------------------------------------------------
// Timings

C_Timing::C_Timing(const Util::C_String& name)
{
	m_Enabled				= false;
	m_Name					= name;
	for(U32 i=0; i<E_Timing_Count; i++)
		m_Time[i]			= 0;

	#if PLATFORM_PS3
	m_TimestampRead			= 1;
	m_TimestampWrite		= 0;
	for(U32 i=0; i<m_TimestampCount; i++)
		m_TimestampIndex[i]	= 0;
	#endif

}

bool C_Timing::Enabled() const
{
	return m_Enabled;
}

void C_Timing::Enabled(const bool enabled)
{
	m_Enabled = enabled;
}

void C_Timing::Update()
{
	m_Time[E_Timing_Prev] = m_Time[E_Timing_Curr];
	
#if PLATFORM_PS3
	U64 oldTime = cellGcmGetTimeStamp(m_TimestampIndex[m_TimestampRead]);
	U64 newTime = cellGcmGetTimeStamp(m_TimestampIndex[m_TimestampRead]+1);

	m_Time[E_Timing_Curr] = newTime-oldTime;

	m_TimestampRead = m_TimestampRead+1 >= m_TimestampCount ? 0 : m_TimestampRead+1;
	m_TimestampWrite = m_TimestampWrite+1 >= m_TimestampCount ? 0 : m_TimestampWrite+1;

	BIZ_ASSERT(m_TimestampRead != m_TimestampWrite);
	BIZ_ASSERT(m_TimestampRead != m_TimestampWrite);
#endif

}

F32 C_Timing::GetTime() const
{
	return (F32)m_Time[E_Timing_Prev] / 1000000.0f;
}

C_TimingManager C_TimingManager::s_Instance;

C_TimingManager::~C_TimingManager()
{
	BIZ_ASSERT(m_Timings.Size() == 0);
}

void C_TimingManager::BeginFrame()
{
}

void C_TimingManager::EndFrame()
{
}

void C_TimingManager::Update()
{
	if(!g_debugDisplay2TimingsEnable)
		return;

	m_Mutex.Wait();

	C_TimingMap::Iterator it = m_Timings.Begin();
	while(it != m_Timings.End())
	{
		C_Timing* pTiming = it->GetValue();
		
		if(pTiming->Enabled())
		{
			pTiming->Update();
		}

		pTiming->Enabled(false); // Will get re-enabled if modified by an event...
		
		it++;
	}

	m_Mutex.Release();
}

C_TimingString::C_TimingString(const Util::C_String80& name, const S32 level) :
	m_Name(name),
	m_Level(level)
{
	m_Key = Btl::Hash(name.Chars());
}

C_TimingString::~C_TimingString()
{
}

MT::ThreadLocalStorage C_TimingEvent::m_Tls;

C_TimingEvent::C_TimingEvent(C_TimingString* pString) :
	m_pString(pString)
{
	if(g_debugDisplay2TimingsEnable)
	{
		m_pParent = (C_TimingEvent*)m_Tls.GetData();
		m_Depth = m_pParent ? m_pParent->m_Depth+1 : 0;
		
		if(!Enabled())
		{
			m_pTiming = NULL;
			return;
		}
		
		U32 key = 0;
		GetKey(key);

		C_TimingMap::Iterator it = C_TimingManager::GetInstance().m_Timings.Find(key);

		if(it == C_TimingManager::GetInstance().m_Timings.End())
		{
			Util::C_String name = "";
			GetName(name);

			Display2::C_TimingManager::GetInstance().m_Mutex.Wait();
			C_TimingManager::GetInstance().m_Timings[key] = new C_Timing(name);
			Display2::C_TimingManager::GetInstance().m_Mutex.Release();

			m_pTiming = C_TimingManager::GetInstance().m_Timings[key];
		}
		else
		{
			m_pTiming = it->GetValue();
		}

		#if PLATFORM_PS3
		m_TimestampIndex = GetNextTimingIndex(2);
		cellGcmSetTimeStamp(gCellGcmCurrentContext, m_TimestampIndex);
		#endif


		m_Tls.SetData(this);

		m_pTiming->m_Enabled = true;
	}
	else
	{
		m_pParent = NULL;
		m_Depth = 0;
		m_pTiming = NULL;
	}
}

C_TimingEvent::~C_TimingEvent()
{
	if(g_debugDisplay2TimingsEnable)
	{
		if(!Enabled())
			return;

		#if PLATFORM_PS3
		m_pTiming->m_TimestampIndex[m_pTiming->m_TimestampWrite] = m_TimestampIndex;
		cellGcmSetTimeStamp(gCellGcmCurrentContext, m_TimestampIndex+1);
		#endif


		m_Tls.SetData(m_pParent);
	}
}

bool C_TimingEvent::Enabled() const
{
	BIZ_ASSERT(m_pString);
	return m_Depth < g_debugDisplay2TimingsDepth && m_pString->m_Level < g_debugDisplay2TimingsLevel ? true : false;
}

void C_TimingEvent::GetName(Util::C_String& name)
{
	if(m_pParent)
	{
		m_pParent->GetName(name);
	}

	if(m_pString)
	{
		if(m_pParent)
		{
			name += ".";
		}

		name += m_pString->m_Name.Chars();
	}
}

void C_TimingEvent::GetKey(U32& key)
{
	if(m_pString)
	{
		key = Btl::Hash(m_pString->m_Key, key);
	}

	if(m_pParent)
	{
		m_pParent->GetKey(key);
	}
}

} // namespace Display2
