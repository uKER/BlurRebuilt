#include "AftkRenderPch.h"

#include "common/GeometryNode.h"
#include "common/VertexFormat.h"

#include "scene/AftkMeshNode.h"


#include "Utility.h"

namespace AftkRender
{
	extern Debug::Logger& g_Log;
}

using namespace AftkRender;

static const U32 MAX_VERTEX_BUFFER_SIZE = 65535;

C_AftkMeshCluster::C_AftkMeshCluster(C_AftkMeshNode *pNode, U8 lod)
: C_GeometryCluster(pNode)
, m_Lod(lod)
{
	const U32 MAX_TEXCOORDINATE_SETS = 2;

	Aftk::C_SceneMeshNode *pAftkSceneMeshNode = pNode->GetAftkSceneMeshNodePtr();
	m_NumTexCoordSets = Maths::Min<U32>(pAftkSceneMeshNode->GetNumTextureCoordinateSets(), MAX_TEXCOORDINATE_SETS);

	//define vertex format
	bool hasColourChannel=false, has2UVs=false;
	m_Flags.isSkinned = pAftkSceneMeshNode->IsSkinned();
	m_Flags.isSkinAnimated = pAftkSceneMeshNode->IsSkinAnimated();
	m_Flags.isStatic = !m_Flags.isSkinned && !m_Flags.isSkinAnimated;

	// Stream 0 - Vertex geometry ...
	m_VertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	m_VertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Normal, 0);

	// ... and vertex colours (if present)
	S32 vertexColourChannel;
	Aftk::E_GenericAttributeType vertexColourType;
	Utility::GetVertexColourChannelAndType(pAftkSceneMeshNode, vertexColourChannel, vertexColourType);

	if (vertexColourChannel != -1)
	{
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_Position_Normals_Colour));
	}
	else
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_Position_Normals));


	//Texcoords ...
	{
		for (U32 i=0; i<m_NumTexCoordSets; i++)
			m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, i);
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_UVs));
	}

	//Vertex weights & blend indices
	if (m_Flags.isSkinned)
	{
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_BlendWeight, 0);
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_U8_4, Display::E_VertexElementUsage_BlendIndices, 0);
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_Weights_PaletteIndices));
	}

	//Vertex tangents & binormals
	{
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Tangent, 0);
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Binormal, 0);
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_Tangents_Binormals));
	}

	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(m_VertexFormat);
}

C_AftkMeshCluster::~C_AftkMeshCluster()
{
}

void C_AftkMeshCluster::Clear()
{
	C_GeometryCluster::Clear();
}

bool C_AftkMeshCluster::Build()
{
	Clear();

	if (!m_pGeometryNode)
		return false;

	bool status;

	status = CreateVertexBuffers();
	if (!status)
		return status;

	status = PopulateVertexBuffers();
	if (!status)
		return status;

	status = CreateIndexBuffers();
	if (!status)
		return status;

	status = PopulateIndexBuffers();
	if (!status)
		return status;

	return true;
}

bool C_AftkMeshCluster::CreateVertexBuffers()
{
	const Aftk::C_SceneMeshNode *pAftkMeshNode = ((C_AftkMeshNode*)m_pGeometryNode)->GetAftkSceneMeshNodePtr();

	if (!pAftkMeshNode || pAftkMeshNode->GetNumLodLevels()<m_Lod || pAftkMeshNode->GetNumPolygons(m_Lod)==0)
		return false;

	m_PolygonVertexIndexMap.Resize(pAftkMeshNode->GetNumPolygonVertices());
	memset(&m_PolygonVertexIndexMap[0], -1, sizeof(U32)*pAftkMeshNode->GetNumPolygonVertices());

	{
		U32 vertexCount = 0;

		const Aftk::PolygonArray& aftkPolygons = pAftkMeshNode->GetPolygonArray(m_Lod);
		for (S32 iPoly=0; iPoly<aftkPolygons.Size(); ++iPoly)
		{
			const Aftk::C_Polygon& polygon = aftkPolygons[iPoly];
			for (U32 jVtx=0; jVtx<polygon.GetNumPolygonVertices(); ++jVtx)
			{
				S32 pvIdx = polygon.GetPolygonVertexIndex(jVtx);
				if (m_PolygonVertexIndexMap[pvIdx] == -1)
				{
					//we've not encountered this polygon vertex before so add it to the mapping
					m_PolygonVertexIndexMap[pvIdx] = vertexCount;
					vertexCount++;
				}
			}
		}

		//Early exit if no vertices have been mapped into vertex buffers
		if (!vertexCount)
			return true;

		//Now create the vertex buffers
		for (S32 iStream=0; iStream<m_VertexStreams.Size(); ++iStream)
		{
			if (!Display::Device()->CreateVertexBuffer(
				vertexCount*m_VertexFormat.GetStride(iStream), 0, &m_VertexStreams[iStream].vertexBuffer))
				return false;
		}
	}

	return true;
}

