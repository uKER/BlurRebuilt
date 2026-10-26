#ifndef D3DTEXTUREMANAGER_H
#define D3DTEXTUREMANAGER_H

#define D3DTEXTUREMANAGER	D3DTextureManager::GetSingleton()

class D3DTextureInfo
	{
	public:
		D3DTextureInfo() 
		{
			pTexture = 0;
			referenceCount = 0;
		}

		~D3DTextureInfo() {}

		std::string fileName;
		IDirect3DTexture9 *pTexture;
		BizMaths::S32 referenceCount;
	};


class D3DTextureManager 
{
public:
	D3DTextureManager();
	~D3DTextureManager();
	static D3DTextureManager &GetSingleton();
	static void Create(IDirect3DDevice9 *pD3DDevice);
	static void Destroy();
	HRESULT Load(const std::string fileName, IDirect3DTexture9 **ppTexture);
	HRESULT Unload(IDirect3DTexture9 *pTexture);

private:
	static void Init();
	static D3DTextureManager *m_singleton;

	IDirect3DDevice9 *m_pD3DDevice;
	std::vector<D3DTextureInfo> m_d3dTextureInfo;
};


#endif