#ifndef D3DIMAGELOADER_H
#define D3DIMAGELOADER_H

namespace NexusRender
{
	class D3DImageLoader 
	{
	public:
		D3DImageLoader();
		~D3DImageLoader();

		// Note that both of these load functions allocate the necessary memory to store the image data.
		// This memory must be explicity released by the user, no management is provided.
		// For texture managment use the higher level D3DTextureManager class.
		HRESULT	LoadToMemoryFromDisk(
			const std::string& fileName,
			S32 sizeCap,
			IDirect3DDevice9* pDevice,
			_D3DFORMAT &format,
			U8* pRawImageData);

		HRESULT	LoadToTextureFromDisk(
			IDirect3DTexture9** ppTexture,
			_D3DFORMAT &format,
			U32& width,
			U32& height,
			const std::string& fileName,
			S32 sizeCap,
			IDirect3DDevice9* pDevice);

		HRESULT	LoadToTextureFromMemory(
			U8* pRawImageData,
			U32 width,
			U32 height,
			U32 bytesPerPixel,
			IDirect3DDevice9* pDevice,
			_D3DFORMAT &format,
			IDirect3DTexture9** ppTexture);
	};
}


#endif