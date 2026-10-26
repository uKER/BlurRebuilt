#ifndef DISPLAY_RENDER_HEAP_RENDER_INL
#define DISPLAY_RENDER_HEAP_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE C_HeapRender* HeapRender()
{
	assert(g_pHeapRender);
	return g_pHeapRender;
}

INLINE const bool HeapRenderExists()
{
	return g_pHeapRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_HEAP_RENDER_INL
