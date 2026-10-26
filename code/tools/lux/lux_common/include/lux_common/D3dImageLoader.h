#ifndef D3DIMAGELOADER_H
#define D3DIMAGELOADER_H

#include <D3DX9.h>

#define D3DIMAGELOADER	D3DImageLoader::GetSingleton()

// Medium level image loader.
//
// Level overview
// Lowest level = image loading API, namely ImageMagick
// This level = calls API to load image
// Higher level = texture management services from D3DTextureManager

class D3DImageLoader 
{
public:
	D3DImageLoader();
	~D3DImageLoader();
	static D3DImageLoader &GetSingleton();
	static void Create();
	static void Destroy();
	// Note that both of these load functions allocate the necessary memory to store the image data.
	// This memory must be explicity released by the user, no management is provided.
	// For texture managment use the higher level D3DTextureManager class.
	HRESULT LoadToSurface(
		const std::string fileName, 
		_D3DFORMAT &format,
		IDirect3DSurface9* surface); // todo
	HRESULT LoadToMemory (
		const std::string fileName,
		_D3DFORMAT &format,
		unsigned char *rawImageData);

private:	
	static void Init();
	static D3DImageLoader *m_singleton;
};


#endif