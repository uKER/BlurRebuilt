#ifndef DISPLAY2_CONTEXT_INL
#define DISPLAY2_CONTEXT_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{
	
//------------------------------------------------------------------------------
// Viewport

INLINE U32 GetViewportX(const C_Viewport* pViewport)
{
	BIZ_ASSERT(pViewport);
	return pViewport->GetX();
}

INLINE U32 GetViewportY(const C_Viewport* pViewport)
{
	BIZ_ASSERT(pViewport);
	return pViewport->GetY();
}

INLINE U32 GetViewportWidth(const C_Viewport* pViewport)
{
	BIZ_ASSERT(pViewport);
	return pViewport->GetWidth();
}

INLINE U32 GetViewportHeight(const C_Viewport* pViewport)
{
	BIZ_ASSERT(pViewport);
	return pViewport->GetHeight();
}

INLINE F32 GetViewportMinZ(const C_Viewport* pViewport)
{
	BIZ_ASSERT(pViewport);
	return pViewport->GetMinZ();
}

INLINE F32 GetViewportMaxZ(const C_Viewport* pViewport)
{
	BIZ_ASSERT(pViewport);
	return pViewport->GetMaxZ();
}

//------------------------------------------------------------------------------
// Vertex element

INLINE U16 GetVertexElementStream(const C_VertexElement* pVertexElement)
{
	BIZ_ASSERT(pVertexElement);
	return pVertexElement->GetStream();
}

INLINE U32 GetVertexElementOffset(const C_VertexElement* pVertexElement)
{
	BIZ_ASSERT(pVertexElement);
	return pVertexElement->GetOffset();
}

INLINE U32 GetVertexElementType(const C_VertexElement* pVertexElement)
{
	BIZ_ASSERT(pVertexElement);
	return pVertexElement->GetType();
}

INLINE U8 GetVertexElementUsage(const C_VertexElement* pVertexElement)
{
	BIZ_ASSERT(pVertexElement);
	return pVertexElement->GetUsage();
}

INLINE U8 GetVertexElementUsageIndex(const C_VertexElement* pVertexElement)
{
	BIZ_ASSERT(pVertexElement);
	return pVertexElement->GetUsageIndex();
}

//------------------------------------------------------------------------------
// Vertex declaration

INLINE bool IsVertexDeclarationCreated(const C_VertexDeclaration* pVertexDeclaration)
{
	BIZ_ASSERT(pVertexDeclaration);
	return pVertexDeclaration->IsCreated();
}

INLINE S32 GetVertexDeclarationElementCount(const C_VertexDeclaration* pVertexDeclaration)
{
	BIZ_ASSERT(pVertexDeclaration);
	return pVertexDeclaration->GetElementCount();
}

//------------------------------------------------------------------------------
// Vertex buffer

INLINE bool IsVertexBufferCreated(const C_VertexBuffer* pVertexBuffer)
{
	BIZ_ASSERT(pVertexBuffer);
	return pVertexBuffer->IsCreated();
}

INLINE U32 GetVertexBufferSize(const C_VertexBuffer* pVertexBuffer)
{
	BIZ_ASSERT(pVertexBuffer);
	return pVertexBuffer->GetSize();
}

INLINE U32 GetVertexBufferFlags(const C_VertexBuffer* pVertexBuffer)
{
	BIZ_ASSERT(pVertexBuffer);
	return pVertexBuffer->GetFlags();
}

//------------------------------------------------------------------------------
// Index buffer

INLINE bool IsIndexBufferCreated(const C_IndexBuffer* pIndexBuffer)
{
	BIZ_ASSERT(pIndexBuffer);
	return pIndexBuffer->IsCreated();
}

INLINE U32 GetIndexBufferSize(const C_IndexBuffer* pIndexBuffer)
{
	BIZ_ASSERT(pIndexBuffer);
	return pIndexBuffer->GetSize();
}

INLINE U32 GetIndexBufferFlags(const C_IndexBuffer* pIndexBuffer)
{
	BIZ_ASSERT(pIndexBuffer);
	return pIndexBuffer->GetFlags();
}

