#include <nexus_render/stdafx.h>
#include <tools_common/Utility.h>
#include <nexus_render/D3DUtility.h>

using namespace NexusRender;

static U32 MAX_VERTEX_BUFFER_SIZE = 65535;

RenderMeshNode::RenderMeshNode(
	SceneRenderer* pSceneRenderer, 
	Aftk::C_SceneNode* pAftkSceneNode,
	Aftk::C_SceneMeshRemappableNode* pAftkMaterialMappingSource)
: RenderMeshRemappableNode(
	pSceneRenderer,
	pAftkSceneNode,
	pAftkMaterialMappingSource)
{
	m_pMeshVBList = 0;
	m_nMeshVBs = 0;
	m_pComponentDisplay = 0;

	//Choose our vertex declaration and effect mode.
	OnAftkMeshUpdate((Aftk::C_SceneMeshNode*)pAftkSceneNode);
}

RenderMeshNode::~RenderMeshNode()
{
	ClearMeshClusters();
	Clear();
}

void RenderMeshNode::OnAftkMeshUpdate(Aftk::C_SceneMeshNode* pAftkMesh)
{
	m_meshFlags.isSkinned = pAftkMesh->IsSkinned();
	m_meshFlags.isSkinAnimated = pAftkMesh->IsSkinAnimated();

	//Get the vertex colour proporties for this mesh.
	S32 vColChannel;
	Aftk::E_GenericAttributeType vColType;
	GetVertexColourChannelAndType(vColChannel, vColType);

	bool hasLightmaps = false;
	for (U32 i = 0; i < pAftkMesh->GetNumTextureCoordinateSets(); i++)
	{
		if (!_stricmp(pAftkMesh->GetTextureCoordinateSetName(i).c_str(), "luxlightmap"))
			hasLightmaps = true;
	}

	if (m_meshFlags.isSkinned)
	{
		if (vColChannel == -1)
		{
			if (hasLightmaps && pAftkMesh->GetNumTextureCoordinateSets() >= 2)
				m_vertexDecl = D3DManager::VERTEX_DECL_POS_NORM_TEX2_WEIGHT4_IDX4;
			else
				m_vertexDecl = D3DManager::VERTEX_DECL_POS_NORM_TEX1_WEIGHT4_IDX4;
		}
		else
		{
			if (hasLightmaps && pAftkMesh->GetNumTextureCoordinateSets() >= 2)
				m_vertexDecl = D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2_WEIGHT4_IDX4;
			else
				m_vertexDecl = D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1_WEIGHT4_IDX4;
		}

		m_effectMode = IRenderMeshNode::EFFECT_MODE_SKINNED;
	}
	else
	{
		if (vColChannel == -1)
		{
			//No vertex colours.
			if (hasLightmaps && pAftkMesh->GetNumTextureCoordinateSets() >= 2)
				m_vertexDecl = D3DManager::VERTEX_DECL_POS_NORM_TEX2;
			else
				m_vertexDecl = D3DManager::VERTEX_DECL_POS_NORM_TEX1;
		}
		else
		{
			//Vertex colours.
			if (hasLightmaps && pAftkMesh->GetNumTextureCoordinateSets() >= 2)
				m_vertexDecl = D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2;
			else
				m_vertexDecl = D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1;
		}
		m_effectMode = IRenderMeshNode::EFFECT_MODE_STATIC;
	}
}

