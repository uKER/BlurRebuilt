#include "DebugSystemsPch.h"
#include <system/perf/Profiler.h>
#include <DebugSystems/DebugHUDPerf.h>
#include <Render/font/Font.h>
#include <Render/debug/DebugRender.h>
#include <algorithm>

namespace DebugSystems
{

Debug::Bool g_PerfStatsDisplay("System.Performance.ShowStats", false );
Debug::Float g_PerfStatsYOffset("System.Performance.Options.PerfStatsYOffset",		0.0f, Debug::Range(-3000.0f, 600.f) );
Debug::Float g_PerfStatsXOffset("System.Performance.Options.PerfStatsXOffset",		0.0f, Debug::Range(-3000.0f, 0.f) );
Debug::Float g_PerfStatsNameSpace("System.Performance.Options.PerfStatsNameSpace",	300.0f, Debug::Range(100.0f, 800.0f) );
Debug::Int g_PerfStatsScale("System.Performance.Options.PerfStatsScale", 20, Debug::Range(1, 100) );
Debug::Int g_PerfThreadID("System.Performance.Options.ThreadID",		0, Debug::Range(0, 100) );
Debug::Int g_PerfStatsLevel("System.Performance.Options.Level", 3, Debug::Range(0, 10));
Debug::Int g_PerfFontScale("System.Performance.Options.FontScale", 24, Debug::Range(5, 500));
static void DumpPerf(void*)
{
	Perf::DumpPerfData();
}

static Debug::Function g_DumpPerf("System.Performance.Options.DumpPerfInfo", DumpPerf, 0);

static void CapturePerf(void*)
{
	Perf::CapturePerfData();
}

static Debug::Function g_CapturePerf("System.Performance.Options.CapturePerfInfo", CapturePerf, 0);

//Debug::Bool g_FragmentationStatsUpdate("System.Memory.UpdateFragmentationStats", false );
//Debug::Int g_FragmentationSysMemAlloc("System.Memory.FragmentationSysMemAlloc",		5, Debug::Range(0, 50) );
//Debug::Bool g_PerfStatsShowPeaks("System.Performance.ShowPeaks", false );
//Debug::Bool g_PerfStatsShowPeaksCalls("System.Performance.ShowPeaksCalls", false );
//Debug::Bool g_PerfStatsShowPeaksCallsAtPeakTime("System.Performance.ShowPeaksCallsAtPeakTime", false );
//Debug::Float g_PerfStatsMinThreshold("System.Performance.MinThreshold",		0.0f, Debug::Range(0.0f, 16000.0f) );

//Debug::Bool g_PerfStatsStack("System.Performance.Advanced.Stack", false );
//Debug::Bool g_PerfStatsRollup("System.Performance.Advanced.RollUp", false );
//Debug::Int g_PerfStatsOffset("System.Performance.Advanced.Offset", 128 );

//ebug::Int g_PerfStatsItem("System.Performance.Advanced.SelectedItem", -1 );
//Debug::Bool g_PerfStatsItemEnabled("System.Performance.Advanced.ItemEnabled", false );


C_DebugHUDPerf*	g_pDebugHUDPerf = NULL;

//Call back
void EndFrameCallBack(Perf::ProfilerCommon *pProfiler, void* /**/)
{
	if (g_pDebugHUDPerf && g_PerfStatsDisplay)
	{
		Perf::C_EventTree * pEventTree = Perf::C_EventTree::CreateProfileTree(pProfiler);

		g_pDebugHUDPerf->SetEventTree( pEventTree );
	}
}

C_DebugHUDPerf::C_DebugHUDPerf()
{
	m_pFontRenderer = NULL;
	m_pEventTree = NULL;

	//Set to global variable to setup the callback
	g_pDebugHUDPerf = this;
	m_bCallbackRegistered = false;
}

C_DebugHUDPerf::~C_DebugHUDPerf()
{
	if (m_pFontRenderer)
		delete m_pFontRenderer;

	Perf::ProfilerCommon::Instance().UnregisterCallback(&EndFrameCallBack);

	SAFE_DELETE_PTR(m_pEventTree);
}

void C_DebugHUDPerf::Update( const Maths::C_TimeS& timeStep )
{
	if (m_bCallbackRegistered != g_PerfStatsDisplay)
	{
		if (g_PerfStatsDisplay)
			Perf::ProfilerCommon::Instance().RegisterCallback(&EndFrameCallBack);
		else
			Perf::ProfilerCommon::Instance().UnregisterCallback(&EndFrameCallBack);			
	}
}

U32 C_DebugHUDPerf::RenderBar(Maths::HVec2 PCREF position,  U64 bar[2], Maths::HVec2& endPosition, const U32 level)
{
	const Maths::RGBA g_BarColours[6] = 
	{
		Maths::RGBA(255, 0, 0, 200),
		Maths::RGBA(0, 255, 0, 200),
		Maths::RGBA(255, 255, 0, 200),
		Maths::RGBA(255, 0, 255, 200),
		Maths::RGBA(128, 255, 0, 200),
		Maths::RGBA(255, 128, 128, 200),
	};

#if defined(PLATFORM_PS3)
	const U64 start = (bar[0] - m_FirstEventTime) / 80;
	const U64 end = (bar[1] - m_FirstEventTime) / 80;  
#else
	const U64 start = (bar[0] - m_FirstEventTime);
	const U64 end = (bar[1] - m_FirstEventTime);  
#endif
	Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
	Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(position.X() + float(start / g_PerfStatsScale) + 4.f, position.Y() + 4.f),Maths::HVec2(float((end - start) / g_PerfStatsScale) - 8.f, m_pFontRenderer->GetScaleY()  - 8.f), g_BarColours[level%6] );
	Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);

	endPosition = Maths::HVec2(position.X() +  float(end / g_PerfStatsScale), position.Y());
	return U32(end - start);
}

