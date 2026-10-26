#ifndef DISPLAY_RENDER_MEMORY_RENDER_INL
#define DISPLAY_RENDER_MEMORY_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE C_MemoryRender* MemoryRender()
{
	assert(g_pMemoryRender);
	return g_pMemoryRender;
}

INLINE const bool MemoryRenderExists()
{
	return g_pMemoryRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_MEMORY_RENDER_INL
