INLINE const bool C_RenderTexture2D::IsCreated() const
{
	return m_Texture.IsCreated() && m_Surface.IsCreated();
}
