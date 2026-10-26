#include "SystemPch.h"

#include "System/perf/ProfilerUtil.h"
#include "system/Perf/ProfilerEnable.h"
#include "system/Perf/ProfilerNetworkCode.h"
#include "System/Perf/Profiler.h"
#include "System/Time/Time.h"
#include "System/Memory/MemoryTracker.h"
#include "System/Perf/ProfilerBackend.h"
#include "Btl/Containers/vector.h"
#include "Core/Maths/MathsUtil.h"
#include <algorithm>
#include "System/Serialise.h"

#ifdef PLATFORM_PS3
#include <sys/ppu_thread.h>
#include <sys/time_util.h>
#include <sys/dbg.h>
#include <cell/atomic.h>
#if CELL_SDK_VERSION < 0x270000
#include "ppu_intrinsics_gcc.h"
#else
#include "ppu_intrinsics.h"
#endif
#endif


namespace Perf
{

extern Debug::Logger &g_Log;

void DumpEventTreeItem(C_EventTreeItem* pItem, U32 level, U64 firstEventTime, U64 lastEventTime)
{
	//Calculate total
	U32 totalTime = 0;
	U32 count = 0;

	U64 bar[2] = {0,0};
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
				bar[0] = firstEventTime;
			}

			bar[1] = PROFILER_EVENT_TIME(pItem->m_EventListPairs[i]);


			#if defined(PLATFORM_PS3)
				U32 time = U32( (bar[1] - bar[0]) / 80);
			#else
				U32 time = U32( bar[1] - bar[0] );
			#endif
			totalTime += time;
			count++;

			bar[0] = 0;
			bar[1] = 0;

			
		}
	}
	if (bar[0] != 0)
	{
		bar[1] = lastEventTime;

		#if defined(PLATFORM_PS3)
			U32 time = U32( (bar[1] - bar[0]) / 80);
		#else
			U32 time = U32( bar[1] - bar[0] );
		#endif
		totalTime += time;
		count++;

		bar[0] = 0;
		bar[1] = 0;
	}

	//Dump
	Util::C_String levelSpaces('|', level);

	int n = levelSpaces.Length() + pItem->m_Name.Length();
	static int iDot = 0;
	Util::C_String sPad;
	if (iDot>0)
	{
		iDot--;
		sPad = Util::C_String(' ', Maths::Max(0, 50-n));
	}
	else
	{
		sPad = Util::C_String('.', Maths::Max(0, 50-n));
		iDot = 3;
	}

#if !CFG_CONVERTER
	LOGDEBUG(g_Log, "%s-%s %s %s - Count: %i",levelSpaces.Chars(),pItem->m_Name.Chars(), sPad.Chars(), Time::FormatProfileDurationString(totalTime).Chars(), count );
#else
	LOGDEBUG(g_Log, "%s-%s %s %s - Count: %i",levelSpaces.Chars(),pItem->m_Name.Chars(), sPad.Chars(), Time::FormatDurationString(totalTime).Chars(), count );
#endif

	//Dump childs
	for (S32 i = 0; i < pItem->m_ChildProfileItems.Size(); i++)
	{
		DumpEventTreeItem(pItem->m_ChildProfileItems[i], level + 1, firstEventTime, lastEventTime);
	}

		
}

///Dump Callback function
void g_DumpPerfCallbackFunction(Perf::ProfilerCommon *pProfiler, void* /**/)
{
	//Get Event Tree
	C_EventTree* eventTree = C_EventTree::CreateProfileTree(pProfiler);

	#if defined(PLATFORM_PS3)
		U32 time = U32( (eventTree->m_EndEventTime - eventTree->m_StartEventTime) / 80);
	#else
		U32 time = U32( eventTree->m_EndEventTime - eventTree->m_StartEventTime );
	#endif

	if (eventTree->m_EventTreeThread.Size() > 0)
	{
		//Something to dump

		//Remove format
		LOGDEBUG(g_Log, "Perf Dump:");
		LOGDEBUG(g_Log, "<tt><pre>");
		//For each threadID
		for (S32 i = 0; i < eventTree->m_EventTreeThread.Size(); i++)
		{
			//Dump Thread
			LOGDEBUG(g_Log, "Profile Thread ID: %u, Time: %s",eventTree->m_EventTreeThread[i].m_ThreadID, Time::FormatProfileDurationString(time).Chars());

			//Dump
			for (S32 j = 0; j < eventTree->m_EventTreeThread[i].m_pEventTree->m_ChildProfileItems.Size(); j++)
			{
				DumpEventTreeItem(eventTree->m_EventTreeThread[i].m_pEventTree->m_ChildProfileItems[j], 1, eventTree->m_StartEventTime, eventTree->m_EndEventTime);
			}
		}
		LOGDEBUG(g_Log, "</tt></pre>");
	}
	SAFE_DELETE_PTR(eventTree);	
}

