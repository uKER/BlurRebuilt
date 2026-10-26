#include <nexus_render/stdafx.h>

#include <nexus_render/D3DImageLoader.h>

//Include the Abstract Texture Transfer Kit
#include <attk/attk.h>


using namespace NexusRender;

D3DImageLoader::D3DImageLoader()
{
}


D3DImageLoader::~D3DImageLoader()
{
}


HRESULT	D3DImageLoader::LoadToTextureFromDisk(
	IDirect3DTexture9** ppTexture,
	_D3DFORMAT &format,
	U32& width,
	U32& height,
	const std::string& fileName,
	S32 sizeCap,
	IDirect3DDevice9* pDevice)
{
	HRESULT hr = D3D_OK;

	const char *psz;
	if (_access(fileName.c_str(),0) == -1) 
	{
		return E_FAIL;	// file does not exist!
	}
	else
		psz = fileName.c_str();

	Attk::C_Status status;
	Attk::C_Image image;
	AttkUt::C_ImageResize imageResize;

	status = image.Read(psz);
	if (!status) 
		return E_FAIL;

	if (sizeCap != -1)
	{
		if ((image.GetWidth() > (U32)sizeCap) || (image.GetHeight() > (U32)sizeCap))
		{
			status = imageResize.Resize(image, sizeCap, sizeCap, 1);
			if (!status)
				return E_FAIL;
		}
	}
	width = image.GetWidth();
	height = image.GetHeight();
	
	S32 bpp = image.GetBitsPerPixel();
	if ((bpp < 24) || (bpp > 32))
	{
		// we do not support this number of bits per pixel
		return E_FAIL;
	}

	_D3DFORMAT textureFormat;
	_D3DFORMAT surfaceFormat;

	U32 bytesPerPixel;
	if (image.GetBitsPerPixel() == 24) 
	{
		//24bpp
		textureFormat = D3DFMT_X8R8G8B8;
		surfaceFormat = D3DFMT_R8G8B8;
		bytesPerPixel = 3;
//		status = image.ChangePixelFormat(Attk::E_PixelFormat_R8G8B8U);
		if (!status)
			return E_FAIL;
	} 
	else
	{
		//32bpp
		textureFormat = D3DFMT_A8R8G8B8;
		surfaceFormat = D3DFMT_A8R8G8B8;
		bytesPerPixel = 4;
//		status = image.ChangePixelFormat(Attk::E_PixelFormat_R8G8B8A8U);
		if (!status)
			return E_FAIL;
	}

	U32 imageSize = image.GetWidth() * image.GetHeight() * bytesPerPixel;

	U8 *pRawImageData = new U8[imageSize];
	//Copy image data.
	memcpy(pRawImageData, image.GetPixelDataPtr(), imageSize);
	U32 test = 0;

	RECT srcRect;
	srcRect.top = 0;
	srcRect.bottom = image.GetHeight();
	srcRect.left = 0;
	srcRect.right = image.GetWidth();

	hr = pDevice->
		CreateTexture(
		image.GetWidth(),
		image.GetHeight(),
		3,
		0,
//		D3DUSAGE_AUTOGENMIPMAP,
		textureFormat,
		D3DPOOL_MANAGED,
		ppTexture,
		NULL);
	if (FAILED(hr))
	{
		return hr;
	}

//	(*ppTexture)->GenerateMipSubLevels();
	IDirect3DSurface9* pRenderSurface;
	(*ppTexture)->GetSurfaceLevel(0,&pRenderSurface);

	hr =  D3DXLoadSurfaceFromMemory( 
		pRenderSurface,					// LPDIRECT3DSURFACE9 pDestSurface,
		0,								// CONST PALETTEENTRY *pDestPalette
		0,								// CONST RECT *pDestRect
		pRawImageData,					// LPCVOID pSrcMemory
		surfaceFormat,					// D3DFORMAT SrcFormat
		bytesPerPixel*image.GetWidth(),	// UINT SrcPitch
		0,								// CONST PALETTEENTRY *pSrcPalette
		&srcRect,						// CONST RECT *pSrcRect
		D3DX_DEFAULT,					// DWORD Filter
		0								// D3DCOLOR ColorKey
	);
	if (FAILED(hr))
	{
		return hr;
	}
	pRenderSurface->Release();

	hr = D3DXFilterTexture((*ppTexture), 0, 0, D3DX_FILTER_BOX);
	if (FAILED(hr))
	{
		return hr;
	}

	if (pRawImageData != 0)
	{
		delete[] pRawImageData;
	}

	return hr;
}

HRESULT	D3DImageLoader::LoadToTextureFromMemory(
	U8* pRawImageData,
	U32 width,
	U32 height,
	U32 bytesPerPixel,
	IDirect3DDevice9* pDevice,
	_D3DFORMAT &format,
	IDirect3DTexture9** ppTexture)
{
	HRESULT hr = D3D_OK;

	_D3DFORMAT textureFormat;
	_D3DFORMAT surfaceFormat;

	if (bytesPerPixel == 3) 
	{
		//24bpp
		textureFormat = D3DFMT_X8R8G8B8;
		surfaceFormat = D3DFMT_R8G8B8;
	} 
	else
	{
		//32bpp
		textureFormat = D3DFMT_A8R8G8B8;
		surfaceFormat = D3DFMT_A8R8G8B8;
	}


	U32 imageSize = width * height * bytesPerPixel;

	RECT srcRect;
	srcRect.top = 0;
	srcRect.bottom = height;
	srcRect.left = 0;
	srcRect.right = width;

	//Don't waste our time with mip-map generation for a 1x1 texture.
	if (width + height == 2)
	{
		hr = pDevice->
			CreateTexture(
			width,
			height,
			0,
			0,
			textureFormat,
			D3DPOOL_MANAGED,
			ppTexture,
			NULL);
	}
	else
	{
		hr = pDevice->
			CreateTexture(
			width,
			height,
			0,
			D3DUSAGE_AUTOGENMIPMAP,
			textureFormat,
			D3DPOOL_MANAGED,
			ppTexture,
			NULL);
	}
	if (FAILED(hr))
	{
		return hr;
	}

	IDirect3DSurface9* pRenderSurface;
	(*ppTexture)->GetSurfaceLevel(0,&pRenderSurface);

	hr =  D3DXLoadSurfaceFromMemory( 
		pRenderSurface,					// LPDIRECT3DSURFACE9 pDestSurface,
		0,								// CONST PALETTEENTRY *pDestPalette
		0,								// CONST RECT *pDestRect
		pRawImageData,					// LPCVOID pSrcMemory
		surfaceFormat,					// D3DFORMAT SrcFormat
		bytesPerPixel*width,			// UINT SrcPitch
		0,								// CONST PALETTEENTRY *pSrcPalette
		&srcRect,						// CONST RECT *pSrcRect
		D3DX_DEFAULT,					// DWORD Filter
		0								// D3DCOLOR ColorKey
	);
	if (FAILED(hr))
	{
		return hr;
	}
	pRenderSurface->Release();

	return hr;
}

HRESULT D3DImageLoader::LoadToMemoryFromDisk(
	const std::string& fileName, 
	S32 sizeCap,
	IDirect3DDevice9* pDevice,
	_D3DFORMAT &format,
	U8 *rawImageData)
{
	HRESULT hr = D3D_OK;

	return hr;
}


