#ifndef DISPLAY_RENDER_TIMING_RENDER_H
#define DISPLAY_RENDER_TIMING_RENDER_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

//! C_Timing

struct C_Timing
{
	F32				m_Peak;
	F32				m_Time;
	const char*		m_pName;
	Maths::RGBA		m_Colour;

	bool operator<(const C_Timing& other) const;
};

//! C_TimingRender

class C_TimingRender
{
public:

	C_TimingRender();
	~C_TimingRender();

	void Draw();

private:

	void GatherCPUTimings(Ut::Vector<C_Timing>& cpuTimings);
	void GatherGPUTimings(Ut::Vector<C_Timing>& gpuTimings);

	void RenderAsBars(Ut::Vector<C_Timing>& cpuTimings, Ut::Vector<C_Timing>& gpuTimings);
	void RenderAsText(Ut::Vector<C_Timing>& cpuTimings, Ut::Vector<C_Timing>& gpuTimings);
	void RenderAsUber(Ut::Vector<C_Timing>& cpuTimings, Ut::Vector<C_Timing>& gpuTimings);
};

extern C_TimingRender* g_pTimingRender;
void TimingRender(C_TimingRender* pTimingRender);
C_TimingRender* TimingRender();
const bool TimingRenderExists();

}; // namespace Display

#include "Display/Render/TimingRender.inl"

#endif // DISPLAY_RENDER_TIMING_RENDER_H
