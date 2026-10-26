#include "SystemPch.h"

#include "System/Perf/ProfilerEnable.h"

#ifdef ENABLE_MERCURY_PROFILER
#include "System/Perf/Profiler.h"
#endif
#include "System/Perf/Profiler.h"
#include "System/Perf/ProfilerBackend.h"
#include "System/Perf/ProfilerNetworkCode.h"
#include "System/Time/Time.h"
#include "System/Artery/Artery.h"
#include "System/Memory/MemoryTracker.h"
#ifdef PLATFORM_PS3
#include <sys/ppu_thread.h>
#include <sys/time_util.h>
#include <sys/dbg.h>
#endif

#include "System/Memory/MemoryTracker.h"

#include "System/Debug/Variables.h"
#include "System/Serialise.h"
#include "System/Serialise/C_MemoryStream.h"
#include <system/misc/StringUtil.h>

namespace Sys
{

// serialise a const char * into an archive
template<typename S>
void Serialise(S &s, char *pszString)
{
	int n = (int)strlen(pszString)+1;
	s << n;
	for (int i=0; i<n; i++)
		s << pszString[i];
}
template<typename S>
void Serialise(S &s, const char *pszString)
{
	int n = (int)strlen(pszString)+1;
	s << n;
	for (int i=0; i<n; i++)
		s << pszString[i];
}

}

