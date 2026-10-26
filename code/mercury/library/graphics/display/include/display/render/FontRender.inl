#ifndef DISPLAY_RENDER_FONT_RENDER_INL
#define DISPLAY_RENDER_FONT_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE bool C_FontRender::IsCreated() const
{
	return m_Created;
}

INLINE C_FontRender* FontRender()
{
	assert(g_pFontRender);
	return g_pFontRender;
}

INLINE bool FontRenderExists()
{
	return g_pFontRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_FONT_RENDER_INL
