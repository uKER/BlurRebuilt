#include "stdafx.h"
#include "Mesh2/Mesh/Mesh2.h"

namespace Render
{

// C_Mesh2Sampler

C_Mesh2Sampler::C_Mesh2Sampler() :
	m_Number(0),
	m_Texture(-1)
{
}

// C_Mesh2Material

C_Mesh2Material::~C_Mesh2Material()
{
}

void C_Mesh2Material::Clear()
{
	m_SamplerList.Clear();
}

// C_Mesh2Batch

C_Mesh2Batch::C_Mesh2Batch() :
	m_VertexDeclaration(-1),
	m_VertexBuffer(-1),
	m_IndexBuffer(-1),
	m_Material(-1),
	m_PrimitiveType(Display2::E_PrimitiveType_TriangleList),
	m_VertexOffset(0),
	m_VertexStart(0),
	m_VertexCount(0),
	m_IndexStart(0),
	m_IndexCount(0)
{
}

// C_Mesh2

C_Mesh2::~C_Mesh2()
{
}

void C_Mesh2::Clear()
{
	for(S32 i=0; i<m_MaterialList.Size(); i++)
		m_MaterialList[i].Clear();

	m_VertexDeclarationList.Clear();
	m_VertexBufferList.Clear();
	m_IndexBufferList.Clear();
	m_TextureList.Clear();
	m_MaterialList.Clear();
	m_BatchList.Clear();
}

} // namespace Render
