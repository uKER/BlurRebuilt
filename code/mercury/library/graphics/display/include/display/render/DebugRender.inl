#ifndef DISPLAY_RENDER_DEBUG_RENDER_INL
#define DISPLAY_RENDER_DEBUG_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE C_DebugRender* DebugRender()
{
	assert(g_pDebugRender);
	return g_pDebugRender;
}

INLINE bool DebugRenderExists()
{
	return g_pDebugRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_DEBUG_RENDER_H
