#include "stdafx.h"
#include "AftkRender.h"
#include "shaders/Constants.h"

using namespace AftkRender;

C_RenderMeshCluster::C_RenderMeshCluster(
	C_RenderMeshNode *pRenderMeshNode,
	S32 aftkMaterialIndex,
	U8 lod)
:
	m_pRenderMeshNode(pRenderMeshNode),
	m_AftkMaterialIndex(aftkMaterialIndex),
	m_lod(lod)
{
	memset(&m_ClusterFlags, 0, sizeof (C_RenderMeshCluster::C_Flags));
}

C_RenderMeshCluster::C_RenderMeshCluster(
	const C_RenderMeshCluster& cluster)
{
	*this = cluster;
}

C_RenderMeshCluster::~C_RenderMeshCluster()
{
	Clear();
}

const C_RenderMeshCluster& C_RenderMeshCluster::operator=(
	const C_RenderMeshCluster &rhs)
{
	m_pRenderMeshNode = rhs.m_pRenderMeshNode;
	m_AftkMaterialIndex = rhs.m_AftkMaterialIndex;
	m_lod = rhs.m_lod;
	m_ClusterFlags = rhs.m_ClusterFlags;

	m_pPolygonIndexList = rhs.m_pPolygonIndexList;

	return *this;
}

void C_RenderMeshCluster::Clear()
{
	DestroyIndexBuffers();
	m_IndexBufferRecords.Clear();
	m_pPolygonIndexList = 0;
	m_pRenderMeshNode = 0;

	memset(&m_ClusterFlags, 0, sizeof (C_RenderMeshCluster::C_Flags));
}


