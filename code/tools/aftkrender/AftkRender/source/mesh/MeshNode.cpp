#include "AftkRenderPch.h"

#include "common/GeometryNode.h"
#include "common/VertexFormat.h"

#include "mesh/MeshElements.h"
#include "mesh/MeshNode.h"
#include "mesh/Mesh.h"


namespace AftkRender
{
	extern Debug::Logger& g_Log;
}

using namespace AftkRender;


template<typename T>
void SetVertexXYZ2(T *pVertex, const F32 *xyz1, const F32 *xyz2)
{
	pVertex->x0 = xyz1[0]; pVertex->y0 = xyz1[1]; pVertex->z0 = xyz1[2];
	pVertex->x1 = xyz2[0]; pVertex->y1 = xyz2[1]; pVertex->z1 = xyz2[2];
}

template<typename T>
void SetVertexColour(T *pVertex, const U8 *c)
{
	pVertex->r = c[0]; pVertex->g = c[1]; pVertex->b = c[2]; pVertex->a = c[3];
}

template<typename T>
void SetVertexTextureCoordinate(T *pVertex, const F32 *uv)
{
	pVertex->u = uv[0]; pVertex->v = uv[1];
}

template<typename T>
void SetVertexWeightsAndIndices(T *pVertex, const F32 *w, const U8 *i)
{
	memcpy(pVertex->w, w, 4*sizeof(F32));
	memcpy(pVertex->idx, i, 4*sizeof(U8));
}


C_MeshCluster::C_MeshCluster(C_MeshNode *pMeshNode, U32 cluster)
: C_GeometryCluster(pMeshNode)
, m_Cluster(cluster)
{
	const C_MeshResource *pResource = pMeshNode->GetMeshResource();
	assert(pResource);

	const C_MeshClusterResource *pClusterResource = pResource->GetCluster(cluster);
	if(!pClusterResource)
		return;

	//stream 0 - Vertex geometry
	m_VertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	m_VertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Normal, 0);

	// ... and vertex colours (if present)
	if (pClusterResource->HasVertexDataElement(E_ElementType_Colour))
	{	
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_Colour, Display::E_VertexElementUsage_Colour, 0);
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_Position_Normals_Colour));
	}
	else
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_Position_Normals));

	//Texcoords ...
	{
		const U32 MAX_TEXCOORDINATE_SETS = 2;
		E_ElementType type = E_ElementType_TexCoord0;
		
		for (U32 i=0; i<MAX_TEXCOORDINATE_SETS; ++i)
		{
			if (pClusterResource->HasVertexDataElement(type))
			{
				m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_F32_2, Display::E_VertexElementUsage_TexCoord, i);
				m_NumTexCoordSets++;
				type = (E_ElementType)((U32)type << 1);
			}
		}
		if (m_NumTexCoordSets)
			m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_UVs));
	}

	//Vertex weights & blend indices
	if (pClusterResource->HasVertexDataElement(E_ElementType_Weights) && pClusterResource->HasVertexDataElement(E_ElementType_BlendIndices))
	{
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_F32_4, Display::E_VertexElementUsage_BlendWeight, 0);
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_U8_4, Display::E_VertexElementUsage_BlendIndices, 0);
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_Weights_PaletteIndices));
	}

	if (pClusterResource->HasVertexDataElement(E_ElementType_Tangent) && pClusterResource->HasVertexDataElement(E_ElementType_Binormal))
	{
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Tangent, 0);
		m_VertexFormat.AddElement(m_VertexStreams.Size(), Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Binormal, 0);
		m_VertexStreams.PushBack(C_VertexStream(m_VertexStreams.Size(), C_VertexStream::E_VertexStreamUsage_Tangents_Binormals));
	}

	m_pVertexDeclaration = Display::Device()->CreateVertexDeclaration(m_VertexFormat);


}

C_MeshCluster::~C_MeshCluster()
{
}

void C_MeshCluster::Clear()
{
	C_GeometryCluster::Clear();
}

