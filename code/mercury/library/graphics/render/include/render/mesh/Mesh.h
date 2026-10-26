#ifndef RENDER_MESH_H
#define RENDER_MESH_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Render/Mesh/MeshShaders.h"

namespace Render
{
namespace Mesh
{

enum E_Pass
{
	E_Pass_Depth,
	E_Pass_World,
};

class C_Material
{
public:

	C_Material();
	~C_Material();

public:

	U32										m_VertexStride;
	S32										m_VertexDeclaration;
	Display::C_VertexShader*				m_pVertexShader;
	Display::C_PixelShader*				m_pPixelShader;
};

class C_Batch
{
public:

	C_Batch();
	~C_Batch();

public:

	S32										m_Material;
	S32										m_VertexBuffer;
	S32										m_IndexBuffer;
	Display::E_PrimitiveType				m_PrimitiveType;
	U32										m_BaseVertex;
	U32										m_StartVertex;
	U32										m_VertexCount;
	U32										m_StartIndex;
	U32										m_IndexCount;
};

class C_Instance
{
public:

	C_Instance();
	~C_Instance();

public:

	bool									m_Visible;
	Display::C_Matrix					m_WorldMatrix;
};

typedef Ut::UtVector<Ut::DynamicVector<Display::C_VertexDeclaration*> >	C_VectorVertexDeclarations;
typedef Ut::UtVector<Ut::DynamicVector<Display::C_VertexBuffer> >		C_VectorVertexBuffers;
typedef Ut::UtVector<Ut::DynamicVector<Display::C_IndexBuffer> >			C_VectorIndexBuffers;
typedef Ut::UtVector<Ut::DynamicVector<C_Material> >						C_VectorMaterials;
typedef Ut::UtVector<Ut::DynamicVector<C_Batch> >							C_VectorBatches;
typedef Ut::UtVector<Ut::DynamicVector<C_Instance*> >						C_VectorInstances;

class C_Mesh : public Display::I_Renderer
{
public:

	C_Mesh();
	~C_Mesh();

	const S32 GetVertexDeclarationCount() const;
	Display::C_VertexDeclaration* GetVertexDeclaration(const S32 index);
	void SetVertexDeclarationCount(const S32 count);
	void SetVertexDeclaration(const S32 index, const Display::C_VertexFormat& vertexFormat);
	void SetVertexDeclaration(const S32 index, Display::C_VertexDeclaration* pVertexDeclaration);

	const S32 GetVertexBufferCount() const;
	Display::C_VertexBuffer* GetVertexBuffer(const S32 index);
	void SetVertexBufferCount(const S32 count);
	
	const S32 GetIndexBufferCount() const;
	Display::C_IndexBuffer* GetIndexBuffer(const S32 index);
	void SetIndexBufferCount(const S32 count);

	const S32 GetMaterialCount() const;
	C_Material* GetMaterial(const S32 index);
	void SetMaterialCount(const S32 count);

	const S32 GetBatchCount() const;
	C_Batch* GetBatch(const S32 index);
	void SetBatchCount(const S32 count);

	const S32 GetInstanceCount() const;
	C_Instance* GetInstance(const S32 index);
	C_Instance* AddInstance();

	bool OnCreate();
	void OnDestroy();

	void Render(const E_Pass pass = E_Pass_World);

public:

	static C_Shaders						m_Shaders;

private:

	C_VectorVertexDeclarations				m_VertexDeclarations;
	C_VectorVertexBuffers					m_VertexBuffers;
	C_VectorIndexBuffers					m_IndexBuffers;
	C_VectorMaterials						m_Materials;
	C_VectorBatches							m_Batches;
	C_VectorInstances						m_Instances;
};

}; // namespace Mesh
}; // namespace Render

#include "Mesh.inl"

#endif // RENDER_MESH_H
