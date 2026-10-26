#include <lighting and pvs application/stdafx.h>

//#include "../include/common/DebugManager.h"
#include <lighting and pvs application/D3DTextureManager.h>
#include <lighting and pvs application/GeneralUtils.h>

#include <Magick++.h>
#include <io.h>


D3DTextureManager *D3DTextureManager::m_singleton = 0;


D3DTextureManager::D3DTextureManager()
{
}


D3DTextureManager::~D3DTextureManager()
{
}


D3DTextureManager &D3DTextureManager::GetSingleton()
{
	if (!m_singleton) 
	{
		m_singleton = new D3DTextureManager;
		Init();
	}

	return *m_singleton;
}


void D3DTextureManager::Create(IDirect3DDevice9 *pD3DDevice)
{
	m_singleton->m_pD3DDevice = pD3DDevice;
}


void D3DTextureManager::Destroy()
{
	// force deletion of remaining textures
	std::vector<D3DTextureInfo>::iterator it = m_singleton->m_d3dTextureInfo.begin();
	while (it != m_singleton->m_d3dTextureInfo.end()) 
	{
		SAFE_RELEASE(it->pTexture);
		it++;
	}

	SAFE_DELETE(m_singleton);
}


HRESULT D3DTextureManager::Load(const std::string fileName, IDirect3DTexture9 **ppTexture)
{
	HRESULT hr = D3D_OK;

	if (_access(fileName.c_str(),0) == -1) {
		// file does not exist!
//		DEBUGMANAGER << "File " << fileName << " does not exist!";
		return E_FAIL;
	}

	std::vector<D3DTextureInfo>::iterator it = m_singleton->m_d3dTextureInfo.begin();
	while (it != m_singleton->m_d3dTextureInfo.end()) {
		if (it->fileName == fileName) {
			// texture has already been loaded, don't load it again
			(*ppTexture) = it->pTexture;
			// increase reference count
			it->referenceCount++;
			return hr;
		}
		it++;
	}

	// If we've got here, then there's some loading to be done
	D3DTextureInfo textureInfo;
	textureInfo.fileName = fileName;

	Magick::Image image;

	try {
		image.read(fileName);
	}
	catch (Magick::Exception &error_ ) {
		#ifdef _DEBUG
///		std::cout << error_ << std::endl;
		#endif
		return E_FAIL;
	}

	U32 bytesPerPixel;
	_D3DFORMAT textureFormat;
	_D3DFORMAT surfaceFormat;

	if (image.matte() == 0) {
		bytesPerPixel = 3;
		textureFormat = D3DFMT_X8R8G8B8;
		surfaceFormat = D3DFMT_R8G8B8;
	} else {
		textureFormat = D3DFMT_A8R8G8B8;
		surfaceFormat = D3DFMT_A8R8G8B8;
		bytesPerPixel = 4;
	}
	U32 columns = image.columns();
	U32 rows = image.rows();
	U32 imageSize = columns * rows * bytesPerPixel;
	unsigned char *rawImageData = new unsigned char[imageSize];
	ZeroMemory(rawImageData, sizeof(*rawImageData));
	Magick::PixelPacket *pixel_cache = image.getPixels(0, 0, columns, rows); 

	U32 row = 0;
	U32 column = 0;
	U32 index = 0;
	for (row = 0; row < image.rows(); row++) {
		for (column = 0; column < columns; column++) {
			Magick::PixelPacket *pixel = pixel_cache + column + columns * row;
			//Magick::PixelPacket *pixel = pixel_cache + columns * column + row;
			Magick::ColorRGB pixelColour = *pixel;
			double red = pixelColour.red() * 255;
			double green = pixelColour.green() * 255;
			double blue = pixelColour.blue() * 255;
			double alpha;
			if (bytesPerPixel == 4) {
				alpha =(1-pixelColour.alpha()) * 255;
			}

			if (bytesPerPixel == 4) {
				// 24bit colour with 8bit alpha channel
				rawImageData[index] = (unsigned char)blue;
				rawImageData[index+1] = (unsigned char)green;
				rawImageData[index+2] = (unsigned char)red;
				rawImageData[index+3] = (unsigned char)alpha;
				index+=4;
			} else {
				// 24bit colour
				rawImageData[index] = (unsigned char)blue;
				rawImageData[index+1] = (unsigned char)green;
				rawImageData[index+2] = (unsigned char)red;
				index+=3;
			}
		}
	}

	RECT srcRect;
	srcRect.top = 0;
	srcRect.bottom = rows;
	srcRect.left = 0;
	srcRect.right = columns;

	m_pD3DDevice->
		CreateTexture(
		columns,
		rows,
		1,
		D3DUSAGE_RENDERTARGET,
		textureFormat,
		D3DPOOL_DEFAULT,
		ppTexture,
		NULL);

	IDirect3DSurface9* pRenderSurface;
	(*ppTexture)->GetSurfaceLevel(0,&pRenderSurface);

	hr =  D3DXLoadSurfaceFromMemory( 
		pRenderSurface,					// LPDIRECT3DSURFACE9 pDestSurface,
		0,								// CONST PALETTEENTRY *pDestPalette
		0,								// CONST RECT *pDestRect
		rawImageData,					// LPCVOID pSrcMemory
		surfaceFormat,					// D3DFORMAT SrcFormat
		bytesPerPixel*columns,			// UINT SrcPitch
		0,								// CONST PALETTEENTRY *pSrcPalette
		&srcRect,						// CONST RECT *pSrcRect
		D3DX_DEFAULT,					// DWORD Filter
		0								// D3DCOLOR ColorKey
	);
	if (FAILED(hr)) {
		return hr;
	}

	SAFE_DELETE_ARRAY(rawImageData);

	//Add to our STL vector
	textureInfo.referenceCount = 1;
	textureInfo.pTexture = *ppTexture;
	m_singleton->m_d3dTextureInfo.push_back(textureInfo);

	return hr;
}


HRESULT D3DTextureManager::Unload(IDirect3DTexture9 *pTexture)
{
	HRESULT hr = D3D_OK;

	std::vector<D3DTextureInfo>::iterator it = m_singleton->m_d3dTextureInfo.begin();
	while (it != m_singleton->m_d3dTextureInfo.end()) {
		if (it->pTexture == pTexture) {
			if (it->referenceCount > 1) {
				// decrease reference count
				it->referenceCount--;
				return hr;
			} else {
				// nobody else needs this texture, lets get rid of it.
				SAFE_RELEASE(it->pTexture);
				m_singleton->m_d3dTextureInfo.erase(it);
				return hr;
			}
		}
		it++;
	}

	return E_FAIL;
}


void D3DTextureManager::Init()
{
}