bool C_MeshCluster::Build()
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

	m_scratchData.vertexCount = 0;
	m_scratchData.chunkVertexCount.Clear();
	m_scratchData.vertexMap.Clear();
	m_scratchData.vertexIndices.Clear();
	m_scratchData.primitiveChunkMap.Clear();

	return true;
}

bool C_MeshCluster::CreateVertexBuffers()
{
	const C_MeshResource* pMeshResource = ((C_MeshNode*)m_pGeometryNode)->GetMeshResource();
	
	if (!pMeshResource)
		return false;

	const C_MeshClusterResource *pClusterResource = pMeshResource->GetCluster(m_Cluster);
	if (!pClusterResource)
		return false;

	if (!pClusterResource->GetNumVertices())
		return true;

	m_scratchData.vertexCount = 0;
	m_scratchData.chunkVertexCount.Clear();
	m_scratchData.vertexIndices.Clear();
	m_scratchData.primitiveChunkMap.Clear();
	m_scratchData.vertexMap.Clear();

	for (U32 i=0; i<pClusterResource->GetNumIndexDataElements(); ++i)
	{
		const C_MeshClusterResource::IndexDataElement *pIndexElement = pClusterResource->GetIndexDataElement(i);
		U32 primitiveCount = GetNumPrimitives(pIndexElement->type, pIndexElement->count);
		U32 primitiveIndices[4];
		
		m_scratchData.primitiveChunkMap.Reserve(m_scratchData.primitiveChunkMap.Size() + primitiveCount);

		for (U32 j=0; j<primitiveCount; ++j)
		{
			bool added = false;
			GetPrimitiveIndices(pIndexElement->type, j, primitiveIndices);

			for (S32 k=0; k<m_scratchData.chunkVertexCount.Size(); ++k)
			{
				U32 nVerticesToAdd = 0;
				U32 lookupOffset = k*pClusterResource->GetNumVertices();
				for (U32 l=0; l<GetPrimitiveVertexCount(pIndexElement->type); ++l)
				{
					U32 lookup = lookupOffset + pIndexElement->pData[primitiveIndices[l]];
					if (m_scratchData.vertexMap[lookup]==-1)
						nVerticesToAdd++;
				}

				if (m_scratchData.chunkVertexCount[k] + nVerticesToAdd < 0x0000ffff)
				{
					for (U32 l=0; l<GetPrimitiveVertexCount(pIndexElement->type); ++l)
					{
						U32 lookup = lookupOffset + pIndexElement->pData[primitiveIndices[l]];
						if (m_scratchData.vertexMap[lookup] == -1)
							m_scratchData.vertexMap[lookup] = m_scratchData.chunkVertexCount[k]++;
					}
					m_scratchData.primitiveChunkMap.PushBack(k);
					added = true;
					break;
				}
			}

			if (!added)
			{
				U32 lookupOffset = m_scratchData.chunkVertexCount.Size()*pClusterResource->GetNumVertices();
				m_scratchData.primitiveChunkMap.PushBack(m_scratchData.chunkVertexCount.Size());
				m_scratchData.chunkVertexCount.PushBack(0);
				m_scratchData.vertexMap.Resize(m_scratchData.chunkVertexCount.Size()*pClusterResource->GetNumVertices());
				memset(&m_scratchData.vertexMap[lookupOffset], -1, sizeof(U32)*pClusterResource->GetNumVertices());

				for (U32 k=0; k<GetPrimitiveVertexCount(pIndexElement->type); ++k)
				{
					U32 lookup = lookupOffset + pIndexElement->pData[primitiveIndices[k]];
					if (m_scratchData.vertexMap[lookup]==-1)
						m_scratchData.vertexMap[lookup] = m_scratchData.chunkVertexCount.Back()++;
				}
			}
		}
	}

	for (S32 i=0; i<m_scratchData.chunkVertexCount.Size(); ++i)
		m_scratchData.vertexCount += m_scratchData.chunkVertexCount[i];

	//populate scratch data structures
	m_scratchData.vertexIndices.Resize(m_scratchData.vertexCount);
	memset(&m_scratchData.vertexIndices[0], -1, sizeof(U32)*m_scratchData.vertexCount);

	Ut::UtVector<Ut::DynamicVector<U32> > chunkOffsets;
	chunkOffsets.PushBack(0);
	for (S32 i=0; i<m_scratchData.chunkVertexCount.Size()-1; ++i)
		chunkOffsets.PushBack(chunkOffsets.Back() + m_scratchData.chunkVertexCount[i]);

	for (S32 i=0; i<m_scratchData.vertexMap.Size(); ++i)
	{
		U32 vertex = m_scratchData.vertexMap[i];
		if (vertex == -1)
			continue;

		U32 chunk = i/pClusterResource->GetNumVertices();
		U32 vertexIndex = i - chunk*pClusterResource->GetNumVertices();
		U32 remapIndex = chunkOffsets[chunk] + vertex;

		assert(m_scratchData.vertexIndices[remapIndex] == -1);
		m_scratchData.vertexIndices[remapIndex] = vertexIndex;
	}

	//Create vertex buffers
	for (S32 iStream=0; iStream<m_VertexStreams.Size(); ++iStream)
	{
		if (!Display::Device()->CreateVertexBuffer(
			m_scratchData.vertexCount*m_VertexFormat.GetStride(iStream), 0, &m_VertexStreams[iStream].vertexBuffer))
			return false;
	}

	return true;
}

