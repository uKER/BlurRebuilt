#include "stdafx.h"
#include "Display/Timing/GPUTiming.h"

#ifdef PLATFORM_PS3
#include "Display/Device/GCM/GCM.h"
#endif

#include "System/Perf/ProfilerEnable.h"

#ifdef PLATFORM_PC
#include "Display/Device/Device.h"	// [Blur2PC] D3D9 timestamp queries
#endif

#ifdef ENABLE_MERCURY_PROFILER
#include <System/Perf/Profiler.h>
#include <System/Perf/ProfilerBackend.h>
#endif

Debug::Int		g_debugDisplayGPUTimingDepth	("Graphics.Display.Render.TimingRender.GPUDepth", 		9, Debug::Range(0, 9));
Debug::Int		g_debugDisplayGPUTimingLevel	("Graphics.Display.Render.TimingRender.GPULevel", 		3, Debug::Range(0, 9));
Debug::Bool		g_debugDisplayGPUUpdateStats	("Graphics.Display.Render.TimingRender.GPUUpdateStats",	true);


namespace Display
{

#ifdef PLATFORM_PC

// [Blur1PCDX11] Native timestamp queries and disjoint query per frame.
namespace
{
S32 s_PCTimingState = -1;
bool s_PCInFrame = false;
U32 s_PCFrame = 0;
ID3D11Query* s_pPCDisjoint[C_GPUTimingStats::m_FrameCount] = {};
bool s_PCFrameIssued[C_GPUTimingStats::m_FrameCount] = {};

ID3D11DeviceContext* TimingContext()
{
    ID3D11Device* device = Device()->GetD3DDevice();
    ID3D11DeviceContext* context = nullptr;
    if (device) device->GetImmediateContext(&context);
    return context;
}

bool PCTimingReady()
{
    if (s_PCTimingState < 0)
    {
        s_PCTimingState = 0;
        ID3D11Device* device = Device()->GetD3DDevice();
        if (device)
        {
            D3D11_QUERY_DESC desc = { D3D11_QUERY_TIMESTAMP_DISJOINT, 0 };
            s_PCTimingState = 1;
            for (U32 i = 0; i < C_GPUTimingStats::m_FrameCount; ++i)
                if (FAILED(device->CreateQuery(&desc, &s_pPCDisjoint[i])))
                    s_PCTimingState = 0;
        }
    }
    return s_PCTimingState == 1;
}
}

void C_GPUTimingStats::ReleaseQueries()
{
    for (C_GPUTimingStats* stats = m_StatsList.First(); stats; stats = stats->Next())
        for (U32 frame = 0; frame < m_FrameCount; ++frame)
        {
            for (U32 pair = 0; pair < m_PairCount; ++pair)
                for (U32 edge = 0; edge < 2; ++edge)
                {
                    if (stats->m_pQueries[frame][pair][edge])
                        stats->m_pQueries[frame][pair][edge]->Release();
                    stats->m_pQueries[frame][pair][edge] = nullptr;
                }
            stats->m_PairsUsed[frame] = 0;
        }
    for (U32 i = 0; i < m_FrameCount; ++i)
    {
        if (s_pPCDisjoint[i]) s_pPCDisjoint[i]->Release();
        s_pPCDisjoint[i] = nullptr;
        s_PCFrameIssued[i] = false;
    }
    s_PCTimingState = -1;
    s_PCInFrame = false;
}

#endif // PLATFORM_PC

// C_GPUTimingStats

Util::C_List<C_GPUTimingStats> C_GPUTimingStats::m_StatsList;

C_GPUTimingStats::C_GPUTimingStats(const S32 level, const Maths::RGBA colour, const Util::C_String80& name) :
	C_TimingStats(level, colour, name)
{
	#ifdef PLATFORM_PS3
	m_TimingRead	= 1;
	m_TimingWrite	= 0;
	for(U32 i=0; i<m_TimingCount; i++)
		m_TimingIndex[i] = 0;
	#endif


	#ifdef PLATFORM_PC
	memset(m_pQueries, 0, sizeof(m_pQueries));
	memset(m_PairsUsed, 0, sizeof(m_PairsUsed));
	m_DisplayTime	= 0;
	#endif

	m_StatsList.PushLast(this);
}

C_GPUTimingStats::~C_GPUTimingStats()
{
	m_StatsList.Remove(this);
}

const bool C_GPUTimingStats::Show() const
{
	if(m_Depth != -1 && m_Depth <= g_debugDisplayGPUTimingDepth && m_Level <= g_debugDisplayGPUTimingLevel)
	{
		return true;
	}
	else
	{
		return false;
	}
}

const C_GPUTimingStats *C_GPUTimingStats::FindStats(const char *psz)
{
	for (C_GPUTimingStats* pStats = m_StatsList.First(); pStats; pStats = pStats->Next())
	{
		if (!stricmp(pStats->m_Name.Chars(), psz))
			return pStats;
	}

	return NULL;
}

void C_GPUTimingStats::UpdateStats()
{
	if(!g_debugDisplayGPUUpdateStats)
		return;

	#ifdef PLATFORM_PS3
	//cellGcmFinish(0);
	#endif

	#ifdef PLATFORM_PC
	// [Blur2PC] the oldest issued frame, in the slot the next BeginFrame reuses
	const U32 readSlot = s_PCFrame % m_FrameCount;
	bool pcFrameReady = false;
	U64 pcFreq = 0;
	if(s_PCTimingState == 1 && s_PCFrameIssued[readSlot])
	{
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjoint = {};
		ID3D11DeviceContext* context = TimingContext();
		if (context)
		{
			if (context->GetData(s_pPCDisjoint[readSlot], &disjoint,
				 sizeof(disjoint), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK &&
				!disjoint.Disjoint && disjoint.Frequency)
			{
				pcFreq = disjoint.Frequency;
				pcFrameReady = true;
			}
			context->Release();
		}
		s_PCFrameIssued[readSlot] = false;
	}
	#endif

	C_GPUTimingStats* pTimingStats = m_StatsList.First();
	while(pTimingStats)
	{
		if(pTimingStats->m_Enabled)
		{
			#ifdef PLATFORM_PS3
			U64 oldTime = cellGcmGetTimeStamp(pTimingStats->m_TimingIndex[pTimingStats->m_TimingRead]);
			U64 newTime = cellGcmGetTimeStamp(pTimingStats->m_TimingIndex[pTimingStats->m_TimingRead]+1);
			U64 theTime = newTime-oldTime;
			pTimingStats->m_Frames[Perf::E_TimingFrame_Curr].m_TotalTime = theTime;
			pTimingStats->AdvanceTimingRead();
			pTimingStats->AdvanceTimingWrite();

			#ifdef ENABLE_MERCURY_PROFILER
			Perf::EmitEvent(pTimingStats->m_Name.Chars(), oldTime/14, -1, 0, 0);
			Perf::EmitEvent(NULL, newTime/14, -1, 0, 1);
			#endif

			#endif


			#ifdef PLATFORM_PC
			// [Blur2PC] keeps the last good value when a frame's queries are not ready or were disjoint
			if(pcFrameReady && pTimingStats->m_PairsUsed[readSlot])
			{
				U64 ticks = 0;
				bool complete = true;
				for(U32 p=0; p<pTimingStats->m_PairsUsed[readSlot] && complete; p++)
				{
					U64 t0 = 0, t1 = 0;
					ID3D11DeviceContext* context = TimingContext();
					const bool ready = context &&
						context->GetData(pTimingStats->m_pQueries[readSlot][p][0], &t0,
							 sizeof(t0), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK &&
						context->GetData(pTimingStats->m_pQueries[readSlot][p][1], &t1,
							 sizeof(t1), D3D11_ASYNC_GETDATA_DONOTFLUSH) == S_OK;
					if (context) context->Release();
					if(ready)
					{
						ticks += (t1 > t0) ? t1-t0 : 0;
					}
					else
					{
						complete = false;
					}
				}
				if(complete)
					pTimingStats->m_DisplayTime = (U64)((double)ticks * 1000000000.0 / (double)pcFreq);
			}
			pTimingStats->m_Frames[Perf::E_TimingFrame_Curr].m_TotalTime = pTimingStats->m_DisplayTime;
			#endif
		}
		else
		{
			pTimingStats->m_Frames[Perf::E_TimingFrame_Curr].m_TotalTime = 0;
		}
		
		pTimingStats->Update();

		pTimingStats = pTimingStats->Next();
	}
}

void C_GPUTimingStats::ResetPeaks()
{
	C_GPUTimingStats* pStats = m_StatsList.First();
	while(pStats)
	{
		pStats->ResetPeak();
		pStats = pStats->Next();
	}
}

void C_GPUTimingStats::BeginFrame()
{
	#ifdef PLATFORM_PC
	if(PCTimingReady())
	{
		// [Blur2PC] this slot was read (or given up on) by the last UpdateStats
		const U32 slot = s_PCFrame % m_FrameCount;
		for(C_GPUTimingStats* pStats = m_StatsList.First(); pStats; pStats = pStats->Next())
			pStats->m_PairsUsed[slot] = 0;
		s_PCFrameIssued[slot] = false;
		ID3D11DeviceContext* context = TimingContext();
		if (context) { context->Begin(s_pPCDisjoint[slot]); context->Release(); }
		s_PCInFrame = true;
	}
	#endif

}

void C_GPUTimingStats::EndFrame()
{
	#ifdef PLATFORM_PC
	if(s_PCInFrame)
	{
		const U32 slot = s_PCFrame % m_FrameCount;
		ID3D11DeviceContext* context = TimingContext();
		if (context) { context->End(s_pPCDisjoint[slot]); context->Release(); }
		s_PCFrameIssued[slot] = true;
		s_PCFrame++;
		s_PCInFrame = false;
	}
	#endif

}

// C_GPUTimingEvent

MT::ThreadLocalStorage C_GPUTimingEvent::m_Tls;

C_GPUTimingEvent::C_GPUTimingEvent(C_GPUTimingStats& stats) // __restrict // JASOND: Intel compiler doesn't allow this...
{
	C_GPUTimingEvent* const __restrict pParent = (C_GPUTimingEvent*)m_Tls.GetData();
	m_Tls.SetData(this);

	m_pParent	= pParent;
	m_pStats	= &stats;
	m_Depth		= m_pParent ? m_pParent->m_Depth+1 : m_Depth = 0;

	#ifdef PLATFORM_PC
	m_Pair		= -1;
	#endif
	
	if(m_Depth <= g_debugDisplayGPUTimingDepth && stats.m_Level <= g_debugDisplayGPUTimingLevel)
	{
		stats.SetDepth(m_Depth);

		#ifdef PLATFORM_PC
		// [Blur2PC] events outside Begin/EndFrame (device resize, loading screen) are not timed
		if(s_PCInFrame)
		{
			const U32 slot = s_PCFrame % C_GPUTimingStats::m_FrameCount;
			const U32 pair = stats.m_PairsUsed[slot];
			if(pair < C_GPUTimingStats::m_PairCount)
			{
				ID3D11Query** ppQuery = stats.m_pQueries[slot][pair];
				ID3D11Device* pDevice = Device()->GetD3DDevice();
				const D3D11_QUERY_DESC desc = { D3D11_QUERY_TIMESTAMP, 0 };
				for(U32 e=0; e<2; e++)
				{
					if(!ppQuery[e] && FAILED(pDevice->CreateQuery(&desc, &ppQuery[e])))
						ppQuery[e] = NULL;
				}
				if(ppQuery[0] && ppQuery[1])
				{
					ID3D11DeviceContext* context = TimingContext();
					if (context) { context->End(ppQuery[0]); context->Release(); }
					stats.m_PairsUsed[slot] = pair+1;
					m_Pair = (S32)pair;
				}
			}
		}
		#endif

		#ifdef PLATFORM_PS3
		m_Index = Display::GCM::GCMGetNextReportIndex2();
		GCMSetTimeStamp(m_Index);
		#endif


		stats.m_Enabled = true;
	}
	else
	{
		stats.m_Enabled = false;
	}
}

C_GPUTimingEvent::~C_GPUTimingEvent() // __restrict // JASOND: Intel compiler doesn't allow this...
{
	C_GPUTimingStats* const __restrict pTimings(m_pStats);
	C_GPUTimingEvent* const __restrict pParent(m_pParent);
	
	pTimings->AddCallCount(Perf::E_TimingFrame_Curr, 1);
	
	m_Tls.SetData(pParent);

	if(m_Depth <= g_debugDisplayGPUTimingDepth && pTimings->m_Level <= g_debugDisplayGPUTimingLevel)
	{
		#ifdef PLATFORM_PS3
		pTimings->m_TimingIndex[pTimings->m_TimingWrite] = m_Index;
		GCMSetTimeStamp(m_Index+1);
		#endif

		#ifdef PLATFORM_PC
		if(m_Pair >= 0 && s_PCInFrame)
		{
			ID3D11DeviceContext* context = TimingContext();
			if (context)
			{
				context->End(pTimings->m_pQueries[s_PCFrame % C_GPUTimingStats::m_FrameCount][m_Pair][1]);
				context->Release();
			}
		}
		#endif

	}
}

} // namespace Display
