#ifndef NEXUS_RENDER_D3D_MANAGER_H
#define NEXUS_RENDER_D3D_MANAGER_H

namespace NexusRender
{
	class D3DManager
	{
	public:
		enum EVertexDecl
		{
			VERTEX_DECL_POS,
			VERTEX_DECL_POS_COL,

			VERTEX_DECL_POS_NORM_TEX1,
			VERTEX_DECL_POS_NORM_TEX2,
			VERTEX_DECL_POS_NORM_TEX1_WEIGHT4_IDX4,
			VERTEX_DECL_POS_NORM_TEX2_WEIGHT4_IDX4,

			VERTEX_DECL_POS_NORM_COL_TEX1,
			VERTEX_DECL_POS_NORM_COL_TEX2,
			VERTEX_DECL_POS_NORM_COL_TEX1_WEIGHT4_IDX4,
			VERTEX_DECL_POS_NORM_COL_TEX2_WEIGHT4_IDX4,

			VERTEX_DECL_COUNT,
			VERTEX_DECL_UNKNOWN,
		};

		///
		///\brief Constructor.
		D3DManager();

		///
		///\brief Destructor.
		~D3DManager();

		static Tu::Status Create(IDirect3DDevice9 *pD3DDevice);
		
		static void Destroy();

		static Tu::Status OnPreResetDevice();

		static Tu::Status OnPostResetDevice();

		static IDirect3DVertexDeclaration9* GetVertexDeclPtr(EVertexDecl vertexDecl);

		static Tu::Status SetVertexDeclaration(EVertexDecl vertexDecl);

		static IDirect3DDevice9* GetDevicePtr();

		__forceinline static const char* GetVertexDeclAsString(EVertexDecl decl);

	private:
		static D3DManager*	m_pSingleton;
		U32					m_referenceCount;

		IDirect3DDevice9*	m_pDevice;

		static char**		m_ppVertexDeclAsString;

		//A list of all of the vertex declaration types.
		IDirect3DVertexDeclaration9* m_pVertexDeclList[VERTEX_DECL_COUNT];
	};

	__forceinline const char* D3DManager::GetVertexDeclAsString(EVertexDecl decl)
	{
		return m_ppVertexDeclAsString[decl];
	}

}

#endif

