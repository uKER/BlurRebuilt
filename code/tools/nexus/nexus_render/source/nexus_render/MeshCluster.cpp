#include <nexus_render/stdafx.h>
#include <nexus_render/VertexFormats.h>

using namespace NexusRender;

Aftk::Matrix44 D3DToTemplateMatrix44(const D3DMATRIX& rhs);


MeshCluster::MeshCluster(
	RenderMeshNode* pRenderMeshNode, 
	S32 aftkMaterialIndex,
	U8 lod)
{
	m_aftkMaterialIndex = aftkMaterialIndex;
	m_pRenderMeshNode = pRenderMeshNode;
	m_pPolyIndexList = 0;
	m_lod = lod;
}

MeshCluster::MeshCluster(const MeshCluster& rhs)
{
	*this = rhs;
}

MeshCluster::~MeshCluster()
{
	Clear();
}

const MeshCluster& MeshCluster::operator=(const MeshCluster& rhs)
{
	m_pRenderMeshNode = rhs.m_pRenderMeshNode;
	m_aftkMaterialIndex = rhs.m_aftkMaterialIndex;
	m_indexBufferLists = rhs.m_indexBufferLists;
	m_pPolyIndexList = rhs.m_pPolyIndexList;

	return *this;
}

Tu::Status MeshCluster::Rebuild(const DynamicArrayU32* pPolyIndexList)
{
	m_pPolyIndexList = pPolyIndexList;

	Tu::Status status;
	if (!m_pRenderMeshNode)
		return Tu::Status(Tu::Status::FAILURE, "Failed to create mesh cluster. Render mesh node is null.");

	status = DestroyIndexBuffers();
	if (!status) return status;

	if (m_pPolyIndexList && (m_pPolyIndexList->Size() == 0))
		return Tu::Status::SUCCESS;

	status = CreateIndexBuffers();
	if (!status) return status;

	status = ResetIndexBuffers();
	if (!status) return status;

	return Tu::Status::SUCCESS;
}


Tu::Status MeshCluster::CreateIndexBuffers()
{
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode = 
		(Aftk::C_SceneMeshNode*)m_pRenderMeshNode->GetAftkSceneNodePtr();

	U32 nVertexBuffers = m_pRenderMeshNode->GetNumMeshVertexBuffers();

	D3DCAPS9 caps;
	D3DManager::GetDevicePtr()->GetDeviceCaps(&caps);
	U32 maxIndexBufferPrimitives = caps.MaxPrimitiveCount;

	//Count the number of primitives within the index buffers. We can have multiple IBs for each VB, hence
	//we need a 3D array  [VB Index] [IB index].
	Ut::UtVector< Ut::DynamicVector<
		Ut::UtVector< Ut::DynamicVector<U32> > > > indexBufferPrimitiveCounts;
	
	U32 nLodLevels = pAftkSceneMeshNode->GetNumLodLevels();
	indexBufferPrimitiveCounts.Resize(nVertexBuffers);
	//Intitialise all primitive counts to zero for each VB.
	for (U32 iVB = 0; iVB < nVertexBuffers; iVB++)
	{
		indexBufferPrimitiveCounts[iVB].PushBack(0);
	}

	//Index buffers cannot contain polygon vertices from different VBs. Therefore we need to build an IB
	//for each VB that we use polygon vertices from.
	U32 nPolys;
	if (m_pPolyIndexList)
	{
		nPolys = (U32)((*m_pPolyIndexList).Size());
	}
	else
	{
		nPolys = pAftkSceneMeshNode->GetNumPolygons(m_lod);
	}

	for (U32 jPolyIndex = 0; jPolyIndex < nPolys; jPolyIndex++)
	{
		U32 aftkPolyIndex;
		if (m_pPolyIndexList)
			aftkPolyIndex = (*m_pPolyIndexList)[jPolyIndex];
		else
			//We're not using a poly list - instead we use all polys in the current lod.
			aftkPolyIndex = jPolyIndex;

		//Find the VB this polygon is from.
		U32 currentVBIndex =
			m_pRenderMeshNode->GetVBIndexFromAftkPolyIndex(m_lod, aftkPolyIndex);
		U32 currentIBIndex = indexBufferPrimitiveCounts[currentVBIndex].Size()-1;

		//Get the polygon...
		const Aftk::C_Polygon* pAftkPolygon = &pAftkSceneMeshNode->GetPolygon(m_lod, aftkPolyIndex);
		Aftk::C_TriangleIterator triIt(*pAftkPolygon, &pAftkSceneMeshNode->GetWorldMatrix());
		U32 nTris = triIt.GetNumTriangles();
		if (nTris == 0)
			continue;

		//If we can fit this polygon within the current IB, we'll need a new one.
		if (nTris > maxIndexBufferPrimitives)
		{
			std::stringstream ss;
			ss << "The maximum supported number of triangles per polygon is " << maxIndexBufferPrimitives << ".";
			return Tu::Status(Tu::Status::FAILURE, ss.str());
		}
		if (indexBufferPrimitiveCounts[currentVBIndex][currentIBIndex] + nTris > maxIndexBufferPrimitives)
		{
			indexBufferPrimitiveCounts[currentVBIndex].PushBack(0);
			currentIBIndex++;
		}
		//Increment the number of primitives for this IB.
		indexBufferPrimitiveCounts[currentVBIndex][currentIBIndex] += nTris;
		
		ASSERT(indexBufferPrimitiveCounts[currentVBIndex][currentIBIndex] != 0);
	}


	//We now have an array containing the number of triangle primtives for each IB that we require.
	//Allocate storage so that we can potentially store a list of index buffers for each VB of our mesh.
	
	//Create a list of index buffers.
	m_indexBufferLists.Resize(nVertexBuffers);
	for (U32 iVB = 0; iVB < nVertexBuffers; iVB++)
	{
		U32 indexBufferCountForVB = (U32)indexBufferPrimitiveCounts[iVB].Size();
		if ((indexBufferCountForVB > 0) && (indexBufferPrimitiveCounts[iVB][0] > 0))
		{
			m_indexBufferLists[iVB].Resize(indexBufferCountForVB);
		}
	}
	
	for (U32 iVB = 0; iVB < nVertexBuffers; iVB++)
	{
		U32 numIBforThisVB = (U32)indexBufferPrimitiveCounts[iVB].Size();
		for (U32 jIB = 0; jIB < numIBforThisVB; jIB++)
		{
			if (indexBufferPrimitiveCounts[iVB][jIB] > 0)
			{
				IndexBuffer& gIndexBuffer = m_indexBufferLists[iVB][jIB];
				gIndexBuffer.size = indexBufferPrimitiveCounts[iVB][jIB]*3;

				HRESULT hr;

				hr = D3DManager::GetDevicePtr()->CreateIndexBuffer( 
					gIndexBuffer.size * sizeof(U16),
					D3DUSAGE_WRITEONLY,
					D3DFMT_INDEX16,
					D3DPOOL_MANAGED,
					&(gIndexBuffer.pIndexBuffer),
					0);

				if (FAILED(hr))
				{
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_CREATE_IB);
				}
			}
			else
			{
				ASSERT(m_indexBufferLists[iVB].Size() == 0);
			}
		}
	}
	return Tu::Status::SUCCESS;
}

