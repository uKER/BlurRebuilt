#ifndef AFTK_RENDER_MESHNODE_INCLUDE
#define AFTK_RENDER_MESHNODE_INCLUDE


#ifdef _MSC_VER
#pragma once
#endif


namespace AftkRender
{
	class C_MeshNode;

	class C_MeshClusterResource;
	class C_MeshResource;

	class C_MeshCluster : public C_GeometryCluster
	{
	public:
		struct Flags
		{
			unsigned isStatic:1;
			unsigned isSkinned:1;
			unsigned isSkinAnimated:1;
		};

	public:
		C_MeshCluster(C_MeshNode *pMeshNode, U32 cluster);
		~C_MeshCluster();

		void Clear();
		bool Build();

	private:
		bool CreateVertexBuffers();
		bool PopulateVertexBuffers();

		bool CreateIndexBuffers();
		bool PopulateIndexBuffers();

		Flags	m_Flags;
		U32		m_Cluster;

		struct ScratchData
		{
			U32	vertexCount;
			Ut::UtVector< Ut::DynamicVector<U32> > chunkVertexCount;
			Ut::UtVector< Ut::DynamicVector<U32> > vertexMap;
			Ut::UtVector< Ut::DynamicVector<U32> > vertexIndices;
			Ut::UtVector< Ut::DynamicVector<U32> > primitiveChunkMap;
		};

		mutable ScratchData	m_scratchData;

	};


	class C_MeshNode : public C_GeometryNode
	{
	public:
		C_MeshNode(C_MeshResource *pMesh);
		~C_MeshNode();

		void Clear();
		bool Build();

		C_MeshResource* GetMeshResource();
		const C_MeshResource* GetMeshResource() const;

	private:
		bool CreateGeometryClusters();

		C_MeshResource *m_pMeshResource;
	};
}



#endif