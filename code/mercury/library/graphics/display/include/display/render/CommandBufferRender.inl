#ifndef DISPLAY_RENDER_COMMAND_BUFFER_RENDER_INL
#define DISPLAY_RENDER_COMMAND_BUFFER_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE C_CommandBufferRender* CommandBufferRender()
{
	assert(g_pCommandBufferRender);
	return g_pCommandBufferRender;
}

INLINE const bool CommandBufferRenderExists()
{
	return g_pCommandBufferRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_COMMAND_BUFFER_RENDER_INL