namespace Perf
{

extern Debug::Logger &g_Log;

static ArteryConnection g_ProfilerArteryConnection;

/// static version of the function above
static void ArterySendCounterList_Static(ProfilerCommon *pProfiler, void* /**/)
{
	SelectedCounters::Instance().ArterySendCounterList(pProfiler);
}

/// clear the list of selected counters
void SelectedCounters::ClearSelectedCounters()
{
	m_SelectedCounters.Destroy();
}

/// add a selected counter to the list
void SelectedCounters::AddSelectedCounter(const char *pszName)
{
	m_SelectedCounters.PushBack();
	m_SelectedCounters.Back().sName = pszName;
	m_SelectedCounters.Back().fValue = 0;	

	Profiler::Instance().RegisterCallback(ArterySendCounterList_Static);
}

/// add counters from an artery counter list message
void SelectedCounters::AddSelectedCountersFromArteryMessage(ArteryMessageReader &msgReader)
{
	m_Mutex.Wait();

	const char *counterString(NULL);

	int nCounters = 0;
	msgReader.Read(nCounters);
	for (int i=0; i<nCounters; i++)
	{
		msgReader.Read(counterString);
		SelectedCounters::Instance().AddSelectedCounter(counterString);
	}

	m_Mutex.Release();
}

/// add a 

/// callback to collect the values of a selection of counters and then send them to the profiling client app via artery
void SelectedCounters::ArterySendCounterList(ProfilerCommon *pProfiler)
{
	m_Mutex.Wait();

	pProfiler->UnregisterCallback(ArterySendCounterList_Static);

	// walk the event list and update selected counters
	EventList eventList = pProfiler->GetEventList();
	for (int i=0; i<eventList.Size(); i++)
	{
		const Event &e = eventList[i];

		if (!PROFILER_EVENT_COUNTER(e))
			continue;

		U32 v = (U32)e.time;
		UpdateSelectedCounterList(*(float*)&v, e.name);
	}

	// check selected counters, if any of them are memory tracker ones then update those
	for (int i=0; i<m_SelectedCounters.Size(); i++)
	{
		char *pszName = m_SelectedCounters[i].sName;
		if (strstr(pszName, "MemoryTracker") == pszName)
		{
			if (pszName[13] == '.')
			{
				Memory::ContextId id = Memory::TrackNodes::Instance().FindId(pszName + 14);
				if (id == INVALID_CONTEXT_ID)
					continue;
				m_SelectedCounters[i].fValue = (float)Memory::Tracker::Instance().GetContextNode(id).Allocated() / 1024.f;
			}
			else if (pszName[13] == 0)
			{
				int n = Memory::TrackNodes::Instance().GetNumContextNodes();
				int iAllocated = 0;
				for (int id=0; id<n; id++)
					iAllocated += (int)Memory::Tracker::Instance().GetContextNode(id).AllocatedSingle();
				m_SelectedCounters[i].fValue = (float)iAllocated / 1024.f;
			}
		}
	}

	// send the selected counter list to the profiling app via artery
	Sys::C_MemoryStream memoryStream(1000);
	Sys::BinarySerialiser<Ut::NativeToBigEndian> ar(memoryStream);
	ar << "Counters" << m_SelectedCounters.Size() + 1;
	for (int i=0; i<m_SelectedCounters.Size(); i++)
		ar << m_SelectedCounters[i];

	SelectedCounter frameIdx;
	frameIdx.sName = "$_frameidx";
	frameIdx.fValue = (float)pProfiler->GetCurrentFrameIndex();
	ar << frameIdx;

	ArteryMessage msg(memoryStream.GetBufferPtr(), (int)memoryStream.GetBufferSize());
	g_ProfilerArteryConnection.Send(msg);

	ClearSelectedCounters();

	m_Mutex.Release();
}

/// update selected counter list with a particular counter value
void SelectedCounters::UpdateSelectedCounterList(float fValue, const char *pszName)
{
	for (int i=0; i<m_SelectedCounters.Size(); i++)
	{
		if (!strcmp(m_SelectedCounters[i].sName, pszName))
		{
			m_SelectedCounters[i].fValue = fValue;
			return;
		}
	}
}

/// send a list of all the game's counters to the profiling app so the user can pick the ones he wants to see
void SendListOfAllCountersToProfilingApp(ProfilerCommon *pProfiler, void* /**/)
{
	Profiler::Instance().UnregisterCallback(SendListOfAllCountersToProfilingApp);

	// send the selected counter list to the profiling app via artery
	Sys::C_MemoryStream memoryStream(1000);
	Sys::BinarySerialiser<Ut::NativeToBigEndian> ar(memoryStream);
	ar << "CounterList";

	// walk the event list and update selected counters
	EventList eventList = pProfiler->GetEventList();
	for (int i=0; i<eventList.Size(); i++)
	{
		const Event &e = eventList[i];

		if (!PROFILER_EVENT_COUNTER(e))
			continue;

		if (strlen(e.name) > 0)
			ar << e.name;
	}

	// get a list of all memory tracker nodes and send counters for those as well
	Ut::Vector<Util::C_String> list;
	Memory::Tracker::Instance().GetContextNodeList(list);
	for (int i=0; i<list.Size(); i++)
		ar << "MemoryTracker." << list[i].Chars();
	ar << "MemoryTracker"; // send the root memory tracker counter

	ar << "";

	g_ProfilerArteryConnection.Send(ArteryMessage(memoryStream.GetBufferPtr(), (int)memoryStream.GetBufferSize()));
}

#ifdef PLATFORM_PS3
/// check the difference between the top 32 bits of SPU and PPU timings and fix the SPU to match the PPU
/// this is because the only the low 32 bits of SPU decrementers match the PPU TIMEBASE
void FixupSPUTimes(EventList &ev)
{
	// find PPU start time
	U64 curTime = -1;
	U64 gpuStartTime = -1;
	for (int i=0; i<ev.Size(); i++)
	{
		U64 iTime = PROFILER_EVENT_TIME(ev[i]);
		int iCore = PROFILER_EVENT_CORE(ev[i]);
		if (iCore == 0) // GPU
		{
			if (iTime<gpuStartTime)
				gpuStartTime = iTime;
		}
		else if (iCore < 3)	// PPU
		{
			if (iTime<curTime)
				curTime = iTime;
		}
	}
	U64 startTime = curTime;
	curTime -= 0x10000000ULL;

	// fixup SPU start times
	for (int i=0; i<ev.Size(); i++)
	{
		if (PROFILER_EVENT_EXTRAINFO(ev[i]))
			continue;
		int iCore = PROFILER_EVENT_CORE(ev[i]);
		if (iCore >= 3)
		{
			U64 eventTime = PROFILER_EVENT_TIME(ev[i]);
			eventTime = eventTime & 0xffffffffULL;
			U64 longTime = eventTime | (curTime & 0xffffffff00000000ULL);
			if (eventTime < (curTime & 0xffffffffULL))
				longTime  += 0x100000000ULL;
			ev[i].time = (ev[i].time & (0xff00000000000000ULL)) | (longTime & 0xffffffffffffffULL);
		}
		else if (iCore == 0)
		{
			U64 eventTime = PROFILER_EVENT_TIME(ev[i]);
			U64 longTime = eventTime + startTime - gpuStartTime;
			ev[i].time = (ev[i].time & (0xff00000000000000ULL)) | (longTime & 0xffffffffffffffULL);
		}
	}
}
#endif

/// return a friendly name for the thread on an event
const char *ProfilerCommon::ThreadGetFriendlyName(const Event &e)
{
	static char buffer[32];
	sprintf(buffer, "%x", e.thread);
	return buffer;

	//DM_THREADINFOEX tix;
	//if (DmGetThreadInfoEx((DWORD)PROFILER_EVENT_TIME(e), &tix) == XBDM_NOERR)
	//	return (const char*)tix.ThreadNameAddress;
}

template<typename Archive>
void SerialiseProfilerCapture(ProfilerCommon *pProfiler, Archive ar, EventList *pEventList)
{
	// build a map of all the strings used
	Ut::UtMap<Util::C_String, int> stringMap;
	typedef Ut::UtMap<Util::C_String, int>::Iterator StringMapIter;
	Ut::Vector<const char *> stringList;

	// build a map for all the threads used
	Ut::UtMap<U64, int> threadMap;
	typedef Ut::UtMap<U64, int>::Iterator ThreadMapIter;
	Ut::Vector<Util::C_String> threadList;

	EventList _eventList = pProfiler->GetEventList();
	if (!pEventList)
		pEventList = &_eventList;

	EventList &eventList = *pEventList;
//	LOGINFO( g_Log, "Sending %d events.\n", eventList.Size());


	ar << "EventList";

	if (eventList.Size() > 0)
	{
#ifdef PLATFORM_PS3
		FixupSPUTimes(eventList);
#endif

		// find the event with the lowest time
		U64 iStartTime = PROFILER_EVENT_TIME(eventList[0]);
		int nRealEvents = 0;
		for (int i=0; i<eventList.Size(); i++)
		{
			if (PROFILER_EVENT_COUNTER(eventList[i]))
				continue;
			if (!PROFILER_EVENT_EXTRAINFO(eventList[i]))
			{
				U64 iTime = PROFILER_EVENT_TIME(eventList[i]);
				if (iTime < iStartTime)
					iStartTime = iTime;
			}
			nRealEvents++;
		}
		ar << nRealEvents;

		// go through each event and add it's info into the network message
		for (int i=0; i<eventList.Size(); i++)
		{
			const Event &e = eventList[i];

			if (PROFILER_EVENT_COUNTER(e))
				continue;

			if (PROFILER_EVENT_EXTRAINFO(e))
			{
				ar << 0xfffffffe;	// signal extra info is coming
				ar << e.thread;		// extra info type
				ar << (int)(e.time&0xffffffffULL);	// extra info
				ar << (int)(INTPTR)e.name;	// [Blur2PC][x64] the profiler protocol sends 32 bits					// extra info
				continue;
			}

			int iStringIdx = 0;
			// lookup the string in the string-map
			if (e.name == (char*)-1)
				iStringIdx = -1;
			else
			{
				const char *pszName = e.name;
				iStringIdx = 0;
				if (pszName)
				{
					StringMapIter iter = stringMap.Find(pszName);
					if (!PROFILER_EVENT_END(e))
					{
						if (iter == stringMap.End())
						{
							stringMap[pszName] = stringList.Size();
							iStringIdx = stringList.Size();
							stringList.PushBack(pszName);
						}
						else
							iStringIdx = iter->GetValue();
					}
				}
			}

			// lookup the thread in the thread-map
			int iThreadIdx = 0;
			{
#if defined(PLATFORM_PS3)
				int iCore = PROFILER_EVENT_CORE(e);
				if (iCore == 2)
					iCore = 1;
#else
				int iCore = 0;
#endif
				U64 iThread = (U64)e.thread | (((U64)(U32)iCore) << 32);
				ThreadMapIter iter = threadMap.Find(iThread);
				if (iter == threadMap.End())
				{
					threadMap[iThread] = threadList.Size();
					iThreadIdx = threadList.Size();
					threadList.PushBack(pProfiler->ThreadGetFriendlyName(e));
				}
				else
					iThreadIdx = iter->GetValue();
			}

			// write out the event data
			ar << iStringIdx;
#if defined(PLATFORM_PS3)
			ar << (U32) ((PROFILER_EVENT_TIME(e)-iStartTime)/80);
#else
			ar << (U32) ((PROFILER_EVENT_TIME(e)-iStartTime));
#endif
			ar << (U8)iThreadIdx << PROFILER_EVENT_CORE(e) << PROFILER_EVENT_END(e);
		}
	}
	else
		 ar<< 0;

	// write out the list of strings
	ar << stringList.Size();
	for (int i=0; i<stringList.Size(); i++)
		ar << stringList[i];

	// write out the list of core names
#if defined(PLATFORM_PS3)
	ar << 9 << "GPU" << "PPU1" << "PPU2";
	ar << "SPU0" << "SPU1" << "SPU2" << "SPU3" << "SPU4" << "SPU5";
#elif defined(PLATFORM_PC)
	ar << 1 << "CPU";
#else
	assert(0); // not implemented
#endif

	// write out the list of thread names
	ar << threadList.Size();
	for (int i=0; i<threadList.Size(); i++)
		ar << threadList[i].Chars();
}

bool g_bTraceEnabled = false;

/// send the contents of the event buffer across the network
void ArterySendEventList(ProfilerCommon *pProfiler, void* /**/)
{
	if (!g_bTraceEnabled)
	{
		g_bTraceEnabled = true;
		Perf::BeginTrace();
		return;
	}
	Perf::EndTrace();
	g_bTraceEnabled = false;
	pProfiler->UnregisterCallback(ArterySendEventList);

	ArteryConnection con = g_ProfilerArteryConnection;

	Sys::C_MemoryStream memoryStream(64*1024);
	Sys::BinarySerialiser<Ut::NativeToBigEndian> ar(memoryStream);
	SerialiseProfilerCapture(pProfiler, ar);

	ArteryMessage newMsg(memoryStream.GetBufferPtr(), (int)memoryStream.GetBufferSize());
	con.Send(newMsg);
}

static MT::Event g_ProfilerPauseEvent;

static bool gbContinuousCaptureStartFirstTime = true;
static bool gbContinuousCaptureInProgress = false;
int PadIntTo512(int i)
{
	return (i+511)&~511;
}
static U32 g_iContinousCaptureOffset = 0;

/// save each frame's capture data to harddisk
void ContinuousCaptureCallback(ProfilerCommon *pProfiler, void* /**/)
{
}

/// start continuous capture
void StartContinuousCapture()
{
}

/// stop continuous capture
void StopContinuousCapture()
{
	Profiler::Instance().UnregisterCallback(ContinuousCaptureCallback);
}

/// retrieve a particular frame from the continous capture file
void RetrieveContinuousCaptureFrame(int iRequestedFrame)
{
	Sys::I_File *pFile = Sys::GetDefaultFileSystem()->Open("perfview.bin");
	if (!pFile)
		return;
	Sys::BinaryDeserialiser<Ut::NoEndianSwap> ar(*pFile);
	int iOffset = 0;
	while (iOffset < (int)g_iContinousCaptureOffset)
	{
		int iFrame, cSize;
		ar >> iFrame >> cSize;
		if (iFrame != iRequestedFrame)
		{
			// this is the wrong frame. skip to the next frame in the file
			iOffset += PadIntTo512(cSize);
			pFile->SetPosition(iOffset);
			continue;
		}

		// this is the correct frame. Read it into a buffer and send it to the profiling app
		assert(cSize>=8);
		U32 _cSize = cSize-8;
		U8 *pBuffer = new U8[_cSize];
		ar.Read(pBuffer, _cSize);
		EventList eventList((Event*)pBuffer, _cSize/sizeof(Event));	

		ArteryConnection con = g_ProfilerArteryConnection;

		Sys::C_MemoryStream memoryStream(64*1024);
		Sys::BinarySerialiser<Ut::NativeToBigEndian> ar(memoryStream);
		SerialiseProfilerCapture(&Profiler::Instance(), ar, &eventList);

		ArteryMessage newMsg(memoryStream.GetBufferPtr(), (int)memoryStream.GetBufferSize());
		con.Send(newMsg);
		break;
	}
	delete pFile;
}

/// pause the profiler until we hit the unpause button
void ProfilerPause(ProfilerCommon *pProfiler, void* /**/)
{
	if (gbContinuousCaptureInProgress)
		StopContinuousCapture();

	pProfiler->UnregisterCallback(ProfilerPause);

	g_ProfilerPauseEvent.Wait();

	if (gbContinuousCaptureInProgress)
		StartContinuousCapture();
}

/// process incoming artery network messages
void ProcessNetworkCommunications(ArteryConnection con, void*)
{
	while (con.IsConnected())
	{
		ArteryMessage msg;

		if (con.Receive(msg))
		{
			ArteryMessageReader msgReader(msg);

			const char* msgString(NULL);

			if (!msgReader.Read(msgString))
				continue;
			
			if (!strcmp(msgString, "EventList"))
			{
				// at the end of the current frame, this callback will send the captured event-list to the analysis app
				g_ProfilerArteryConnection = con;
#ifdef ENABLE_MERCURY_PROFILER
				Profiler::Instance().RegisterCallback(ArterySendEventList);
#else
				ArteryMessageBuilder b;
				b.Write("Timing capture failed.\nThe target application does not have ENABLE_MERCURY_PROFILER defined.");
				ArteryMessage newMsg;
				if (b.Build(newMsg))
					con.Send(newMsg);
#endif
			}
			else if (!strcmp(msgString, "Counters"))
			{
				g_ProfilerArteryConnection = con;

				SelectedCounters::Instance().AddSelectedCountersFromArteryMessage(msgReader);
			}
			else if (!strcmp(msgString, "ListCounters"))
			{
				g_ProfilerArteryConnection = con;

				Profiler::Instance().RegisterCallback(SendListOfAllCountersToProfilingApp);
			}
			else if (!strcmp(msgString, "Allocations"))
			{
				Memory::Tracker::Instance().SendAllocationList(0, false, con, true);
			}
			else if (!strcmp(msgString, "ContinuousCaptureOn"))
			{
				StartContinuousCapture();
				gbContinuousCaptureInProgress = true;
			}
			else if (!strcmp(msgString, "ContinuousCaptureOff"))
			{
				gbContinuousCaptureInProgress = false;
				StopContinuousCapture();
			}
			else if (!strcmp(msgString, "Pause"))
			{
				Profiler::Instance().RegisterCallback(ProfilerPause);
			}
			else if (!strcmp(msgString, "Unpause"))
			{
				g_ProfilerPauseEvent.Signal();
			}
			else if (strstr(msgString, "RetrieveTimingCapture"))
			{
				RetrieveContinuousCaptureFrame(atoi(msgString+21));
			}
		}
	}
}

void g_CapturePerfCallbackFunction(Perf::ProfilerCommon *pProfiler, void* pParam)
{
	Util::C_String captureFileName;

	if (pParam == NULL)
	{
		captureFileName.Printf("dontship/captures/capture%s.%s.%i.perf", Time::FormatDateString(Time::GetCurrentTime()).Chars(), Time::FormatTimeString2(Time::GetCurrentTime()).Chars(), Time::GetCurrentTime()%1000);
	}
	else
	{
		//Get from pParam
		captureFileName.Printf("%s", (char*) pParam);
	}

	Sys::GetDefaultFileSystem()->CreateDirectory(Sys::ExtractDirectory(captureFileName));
	Sys::I_File* pFile = Sys::GetDefaultFileSystem()->Open(captureFileName, Sys::E_FileAccess_Write);
	if (pFile)
	{
		Sys::BinarySerialiser<Ut::NativeToLittleEndian> ar(*pFile);
		SerialiseProfilerCapture<Sys::BinarySerialiser<Ut::NativeToLittleEndian> >(pProfiler, ar);
		SAFE_DELETE_PTR(pFile);

		LOGINFO(g_Log,"Perf capture file %s", captureFileName.Chars());
	}
	else
	{
		LOGWARN(g_Log,"Unable to create perf capture file %s", captureFileName.Chars());
	}

}


/// initialise server that allows the PC profiler client to connect to this app
int InitialiseNetworkConnection()
{
#ifndef CFG_GOLD
	ArteryRegister("Profiler", ProcessNetworkCommunications, 0);
#endif
	return 0;
}

}; // namespace Perf


