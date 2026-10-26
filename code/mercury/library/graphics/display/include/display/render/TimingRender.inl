#ifndef DISPLAY_RENDER_TIMING_RENDER_INL
#define DISPLAY_RENDER_TIMING_RENDER_INL

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

INLINE C_TimingRender* TimingRender()
{
	assert(g_pTimingRender);
	return g_pTimingRender;
}

INLINE const bool TimingRenderExists()
{
	return g_pTimingRender != NULL;
}

}; // namespace Display

#endif // DISPLAY_RENDER_TIMING_RENDER_INL
