#ifndef RD_RENDERING_STATISTICS_H
#define RD_RENDERING_STATISTICS_H
#ifdef _MSC_VER
#pragma once
#endif

#include <system/time/time.h>

namespace RenderingData {
namespace Target
{

class LoadingStatistics
{
public:
	static LoadingStatistics& Instance() {return s_instance;}

	void Initialise();
	void SetPreLoad(bool preLoad);
	void Read(U64 offset, U64 size);
	void Update();
	void WaitForTasks();
	void IsLoadingChunk();
	void NoRequests(); 
	float LoadingTime() const;

private:
	LoadingStatistics() {}

	struct Stats
	{
		void Initialise();

		void Read(U64 offset, U64 size);

		void Print();

		U64 totalSeekDistance;
		U64 lastOffset;
		U64 bytesRead;
		S32 nReads;
		S32 frames;
		S32 waitForTasks;
		S32 isLoadingChunk;
		S32 noRequests;
		float mbPerSecond;

		Time::C_Time startTime;
	};

	Stats g_statsPreload;
	Stats g_streaming;
	Stats g_current;
	Stats g_previous;
	bool preLoad;
	Time::C_Time m_startTime;

	static LoadingStatistics s_instance;
};

}}

#endif
