#include "FrameworkPch.h"

#include "system/automation/Automation.h"
#include "system\datalogging\DataLogging.h"

#include <camera2/stock/DebugFeed.h>
#include "Framework/Application/ApplicationBase.h"
#include "Framework/Application/PerformanceLogging.h"
#include "Framework/View/View.h"


namespace PerformanceLogging
{

#ifndef CFG_GOLD

Debug::Logger &g_Log( Debug::GetLogger("performance.logging") );

int gDataLogHandle	= -1;

bool gInSubSession		= false;
bool gRecordingPerf		= false;
bool gLoading			= false;
bool gCaptureRacePerf	= false;
bool gStoppingPerfRun	= false;

Btl::String				gPerfComment;

TableCreationCallback	gCreateTableCallback	= NULL;
TableUpdateCallback		gUpdateTableCallback	= NULL;
UpdateCountersCallback	gUpdateCountersCallback	= NULL;

Time::C_Time g_timeLastUploadedStats	= 0;

Time::C_Time g_timeAtStartOfPerfRun;			// Time at start of perf session

Time::C_Time g_timeFrameStartPrevious;

Time::C_Time g_timeFrameStart;
Time::C_Time g_timeFrameUpdateEnd;

Time::C_Time g_timeLoadingStart;
Time::C_Time g_timeLoadingStreamerPreFill;
Time::C_Time g_timeLoadingEnd;

Time::C_Time g_timeGpuBegin;

Time::C_Time g_loadingStall;

struct DlSubSession
{
	DataLog::DataLogF32*	m_dltLoadingTime;
	DataLog::DataLogF32*	m_dltLoadingStall;
	DataLog::DataLogString*	m_dltComment;
} gDlSubSession = { NULL };

struct DlPerfInfo
{
	DataLog::DataLogTable*	m_dlt;
	
	DataLog::DataLogF32*	m_dltFrameTime;
	DataLog::DataLogF32*	m_dltX;
	DataLog::DataLogF32*	m_dltY;
	DataLog::DataLogF32*	m_dltZ;

	DataLog::DataLogU32*	m_dltFreeMemory;
	
	DataLog::DataLogF32*	m_dltFrameLength;
	
	DataLog::DataLogF32*	m_dltCpuTotalMs;
	DataLog::DataLogF32*	m_dltCpuUpdateMs;
	DataLog::DataLogF32*	m_dltCpuRenderMs;
	
