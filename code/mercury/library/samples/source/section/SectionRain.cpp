#include "StdAfx.h"
#include "Samples/Section/SectionRain.h"
#include "Render/Render/GridRender.h"

#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"

#include "Render/Debug/Charts/ChartRenderer.h"
#include "Render/Debug/Charts/LineChart.h"

#include <Render/State/RenderState.h>

#include <core/utility/Singleton.h>
#include <system/debug/debug.h>

namespace Samples
{

Debug::Float	g_debugRainFadeBegin	("Samples.Rain.FadeBegin",	0.0f,	Debug::Range(0.f, 2000.0f), Debug::Step(1.0f));
Debug::Float	g_debugRainFadeEnd		("Samples.Rain.FadeEnd",	0.1f,	Debug::Range(0.f, 2000.0f), Debug::Step(1.0f));

C_SectionRain::C_SectionRain(const Util::C_String& name) : C_Section(name)
{
	//m_pChartRenderer = new Render::Chart::C_ChartRenderer();
	m_pFpsChart = new Render::Chart::C_LineChart();

	m_pFpsChart->SetPosition(0.0f, 0.9f);
	m_pFpsChart->SetSize(1.0f, 0.1f);
	m_pFpsChart->SetAxisRange(0.0f, 60.0f);
	m_pFpsChart->SetHistorySize(512);
	m_pFpsChart->AddSeries("FPS", Maths::C_RGBA_U8(255, 0, 0, 255));

	m_pDebugRenderSys = new Render::Debug::C_DebugRenderSys;
	DbUtil::DefaultDebugRenderSys(m_pDebugRenderSys);
	
	//m_RainDrops.SetNumSplits(3);
	m_RainDrops.Initialise();
	m_RainSheet.Initialise();
	m_RainSplashes.Initialise();
}

C_SectionRain::~C_SectionRain()
{
	m_RainDrops.Shutdown();
	m_RainSheet.Shutdown();
	m_RainSplashes.Shutdown();

	delete m_pDebugRenderSys;
	delete m_pFpsChart;
	//delete m_pChartRenderer;
}

bool C_SectionRain::OnCreate()
{
	if (!Display::CreateTexture2DFromFile("Textures/RainHeightMap.dds", &m_RainHeightMap))
		return false;

	//Maths::C_AxisBox axisBox(Maths::HVec3(0.0f, 0.0f, 0.0f), Maths::HVec3(25.0f, 10.0f, 25.0f));
	
	//Maths::HVec3 vMin(-3.3979f, -.8107f, -13.141f);
	//Maths::HVec3 vMax(13.5151f, 1.6045f, -1.130f);

	Maths::HVec3 vMin(-.5f, -.5f, -.5f);
	Maths::HVec3 vMax(.5f, .5f, .5f);
	
	Maths::C_AxisBox axisBox(vMin, vMax);
		
	Util::C_Random random;
	Maths::C_MatrixScale scale(9.0f, 1.0f, 5.0f);
	Maths::C_MatrixTranslate trans(4.0f, 0.0f, 1.0f);
	Maths::C_MatrixRotateRPH rph(0.0f, 0.0f, random.FRand(Maths::Pi2));
	Maths::C_MatrixSRT srt(scale, rph, trans);
	//Maths::HMat33 mMatrix(rph);
	
	Maths::HMat43 mTransform(srt);
	
	m_RainDrops.AddOcclusionBox(axisBox, mTransform, NULL);

	return true;
}

void C_SectionRain::OnDestroy()
{
	if(m_RainHeightMap.IsCreated())
		Display::Device()->Destroy(&m_RainHeightMap);
}

void C_SectionRain::OnUpdate(const C_Input& input, const C_Timer& timer)
{
	//m_pChartRenderer->ResetDeferred();
	m_RainSheet.Update(timer.GetFrameTime(), timer.IsPaused());
	m_RainDrops.Update(timer.GetFrameTime(), timer.IsPaused());
	m_RainSplashes.Update(timer.GetFrameTime(), timer.IsPaused());
}

void C_SectionRain::OnPreRender(const C_Input& input, const C_Timer& timer)
{
	m_RainSplashes.PreBuild();
}

void C_SectionRain::OnRender(const C_Input& input, const C_Timer& timer)
{
	PERF_EVENT("RenderSectionRain");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(32, 32, 32, 255));

	Render::C_RenderState::Instance().SetFadeIn(g_debugRainFadeBegin, g_debugRainFadeEnd);
	m_pFpsChart->SetValue(0, timer.GetFrameRate());

	m_SkyDome.Render();
	Render::DrawGrid();

	m_RainSheet.Display();
	m_RainDrops.Display();
	m_RainSplashes.Display();

	Util::C_String str;
	str.Printf("FPS:%.1f", timer.GetFrameRate());
	DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(50, 50), str);

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
	//m_pChartRenderer->RenderDeferred();

	//m_pFpsChart->Render();
	
	
}

}; // namespace Samples
