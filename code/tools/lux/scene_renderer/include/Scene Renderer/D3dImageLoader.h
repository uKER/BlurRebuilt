#ifndef D3DIMAGELOADER_H
#define D3DIMAGELOADER_H

namespace SRD
{
	class D3dImageLoader 
	{
	public:
		D3dImageLoader();
		~D3dImageLoader();

		// Note that both of these load functions allocate the necessary memory to store the image data.
		// This memory must be explicity released by the user, no management is provided.
		// For texture managment use the higher level D3DTextureManager class.
		HRESULT					LoadToSurface(
			const std::string& fileName, 
			IDirect3DDevice9* pDevice,
			_D3DFORMAT &format,
			IDirect3DSurface9** ppSurface);

		HRESULT					LoadToMemory(
			const std::string& fileName,
			IDirect3DDevice9* pDevice,
			_D3DFORMAT &format,
			U8* pRawImageData);

		HRESULT					LoadToTexture(
			const std::string& fileName,
			IDirect3DDevice9* pDevice,
			_D3DFORMAT &format,
			IDirect3DTexture9** ppTexture);

		int FindThisTexture(char *psz, const char *fileName);
	};
}


#endif