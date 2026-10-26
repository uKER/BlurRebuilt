#ifndef D3D_TEXTURE_MANAGER_H
#define D3D_TEXTURE_MANAGER_H

namespace NexusRender
{
	class D3DTextureManager 
	{
	public:
		///
		///\brief Constructor.
		D3DTextureManager();

		///
		///\brief Destructor.
		~D3DTextureManager();

		void Create(IDirect3DDevice9 *pD3DDevice);
		void Destroy();

		IDirect3DTexture9* Blank();

		IDirect3DTexture9* Load(
			const std::string &fileName,
			S32 sizeCap,
			U32* pWidth = 0,
			U32* pHeight = 0);

		IDirect3DTexture9* LoadBlank(const std::string &fileName);

		IDirect3DTexture9* Find(const std::string &fileName);

		S32 TextureIndex(const std::string& fileName) const;

		void Unload(IDirect3DTexture9* pTexture);

		U32	GetNumTextures() const;

		Tu::Status OnPreResetDevice();

		Tu::Status OnPostResetDevice();

		///
		///\brief Clear the contents of this resource manager.
		void Clear();

	private:
		class D3DTextureResource
		{
		public:
			D3DTextureResource() {}
			~D3DTextureResource() {}

			std::string			m_fileName;
			IDirect3DTexture9*	m_pTexture;
			U32					m_referenceCount;
			U32					m_index;
			U32					m_width;
			U32					m_height;
		};

		void						Reset();

		IDirect3DTexture9*			LoadFromDisk(
			U32& width, 
			U32& height,
			const std::string fileName,
			S32 sizeCap);

		IDirect3DTexture9*			LoadBlankFromMemory();

		///
		///Deny access to copy constructor
		D3DTextureManager(const D3DTextureManager& rhs);
		///
		///Deny access to asignment operator.
		const D3DTextureManager&	operator=(const D3DTextureManager& rhs);

		std::map<std::string, D3DTextureResource*>	m_textureResources;
		std::map<IDirect3DTexture9*, std::string>	m_textureResourcesByTexturePointer;
		Ut::UtVector<Ut::DynamicVector<D3DTextureResource*> > m_textureResourceArray;

		std::string									m_lastTextureFileName;
		IDirect3DTexture9*							m_pLastTexturePointerReturned;

		IDirect3DDevice9*							m_pD3DDevice;

		IDirect3DTexture9*							m_pBlankTexture;

		U32											m_referenceCount;
	};

}

#endif

