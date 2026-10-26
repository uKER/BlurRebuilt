#ifndef DISPLAY_RENDER_RESOURCE_RENDER_INL
#define DISPLAY_RENDER_RESOURCE_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE C_ResourceRender* ResourceRender()
{
	assert(g_pResourceRender);
	return g_pResourceRender;
}

INLINE const bool ResourceRenderExists()
{
	return g_pResourceRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_RESOURCE_RENDER_INL
