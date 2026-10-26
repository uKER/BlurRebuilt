#ifndef RENDER_MESH2_H
#define RENDER_MESH2_H

#ifdef _MSC_VER
#pragma once
#endif

#if PLATFORM_SPU
#include <Display2/Context/GCMContextSPU.h>
#else
#include <Display2/Context/Context.h>
#endif

namespace Render
{

// C_Mesh2Sampler

struct STOREALIGN16 C_Mesh2Sampler
{
	C_Mesh2Sampler();

	U32													m_Number;
	S32													m_Texture;
};

// C_Mesh2Material

struct STOREALIGN16 C_Mesh2Material
{
	~C_Mesh2Material();

	void Clear();

	Btl::Vector<C_Mesh2Sampler>							m_SamplerList;
};

// C_Mesh2Batch

struct STOREALIGN16 C_Mesh2Batch
{
	C_Mesh2Batch();

	S32													m_VertexDeclaration;
	S32													m_VertexBuffer;
	S32													m_IndexBuffer;
	S32													m_Material;
	Display2::E_PrimitiveType							m_PrimitiveType;
	U32													m_VertexOffset;
	U32													m_VertexStart;
	U32													m_VertexCount;
	U32													m_IndexStart;
	U32													m_IndexCount;
};

// C_Mesh2

struct STOREALIGN16 C_Mesh2
{
	~C_Mesh2();

	void Clear();

	Btl::Vector<Display2::C_VertexDeclaration>			m_VertexDeclarationList;
	Btl::Vector<Display2::C_VertexBuffer>				m_VertexBufferList;
	Btl::Vector<Display2::C_IndexBuffer>				m_IndexBufferList;
	Btl::Vector<Display2::C_Texture>					m_TextureList;
	Btl::Vector<C_Mesh2Material>						m_MaterialList;
	Btl::Vector<C_Mesh2Batch>							m_BatchList;
};

} // namespace Render

#endif // RENDER_MESH2_H
