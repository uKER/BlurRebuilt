#ifndef RENDER_MESHBUILDER_H
#define RENDER_MESHBUILDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Render
{
namespace Mesh
{

enum E_MeshBuilderTexture
{
	E_MeshBuilderTexture_Albedo,
	E_MeshBuilderTexture_Lightmap,
	E_MeshBuilderTexture_Normalmap,
	E_MeshBuilderTexture_Specular,
};

struct C_MeshBuilderTexture
{
	E_MeshBuilderTexture	m_Type;
	S32						m_Texture;

	bool operator==(const C_MeshBuilderTexture& other) const
	{
		if (m_Type != other.m_Type)
			return false;
		if (m_Texture != other.m_Texture)
			return false;
		return true;
	}

	bool operator!=(const C_MeshBuilderTexture& other) const
	{
		if (m_Type != other.m_Type)
			return true;
		if (m_Texture != other.m_Texture)
			return true;
		return false;
	}

	const C_MeshBuilderTexture& operator=(const C_MeshBuilderTexture& other)
	{
		m_Type		= other.m_Type;
		m_Texture	= other.m_Texture;
		return *this;
	}
};

struct C_MeshBuilderMaterial
{
	U32						m_VertexStride;
	S32						m_VertexDeclaration;
	S32						m_VertexShader;
	S32						m_PixelShader;

	//Ut::UtVector<Ut::DynamicVector<C_MeshBuilderTexture> > m_Textures;

	bool operator==(const C_MeshBuilderMaterial& other) const
	{
		if (m_VertexStride != other.m_VertexStride)
			return false;
		if (m_VertexDeclaration	!= other.m_VertexDeclaration)
			return false;
		if (m_VertexShader != other.m_VertexShader)
			return false;
		if (m_PixelShader	!= other.m_PixelShader)
			return false;
		//for (S32 i=0; i<m_Textures.Size(); i++)
			//if (m_Textures[i] != other.m_Textures[i])
				//return false;
		return true;
	}

	const C_MeshBuilderMaterial& operator=(const C_MeshBuilderMaterial& other)
	{
		m_VertexStride		= other.m_VertexStride;
		m_VertexDeclaration	= other.m_VertexDeclaration;
		m_VertexShader		= other.m_VertexShader;
		m_PixelShader		= other.m_PixelShader;
		return *this;
	}
};

struct C_MeshBuilderBatch
{
	S32 m_Material;
	S32 m_VertexBuffer;
	S32 m_IndexBuffer;
	U32 m_PrimitiveType;
	U32 m_BaseVertex;
	U32 m_StartVertex;
	U32 m_VertexCount;
	U32 m_StartIndex;
	U32 m_IndexCount;
};

class C_MeshBuilder
{
public:

	C_MeshBuilder();
	~C_MeshBuilder();

	S32 AddVertexDeclaration(Display::C_VertexDeclaration* pVertexDeclaration);
	S32 AddVertexBuffer(Display::C_VertexBuffer* pVertexBuffer);
	S32 AddIndexBuffer(Display::C_IndexBuffer* pIndexBuffer);
	S32 AddTexture(const C_MeshBuilderTexture& texture);
	S32 AddMaterial(const C_MeshBuilderMaterial& material);
	S32 AddBatch(const C_MeshBuilderBatch& batch);

public:

	Ut::UtVector<Ut::DynamicVector<Display::C_VertexDeclaration*> >	m_VertexDeclarations;
	Ut::UtVector<Ut::DynamicVector<Display::C_VertexBuffer*> >		m_VertexBuffers;
	Ut::UtVector<Ut::DynamicVector<Display::C_IndexBuffer*> >		m_IndexBuffers;
	Ut::UtVector<Ut::DynamicVector<C_MeshBuilderTexture> >				m_Textures;
	Ut::UtVector<Ut::DynamicVector<C_MeshBuilderMaterial> >				m_Materials;
	Ut::UtVector<Ut::DynamicVector<C_MeshBuilderBatch> >				m_Batches;
};

class C_Mesh;

bool BuildMeshTestCube(C_Mesh* pMesh);

}; // namespace Mesh
}; // namespace Render

#endif // RENDER_MESHBUILDER_H
