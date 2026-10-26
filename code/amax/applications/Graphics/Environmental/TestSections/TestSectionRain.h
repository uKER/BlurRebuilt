#ifndef TEST_SECTION_RAIN_H
#define TEST_SECTION_RAIN_H

#ifdef _MSC_VER
#pragma once
#endif

#include <TestSections/TestSection.h>
#include "Effect/Rain/Drop.h"
#include "Effect/Rain/Splash.h"
#include "Effect/Rain/Sheet.h"
#include "Render/Sky/SkyDome.h"

namespace Render { namespace Debug { class C_DebugRenderSys; } }
namespace Render { namespace Chart { class C_ChartRenderer; } }
namespace Render { namespace Chart { class C_LineChart; } }

class C_TestSectionRain : public Framework::C_TestSection
{
public:

	C_TestSectionRain(const Util::C_String& name);
	~C_TestSectionRain();

	void OnUpdate();
	void OnPreRender();
	void OnRender3DViewWithAA(const S32 viewIndex);
	bool OnCreate();
	void OnDestroy();

private:

	Effect::Rain::C_DropRenderer		m_RainDrops;
	Effect::Rain::C_SheetRenderer		m_RainSheet;
	Effect::Rain::C_SplashRenderer<>	m_RainSplashes;
	Render::Sky::C_SkyDome				m_SkyDome;

	Display::C_Texture2D				m_RainHeightMap;

	Render::Chart::C_ChartRenderer*		m_pChartRenderer;
	Render::Chart::C_LineChart*			m_pFpsChart;

};

#endif // TEST_SECTION_RAIN_H
