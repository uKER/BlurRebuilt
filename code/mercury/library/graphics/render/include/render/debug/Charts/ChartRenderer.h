#ifndef RENDER_CHART_RENDERER_H
#define RENDER_CHART_RENDERER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\utility\Singleton.h>

#include "Display/Device/Device.h"
#include "Display/Utils/Renderer.h"

namespace Render
{
namespace Chart
{

class I_Chart;

class C_ChartRenderer : public Display::I_Renderer, public Ut::Singleton<C_ChartRenderer, Ut::CreateExternal>
{
public:
	C_ChartRenderer();
	virtual ~C_ChartRenderer();

	virtual bool OnCreate();
	virtual void OnDestroy();

	void Prepare(const I_Chart* pChart) const;
	
	// Deferred rendering control
	void AddDeferredChart(const I_Chart* pChart);
	void ResetDeferred();

	void RenderDeferred();

private:
	Ut::Vector<const I_Chart*>			m_DeferredCharts;
	Display::C_VertexShader			m_VertexShader;
	Display::C_PixelShader			m_PixelShader;
};

INLINE void C_ChartRenderer::AddDeferredChart(const I_Chart* pChart)
{
	if(m_DeferredCharts.Size() < 100)
		m_DeferredCharts.PushBack(pChart);
}

INLINE void C_ChartRenderer::ResetDeferred()
{
	m_DeferredCharts.Clear();
}

C_ChartRenderer*			ChartRenderer();
const bool					ChartRendererExists();

INLINE C_ChartRenderer* ChartRenderer()
{
	return &C_ChartRenderer::Instance();
}

INLINE const bool ChartRendererExists()
{
	return C_ChartRenderer::Exists();
}

} // namespace Chart
} // namespace Render

extern Debug::Logger &g_Log;

#endif // RENDER_CHART_RENDERER_H
