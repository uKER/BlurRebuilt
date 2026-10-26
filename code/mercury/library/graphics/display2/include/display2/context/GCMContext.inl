#ifndef DISPLAY2_GCM_CONTEXT_INL
#define DISPLAY2_GCM_CONTEXT_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display2
{

INLINE bool C_RenderBuffer::ColourBufferEnabled(const U32 i) const
{
	BIZ_ASSERT(i < 4);
	return m_ColourSurfaceFormat[i] != E_SurfaceFormat_Unknown ? true : false;
}

INLINE bool C_RenderBuffer::DepthStencilBufferEnabled() const
{
	return m_DepthStencilSurfaceFormat != E_SurfaceFormat_Unknown ? true : false;
}

INLINE U32 C_RenderBuffer::GetWidth() const
{
	return m_Width;
}

INLINE U32 C_RenderBuffer::GetHeight() const
{
	return m_Height;
}

INLINE U32 C_RenderBuffer::GetLevels() const
{
	return m_Levels;
}

INLINE U32 C_RenderBuffer::GetFlags() const
{
	return m_Flags;
}

INLINE E_Multisample C_RenderBuffer::GetMultisample() const
{
	return m_Multisample;
}

INLINE U32 C_RenderBuffer::GetColourFlags(const U32 i) const
{
	BIZ_ASSERT(i < 4);
	return m_ColourFlags[i];
}

INLINE E_SurfaceFormat C_RenderBuffer::GetColourSurfaceFormat(const U32 i) const
{
	BIZ_ASSERT(i < 4);
	return m_ColourSurfaceFormat[i];
}
	
INLINE E_TextureFormat C_RenderBuffer::GetColourTextureFormat(const U32 i) const
{
	BIZ_ASSERT(i < 4);
	return m_ColourTextureFormat[i];
}

INLINE U32 C_RenderBuffer::GetDepthStencilFlags() const
{
	return m_DepthStencilFlags;
}

INLINE E_SurfaceFormat C_RenderBuffer::GetDepthStencilSurfaceFormat() const
{
	return m_DepthStencilSurfaceFormat;
}
	
INLINE E_TextureFormat C_RenderBuffer::GetDepthStencilTextureFormat() const
{
	return m_DepthStencilTextureFormat;
}

INLINE C_Surface* C_RenderBuffer::GetColourSurface(const U32 i)
{
	BIZ_ASSERT(i < 4);
	return ColourBufferEnabled(i) ? &m_ColourBuffer[i].GetSurface() : NULL;
}
	
INLINE const C_Surface* C_RenderBuffer::GetColourSurface(const U32 i) const
{
	BIZ_ASSERT(i < 4);
	return ColourBufferEnabled(i) ? &m_ColourBuffer[i].GetSurface() : NULL;
}

INLINE C_Texture* C_RenderBuffer::GetColourTexture(const U32 i)
{
	BIZ_ASSERT(i < 4);
	return ColourBufferEnabled(i) ? &m_ColourBuffer[i].GetTexture() : NULL;
}
	
INLINE const C_Texture* C_RenderBuffer::GetColourTexture(const U32 i) const
{
	BIZ_ASSERT(i < 4);
	return ColourBufferEnabled(i) ? &m_ColourBuffer[i].GetTexture() : NULL;
}

INLINE C_Surface* C_RenderBuffer::GetDepthStencilSurface()
{
	return DepthStencilBufferEnabled() ? &m_DepthStencilBuffer.GetSurface() : NULL;
}
	
INLINE const C_Surface* C_RenderBuffer::GetDepthStencilSurface() const
{
	return DepthStencilBufferEnabled() ? &m_DepthStencilBuffer.GetSurface() : NULL;
}

INLINE C_Texture* C_RenderBuffer::GetDepthStencilTexture()
{
	return DepthStencilBufferEnabled() ? &m_DepthStencilBuffer.GetTexture() : NULL;
}
	
INLINE const C_Texture* C_RenderBuffer::GetDepthStencilTexture() const
{
	return DepthStencilBufferEnabled() ? &m_DepthStencilBuffer.GetTexture() : NULL;
}

} // namespace Display2

#endif // DISPLAY2_GCM_CONTEXT_INL
