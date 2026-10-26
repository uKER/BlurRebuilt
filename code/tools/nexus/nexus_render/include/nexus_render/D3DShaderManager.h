#ifndef D3D_SHADER_MANAGER_H
#define D3D_SHADER_MANAGER_H

namespace NexusRender
{
	class D3DShaderManager 
	{
	public:
		///
		///\brief Constructor.
		D3DShaderManager();

		///
		///\brief Destructor.
		~D3DShaderManager();

		void Create(IDirect3DDevice9 *pD3DDevice);

		void Destroy();

		Tu::Status OnPreResetDevice();

		Tu::Status OnPostResetDevice();

		ID3DXEffect* Load(const std::string &fileName);

		void Unload(ID3DXEffect* pShader);

		U32	NumShaders() const;

		///
		///\brief Clear the contents of this resource manager.
		void Clear();

	private:
		struct D3DShaderResource
		{
			std::string			m_fileName;
			ID3DXEffect*		m_pShader;
			U32					m_referenceCount;
		};

		void Reset();

		ID3DXEffect* LoadFromDisk(const std::string fileName);

		///
		///Deny access to copy constructor
		D3DShaderManager(const D3DShaderManager& rhs);

		///
		///Deny access to asignment operator.
		const D3DShaderManager&	operator=(const D3DShaderManager& rhs);

		std::map<std::string, D3DShaderResource>	m_shaderResources;
		std::map<ID3DXEffect*, std::string>			m_shaderResourcesByShaderPointer;

		std::string									m_lastShaderFileName;
		ID3DXEffect*								m_pLastShaderPointerReturned;

		IDirect3DDevice9*							m_pD3DDevice;
	};

}

#endif