void C_DebugHUDPerf::RenderItem( U32& index, const U32 level, Perf::C_EventTreeItem* pItem )
{
	//Main Position
	Maths::HVec2 position(g_PerfStatsNameSpace + g_PerfStatsXOffset ,index * m_pFontRenderer->GetScaleY() + 100 + g_PerfStatsYOffset);
	
	//Draw odd bar
	if (index%2)
	{
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
		Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(0.f, position.Y()), Maths::HVec2(1024.f,m_pFontRenderer->GetScaleY()), Maths::RGBA(0,0,0,80) );
		Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
	}

	U32 totalTime = 0;
	U32 count = 0;
	//Draw bars and calculate time and counts
	U64 bar[2] = {0,0};
	Maths::HVec2 barPosition  = position;
	Maths::HVec2 endBarPosition;

	for (S32 i = 0; i < pItem->m_EventListPairs.Size(); i++)
	{
		if (PROFILER_EVENT_END(pItem->m_EventListPairs[i]) == 0)
		{
			//Start
			bar[0] = PROFILER_EVENT_TIME(pItem->m_EventListPairs[i]);
		}
		else
		{
			if (bar[0] == 0)
			{
				//it started last frame
				bar[0] = m_FirstEventTime;
			}

			bar[1] = PROFILER_EVENT_TIME(pItem->m_EventListPairs[i]);

			//Render Bar
			totalTime += RenderBar(barPosition, bar, endBarPosition, level);
			count++;

			bar[0] = 0;
			bar[1] = 0;

			
		}
	}
	if (bar[0] != 0)
	{
		bar[1] = m_LastEventTime;

		//Render Bar
		totalTime += RenderBar(barPosition, bar, endBarPosition, level);
		count++;

		bar[0] = 0;
		bar[1] = 0;
	}

	//Print total time and count
	Util::C_String data;
	data.Printf("Count: %i Time: %iµs", count, totalTime);
	m_pFontRenderer->Print(endBarPosition, data);

	//Print Back
	Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
	Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(0, position.Y()), Maths::HVec2(Maths::HF32(g_PerfStatsNameSpace),Maths::HF32(m_pFontRenderer->GetScaleY())), Maths::RGBA(40,40,40,100) );
	Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
	
	//Print Name
	m_pFontRenderer->Print(Maths::HVec2(Maths::Max(position.X(),g_PerfStatsNameSpace)  - level * m_pFontRenderer->GetScaleX(), position.Y()), pItem->m_Name.Chars(), Render::Font::E_Justification_Right);
	
	index++;

	//Check levels and render children
	if (level < U32(g_PerfStatsLevel))
	{
		for (S32 i = 0; i < pItem->m_ChildProfileItems.Size(); i++)
		{
			RenderItem(index, level + 1, pItem->m_ChildProfileItems[i]);
		}
	}
}