INLINE E_IndexFormat GetIndexBufferFormat(const C_IndexBuffer* pIndexBuffer)
{
	BIZ_ASSERT(pIndexBuffer);
	return pIndexBuffer->GetFormat();
}

//------------------------------------------------------------------------------
// Vertex shader

INLINE bool IsVertexShaderCreated(const C_VertexShader* pVertexShader)
{
	BIZ_ASSERT(pVertexShader);
	return pVertexShader->IsCreated();
}

INLINE U32 GetVertexShaderSize(const C_VertexShader* pVertexShader)
{
	BIZ_ASSERT(pVertexShader);
	return pVertexShader->GetSize();
}

INLINE U32 GetVertexShaderFlags(const C_VertexShader* pVertexShader)
{
	BIZ_ASSERT(pVertexShader);
	return pVertexShader->GetFlags();
}

//------------------------------------------------------------------------------
// Pixel shader

INLINE bool IsPixelShaderCreated(const C_PixelShader* pPixelShader)
{
	BIZ_ASSERT(pPixelShader);
	return pPixelShader->IsCreated();
}

INLINE U32 GetPixelShaderSize(const C_PixelShader* pPixelShader)
{
	BIZ_ASSERT(pPixelShader);
	return pPixelShader->GetSize();
}

INLINE U32 GetPixelShaderFlags(const C_PixelShader* pPixelShader)
{
	BIZ_ASSERT(pPixelShader);
	return pPixelShader->GetFlags();
}

//------------------------------------------------------------------------------
// Surface

INLINE bool IsSurfaceCreated(const C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	return pSurface->IsCreated();
}

INLINE U32 GetSurfaceSize(const C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	return pSurface->GetSize();
}

INLINE U32 GetSurfaceWidth(const C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	return pSurface->GetWidth();
}

INLINE U32 GetSurfaceHeight(const C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	return pSurface->GetHeight();
}

INLINE E_SurfaceFormat GetSurfaceFormat(const C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	return pSurface->GetFormat();
}

INLINE E_Multisample GetSurfaceMultisample(const C_Surface* pSurface)
{
	BIZ_ASSERT(pSurface);
	return (E_Multisample)pSurface->GetMultisample(); // Whilst on gcm returns a U32 (need to fix up the casting warnings!)
}

//------------------------------------------------------------------------------
// Texture

INLINE bool IsTextureCreated(const C_Texture* pTexture)
{
	BIZ_ASSERT(pTexture);
	return pTexture->IsCreated();
}

INLINE E_TextureType GetTextureType(const C_Texture* pTexture)
{
	BIZ_ASSERT(pTexture);
	return pTexture->GetType();
}

INLINE U32 GetTextureSize(const C_Texture* pTexture)
{
	BIZ_ASSERT(pTexture);
	return pTexture->GetSize();
}

INLINE U32 GetTextureFlags(const C_Texture* pTexture)
{
	BIZ_ASSERT(pTexture);
	return pTexture->GetFlags();
}

INLINE U32 GetTextureWidth(const C_Texture* pTexture, const U32 level)
{
	BIZ_ASSERT(pTexture);
	return pTexture->GetWidth(level);
}

INLINE U32 GetTextureHeight(const C_Texture* pTexture, const U32 level)
{
	BIZ_ASSERT(pTexture);
	return pTexture->GetHeight(level);
}

INLINE U32 GetTextureDepth(const C_Texture* pTexture, const U32 level)
{
	BIZ_ASSERT(pTexture);
	return pTexture->GetDepth(level);
}

INLINE U32 GetTextureLevels(const C_Texture* pTexture)
{
	BIZ_ASSERT(pTexture);
	return pTexture->GetLevels();
}

INLINE E_TextureFormat GetTextureFormat(const C_Texture* pTexture)
{
	BIZ_ASSERT(pTexture);
	return pTexture->GetFormat();
}

} // namespace Display2

#endif // DISPLAY2_CONTEXT_INL
