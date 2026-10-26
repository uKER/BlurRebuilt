#include "stdafx.h"
#include "Render/Mesh/Mesh.h"
#include "Render/Shaders/Constants.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

namespace Render
{
namespace Mesh
{

C_Shaders C_Mesh::m_Shaders;

C_Material::C_Material() :
	m_VertexStride(0),
	m_VertexDeclaration(-1),
	m_pVertexShader(NULL),
	m_pPixelShader(NULL)
{
}

C_Material::~C_Material()
{
}

C_Batch::C_Batch() :
	m_Material(-1),
	m_VertexBuffer(-1),
	m_IndexBuffer(-1),
	m_PrimitiveType(Display::E_PrimitiveType_TriangleList),
	m_BaseVertex(0),
	m_StartVertex(0),
	m_VertexCount(0),
	m_StartIndex(0),
	m_IndexCount(0)
{
}

C_Batch::~C_Batch()
{
}

C_Instance::C_Instance() : m_Visible(true)
{
	Display::MatrixSetIdentity(&m_WorldMatrix);
}

C_Instance::~C_Instance()
{
}

C_Mesh::C_Mesh() : Display::I_Renderer("Mesh")
{
}

C_Mesh::~C_Mesh()
{
	DestroyRenderer();
}

void C_Mesh::SetVertexDeclarationCount(const S32 count)
{
	assert(m_VertexDeclarations.Size() == 0);
	m_VertexDeclarations.Resize(count);
}

void C_Mesh::SetVertexDeclaration(const S32 index, const Display::C_VertexFormat& vertexFormat)
{
	assert(index >= 0 && index < m_VertexDeclarations.Size());
	m_VertexDeclarations[index] = Display::Device()->CreateVertexDeclaration(vertexFormat);
}

void C_Mesh::SetVertexDeclaration(const S32 index, Display::C_VertexDeclaration* pVertexDeclaration)
{
	assert(index >= 0 && index < m_VertexDeclarations.Size());
	m_VertexDeclarations[index] = pVertexDeclaration;
}

void C_Mesh::SetVertexBufferCount(const S32 count)
{
	assert(m_VertexBuffers.Size() == 0);
	m_VertexBuffers.Resize(count);
}

void C_Mesh::SetIndexBufferCount(const S32 count)
{
	assert(m_IndexBuffers.Size() == 0);
	m_IndexBuffers.Resize(count);
}

void C_Mesh::SetMaterialCount(const S32 count)
{
	assert(m_Materials.Size() == 0);
	m_Materials.Resize(count);
}

void C_Mesh::SetBatchCount(const S32 count)
{
	assert(m_Batches.Size() == 0);
	m_Batches.Resize(count);
}

C_Instance* C_Mesh::AddInstance()
{
	C_Instance* pInstance = new C_Instance();
	m_Instances.PushBack(pInstance);
	return pInstance;
}

bool C_Mesh::OnCreate()
{
	if (!m_Shaders.CreateShaders())
		return false;

	return true;
}

void C_Mesh::OnDestroy()
{
	m_Shaders.DestroyShaders();

	m_VertexDeclarations.Clear();
	m_Materials.Clear();
	m_Batches.Clear();

	for (S32 i=0; i<m_VertexBuffers.Size(); i++)
		Display::Device()->Destroy(&m_VertexBuffers[i]);
	m_VertexBuffers.Clear();

	for (S32 i=0; i<m_IndexBuffers.Size(); i++)
		Display::Device()->Destroy(&m_IndexBuffers[i]);
	m_IndexBuffers.Clear();
	
	for (S32 i=0; i<m_Instances.Size(); i++)
		delete m_Instances[i];
	m_Instances.Clear();
}

void C_Mesh::Render(const E_Pass pass)
{
	PERF_EVENT("RenderMesh");

	//Display::Device()->SetFillMode(Display::E_FillMode_Wireframe);

	for (S32 instanceIndex=0; instanceIndex<m_Instances.Size(); instanceIndex++)
	{
		const C_Instance* pInstance = m_Instances[instanceIndex];
		
		if (!pInstance->m_Visible)
			continue;

		Display::Device()->SetVertexShaderConstantF(CV_DISPLAY_WORLD_MATRIX, (F32*)&pInstance->m_WorldMatrix, 4);

		for (S32 batchIndex=0; batchIndex<m_Batches.Size(); batchIndex++)
		{
			const C_Batch& batch = m_Batches[batchIndex];

			assert(batch.m_Material != -1);
			const C_Material& material = m_Materials[batch.m_Material];

			if (pass == E_Pass_World)
			{
				if (material.m_pVertexShader)
					Display::Device()->SetVertexShader(material.m_pVertexShader);

				if (material.m_pPixelShader)
					Display::Device()->SetPixelShader(material.m_pPixelShader);
			}

			assert(material.m_VertexDeclaration != -1);
			const Display::C_VertexDeclaration* pVertexDeclaration = m_VertexDeclarations[material.m_VertexDeclaration];
			Display::Device()->SetVertexDeclaration(pVertexDeclaration);

			assert(batch.m_VertexBuffer != -1);
			const Display::C_VertexBuffer& vertexBuffer = m_VertexBuffers[batch.m_VertexBuffer];
			Display::Device()->SetVertexBuffer(0, &vertexBuffer, 0, material.m_VertexStride);

			assert(batch.m_IndexBuffer != -1);
			const Display::C_IndexBuffer& indexBuffer = m_IndexBuffers[batch.m_IndexBuffer];
			Display::Device()->SetIndexBuffer(&indexBuffer);

			assert(batch.m_IndexCount > 0);
			Display::Device()->DrawIndexedPrimitive(batch.m_PrimitiveType, batch.m_BaseVertex, batch.m_StartVertex, batch.m_VertexCount, batch.m_StartIndex, batch.m_IndexCount);
		}
	}

	//Display::Device()->SetFillMode(Display::E_FillMode_Default);
}

}; // namespace Mesh
}; // namespace Render