bool C_AftkMeshCluster::PopulateVertexBuffers()
{
	const Aftk::C_SceneMeshNode *pAftkMeshNode = ((C_AftkMeshNode*)m_pGeometryNode)->GetAftkSceneMeshNodePtr();

	if (!pAftkMeshNode || !m_PolygonVertexIndexMap.Size())
		return false;

	Ut::UtVector< Ut::DynamicVector<U32> > vertexBufferIndexMap;
	//build vertex buffer->aftk polygon vertex map
	{
		U32 nVertices = 0;
		for (S32 i=0; i<m_PolygonVertexIndexMap.Size(); ++i)
			if (m_PolygonVertexIndexMap[i] != -1)
				nVertices++;

		if (!nVertices)
			return true;

		vertexBufferIndexMap.Resize(nVertices);
		memset(&vertexBufferIndexMap[0], -1, sizeof(U32)*nVertices);
		for (S32 i=0; i<m_PolygonVertexIndexMap.Size(); ++i)
		{
			if (m_PolygonVertexIndexMap[i] != -1)
			{
				ASSERT(vertexBufferIndexMap[m_PolygonVertexIndexMap[i]]==-1 && "multiple aftk polygon vertices mapped to the same vertex buffer index");
				vertexBufferIndexMap[m_PolygonVertexIndexMap[i]] = i;
			}
		}
	}

	S32 vertexColourChannel = -1;
	Aftk::E_GenericAttributeType vertexColourType = Aftk::E_GenericAttributeType_Unknown;
	Utility::GetVertexColourChannelAndType(pAftkMeshNode, vertexColourChannel, vertexColourType);

	for (S32 iStream=0; iStream<m_VertexStreams.Size(); ++iStream)
	{
		switch(m_VertexStreams[iStream].usage)
		{
			case C_VertexStream::E_VertexStreamUsage_Position_Normals:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==vertexBufferIndexMap.Size()*sizeof(Vertex_XYZ2));

				Vertex_XYZ2 *pVertexData = (Vertex_XYZ2*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				for (S32 jPv=0; jPv<vertexBufferIndexMap.Size(); ++jPv)
				{
					U32 aftkPVIndex = vertexBufferIndexMap[jPv];

					const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
					SetVertexPosition(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_Flags.isSkinned, aftkPVIndex);
					SetVertexNormal(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_Flags.isSkinned, aftkPVIndex);
				}
				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			case C_VertexStream::E_VertexStreamUsage_Position_Normals_Colour:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==vertexBufferIndexMap.Size()*sizeof(Vertex_XYZ2_C));

				Vertex_XYZ2_C *pVertexData = (Vertex_XYZ2_C*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				for (S32 jPv=0; jPv<vertexBufferIndexMap.Size(); ++jPv)
				{
					U32 aftkPVIndex = vertexBufferIndexMap[jPv];

					const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
					SetVertexPosition(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_Flags.isSkinned, aftkPVIndex);
					SetVertexNormal(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_Flags.isSkinned,aftkPVIndex);
					SetVertexColour(&pVertexData[jPv], vertexColourChannel, vertexColourType, pAftkPolygonVertex);
				}
				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			case C_VertexStream::E_VertexStreamUsage_UVs:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==vertexBufferIndexMap.Size()*m_NumTexCoordSets*sizeof(Vertex_UV));

				Vertex_UV *pVertexData = (Vertex_UV*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				for (S32 jPv=0; jPv<vertexBufferIndexMap.Size(); ++jPv)
				{
					for (U32 kUV=0; kUV<m_NumTexCoordSets; ++kUV)
					{
						U32 aftkPVIndex = vertexBufferIndexMap[jPv];
						const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
						SetVertexTextureCoordinates(&pVertexData[jPv*m_NumTexCoordSets + kUV], kUV, pAftkMeshNode, pAftkPolygonVertex);
					}
				}
				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			case C_VertexStream::E_VertexStreamUsage_Weights_PaletteIndices:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==vertexBufferIndexMap.Size()*sizeof(Vertex_W4_Idx4));

				Vertex_W4_Idx4 *pVertexData = (Vertex_W4_Idx4*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				for (S32 jPv=0; jPv<vertexBufferIndexMap.Size(); ++jPv)
				{
					U32 aftkPVIndex = vertexBufferIndexMap[jPv];
					const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
					SetVertexWeightsAndIndices(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex);
				}
				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			case C_VertexStream::E_VertexStreamUsage_Tangents_Binormals:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==vertexBufferIndexMap.Size()*sizeof(Vertex_XYZ2));

				Vertex_XYZ2 *pVertexData = (Vertex_XYZ2*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				for (S32 jPv=0; jPv<vertexBufferIndexMap.Size(); ++jPv)
				{
					U32 aftkPVIndex = vertexBufferIndexMap[jPv];

					const Aftk::C_PolygonVertex *pAftkPolygonVertex = &pAftkMeshNode->GetPolygonVertex(aftkPVIndex);
					SetVertexTangentAndBinormal(&pVertexData[jPv], pAftkMeshNode, pAftkPolygonVertex, m_Flags.isSkinned, aftkPVIndex);
				}
				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			default:
				ASSERT(false && "Invalid vertex data stream");
				return false;
		}
	}

	return true;

}