Tu::Status MeshCluster::DestroyIndexBuffers()
{
	for (U32 jVB = 0; jVB < (U32)m_indexBufferLists.Size(); jVB++)
	{
		for (U32 kIB = 0; kIB < (U32)m_indexBufferLists[jVB].Size(); kIB++)
		{
			IndexBuffer gIndexBuffer = m_indexBufferLists[jVB][kIB];
			ASSERT(m_indexBufferLists[jVB][kIB].pIndexBuffer);

			if (m_indexBufferLists[jVB][kIB].pIndexBuffer)
			{
				if (FAILED(gIndexBuffer.pIndexBuffer->Release()))
				{
					return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_RELEASE_IB);
				}
			}
		}
	}
	m_indexBufferLists.Clear();
	return Tu::Status::SUCCESS;
}

Tu::Status MeshCluster::ResetIndexBuffers()
{
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode = 
		(Aftk::C_SceneMeshNode*)m_pRenderMeshNode->GetAftkSceneNodePtr();

	HRESULT hr;

	//Copy all of the polygon indices from each VB in to their IBs
	for (U32 jVB = 0; jVB < (U32)m_indexBufferLists.Size(); jVB++)
	{
		U32 polyIndex = 0;

		for (U32 kIB = 0; kIB < (U32)m_indexBufferLists[jVB].Size(); kIB++)
		{
			IndexBuffer gIndexBuffer = m_indexBufferLists[jVB][kIB];

			//Pointer to our indices
			U16* pIndexList;

			//Lock the next IB
			hr = gIndexBuffer.pIndexBuffer->Lock( 
				0,						//Fill from stsart of the buffer.
				gIndexBuffer.size * sizeof(U16),	// Size of the data to load.
				(void**)&pIndexList,	// Returned index data.
				0);
			if (FAILED(hr))
			{
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_LOCK_IB);
			}

			for (U32 lIndex = 0; lIndex < gIndexBuffer.size;)
			{
				//Copy the triangles within a polygon.
				{
					U32 aftkPolyIndex;
					if (m_pPolyIndexList)
						aftkPolyIndex = (*m_pPolyIndexList)[polyIndex];
					else
						//We're not using a list of polys - use all of them.
						aftkPolyIndex = polyIndex;

					const Aftk::C_Polygon* pAftkPolygon = 
						&pAftkSceneMeshNode->GetPolygon(m_lod, aftkPolyIndex);
					//Only process this polygon if it's within the current VB
					if (m_pRenderMeshNode->GetVBIndexFromAftkPolyIndex(m_lod, aftkPolyIndex) == jVB)
					{
						Aftk::C_TriangleIterator triIt(*pAftkPolygon, &pAftkSceneMeshNode->GetWorldMatrix());
						U32 nTris = triIt.GetNumTriangles();

						for (U32 mTri = 0; mTri < nTris; mTri++)
						{
							const U32* pTrianglePolyVertexIndices = triIt;

							//Loop thru the poly verts for this triangle.
							for (U32 nPolyVert = 0; nPolyVert < 3; nPolyVert++)
							{
								//We need to map from AFTK polygon vertex indices -> to VB polygon vertex indices.
								//Not only that, but we also need to know which VB our indices are in.
								U32 aftkPVIndex = pTrianglePolyVertexIndices[nPolyVert];
								S32 aftkVBMapping = 
									m_pRenderMeshNode->GetVBPVIndexFromAftkPVIndex(jVB, aftkPVIndex);
								ASSERT(aftkVBMapping != -1);

								ASSERT(lIndex < gIndexBuffer.size);
								pIndexList[lIndex] = aftkVBMapping;
								//skip to next list item
								lIndex++;
							}
							triIt++;
						}
					}
					polyIndex++;
				}
			}

			//Unlock current IB
			if (FAILED(gIndexBuffer.pIndexBuffer->Unlock()))
			{
				return Tu::Status(Tu::Status::FAILURE, SRD_E_FAILED_TO_UNLOCK_IB);
			}
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status MeshCluster::Draw()
{
	for (U32 jVB = 0; jVB < (U32)m_indexBufferLists.Size(); jVB++)
	{
		VertexBuffer* pVertexBuffer = m_pRenderMeshNode->GetVertexBufferPtr(jVB);

		//Choose appropriate vertex declaration.
		switch (m_pRenderMeshNode->GetVertexDecl())
		{
		case D3DManager::VERTEX_DECL_POS_NORM_TEX1:
			if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
				0, 
				pVertexBuffer->pBuffer, 
				0,
				sizeof(D3DVertexPosNormTex1))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
			break;

		case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1:
			if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
				0, 
				pVertexBuffer->pBuffer, 
				0,
				sizeof(D3DVertexPosNormColTex1))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
			break;

		case D3DManager::VERTEX_DECL_POS_NORM_TEX2:
			if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
				0, 
				pVertexBuffer->pBuffer, 
				0,
				sizeof(D3DVertexPosNormTex2))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
			break;

		case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2:
			if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
				0, 
				pVertexBuffer->pBuffer, 
				0,
				sizeof(D3DVertexPosNormColTex2))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
			break;

		case D3DManager::VERTEX_DECL_POS_NORM_TEX1_WEIGHT4_IDX4:
			if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
				0, 
				pVertexBuffer->pBuffer, 
				0,
				sizeof(D3DVertexPosNormTex1Weight4Idx4))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
			break;

		case D3DManager::VERTEX_DECL_POS_NORM_TEX2_WEIGHT4_IDX4:
			if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
				0, 
				pVertexBuffer->pBuffer, 
				0,
				sizeof(D3DVertexPosNormTex2Weight4Idx4))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
			break;

		case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX1_WEIGHT4_IDX4:
			if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
				0, 
				pVertexBuffer->pBuffer, 
				0,
				sizeof(D3DVertexPosNormColTex1Weight4Idx4))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
			break;

		case D3DManager::VERTEX_DECL_POS_NORM_COL_TEX2_WEIGHT4_IDX4:
			if (FAILED(D3DManager::GetDevicePtr()->SetStreamSource(
				0, 
				pVertexBuffer->pBuffer, 
				0,
				sizeof(D3DVertexPosNormColTex2Weight4Idx4))))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
			break;

		default:
			return Tu::Status(Tu::Status::FAILURE, "Failed to set VB stream source.");
		}

		for (U32 kIB = 0; kIB < (U32)m_indexBufferLists[jVB].Size(); kIB++)
		{
			const IndexBuffer& gIndexBuffer = m_indexBufferLists[jVB][kIB];
			if (FAILED(D3DManager::GetDevicePtr()->SetIndices( gIndexBuffer.pIndexBuffer )))
				return Tu::Status(Tu::Status::FAILURE, "Failed to set device index buffer.");

			HRESULT hr;
			hr = D3DManager::GetDevicePtr()->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST,
				0,
				0,
				pVertexBuffer->size,
				0,
				gIndexBuffer.size / 3);
			if (FAILED(hr))
				return Tu::Status(Tu::Status::FAILURE, "DrawIndexedPrimitive failed.");
		}
	}

	return Tu::Status::SUCCESS;
}

Tu::Status MeshCluster::OnPreResetDevice()
{
	return Tu::Status::SUCCESS;
}

Tu::Status MeshCluster::OnPostResetDevice()
{
	return Tu::Status::SUCCESS;
}

S32 MeshCluster::GetAftkMaterialIndex() const
{
	return m_aftkMaterialIndex;
}

void MeshCluster::Clear()
{
	DestroyIndexBuffers();
	m_pPolyIndexList = 0;
}


