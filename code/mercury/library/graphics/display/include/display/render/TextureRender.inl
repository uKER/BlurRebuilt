#ifndef DISPLAY_RENDER_TEXTURE_RENDER_INL
#define DISPLAY_RENDER_TEXTURE_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE bool C_TextureRender::IsCreated() const
{
	return m_Created;
}

INLINE C_TextureRender* TextureRender()
{
	assert(g_pTextureRender);
	return g_pTextureRender;
}

INLINE bool TextureRenderExists()
{
	return g_pTextureRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_TEXTURE_RENDER_INL
