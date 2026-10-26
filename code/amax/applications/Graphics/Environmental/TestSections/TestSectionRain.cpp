#include "StdAfx.h"
#include "TestSections/TestSectionRain.h"
#include "Framework/Application/Application.h"

#include "Render/Render/GridRender.h"
#include "Render/Debug/DebugRender.h"
#include "Render/Debug/ImmediateDebugRender.h"

#include "Render/Debug/Charts/ChartRenderer.h"
#include "Render/Debug/Charts/LineChart.h"

#include <Render/State/RenderState.h>

#include <core/utility/Singleton.h>
#include <system/debug/debug.h>

Debug::Float	g_debugRainFadeBegin	("TestSection.Rain.FadeBegin",	0.0f,	Debug::Range(0.f, 2000.0f), Debug::Step(1.0f));
Debug::Float	g_debugRainFadeEnd		("TestSection.Rain.FadeEnd",	0.1f,	Debug::Range(0.f, 2000.0f), Debug::Step(1.0f));

C_TestSectionRain::C_TestSectionRain(const Util::C_String& name) : Framework::C_TestSection(name)
{
	m_pFpsChart = new Render::Chart::C_LineChart();

	m_pFpsChart->SetPosition(0.0f, 0.9f);
	m_pFpsChart->SetSize(1.0f, 0.1f);
	m_pFpsChart->SetAxisRange(0.0f, 60.0f);
	m_pFpsChart->SetHistorySize(512);
	m_pFpsChart->AddSeries("FPS", Maths::C_RGBA_U8(255, 0, 0, 255));

	m_RainDrops.Initialise();
	m_RainSheet.Initialise();
	m_RainSplashes.Initialise();
}

C_TestSectionRain::~C_TestSectionRain()
{
	m_RainDrops.Shutdown();
	m_RainSheet.Shutdown();
	m_RainSplashes.Shutdown();

	delete m_pFpsChart;
}

bool C_TestSectionRain::OnCreate()
{
	if (!Display::CreateTexture2DFromFile("Textures/RainHeightMap.dds", &m_RainHeightMap))
		return false;

	Maths::HVec3 vMin(-.5f, -.5f, -.5f);
	Maths::HVec3 vMax(.5f, .5f, .5f);
	
	Maths::C_AxisBox axisBox(vMin, vMax);
		
	Util::C_Random random;
	Maths::C_MatrixScale scale(9.0f, 1.0f, 5.0f);
	Maths::C_MatrixTranslate trans(4.0f, 0.0f, 1.0f);
	Maths::C_MatrixRotateRPH rph(0.0f, 0.0f, random.FRand(Maths::Pi2));
	Maths::C_MatrixSRT srt(scale, rph, trans);
	
	Maths::HMat43 mTransform(srt);
	
	m_RainDrops.AddOcclusionBox(axisBox, mTransform, NULL);

	return true;
}

void C_TestSectionRain::OnDestroy()
{
	if(m_RainHeightMap.IsCreated())
		Display::Device()->Destroy(&m_RainHeightMap);
}

void C_TestSectionRain::OnUpdate()
{
	m_RainSheet.Update(Framework::C_Application::GetInstance()->GetTimer().GetFrameTime(), Framework::C_Application::GetInstance()->GetTimer().IsPaused());
	m_RainDrops.Update(Framework::C_Application::GetInstance()->GetTimer().GetFrameTime(), Framework::C_Application::GetInstance()->GetTimer().IsPaused());
	m_RainSplashes.Update(Framework::C_Application::GetInstance()->GetTimer().GetFrameTime(), Framework::C_Application::GetInstance()->GetTimer().IsPaused());
}

void C_TestSectionRain::OnPreRender()
{
	m_RainSplashes.PreBuild();
}

void C_TestSectionRain::OnRender3DViewWithAA(const S32 viewIndex)
{
	PERF_EVENT("RenderSectionRain");

	Display::Device()->Clear(Display::E_Clear_Colour | Display::E_Clear_Depth, Maths::RGBA(32, 32, 32, 255));

	Render::C_RenderState::Instance().SetFadeIn(g_debugRainFadeBegin, g_debugRainFadeEnd);
	m_pFpsChart->SetValue(0, Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());

	m_SkyDome.Render();
	Render::DrawGrid();

	m_RainSheet.Display();
	m_RainDrops.Display();
	m_RainSplashes.Display();

	Util::C_String str;
	str.Printf("FPS:%.1f", Framework::C_Application::GetInstance()->GetTimer().GetFrameRate());
	//DbUtil::DefaultDebugRenderSys()->Deferred()->Draw2dText(Maths::HVec2(50, 50), str);

	DbUtil::DefaultDebugRenderSys()->RenderDeferred();
}
