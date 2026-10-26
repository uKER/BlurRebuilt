#ifndef AFTK_RENDER_MESH_CLUSTER
#define AFTK_RENDER_MESH_CLUSTER

#ifdef _MSC_VER
#pragma once
#endif

namespace Aftk
{
	class C_Status;
}

namespace AftkRender
{
	class C_RenderMeshCluster
	{
	public:

		struct C_Flags
		{
			unsigned speedtree_leaf:1;
		};

		C_RenderMeshCluster(
			C_RenderMeshNode *pRenderMeshNode,
			S32 aftkMaterialIndex,
			U8 lod);

		C_RenderMeshCluster(
			const C_RenderMeshCluster& cluster);

		~C_RenderMeshCluster();

		const C_RenderMeshCluster& operator=(
			const C_RenderMeshCluster& rhs);

		void Clear();

		Aftk::C_Status Rebuild(
			const Ut::UtVector< Ut::DynamicVector<U32> >* pPolygonIndexList);

		Aftk::C_Status Render();

		S32 GetAftkMaterialIndex() const;

		C_Flags& GetClusterFlags();
		const C_Flags& GetClusterFlags() const;

	private:

		//List of IBs per VB
		struct IndexBufferRecord
		{
			U32 m_VertexBuffer;
			Ut::UtVector< Ut::DynamicVector<Display::C_IndexBuffer> > m_IndexBuffers;
		};

		Aftk::C_Status CreateIndexBuffers();
		void DestroyIndexBuffers();

		Aftk::C_Status FillIndexBuffers();

		C_RenderMeshNode   *m_pRenderMeshNode;
		S32					m_AftkMaterialIndex;
		U8					m_lod;
		C_Flags				m_ClusterFlags;	

		// list of polygons which this cluster is responsible for
		const Ut::UtVector< Ut::DynamicVector<U32> >* m_pPolygonIndexList;

		Ut::UtVector< Ut::DynamicVector<IndexBufferRecord> > m_IndexBufferRecords;
	};
}


#endif