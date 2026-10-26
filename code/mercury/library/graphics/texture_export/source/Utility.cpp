#include "stdafx.h"

#include "texture_export/utility.h"
#include "texture_export/texture.h"
#include <attk/attk.h>
#include <il/DevIL.h>

#include "ATI_Compress/ATI_Compress.h"
#include "texture_export/converters/atidxt.h"

namespace TextureExport
{

Debug::Logger &g_Log = Debug::GetLogger("graphics.texture_export");

ActTools::Texture *LoadTexture(const char *pFilename)
{
	DevIL::Image ilImage;

	if (!ilImage.Load(pFilename))
		return NULL;

	ILuint ilImages = ilImage.NumImages()+1;
	ILuint ilMipLevels = ilImage.NumMipmaps() + 1;
	ILuint ilWidth = ilImage.GetWidth();
	ILuint ilHeight = ilImage.GetHeight();
	ILuint ilDepth = ilImage.GetDepth();

	ILenum ilTargetFormat = IL_BGRA;
	ILenum ilTargetType = IL_UNSIGNED_BYTE;

	ActTools::TexelFormat *pTexelFormat = new ActTools::A8R8G8B8;

	ATI_TC_FORMAT decompressFormat = ATI_TC_FORMAT_Unknown;

	ILenum ilSourceFormat = IL_BGRA;
	{
		ILenum ilDXTFormat = ilImage.GetDXTFormat();
		//keep mipmapped cubemaps compressed as these will most likely have been
		//created using the special dxt compression in ATI's CubeMapGen tool
		bool keepCompressed =	(ilImages == 6 && ilWidth == ilHeight) && ilMipLevels > 1;
		switch (ilDXTFormat)
		{
		case IL_DXT1:			
			ilSourceFormat = IL_DXT1;
			decompressFormat = ATI_TC_FORMAT_DXT1;
			if (keepCompressed)
			{
				ilTargetFormat = IL_DXT1;
				delete pTexelFormat;
				pTexelFormat = AtiDXT::GetTexelFormat(ATIDXT_DXT1);
			}
			break;

		case IL_DXT3:
			ilSourceFormat = IL_DXT3;
			decompressFormat = ATI_TC_FORMAT_DXT3;
			if (keepCompressed)
			{
				ilTargetFormat = IL_DXT3;
				delete pTexelFormat;
				pTexelFormat = AtiDXT::GetTexelFormat(ATIDXT_DXT3);
			}
			break;

		case IL_DXT5:
			ilSourceFormat = IL_DXT5;
			decompressFormat = ATI_TC_FORMAT_DXT5;
			if (keepCompressed)
			{
				ilTargetFormat = IL_DXT5;
				delete pTexelFormat;
				pTexelFormat = AtiDXT::GetTexelFormat(ATIDXT_DXT5);
			}
			break;
		};

		//(from Attk) DevIL may screw up and not understand it should be using 2 bytes per pixel...
		if (ilTargetType == IL_UNSIGNED_BYTE && ilImage.GetBpp() == 2)
			ilTargetType = IL_UNSIGNED_SHORT;
	}



	ActTools::Texture *pTex = new ActTools::Texture;
	

	if (ilImages == 6 && ilWidth == ilHeight)
	{		
		pTex->CreateCubeTexture(pTexelFormat, ilWidth, ilMipLevels);
	}
	else if (ilImages > 1)
	{
		//otao: array texture ?
		pTex->CreateVolumeTexture(pTexelFormat, ilWidth, ilHeight, ilImages, 1);
		ilMipLevels = 1;
	}
	else
		pTex->CreateTexture(pTexelFormat, ilWidth, ilHeight, ilMipLevels);

	for (U32 i=0; i<ilImages; i++)
	{
		for (U32 j=0; j<ilMipLevels; ++j)
		{
			ilBindImage(ilImage.GetId());
			ilImage.ActiveImage(i);
			ilImage.ActiveMipmap(j);

			ilWidth = ilImage.GetWidth();
			ilHeight = ilImage.GetHeight();

			ActTools::Surface *pSurface = pTex->GetMipmap(i,j);
			pSurface->Create(ilWidth,ilHeight);
			U32 *pTexels = (U32*)pSurface->GetTexels();

			if (ilSourceFormat == IL_DXT1 ||
				ilSourceFormat == IL_DXT3 ||
				ilSourceFormat == IL_DXT5)
			{
				U32 requiredSize =  ilImage.GetDXTCData(NULL, 0, ilTargetFormat);
				U32 reservedSize = pSurface->GetSize();

				if (ilSourceFormat == ilTargetFormat && decompressFormat != ATI_TC_FORMAT_Unknown)
				{
					//keep compressed
					ILuint copied = ilImage.GetDXTCData(pTexels, reservedSize, ilSourceFormat);
				}
				else
				{
					//The implicit decompression performed by DevIL (by calling ilCopyPixels) is crap.
					//(introduces a green hue - they're possibly using an rgb565 buffer for decompression)
					//Copy raw compressed data into our own buffer and do the decompression ourselves

					assert(ilTargetFormat == IL_BGRA);

					ActTools::Texture tmpBuffer;
					U8 *pBuffer = new U8[reservedSize];
					ILuint copied = ilImage.GetDXTCData(pBuffer, reservedSize, ilSourceFormat);

					ATI_TC_Texture src;
					src.dwSize = sizeof(src);
					src.dwWidth = ilWidth;
					src.dwHeight = ilHeight;
					src.dwPitch = 0;
					src.format = decompressFormat;
					src.dwDataSize = ATI_TC_CalculateBufferSize( &src );
					src.pData = (ATI_TC_BYTE*)pBuffer;

					ATI_TC_Texture dst;
					dst.dwSize = sizeof(dst);
					dst.dwWidth = ilWidth;
					dst.dwHeight = ilHeight;
					dst.dwPitch = pSurface->GetFormat()->Bytes(ilWidth, 1);
					dst.format = ATI_TC_FORMAT_ARGB_8888;
					dst.dwDataSize = ATI_TC_CalculateBufferSize( &dst );
					dst.pData = (ATI_TC_BYTE*)pTexels;	

					ATI_TC_CompressOptions options = { 0 };
					options.dwSize = sizeof(options);
					ATI_TC_ConvertTexture( &src, &dst, &options, NULL, NULL, NULL );

					delete[] pBuffer;
				}
			}
			else
			{
				ilCopyPixels(
					0,
					0,
					0,
					ilWidth,
					ilHeight,
					1,
					ilTargetFormat,
					ilTargetType,
					pTexels);
			}
		}
	}

	delete pTexelFormat;
	return pTex;
}

// Convert a 2d Attk::C_Image to an ActTools::Texture
// the returned texture is in A8R8G8B8 format (which is actually R8G8B8A8 in ATTK speak)
ActTools::Texture *TextureFromAttkImage(Attk::C_Image *pImage)
{
	int w = pImage->GetWidth();
	int h = pImage->GetHeight();
	ActTools::Texture *pTexture = new ActTools::Texture();
	ActTools::A8R8G8B8 texelFormat;
	pTexture->CreateTexture(&texelFormat, w, h, 1);
	ActTools::Surface *pSurface = pTexture->GetMipmap(0,0);
	pSurface->Create(w,h);
	U32 *pTexels = (U32*)pSurface->GetTexels();
	for (int y=0; y<h; y++)
	{
		for (int x=0; x<w; x++)
		{
			Attk::C_PixelR32G32B32A32F pixel = pImage->GetPixel(x,y);
			Attk::C_PixelB8G8R8A8U pixel32;
			pixel32.Set(pixel);
			pTexels[x+y*w] = pixel32.data;
		}
	}

	return pTexture;
}

// Convert a 2d Attk::C_Image to an ActTools::Texture
// the returned texture is in A32R32G32B32F format
ActTools::Texture *TextureFromAttkImageF(Attk::C_Image *pImage)
{
	int w = pImage->GetWidth();
	int h = pImage->GetHeight();
	ActTools::Texture *pTexture = new ActTools::Texture();
	ActTools::A32R32G32B32F texelFormat;
	pTexture->CreateTexture(&texelFormat, w, h, 1);
	ActTools::Surface *pSurface = pTexture->GetMipmap(0,0);
	pSurface->Create(w,h);
	F32 *pTexels = (F32*)pSurface->GetTexels();
	for (int y=0; y<h; y++)
	{
		for (int x=0; x<w; x++)
		{
			Attk::C_PixelR32G32B32A32F p = pImage->GetPixel(x, y);
			*pTexels++ = p.alpha;
			*pTexels++ = p.red;
			*pTexels++ = p.green;
			*pTexels++ = p.blue;
		}
	}

	return pTexture;
}

// Convert a ActTools::Texture to an Attk::C_Image
Attk::C_Image *AttkImageFromTexture(ActTools::Texture *pInTexture)
{
	Attk::C_Image *pImage = new Attk::C_Image();

	// make a copy of the input texture and convert it to A8B8G8R8
	ActTools::A8R8G8B8 texelFormat;
	ActTools::Texture *pTexture = new ActTools::Texture;
	pTexture->CopyFrom(*pInTexture);
	pTexture->ConvertTo(&texelFormat);

	// use the resulting texture data to create an Attk image
	ActTools::Surface *pSurface = pTexture->GetMipmap(0,0);
	pImage->NewPixelData(pTexture->GetWidth(), pTexture->GetHeight(), pTexture->GetDepth(), Attk::E_PixelFormat_B8G8R8A8U, pSurface->GetTexels());
	delete pTexture;

	return pImage;
}

}
