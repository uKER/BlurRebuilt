#ifndef __SYSTEM_PROFILER_UTIL_H__
#define __SYSTEM_PROFILER_UTIL_H__

#include <core/utility/utility.h>
#include <core/utility/singleton.h>
#include <core/containers/string.h>
#include <btl/Containers/vector.h>
#include <System/Perf/ProfilerBackend.h>

namespace Perf
{
//Callback function to dump in debug log the profile information
extern void g_DumpPerfCallbackFunction(Perf::ProfilerCommon *pProfiler, void* pParam);



class C_EventTreeItem
{
public:
	Util::C_String						m_Name;
	C_EventTreeItem*					m_pParent;

	//Pairs of events associate to this profile item
	Btl::Vector<Perf::Event>			m_EventListPairs;

	Btl::Vector<C_EventTreeItem*>		m_ChildProfileItems;

	bool								IsRoot() const { return (m_pParent == NULL);}
	C_EventTreeItem*					GetProfileItembyName(const char* name) const
	{
		for (S32 i = 0; i < m_ChildProfileItems.Size(); i++)
			if (m_ChildProfileItems[i]->m_Name == name)
				return m_ChildProfileItems[i];
		return NULL; //No found
	}

	~C_EventTreeItem()
	{
		for (S32 i = 0; i < m_ChildProfileItems.Size(); i++)
			delete m_ChildProfileItems[i];
	}

};

struct C_EventTreeThread
{
	U32 m_ThreadID;
	C_EventTreeItem* m_pEventTree;

	C_EventTreeThread(U32 threadID)
	{
		m_ThreadID = threadID;
		m_pEventTree = NULL;
	}
	~C_EventTreeThread()
	{
		SAFE_DELETE_PTR(m_pEventTree);
	}
	const bool operator<(const C_EventTreeThread& other) const
	{
		return m_ThreadID < other.m_ThreadID;
	}

};

class C_EventTree
{
public:
	Btl::Vector<C_EventTreeThread>	m_EventTreeThread;
	U64								m_StartEventTime;
	U64								m_EndEventTime;
	
	static C_EventTreeItem* ProcessProfileBranch(C_EventTreeItem *pParent, S32& eventIndex, Perf::EventList& eventList, U32 threadID);
	static C_EventTree* CreateProfileTree(ProfilerCommon *pProfiler);
};


}
#endif
