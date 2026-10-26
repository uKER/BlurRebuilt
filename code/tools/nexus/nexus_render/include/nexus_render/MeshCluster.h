#ifndef NEXUS_RENDER_MESH_CLUSTER_H
#define NEXUS_RENDER_MESH_CLUSTER_H

namespace NexusRender
{
	class RenderMeshNode;

	class MeshCluster
	{
	public:
		MeshCluster(
			RenderMeshNode* pRenderMeshNode, 
			S32 aftkMaterialIndex,
			U8 lod);

		MeshCluster(const MeshCluster& rhs);

		virtual ~MeshCluster();

		const MeshCluster& operator=(const MeshCluster& rhs);

		Tu::Status Rebuild(const DynamicArrayU32* pPolyIndexList);

		Tu::Status Draw();

		Tu::Status OnPreResetDevice();

		Tu::Status OnPostResetDevice();

		S32 GetAftkMaterialIndex() const;

		void Clear();

	private:
		Tu::Status CreateIndexBuffers();

		Tu::Status DestroyIndexBuffers();

		Tu::Status ResetIndexBuffers();

		//We have an index buffer list for each VB. Note there may not be any index buffers
		//for a given VB.
		//[VB][IB]
		Ut::UtVector< Ut::DynamicVector<
			Ut::UtVector< Ut::DynamicVector<IndexBuffer> > > >	m_indexBufferLists;
		//Store a pointer to the scene renderer. This provides access to features such as the D3D device
		RenderMeshNode*			m_pRenderMeshNode;
		//Array of the polys for this cluster per polygon set.
		const DynamicArrayU32*	m_pPolyIndexList;
		S32						m_aftkMaterialIndex;
		U8						m_lod;
	};
}

#endif
