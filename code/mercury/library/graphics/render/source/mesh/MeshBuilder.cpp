#include "stdafx.h"
#include "Render/Mesh/MeshBuilder.h"
#include "Render/Mesh/Mesh.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Render
{
namespace Mesh
{

C_MeshBuilder::C_MeshBuilder()
{
}

C_MeshBuilder::~C_MeshBuilder()
{
	m_VertexDeclarations.Clear();
	m_VertexBuffers.Clear();
	m_IndexBuffers.Clear();
	m_Textures.Clear();
	m_Materials.Clear();
	m_Batches.Clear();
}

S32 C_MeshBuilder::AddVertexDeclaration(Display::C_VertexDeclaration* pVertexDeclaration)
{
	for (S32 i=0; i<m_VertexDeclarations.Size(); i++)
		if (pVertexDeclaration == m_VertexDeclarations[i])
			return i;
	m_VertexDeclarations.PushBack(pVertexDeclaration);
	return m_VertexDeclarations.Size() - 1;
}

S32 C_MeshBuilder::AddVertexBuffer(Display::C_VertexBuffer* pVertexBuffer)
{
	for (S32 i=0; i<m_VertexBuffers.Size(); i++)
		if (pVertexBuffer == m_VertexBuffers[i])
			return i;
	m_VertexBuffers.PushBack(pVertexBuffer);
	return m_VertexBuffers.Size() - 1;
}

S32 C_MeshBuilder::AddIndexBuffer(Display::C_IndexBuffer* pIndexBuffer)
{
	for (S32 i=0; i<m_IndexBuffers.Size(); i++)
		if (pIndexBuffer == m_IndexBuffers[i])
			return i;
	m_IndexBuffers.PushBack(pIndexBuffer);
	return m_IndexBuffers.Size() - 1;
}

S32 C_MeshBuilder::AddTexture(const C_MeshBuilderTexture& texture)
{
	for (S32 i=0; i<m_Textures.Size(); i++)
		if (texture == m_Textures[i])
			return i;
	m_Textures.PushBack(texture);
	return m_Textures.Size() - 1;
}

S32 C_MeshBuilder::AddMaterial(const C_MeshBuilderMaterial& material)
{
	for (S32 i=0; i<m_Materials.Size(); i++)
		if (material == m_Materials[i])
			return i;
	m_Materials.PushBack(material);
	return m_Materials.Size() - 1;
}

S32 C_MeshBuilder::AddBatch(const C_MeshBuilderBatch& batch)
{
	m_Batches.PushBack(batch);
	return m_Batches.Size() - 1;
}

bool BuildMeshTestCube(C_Mesh* pMesh)
{
	assert(pMesh);

	// create the vertex declaration

	Display::C_VertexFormat vertexFormat;
	vertexFormat.AddElement(0, Display::E_VertexElementType_F32_3, Display::E_VertexElementUsage_Position, 0);
	
	pMesh->SetVertexDeclarationCount(1);
	pMesh->SetVertexDeclaration(0, vertexFormat);
	Display::C_VertexDeclaration* pVertexDeclaration = pMesh->GetVertexDeclaration(0);

	// create the vertex buffer

	struct C_Vertex
	{
		F32 x, y, z;
	};

	const U32 vertexCount	= 8;
	const U32 vertexSize	= vertexCount * sizeof(C_Vertex);
	
	pMesh->SetVertexBufferCount(1);
	Display::C_VertexBuffer* pVertexBuffer = pMesh->GetVertexBuffer(0);
	
	if (!Display::Device()->CreateVertexBuffer(vertexSize, 0, pVertexBuffer))
	{
		LOGERROR(g_Log, "Failed to create the vertex buffer\n");
		return false;
	}

	C_Vertex* pVertex = (C_Vertex*)pVertexBuffer->Lock();
	assert(pVertex);

	pVertex->x = -1.0f; pVertex->y = -1.0f; pVertex->z = -1.0f; pVertex++;
	pVertex->x = -1.0f; pVertex->y = -1.0f; pVertex->z =  1.0f; pVertex++;
	pVertex->x = -1.0f; pVertex->y =  1.0f; pVertex->z = -1.0f; pVertex++;
	pVertex->x = -1.0f; pVertex->y =  1.0f; pVertex->z =  1.0f; pVertex++;
	pVertex->x =  1.0f; pVertex->y = -1.0f; pVertex->z = -1.0f; pVertex++;
	pVertex->x =  1.0f; pVertex->y = -1.0f; pVertex->z =  1.0f; pVertex++;
	pVertex->x =  1.0f; pVertex->y =  1.0f; pVertex->z = -1.0f; pVertex++;
	pVertex->x =  1.0f; pVertex->y =  1.0f; pVertex->z =  1.0f; pVertex++;

	pVertexBuffer->Unlock();

	// create the index buffer

	const U32 indexCount	= 36;
	const U32 indexSize		= indexCount * sizeof(U16);

	pMesh->SetIndexBufferCount(1);
	Display::C_IndexBuffer* pIndexBuffer = pMesh->GetIndexBuffer(0);

	if (!Display::Device()->CreateIndexBuffer(indexSize, 0, Display::E_Format_Index16, pIndexBuffer))
	{
		LOGERROR(g_Log, "Failed to create the index buffer\n");
		return false;
	}

	U16* pIndex = (U16*)pIndexBuffer->Lock();
	assert(pIndex);

	*pIndex = 2; pIndex++; *pIndex = 3; pIndex++; *pIndex = 7; pIndex++;
	*pIndex = 2; pIndex++; *pIndex = 7; pIndex++; *pIndex = 6; pIndex++;
	*pIndex = 0; pIndex++; *pIndex = 5; pIndex++; *pIndex = 1; pIndex++;
	*pIndex = 0; pIndex++; *pIndex = 4; pIndex++; *pIndex = 5; pIndex++;
	*pIndex = 1; pIndex++; *pIndex = 3; pIndex++; *pIndex = 2; pIndex++;
	*pIndex = 1; pIndex++; *pIndex = 2; pIndex++; *pIndex = 0; pIndex++;
	*pIndex = 4; pIndex++; *pIndex = 6; pIndex++; *pIndex = 5; pIndex++;
	*pIndex = 5; pIndex++; *pIndex = 6; pIndex++; *pIndex = 7; pIndex++;
	*pIndex = 2; pIndex++; *pIndex = 6; pIndex++; *pIndex = 0; pIndex++;
	*pIndex = 0; pIndex++; *pIndex = 6; pIndex++; *pIndex = 4; pIndex++;
	*pIndex = 1; pIndex++; *pIndex = 7; pIndex++; *pIndex = 3; pIndex++;
	*pIndex = 1; pIndex++; *pIndex = 5; pIndex++; *pIndex = 7; pIndex++;

	pIndexBuffer->Unlock();

	// create the mateiral

	pMesh->SetMaterialCount(1);
	C_Material* pMaterial = pMesh->GetMaterial(0);

	pMaterial->m_VertexStride		= vertexFormat.GetStride(0);
	pMaterial->m_VertexDeclaration	= 0;
	pMaterial->m_pVertexShader		= &pMesh->m_Shaders.m_VertexShader[0];
	pMaterial->m_pPixelShader		= &pMesh->m_Shaders.m_PixelShader[0];

	// create the batch

	pMesh->SetBatchCount(1);
	C_Batch* pBatch = pMesh->GetBatch(0);

	pBatch->m_Material		= 0;
	pBatch->m_VertexBuffer	= 0;
	pBatch->m_IndexBuffer	= 0;
	pBatch->m_PrimitiveType	= Display::E_PrimitiveType_TriangleList;
	pBatch->m_VertexCount	= vertexCount;
	pBatch->m_IndexCount	= indexCount;

	// create the instance

	C_Instance* pInstance = pMesh->AddInstance();

	pInstance->m_Visible	= true;

	return true;
}

}; // namespace Mesh
}; // namespace Render
