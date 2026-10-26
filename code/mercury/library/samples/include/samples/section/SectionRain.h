#ifndef SAMPLES_SECTIONRAIN_H
#define SAMPLES_SECTIONRAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Samples/Section/Section.h"
#include "Effect/Rain/Drop.h"
#include "Effect/Rain/Splash.h"
#include "Effect/Rain/Sheet.h"
#include "Render/Sky/SkyDome.h"

namespace Render
{
	namespace Debug
	{
		class C_DebugRenderSys;
	}
	namespace Chart
	{
		class C_ChartRenderer;
		class C_LineChart;
	}
}

namespace Samples
{

class C_SectionRain : public C_Section
{
public:

	C_SectionRain(const Util::C_String& name);
	~C_SectionRain();

	void OnUpdate(const C_Input& input, const C_Timer& timer);
	void OnPreRender(const C_Input& input, const C_Timer& timer);
	void OnRender(const C_Input& input, const C_Timer& timer);
	bool OnCreate();
	void OnDestroy();

private:
	Render::Debug::C_DebugRenderSys*	m_pDebugRenderSys;
	Effect::Rain::C_DropRenderer		m_RainDrops;
	Effect::Rain::C_SheetRenderer		m_RainSheet;
	Effect::Rain::C_SplashRenderer<>	m_RainSplashes;
	Render::Sky::C_SkyDome				m_SkyDome;

	Display::C_Texture2D				m_RainHeightMap;

	Render::Chart::C_ChartRenderer*		m_pChartRenderer;
	Render::Chart::C_LineChart*			m_pFpsChart;
};

} // namespace Samples

#endif // SAMPLES_SECTIONRAIN_H