bool C_MeshCluster::PopulateVertexBuffers()
{
	const C_MeshResource* pMeshResource = ((C_MeshNode*)m_pGeometryNode)->GetMeshResource();
	
	if (!pMeshResource)
		return false;

	const C_MeshClusterResource *pClusterResource = pMeshResource->GetCluster(m_Cluster);
	if (!pClusterResource)
		return false;

	if (!m_scratchData.vertexCount)
		return false;

	U32 nVertices = m_scratchData.vertexCount;
	U32 currentVertex = 0;

	for (S32 iStream=0; iStream<m_VertexStreams.Size(); ++iStream)
	{
		switch (m_VertexStreams[iStream].usage)
		{
			case C_VertexStream::E_VertexStreamUsage_Position_Normals:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==nVertices*sizeof(Vertex_XYZ2));

				Vertex_XYZ2 *pVertexData = (Vertex_XYZ2*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				const F32 *pPositions = (const F32*) pClusterResource->GetVertexDataElement(E_ElementType_Position);
				const F32 *pNormals = (const F32*) pClusterResource->GetVertexDataElement(E_ElementType_Normal);

				for (U32 jV=0; jV<nVertices; ++jV)
					SetVertexXYZ2(&pVertexData[jV], &pPositions[3*m_scratchData.vertexIndices[jV]], &pNormals[3*m_scratchData.vertexIndices[jV]]);

				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			case C_VertexStream::E_VertexStreamUsage_Position_Normals_Colour:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==nVertices*sizeof(Vertex_XYZ2_C));

				Vertex_XYZ2_C *pVertexData = (Vertex_XYZ2_C*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				const F32 *pPositions = (const F32*) pClusterResource->GetVertexDataElement(E_ElementType_Position);
				const F32 *pNormals = (const F32*) pClusterResource->GetVertexDataElement(E_ElementType_Normal);
				const U8 *pColours = (const U8*) pClusterResource->GetVertexDataElement(E_ElementType_Colour);

				for (U32 jV=0; jV<nVertices; ++jV)
				{
					SetVertexXYZ2(&pVertexData[jV], &pPositions[3*m_scratchData.vertexIndices[jV]], &pNormals[3*m_scratchData.vertexIndices[jV]]);
					SetVertexColour(&pVertexData[jV], &pColours[4*m_scratchData.vertexIndices[jV]]);
				}
				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			case C_VertexStream::E_VertexStreamUsage_UVs:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==nVertices*m_NumTexCoordSets*sizeof(Vertex_UV));

				Vertex_UV *pVertexData = (Vertex_UV*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);

				Ut::UtVector< Ut::DynamicVector<const F32*> > pUVs;
				pUVs.Resize(m_NumTexCoordSets);
				
				{
					//populate cache of individual UV data
					E_ElementType type = E_ElementType_TexCoord0;
					for (U32 jSet=0; jSet<m_NumTexCoordSets; ++jSet)
					{
						pUVs[jSet] = (const F32*) pClusterResource->GetVertexDataElement(type);
						assert(pUVs[jSet]);
						type = (AftkRender::E_ElementType)((U32)type << 1);
					}
				}

				for (U32 jV=0; jV<nVertices; ++jV)
				{
					for (U32 kUV=0; kUV<m_NumTexCoordSets; ++kUV)
						SetVertexTextureCoordinate(&pVertexData[jV*m_NumTexCoordSets + kUV], &pUVs[kUV][2*m_scratchData.vertexIndices[jV]]);
				}
				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			case C_VertexStream::E_VertexStreamUsage_Weights_PaletteIndices:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==nVertices*sizeof(Vertex_W4_Idx4));

				Vertex_W4_Idx4 *pVertexData = (Vertex_W4_Idx4*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				const F32 *pWeights = (const F32*) pClusterResource->GetVertexDataElement(E_ElementType_Weights);
				const U8 *pIndices = (const U8*) pClusterResource->GetVertexDataElement(E_ElementType_BlendIndices);

				for (U32 jV=0; jV<nVertices; ++jV)
					SetVertexWeightsAndIndices(&pVertexData[jV], &pWeights[4*m_scratchData.vertexIndices[jV]],&pIndices[4*m_scratchData.vertexIndices[jV]]);

				m_VertexStreams[iStream].vertexBuffer.Unlock();
				break;
			}

			case C_VertexStream::E_VertexStreamUsage_Tangents_Binormals:
			{
				ASSERT(m_VertexStreams[iStream].vertexBuffer.IsCreated());
				ASSERT(m_VertexStreams[iStream].vertexBuffer.GetSize()==nVertices*sizeof(Vertex_XYZ2));

				Vertex_XYZ2 *pVertexData = (Vertex_XYZ2*)m_VertexStreams[iStream].vertexBuffer.Lock();
				ASSERT(pVertexData);
				const F32 *pTangents	= (const F32*) pClusterResource->GetVertexDataElement(E_ElementType_Tangent);
				const F32 *pBinormals = (const F32*) pClusterResource->GetVertexDataElement(E_ElementType_Binormal);

				for (U32 jV=0; jV<nVertices; ++jV)
					SetVertexXYZ2(&pVertexData[jV], &pTangents[3*m_scratchData.vertexIndices[jV]], &pBinormals[3*m_scratchData.vertexIndices[jV]]);

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

bool C_MeshCluster::CreateIndexBuffers()
{
	const C_MeshResource* pMeshResource = ((C_MeshNode*)m_pGeometryNode)->GetMeshResource();
	
	if (!pMeshResource)
		return false;

	const C_MeshClusterResource *pClusterResource = pMeshResource->GetCluster(m_Cluster);
	if (!pClusterResource)
		return false;

	//D3DCAPS9 caps;
	//Display::Device()->GetD3DDevice()->GetDeviceCaps(&caps);
	//U32 maxPrimitives = caps.MaxPrimitiveCount;

	U32 nIndices = 0;
	for (S32 i=0; i<(S32)pClusterResource->GetNumIndexDataElements(); ++i)
	{
		const C_MeshClusterResource::IndexDataElement *pIndexElement = pClusterResource->GetIndexDataElement(i);
		switch (pIndexElement->type)
		{
		case E_PrimitiveType_PointList:
			nIndices += pIndexElement->count; break;
		case E_PrimitiveType_LineList:
			nIndices += pIndexElement->count; break;
		case E_PrimitiveType_LineStrip:	//convert linestrip to linelist
			nIndices += 2*(pIndexElement->count-1); break;
		case E_PrimitiveType_TriList:
			nIndices += pIndexElement->count; break;
		case E_PrimitiveType_TriStrip:	//convert tristrip to trilist
			nIndices += 3*(pIndexElement->count-2); break;
		case E_PrimitiveType_TriFan:	//convert trifan to trilist
			nIndices += 3*(pIndexElement->count-2); break;
		default:
			LOGERROR(g_Log, "Unsupported primitive type encounted while building index buffers");
		};
	}

	if (nIndices)
	{
		if (!Display::Device()->CreateIndexBuffer(nIndices*sizeof(U16), 0, Display::E_Format_Index16, &m_IndexBuffer))
		{
			LOGWARN(g_Log, "Failed to create index buffer\n");
			return false;
		}
	}

	return true;
}

bool C_MeshCluster::PopulateIndexBuffers()
{
	const C_MeshResource *pMeshResource = ((C_MeshNode*)m_pGeometryNode)->GetMeshResource();

	if (!pMeshResource)
		return false;

	const C_MeshClusterResource *pClusterResource = pMeshResource->GetCluster(m_Cluster);
	if (!pClusterResource)
		return false;


	if (!m_IndexBuffer.IsCreated())
		return false;

	U32 nIndexElements = pClusterResource->GetNumIndexDataElements();

	U16 *pIndexData = (U16*)m_IndexBuffer.Lock();
	if (!pIndexData)
		return false;

	U32 primitiveOffset = 0;
	U32 indexOffset = 0;
	for (U32 i=0; i<nIndexElements; ++i)
	{
		const C_MeshClusterResource::IndexDataElement *pIndexElement = pClusterResource->GetIndexDataElement(i);
		ASSERT(pIndexElement && pIndexElement->pData);

		U32 nPrimitives = GetNumPrimitives(pIndexElement->type, pIndexElement->count);
		U32 vertexOffset = 0;
		for (S32 j=0; j<m_scratchData.chunkVertexCount.Size(); ++j)
		{
			switch (pIndexElement->type)
			{
			case E_PrimitiveType_PointList:
			{
				U32 nIndices = 0;
				U32 offset = j*pClusterResource->GetNumVertices();
				for (U32 k=0; k<nPrimitives; ++k)
				{
					if (m_scratchData.primitiveChunkMap[primitiveOffset + k] == j)
					{
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[k]]);
						nIndices++;
					}
				}
				if (nIndices)
				{
					C_PrimitiveBatch batch = {vertexOffset, indexOffset, nIndices, pIndexElement->materialId, PrimitiveTypeLookup[E_PrimitiveType_PointList]};
					m_PrimitiveBatches.PushBack(batch);

					indexOffset += nIndices;
				}
				break;
			}
			case E_PrimitiveType_LineList:
			{
				U32 nIndices = 0;
				U32 offset = j*pClusterResource->GetNumVertices();
				for (U32 k=0; k<nPrimitives; ++k)
				{
					if (m_scratchData.primitiveChunkMap[primitiveOffset + k] == j)
					{
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[2*k]]);
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[2*k+1]]);
						nIndices += 2;
					}
				}
				if (nIndices)
				{
					C_PrimitiveBatch batch = {vertexOffset, indexOffset, nIndices/2, pIndexElement->materialId, PrimitiveTypeLookup[E_PrimitiveType_LineList]};
					m_PrimitiveBatches.PushBack(batch);

					indexOffset += nIndices;
				}
				break;
			}
			case E_PrimitiveType_LineStrip:	//convert linestrip to linelist
			{
				U32 nIndices = 0;
				U32 offset = j*pClusterResource->GetNumVertices();
				for (U32 k=0; k<nPrimitives; ++k)
				{
					if (m_scratchData.primitiveChunkMap[primitiveOffset + k] == j)
					{
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[k]]);
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[k+1]]);
						nIndices += 2;
					}
				}
				if (nIndices)
				{
					C_PrimitiveBatch batch = {vertexOffset, indexOffset, nIndices/2, pIndexElement->materialId, PrimitiveTypeLookup[E_PrimitiveType_LineList]};
					m_PrimitiveBatches.PushBack(batch);

					indexOffset += nIndices;
				}
				break;
			}
			case E_PrimitiveType_TriList:
			{
				U32 nIndices = 0;
				U32 offset = j*pClusterResource->GetNumVertices();
				for (U32 k=0; k<nPrimitives; ++k)
				{
					if (m_scratchData.primitiveChunkMap[primitiveOffset + k] == j)
					{
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[3*k]]);
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[3*k+1]]);
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[3*k+2]]);
						nIndices += 3;
					}
				}
				if (nIndices)
				{
					C_PrimitiveBatch batch = {vertexOffset, indexOffset, nIndices/3, pIndexElement->materialId, PrimitiveTypeLookup[E_PrimitiveType_TriList]};
					m_PrimitiveBatches.PushBack(batch);

					indexOffset += nIndices;
				}
				break;
			}
			case E_PrimitiveType_TriStrip:	//convert tristrip to trilist
			{
				U32 nIndices = 0;
				U32 offset = j*pClusterResource->GetNumVertices();
				for (U32 k=0; k<nPrimitives; ++k)
				{
					if (m_scratchData.primitiveChunkMap[primitiveOffset + k] == j)
					{
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[k]]);
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[k+1]]);
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[k+2]]);
						nIndices += 3;
					}
				}
				if (nIndices)
				{
					C_PrimitiveBatch batch = {vertexOffset , indexOffset, nIndices/3, pIndexElement->materialId, PrimitiveTypeLookup[E_PrimitiveType_TriList]};
					m_PrimitiveBatches.PushBack(batch);

					indexOffset += nIndices;
				}
				break;
			}
			case E_PrimitiveType_TriFan:	//convert trifan to trilist
			{
				U32 nIndices = 0;
				U32 offset = j*pClusterResource->GetNumVertices();
				for (U32 k=0; k<nPrimitives; ++k)
				{
					if (m_scratchData.primitiveChunkMap[primitiveOffset + k] == j)
					{
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[0]]);
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[k+1]]);
						*(pIndexData++) = (U16) (m_scratchData.vertexMap[offset + pIndexElement->pData[k+2]]);
						nIndices += 3;
					}
				}
				if (nIndices)
				{
					C_PrimitiveBatch batch = {vertexOffset, indexOffset, nIndices/3, pIndexElement->materialId, PrimitiveTypeLookup[E_PrimitiveType_TriFan]};
					m_PrimitiveBatches.PushBack(batch);

					indexOffset += nIndices;
				}
				break;
			}
			default:
				continue;
			};

			vertexOffset += m_scratchData.chunkVertexCount[j];
		}

		primitiveOffset += GetNumPrimitives(pIndexElement->type, pIndexElement->count);
	}

	m_IndexBuffer.Unlock();

	return true;
}



C_MeshNode::C_MeshNode(C_MeshResource *pMesh)
: C_GeometryNode()
, m_pMeshResource(pMesh)
{
}

C_MeshNode::~C_MeshNode()
{
}

void C_MeshNode::Clear()
{
	C_GeometryNode::Clear();
}

bool C_MeshNode::Build()
{
	if (!CreateGeometryClusters())
		return false;

	return true;
}

C_MeshResource* C_MeshNode::GetMeshResource()
{
	return m_pMeshResource;
}

const C_MeshResource* C_MeshNode::GetMeshResource() const
{
	return m_pMeshResource;
}

bool C_MeshNode::CreateGeometryClusters()
{
	m_GeometryClusters.Clear();
	for (U8 iCluster=0; iCluster<m_pMeshResource->GetNumClusters(); ++iCluster)
	{
		C_MeshCluster *pCluster = new C_MeshCluster(this, iCluster);
		m_GeometryClusters.PushBack(pCluster);
		if (!pCluster || !pCluster->Build())
		{
			LOGWARN(g_Log, "Failed to build geometry cluster");
			return false;
		}
	}

	return true;
}