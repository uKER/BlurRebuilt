#include "AftkRenderPch.h"
#include "common/GeometryNode.h"
#include "common/VertexFormat.h"

using namespace AftkRender;

C_GeometryCluster::C_GeometryCluster(C_GeometryNode* pNode)
: m_pGeometryNode(pNode)
, m_pVertexDeclaration(NULL)
, m_NumTexCoordSets(0)
{
}

C_GeometryCluster::~C_GeometryCluster()
{
	Clear();
}

void C_GeometryCluster::Clear()
{
	DestroyVertexBuffers();
	DestroyIndexBuffers();
	m_PrimitiveBatches.Clear();
}

bool C_GeometryCluster::Build()
{
	return true;
}

void C_GeometryCluster::RenderGeometry(U32 b) const
{
	const C_PrimitiveBatch& batch = m_PrimitiveBatches[b];

	U32 numVertices = 0;
	for (S32 i=0; i<m_VertexStreams.Size(); ++i)
	{
		const C_VertexStream& stream = m_VertexStreams[i];
		switch (stream.usage)
		{
		case C_VertexStream::E_VertexStreamUsage_Position_Normals:
			Display::Device()->SetVertexBuffer(stream.streamIndex, &stream.vertexBuffer, 0, sizeof(AftkRender::Vertex_XYZ2));
			numVertices = stream.vertexBuffer.GetSize() / sizeof(AftkRender::Vertex_XYZ2);
			break;
		case C_VertexStream::E_VertexStreamUsage_Position_Normals_Colour:
			Display::Device()->SetVertexBuffer(stream.streamIndex, &stream.vertexBuffer, 0, sizeof(AftkRender::Vertex_XYZ2_C));
			numVertices = stream.vertexBuffer.GetSize() / sizeof(AftkRender::Vertex_XYZ2_C);
			break;
		case C_VertexStream::E_VertexStreamUsage_UVs:
			Display::Device()->SetVertexBuffer(stream.streamIndex, &stream.vertexBuffer, 0, sizeof(AftkRender::Vertex_UV)*m_NumTexCoordSets);
			break;
		case C_VertexStream::E_VertexStreamUsage_Weights_PaletteIndices:
			Display::Device()->SetVertexBuffer(stream.streamIndex, &stream.vertexBuffer, 0, sizeof(AftkRender::Vertex_W4_Idx4));
			break;
		case C_VertexStream::E_VertexStreamUsage_Tangents_Binormals:
			Display::Device()->SetVertexBuffer(stream.streamIndex, &stream.vertexBuffer, 0, sizeof(AftkRender::Vertex_XYZ2));
			break;
		default:
			//ASSERT(false && "Invalid vertex data stream");
			break;
		}
	}

	assert(m_pVertexDeclaration);
	Display::Device()->SetVertexDeclaration(m_pVertexDeclaration);

	if (m_IndexBuffer.IsCreated())
	{
		Display::Device()->SetIndexBuffer(&m_IndexBuffer);
		Display::Device()->DrawIndexedPrimitive(batch.primitiveType, batch.vertexOffset, 0, numVertices, batch.indexOffset, Display::IndexCount(batch.primitiveType, batch.primitiveCount));
	}
	else
	{
		Display::Device()->DrawPrimitive(batch.primitiveType, batch.vertexOffset, Display::IndexCount(batch.primitiveType, batch.primitiveCount));
	}

}

U32 C_GeometryCluster::GetNumVertexStreams() const
{
	return m_VertexStreams.Size();
}

const C_VertexStream& C_GeometryCluster::GetVertexStream(U32 i) const
{
	return m_VertexStreams[i];
}

U32 C_GeometryCluster::GetNumPrimitiveBatches() const
{
	return m_PrimitiveBatches.Size();
}

const C_PrimitiveBatch& C_GeometryCluster::GetPrimitiveBatch(U32 i) const
{
	return m_PrimitiveBatches[i];
}

const C_GeometryNode* C_GeometryCluster::GetGeometryNode() const
{
	return m_pGeometryNode;
}

const Display::C_VertexDeclaration* C_GeometryCluster::GetVertexDeclaration() const
{
	return m_pVertexDeclaration;
}

void C_GeometryCluster::DestroyVertexBuffers()
{
	for (S32 i=0; i<m_VertexStreams.Size(); ++i)
		Display::Device()->DestroyVertexBuffer(&m_VertexStreams[i].vertexBuffer);
}

void C_GeometryCluster::DestroyIndexBuffers()
{
	Display::Device()->DestroyIndexBuffer(&m_IndexBuffer);
}







C_GeometryNode::C_GeometryNode()
{
}

C_GeometryNode::~C_GeometryNode()
{
	Clear();
}

void C_GeometryNode::Clear()
{
	DestroyGeometryClusters();
}

bool C_GeometryNode::Build()
{
	return true;
}



U32 C_GeometryNode::GetNumClusters() const
{
	return m_GeometryClusters.Size();
}

const C_GeometryCluster* C_GeometryNode::GetClusterPtr(U32 index) const
{
	return m_GeometryClusters[index];
}


void C_GeometryNode::DestroyGeometryClusters()
{
	for (S32 i=0; i<m_GeometryClusters.Size(); ++i)
		delete m_GeometryClusters[i];
	
	m_GeometryClusters.Clear();
}