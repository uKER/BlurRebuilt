#include "stdafx.h"
#include "Display/Render/TimingRender.h"
#include "Display/Render/BasicRender.h"
#include "Display/Render/FontRender.h"
#include "Display/Timing/Timings.h"
#include <algorithm>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

Debug::Int	g_debugDisplayTimingRenderBarsFade			("Graphics.Display.Render.TimingRender.BarsFade",		128,		Debug::Range(0, 255));
Debug::Int	g_debugDisplayTimingRenderTextFade			("Graphics.Display.Render.TimingRender.TextFade",		0,			Debug::Range(0, 255));

Debug::Int	g_debugDisplayTimingRenderGraphX			("Graphics.Display.Render.TimingRender.GraphX",			64,			Debug::Range(0, 1024));
Debug::Int	g_debugDisplayTimingRenderGraphY			("Graphics.Display.Render.TimingRender.GraphY",			96,			Debug::Range(0, 768));
Debug::Int	g_debugDisplayTimingRenderGraphWidth		("Graphics.Display.Render.TimingRender.GraphWidth",		256,		Debug::Range(0, 1024));
Debug::Int	g_debugDisplayTimingRenderBarHeight			("Graphics.Display.Render.TimingRender.BarHeight",		5,			Debug::Range(0, 50));
Debug::Int	g_debugDisplayTimingRenderBarMarker			("Graphics.Display.Render.TimingRender.BarMarker",		1,			Debug::Range(0, 10));
Debug::Bool	g_debugDisplayTimingRenderShowHelp			("Graphics.Display.Render.TimingRender.ShowHelp",		true);
Debug::Bool	g_debugDisplayTimingRenderRollUp			("Graphics.Display.Render.TimingRender.RollUp",			false);
Debug::Bool	g_debugDisplayTimingRenderCPUShowPeaks		("Graphics.Display.Render.TimingRender.CPUShowPeaks",	false);
Debug::Bool	g_debugDisplayTimingRenderCPUResetPeaks		("Graphics.Display.Render.TimingRender.CPUResetPeaks",	false);
Debug::Bool	g_debugDisplayTimingRenderGPUShowPeaks		("Graphics.Display.Render.TimingRender.GPUShowPeaks",	true);
Debug::Bool	g_debugDisplayTimingRenderGPUResetPeaks		("Graphics.Display.Render.TimingRender.GPUResetPeaks",	false);
Debug::Int	g_debugDisplayTimingRenderCPUOffsetY		("Graphics.Display.Render.TimingRender.CPUOffsetY",		0,			Debug::Range(0, 100));
Debug::Int	g_debugDisplayTimingRenderGPUOffsetY		("Graphics.Display.Render.TimingRender.GPUOffsetY",		0,			Debug::Range(0, 100));
Debug::Int	g_debugDisplayTimingRenderFontPosX			("Graphics.Display.Render.TimingRender.FontPosX",		40,			Debug::Range(0, 1024));
Debug::Int	g_debugDisplayTimingRenderFontPosY			("Graphics.Display.Render.TimingRender.FontPosY",		160,		Debug::Range(0, 768));
Debug::Int	g_debugDisplayTimingRenderFontSize			("Graphics.Display.Render.TimingRender.FontSize",		16,			Debug::Range(0, 50));

enum E_TimingRender
{
	E_TimingRender_Bars_CPU_GPU,
	E_TimingRender_Bars_CPU,
	E_TimingRender_Bars_GPU,
	E_TimingRender_Text_CPU_GPU,
	E_TimingRender_Text_CPU,
	E_TimingRender_Text_GPU,
	E_TimingRender_Uber_GPU,
};

DEBUG_ENUM(E_TimingRender)
	DE_OPTION(E_TimingRender_Bars_CPU_GPU,	"Bars_CPU_GPU")
	DE_OPTION(E_TimingRender_Bars_CPU,		"Bars_CPU")
	DE_OPTION(E_TimingRender_Bars_GPU,		"Bars_GPU")
	DE_OPTION(E_TimingRender_Text_CPU_GPU,	"Text_CPU_GPU")
	DE_OPTION(E_TimingRender_Text_CPU,		"Text_CPU")
	DE_OPTION(E_TimingRender_Text_GPU,		"Text_GPU")
	DE_OPTION(E_TimingRender_Uber_GPU,		"Uber_GPU")
END_DEBUG_ENUM

Debug::Enum<E_TimingRender> g_debugDisplayTimingRenderShowTimings("Graphics.Display.Render.TimingRender.ShowTimings", E_TimingRender_Text_GPU);

