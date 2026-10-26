#ifndef D3D_TEXTURE_MANAGER_H
#define D3D_TEXTURE_MANAGER_H

namespace SRD
{
	#define SR_D3D_TEXTURE_MANAGER D3dTextureManager::GetSingleton()

	class D3dTextureManager 
	{
	public:
		///
		///\brief Constructor.
		D3dTextureManager();

		///
		///\brief Destructor.
		~D3dTextureManager();

		static D3dTextureManager&	GetSingleton();
		static void					Create(IDirect3DDevice9 *pD3DDevice);
		static void					Destroy();

		IDirect3DTexture9*			Load(const std::string &fileName);

		void						Unload(IDirect3DTexture9* pTexture);

		U32							GetNumTextures() const;

		///
		///\brief Clear the contents of this resource manager.
		void		Clear();

	private:
		struct D3dTextureResource
		{
			std::string			m_fileName;
			IDirect3DTexture9*	m_pTexture;
			U32					m_referenceCount;
		};

		void						Reset();

		IDirect3DTexture9*			LoadFromDisk(const std::string fileName);

		///
		///Deny access to copy constructor
		D3dTextureManager(const D3dTextureManager& rhs);
		///
		///Deny access to asignment operator.
		const D3dTextureManager&	operator=(const D3dTextureManager& rhs);

		std::map<std::string, D3dTextureResource>	m_textureResources;
		std::map<IDirect3DTexture9*, std::string>	m_textureResourcesByTexturePointer;

		std::string									m_lastTextureFileName;
		IDirect3DTexture9*							m_pLastTexturePointerReturned;

		IDirect3DDevice9*							m_pD3DDevice;
		static D3dTextureManager*					m_pSingleton;
	};

}

#endif

