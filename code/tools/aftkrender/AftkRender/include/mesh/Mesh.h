#ifndef AFTK_RENDER_MESHRESOURCE_INCLUDE
#define AFTK_RENDER_MESHRESOURCE_INCLUDE

#ifdef _MSC_VER
#pragma once
#endif


namespace AftkRender
{
	class C_MeshResource;
	class C_GeometryNode;

	class C_MeshClusterResource
	{
		friend class C_MeshResource;
	public:
		struct VertexDataElement
		{
			void* pData;
			E_ElementType type;

			VertexDataElement()
				: pData(NULL), type(E_ElementType_Unknown) { }
		};
		typedef Ut::UtVector< Ut::DynamicVector<VertexDataElement> > VertexDataElementArray;

		struct IndexDataElement
		{
			U32* pData;
			U32	 count;
			E_PrimitiveType type;
			S16	 materialId;

			IndexDataElement()
				: pData(NULL), count(0), materialId(-1) { }
		};

	public:
		C_MeshClusterResource();
		~C_MeshClusterResource();

		C_MeshResource* GetMesh();
		const C_MeshResource* GetMesh() const;

		bool AddIndexData(const U32* pData, U32 count, E_PrimitiveType type, S16 id);
		bool AttachIndexData(U32* pData, U32 count, E_PrimitiveType type, S16 id);
		U32 GetNumIndexDataElements() const;
		const IndexDataElement* GetIndexDataElement(U32 index) const;

		void SetNumVertices(U32 nVertices);
		U32 GetNumVertices() const;

		bool AddVertexDataElement(const void *pData, E_ElementType type);
		bool AttachVertexDataElement(void *pData, E_ElementType type);

		bool HasVertexDataElement(E_ElementType type) const;
		const void* GetVertexDataElement(E_ElementType type) const;

		void SetNonIndexedPrimitiveType(E_PrimitiveType type);
		E_PrimitiveType GetNonIndexedPrimitiveType() const;

	private:
		typedef Ut::UtVector< Ut::DynamicVector< IndexDataElement > > IndexDataElementArray;

		C_MeshResource			*m_pMesh;
		E_PrimitiveType			m_NonIndexedPrimitiveType;
		

		U32						m_nVertices;
		mutable U32				m_VertexDataElements;

		VertexDataElementArray	m_VertexData;
		IndexDataElementArray	m_IndexData;
	};




	class C_MeshResource
	{
	public:
		C_MeshResource();
		~C_MeshResource();

		U32 GetNumClusters() const;

		void AddCluster(C_MeshClusterResource *pCluster);

		C_MeshClusterResource* GetCluster(U32 index);
		const C_MeshClusterResource* GetCluster(U32 index) const;

		void FreeMeshClusters();
		C_MeshClusterResource* DetachCluster(U32 index);

	private:
		typedef Ut::UtVector< Ut::DynamicVector<C_MeshClusterResource*> > MeshClusterArray;

		mutable U32			m_VertexDataElements;
		MeshClusterArray	m_Clusters;
	};
}

#endif