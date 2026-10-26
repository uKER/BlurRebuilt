#ifndef DISPLAY_RENDER_BASIC_RENDER_INL
#define DISPLAY_RENDER_BASIC_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE bool C_BasicRender::IsCreated() const
{
	return m_Created;
}

INLINE C_BasicRender* BasicRender()
{
	assert(g_pBasicRender);
	return g_pBasicRender;
}

INLINE bool BasicRenderExists()
{
	return g_pBasicRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_BASIC_RENDER_INL
