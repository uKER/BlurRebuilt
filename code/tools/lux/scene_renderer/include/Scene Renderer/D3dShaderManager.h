#ifndef D3D_SHADER_MANAGER_H
#define D3D_SHADER_MANAGER_H

namespace SRD
{
	#define SR_D3D_SHADER_MANAGER D3dShaderManager::GetSingleton()

	class D3dShaderManager 
	{
	public:
		///
		///\brief Constructor.
		D3dShaderManager();

		///
		///\brief Destructor.
		~D3dShaderManager();

		static D3dShaderManager&	GetSingleton();
		static void					Create(IDirect3DDevice9 *pD3DDevice);
		static void					Destroy();

		ID3DXEffect*				Load(const std::string &fileName);

		void						Unload(ID3DXEffect* pShader);

		U32							GetNumShaders() const;

		///
		///\brief Clear the contents of this resource manager.
		void						Clear();

	private:
		struct D3dShaderResource
		{
			std::string			m_fileName;
			ID3DXEffect*		m_pShader;
			U32					m_referenceCount;
		};

		void						Reset();

		ID3DXEffect*				LoadFromDisk(const std::string fileName);

		///
		///Deny access to copy constructor
		D3dShaderManager(const D3dShaderManager& rhs);
		///
		///Deny access to asignment operator.
		const D3dShaderManager&		operator=(const D3dShaderManager& rhs);

		std::map<std::string, D3dShaderResource>	m_shaderResources;
		std::map<ID3DXEffect*, std::string>			m_shaderResourcesByShaderPointer;

		std::string									m_lastShaderFileName;
		ID3DXEffect*								m_pLastShaderPointerReturned;

		IDirect3DDevice9*							m_pD3DDevice;
		static D3dShaderManager*					m_pSingleton;
	};

}

#endif