	DataLog::DataLogF32*	m_dltGpuTotalMs;

} gDlPerfInfo = { NULL };

typedef struct
{
	float			m_frameTime;		// Time in race
	float			m_x, m_y, m_z;		// Position

	DWORD			m_freeMemory;		// Bytes of free memory available

	float			m_frameLength;		// Time in 'ms' since previous frame (1000/m_frameTime = FPS)

	float			m_cpuTotalMs;
	float			m_cpuUpdateMs;
	float			m_cpuRenderMs;

	float			m_gpuTotalMs;

	LONGLONG		m_gpuTime;

	DataLog::DataLogEvent*	dle;

	bool			updatePresentRender;
	volatile bool	allocated;
} PerfLogData;

const int NUM_PERFLOG_SLOTS = 3;

PerfLogData		g_Stats[NUM_PERFLOG_SLOTS] = { 0 };

int lastSlot = 0;
int currentSlot = 0;

#endif

//DWORD freePages = 0;

void CreateTables()
{
#ifndef CFG_GOLD
	if (gDlPerfInfo.m_dlt == NULL)
	{
		DataLog::DataLogTable* dlt = new DataLog::DataLogTable(gDataLogHandle, "PerfStat");

		gDlPerfInfo.m_dltFrameTime		= dlt->AddF32("FrameTime");
		gDlPerfInfo.m_dltX				= dlt->AddF32("X");
		gDlPerfInfo.m_dltY				= dlt->AddF32("Y");
		gDlPerfInfo.m_dltZ				= dlt->AddF32("Z");
		
		gDlPerfInfo.m_dltFreeMemory		= dlt->AddU32("FreeMemory");
		
		gDlPerfInfo.m_dltFrameLength	= dlt->AddF32("FrameLength");
		
		gDlPerfInfo.m_dltCpuTotalMs		= dlt->AddF32("CpuTotal");
		gDlPerfInfo.m_dltCpuUpdateMs	= dlt->AddF32("CpuUpdate");
		gDlPerfInfo.m_dltCpuRenderMs	= dlt->AddF32("CpuRender");
		
		gDlPerfInfo.m_dltGpuTotalMs		= dlt->AddF32("GpuTotal");

		// Now we give the game's a chance to add their own fields onto the data we collect
		if (gCreateTableCallback != NULL)
			gCreateTableCallback(kPerfStat, dlt);

		dlt->Finalise();
		
		gDlPerfInfo.m_dlt			= dlt;
	}
#endif
}

void TableCreate(DataLog::eDataLogType type, DataLog::DataLogTable* dlt)
{
#ifndef CFG_GOLD
	if (type == DataLog::kDlSubSession)
	{
		BIZ_ASSERT_SPEED(gDlSubSession.m_dltLoadingTime == NULL);

		gDlSubSession.m_dltLoadingTime	= dlt->AddF32("LoadingTime");
		gDlSubSession.m_dltLoadingStall	= dlt->AddF32("LoadingStall");
		gDlSubSession.m_dltComment		= dlt->AddString("Comment", 64);

		if (gCreateTableCallback)
			gCreateTableCallback(kPerfSubSession, dlt);
	}
#endif
}

void TableUpdate(DataLog::eDataLogType type, DataLog::DataLogEvent* e)
{
#ifndef CFG_GOLD
	if (type == DataLog::kDlSubSession)
	{
		float totalLoadTime = (F32)(g_timeLoadingEnd - g_timeLoadingStart) / (F32)Time::GetPerformanceFrequency();
		float loadingStall = (F32)g_loadingStall / (F32)Time::GetPerformanceFrequency();

		e->Set(gDlSubSession.m_dltLoadingTime, totalLoadTime);
		e->Set(gDlSubSession.m_dltLoadingStall, loadingStall);
		e->Set(gDlSubSession.m_dltComment, gPerfComment.Chars());

		if (gUpdateTableCallback)
			gUpdateTableCallback(kPerfSubSession, e);
	}
#endif
}

/*
	PerfStart	- Begin logging data as soon as possible
	PerfStop	- Stop logging data
	
	PerfPass	- Warn the system this is a perf run and we should establish connection to perf logger
*/
bool AutomationCallback(Automation::CommandContext& commandContext)
{
#ifndef CFG_GOLD
	if (!strcmp(commandContext.Command(), "PerfStart"))
	{
		gPerfComment.Clear();

		const char* databaseName(NULL);
		const char* referenceString(NULL);

		if (commandContext.MessageReader().Read(databaseName))
		{
			// Allow us to override the reference string in the data log system to group together a performance pass
			if (commandContext.MessageReader().Read(referenceString))
			{
				DataLog::SetReference(gDataLogHandle, referenceString);
			}
		}

		DataLog::SetEnabledStatus(gDataLogHandle, true);

		memset(g_Stats, 0, sizeof(g_Stats));

		commandContext.SingleReply("PerfStart");
		gCaptureRacePerf = false;
		gStoppingPerfRun = false;
		gInSubSession = false;

		currentSlot = 0;
		lastSlot = 0;
	}
	else if (!strcmp(commandContext.Command(), "PerfStop"))
	{
		gStoppingPerfRun = true;
		gCaptureRacePerf = false;
		gRecordingPerf = false;
		for (int i=0; i < NUM_PERFLOG_SLOTS; i++)
		{
			delete g_Stats[i].dle;
			g_Stats[i].dle = NULL;
			g_Stats[i].allocated = false;
		}
		commandContext.SingleReply("PerfStop");
	}
	else if (!strcmp(commandContext.Command(), "PerfComment"))
	{
		const char* comment(NULL);

		if (commandContext.MessageReader().Read(comment))
		{
			gPerfComment = comment;
		}
		commandContext.SingleReply("PerfComment");
	}
	else
	{
		return false;
	}
#endif
	
	return true;
}

// Establishes the connection to the database and creates a database with the name passed in - needs to be unique per game
void Initialise(const char* perfName)
{
#ifndef CFG_GOLD
	Automation::RegisterCallback(AutomationCallback);

	// TODO: We need to also pass in some callbacks so we can say what we want called to add/update tables
	gDataLogHandle = DataLog::Connect(perfName, TableCreate, TableUpdate);

	CreateTables();
#endif
}

void Initialise(const char* perfName, TableCreationCallback createCallback, TableUpdateCallback updateCallback, UpdateCountersCallback updateCountersCallback)
{
#ifndef CFG_GOLD
	gCreateTableCallback	= createCallback;
	gUpdateTableCallback	= updateCallback;
	gUpdateCountersCallback	= updateCountersCallback;

	Initialise(perfName);
#endif
}

void BeginFrame()
{
#ifndef CFG_GOLD
	g_timeFrameStart = Time::GetPerformanceCounter();

	Time::C_Time diffSinceLastFrame = g_timeFrameStart - g_timeFrameStartPrevious;

	if (gLoading)
	{
		if (diffSinceLastFrame > g_loadingStall)
			g_loadingStall = diffSinceLastFrame;

		if (diffSinceLastFrame > Time::GetPerformanceFrequency())
		{
			LOGWARN(g_Log, "LOADING STALL: %.2fsecs MAX(%.2fsecs)\n", diffSinceLastFrame / (F32)Time::GetPerformanceFrequency(), g_loadingStall / (F32)Time::GetPerformanceFrequency());
		}
	}
	else if (gInSubSession && !gCaptureRacePerf && !gStoppingPerfRun)
	{
		Time::C_Time now = Time::GetPerformanceCounter();

		if ((now - g_timeAtStartOfPerfRun) >= Time::GetPerformanceFrequency())
		{
			gCaptureRacePerf = true;
		}
	}

	if (gInSubSession && gCaptureRacePerf)
	{
		gRecordingPerf = true;

		BIZ_ASSERT_SPEED(g_Stats[currentSlot].allocated == false);
		g_Stats[currentSlot].allocated = true;
	
		int prevCounter = (currentSlot-1+NUM_PERFLOG_SLOTS) % NUM_PERFLOG_SLOTS;
		if (g_Stats[prevCounter].allocated)
		{
			g_Stats[prevCounter].m_frameLength = diffSinceLastFrame / 1000.f;
		}

		// Calculate the amount of free memory available
		g_Stats[currentSlot].m_freeMemory = 0;
	}
	else
		gRecordingPerf = false;

	g_timeFrameStartPrevious = g_timeFrameStart;
#endif
}

void EndFrame()
{
#ifndef CFG_GOLD
	if (gRecordingPerf && gInSubSession)
	{
		const Time::C_Time currTime = Time::GetPerformanceCounter();

		PerfLogData& stats = g_Stats[currentSlot];

		// If we present in the middle now, we need to work out how longer Render took
		if (stats.updatePresentRender)
		{
			// Just the render time
			Time::C_Time diffFrameTime = currTime - g_timeFrameStart;

			stats.m_cpuRenderMs = diffFrameTime / 1000.f;
			stats.m_cpuTotalMs += stats.m_cpuRenderMs;
		}

		Time::C_Time timeDiff = currTime - g_timeLastUploadedStats;

		// Upload interval - 5 times per second
		bool uploadStats = (timeDiff * 5 >= Time::GetPerformanceFrequency());

		if (gDlPerfInfo.m_dlt)
		{
			if (!stats.dle)
				stats.dle = new DataLog::DataLogEvent(gDlPerfInfo.m_dlt);

			stats.dle->Set(gDlPerfInfo.m_dltFrameTime,		stats.m_frameTime);
			stats.dle->Set(gDlPerfInfo.m_dltX,				stats.m_x);
			stats.dle->Set(gDlPerfInfo.m_dltY,				stats.m_y);
			stats.dle->Set(gDlPerfInfo.m_dltZ,				stats.m_z);
			
			stats.dle->Set(gDlPerfInfo.m_dltFreeMemory,		stats.m_freeMemory);
			
			stats.dle->Set(gDlPerfInfo.m_dltCpuTotalMs,		stats.m_cpuTotalMs);
			stats.dle->Set(gDlPerfInfo.m_dltCpuUpdateMs,	stats.m_cpuUpdateMs);
			stats.dle->Set(gDlPerfInfo.m_dltCpuRenderMs,	stats.m_cpuRenderMs);
			
			// Now we give the game's a chance to update their custom fields
			if (gUpdateTableCallback != NULL)
				gUpdateTableCallback(kPerfStat, stats.dle);
		}
		
		if (uploadStats)
			g_timeLastUploadedStats = currTime;

#ifdef PLATFORM_PS3
		if (g_Stats[lastSlot].m_frameLength != 0)
		{
			float gpuTime = Display::Device()->GetGpuTime();

			g_Stats[lastSlot].m_gpuTime = (LONGLONG)(gpuTime * 1000.f);
		}
#endif

		while (g_Stats[lastSlot].m_gpuTime != 0)
		{
			PerfLogData& stats = g_Stats[lastSlot];
			
			BIZ_ASSERT_SPEED(stats.allocated == true);

			// Convert time to 'ms'
			stats.m_gpuTotalMs	= stats.m_gpuTime / 1000.f;

			if (uploadStats)
			{
				// Send stats to data log server
				if (gRecordingPerf && gDlPerfInfo.m_dlt && stats.dle)
				{
					stats.dle->Set(gDlPerfInfo.m_dltFrameLength,	stats.m_frameLength);
					stats.dle->Set(gDlPerfInfo.m_dltGpuTotalMs,		stats.m_gpuTotalMs);

					stats.dle->Send();

					delete stats.dle;
					stats.dle = NULL;
				}
			}

			stats.m_frameLength = 0;
			stats.m_gpuTime	= 0;
			stats.allocated = false;

			lastSlot = (lastSlot+1) % NUM_PERFLOG_SLOTS;
		}
		
		currentSlot = (currentSlot+1) % NUM_PERFLOG_SLOTS;
	}
#endif
}

void UpdateCounters(ePerfStage pi)
{
#ifndef CFG_GOLD
	if (gUpdateCountersCallback != NULL)
		gUpdateCountersCallback(pi);

	switch (pi)
	{
	case kGameBeginFrame:
		BeginFrame();
		break;

	case kGameEndFrame:
		EndFrame();
		break;

	case kGameUpdateFinished:
		if (gRecordingPerf)
		{
			g_timeFrameUpdateEnd = Time::GetPerformanceCounter();

			Time::C_Time diff = g_timeFrameUpdateEnd - g_timeFrameStart;
			g_Stats[currentSlot].m_cpuUpdateMs = diff / 1000.f;

			const Cam2::C_DebugFeed& df = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenView(0).DebugFeed();

			const Maths::HVec3 dfPos = df.Position();

			PerfLogData& stats = g_Stats[currentSlot];

			BIZ_ASSERT_SPEED(stats.allocated == true);

			const Time::C_Time currTime = Time::GetPerformanceCounter();
			float timeDiff = (F32)(currTime - g_timeAtStartOfPerfRun) / (F32)Time::GetPerformanceFrequency();

			stats.m_frameTime = timeDiff;
			stats.m_x = dfPos.X();
			stats.m_y = dfPos.Y();
			stats.m_z = dfPos.Z();

			int viewCount = Framework::C_ApplicationBase::GetInstance()->GetSplitscreenViewCount();
			//CORE_PRINT("CamPos: %d - %.2f, %.2f, %.2f\n", viewCount, (float)dfPos.X(), (float)dfPos.Y(), (float)dfPos.Z());
		}
		break;

	case kGameSubSessionStarted:
		g_timeLoadingStart = Time::GetPerformanceCounter();
		gLoading = true;
		g_loadingStall = 0;
		g_timeFrameStartPrevious = Time::GetPerformanceCounter();
		break;

	case kGameSubSessionEnded:
		if (gInSubSession)
		{
			gInSubSession = false;
			DataLog::EndSubSession(gDataLogHandle);
		}
		break;
	
	case kGameSubSessionLoaded:
		g_timeLoadingEnd = Time::GetPerformanceCounter();

		float totalLoadTime = (F32)(g_timeLoadingEnd - g_timeLoadingStart) / (F32)Time::GetPerformanceFrequency();

		LOGINFO(g_Log, "PERF: Total Load time: %.2fsecs\n", totalLoadTime);

		gInSubSession = false;
		if (DataLog::IsConnected(gDataLogHandle))
		{
			if (!gInSubSession)
			{
				gInSubSession = true;
				
				DataLog::BeginSubSession(gDataLogHandle);
				g_timeAtStartOfPerfRun = Time::GetPerformanceCounter();
			}
		}

		gLoading = false;
		break;
	}
#endif
}

const Time::C_Time QueryCounter(ePerfStage pi)
{
#ifndef CFG_GOLD
	switch (pi)
	{
	case kGameSubSessionStarted:
		return g_timeLoadingStart;
	case kGameSubSessionLoaded:
		return g_timeLoadingEnd;

	default:
		return 0;
	}
#else
	return 0;
#endif
}


void BeginGPU(Display::C_Device* pDevice)
{
#ifndef CFG_GOLD
	if (gRecordingPerf)
	{
	}
#endif
}

void PrePresent(Display::C_Device* pDevice, bool updatePresentRender)
{
#ifndef CFG_GOLD
	if (gRecordingPerf)
	{
		PerfLogData& stats = g_Stats[currentSlot];

		stats.updatePresentRender = updatePresentRender;

		Time::C_Time timeFrameEnd = Time::GetPerformanceCounter();
		Time::C_Time diffFrameTime = timeFrameEnd - g_timeFrameStart;

		stats.m_cpuTotalMs = diffFrameTime / 1000.f;

		// If we're doing update-present-render then we don't have numbers for Render just yet
		if (!updatePresentRender)
		{
			stats.m_cpuRenderMs = stats.m_cpuTotalMs - stats.m_cpuUpdateMs;
		}

	}
#endif
}

void PostPresent(Display::C_Device* pDevice)
{
#ifndef CFG_GOLD
	if (gRecordingPerf && g_Stats[currentSlot].updatePresentRender)
	{
		// Reset the clock for Render
		g_timeFrameStart = Time::GetPerformanceCounter();
	}
#endif
}

}
