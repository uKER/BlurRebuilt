#include "stdafx.h"
#include "streamingstatistics.h"
#include "system/Perf/PerfEvent.h"

//#pragma optimize("", off)

namespace RenderingData {
namespace Target
{

static Debug::Logger &g_LogTiming = Debug::GetLogger("graphics.renderingdata.streamer.timing");

LoadingStatistics LoadingStatistics::s_instance;

void LoadingStatistics::Initialise()
{
	g_statsPreload.Initialise();
	g_streaming.Initialise();
	g_current.Initialise();
	g_previous.Initialise();
}

void LoadingStatistics::SetPreLoad(bool preLoad)
{
	if (preLoad)
	{
		g_statsPreload.Initialise();
		preLoad = true;
	}
	else
	{
		g_streaming.Initialise();
		preLoad = false;
	}

	g_current.Initialise();
	g_previous.Initialise();
}

void LoadingStatistics::Update()
{
	PERF_EVENT("LoadingStatistics::Update");

	g_current.frames++;
	g_streaming.frames++;

	Time::C_Time time = Time::GetPerformanceCounter();

	if (time - g_current.startTime > 1000000) // 1 per second
	{
		g_previous = g_current;
		g_current.Initialise();
		
		{
			LOGDEBUG(g_LogTiming, "Current");
			g_previous.Print();
			LOGDEBUG(g_LogTiming, "Total");
			g_streaming.Print();
			//LOGDEBUG(g_LogTiming, "Pre-load");
			//g_statsPreload.Print();
		}
	}
}

void LoadingStatistics::WaitForTasks()
{
	g_current.waitForTasks++;
	g_streaming.waitForTasks++;
}

void LoadingStatistics::IsLoadingChunk()
{
	g_current.isLoadingChunk++;
	g_streaming.isLoadingChunk++;
}

void LoadingStatistics::NoRequests()
{
	g_current.noRequests++;
	g_streaming.noRequests++;
}

void LoadingStatistics::Read(U64 offset, U64 size)
{
	if (preLoad)
		g_statsPreload.Read(offset, size);
	else
		g_streaming.Read(offset, size);

	g_current.Read(offset, size);

	m_startTime = Time::GetPerformanceCounter();
}

float LoadingStatistics::LoadingTime() const
{
	return (Time::GetPerformanceCounter() - m_startTime) / 1000000.f;
}

void LoadingStatistics::Stats::Initialise() 
{
	memset(this, 0, sizeof(*this));
	startTime = Time::GetPerformanceCounter();
}

void LoadingStatistics::Stats::Read(U64 offset, U64 size)
{
	lastOffset = offset + size;
	bytesRead += size;
	nReads++;
}

void LoadingStatistics::Stats::Print()
{
	Time::C_Time time = Time::GetPerformanceCounter();
	Time::C_Time duration = time - startTime;
	float m = 1000000.f;

	float bytesPerSecond = bytesRead * m / duration;
	float readsPerSecond = nReads * m / duration;
	mbPerSecond = bytesPerSecond / (1024 * 1024);
	
	LOGDEBUG(g_LogTiming, "%0.2f Mb / s", bytesPerSecond / (1024 * 1024));
	LOGDEBUG(g_LogTiming, "%0.2f seeks / s", readsPerSecond);

	if (nReads > 0)
	{
		float averageRead = (float)bytesRead / nReads;
		LOGDEBUG(g_LogTiming, "Average read: %0.2f Mb", averageRead / (1024 * 1024));
	}

	if (nReads > 1)
	{
		float averageSeek = (float)totalSeekDistance / (nReads - 1);
		LOGDEBUG(g_LogTiming, "Average seek: %0.2f Mb", averageSeek / (1024 * 1024));
	}

	LOGDEBUG(g_LogTiming, "frames: %i, wait for tasks %i, wait for read %i, no requests %i", frames, waitForTasks, isLoadingChunk, noRequests);
}

}}