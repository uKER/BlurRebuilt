INLINE const U32 C_BackBuffer::GetWidth() const
{
	return m_Width;
}

INLINE const U32 C_BackBuffer::GetHeight() const
{
	return m_Height;
}

INLINE const E_Multisample C_BackBuffer::GetMultisample() const
{
	return m_Multisample;
}

INLINE const E_Format C_BackBuffer::GetColourFormat() const
{
	return m_ColourFormat;
}

INLINE const E_Format C_BackBuffer::GetDepthStencilFormat() const
{
	return m_DepthStencilFormat;
}

INLINE const C_Texture2D* C_FrontBuffer::GetTexture() const
{
	assert(m_Texture < 2);
	return &m_Textures[m_Texture];
}

INLINE const U32 C_FrontBuffer::GetWidth() const
{
	return m_Width;
}

INLINE const U32 C_FrontBuffer::GetHeight() const
{
	return m_Height;
}

INLINE const E_Format C_FrontBuffer::GetFormat() const
{
	return m_Format;
}

INLINE const E_Multisample C_FrontBuffer::GetMultisample() const
{
	return m_Multisample;
}

INLINE void C_FrontBuffer::SwapTexture()
{
	m_Texture = m_Texture == 0 ? m_Texture + 1 : 0;
}

INLINE const C_Surface* C_RenderTarget::GetColourSurface()
{
	return m_pColourSurface;
}

INLINE const C_Surface* C_RenderTarget::GetDepthStencilSurface()
{
	return m_pDepthStencilSurface;
}
