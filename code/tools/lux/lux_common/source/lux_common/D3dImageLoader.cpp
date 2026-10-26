#include <lighting and pvs application/stdafx.h>

#include <lighting and pvs application/D3DImageLoader.h>
#include <lighting and pvs application/GeneralUtils.h>

#include <Magick++.h>

D3DImageLoader *D3DImageLoader::m_singleton = 0;


D3DImageLoader::D3DImageLoader()
{
}


D3DImageLoader::~D3DImageLoader()
{
}


D3DImageLoader &D3DImageLoader::GetSingleton()
{
	if (!m_singleton) {
		m_singleton = new D3DImageLoader;
		Init();
	}

	return *m_singleton;
}


void D3DImageLoader::Create()
{
}


void D3DImageLoader::Destroy()
{
	SAFE_DELETE(m_singleton);
}


// Note that both of these load functions allocate the necessary memory to store the image data.
// This memory must be explicity released by the user, no management is provided.
// For texture managment use the higher level D3DTextureManager class.
HRESULT D3DImageLoader::LoadToSurface(
	const std::string fileName,
	_D3DFORMAT &format,
	IDirect3DSurface9* surface) // todo
{
	HRESULT hr = D3D_OK;
	return hr;
}


HRESULT D3DImageLoader::LoadToMemory (
	const std::string fileName, 
	_D3DFORMAT &format,
	unsigned char *rawImageData)
{
	HRESULT hr = D3D_OK;

	Magick::Image image;

	try {
		image.read(fileName);
	}
	catch (Magick::Exception &error_ ) {
		return FALSE;
	}

	#ifdef _DEBUG
//		std::cout << error << std::endl;
	#endif

//		Magick::
	
	Magick::PixelPacket *pixel_cache = image.getPixels(0,0,1024,1024); 

	int bytesPerPixel;
	if (image.matte == 0) {
		bytesPerPixel = 3;
		format = D3DFMT_X8R8G8B8;
	} else {
		bytesPerPixel = 4;
	}
	int imageSize = image.columns * image.rows * bytesPerPixel;
	unsigned char *rawImageData = new unsigned char[imageSize];
	ZeroMemory(rawImageData, sizeof(*rawImageData));

	int i = 0;
	int row = 0;
	int column = 0;
	int index = 0;
	for (row = 0; row < 1024; row++) {
		for (column = 0; column < 1024; column++) {
			Magick::PixelPacket *pixel = pixel_cache+1024*column+row;
			Magick::ColorRGB pixelColour = *pixel;
			double red = pixelColour.red() * 255;
			double green = pixelColour.green() * 255;
			double blue = pixelColour.blue() * 255;
			if (bytesPerPixel == 4) {
				double alpha = pixelColour.alpha() * 255;
			}
			rawImageData[index] = (unsigned char)blue;
			rawImageData[index+1] = (unsigned char)green;
			rawImageData[index+2] = (unsigned char)red;

			if (bytesPerPixel == 4) {
				// 24bit colour with 8bit alpha channel
				rawImageData[index+1] = (unsigned char)alpha;
				rawImageData[index+2] = (unsigned char)blue;
				rawImageData[index+3] = (unsigned char)green;
				rawImageData[index+4] = (unsigned char)red;
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

	return hr;
}


void D3DImageLoader::Init()
{
}