namespace Display
{

bool C_Timing::operator<(const C_Timing& other) const
{
	return strcmp(m_pName, other.m_pName) <= 0;
}

C_TimingRender::C_TimingRender()
{
	assert(g_pTimingRender == NULL);
	g_pTimingRender = this;
}

C_TimingRender::~C_TimingRender()
{
	assert(g_pTimingRender == this);
	g_pTimingRender = NULL;
}

void C_TimingRender::GatherCPUTimings(Ut::Vector<C_Timing>& cpuTimings)
{
	Perf::C_CPUTimingStats* pStats = Perf::C_CPUTimingStats::m_StatsList.First();
	while(pStats)
	{
		if(pStats->Show())
		{
			C_Timing timing;
			timing.m_Peak	= ((F32)pStats->GetTotalTime(Perf::E_TimingFrame_Peak) / (F32)SYS_USEC_PER_SEC) * 1000.f;
			timing.m_Time	= ((F32)pStats->GetTotalTime(Perf::E_TimingFrame_Prev) / (F32)SYS_USEC_PER_SEC) * 1000.f;
			timing.m_pName	= pStats->m_Name.Chars();
			timing.m_Colour	= pStats->m_Colour;
			cpuTimings.PushBack(timing);
		}
		
		pStats = pStats->Next();
	}
}

void C_TimingRender::GatherGPUTimings(Ut::Vector<C_Timing>& gpuTimings)
{
#if USE_GPU_TIMINGS2
	Display2::C_TimingManager::GetInstance().m_Mutex.Wait();

	Display2::C_TimingMap::Iterator it = Display2::C_TimingManager::GetInstance().m_Timings.Begin();
	while(it != Display2::C_TimingManager::GetInstance().m_Timings.End())
	{
		Display2::C_Timing* pTiming = it->GetValue();

		//if(pTiming->Enabled())
		{
			C_Timing timing;

			timing.m_Peak	= 0.0f;
			timing.m_Time	= pTiming->GetTime();
			timing.m_pName	= pTiming->m_Name.Chars();
			timing.m_Colour	= Maths::RGBA(255, 255, 255, 255);

			gpuTimings.PushBack(timing);
		}

		it++;
	}

	Display2::C_TimingManager::GetInstance().m_Mutex.Release();

	std::sort(gpuTimings.Begin(), gpuTimings.End());
#else
	C_GPUTimingStats* pStats = C_GPUTimingStats::m_StatsList.First();
	while(pStats)
	{
		if(pStats->Show())
		{
			C_Timing timing;
			timing.m_Peak	= (F32)pStats->GetTotalTime(Perf::E_TimingFrame_Peak) / 1000000.0f;
			timing.m_Time	= (F32)pStats->GetTotalTime(Perf::E_TimingFrame_Prev) / 1000000.0f;
			timing.m_pName	= pStats->m_Name.Chars();
			timing.m_Colour	= pStats->m_Colour;
			gpuTimings.PushBack(timing);
		}
		
		pStats = pStats->Next();
	}
#endif
}

void C_TimingRender::RenderAsBars(Ut::Vector<C_Timing>& cpuTimings, Ut::Vector<C_Timing>& gpuTimings)
{
	S32 cpuTimingCount 	= cpuTimings.Size();
	S32 gpuTimingCount 	= gpuTimings.Size();

	F32 timingCount 	= (F32)cpuTimingCount+(F32)gpuTimingCount;

	F32 barMaker		= (F32)g_debugDisplayTimingRenderBarMarker;
	F32 barHeight		= (F32)g_debugDisplayTimingRenderBarHeight;
	F32 graphX			= (F32)g_debugDisplayTimingRenderGraphX;
	F32 graphY			= (F32)g_debugDisplayTimingRenderGraphY;
	F32 graphWidth		= (F32)g_debugDisplayTimingRenderGraphWidth;
	
	F32 graphHeight		= 0.0f;

	if(g_debugDisplayTimingRenderRollUp)
	{
		graphHeight = barHeight+(barHeight*2.0f)+(barHeight*2.0f);
	}
	else
	{
		if(cpuTimingCount > 0 && gpuTimingCount > 0)
		{
			graphHeight = (barHeight*timingCount)+(barHeight*2.0f)+(barHeight*2.0f);
		}
		else
		{
			graphHeight = (barHeight*timingCount)+(barHeight*2.0f);
		}
	}
	
	// Draw bar background
	{
		F32 x = graphX;
		F32 y = graphY-barHeight;
		F32 w = graphWidth;
		F32 h = graphHeight;

		BasicRender()->DrawColoured2DRect(x, y, w, h, Maths::RGBA(0, 0, 0, g_debugDisplayTimingRenderBarsFade));
	}

	// Draw bar markets
	{
		for(U32 i=0; i<5; i++)
		{
			F32 x = graphX+((F32)i)*(graphWidth/4.0f);
			F32 y = graphY-barHeight;
			F32 w = barMaker;
			F32 h = graphHeight;

			BasicRender()->DrawColoured2DRect(x, y, w, graphHeight, Maths::RGBA(255, 0, 0, g_debugDisplayTimingRenderBarsFade));
		}
	}

	// Draw bar timings
	{
		F32 x = graphX;
		F32 y = graphY;
		F32 h = barHeight;

		if(cpuTimingCount > 0)
		{
			for(S32 cpuTimingIndex=0; cpuTimingIndex<cpuTimingCount; cpuTimingIndex++)
			{
				C_Timing& timing = cpuTimings[cpuTimingIndex];
				
				F32 w = timing.m_Time > 0.0f ? ((timing.m_Time / 32.0f) * graphWidth) : 0.0f;
				BasicRender()->DrawColoured2DRect(x, y, w, h, timing.m_Colour);
				
				if(g_debugDisplayTimingRenderCPUShowPeaks)
				{
					F32 peakw = timing.m_Peak > 0.0f ? ((timing.m_Peak / 32.0f) * graphWidth) : 0.0f;
					BasicRender()->DrawColoured2DRect(x+peakw, y, 2.0f, h, timing.m_Colour);
				}
				
				if(g_debugDisplayTimingRenderRollUp)
				{
					x += w;
				}
				else
				{
					y += h;
				}
			}

			y += barHeight*2.0f;
		}

		if(gpuTimingCount > 0)
		{
			x = graphX;

			for(S32 gpuTimingIndex=0; gpuTimingIndex<gpuTimingCount; gpuTimingIndex++)
			{
				C_Timing& timing = gpuTimings[gpuTimingIndex];

				F32 w = timing.m_Time > 0.0f ? ((timing.m_Time / 32.0f) * graphWidth) : 0.0f;
				BasicRender()->DrawColoured2DRect(x, y, w, h, timing.m_Colour);

				if(g_debugDisplayTimingRenderRollUp)
				{
					x += w;
				}
				else
				{
					y += h;
				}
			}
		}
	}

	if(g_debugDisplayTimingRenderShowHelp)
	{
		char buffer[128];

		F32 x = graphX;
		F32 y = graphY+graphHeight+5.0f;
		F32 w = 15;
		F32 h = 15;
		
		const S32 cpuTimingCount = cpuTimings.Size();
		for(S32 cpuTimingIndex=0; cpuTimingIndex<cpuTimingCount; cpuTimingIndex++)
		{
			C_Timing& timing = cpuTimings[cpuTimingIndex];

			BasicRender()->DrawColoured2DRect(x+1.0f, y-(h/2.0f)+1.0f, w-2.0f, h-2.0f, timing.m_Colour);
			
			sprintf_s(buffer, sizeof(buffer), "= %s", timing.m_pName);
			FontRender()->Draw(buffer, x+w+5.0f, y, w, h, timing.m_Colour);

			y += h;
		}

		const S32 gpuTimingCount = gpuTimings.Size();
		for(S32 gpuTimingIndex=0; gpuTimingIndex<gpuTimingCount; gpuTimingIndex++)
		{
			C_Timing& timing = gpuTimings[gpuTimingIndex];

			BasicRender()->DrawColoured2DRect(x+1.0f, y-(h/2.0f)+1.0f, w-2.0f, h-2.0f, timing.m_Colour);
			
			sprintf_s(buffer, sizeof(buffer), "= %s", timing.m_pName);
			FontRender()->Draw(buffer, x+w+5.0f, y, w, h, timing.m_Colour);

			y += h;
		}
	}
}

void C_TimingRender::RenderAsText(Ut::Vector<C_Timing>& cpuTimings, Ut::Vector<C_Timing>& gpuTimings)
{
	BasicRender()->DrawColoured2DRect(20.f, 20.f, 984.f, 728.f, Maths::RGBA(0, 0, 0, g_debugDisplayTimingRenderTextFade));
	
	F32 x = (F32)g_debugDisplayTimingRenderFontPosX;
	F32 y = (F32)g_debugDisplayTimingRenderFontPosY;
	F32 s = (F32)g_debugDisplayTimingRenderFontSize;

	char buffer[256];

	const S32 cpuTimingCount = cpuTimings.Size();
	for(S32 cpuTimingIndex=0; cpuTimingIndex<cpuTimingCount; cpuTimingIndex++)
	{
		if(cpuTimingIndex < g_debugDisplayTimingRenderCPUOffsetY)
			continue;
		
		C_Timing& cpuTiming = cpuTimings[cpuTimingIndex];

		F32 x1 = x+s*3.0f;

		sprintf_s(buffer, sizeof(buffer), "%.2f", cpuTiming.m_Time);
		FontRender()->Draw(buffer, x1, y, s, s, cpuTiming.m_Colour, Display::E_FontRenderAlign_Rt);

		x1 += 10.0f;

		sprintf_s(buffer, sizeof(buffer), "= %s", cpuTiming.m_pName);
		FontRender()->Draw(buffer, x1, y, s, s, cpuTiming.m_Colour, Display::E_FontRenderAlign_Lt);

		y += s+5.0f;
	}

	const S32 gpuTimingCount = gpuTimings.Size();
	for(S32 gpuTimingIndex=0; gpuTimingIndex<gpuTimingCount; gpuTimingIndex++)
	{
		if(gpuTimingIndex < g_debugDisplayTimingRenderGPUOffsetY)
			continue;

		C_Timing& gpuTiming = gpuTimings[gpuTimingIndex];

		F32 x1 = x+s*3.0f;
		sprintf_s(buffer, sizeof(buffer), "%.2f", gpuTiming.m_Time);
		FontRender()->Draw(buffer, x1, y, s, s, gpuTiming.m_Colour, Display::E_FontRenderAlign_Rt);

		x1 += 10.0f;

		if(g_debugDisplayTimingRenderGPUShowPeaks)
			sprintf_s(buffer, sizeof(buffer), "= %s (%.2f)", gpuTiming.m_pName, gpuTiming.m_Peak);
		else
			sprintf_s(buffer, sizeof(buffer), "= %s", gpuTiming.m_pName);
		FontRender()->Draw(buffer, x1, y, s, s, gpuTiming.m_Colour, Display::E_FontRenderAlign_Lt);

		y += s+5.0f;
	}
}

void C_TimingRender::RenderAsUber(Ut::Vector<C_Timing>& cpuTimings, Ut::Vector<C_Timing>& gpuTimings)
{
}

void C_TimingRender::Draw()
{
	if(g_debugDisplayTimingRenderCPUResetPeaks)
	{
		Perf::C_CPUTimingStats::ResetPeaks();
		g_debugDisplayTimingRenderCPUResetPeaks = false;
	}

	if(g_debugDisplayTimingRenderGPUResetPeaks)
	{
		C_GPUTimingStats::ResetPeaks();
		g_debugDisplayTimingRenderGPUResetPeaks = false;
	}

	Ut::Vector<C_Timing> cpuTimings;
	Ut::Vector<C_Timing> gpuTimings;

	switch(g_debugDisplayTimingRenderShowTimings)
	{
	case E_TimingRender_Bars_CPU_GPU:
		GatherCPUTimings(cpuTimings);
		GatherGPUTimings(gpuTimings);
		RenderAsBars(cpuTimings, gpuTimings);
		break;
	case E_TimingRender_Bars_CPU:
		GatherCPUTimings(cpuTimings);
		RenderAsBars(cpuTimings, gpuTimings);
		break;
	case E_TimingRender_Bars_GPU:
		GatherGPUTimings(gpuTimings);
		RenderAsBars(cpuTimings, gpuTimings);
		break;
	case E_TimingRender_Text_CPU_GPU:
		GatherCPUTimings(cpuTimings);
		GatherGPUTimings(gpuTimings);
		RenderAsText(cpuTimings, gpuTimings);
		break;
	case E_TimingRender_Text_CPU:
		GatherCPUTimings(cpuTimings);
		RenderAsText(cpuTimings, gpuTimings);
		break;
	case E_TimingRender_Text_GPU:
		GatherGPUTimings(gpuTimings);
		RenderAsText(cpuTimings, gpuTimings);
		break;
	case E_TimingRender_Uber_GPU:
		GatherGPUTimings(gpuTimings);
		RenderAsUber(cpuTimings, gpuTimings);
		break;
	}

	cpuTimings.Clear();
	gpuTimings.Clear();
}

C_TimingRender* g_pTimingRender = NULL;

void TimingRender(C_TimingRender* pTimingRender)
{
	g_pTimingRender = pTimingRender;
}

}; // namespace Display