C_EventTree* C_EventTree::CreateProfileTree( ProfilerCommon *pProfiler )
{
	C_EventTree*	pEventTree;

	pEventTree = new C_EventTree();
	
	//Get EventList
	Perf::EventList eventList = pProfiler->GetEventList();

	pEventTree->m_StartEventTime = U64(-1);
	pEventTree->m_EndEventTime = 0;


	//Calculate Thread ID list
	for (S32 i = 0; i < eventList.Size(); i++)
	{
		if (U64(eventList[i].name) == 0xFFFFFFFF)
			continue;

		U32 currentThreadID = eventList[i].thread;

		pEventTree->m_StartEventTime = Maths::Min(pEventTree->m_StartEventTime, PROFILER_EVENT_TIME(eventList[i]));
		pEventTree->m_EndEventTime = Maths::Max(pEventTree->m_EndEventTime, PROFILER_EVENT_TIME(eventList[i]));

		bool found = false;
		//Check in the list
		for (S32 j = 0; j < pEventTree->m_EventTreeThread.Size(); j++)
		{
			if (currentThreadID == pEventTree->m_EventTreeThread[j].m_ThreadID)
			{
				found = true;
				break;
			}
		}
		if (!found)
		{
			C_EventTreeThread eventTreeThread (currentThreadID);
			//Need to add
			pEventTree->m_EventTreeThread.PushBack(eventTreeThread);
		}
	}
	//Sort by threadID
	if (pEventTree->m_EventTreeThread.Size() > 0)
		std::sort(&pEventTree->m_EventTreeThread[0], &pEventTree->m_EventTreeThread[pEventTree->m_EventTreeThread.Size() - 1]);


	//Build the event tree for each thread ID
	for (S32 i = 0; i < pEventTree->m_EventTreeThread.Size(); i++)
	{
		//Create root
		C_EventTreeItem* pRoot =  new C_EventTreeItem();
		pRoot->m_Name = "Root";
		pRoot->m_pParent = NULL;

		pEventTree->m_EventTreeThread[i].m_pEventTree = pRoot;

		S32 eventIndex = 0;
		while (eventIndex < eventList.Size())
		{
			if (eventList[eventIndex].thread == pEventTree->m_EventTreeThread[i].m_ThreadID)
			{
				//Create a branch
				C_EventTreeItem* pChildProfileEvent = ProcessProfileBranch(pRoot, eventIndex, eventList, pEventTree->m_EventTreeThread[i].m_ThreadID);

				if (pChildProfileEvent)
					pRoot->m_ChildProfileItems.PushBack(pChildProfileEvent);
			}
			else
			{
				//next event
				eventIndex++;
			}
		}
	}

	
	return pEventTree;
}

C_EventTreeItem* C_EventTree::ProcessProfileBranch( C_EventTreeItem *pParent, S32& eventIndex, Perf::EventList& eventList, U32 threadID )
{
	if (U64(eventList[eventIndex].name) == 0xFFFFFFFF)
	{
		//Context switch event, only PS3	
		eventIndex++;
		return NULL;
	}

	if (eventList[eventIndex].name == NULL)
	{
		//It is an event that finish in this frame but it didn't start in this frame
		eventIndex++;
		return NULL;
	}

	//Create a profile item, check if is already in the parent
	C_EventTreeItem* pProfileItem = pParent->GetProfileItembyName(eventList[eventIndex].name);

	bool addedNeeded = false;

	if (pProfileItem == NULL)
	{
		//Create a new one
		pProfileItem = new C_EventTreeItem();
		pProfileItem->m_Name = eventList[eventIndex].name;
		
		pProfileItem->m_EventListPairs.PushBack(eventList[eventIndex]);
		addedNeeded = true;
	}
	else
	{
		//Add
		pProfileItem->m_EventListPairs.PushBack(eventList[eventIndex]);
	}
	eventIndex++;

	//Process childs
	while (eventIndex < eventList.Size())
	{
		if (eventList[eventIndex].thread == threadID)
		{
			if (eventList[eventIndex].name == NULL)
			{				
				pProfileItem->m_EventListPairs.PushBack(eventList[eventIndex]);
				break;
			}
			else
			{
				//Create a branch
				C_EventTreeItem* pChildProfileEvent = ProcessProfileBranch(pProfileItem, eventIndex, eventList, threadID);
				
				if (pChildProfileEvent)
					pProfileItem->m_ChildProfileItems.PushBack(pChildProfileEvent);
			}
			
		}
		else
			//next event
			eventIndex++;
	}
	
	//Next event
	eventIndex++;

	if (addedNeeded)
		return pProfileItem;
	else
		return NULL;
}

}; // namespace Perf

