#ifndef DISPLAY_RENDER_SYSTEM_RENDER_INL
#define DISPLAY_RENDER_SYSTEM_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE C_SystemRender* SystemRender()
{
	assert(g_pSystemRender);
	return g_pSystemRender;
}

INLINE const bool SystemRenderExists()
{
	return g_pSystemRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_SYSTEM_RENDER_INL