void C_DebugHUDPerf::Render()
{
	if (!g_PerfStatsDisplay)
		return;

	PERF_EVENT("DebugHUDPerf Render");

	Render::Debug::Immediate()->SetDepthEnable(false);
	if (m_pFontRenderer == NULL)
	{
		//Create the font renderer
		m_pFontRenderer = Render::Font::FontSystem()->GetFontRenderer("dontship/fonts/arialdebug.font");

		m_pFontRenderer->SetShadowEnable( true );
		m_pFontRenderer->SetColourOutline(Maths::RGBA(0,0,25,255));
		m_pFontRenderer->SetOutlineSize(0.35f);
	}
	
	//Set Font Size
	m_pFontRenderer->SetScaleX(F32(g_PerfFontScale));
	m_pFontRenderer->SetScaleY(F32(g_PerfFontScale));

	if (!m_pEventTree)
	{
		Render::Debug::Immediate()->SetDepthEnable(true);
		return;
	}

	//Total, bar and time
	U32 index = 1;

#if defined(PLATFORM_PS3)
	const U64 totalSize = (m_pEventTree->m_EndEventTime - m_pEventTree->m_StartEventTime)/80;     
#else
	const U64 totalSize = (m_pEventTree->m_EndEventTime - m_pEventTree->m_StartEventTime);     
#endif

	m_FirstEventTime = m_pEventTree->m_StartEventTime;
	m_LastEventTime = m_pEventTree->m_EndEventTime;

	Maths::HVec2 position(g_PerfStatsNameSpace + g_PerfStatsXOffset , 100 + g_PerfStatsYOffset);
	Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Alpha);
	Render::Debug::Immediate()->Draw2dBar(Maths::HVec2(position.X() + 4.f, position.Y() + 4.f),Maths::HVec2(float((totalSize) / g_PerfStatsScale) - 8.f, m_pFontRenderer->GetScaleY()  - 8.f), Maths::RGBA(200,100,100,200) );
	Render::Debug::Immediate()->SetBlendMode(DbUtil::E_BlendMode_Solid);
	m_pFontRenderer->Print(Maths::HVec2(Maths::Max(position.X(), g_PerfStatsNameSpace), position.Y()), "TOTAL", Render::Font::E_Justification_Right);
	
	Util::C_String data;
	data.Printf("Time: %iµs FPS: %i", U32(totalSize), (1000000L/U32(totalSize)));
	m_pFontRenderer->Print(Maths::HVec2(position.X() + float((totalSize) / g_PerfStatsScale), position.Y()) , data.Chars());

	// draw helper grid
	U64 gridStep = 0;
	while (gridStep < totalSize)
	{
		Render::Debug::Immediate()->Draw2dBar( Maths::HVec2(position.X() + float((gridStep) / g_PerfStatsScale), 100 + g_PerfStatsYOffset + m_pFontRenderer->GetScaleY()), Maths::HVec2( 4.0f, 768 - 100 - g_PerfStatsYOffset + m_pFontRenderer->GetScaleY() ), Maths::RGBA(0x00, 0x00, 0x00, 0xff));
		gridStep += 5000L; //Steps of 5 ms 
	}

	//Thread ID
	if (g_PerfThreadID >= m_pEventTree->m_EventTreeThread.Size())
	{
		g_PerfThreadID = m_pEventTree->m_EventTreeThread.Size() - 1;
	}

	//Render root profile items
	for (S32 i = 0; i < m_pEventTree->m_EventTreeThread[g_PerfThreadID].m_pEventTree->m_ChildProfileItems.Size(); i++)
	{
		RenderItem(index, 0, m_pEventTree->m_EventTreeThread[g_PerfThreadID].m_pEventTree->m_ChildProfileItems[i]);
	}
	
	Render::Debug::Immediate()->SetDepthEnable(true);
}

}