Tu::Status RenderMeshNode::RebuildClusterRecordLists()
{
	if (m_nMeshVBs == 0)
		return Tu::Status::SUCCESS;

	Tu::Status status;
	Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
	U8 numLodLevels = (U8)pAftkMesh->GetNumLodLevels();

	//Store one set of cluster records for each level of detail.
	m_meshClusterLists.Resize(numLodLevels);

	//SPECIAL_CLUSTER_COUNT is used to reserve the following:
	//	The first index list for a list of all of the polys in this lod.
	//	The second index list for polys with no material.
	U32 numClusters = SPECIAL_CLUSTER_COUNT;

	for (U8 iLod = 0; iLod < numLodLevels; iLod++)
	{
		//Create a hash-table to map between polygon material mappings and a dense array
		//of clusters.
		DynamicArrayU32 materialIndexToClusterIndex;
		materialIndexToClusterIndex.Resize(m_pAftkMaterialMappingSource->GetNumPolygonMaterialMappings());

		//If we're not updating an existing mesh, then ignore unused poly clusters.
		if (m_meshClusterLists[iLod].Size() == 0)
		{
			for (U16 iMat = 0; iMat < m_pAftkMaterialMappingSource->GetNumPolygonMaterialMappings(); iMat++)
			{
				if (m_pAftkMaterialMappingSource->GetPolygonMaterialMappingReferenceCount(iMat) > 0)
				{
					materialIndexToClusterIndex[iMat] = numClusters;
					numClusters++;
				}
			}
		}
		else
		{
			for (U16 iMat = 0; iMat < m_pAftkMaterialMappingSource->GetNumPolygonMaterialMappings(); iMat++)
			{
				materialIndexToClusterIndex[iMat] = numClusters;
				numClusters++;
			}
		}

		Ut::UtVector<Ut::DynamicVector<const Aftk::C_MaterialResource*> > materialsPerCluster;
		materialsPerCluster.Resize(numClusters);
		memset(&(materialsPerCluster[0]), 0, sizeof(void*)*numClusters);

		//Reserve space for clusters.
		if (m_meshClusterLists[iLod].Size() != numClusters)
		{
			ASSERT(m_meshClusterLists[iLod].Size() == 0);
			m_meshClusterLists[iLod].Resize(numClusters);
			memset(&(m_meshClusterLists[iLod][0]), 0, sizeof(ClusterRecord)*numClusters);
		}
		else
		{
			//Clear existing poly index lists.
			for (S32 j = 0; j < m_meshClusterLists[iLod].Size(); j++)
			{
				ClusterRecord& r = m_meshClusterLists[iLod][j];
				r.polyIndexList.Clear();
			}
		}

		//Create index lists for polys with no material.
		for (U32 jPoly = 0; jPoly < pAftkMesh->GetNumPolygons(iLod); jPoly++)
		{
			const Aftk::C_Polygon* pPoly = &pAftkMesh->GetPolygon(iLod, jPoly);
			if (!m_pSceneRenderer->DoWeRenderThisPolygon(pPoly)) 
				continue;

			S16 polyMaterialIndex = pPoly->GetMaterialMappingIndex();
			if (polyMaterialIndex == -1)
			{
				ClusterRecord& r = m_meshClusterLists[iLod][SPECIAL_CLUSTER_NO_MATERIAL];
				r.polyIndexList.PushBack(jPoly);
			}
			else
			{
				U32 clusterIndex = materialIndexToClusterIndex[polyMaterialIndex];
				//The line below could be more efficient.
				materialsPerCluster[clusterIndex] = 
					m_pAftkMaterialMappingSource->GetPolygonMaterialMappingPtr(polyMaterialIndex);
				ClusterRecord& r = m_meshClusterLists[iLod][clusterIndex];
				//Store poly index
				r.polyIndexList.PushBack(jPoly);
			}
		}

		//Create clusters for this lod level.
		for (U32 jCluster = 0; jCluster < (U32)m_meshClusterLists[iLod].Size(); jCluster++)
		{
			const Aftk::C_MaterialResource* pAftkMaterial = materialsPerCluster[jCluster];
			S32 aftkMaterialIndex;
			if (pAftkMaterial)
				aftkMaterialIndex = pAftkMaterial->GetIndex();
			else
				aftkMaterialIndex = -1;

			ClusterRecord& r = m_meshClusterLists[iLod][jCluster];
			if (!r.pCluster)
			{
				if (jCluster == SPECIAL_CLUSTER_ENTIRE_MESH)
				{
					r.pCluster = new MeshCluster(
						this,
						aftkMaterialIndex,
						iLod);
					if (!r.pCluster) return Tu::Status::OUT_OF_MEMORY;
				}
				else
				{
					//Only create a new cluster if we've got some polygons to insert into it.
					if (r.polyIndexList.Size() > 0)
					{
						r.pCluster = new MeshCluster(
							this,
							aftkMaterialIndex,
							iLod);
						if (!r.pCluster) return Tu::Status::OUT_OF_MEMORY;
					}
				}
			}

			//Rebuild cluster.
			if (jCluster == SPECIAL_CLUSTER_ENTIRE_MESH)
			{
				if (r.pCluster)
				{
					status = r.pCluster->Rebuild(0);
					if (!status) return status;
				}
			}
			else
			{
				if (r.pCluster)
				{
					status = r.pCluster->Rebuild(&r.polyIndexList);
					if (!status) return status;
				}
			}
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::Rebuild()
{
	Tu::Status status;

	//Clear the proporties of this class that are affected by the nature of the
	//currently loaded scene. Note mesh clusters aren't cleared.
	Clear();

	status = RenderMeshRemappableNode::Rebuild();
	if (!status) return status;

	//Clear out the contents of any existing mesh clusters. This way, if there are any existing clusters,
	//then we can overwrite their contents.
	for (S32 iLod = 0; iLod < m_meshClusterLists.Size(); iLod++)
	{
		for (S32 jCluster = 0; jCluster < m_meshClusterLists[iLod].Size(); jCluster++)
		{
			ClusterRecord& r = m_meshClusterLists[iLod][jCluster];
			if (r.pCluster)
				r.pCluster->Clear();
		}
	}

	Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
	//Skip empty mesh.
	{
		U32 polyCount = 0;
		for (U32 iLodLevel = 0; iLodLevel < pAftkMesh->GetNumLodLevels(); iLodLevel++)
		{
			polyCount += pAftkMesh->GetNumPolygons(iLodLevel);
		}
		if (polyCount == 0)
		{
			m_nodeFlags.isBuilt = true;
			return Tu::Status::SUCCESS;
		}
	}

	m_nUVSets				= pAftkMesh->GetNumTextureCoordinateSets();
	Aftk::C_Status aftkStatus	= pAftkMesh->BuildAnimationHelpers();
	if (!status)
	{
		return Tu::Status(Tu::Status::FAILURE, aftkStatus.GetErrorString());
	}
	AFTK_ASSERT(status);

	//Rebuild the vertex buffers
	status = CreateVertexBuffers();
	if (!status) return status;

	//Populate the vertex buffers
	status = ResetMeshVertexBuffers();
	if (!status) return status;

	//Rebuild the list of polygons used by each cluster.
	status = RebuildClusterRecordLists();
	if (!status) return status;

	m_nodeFlags.isBuilt = true;
	return Tu::Status::SUCCESS;
}

void RenderMeshNode::DestroyVertexBuffers()
{
	Tu::Status status;
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;

	if (m_pMeshVBList)
	{
		for (U16 iVB = 0; iVB < m_nMeshVBs; iVB++)
		{
			if (m_pMeshVBList[iVB].pBuffer) 
				m_pMeshVBList[iVB].pBuffer->Release();
		}
		delete[] m_pMeshVBList;
		m_pMeshVBList = 0;
		m_nMeshVBs = 0;
	}

	if (m_pComponentDisplay)
	{
		m_pComponentDisplay->Destroy();
		delete m_pComponentDisplay;
		m_pComponentDisplay = 0;
	}
}

Tu::Status RenderMeshNode::CreateVertexBuffers()
{
	Tu::Status status;

	const Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
//	g_debugManager << "Processing " << pAftkSceneMeshNode->GetName() << "\n";
		
	//Allocate storage for aftk polygon vertex -> vb mapping table.
	m_aftkVBPVMappingArrayLists.PushBack();
	m_aftkVBPVMappingArrayLists[0].Resize(pAftkSceneMeshNode->GetNumPolygonVertices());
	for (U32 i = 0; i < (U32)m_aftkVBPVMappingArrayLists[0].Size(); i++)
	{
		m_aftkVBPVMappingArrayLists[0][i] = -1;
	}

	U32 nAftkPolygonVertices = pAftkSceneMeshNode->GetNumPolygonVertices();

	m_aftkPolyVBMappingLists.Resize(pAftkSceneMeshNode->GetNumLodLevels());
	for (U32 iLodLevel = 0; iLodLevel < pAftkSceneMeshNode->GetNumLodLevels(); iLodLevel++)
	{
		m_aftkPolyVBMappingLists[iLodLevel].Resize(
			pAftkSceneMeshNode->GetNumPolygons(iLodLevel)
		);
	}

	Ut::UtVector<Ut::DynamicVector< U32 > > m_vertexBufferSizes;
	m_vertexBufferSizes.PushBack(0);
	//Add a new mapping entry for the first VB.
	m_vbAftkPVMappingList.PushBack(Ut::UtVector<Ut::DynamicVector<U32> >());
	//Reserve the space for a aftk pv -> vb pv mapping table for the first VB.
	m_vbAftkPVMappingList[0].Reserve(pAftkSceneMeshNode->GetNumPolygonVertices());
	{
		for (U32 iLodLevel = 0; iLodLevel < pAftkSceneMeshNode->GetNumLodLevels(); iLodLevel++)
		{
			//Increment current VB size whenever we encounter a new PV for the first time.
			U32 currentVb = 0;
			for (U32 jPoly = 0; jPoly < pAftkSceneMeshNode->GetNumPolygons(iLodLevel); jPoly++)
			{
				const Aftk::C_Polygon* pAftkPoly = &pAftkSceneMeshNode->GetPolygon(iLodLevel, jPoly);
				//Create a triangle iterator for this poly.
				Aftk::C_TriangleIterator triIt(*pAftkPoly, &pAftkSceneMeshNode->GetWorldMatrix());

				//Create a new VB size container if needed - and skip to processing the next vertex buffer.
				U32 currentCount = m_vertexBufferSizes[currentVb];
				U32 numPVs = triIt.GetNumTriangles()*3;

				if ((currentCount + numPVs) > MAX_VERTEX_BUFFER_SIZE)
				{
					m_vertexBufferSizes.PushBack(0);

					//Add a new mapping entry for the next VB.
					m_vbAftkPVMappingList.PushBack();
					//Reserve the space for a aftk pv -> vb pv mapping table for the next VB.
					m_vbAftkPVMappingList[currentVb+1].Reserve(pAftkSceneMeshNode->GetNumPolygonVertices());

					currentVb++;
					m_aftkVBPVMappingArrayLists.PushBack();
					m_aftkVBPVMappingArrayLists[currentVb].Resize(pAftkSceneMeshNode->GetNumPolygonVertices());
					for (U32 k = 0; k < (U32)m_aftkVBPVMappingArrayLists[currentVb].Size(); k++)
						m_aftkVBPVMappingArrayLists[currentVb][k] = -1;
				}

				//Associate this poly with the current VB
				m_aftkPolyVBMappingLists[iLodLevel][jPoly] = currentVb;

				for (U32 kTri = 0; kTri < triIt.GetNumTriangles(); kTri++)
				{
					const U32* aftkPvIndices = triIt;

					//Increment the reference count for these polygon vertex indices in the current VB.
					for (U32 lPv = 0; lPv < 3; lPv++)
					{
						U32 aftkPVIndex = aftkPvIndices[lPv];
						{
							//Mark this vertex index within the current vertex buffer as using the
							//polygon vertex contained in aftkPvIndices[kPv].
							m_vbAftkPVMappingList[currentVb].PushBack(aftkPVIndex);

							//Map from Aftk PV index to VB relative PV index.
							m_aftkVBPVMappingArrayLists[currentVb][aftkPVIndex] = 
								m_vertexBufferSizes[currentVb];

							//Increment the size of our current vertex buffer.
							m_vertexBufferSizes[currentVb]++;
						}
					}
					triIt++;
				}
			}
		}
	}

	{
		U32 totalVBSizes = 0;
		for (U32 i = 0; i < (U32)m_vertexBufferSizes.Size(); i++)
			totalVBSizes += m_vertexBufferSizes[i];

		//Abort if we've nothing to put into our vbs.
		if (totalVBSizes == 0)
		{
			m_meshClusterLists.Clear();
			m_nMeshVBs = 0;
			m_pMeshVBList = 0;
			m_nUVSets = 0;

			m_aftkPolyVBMappingLists.Clear();
			m_aftkVBPVMappingArrayLists.Clear();
			m_vbAftkPVMappingList.Clear();

			return Tu::Status::SUCCESS;
		}
	}

	//Store the number of vertex buffers we're going to create.
	m_nMeshVBs = (U32)m_vertexBufferSizes.Size();

	//Allocate storage for our vertex buffer list.
	m_pMeshVBList = new VertexBuffer[m_nMeshVBs];
	
	//We now have an array of the sizes of all of the vertex buffers. And so we also know how many vertex 
	//buffers we now need. So let's create them.
	for (U32 iVB = 0; iVB < m_nMeshVBs; iVB++)
	{
		VertexBuffer* pVertexBuffer = &m_pMeshVBList[iVB];
		pVertexBuffer->size = m_vertexBufferSizes[iVB];

		//Create vertex buffer for skinned meshes
		switch (m_vertexDecl)
		{
			case D3DManager::VERTEX_DECL_POS_NORM_TEX1:
				if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
					pVertexBuffer->size * sizeof(D3DVertexPosNormTex1),
					D3DUSAGE_WRITEONLY, 
					0,
					D3DPOOL_MANAGED, 
					&pVertexBuffer->pBuffer,
					NULL)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;

			case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1:
				if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
					pVertexBuffer->size * sizeof(D3DVertexPosNormColTex1),
					D3DUSAGE_WRITEONLY, 
					0,
					D3DPOOL_MANAGED, 
					&pVertexBuffer->pBuffer,
					NULL)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;

			case D3DManager::VERTEX_DECL_POS_NORM_TEX2:
				if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
					pVertexBuffer->size * sizeof(D3DVertexPosNormTex2),
					D3DUSAGE_WRITEONLY, 
					0,
					D3DPOOL_MANAGED, 
					&pVertexBuffer->pBuffer,
					NULL)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;

			case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2:
				if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
					pVertexBuffer->size * sizeof(D3DVertexPosNormColTex2),
					D3DUSAGE_WRITEONLY, 
					0,
					D3DPOOL_MANAGED, 
					&pVertexBuffer->pBuffer,
					NULL)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;

			case D3DManager::VERTEX_DECL_POS_NORM_TEX1_WEIGHT4_IDX4:
				//Choose appropriate vertex declaration.
				if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
					pVertexBuffer->size * sizeof(D3DVertexPosNormTex1Weight4Idx4),
					D3DUSAGE_WRITEONLY, 
					0,
					D3DPOOL_MANAGED, 
					&pVertexBuffer->pBuffer,
					NULL)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;

			case D3DManager::VERTEX_DECL_POS_NORM_TEX2_WEIGHT4_IDX4:
				//Choose appropriate vertex declaration.
				if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
					pVertexBuffer->size * sizeof(D3DVertexPosNormTex2Weight4Idx4),
					D3DUSAGE_WRITEONLY, 
					0,
					D3DPOOL_MANAGED, 
					&pVertexBuffer->pBuffer,
					NULL)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;

			case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1_WEIGHT4_IDX4:
				//Choose appropriate vertex declaration.
				if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
					pVertexBuffer->size * sizeof(D3DVertexPosNormColTex1Weight4Idx4),
					D3DUSAGE_WRITEONLY, 
					0,
					D3DPOOL_MANAGED, 
					&pVertexBuffer->pBuffer,
					NULL)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;

			case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2_WEIGHT4_IDX4:
				//Choose appropriate vertex declaration.
				if (FAILED(D3DManager::GetDevicePtr()->CreateVertexBuffer( 
					pVertexBuffer->size * sizeof(D3DVertexPosNormColTex2Weight4Idx4),
					D3DUSAGE_WRITEONLY, 
					0,
					D3DPOOL_MANAGED, 
					&pVertexBuffer->pBuffer,
					NULL)))
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;

			default:
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_VB);
				break;
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::ResetMeshVertexBuffers()
{
	if (m_nMeshVBs == 0)
		return Tu::Status::SUCCESS;

	const Aftk::C_SceneMeshNode* pAftkSceneMeshNode = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;

 	Tu::Status status;

	//Look for the first vertex data channel.
	S32 vertexColourChannel = -1;
	Aftk::E_GenericAttributeType vertexColourType = 
		Aftk::E_GenericAttributeType_Unknown;
	for (U32 iChannel = 0; iChannel < pAftkSceneMeshNode->GetNumVertexDataChannels(); iChannel++)
	{
		const Aftk::C_VertexDataChannelInfo* pChannelInfo = 
			&pAftkSceneMeshNode->GetVertexDataChannelInfo(iChannel);
		if (pChannelInfo->GetName() == "vertexColour")
		{
			vertexColourChannel = iChannel;
			vertexColourType = pChannelInfo->GetDataType();
		}
	}

	U32 numUVSets = pAftkSceneMeshNode->GetNumTextureCoordinateSets();

	//Find lightmap UV set
	S32 iLMapUVSet = -1;
	for (U32 i = 0; i < pAftkSceneMeshNode->GetNumTextureCoordinateSets(); i++)
	{
		if ( pAftkSceneMeshNode->GetTextureCoordinateSetName(i) == "luxlightmap" )
		{
			iLMapUVSet = i;
			break;
		}
	}

	//Populate VB
	for (U32 iVB = 0; iVB < m_nMeshVBs; iVB++)
	{
		VertexBuffer* pVertexBuffer = &m_pMeshVBList[iVB];
		if (pVertexBuffer->pBuffer)
		{
			void* pVertexBufferContents = 0;
			status = LockMeshVertexBuffer(iVB, pVertexBufferContents);
			if (!status) return status;

			for (U32 jVBPVIndex = 0; jVBPVIndex < m_pMeshVBList[iVB].size; jVBPVIndex++)
			{
				U32 aftkPVIndex = m_vbAftkPVMappingList[iVB][jVBPVIndex];

				switch(m_vertexDecl)
				{
					case D3DManager::VERTEX_DECL_POS_NORM_TEX1:
					{
						D3DVertexPosNormTex1* pBuffer = 
							((D3DVertexPosNormTex1*&)pVertexBufferContents);
						status = CopyPolygonVertex(
							&pBuffer[jVBPVIndex],
							vertexColourChannel, 
							vertexColourType,
							iLMapUVSet,
							aftkPVIndex);
						break;
					}

					case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1:
					{
						D3DVertexPosNormColTex1* pBuffer = 
							((D3DVertexPosNormColTex1*&)pVertexBufferContents);
						status = CopyPolygonVertex(
							&pBuffer[jVBPVIndex],
							vertexColourChannel, 
							vertexColourType,
							iLMapUVSet,
							aftkPVIndex);
						break;
					}

					case D3DManager::VERTEX_DECL_POS_NORM_TEX2:
					{
						D3DVertexPosNormTex2* pBuffer = 
							((D3DVertexPosNormTex2*&)pVertexBufferContents);
						status = CopyPolygonVertex(
							&pBuffer[jVBPVIndex],
							vertexColourChannel, 
							vertexColourType,
							iLMapUVSet,
							aftkPVIndex);
						break;
					}

					case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2:
					{
						D3DVertexPosNormColTex2* pBuffer = 
							((D3DVertexPosNormColTex2*&)pVertexBufferContents);
						status = CopyPolygonVertex(
							&pBuffer[jVBPVIndex],
							vertexColourChannel, 
							vertexColourType,
							iLMapUVSet,
							aftkPVIndex);
						break;
					}

					case D3DManager::VERTEX_DECL_POS_NORM_TEX1_WEIGHT4_IDX4:
					{
						D3DVertexPosNormTex1Weight4Idx4* pBuffer = 
							(D3DVertexPosNormTex1Weight4Idx4*&)pVertexBufferContents;
						status = CopyPolygonVertex(
							&pBuffer[jVBPVIndex],
							vertexColourChannel, 
							vertexColourType,
							iLMapUVSet,
							aftkPVIndex);
							break;
					}

					case D3DManager::VERTEX_DECL_POS_NORM_TEX2_WEIGHT4_IDX4:
					{
						D3DVertexPosNormTex2Weight4Idx4* pBuffer = 
							(D3DVertexPosNormTex2Weight4Idx4*&)pVertexBufferContents;
						status = CopyPolygonVertex(
							&pBuffer[jVBPVIndex],
							vertexColourChannel, 
							vertexColourType,
							iLMapUVSet,
							aftkPVIndex);
							break;
					}

					case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1_WEIGHT4_IDX4:
					{
						D3DVertexPosNormColTex1Weight4Idx4* pBuffer = 
							(D3DVertexPosNormColTex1Weight4Idx4*&)pVertexBufferContents;
						status = CopyPolygonVertex(
							&pBuffer[jVBPVIndex],
							vertexColourChannel, 
							vertexColourType,
							iLMapUVSet,
							aftkPVIndex);
							break;
					}

					case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2_WEIGHT4_IDX4:
					{
						D3DVertexPosNormColTex2Weight4Idx4* pBuffer = 
							(D3DVertexPosNormColTex2Weight4Idx4*&)pVertexBufferContents;
						status = CopyPolygonVertex(
							&pBuffer[jVBPVIndex],
							vertexColourChannel, 
							vertexColourType,
							iLMapUVSet,
							aftkPVIndex);
							break;
					}

					default:
						break;
				}
			}
			if (!status) return status;
			status = UnlockMeshVertexBuffer(iVB);
			if (!status) return status;
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::LockMeshVertexBuffer(U32 index, void*& pVertexBufferContents)
{
	switch(m_vertexDecl)
	{
	case D3DManager::VERTEX_DECL_POS_NORM_TEX1:
		if (FAILED(m_pMeshVBList[index].pBuffer->Lock( 
			0, 
			m_pMeshVBList[index].size * sizeof(D3DVertexPosNormTex1), 
			(void**)&pVertexBufferContents, 
			0)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
		break;

	case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1:
		if (FAILED(m_pMeshVBList[index].pBuffer->Lock( 
			0, 
			m_pMeshVBList[index].size * sizeof(D3DVertexPosNormColTex1), 
			(void**)&pVertexBufferContents, 
			0)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
		break;

	case D3DManager::VERTEX_DECL_POS_NORM_TEX2:
		if (FAILED(m_pMeshVBList[index].pBuffer->Lock( 
			0, 
			m_pMeshVBList[index].size * sizeof(D3DVertexPosNormTex2), 
			(void**)&pVertexBufferContents, 
			0)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
		break;

	case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2:
		if (FAILED(m_pMeshVBList[index].pBuffer->Lock( 
			0, 
			m_pMeshVBList[index].size * sizeof(D3DVertexPosNormColTex2), 
			(void**)&pVertexBufferContents, 
			0)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
		break;

	case D3DManager::VERTEX_DECL_POS_NORM_TEX1_WEIGHT4_IDX4:
		if (FAILED(m_pMeshVBList[index].pBuffer->Lock( 
			0, 
			m_pMeshVBList[index].size * sizeof(D3DVertexPosNormTex1Weight4Idx4), 
			(void**)&pVertexBufferContents, 
			0)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
		break;

	case D3DManager::VERTEX_DECL_POS_NORM_TEX2_WEIGHT4_IDX4:
		if (FAILED(m_pMeshVBList[index].pBuffer->Lock( 
			0, 
			m_pMeshVBList[index].size * sizeof(D3DVertexPosNormTex2Weight4Idx4), 
			(void**)&pVertexBufferContents, 
			0)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
		break;

	case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1_WEIGHT4_IDX4:
		if (FAILED(m_pMeshVBList[index].pBuffer->Lock( 
			0, 
			m_pMeshVBList[index].size * sizeof(D3DVertexPosNormColTex1Weight4Idx4), 
			(void**)&pVertexBufferContents, 
			0)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
		break;

	case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2_WEIGHT4_IDX4:
		if (FAILED(m_pMeshVBList[index].pBuffer->Lock( 
			0, 
			m_pMeshVBList[index].size * sizeof(D3DVertexPosNormColTex2Weight4Idx4), 
			(void**)&pVertexBufferContents, 
			0)))
			return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
		break;

	default:
		return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_VB);
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::UnlockMeshVertexBuffer(U32 index)
{
	if (FAILED(m_pMeshVBList[index].pBuffer->Unlock()))
		return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_UNLOCK_VB);

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::EvaluateMeshVertexBuffers()
{
	if (m_nMeshVBs == 0)
	{
		return Tu::Status::SUCCESS;
	}

	if (!m_meshFlags.isSkinned)
	{
		return Tu::Status::SUCCESS;	
	}

	Tu::Status status;

	const Aftk::C_SceneMeshNode* pAftkSceneMeshNode = 
		(Aftk::C_SceneMeshNode*)m_pAftkSceneNode;

	//Populate VB
	const SceneRendererSettings* pSettings = m_pSceneRenderer->GetSettingsPtr();
	for (U32 iVB = 0; iVB < m_nMeshVBs; iVB++)
	{
		void* pVertexBufferContents = 0;
		status = LockMeshVertexBuffer(iVB, pVertexBufferContents);
		if (!status) return status;

		for (U32 jVBPVIndex = 0; jVBPVIndex < m_pMeshVBList[iVB].size; jVBPVIndex++)
		{
			U32 aftkPVIndex = m_vbAftkPVMappingList[iVB][jVBPVIndex];

			switch(m_vertexDecl)
			{
				case D3DManager::VERTEX_DECL_POS_NORM_TEX1_WEIGHT4_IDX4:
				{
					D3DVertexPosNormTex1Weight4Idx4* pBuffer = 
						(D3DVertexPosNormTex1Weight4Idx4*&)pVertexBufferContents;
					status = CopyEvaluatedPositionAndNormal(
						&pBuffer[jVBPVIndex],
						aftkPVIndex);
					break;
				}

				case D3DManager::VERTEX_DECL_POS_NORM_TEX2_WEIGHT4_IDX4:
				{
					D3DVertexPosNormTex2Weight4Idx4* pBuffer = 
						(D3DVertexPosNormTex2Weight4Idx4*&)pVertexBufferContents;
					status = CopyEvaluatedPositionAndNormal(
						&pBuffer[jVBPVIndex],
						aftkPVIndex);
					break;
				}

				case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1_WEIGHT4_IDX4:
				{
					D3DVertexPosNormColTex1Weight4Idx4* pBuffer = 
						(D3DVertexPosNormColTex1Weight4Idx4*&)pVertexBufferContents;
					status = CopyEvaluatedPositionAndNormal(
						&pBuffer[jVBPVIndex],
						aftkPVIndex);
					break;
				}

				case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2_WEIGHT4_IDX4:
				{
					D3DVertexPosNormColTex2Weight4Idx4* pBuffer = 
						(D3DVertexPosNormColTex2Weight4Idx4*&)pVertexBufferContents;
					status = CopyEvaluatedPositionAndNormal(
						&pBuffer[jVBPVIndex],
						aftkPVIndex);
					break;
				}

				default:
				{
					ASSERT(false);
					return Tu::Status::FAILURE;
				}
			}

			if (!status) return status;
		}
	
		status = UnlockMeshVertexBuffer(iVB);
		if (!status) return status;
	}
	
	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::Evaluate()
{
	Tu::Status status;

	if (m_meshFlags.isSkinned && m_meshFlags.isSkinAnimated) 
	{
		//We're using fixed function skinning.
		if (m_pSceneRenderer->GetSettingsPtr()->skinningMode  == 1)
		{
			Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
			pAftkMesh->Evaluate();

			status = EvaluateMeshVertexBuffers();
			if (!status) return status;
		}
	}

	return Tu::Status::SUCCESS;
}

void RenderMeshNode::SetActiveLodLevel()
{
	m_activeLod = GetActiveLodLevel(this);
}

Tu::Status RenderMeshNode::Update(
	bool timeChanged,
	bool scaleChanged,
	bool skinningModeChanged) 
{
	Tu::Status status;
	status = RenderNode::Update(
		timeChanged, 
		scaleChanged, 
		skinningModeChanged);
	if (!status) return status;

	if (m_nodeFlags.isCulled)
		return Tu::Status::SUCCESS;

	if (GetType() == RenderMeshNode::GetType())
	{
		//Determine if we're hidden or not by node type.
		m_nodeFlags.isHidden = 
			!m_pSceneRenderer->GetSettingsPtr()->renderMeshes;
		if (m_nodeFlags.isHidden || m_nodeFlags.isCulled)
			return Tu::Status::SUCCESS;
	}

	if (m_pSceneRenderer->GetSettingsPtr()->skinningMode == 0 && skinningModeChanged)
	{
		status = ResetMeshVertexBuffers();
		if (!status) return status;
	}

	if ((scaleChanged || skinningModeChanged || timeChanged) && m_pComponentDisplay)
	{
		bool evaluate = m_pSceneRenderer->GetSettingsPtr()->skinningMode != 0;
		if (m_pSceneRenderer->GetSettingsPtr()->renderNormals)
		{
			m_pComponentDisplay->UpdateNormals(
				m_activeLod, 
				evaluate);
		}
		if (m_pSceneRenderer->GetSettingsPtr()->tangentSpaceMode != -1)
		{
			m_pComponentDisplay->UpdateTangentSpace(
				m_activeLod, 
				(U16)m_pSceneRenderer->GetSettingsPtr()->tangentSpaceMode,
				evaluate);
		}

		m_pComponentDisplay->UpdateFaces(
			m_activeLod, 
			evaluate);
		m_pComponentDisplay->UpdatePoints
			(m_activeLod,
			evaluate);
	}

	return Tu::Status::SUCCESS;
}

D3DManager::EVertexDecl RenderMeshNode::GetVertexDecl() const
{
	return m_vertexDecl;
}

IRenderMeshNode::EEffectMode RenderMeshNode::GetEffectMode() const
{
	return m_effectMode;
}

Tu::Status RenderMeshNode::OnSelected()
{
	Tu::Status status;
	status = RenderNode::OnSelected();
	if (!status) return status;

	if (m_nodeFlags.isSelected && !m_pComponentDisplay)
	{
		m_pComponentDisplay = new MeshComponentDisplay();
		if (!m_pComponentDisplay)
			return Tu::Status::OUT_OF_MEMORY;
		status = m_pComponentDisplay->Create(this);
		if (!status) return status;
		m_pComponentDisplay->Update(m_pSceneRenderer->GetSettingsPtr()->skinningMode != 0);
	}
	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::OnDeselected()
{
	Tu::Status status;
	status = RenderNode::OnDeselected();
	if (!status) return status;

	if (!m_nodeFlags.isSelected && m_pComponentDisplay)
	{
		m_pComponentDisplay->Destroy();
		delete m_pComponentDisplay;
		m_pComponentDisplay = 0;
	}
	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::DrawNormals(U32 lodLevel)
{
	if (m_nodeFlags.isSelected)
	{
		Tu::Status status;
		if (!m_pComponentDisplay)
		{
			status = OnSelected();
			if (!status) return status;
		}

		ASSERT(m_pComponentDisplay);
		m_pComponentDisplay->DrawNormals(lodLevel);
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::DrawTangentSpace(U32 lodLevel, U16 set)
{
	if (m_nodeFlags.isSelected)
	{
		Tu::Status status;
		if (!m_pComponentDisplay)
		{
			status = OnSelected();
			if (!status) return status;
		}

		ASSERT(m_pComponentDisplay);
		m_pComponentDisplay->DrawTangentSpace(lodLevel, set);
	}

	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::DrawSelectedComponents(U32 lodLevel)
{
	if (m_pComponentDisplay)
	{
		m_pComponentDisplay->DrawSelectedFaces(lodLevel);
		m_pComponentDisplay->DrawSelectedPoints(lodLevel);
	}
	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::Draw()
{
	if (m_meshClusterLists.Size() == 0)
		return Tu::Status::SUCCESS;

	return m_meshClusterLists[m_activeLod][SPECIAL_CLUSTER_ENTIRE_MESH].pCluster->Draw();
}

Tu::Status RenderMeshNode::Draw(U32 lodLevel, U32 clusterIndex)
{
	ASSERT(lodLevel < (U32)m_meshClusterLists.Size());
	ASSERT(clusterIndex < (U32)m_meshClusterLists[lodLevel].Size());
	return m_meshClusterLists[lodLevel][clusterIndex].pCluster->Draw();
}

const MeshCluster* RenderMeshNode::GetClusterPtr(U32 lodLevel, U32 clusterIndex) const
{
	ASSERT(lodLevel < (U32)m_meshClusterLists.Size());
	ASSERT(clusterIndex < (U32)m_meshClusterLists[lodLevel].Size());
	return m_meshClusterLists[lodLevel][clusterIndex].pCluster;
}

MeshCluster* RenderMeshNode::GetClusterPtr(U32 lodLevel, U32 clusterIndex)
{
	ASSERT(lodLevel < (U32)m_meshClusterLists.Size());
	ASSERT(clusterIndex < (U32)m_meshClusterLists[lodLevel].Size());
	return m_meshClusterLists[lodLevel][clusterIndex].pCluster;
}

void RenderMeshNode::ClearMeshClusters()
{
	//Delete existing cluster information.
	for (U32 iLod = 0; iLod < (U32)m_meshClusterLists.Size(); iLod++)
	{
		for (U32 jCluster = 0; jCluster < (U32)m_meshClusterLists[iLod].Size(); jCluster++)
		{
			ClusterRecord& r = m_meshClusterLists[iLod][jCluster];
			delete r.pCluster;
		}
	}
	m_meshClusterLists.Clear();
}

void RenderMeshNode::Clear()
{
	DestroyVertexBuffers();

	//Clear Aftk<->VB mapping arrays.
	m_aftkVBPVMappingArrayLists.Clear();
	m_vbAftkPVMappingList.Clear();
	m_aftkPolyVBMappingLists.Clear();

	m_nodeFlags.isBuilt = false;
}

bool RenderMeshNode::IsSkinned() const
{
	return m_meshFlags.isSkinned;
}

bool RenderMeshNode::IsSkinAnimated() const
{
	return m_meshFlags.isSkinAnimated;
}

VertexBuffer* RenderMeshNode::GetVertexBufferPtr(U32 index)
{
	ASSERT(index < m_nMeshVBs);
	return &m_pMeshVBList[index];
}

U32	RenderMeshNode::GetVBIndexFromAftkPolyIndex(U32 lodLevel, U32 aftkPolyIndex) const
{
	ASSERT(aftkPolyIndex < (U32)m_aftkPolyVBMappingLists[lodLevel].Size());
	return m_aftkPolyVBMappingLists[lodLevel][aftkPolyIndex];
}

S32 RenderMeshNode::GetVBPVIndexFromAftkPVIndex(U32 vbIndex, U32 aftkPVIndex) const
{
	ASSERT(vbIndex < (U32)m_aftkVBPVMappingArrayLists.Size());
	ASSERT(aftkPVIndex < ((const Aftk::C_SceneMeshNode*)m_pAftkSceneNode)->GetNumPolygonVertices());
	ASSERT(aftkPVIndex < (U32)m_aftkVBPVMappingArrayLists[vbIndex].Size());
	return m_aftkVBPVMappingArrayLists[vbIndex][aftkPVIndex];
}

U32	RenderMeshNode::GetAftkPVIndexFromVBPVIndex(U32 vbIndex, U32 vbPVIndex) const
{
	ASSERT(vbIndex < (U32)m_vbAftkPVMappingList.Size());
	ASSERT(vbPVIndex < (U32)m_vbAftkPVMappingList[vbIndex].Size());
	return m_vbAftkPVMappingList[vbIndex][vbPVIndex];
}

U32 RenderMeshNode::GetNumMeshVertexBuffers() const
{
	return m_nMeshVBs;
}

Tu::Status RenderMeshNode::OnPreResetDevice()
{
	return Tu::Status::SUCCESS;
}

Tu::Status RenderMeshNode::OnPostResetDevice()
{
	return Tu::Status::SUCCESS;
}

U8 RenderMeshNode::GetNumUVSets() const
{
	return m_nUVSets;
}

RenderNode::ERenderNodeType RenderMeshNode::GetType() const
{
	return RenderNode::RENDER_NODE_TYPE_MESH;
}

bool RenderMeshNode::HasType(RenderNode::ERenderNodeType type) const
{
	if (__super::HasType(type))
		return true;
	if (type == RenderMeshNode::GetType()) 
		return true;

	return false;
}

U32 RenderMeshNode::GetNumClusters(U32 lod) const
{
	if (lod >= (U32)m_meshClusterLists.Size())
		return 0;
	return (U32)m_meshClusterLists[lod].Size();
}

void RenderMeshNode::GetVertexColourChannelAndType(S32& channel, Aftk::E_GenericAttributeType& type)
{
	Aftk::C_SceneMeshNode* pAftkMesh = (Aftk::C_SceneMeshNode*)m_pAftkSceneNode;
	//Look for the first vertex data channel.
	channel = -1;
	type = Aftk::E_GenericAttributeType_Unknown;
	for (U32 iChannel = 0; iChannel < pAftkMesh->GetNumVertexDataChannels(); iChannel++)
	{
		const Aftk::C_VertexDataChannelInfo* pChannelInfo = 
			&pAftkMesh->GetVertexDataChannelInfo(iChannel);
		if (pChannelInfo->GetName() == "vertexColour")
		{
			channel = iChannel;
			type = pChannelInfo->GetDataType();
		}
	}
}

void RenderMeshNode::SelectFaces(const Ut::UtSet<Nexus::C_ComponentSelection>& selected)
{
	ASSERT(m_pComponentDisplay);

	m_pComponentDisplay->SelectFaces(
		selected);

	m_pComponentDisplay->UpdateFaces(
		m_pSceneRenderer->GetSettingsPtr()->skinningMode != 0);
}

void RenderMeshNode::SelectPoints(const Ut::UtSet<Nexus::C_ComponentSelection>& selected)
{
	ASSERT(m_pComponentDisplay);
	m_pComponentDisplay->SelectFaces(selected);
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS

#endif