Aftk::C_Status C_RenderMeshCluster::Rebuild(const Ut::UtVector< Ut::DynamicVector<U32> >* pPolygonIndexList)
{

	m_pPolygonIndexList = pPolygonIndexList;

	Aftk::C_Status status;
	if (!m_pRenderMeshNode)
		return Aftk::C_Status(Aftk::C_Status::E_Status_Failure, "Null render mesh node");

	DestroyIndexBuffers();

	if (m_pPolygonIndexList && (m_pPolygonIndexList->Size()==0))
		return Aftk::C_Status::E_Status_Success;

	status = CreateIndexBuffers();
	if (!status)
		return status;

	status = FillIndexBuffers();
	if (!status)
		return status;

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_RenderMeshCluster::CreateIndexBuffers()
{

	Aftk::C_SceneMeshNode *pAftkSceneMeshNode = 
		(Aftk::C_SceneMeshNode*)m_pRenderMeshNode->GetAftkSceneNodePtr();

	D3DCAPS9 caps;
	Display::Device()->GetD3DDevice()->GetDeviceCaps(&caps);

	U32 maxIndexBufferPrimitives = caps.MaxPrimitiveCount;

	U32 numVertexBuffers = m_pRenderMeshNode->GetNumVertexBuffers();
	U32 numIBRecords = 0;

	Ut::UtVector< Ut::DynamicVector<S32> > vbIndexToIndexBufferRecord;
	vbIndexToIndexBufferRecord.Resize(numVertexBuffers);
	for (U32 iVB=0; iVB<numVertexBuffers; ++iVB)
		vbIndexToIndexBufferRecord[iVB] = -1;

	Ut::UtVector< Ut::DynamicVector<
		Ut::UtVector< Ut::DynamicVector<U32> > > > indexBufferPrimitiveCounts;

	
	//Need to ensure IBs do not contain vertices from other VBs
	//Work out how many index buffer records are needed, and how many IBs in each one
	U32 numPolys;
	if (m_pPolygonIndexList)
		numPolys = m_pPolygonIndexList->Size();
	else
		numPolys = pAftkSceneMeshNode->GetNumPolygons(m_lod);	// Assume entire scene if no poly list given

	for (U32 iPolygon=0; iPolygon<numPolys; ++iPolygon)
	{
		U32 aftkPolyIndex;
		if (m_pPolygonIndexList)
			aftkPolyIndex = (*m_pPolygonIndexList)[iPolygon];
		else
			aftkPolyIndex = iPolygon;

		U32 currentVBIndex = m_pRenderMeshNode->GetVBIndexFromAftkPolyIndex(m_lod, aftkPolyIndex);
		S32 remappedVBIndex = vbIndexToIndexBufferRecord[currentVBIndex];
		U32 currentIBIndex;

		if (remappedVBIndex == -1)
		{
			vbIndexToIndexBufferRecord[currentVBIndex] = numIBRecords;
			remappedVBIndex = numIBRecords;

			indexBufferPrimitiveCounts.PushBack();
			indexBufferPrimitiveCounts[numIBRecords].PushBack(0);

			currentIBIndex = 0;
			numIBRecords++;
		}
		else
			currentIBIndex = indexBufferPrimitiveCounts[remappedVBIndex].Size()-1;


		const Aftk::C_Polygon* pAftkPolygon = &pAftkSceneMeshNode->GetPolygon(m_lod, aftkPolyIndex);
		Aftk::C_TriangleIterator triIt(*pAftkPolygon, &pAftkSceneMeshNode->GetWorldMatrix());
		U32 nTris = triIt.GetNumTriangles();
		if (nTris == 0)
			continue;

		if (nTris > maxIndexBufferPrimitives)
		{
			LOGWARN(g_Log, "Polygon is too large for index buffer: %d triangles\n", nTris);
			return Aftk::C_Status::E_Status_Failure;
		}

		//If we can't fit this polygon within the current IB, we'll need a new one.
		if (indexBufferPrimitiveCounts[remappedVBIndex][currentIBIndex] + nTris > maxIndexBufferPrimitives)
		{
			indexBufferPrimitiveCounts[remappedVBIndex].PushBack(0);
			currentIBIndex++;
		}

		//Increment the number of primitives for this IB.
		indexBufferPrimitiveCounts[remappedVBIndex][currentIBIndex] += nTris;
		
		ASSERT(indexBufferPrimitiveCounts[currentVBIndex][currentIBIndex] != 0);
	}

	m_IndexBufferRecords.Resize(numIBRecords);
	for (U32 iVB=0; iVB<numVertexBuffers; ++iVB)
	{
		U32 remappedVBIndex = vbIndexToIndexBufferRecord[iVB];
		if (remappedVBIndex != -1)
		{
			IndexBufferRecord& record = m_IndexBufferRecords[remappedVBIndex];
			record.m_VertexBuffer = iVB;

			record.m_IndexBuffers.Resize((U32)indexBufferPrimitiveCounts[remappedVBIndex].Size());
		}
	}

	for (U32 iRecord=0; iRecord<numIBRecords; ++iRecord)
	{
		for (U32 jIB=0; jIB<(U32)indexBufferPrimitiveCounts[iRecord].Size(); ++jIB)
		{
			Display::C_IndexBuffer& indexBuffer = m_IndexBufferRecords[iRecord].m_IndexBuffers[jIB];

			U32 nIndices = indexBufferPrimitiveCounts[iRecord][jIB]*3;

			if (!Display::Device()->CreateIndexBuffer(nIndices*sizeof(U16), 0, Display::E_Format_Index16, &indexBuffer))
			{
				LOGWARN(g_Log, "Failed to create index buffer\n");
				return Aftk::C_Status::E_Status_Failure;
			}
		}
	}

	return Aftk::C_Status::E_Status_Success;
}


void C_RenderMeshCluster::DestroyIndexBuffers()
{

	for (U32 iRecord=0; iRecord<(U32)m_IndexBufferRecords.Size(); ++iRecord)
	{
		for (U32 jIB=0; jIB<(U32)m_IndexBufferRecords[iRecord].m_IndexBuffers.Size(); ++jIB)
		{
			Display::C_IndexBuffer& indexBuffer = m_IndexBufferRecords[iRecord].m_IndexBuffers[jIB];

			Display::Device()->DestroyIndexBuffer(&indexBuffer);
		}
	}
}


Aftk::C_Status C_RenderMeshCluster::FillIndexBuffers()
{
	Aftk::C_SceneMeshNode* pAftkSceneMeshNode = 
		(Aftk::C_SceneMeshNode*)m_pRenderMeshNode->GetAftkSceneNodePtr();

	U32 numPolys;
	if (m_pPolygonIndexList)
		numPolys = m_pPolygonIndexList->Size();
	else
		numPolys = pAftkSceneMeshNode->GetNumPolygons(m_lod);



	for (U32 iRecord=0; iRecord<(U32)m_IndexBufferRecords.Size(); ++iRecord)
	{
		U32 polyIndex = 0;
		IndexBufferRecord& ibRecord = m_IndexBufferRecords[iRecord];

		for (U32 jIB=0; jIB<(U32)ibRecord.m_IndexBuffers.Size(); ++jIB)
		{
			Display::C_IndexBuffer& indexBuffer = ibRecord.m_IndexBuffers[jIB];
			U16* pIndexList;
			pIndexList = (U16*) indexBuffer.Lock();

			U32 kIndex = 0;
			while (kIndex<ibRecord.m_IndexBuffers[jIB].GetSize()/sizeof(U16) && polyIndex<numPolys)
			{
				U32 aftkPolyIndex;
				if (m_pPolygonIndexList)
					aftkPolyIndex = (*m_pPolygonIndexList)[polyIndex];
				else
					aftkPolyIndex = polyIndex;

				//Check that polygon belongs to the vertex buffer to which this record is assigned
				if (m_pRenderMeshNode->GetVBIndexFromAftkPolyIndex(m_lod, aftkPolyIndex) == ibRecord.m_VertexBuffer)
				{
					const Ut::UtVector<Ut::DynamicVector<U32> >triangulatedPolyIndices = 
						m_pRenderMeshNode->GetVBPVIndicesFromAftkPolyIndex(m_lod, aftkPolyIndex);

					for (U32 lTriVtx=0; lTriVtx<(U32)triangulatedPolyIndices.Size(); ++lTriVtx)
					{
						pIndexList[kIndex] = (U16)triangulatedPolyIndices[lTriVtx];
						kIndex++;
					}
				}
				polyIndex++;
			}
			indexBuffer.Unlock();
		}
	}

	return Aftk::C_Status::E_Status_Success;
}

Aftk::C_Status C_RenderMeshCluster::Render()
{

	if (m_AftkMaterialIndex != -1)
	{
		C_SceneRenderer* pSceneRenderer = (C_SceneRenderer*)(m_pRenderMeshNode->GetRenderer());
		const C_RenderMaterial* pMaterial = pSceneRenderer->GetRenderMaterial(m_AftkMaterialIndex);

		if (pMaterial)
			pMaterial->OnEnable();
	}

	for (U32 iRecord=0; iRecord<(U32)m_IndexBufferRecords.Size(); ++iRecord)
	{
		IndexBufferRecord& ibRecord = m_IndexBufferRecords[iRecord];
		const Ut::UtVector< Ut::DynamicVector<C_RenderMeshNode::VBStream> >& vbStreams
			= m_pRenderMeshNode->GetVertexBufferStreams(ibRecord.m_VertexBuffer);

		U32 numVertices = 0;
		for (U32 jStream=0; jStream<(U32)vbStreams.Size(); ++jStream)
		{
			C_RenderMeshNode::VBStream::E_VertexStreamUsage usage = m_pRenderMeshNode->GetVertexStreamUsage(jStream);
			switch (usage)
			{
			case C_RenderMeshNode::VBStream::E_VertexStreamUsage_Position_Normals:
				Display::Device()->SetVertexBuffer(vbStreams[jStream].m_StreamIndex, &(vbStreams[jStream].m_VertexBuffer), 0, sizeof(Display::Vertex_XYZ2));
				numVertices = vbStreams[jStream].m_VertexBuffer.GetSize() / sizeof(Display::Vertex_XYZ2);
				break;
			case C_RenderMeshNode::VBStream::E_VertexStreamUsage_Position_Normals_Colour:
				Display::Device()->SetVertexBuffer(vbStreams[jStream].m_StreamIndex, &(vbStreams[jStream].m_VertexBuffer), 0, sizeof(Display::Vertex_XYZ2_C));
				numVertices = vbStreams[jStream].m_VertexBuffer.GetSize() / sizeof(Display::Vertex_XYZ2_C);
				break;
			case C_RenderMeshNode::VBStream::E_VertexStreamUsage_UVs:
				Display::Device()->SetVertexBuffer(vbStreams[jStream].m_StreamIndex, &(vbStreams[jStream].m_VertexBuffer), 0, sizeof(Display::Vertex_UV)*m_pRenderMeshNode->GetNumUVSets());
				break;
			case C_RenderMeshNode::VBStream::E_VertexStreamUsage_Weights_PaletteIndices:
				Display::Device()->SetVertexBuffer(vbStreams[jStream].m_StreamIndex, &(vbStreams[jStream].m_VertexBuffer), 0, sizeof(Display::Vertex_W4_Idx4));
				break;
			case C_RenderMeshNode::VBStream::E_VertexStreamUsage_Tangents_Binormals:
				Display::Device()->SetVertexBuffer(vbStreams[jStream].m_StreamIndex, &(vbStreams[jStream].m_VertexBuffer), 0, sizeof(Display::Vertex_XYZ2));
				break;
			default:
				ASSERT(false && "Invalid vertex data stream");
			}
		}

		//Display::Device()->GetD3DDevice()->SetStreamSourceFreq(0,1);
		//Display::Device()->GetD3DDevice()->SetStreamSourceFreq(1,1);
		Display::Device()->SetVertexDeclaration(m_pRenderMeshNode->GetVertexDeclaration());

		for (U32 jIB=0; jIB<(U32)ibRecord.m_IndexBuffers.Size(); ++jIB)
		{
			Display::Device()->SetIndexBuffer(&(ibRecord.m_IndexBuffers[jIB]));
			Display::Device()->DrawIndexedPrimitive(Display::E_PrimitiveType_TriangleList, 0, 0, numVertices, 0, ibRecord.m_IndexBuffers[jIB].GetSize()/sizeof(U16));
		}

	}
	return Aftk::C_Status::E_Status_Success;
}

S32 C_RenderMeshCluster::GetAftkMaterialIndex() const
{
	return m_AftkMaterialIndex;
}

C_RenderMeshCluster::C_Flags& C_RenderMeshCluster::GetClusterFlags()
{
	return m_ClusterFlags;
}

const C_RenderMeshCluster::C_Flags& C_RenderMeshCluster::GetClusterFlags() const
{
	return m_ClusterFlags;
}