#ifndef OBJECT_RENDERING_BATCH_H
#define OBJECT_RENDERING_BATCH_H

namespace SRD
{
	class ObjectRenderingBatch
	{
	public:
		ObjectRenderingBatch();
		~ObjectRenderingBatch();

		void SetD3DWindowPtr(D3DWindow* pD3DWindow);
		void SetAftkScenePtr(const AftkScene* pScene);
		void SetAftkSceneMeshInterfaceNodePtr(AftkISceneMeshNode* pMeshInterfaceNode);

		const AftkSharedResource< AftkMaterialNode >*
			GetMaterialResourcePtr(U32 instance);

		U32	GetNumVertexBufferIndexArrays();
		U32	GetNumVertexBufferIndices(
			const AftkSharedResource< AftkMaterialNode >* pMaterialResource);
		U32	GetVertexBufferIndex(
			const AftkSharedResource< AftkMaterialNode >* pMaterialResource, 
			U32 instance);
		U32	GetVertexBufferIndex(U32 materialIndex, U32 instance);
		const AftkISceneMeshNode* GetAftkSceneMeshInterfaceNodePtr() const;
		AftkISceneMeshNode* GetAftkSceneMeshInterfaceNodePtr();


		IDirect3DVertexBuffer9*		GetVertexBufferPtr(U32 instance) const;
		U32							GetNumTrianglesInVertexBuffer(U32 instance) const;

		bool						RebuildVertexBuffers();

		void						OnUpdateDevice();

		void						Clear();

	private:
		bool						BuildVertexBuffers();
		bool						BuildVertexBufferFromIndexArray(
			const AftkSharedResource< AftkMaterialNode >* pMaterialResource,
			const Ut::UtVector< Ut::DynamicVector< U32 > >& polygonIndices);

		void						Reset();

		const						ObjectRenderingBatch(const ObjectRenderingBatch& rhs);
		const ObjectRenderingBatch& operator=(const ObjectRenderingBatch& rhs);

		std::map< 
			const AftkSharedResource< AftkMaterialNode >*, 
			Ut::UtVector< Ut::DynamicVector< U32 > > >				m_vertexBufferIndexArrays;

		Ut::UtVector<Ut::DynamicVector<IDirect3DVertexBuffer9*> >	m_vertexBuffers;
		Ut::UtVector<Ut::DynamicVector<U32> >						m_vertexBufferSizes;

		const AftkScene*			m_pScene;
		///
		///Store the window that we are attached to.
		D3DWindow*					m_pD3DWindow;
		///
		bool						m_vertexBuffersBuiltFlag;

		AftkISceneMeshNode* m_pMeshInterfaceNode;
	};
}

#endif
