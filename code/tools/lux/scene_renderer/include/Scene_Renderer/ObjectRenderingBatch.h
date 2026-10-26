#ifndef OBJECT_RENDERING_BATCH_H
#define OBJECT_RENDERING_BATCH_H

namespace SRD
{
	class ObjectRenderingBatch
	{
	public:
		ObjectRenderingBatch();
		~ObjectRenderingBatch();

		void						SetD3dWindowPtr(D3dWindow* pD3dWindow);
		void						SetAftkScenePtr(const Aftk::C_Scene* pScene);
		void						SetAftkSceneNodePtr(
			Aftk::C_SceneMeshNode* pMeshInterfaceNode);

		const Aftk::C_MaterialResource*
									GetMaterialResourcePtrFromInstance(U32 instance);

		U32							GetNumVertexBufferIndexArrays();
		U32							GetNumVertexBufferIndices(
			const Aftk::C_MaterialResource* pMaterialResource);
		U32							GetVertexBufferIndex(
			const Aftk::C_MaterialResource* pMaterialResource, 
			U32 instance);
		U32							GetVertexBufferIndex(
			U32 materialIndex, 
			U32 instance);

		IDirect3DVertexBuffer9*		GetVertexBufferPtr(U32 instance) const;
		U32							GetNumTrianglesInVertexBuffer(U32 instance) const;

		bool						RebuildVertexBuffers();

		void						OnUpdateDevice();

		void						Clear();

	private:
		bool						BuildVertexBuffers();
		bool						BuildVertexBufferFromIndexArray(
			const Aftk::C_MaterialResource* pMaterialResource,
			const Ut::UtVector< Ut::DynamicVector< U32 > >& polygonIndices);

		void						Reset();

		const						ObjectRenderingBatch(const ObjectRenderingBatch& rhs);
		const ObjectRenderingBatch& operator=(const ObjectRenderingBatch& rhs);

		std::map< 
			const Aftk::C_MaterialResource*, 
			Ut::UtVector< Ut::DynamicVector< U32 > > >				m_vertexBufferIndexArrays;

		Ut::UtVector<Ut::DynamicVector<IDirect3DVertexBuffer9*> >	m_vertexBuffers;
		Ut::UtVector<Ut::DynamicVector<U32> >						m_vertexBufferSizes;

		const Aftk::C_Scene*			m_pScene;
		///
		///Store the window that we are attached to.
		D3dWindow*					m_pD3dWindow;
		///
		bool						m_vertexBuffersBuiltFlag;

		Aftk::C_SceneMeshNode*			m_pMeshInterfaceNode;
	};
}

#endif
