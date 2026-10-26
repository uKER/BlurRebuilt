#include "StdAfx.h"
#include "Display2/Context/Context.h"

namespace Display2
{

//------------------------------------------------------------------------------
// Viewport

void SetViewportX(C_Viewport* pViewport, const U32 x)
{
	BIZ_ASSERT(pViewport);
	pViewport->SetX(x);
}

void SetViewportY(C_Viewport* pViewport, const U32 y)
{
	BIZ_ASSERT(pViewport);
	pViewport->SetY(y);
}

void SetViewportWidth(C_Viewport* pViewport, const U32 width)
{
	BIZ_ASSERT(pViewport);
	pViewport->SetWidth(width);
}

void SetViewportHeight(C_Viewport* pViewport, const U32 height)
{
	BIZ_ASSERT(pViewport);
	pViewport->SetHeight(height);
}

void SetViewportMinZ(C_Viewport* pViewport, const F32 minZ)
{
	BIZ_ASSERT(pViewport);
	pViewport->SetMinZ(minZ);
}

void SetViewportMaxZ(C_Viewport* pViewport, const F32 maxZ)
{
	BIZ_ASSERT(pViewport);
	pViewport->SetMaxZ(maxZ);
}

//------------------------------------------------------------------------------
// Vertex element

void SetVertexElementStream(C_VertexElement* pVertexElement, const U8 stream)
{
	BIZ_ASSERT(pVertexElement);
	pVertexElement->SetStream(stream);
}

void SetVertexElementOffset(C_VertexElement* pVertexElement, const U32 offset)
{
	BIZ_ASSERT(pVertexElement);
	pVertexElement->SetOffset(offset);
}

void SetVertexElementType(C_VertexElement* pVertexElement, const U8 type)
{
	BIZ_ASSERT(pVertexElement);
	pVertexElement->SetType(type);
}

void SetVertexElementUsage(C_VertexElement* pVertexElement, const U8 usage)
{
	BIZ_ASSERT(pVertexElement);
	pVertexElement->SetUsage(usage);
}

void SetVertexElementUsageIndex(C_VertexElement* pVertexElement, const U8 usageIndex)
{
	BIZ_ASSERT(pVertexElement);
	pVertexElement->SetUsageIndex(usageIndex);
}

U32 GetVertexElementSize(const C_VertexElement* pVertexElement)
{
	BIZ_ASSERT(pVertexElement);
	return pVertexElement->GetSize();
}

U32 GetVertexElementCount(const C_VertexElement* pVertexElement)
{
	BIZ_ASSERT(pVertexElement);
	return pVertexElement->GetCount();
}

//------------------------------------------------------------------------------
// Vertex declaration

U32 GetVertexDeclarationStride(const C_VertexDeclaration* pVertexDeclaration, const U32 stream)
{
	BIZ_ASSERT(pVertexDeclaration);
	return pVertexDeclaration->GetStride(stream);
}

//------------------------------------------------------------------------------
// Vertex buffer

void* LockVertexBuffer(C_VertexBuffer* pVertexBuffer, const U32 offset, const U32 size, const U32 flags)
{
	BIZ_ASSERT(pVertexBuffer);
	return pVertexBuffer->Lock(offset, size, flags);
}

void UnlockVertexBuffer(C_VertexBuffer* pVertexBuffer)
{
	BIZ_ASSERT(pVertexBuffer);
	return pVertexBuffer->Unlock();
}

//------------------------------------------------------------------------------
// Index buffer

void* LockIndexBuffer(C_IndexBuffer* pIndexBuffer, const U32 offset, const U32 size, const U32 flags)
{
	BIZ_ASSERT(pIndexBuffer);
	return pIndexBuffer->Lock(offset, size, flags);
}

void UnlockIndexBuffer(C_IndexBuffer* pIndexBuffer)
{
	BIZ_ASSERT(pIndexBuffer);
	return pIndexBuffer->Unlock();
}

//------------------------------------------------------------------------------
// Vertex shader

C_VertexShaderParam GetVertexShaderParamFromName(C_VertexShader* pVertexShader, const char* name)
{
	BIZ_ASSERT(pVertexShader);
	return pVertexShader->GetParamFromName(name);
}

//------------------------------------------------------------------------------
// Pixel shader

C_PixelShaderParam GetPixelShaderParamFromName(C_PixelShader* pPixelShader, const char* name)
{
	BIZ_ASSERT(pPixelShader);
	return pPixelShader->GetParamFromName(name);
}

//------------------------------------------------------------------------------
// Surface

C_LockedRect LockSurfaceRect(C_Surface* pSurface, const U32 flags)
{
	BIZ_ASSERT(pSurface);
	return pSurface->LockRect(flags);
}

void UnlockSurfaceRect(C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	pSurface->UnlockRect();
}

//------------------------------------------------------------------------------
// Texture

C_LockedRect LockTextureRect2D(C_Texture* pTexture, const U32 level, const U32 flags)
{
	BIZ_ASSERT(pTexture);
	return pTexture->LockRect2D(level, flags);
}

void UnlockTextureRect2D(C_Texture* pTexture, const U32 level)
{
	BIZ_ASSERT(pTexture);
	pTexture->UnlockRect2D(level);
}

C_LockedBox LockTextureBox3D(C_Texture* pTexture, const U32 level, const U32 flags)
{
	BIZ_ASSERT(pTexture);
	return pTexture->LockBox3D(level, flags);
}

void UnlockTextureBox3D(C_Texture* pTexture, const U32 level)
{
	BIZ_ASSERT(pTexture);
	pTexture->UnlockBox3D(level);
}

C_LockedRect LockTextureRectCube(C_Texture* pTexture, const E_CubeMapFace face, const U32 level, const U32 flags)
{
	BIZ_ASSERT(pTexture);
	return pTexture->LockRectCube(face, level, flags);
}

void UnlockTextureRectCube(C_Texture* pTexture, const E_CubeMapFace face, const U32 level)
{
	BIZ_ASSERT(pTexture);
	pTexture->UnlockRectCube(face, level);
}

//------------------------------------------------------------------------------
// Vertex format


C_VertexFormat::C_VertexFormat()
{
	m_ElementCount		= 0;

	for(S32 i=0; i<m_MaxElementCount; i++)
		m_Elements[i]	= GPU_VERTEX_ELEMENT_END();
}

C_VertexFormat::~C_VertexFormat()
{
}

U32 C_VertexFormat::GetStride(const U32 stream) const
{
	U32 stride = 0;
	
	for(S32 i=0; i<m_ElementCount; i++)
	{
		if(stream == m_Elements[i].GetStream() || stream == UINT_MAX)
		{
			stride += m_Elements[i].GetSize();
		}
	}

	return stride;
}

void C_VertexFormat::AddElement(const U16 stream, const U32 type, const U8 usage, const U8 usageIndex)
{
	const U32 offset = GetStride(stream);

	AddElement(stream, offset, type, usage, usageIndex);
}

void C_VertexFormat::AddElement(const U16 stream, const U32 offset, const U32 type, const U8 usage, const U8 usageIndex)
{
	BIZ_ASSERT(m_ElementCount < m_MaxElementCount-1);
	
	m_Elements[m_ElementCount].SetStream(stream);
	m_Elements[m_ElementCount].SetOffset(offset);
	m_Elements[m_ElementCount].SetType(type);
	m_Elements[m_ElementCount].SetUsage(usage);
	m_Elements[m_ElementCount].SetUsageIndex(usageIndex);
	
	m_ElementCount++;

	m_Elements[m_ElementCount] = GPU_VERTEX_ELEMENT_END();
}

} // namespace Display2
