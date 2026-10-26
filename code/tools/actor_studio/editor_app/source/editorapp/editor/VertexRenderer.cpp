#include "stdafx.h"

#include <EditorApp/Editor/DgNodeSelection.h>
#include <EditorApp/Editor/EditorRenderer.h>
#include <EditorApp/Editor/VertexRenderer.h>

namespace Editor
{
C_VertexRenderer::C_VertexRenderer(int size)
{
	m_VertexBufferSize = size;
	m_BeginVerticesSize = size;
	m_BeginVerticesStart = 0;
	m_BeginVerticesUsage = 0;
	m_MaxBeginVerticesUsage = 0;
	m_BeginVerticesLocked = false;
}

C_VertexRenderer::~C_VertexRenderer()
{
}

bool C_VertexRenderer::OnCreate()
{
	return I_EditorRenderer::OnCreate();
}

bool C_VertexRenderer::OnCreateVolatile()
{
	if(!m_DynamicVertexBuffer.Create(m_VertexBufferSize))
		assert(false);

	m_MaxBeginVerticesUsage = m_VertexBufferSize;


	return I_EditorRenderer::OnCreateVolatile();
}

void C_VertexRenderer::OnDestroy()
{
	I_EditorRenderer::OnDestroy();
}

void C_VertexRenderer::OnDestroyVolatile()
{
	m_DynamicVertexBuffer.Destroy();
	I_EditorRenderer::OnDestroyVolatile();
}

void C_VertexRenderer::Clear()
{
}

void* C_VertexRenderer::BeginVertices(const Display::E_PrimitiveType primitiveType, const U32 vertexCount, const U32 vertexStride, Display::C_VertexDeclaration* pVertexDeclaration, const bool checkUsage)
{
	assert(!m_BeginVerticesLocked);

	U32 size = vertexCount * vertexStride;

	if(checkUsage)
	{
		if((m_BeginVerticesUsage+size) > m_MaxBeginVerticesUsage)
		{
			int increaseRemainder = ((m_BeginVerticesUsage+size) - m_MaxBeginVerticesUsage) % 1048576;
			int increaseMegabytes = ((((m_BeginVerticesUsage+size) - m_MaxBeginVerticesUsage) - increaseRemainder) / 1048576) + 1;
			int increaseBytes = increaseMegabytes * 1048576;
			m_DynamicVertexBuffer.Destroy();
			m_VertexBufferSize += increaseBytes; // Increase buffer size by the correct amount
			m_MaxBeginVerticesUsage = m_VertexBufferSize;
			m_DynamicVertexBuffer.Create(m_VertexBufferSize);
		}
	}
	
	void* pVertices = m_DynamicVertexBuffer.Begin(primitiveType, vertexCount, vertexStride, pVertexDeclaration);
	
	if(pVertices)
	{
		m_BeginVerticesLocked = true;
		m_BeginVerticesUsage += size;

		#ifndef CFG_GOLD
		m_BeginVerticesSize = vertexCount * vertexStride;
		m_BeginVerticesStart = (U8*)pVertices;
		#endif

		return pVertices;
	}
	else
	{
		LOGFATAL(g_Log, "Begin vertices returned null! Failed to render count '%i' stride '%i' size '%i' primitive '%i'\n", vertexCount, vertexStride, vertexCount * vertexStride, primitiveType);

		return NULL;
	}
	return NULL;
}

void C_VertexRenderer::EndVertices(void* pVertex)
{
	assert(m_BeginVerticesLocked);

	m_DynamicVertexBuffer.End();
	
	Display::Device()->SetVertexBuffer(0, m_DynamicVertexBuffer.GetVertexBuffer(), m_DynamicVertexBuffer.GetOffset() - m_DynamicVertexBuffer.GetVertexSize(), m_DynamicVertexBuffer.GetVertexStride());
	Display::Device()->SetVertexDeclaration(m_DynamicVertexBuffer.GetVertexDeclaration());
	
	Display::Device()->DrawPrimitive(m_DynamicVertexBuffer.GetPrimitiveType(), 0, m_DynamicVertexBuffer.GetVertexCount());

	m_BeginVerticesLocked = false;

	#ifndef CFG_GOLD
	if(pVertex)
	{
		U32 usedEnd = (U32)pVertex;
		U32 actualEnd = (U32)m_BeginVerticesStart+m_BeginVerticesSize;
		if(usedEnd > actualEnd)
		{
			LOGFATAL(g_Log, "C_PCD3D9Device::EndVertices - usedEnd > actualEnd by %i\n", usedEnd-actualEnd);
		}
		else
		if(usedEnd < actualEnd)
		{
			LOGWARN(g_Log, "C_PCD3D9Device::EndVertices - usedEnd < actualEnd by %i\n", actualEnd-usedEnd);
		}
	}
	m_BeginVerticesSize = 0;
	m_BeginVerticesStart = NULL;
	#endif
}

void C_VertexRenderer::Update(AftkRender::E_RendererPass pass)
{
}

void C_VertexRenderer::GetVisible(
			const RECT& region, 
			C_DgNodeSelection& selection)
{
}

void C_VertexRenderer::Render()
{
	// Swap to the new dynamic vertex buffer
	m_DynamicVertexBuffer.Swap();

	// Reset begin vertices usage
	m_BeginVerticesUsage = 0;
}

}