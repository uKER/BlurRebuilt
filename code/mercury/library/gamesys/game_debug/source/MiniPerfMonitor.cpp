#include "GameDebugPch.h"

#include "game_debug/MiniPerfMonitor.h"

#include <Display/Timing/Timings.h>
#include <Display/Timing/GPUTiming.h>
#include <Display/Render/BasicRender.h>
#include <Display/Render/FontRender.h>

#include <Lod/LodManager.h>

namespace GameDebug
{

Debug::Bool		g_debugMiniMonitorEnable	("GameSys.MiniMonitor.Enable",		true);	// [Jan] DO NOT MAKE THIS A PERSIST

Debug::Float	g_debugMiniMonitorPosX		("GameSys.MiniMonitor.PosX",		30.f, Debug::Range(0, 1024));
Debug::Float	g_debugMiniMonitorPosY		("GameSys.MiniMonitor.PosY",		670.f, Debug::Range(0, 768));
Debug::Float	g_debugMiniMonitorSize		("GameSys.MiniMonitor.Size",		10.3f, Debug::Range(0, 50));

Debug::Float	g_debugMiniMonitorColumn1	("GameSys.MiniMonitor.Column1",		47.f, Debug::Range(0, 100));
Debug::Float	g_debugMiniMonitorGraphW	("GameSys.MiniMonitor.GraphWidth",	80.f, Debug::Range(0, 50));

Debug::Float	g_debugMiniMonitorBarYOffset("GameSys.MiniMonitor.GraphYOffset", 1.5f, Debug::Range(-10.f, 10.f));
Debug::Float	g_debugMiniMonitorGraphBarH ("GameSys.MiniMonitor.GraphBarHeight", 5.f, Debug::Range(0.f, 10.f));

Debug::Float	g_debugMiniMonitorFPSX		("GameSys.MiniMonitor.FPSX",		80.0f, Debug::Range(0.f, 100.f));
Debug::Float	g_debugMiniMonitorFPSY		("GameSys.MiniMonitor.FPSY",		0.f, Debug::Range(-40.f, 40.f));
Debug::Float	g_debugMiniMonitorFPSScale	("GameSys.MiniMonitor.FPSScale",	15.f, Debug::Range(0.f,20.f));

struct TimingStats
{
	const Perf::C_CPUTimingStats *pCPU, *pCPUPresent, *pPhysics, *pUpdate, *pRender;
	const Display::C_GPUTimingStats *pGPU, *pGPUPresent;