bool C_AftkMeshCluster::CreateIndexBuffers()
{
	const Aftk::C_SceneMeshNode *pAftkMeshNode = ((C_AftkMeshNode*)m_pGeometryNode)->GetAftkSceneMeshNodePtr();

	if (!pAftkMeshNode || pAftkMeshNode->GetNumLodLevels()<m_Lod || pAftkMeshNode->GetNumPolygons(m_Lod)==0)
		return false;

	D3DCAPS9 caps;
	Display::Device()->GetD3DDevice()->GetDeviceCaps(&caps);
	U32 maxPrimitives = caps.MaxPrimitiveCount;

	//Work out how to batch polygons using the polygon material
	struct BatchData
	{
		S16 material;
		U32 primitiveCount;
	};
	Ut::UtVector< Ut::DynamicVector<BatchData> > batchData;

	const Aftk::PolygonArray& aftkPolygons = pAftkMeshNode->GetPolygonArray(m_Lod);
	m_PolygonIndexBatchMap.Resize(aftkPolygons.Size());
	memset(&m_PolygonIndexBatchMap[0], -1, sizeof(U32)*aftkPolygons.Size());

	for (S32 iPoly=0; iPoly<aftkPolygons.Size(); ++iPoly)
	{
		const Aftk::C_Polygon *pPoly = &aftkPolygons[iPoly];
		S16 material = pPoly->GetMaterialMappingIndex();

		Aftk::C_TriangleIterator triIt(*pPoly, &pAftkMeshNode->GetWorldMatrix());
		U32 nTriangles = triIt.GetNumTriangles();

		bool added = false;
		for(S32 jBatch=0; jBatch<batchData.Size(); ++jBatch)
		{
			if (batchData[jBatch].material != material)
				continue;

			//need to ensure the primitive count per batch does not exceed hardware limits
			//TODO: make use of vertex offset to ensure index range does not exceed 2^16-1 (we assume 16 bit indices)
			if (batchData[jBatch].primitiveCount+nTriangles <= maxPrimitives)
			{
				m_PolygonIndexBatchMap[iPoly] = jBatch;
				batchData[jBatch].primitiveCount += nTriangles;
				added = true;
				break;
			}
		}

		if (!added)
		{
			BatchData data;
			data.material = material;
			data.primitiveCount = nTriangles;

			batchData.PushBack(data);
			m_PolygonIndexBatchMap[iPoly] = batchData.Size()-1;
		}
	}

	//We now know how many batches we need
	m_PrimitiveBatches.Resize(batchData.Size());
	U32 indexCount = 0;
	for (S32 iBatch=0; iBatch<m_PrimitiveBatches.Size(); ++iBatch)
	{
		C_PrimitiveBatch& batch = m_PrimitiveBatches[iBatch];
		batch.vertexOffset = 0;
		batch.indexOffset = indexCount;
		batch.material = batchData[iBatch].material;
		batch.primitiveCount = batchData[iBatch].primitiveCount;
		batch.primitiveType = Display::E_PrimitiveType_TriangleList;

		indexCount += 3*batch.primitiveCount;
	}

	if (indexCount)
	{
		//finally, create the index buffer
		if (!Display::Device()->CreateIndexBuffer(
			indexCount*sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		{
			LOGERROR(g_Log, "Failed to create index buffer\n");
			return false;
		}
	}

	return true;
}

bool C_AftkMeshCluster::PopulateIndexBuffers()
{
	const Aftk::C_SceneMeshNode *pAftkMeshNode = ((C_AftkMeshNode*)m_pGeometryNode)->GetAftkSceneMeshNodePtr();

	if (!pAftkMeshNode || pAftkMeshNode->GetNumLodLevels()<m_Lod || pAftkMeshNode->GetNumPolygons(m_Lod)==0)
		return false;

	if (!m_PrimitiveBatches.Size())
		return true;

	Ut::UtVector< Ut::DynamicVector<
		Ut::UtVector< Ut::DynamicVector<U32> >
	> > batchPolygonMap;

	batchPolygonMap.Resize(m_PrimitiveBatches.Size());

	//generate batch->polygon map
	const Aftk::PolygonArray& aftkPolygons = pAftkMeshNode->GetPolygonArray(m_Lod);
	for (S32 iPoly=0; iPoly<aftkPolygons.Size(); ++iPoly)
	{
		U32 batch = m_PolygonIndexBatchMap[iPoly];
		batchPolygonMap[batch].PushBack(iPoly);
	}

	//now fill the index buffer
	U16 *pIndexList = (U16*)m_IndexBuffer.Lock();
	ASSERT(pIndexList);

	U32 nIndices = 0;
	for (S32 iBatch=0; iBatch<batchPolygonMap.Size(); ++iBatch)
	{
		const Ut::UtVector< Ut::DynamicVector<U32> >& batchPolygons = batchPolygonMap[iBatch];
		for (S32 jPoly=0; jPoly<batchPolygons.Size(); ++jPoly)
		{
			const Aftk::C_Polygon& polygon = aftkPolygons[batchPolygons[jPoly]];
			Aftk::C_TriangleIterator triIt(polygon, &pAftkMeshNode->GetWorldMatrix());

			while ((const U32*)triIt)
			{
				const U32* pvIndices = triIt;
				(*pIndexList++) = (U16)m_PolygonVertexIndexMap[pvIndices[0]];
				(*pIndexList++) = (U16)m_PolygonVertexIndexMap[pvIndices[1]];
				(*pIndexList++) = (U16)m_PolygonVertexIndexMap[pvIndices[2]];

				nIndices += 3;
				triIt++;
			}
		}
	}
	m_IndexBuffer.Unlock();
	ASSERT(m_IndexBuffer.GetSize() == nIndices*sizeof(U16));

	return true;
}



C_AftkMeshNode::C_AftkMeshNode(Aftk::C_SceneMeshNode *pNode)
: m_pAftkSceneMeshNode(pNode)
{
}

C_AftkMeshNode::~C_AftkMeshNode()
{
	Clear();
}

void C_AftkMeshNode::Clear()
{
	C_GeometryNode::Clear();
}

bool C_AftkMeshNode::Build()
{
	Clear();

	//Skip empty mesh
	{
		U32 polyCount = 0;
		for (U32 iLod=0; iLod<m_pAftkSceneMeshNode->GetNumLodLevels(); ++iLod)
			polyCount += m_pAftkSceneMeshNode->GetNumPolygons(iLod);

		if (polyCount == 0)
			return false;
	}

	Aftk::C_MeshCalculateTangents::CalculateTangents(m_pAftkSceneMeshNode, true, true);

	if (!CreateGeometryClusters())
		return false;

	return true;
}

Aftk::C_SceneMeshNode* C_AftkMeshNode::GetAftkSceneMeshNodePtr()
{
	return m_pAftkSceneMeshNode;
}

const Aftk::C_SceneMeshNode* C_AftkMeshNode::GetAftkSceneMeshNodePtr() const
{
	return m_pAftkSceneMeshNode;
}

bool C_AftkMeshNode::CreateGeometryClusters()
{
	m_GeometryClusters.Resize(m_pAftkSceneMeshNode->GetNumLodLevels());

	U32 nErrors = 0;
	for (U8 iLod=0; iLod<m_pAftkSceneMeshNode->GetNumLodLevels(); ++iLod)
	{
		C_AftkMeshCluster *pCluster = new C_AftkMeshCluster(this, iLod);
		m_GeometryClusters[iLod] = pCluster;

		if (!pCluster || !pCluster->Build())
		{
			LOGWARN(g_Log, "Failed to build mesh geometry cluster (%s lod %i)", m_pAftkSceneMeshNode->GetPath().c_str(), iLod);
			nErrors++;
		}
	}

	return (nErrors == 0);
}
