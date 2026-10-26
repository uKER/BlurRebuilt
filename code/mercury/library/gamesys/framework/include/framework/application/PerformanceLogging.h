#ifndef PERFORMANCELOGGING_H
#define PERFORMANCELOGGING_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Time
{
	typedef U64 C_Time;
}

namespace Display
{
	class C_Device;
}

namespace DataLog
{
	class DataLogTable;
	class DataLogEvent;
}

namespace PerformanceLogging
{
	enum ePerfLogType
	{
		kPerfSubSession,
		kPerfStat
	};

	enum ePerfStage
	{
		kGameBeginFrame,
		kGameEndFrame,

		kGameUpdateFinished,
		
		kGameSubSessionStarted,
		kGameSubSessionEnded,
		kGameSubSessionLoaded,
	};

	typedef void (*TableCreationCallback)(ePerfLogType type, DataLog::DataLogTable* dlt);
	typedef void (*TableUpdateCallback)(ePerfLogType type, DataLog::DataLogEvent* e);
	
	typedef void (*UpdateCountersCallback)(ePerfStage pi);

	void Initialise(const char* perfName);
	void Initialise(const char* perfName, TableCreationCallback createCallback, TableUpdateCallback updateCallback, UpdateCountersCallback updateCountersCallback=NULL);

	void UpdateCounters(ePerfStage pi);

	const Time::C_Time QueryCounter(ePerfStage pi);

	void Enable(bool enable);

	void BeginGPU(Display::C_Device* pDevice);
	
	void PrePresent(Display::C_Device* pDevice, bool updatePresentRender);
	void PostPresent(Display::C_Device* pDevice);

}

#endif // PERFORMANCELOGGING_H