	float fMax[5];
};

/// constructor
C_MiniPerfMonitor::C_MiniPerfMonitor()
{
	for (int i=0; i<5; i++)
		m_fLimit[i] = 33.f;
}

/// initialise
void C_MiniPerfMonitor::Initialise()
{
	m_pStats = new TimingStats;

	m_iMaxReset = 0;

	for (int i=0; i<5; i++)
	{
		m_fAvg[i] = 0.f;
		m_fLastAvg[i] = 0.f;
	}
	m_fFPSAvg = 0;
	m_fFPSLastAvg = 0;
	m_iAvgCount = 0;

	m_pStats->pCPU = Perf::C_CPUTimingStats::FindStats("CPU");
	m_pStats->pGPU = Display::C_GPUTimingStats::FindStats("GPU");
	m_pStats->pCPUPresent = Perf::C_CPUTimingStats::FindStats("CPU.Present");
	m_pStats->pGPUPresent = Display::C_GPUTimingStats::FindStats("GPU.Present");
	m_pStats->pRender = Perf::C_CPUTimingStats::FindStats("CPU.Render");
	m_pStats->pUpdate = Perf::C_CPUTimingStats::FindStats("CPU.Update");
	m_pStats->pPhysics = NULL;
}

/// free
void C_MiniPerfMonitor::Free()
{
	delete m_pStats;
}

float GetTime(const Perf::C_CPUTimingStats* p)
{
	if (!p)
		return 0;

	return ((F32)p->GetTotalTime(Perf::E_TimingFrame_Prev) / (F32)SYS_USEC_PER_SEC) * 1000.f;
}

float GetTime(const Display::C_GPUTimingStats* p)
{
	if (!p)
		return 0;

	return (F32)p->GetTotalTime(Perf::E_TimingFrame_Prev) / 1000000.0f;
}

/// set limits for each subsystem
void C_MiniPerfMonitor::SetLimits(float fGPU, float fCPU, float fPhysics, float fUpdate, float fGraphics)
{
	m_fLimit[0] = fGPU;
	m_fLimit[1] = fCPU;
	m_fLimit[2] = fPhysics;
	m_fLimit[3] = fUpdate;
	m_fLimit[4] = fGraphics;
}

#define NUM_AVERAGE_FRAMES 4

/// render the graph
void C_MiniPerfMonitor::Render()
{
	if (!g_debugMiniMonitorEnable)
		return;

	PERF_EVENT("MiniPerfMonitor.Render");

	m_iMaxReset--;
	if (m_iMaxReset <= 0)
	{
		for (int i=0; i<5; i++)
		m_fMax[i] = 0;
	}

	if (!m_pStats->pPhysics)
		m_pStats->pPhysics = Perf::C_CPUTimingStats::FindStats("CPU.Update.Physics");

	// get values out
	float fCPUPresent = GetTime(m_pStats->pCPUPresent);
	float fGPUPresent = GetTime(m_pStats->pGPUPresent);
	float fCPUTotal = GetTime(m_pStats->pCPU);
	float fCPU = fCPUTotal - fCPUPresent;
	float fGPU = GetTime(m_pStats->pGPU) - fGPUPresent;
	float fPhysics = GetTime(m_pStats->pPhysics);
	float fUpdate = GetTime(m_pStats->pUpdate) - fPhysics;
	float fRender = GetTime(m_pStats->pRender) - fCPUPresent;

	static const char *pszStrings[] = { "GPU", "CPU", "Physics", "Update", "Render" };
	static const Maths::RGBA colours[] = { Maths::RGBA(255, 255, 200), Maths::RGBA(200, 220, 220), Maths::RGBA(160,160,255), Maths::RGBA(50,255,50), Maths::RGBA(255,140,80) };
	float fValues[] = { fGPU, fCPU, fPhysics, fUpdate, fRender };
	float fBarValues[5];

	float fFPS = 999.f/fCPUTotal;

	m_iAvgCount--;

	// clamp bar values and calculate averages for the numbers
	for (int i=0; i<5; i++)
	{
		fBarValues[i] = Maths::Min(fValues[i], 66.f);
		if (fBarValues[i] > m_fMax[i])
		{
			m_fMax[i] = fValues[i];
			m_iMaxReset = 30;
		}

		m_fAvg[i] += fValues[i];

		if (m_iAvgCount <= 0)
		{
			m_fLastAvg[i] = Maths::Min(99.99f, m_fAvg[i] / (F32)NUM_AVERAGE_FRAMES);
			m_fAvg[i] = 0;
		}
	}

	m_fFPSAvg += fFPS;
	if (m_iAvgCount <= 0)
	{
		m_fFPSLastAvg = m_fFPSAvg/(F32)NUM_AVERAGE_FRAMES;
		m_fFPSAvg = 0;
		m_iAvgCount = NUM_AVERAGE_FRAMES;
	}

	// draw
	F32 posx = (F32)g_debugMiniMonitorPosX;
	F32 posy = (F32)g_debugMiniMonitorPosY;
	F32 s = (F32)g_debugMiniMonitorSize;

	F32 yOffset = g_debugMiniMonitorBarYOffset;
	F32 barH = g_debugMiniMonitorGraphBarH;
	F32 graphW = g_debugMiniMonitorGraphW;
	F32 col = g_debugMiniMonitorColumn1;

#if CFG_DEBUG
	F32 fBuildAdjust = 100.f;
#elif CFG_RELEASE
	F32 fBuildAdjust  = 4.f;
#else
	F32 fBuildAdjust = 1.f;
#endif

	F32 fTotalW = s*3.f + graphW + col;

	Display::BasicRender()->DrawColoured2DRect(posx, posy-s*0.5f, fTotalW, (s+5.f)*5.f, Maths::RGBA(0, 0, 0, 128));

	for(U32 i=0; i<5; i++)
	{
		F32 x = posx+s*3.f+col+((F32)i)*(graphW/4.0f);
		F32 y = posy-s*0.5f;
		F32 w = 0.5f;
		F32 h = (s+5.f)*5.f;

		Display::BasicRender()->DrawColoured2DRect(x, y, w, h, Maths::RGBA(0, 0, 0, 128));
	}

	char buffer[128];
	F32 x = posx;
	F32 y = posy;

	for (int i=0; i<5; i++)
	{
		F32 x1 = x+s*3.0f;
		F32 f = m_fLastAvg[i];

		Display::FontRender()->Draw(pszStrings[i], x1, y, s, s, colours[i], Display::E_FontRenderAlign_Lt);

		x1 -= 2.f;

		sprintf(buffer, "%.2f", f);
		Display::FontRender()->Draw(buffer, x1, y, s, s, colours[i], Display::E_FontRenderAlign_Rt);

		x1 += 2.f + col;

		if (f > m_fLimit[i] * fBuildAdjust)
			Display::BasicRender()->DrawColoured2DRect(x, y-yOffset-4.f, fTotalW, barH+8.f, Maths::RGBA(255, 0, 0, 128));

		f = fBarValues[i];
		F32 w = f > 0.0f ? ((f / 32.0f) * graphW) : 0.0f;
		Display::BasicRender()->DrawColoured2DRect(x1, y-yOffset, w, barH , colours[i]);

		F32 m = m_fMax[i];
		m = m > 0.0f ? ((m / 32.0f) * graphW) : 0.0f;
		Display::BasicRender()->DrawColoured2DRect(x1+m, y-yOffset, 1.f, barH , colours[i]);

		y += s+5.0f;
	}

	sprintf_s(buffer, sizeof(buffer), "%3.1f FPS", m_fFPSLastAvg);
	Display::FontRender()->Draw(buffer, x+g_debugMiniMonitorFPSX, g_debugMiniMonitorPosY+g_debugMiniMonitorFPSY-g_debugMiniMonitorFPSScale, g_debugMiniMonitorFPSScale, g_debugMiniMonitorFPSScale, Maths::RGBA(255,255,255,255), Display::E_FontRenderAlign_Ct);
}

}; // namespace GameDebug
