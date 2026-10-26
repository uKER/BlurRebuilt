#include "stdafx.h"

#include <scene_renderer/D3dImageLoader.h>

using namespace SRD;

D3dImageLoader::D3dImageLoader()
{
}


D3dImageLoader::~D3dImageLoader()
{
}

// find this texture in various places
int D3dImageLoader::FindThisTexture(char *psz, const char *fileName)
{
	for (int i=0; i<2; i++)
	{
		char buffer[512];
		char buffer2[512];
		char *p;

		strcpy(buffer2,fileName);

		// first try .dds file
		if (i==0)
		{
			p = strrchr(buffer2,'.');
			if (!p)
			{
				// no file extension wtf?
				strcpy(psz, fileName);
				return 1;
			}
			strcpy(p,".dds");
		}

		if (_access(buffer2,0) == -1)
		{
			// try to load it from our local directory
			p = strrchr(buffer2,'/');
			if (p)
				strcpy(buffer2,p+1);
			else
			{
				p = strrchr(buffer2,'\\');
				if (p)
					strcpy(buffer2,p+1);
			}

			sprintf(buffer,"d:/lightmaptest/%s",buffer2);
			strcpy(psz, buffer);
		}
		else
		{
			strcpy(psz, buffer2);
		}

		if (_access(psz,0) != -1)
		{
			// found it - return
			return i;
		}
	}

	// couldn't find it
	strcpy(psz, fileName);
	return 1;
}

// Note that both of these load functions allocate the necessary memory to store the image data.
// This memory must be explicity released by the user, no management is provided.
// For texture managment use the higher level D3DTextureManager class.
HRESULT D3dImageLoader::LoadToSurface(
	const std::string& fileName,
	IDirect3DDevice9* pDevice,
	_D3DFORMAT &format,
	IDirect3DSurface9** surface) // todo
{
	HRESULT hr = D3D_OK;
	return hr;
}

HRESULT	D3dImageLoader::LoadToTexture(
	const std::string& fileName,
	IDirect3DDevice9* pDevice,
	_D3DFORMAT &format,
	IDirect3DTexture9** ppTexture)
{
#ifdef KEN
	{
		char buf[1024];
		_snprintf(buf, sizeof(buf), "Loading file %s\r\n", fileName.c_str() );
		OutputDebugString(buf);
	}
#endif

	HRESULT hr = D3D_OK;

	char psz[512];
	if (FindThisTexture(psz,fileName.c_str()) == 0)
	{
		// load a DDS
		D3DXCreateTextureFromFileEx( pDevice, psz, 
			D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
			D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, ppTexture );
		return E_FAIL;
	}

	// load a BMP
	return D3DXCreateTextureFromFileEx( pDevice, psz, 
		D3DX_DEFAULT, D3DX_DEFAULT, D3DX_DEFAULT, 0, D3DFMT_UNKNOWN, D3DPOOL_MANAGED,
		D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, ppTexture );

/*
	BTCImageData* pImageData;
	BTCImageObject imageObject;
	long codecId = imageObject.Load(psz);
	if (codecId == -1)
	{
		return E_FAIL;
	}

	pImageData = imageObject.GetObjectDataPtr();

	if (!strstr(fileName.c_str(),"lightmap"))
	{
		if ((pImageData->GetWidth() > 128) || (pImageData->GetHeight() > 128))
		{
			pImageData->Resize(128,128);
		}
	}

	S32 bpp = pImageData->GetBitsPerPixel();
	if ((bpp < 24) || (bpp > 32))
	{
		// we do not support this number of bits per pixel
		return E_FAIL;
	}

	_D3DFORMAT textureFormat;
	_D3DFORMAT surfaceFormat;

	U32 bytesPerPixel;
	if (pImageData->GetBitsPerPixel() == 24) 
	{
		//24bpp
		textureFormat = D3DFMT_X8R8G8B8;
		surfaceFormat = D3DFMT_R8G8B8;
		bytesPerPixel = 3;
	} 
	else
	{
		//32bpp
		textureFormat = D3DFMT_A8R8G8B8;
		surfaceFormat = D3DFMT_A8R8G8B8;
		bytesPerPixel = 4;
	}

	U32 imageSize =pImageData->GetWidth() * pImageData->GetHeight() * bytesPerPixel;

	U8 *pRawImageData = new U8[imageSize];
	ZeroMemory(pRawImageData, sizeof(*pRawImageData));

	U32 index = 0;
	for (U32 y = 0; y < (U32)pImageData->GetHeight(); y++) 
	{
		for (U32 x = 0; x < (U32)pImageData->GetWidth(); x++) 
		{
			if (pImageData->GetBitsPerPixel() == 32) 
			{
				// 24bit colour with 8bit alpha channel
				U8* pRGBA = pImageData->GetBits(x,y);
				pRawImageData[index] = pRGBA[0];
				pRawImageData[index+1] = pRGBA[1];
				pRawImageData[index+2] = pRGBA[2];
				pRawImageData[index+3] = pRGBA[3];

				//Increment index by 4 bytes
				index += 4;
			}
			else
			{
				// 24bit colour
				U8* pRGB = pImageData->GetBits(x,y);
				pRawImageData[index] = pRGB[0];
				pRawImageData[index+1] = pRGB[1];
				pRawImageData[index+2] = pRGB[2];

				//Increment index by 3 bytes
				index += 3;
			}
		}
	}

	RECT srcRect;
	srcRect.top = 0;
	srcRect.bottom = pImageData->GetHeight();
	srcRect.left = 0;
	srcRect.right = pImageData->GetWidth();

	hr = pDevice->
		CreateTexture(
		pImageData->GetWidth(),
		pImageData->GetHeight(),
		0,
		D3DUSAGE_AUTOGENMIPMAP,
		textureFormat,
		D3DPOOL_MANAGED,
		ppTexture,
		NULL);

	IDirect3DSurface9* pRenderSurface;
	(*ppTexture)->GetSurfaceLevel(0,&pRenderSurface);

	hr =  D3DXLoadSurfaceFromMemory( 
		pRenderSurface,					// LPDIRECT3DSURFACE9 pDestSurface,
		0,								// CONST PALETTEENTRY *pDestPalette
		0,								// CONST RECT *pDestRect
		pRawImageData,					// LPCVOID pSrcMemory
		surfaceFormat,					// D3DFORMAT SrcFormat
		bytesPerPixel*pImageData->GetWidth(),			// UINT SrcPitch
		0,								// CONST PALETTEENTRY *pSrcPalette
		&srcRect,						// CONST RECT *pSrcRect
		D3DX_DEFAULT,					// DWORD Filter
		0								// D3DCOLOR ColorKey
	);

	pRenderSurface->Release();

	if (pRawImageData != 0)
	{
		delete[] pRawImageData;
	}

	return hr;
*/
}

HRESULT D3dImageLoader::LoadToMemory (
	const std::string& fileName, 
	IDirect3DDevice9* pDevice,
	_D3DFORMAT &format,
	U8 *rawImageData)
{
	HRESULT hr = D3D_OK;

	return hr;
}


