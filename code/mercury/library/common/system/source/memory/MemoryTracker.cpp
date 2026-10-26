#include "SystemPch.h"

#if !defined( COMPILER_VC )
#include <pthread.h>
#endif

#include <algorithm>

#ifndef PLATFORM_PS3
#include <malloc.h>
#else
#include <alloca.h>
#include <sys/memory.h>
#endif

#include "btl/containers/map.h"

#include <core/threads/ThreadLocalStorage.h>
#include <core/utility/endian.h>
#include <system/artery/artery.h>
#include <system/debug/log/filesystem.h>
#include <system/debug/log/ThreadContextTemplate.h>
#include <system/debuglog.h>
#include <system/memory/BlockAllocator.h>
#include <system/memory/CallStack.h>
#include <system/memory/MemoryTracker.h>
#include <system/memory/MemoryTrackerDefines.h>
#include "System/Perf/Profiler.h"
#include <system/serialise.h>
#include <stddef.h>

#include <System/Debug/Debug.h>

#if !defined( COMPILER_VC )
#define _msize(X) malloc_usable_size(X)
#endif

#define INVALID_MEMTRACK_ADDR	0xf00baaa

namespace Debug
{
template <>
U8 InitialiseEmpty()
{
	return U8(0);
}
}

namespace Memory
{

#ifndef CFG_SPEED
TrackNodes *g_TrackNodes;
Tracker *g_Tracker;
#endif

// class used to sort the nodes
class ContextNodeNamePtr
{
public:
	const ContextNodeName	*pNodeName;
	const ContextNode		*pNode;

	bool operator < (const ContextNodeNamePtr &B) const
	{
		return strcmp(pNodeName->m_name, B.pNodeName->m_name)<0;
	}
};

static Debug::Logger &g_Log = Debug::GetLogger("debug.memtrack");

static ContextId g_memoryTrackerID = TrackNodes::Instance().GetId("debug.memtrack");
static ContextId g_memoryTrackerCallstackID = TrackNodes::Instance().GetId("debug.memtrack.callstacks");

Debug::Bool g_showTagIncrement("System.Profile.Memory.ShowTagIncrements", false, Debug::Persist());
Debug::Bool g_showTagStatsOnIncrement("System.Profile.Memory.DumpTagStatsOnIncrement", false, Debug::Persist());

BlockAllocator* g_smallBlockAllocatorPtr = NULL;

//bool Tracker::m_bDumpStatsOnExit = true;
//bool Tracker::m_bDumpBlocksOnExit = false;
//int Tracker::m_iFlags = 0;
//OutputCallBackFunction *Tracker::m_pOutputCallBackFunction = NULL;

//Tracker g_Tracker;

//::Debug::ThreadContextTemplate<ContextId> g_ThreadContext;

//bool Tracker::m_bInitialized = false;

// utility
Util::C_String MemSizeToStringK(size_t dwSize, bool bPad=true)
{
	Util::C_String szString;
	
	if (dwSize > 236*1024)
	{
		if (bPad)	szString.Printf("%7.1fM", float(dwSize)/(1024.f*1024.f));
		else		szString.Printf("%.1fM", float(dwSize)/(1024.f*1024.f));
	}
	else if (dwSize > 128)
	{
		if (bPad)	szString.Printf("%7.1fK", float(dwSize)/1024.f);
		else		szString.Printf("%.1fK", float(dwSize)/1024.f);
	}
	else
	{
		if (bPad)	szString.Printf("%7db", dwSize);
		else		szString.Printf("%db", dwSize);
	}

	return szString;
}

#define STRUCT_OFFSET(X) offsetof(AllocationNode, X)

static int giNODE_ID_SIZE						= STRUCT_OFFSET(m_pNext);
static int giENUMERATE_ALLOCATIONS_FLAG_SIZE	= STRUCT_OFFSET(m_pCallStack) - STRUCT_OFFSET(m_pNext);
static int giCALLSTACK_FLAG_SIZE				= STRUCT_OFFSET(m_pAddress) - STRUCT_OFFSET(m_pCallStack);
static int giBLOCK_LOCATIONS_FLAG_SIZE			= (int)sizeof(AllocationNode) - STRUCT_OFFSET(m_pAddress);

U32 AllocationNode::Size(int iFlags)
{
	U32 size = giNODE_ID_SIZE;

	if (iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG)
		size += giENUMERATE_ALLOCATIONS_FLAG_SIZE;

	if (iFlags & MEMORY_TRACKER_CALLSTACK_FLAG)
		size += giCALLSTACK_FLAG_SIZE;

	if (iFlags & MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG)
		size += giBLOCK_LOCATIONS_FLAG_SIZE;
	
	return size;
}

/// amount of memory allocated within this node
size_t ContextNode::AllocatedSingle() const
{
	return m_iAllocatedSingle;
}

/// amount of memory allocated within this node and all it's children
size_t ContextNode::Allocated() const
{
	return m_iAllocated;
}

/// maximum amount that was ever allocated on this node and all it's children
size_t ContextNode::AllocatedMax() const
{
	return m_iAllocatedMax;
}

/// maximum amount that was ever allocated on this node
size_t ContextNode::AllocatedMaxSingle() const
{
	return m_iAllocatedMaxSingle;
}

/// amount of wasted memory within this node
size_t ContextNode::WastedSingle() const
{
	return m_iWastedSingle;
}

/// amount of wasted memory withing this node and all it's children
size_t ContextNode::Wasted() const
{
	return m_iWasted;
}

/// number of allocations within this node
int ContextNode::AllocationsSingle() const
{
	return m_nAllocationsSingle;
}

/// number of allocations within this node and all it's children
int ContextNode::Allocations() const
{
	return m_nAllocations;
}

/// maximum amount of allocations ever within this node
int ContextNode::AllocationsMaxSingle() const
{
	return m_nAllocationsMax;
}

/// maximum amount of allocations ever within this node and all it's children
int ContextNode::AllocationsMax() const
{
	return m_nAllocationsMax;
}

void ContextNode::Add(const ContextNode& cn)
{
	m_iAllocated			+= cn.m_iAllocated;
	m_iAllocatedSingle		+= cn.m_iAllocatedSingle;
	m_iAllocatedMax			+= cn.m_iAllocatedMax;
	m_iAllocatedMaxSingle	+= cn.m_iAllocatedMaxSingle;
	m_iWasted				+= cn.m_iWasted;
	m_iWastedSingle			+= cn.m_iWastedSingle;
	m_maxAllowed			+= cn.m_maxAllowed;
	m_nAllocations			+= cn.m_nAllocations;
	m_nAllocationsSingle	+= cn.m_nAllocationsSingle;
	m_nAllocationsMax		+= cn.m_nAllocationsMax;
	m_nAllocationsMaxSingle	+= cn.m_nAllocationsMaxSingle;
}

/// constructor
TrackNodes::TrackNodes() :
	m_pTrackers(NULL),
	m_tag(0)
{
#ifndef CFG_SPEED
	g_TrackNodes = this;
#endif
	Initialize();
}

void TrackNodes::Initialize()
{
	static bool init(false);

	if (!init)
	{
		init = true;
		
		m_nNodes = 4;
		
		strcpy(m_Node[0].m_name, "unknown");
		strcpy(m_Node[1].m_name, "debug");
		strcpy(m_Node[2].m_name, "debug.memtrack");
		strcpy(m_Node[3].m_name, "debug.memtrack.callstacks");

		m_Node[2].m_parent	= 1;
		m_Node[2].m_depth	= 1;
		m_Node[3].m_parent	= 2;
		m_Node[3].m_depth	= 2;

		g_memoryTrackerID			= 2;
		g_memoryTrackerCallstackID	= 3;
	}
}

void TrackNodes::RegisterTracker(Tracker *tracker)
{
	tracker->m_pNext = m_pTrackers;
	m_pTrackers = tracker;

	// We let the other trackers know about the context node changes (new parent link, etc)
	{
		int parentInfo[Enum_Max_ContextNodes];

		for (int iCount=0; iCount < m_nNodes; iCount++)
			parentInfo[iCount] = m_Node[iCount].m_parent;
		
		tracker->UpdateContext(m_nNodes, parentInfo);
	}
}

void TrackNodes::UpdateParentPointers(ContextNode* pNodes, int numNodes)
{
	BIZ_ASSERT(m_nNodes == numNodes);

	for (int iCount=0; iCount < m_nNodes; iCount++)
	{
		if (m_Node[iCount].m_parent != 0)
		{
			pNodes[iCount].m_pParent = pNodes + m_Node[iCount].m_parent;
		}
	}
}

/// constructor
Tracker::Tracker() : 
	m_bDumpStatsOnExit(true),
	m_bDumpBlocksOnExit(false),
	m_iFlags(0),
	m_allocationsSinceLastUpdate(0),
	m_pOutputCallBackFunction(NULL),
	m_bInitialized(false),
	m_pDumpStatsBool(NULL),
	m_pDumpTagNode(NULL)
{
	TrackNodes::Instance().Initialize();
	//Initialize(0);
}

void Tracker::Initialize(const Util::C_String& name, Tracker& clone, size_t bufferSize)
{
	if (clone.Initialised())
	{
		Initialize(name, clone.m_callStackSkip / sizeof(void*), clone.m_iFlags, false);
	}

	m_bufferSize = bufferSize;
}

void Tracker::Initialize(const Util::C_String& name, int callStackSkip, unsigned int flags, bool mainTracker)
{
#ifndef CFG_SPEED
	if (mainTracker)
		g_Tracker = this;
#endif

	m_name			= name;

	m_iFlags		= flags;
	m_callStackSkip = (callStackSkip != -1) ? (callStackSkip * sizeof(void*)) : 0;
	m_mainTracker	= mainTracker;

	m_trackerStructSize			= AllocationNode::Size(m_iFlags);
	m_trackerAlignedStructSize	= (m_trackerStructSize + 15) & ~15;

	if (m_bInitialized)
		return;

	TrackNodes::Instance().RegisterTracker(this);

	if (!mainTracker)
		Tracker::Instance().AddSubSystem(this);

	m_root.m_pNext = NULL;

	m_bDumpStatsOnExit	= true;
	m_bDumpBlocksOnExit	= false;
	m_bInitialized		= true;

	Util::C_String tn = "System.Profile.Memory." + name + ".";

	m_pDumpStatsBool = new Debug::Bool((tn + "DumpStats").Chars(), false, Debug::Dynamic());
	m_pDumpTagNode = new Debug::Bool((tn + "DumpTagAndNode").Chars(), false, Debug::Dynamic());
}

void Tracker::AddSubSystem(Tracker* tracker)
{
	m_Lock.Enter();
	m_subTrackers.PushBack(tracker);
	m_Lock.Exit();
}

void Tracker::RemoveSubSystem(Tracker* tracker)
{
	m_Lock.Enter();
	for (int iSubSys=0; iSubSys < m_subTrackers.Size(); iSubSys++)
	{
		if (m_subTrackers[iSubSys] == tracker)
		{
			m_subTrackers.EraseByIndex(iSubSys);
			break;
		}
	}
	m_Lock.Exit();
}

// TODO: This need's to only be updating stack usage for the main tracker
void Tracker::Update()
{
#ifndef CFG_GOLD
	char buffer[64];

	if (m_mainTracker)
	{
		unsigned int totalAllocsPerFrame = m_allocationsSinceLastUpdate;

		for (int iSubTrack=0; iSubTrack < m_subTrackers.Size(); iSubTrack++)
		{
			totalAllocsPerFrame += m_subTrackers[iSubTrack]->m_allocationsSinceLastUpdate;
		}
		
		strcpy_s(buffer, lengthof(buffer), "MAllocsPerFrame - ");
		strcat_s(buffer, lengthof(buffer), "Total");
		Perf::AddNamedCounter((F32)totalAllocsPerFrame, buffer);
	}

	strcpy_s(buffer, lengthof(buffer), "MAllocsPerFrame - ");
	strcat_s(buffer, lengthof(buffer), m_name.Chars());
	Perf::AddNamedCounter((F32)m_allocationsSinceLastUpdate, buffer);
	m_allocationsSinceLastUpdate = 0;

	if (m_mainTracker)
	{

#ifdef PLATFORM_PS3
		{
		}
#endif
	}

	if (m_pDumpStatsBool && m_pDumpStatsBool->Value() == true)
	{
		DumpNodeStats(false, false);
		DumpTagStats(0, NULL, false);
		m_pDumpStatsBool->SetValue(false);
	}

	if (m_pDumpTagNode && m_pDumpTagNode->Value() == true)
	{
		DebugTagAndNode(false);
		m_pDumpTagNode->SetValue(false);
	}

	for (int iSubTrack=0; iSubTrack < m_subTrackers.Size(); iSubTrack++)
	{
		m_subTrackers[iSubTrack]->Update();
	}
#endif
}

/// destructor
Tracker::~Tracker()
{
	delete m_pDumpStatsBool;
	delete m_pDumpTagNode;

	m_bInitialized = false;

	if (m_bDumpStatsOnExit)
		DumpNodeStats(false, false);
	if (m_bDumpBlocksOnExit)
		DumpAllocatedBlocks();		
}

/// Push context item onto the stack
void TrackNodes::Push(const ContextId item)
{
	return ::Debug::ThreadContextTemplate<ContextId>::Instance().Push(item);
}

#ifdef CFG_DEBUG

/// Pop context item from the stack
void TrackNodes::Pop(const ContextId &item)
{
	const ::Memory::ContextId& currentOnStack = ::Debug::ThreadContextTemplate<ContextId>::Instance().GetContext();

	if (item != currentOnStack)
	{
		const ::Memory::ContextNodeName& popName = ::Memory::TrackNodes::Instance().GetContextNode(item);
		const ::Memory::ContextNodeName& cnn = ::Memory::TrackNodes::Instance().GetContextNode(currentOnStack);
		
		LOGFATAL(g_Log, "TrackNodes::Pop - Went to pop '%s' - Found '%s' instead.", popName.m_name, cnn.m_name);
	}

	::Debug::ThreadContextTemplate<ContextId>::Instance().Pop();
}

#else

/// Pop context item from the stack and check it's what we thought
void TrackNodes::Pop()
{
	::Debug::ThreadContextTemplate<ContextId>::Instance().Pop();
}

#endif

/// Get current thread context item
const ContextId &TrackNodes::GetContext()
{
	//return g_ThreadContext.GetContext();
	return ::Debug::ThreadContextTemplate<ContextId>::Instance().GetContext();
};

ContextId TrackNodes::GetParentId(ContextId id)
{
	if (m_Node[id].m_parent)
		return ContextId(m_Node[id].m_parent);

	return ContextId(-1);
}

/// retrieve an Id for a given context hierarchy string
ContextId TrackNodes::GetId(const Util::C_String &string)
{
	const Util::C_String s = string.Lower();

	ContextId rtn = GetNode(s);

	return rtn;
}

/// retrieve an Id for the current context hierarchy with this sub-string applied
ContextId TrackNodes::GetIdSub(const Util::C_String &string)
{
	const Util::C_String s = string.Lower();

	const ::Memory::ContextId&			currentOnStack = ::Debug::ThreadContextTemplate<ContextId>::Instance().GetContext();
	const ::Memory::ContextNodeName&	cnn = ::Memory::TrackNodes::Instance().GetContextNode(currentOnStack);

	Util::C_String fullName(Util::C_String::Format("%s.%s", cnn.m_name, s.Chars()));
		
	ContextId rtn = GetNode(fullName);

	return rtn;
}

#ifdef CFG_DEBUG

/// we use this to check that the current tag ends with this sub string
ContextId TrackNodes::GetCurrentIdWithSub(const Util::C_String &string)
{
	const Util::C_String s = string.Lower();

	const ::Memory::ContextId&			currentOnStack = ::Debug::ThreadContextTemplate<ContextId>::Instance().GetContext();
	const ::Memory::ContextNodeName&	cnn = ::Memory::TrackNodes::Instance().GetContextNode(currentOnStack);

	int len = strlen(cnn.m_name);

	BIZ_ASSERT(len > s.Length()+1);
	BIZ_ASSERT(cnn.m_name[len - s.Length() - 1] == '.');
	BIZ_ASSERT(strcmp(cnn.m_name + len - string.Length(), s.Chars()) == 0);

	return currentOnStack;
}

#endif

/// retrieve an Id for a given context hierarchy string
ContextId TrackNodes::GetIdDynamic(const char* string, ...)
{
	char expandedString[100];
	va_list arglist;
	
	va_start(arglist, string);
#if defined(COMPILER_VC)
	_vsnprintf_s(expandedString, lengthof(expandedString), _TRUNCATE, string, arglist);
#else
	vsprintf_s(expandedString, lengthof(expandedString), string, arglist);
#endif
	va_end(arglist);

	const Util::C_String s = Util::C_String(expandedString).Lower();

	ContextId rtn = GetNode(s);

	return rtn;
}

/// constructor
ContextNodeName::ContextNodeName() :
	m_parent(0),
	m_depth(0)
{
}

/// constructor
ContextNode::ContextNode() :
	m_iAllocated(0), m_iAllocatedSingle(0),
	m_iAllocatedMax(0), m_iAllocatedMaxSingle(0),
	m_iWasted(0), m_iWastedSingle(0),
	m_nAllocations(0), m_nAllocationsSingle(0),
	m_nAllocationsMax(0), m_nAllocationsMaxSingle(0),
	m_maxAllowed(0),
	m_pParent(NULL)
{
}

/// Change the amount of allocated/wasted memory of this node and it's parents
void ContextNode::UpdateMemoryOnNode(int iAllocated, int iWasted, int nAllocations, bool bInherited)
{
	//Assert fails in Aligned memory.
	//assert((iAllocated > 0) || (m_iAllocated >= size_t(-iAllocated)));
	BIZ_ASSERT_MSG((iWasted > 0) || (m_iWasted >= size_t(-iWasted)), "iAllocated(%d) m_iAllocated(%d) iWasted(%d) m_iWasted(%d) nAllocations(%d) m_nAllocations(%d)",
		iAllocated, m_iAllocated, iWasted, m_iWasted, nAllocations, m_nAllocations);
	BIZ_ASSERT_MSG((nAllocations > 0) || (m_nAllocations >= -nAllocations), "iAllocated(%d) m_iAllocated(%d) iWasted(%d) m_iWasted(%d) nAllocations(%d) m_nAllocations(%d)",
		iAllocated, m_iAllocated, iWasted, m_iWasted, nAllocations, m_nAllocations);

	m_iAllocated += iAllocated;
	m_iWasted += iWasted;
	m_nAllocations += nAllocations;
	if (!bInherited)
	{
		m_iAllocatedSingle += iAllocated;
		m_iWastedSingle += iWasted;
		m_nAllocationsSingle += nAllocations;
	}

	if (AllocatedSingle() > m_iAllocatedMaxSingle)
		m_iAllocatedMaxSingle = AllocatedSingle();
	if (Allocated() > m_iAllocatedMax)
		m_iAllocatedMax = Allocated();
	if (AllocationsSingle() > m_nAllocationsMaxSingle)
		m_nAllocationsMaxSingle = AllocationsSingle();
	if (Allocations() > m_nAllocationsMax)
		m_nAllocationsMax = Allocations();

	if (m_pParent)
		m_pParent->UpdateMemoryOnNode(iAllocated, iWasted, nAllocations, true);
}

// generate parent name from a given context name
Util::C_String TrackNodes::GenerateParentName(const Util::C_String &name)
{
	S32 i = name.RevSearch('.');
	if (i == -1)
		return Util::C_String("");
	
	return name(0,i);
}

/// find an existing node index, given a string
/// retrurns -1 if the node was not found
ContextId TrackNodes::FindId(const Util::C_String &string)
{
	m_Lock.Enter();

	for (int i=0; i < Enum_Max_ContextNodes; i++)
	{
		if (!stricmp(m_Node[i].m_name, string))
		{
			m_Lock.Exit();
			return i;
		}
	}

	m_Lock.Exit();
	return INVALID_CONTEXT_ID;
}


/// find an existing node index, given a string
/// if the node is not found, create it
/// returns INVALID_CONTEXT_ID if the node array is full
ContextId TrackNodes::GetNode(const Util::C_String &string, bool bMutex)
{
	for (int i=0; i < Enum_Max_ContextNodes; i++)
	{
		if (!stricmp(m_Node[i].m_name, string))
			return i;
	}

	if (bMutex)
		m_Lock.Enter();

	// Ok... we've gone to the effort of locking it, let's check again in case someone on a different thread sneakily added it while we waited
	for (int i=0; i < Enum_Max_ContextNodes; i++)
	{
		if (!stricmp(m_Node[i].m_name, string))
		{
			if (bMutex)
				m_Lock.Exit();
			return i;
		}
	}

	// not found, add it
	if (m_nNodes >= Enum_Max_ContextNodes)
	{
		if (bMutex)
			m_Lock.Exit();
		BIZ_WARNING_FAIL("TrackNodes::GetNode - We've run out of context nodes");
		return INVALID_CONTEXT_ID;	// error, too many nodes
	}

	// create new node
	strcpy_s(m_Node[m_nNodes].m_name, MEMORY_TRACKER_MAX_NODE_NAME_SIZE, string.Chars());
	ContextId rtn = (ContextId)m_nNodes;
	m_nNodes++;
	// generate parent string
	Util::C_String sParent = GenerateParentName(string);
	// get parent pointer
	if (sParent.Length()>0)
	{
		ContextId parentidx = GetNode(sParent, false);
		
		if (parentidx != INVALID_CONTEXT_ID)
		{
			m_Node[rtn].m_parent = parentidx;
			m_Node[rtn].m_depth = m_Node[parentidx].m_depth + 1;

			// We let the other trackers know about the context node changes (new parent link, etc)
			{
				int parentInfo[Enum_Max_ContextNodes];

				for (int iCount=0; iCount < m_nNodes; iCount++)
					parentInfo[iCount] = m_Node[iCount].m_parent;
				
				for (Tracker *pTracker=m_pTrackers; pTracker; pTracker=pTracker->m_pNext)
				{
					pTracker->UpdateContext(m_nNodes, parentInfo);
				}
			}
		}
	}

	// Now we sort the nodes and store down the hierachy for quick traversal later
	{
		// collect pointers to nodes
		ContextNodeNamePtr *pList = (ContextNodeNamePtr*) malloc(sizeof(ContextNodeNamePtr) * m_nNodes);
		BIZ_ASSERT(pList);
		
		int i;
		for (i=0; i < m_nNodes; i++)
		{
			pList[i].pNodeName	= &m_Node[i];
			pList[i].pNode		= NULL;
		}

		// Sort nodes
		std::sort(&pList[0], &pList[m_nNodes]);

		m_sortedFirstNode = (U8)(pList[0].pNodeName - m_Node);

		for (i=0; i < m_nNodes-1; i++)
		{
			const_cast<ContextNodeName*>(pList[i].pNodeName)->m_sibling = (U8)(pList[i+1].pNodeName - m_Node);
		}
		const_cast<ContextNodeName*>(pList[i].pNodeName)->m_sibling = i;

		free(pList);
	}

	if (bMutex)
		m_Lock.Exit();

	return rtn;
}

#define POINTER_OFFSET(ADDR, X) ((AllocationNode*)(U64)((((U64)(ADDR))+(U64)((int)(X)))))

void Tracker::TrackAllocation(ContextId id, size_t iSize, size_t iWasted)
{
	UpdateMemory(id, int(iSize), int(iWasted), 1, kMemType_System);
}

void Tracker::TrackDeallocation(ContextId id, size_t iSize, size_t iWasted)
{
	UpdateMemory(id, -int(iSize), -int(iWasted), -1, kMemType_System);
}

void Tracker::UpdateMemory(ContextId id, int iAllocated, int iWasted, int nAllocations, E_MEMORY_TYPES memoryType)
{
	m_Lock.Enter();
	{
		// recursively add the memory size to the context node and it's parents
		m_Node[id].UpdateMemoryOnNode(iAllocated, int(iWasted), nAllocations);

		if (memoryType < kMemType_Physical)
			m_systemMemoryNode.UpdateMemoryOnNode(iAllocated, iWasted, nAllocations);
		else
			m_physicalMemoryNode.UpdateMemoryOnNode(iAllocated, iWasted, nAllocations);
	}
	m_Lock.Exit();
}

void Tracker::TrackHeapAllocation(void *pAddress, size_t iSize, E_MEMORY_TYPES memoryType)
{
	// get current context node
	ContextId id = TrackNodes::Instance().GetContext();

	SubHeapTrack sht;
	sht.m_size		= iSize;
	sht.m_iTag		= U8(TrackNodes::Instance().Tag());
	sht.m_type		= memoryType;
	sht.m_contextId	= id;
	sht.pAddress	= pAddress;

	if (m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG)
	{
		// store callstack
		void *pCallStack[MEMORY_TRACKER_MAX_CALLSTACK_SIZE];

		int callstackSize = CaptureCallStack(pCallStack, MEMORY_TRACKER_MAX_CALLSTACK_SIZE);
			
		if (callstackSize > 0)
		{
			// How much to trim off the callstack to allow for the function call level getting here
			int callstackOverhead = 0;

			callstackSize -= m_callStackSkip + callstackOverhead;

			sht.m_pCallStack = BizNonTrackedMalloc(callstackSize);
			size_t iActualSize = BizNonTrackedSize(sht.m_pCallStack);

			TrackAllocation(g_memoryTrackerCallstackID, iActualSize, 0);

			memcpy(sht.m_pCallStack, ((U8*)pCallStack) + m_callStackSkip + callstackOverhead, callstackSize);
		}
		else
			sht.m_pCallStack = NULL;
	}
	else
		sht.m_pCallStack = NULL;

	m_Lock.Enter();
	m_subTrackNodes.PushBack(sht);
	m_Lock.Exit();
}

void Tracker::TrackHeapDeallocation(void *pAddress, size_t iSize, E_MEMORY_TYPES memoryType)
{
	m_Lock.Enter();

	for (int iSubHeap=0; iSubHeap < m_subTrackNodes.Size(); iSubHeap++)
	{
		SubHeapTrack& sht = m_subTrackNodes[iSubHeap];

		if (sht.pAddress == pAddress)
		{
			if (m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG)
			{
				// free callstack
				if (sht.m_pCallStack)
				{
#ifdef CFG_DEBUG
					BIZ_ASSERT(sht.m_pCallStack != (void*)INVALID_MEMTRACK_ADDR);
#endif

					const size_t callstackSize = BizNonTrackedSize(sht.m_pCallStack);

					TrackDeallocation(g_memoryTrackerCallstackID, callstackSize, 0);

					BizNonTrackedFree(sht.m_pCallStack);
				}
			}

			m_subTrackNodes.EraseByIndex(iSubHeap);
			break;
		}
	}

	m_Lock.Exit();
}

/// register an allocation with the tracker
/// pointer to an AllocationNode structure is to be supplied by the caller
//lint -save -e645
void Tracker::TrackAllocation(void *pAddress, size_t iOrigSize, size_t iAllocSize, E_MEMORY_TYPES memoryType, AllocationNode *pNode, int callstackOverhead)
{
#ifndef CFG_GOLD
	m_allocationsSinceLastUpdate++;

	if (!m_bInitialized)
	{
		pNode->m_contextId = ContextId((U32)-1);
		if (m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG)
		{
			pNode->m_pNext = pNode;
			pNode->m_pPrev = pNode;
		}
		else
			pNode = POINTER_OFFSET(pNode, -giENUMERATE_ALLOCATIONS_FLAG_SIZE);

		if (m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG)
		{
			pNode->m_pCallStack = NULL;
		}
		return;
	}

	BIZ_ASSERT(g_memoryTrackerID != 0);
	TrackAllocation(g_memoryTrackerID, m_trackerStructSize, 0);

	// get current context node
	ContextId id = TrackNodes::Instance().GetContext();

#ifdef CFG_DEBUG
	const ContextNodeName&	cnn	= TrackNodes::Instance().GetContextNode(id);
#endif
	ContextNode&			cn	= m_Node[id];

	// recursively add the memory size to the context node and it's parents
	size_t iWasted = iAllocSize - iOrigSize;

	UpdateMemory(id, int(iAllocSize), int(iWasted), 1, memoryType);

	// Allow the programs to pass in a slightly larger buffer and store down the true wasted size
	if ((memoryType != (U32)kMemType_System) && (_msize(pNode) == (U32)m_trackerStructSize+4))
	{
		*((U32*)(((U8*)pNode)+m_trackerStructSize)) = (U32)iWasted;
	}

	// store size + context
	pNode->m_iWaste = U16(iWasted);
	pNode->m_iTag = U8(TrackNodes::Instance().Tag());
	pNode->m_type = memoryType;
	pNode->m_contextId = id;

	if (m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG)
	{
		m_Lock.Enter();

		// add to linked list of allocation nodes
		pNode->m_pPrev = &m_root;
		pNode->m_pNext = m_root.m_pNext;
		if (pNode->m_pNext)
			pNode->m_pNext->m_pPrev = pNode;
		m_root.m_pNext = pNode;

		m_Lock.Exit();
	}
	else
	{
		pNode = POINTER_OFFSET(pNode, -giENUMERATE_ALLOCATIONS_FLAG_SIZE);
	}

	if (m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG)
	{
		// store callstack
		void *pCallStack[MEMORY_TRACKER_MAX_CALLSTACK_SIZE];

		int callstackSize = CaptureCallStack(pCallStack, MEMORY_TRACKER_MAX_CALLSTACK_SIZE);
			
		if (callstackSize > 0)
		{
			callstackSize -= m_callStackSkip + callstackOverhead;

			pNode->m_pCallStack = BizNonTrackedMalloc(callstackSize);
			size_t iActualSize = BizNonTrackedSize(pNode->m_pCallStack);

			TrackAllocation(g_memoryTrackerCallstackID, iActualSize, 0);

			memcpy(pNode->m_pCallStack, ((U8*)pCallStack) + m_callStackSkip + callstackOverhead, callstackSize);
		}
		else
			pNode->m_pCallStack = NULL;
	}
	else
		pNode = POINTER_OFFSET(pNode, -giCALLSTACK_FLAG_SIZE);

	if (m_iFlags & MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG)
	{
		pNode->m_pAddress = pAddress;
		pNode->m_cSize = U32(iAllocSize);
	}
	else
		pNode = POINTER_OFFSET(pNode, -giBLOCK_LOCATIONS_FLAG_SIZE);
#endif
}
//lint -restore

void Tracker::RemoveAllocationFromStatsAndList(int iAllocSize, E_MEMORY_TYPES memoryType, AllocationNode *pNode)
{
#ifndef CFG_GOLD
	if (!m_bInitialized)
		return;

	TrackDeallocation(g_memoryTrackerID, m_trackerStructSize, 0);

    ContextId id = pNode->m_contextId;
	if (id == ContextId((U32)-1))
		return;

	if (id == ContextId((U32)-2))
	{
		LOGFATAL( g_Log, "Memory address was already freed near %x\n", pNode);
	}

#ifdef CFG_DEBUG
	const ContextNodeName&	cnn	= TrackNodes::Instance().GetContextNode(id);
#endif
	ContextNode&			cn	= m_Node[id];

	// Allow the programs to pass in a slightly larger buffer and store down the true wasted size
	if ((memoryType != (U32)kMemType_System) && (_msize(pNode) == (U32)m_trackerStructSize+4))
	{
		// recursively add the memory size to the context node and it's parents
		UpdateMemory(id, -iAllocSize, -*((S32*)(((U8*)pNode)+m_trackerStructSize)), -1, memoryType);
	}
	else
	{
		// recursively add the memory size to the context node and it's parents
		UpdateMemory(id, -iAllocSize, -pNode->m_iWaste, -1, memoryType);
	}

	if (m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG)
	{
		m_Lock.Enter();

		// remove from linked list of allocation nodes
		if (pNode->m_pPrev)
		{
			BIZ_ASSERT(pNode->m_pPrev != (Memory::AllocationNode*)INVALID_MEMTRACK_ADDR);
			pNode->m_pPrev->m_pNext = pNode->m_pNext;
			
		}
		if (pNode->m_pNext)
		{
			BIZ_ASSERT(pNode->m_pNext != (Memory::AllocationNode*)INVALID_MEMTRACK_ADDR);
			pNode->m_pNext->m_pPrev = pNode->m_pPrev;
		}

#ifdef CFG_DEBUG
		pNode->m_pPrev = (Memory::AllocationNode*)INVALID_MEMTRACK_ADDR;
		pNode->m_pNext = (Memory::AllocationNode*)INVALID_MEMTRACK_ADDR;
#endif

		m_Lock.Exit();
	}
	else
		pNode = POINTER_OFFSET(pNode, -giENUMERATE_ALLOCATIONS_FLAG_SIZE);

#endif
}

/// register a deallocation with the tracker
/// after this call, the supplied AllocationNode structure can be freed
void Tracker::TrackDeallocation(int iAllocSize, E_MEMORY_TYPES memoryType, AllocationNode *pNode)
{
#ifndef CFG_GOLD
	if (!m_bInitialized)
		return;

	RemoveAllocationFromStatsAndList(iAllocSize, memoryType, pNode);

	pNode->m_contextId = -2;

	if ((m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG) == 0)
		pNode = POINTER_OFFSET(pNode, -giENUMERATE_ALLOCATIONS_FLAG_SIZE);

	if (m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG)
	{
		// free callstack
		if (pNode->m_pCallStack)
		{
#ifdef CFG_DEBUG
			BIZ_ASSERT(pNode->m_pCallStack != (void*)INVALID_MEMTRACK_ADDR);
#endif

			const size_t callstackSize = BizNonTrackedSize(pNode->m_pCallStack);

			TrackDeallocation(g_memoryTrackerCallstackID, callstackSize, 0);

			BizNonTrackedFree(pNode->m_pCallStack);

#ifdef CFG_DEBUG
			pNode->m_pCallStack = (void*)INVALID_MEMTRACK_ADDR;
#endif
		}
	}
#endif
}

void Tracker::CompressMemoryUsage()
{
	m_Lock.Enter();
//	m_hashVirPhysical.TrimCapacity();
	m_Lock.Exit();
}

bool Tracker::RememberAllocNodeForVirPhys(AllocationNode *pNode, void *ptr)
{
	MEMORY_TRACKER_PUSH("debug.memtrack.virphys_hash");

	m_Lock.Enter();
	HashVirPhysicalMap::InsertionReturnType result = m_hashVirPhysical.Insert(ptr);

	MEMORY_TRACKER_POP("debug.memtrack.virphys_hash");

	BIZ_ASSERT(result.inserted == true);

	if (result.inserted)
		result.iterator->GetValue() = pNode;
	
	m_Lock.Exit();

	return result.inserted;
}

AllocationNode* Tracker::ForgetAllocNodeForVirPhys(void *ptr)
{
	Memory::AllocationNode* allocNode;

	m_Lock.Enter();

	HashVirPhysicalMap::Iterator result = m_hashVirPhysical.Find(ptr);

	if (result != m_hashVirPhysical.End())
	{
		allocNode = result->GetValue();

		m_hashVirPhysical.Erase(result);
	}
	else
	{
		allocNode = NULL;
		BIZ_ASSERT_FAIL("Failed to find tracked allocation to remove");
	}

	m_Lock.Exit();

	return allocNode;
}

/// get first allocation node (from a linked list)
AllocationNode *Tracker::GetFirstAllocationNode() const
{
	return m_root.m_pNext;
}

void Tracker::UpdateContext(unsigned int nodeCount, int* parentInfo)
{
	for (unsigned int iCount=0; iCount < nodeCount; iCount++)
	{
		if (parentInfo[iCount])
			m_Node[iCount].m_pParent = &m_Node[parentInfo[iCount]];
	}
}

/// dump information about all allocated blocks to the debug logging system
void Tracker::DumpAllocatedBlocks(int iTag)
{
#ifndef CFG_GOLD
	if (!(m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG))
		return;

	for (AllocationNode *pNode = Tracker::GetFirstAllocationNode(); pNode; (pNode=pNode->m_pNext))
	{
		if (iTag != -1 && pNode->m_iTag != iTag)
			continue;

		const ContextNodeName&	contextName	= TrackNodes::Instance().GetContextNode(pNode->m_contextId);
		const ContextNode&		context		= GetContextNode(pNode->m_contextId);

		Output( "-- allocation\n");
		Output( "context: %s (%d)\n", contextName.m_name, U32(pNode->m_contextId));

		if (m_iFlags & MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG)
		{
			AllocationNode *p = pNode;
			if (!(m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG))
				p = POINTER_OFFSET(pNode, -giCALLSTACK_FLAG_SIZE);

			Output( "address: %x\nsize:    %d\n", (int)(U64)p->m_pAddress, p->m_cSize);
		}

		if (m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG)
		{
			const size_t callStackSize = (pNode->m_pCallStack) ? (_msize(pNode->m_pCallStack) / sizeof(void*)) : 0;

#ifndef PLATFORM_PC
			char concatbuffer[1024];
			char tmpbuffer[1024];

			sprintf(concatbuffer, "stack: (%d) ", callStackSize);

			int *pCallStack = (int*)pNode->m_pCallStack;

			for (size_t i=0; i < callStackSize; i++)
			{
				sprintf(tmpbuffer, "%08x ", pCallStack[i]);
				strcat(concatbuffer, tmpbuffer);
			}
			strcat(concatbuffer, "\n");
			Output( "%s", concatbuffer);
#else
#ifndef WIN64
			int *pCallStack = (int*)pNode->m_pCallStack;

			Output( "callstack:\n");
			for (size_t i=0; i < callStackSize; i++)
			{
				char buffer[1024];
				Memory::GetAddressInfo((void*)pCallStack[i], buffer);
				Output( "%s\n", buffer);
			}
#endif
#endif
		}
	}
#endif
}

/*
	Memory leak detection code
*/
void Tracker::UpdateTagUsageFromAllocs(U32 exclusionCount, U32* exclusionList, TagUsageInfo* tagUsage) const
{
	const AllocationNode *nodeIterator = GetFirstAllocationNode();
	const AllocationNode *previousNode = nodeIterator->m_pPrev;

	for (; nodeIterator; previousNode=nodeIterator, (nodeIterator=nodeIterator->m_pNext))
	{
		BIZ_ASSERT(nodeIterator->m_pPrev == previousNode);

		const AllocationNode *pNode = nodeIterator;

		const U32 iTag = pNode->m_iTag;

		if (exclusionCount > 0)
		{
			bool match(false);

			for (U32 iCount=0; iCount < exclusionCount && !match; iCount++)
			{
				ContextId id = pNode->m_contextId;

				while (id != 255)
				{
					if (exclusionList[iCount] == id)
					{
						match = true;
						break;
					}

					id = TrackNodes::Instance().GetParentId(id);
				}
			}

			if (match)
				continue;
		}

		// We need to store these here as we'll be moving the pointer around in a bit to much hilarity trying to access earlier member variables
		const bool isPhysical = (pNode->m_type >= kMemType_Physical);
		unsigned int waste;

		// Need to handle the case where we store down extra space for wastage (to handle big amounts of waste)
		if ((nodeIterator->m_type != (U32)kMemType_System) && (_msize((void*)nodeIterator) == (U32)m_trackerStructSize+4))
			waste = *((S32*)(((U8*)nodeIterator)+m_trackerStructSize));
		else
			waste = nodeIterator->m_iWaste;

		if ((m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG) == 0)
			pNode = POINTER_OFFSET(pNode, -giENUMERATE_ALLOCATIONS_FLAG_SIZE);

		if ((m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG) == 0)
			pNode = POINTER_OFFSET(pNode, -giCALLSTACK_FLAG_SIZE);

		if (m_iFlags & MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG)
		{
			if (!isPhysical)
				tagUsage[iTag].systemSize += pNode->m_cSize + waste + m_trackerStructSize;
			else
				tagUsage[iTag].physicalSize += pNode->m_cSize + waste + m_trackerStructSize;
		}

		tagUsage[iTag].numAllocs++;
	}
}

int Tracker::CheckForMemoryLeaks(U32 exclusionCount, U32* exclusionList, MemoryLeakInfo& info)
{
#ifndef CFG_GOLD
	memset(&info, 0, sizeof(info));

	if (TrackNodes::Instance().Tag() < 8)
	{
		return -2;	// Not enough iterations to be able to work properly
	}

	if (!(m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG))
		return -1;

	m_Lock.Enter();


	const unsigned int currentTag = TrackNodes::Instance().Tag();

	TagUsageInfo* tagUsage = (TagUsageInfo*) malloc(sizeof(TagUsageInfo) * (currentTag+1));

	if (!tagUsage)
	{
		m_Lock.Exit();
		return -1;
	}

	memset(tagUsage, 0, sizeof(TagUsageInfo) * (currentTag+1));

	// Main heap
	{
		UpdateTagUsageFromAllocs(exclusionCount, exclusionList, tagUsage);

		for (int iHeap=0; iHeap < m_subTrackers.Size(); iHeap++)
		{
			m_subTrackers[iHeap]->UpdateTagUsageFromAllocs(exclusionCount, exclusionList, tagUsage);
		}
	}

	Output( "\n");
	Output( "-- memory tracker tag stats --      system     phys       total     allocs\n");

	size_t totalSystem(0), totalPhysical(0), totalAllocs(0);
	for (unsigned int i=0; i <= currentTag; i++)
	{
		totalSystem += tagUsage[i].systemSize;
		totalPhysical += tagUsage[i].physicalSize;
		totalAllocs += tagUsage[i].numAllocs;

		if ((i > 0) && (i % 2 == 0))
			Output( "---------------------------------------------------------------------------\n");

		TrackNodes::eTagType tt = TrackNodes::Instance().TagType(i);

		Output("tag %3d :                         %6s %6s    %6s      %5d    %s\n", i,
			MemSizeToStringK(tagUsage[i].systemSize).Chars(),
			MemSizeToStringK(tagUsage[i].physicalSize).Chars(), 
			MemSizeToStringK(tagUsage[i].systemSize + tagUsage[i].physicalSize).Chars(), 
			tagUsage[i].numAllocs,
			(tt == 0) ? "Frontend" : (tt == 1) ? "Game" : "Unknown"
			);
	}

	size_t heapSys(0), heapPhy(0);
	for (int iHeap=0; iHeap < m_subTrackNodes.Size(); iHeap++)
	{
		const SubHeapTrack& sht = m_subTrackNodes[iHeap];

		switch (sht.m_type)
		{
		case kMemType_System:
			heapSys += sht.m_size;
			totalSystem += sht.m_size;
			break;

		case kMemType_Physical:
			heapPhy += sht.m_size;
			totalPhysical += sht.m_size;
			break;
		
		default:
			BIZ_ASSERT_FAIL("Not supported");
			break;
		}
	}

	if (heapSys || heapPhy)
	{
		Output("---------------------------------------------------------------------------\n");
		Output("heaps   :                         %6s %6s    %6s      %5d\n",
				MemSizeToStringK(heapSys).Chars(),
				MemSizeToStringK(heapPhy).Chars(),
				MemSizeToStringK(heapSys + heapPhy).Chars(),
				m_subTrackNodes.Size());
	}
	
	Output("---------------------------------------------------------------------------\n");
	Output("total   :                         %6s %6s    %6s      %5d\n",
			MemSizeToStringK(totalSystem).Chars(),
			MemSizeToStringK(totalPhysical).Chars(),
			MemSizeToStringK(totalSystem + totalPhysical).Chars(),
			totalAllocs);

	if (exclusionCount > 0)
	{
		for (U32 iCount=0; iCount < exclusionCount; iCount++)
		{
			const ContextNodeName& contextNodeName = TrackNodes::Instance().GetContextNode(exclusionList[iCount]);

			Output("excluded: %s\n", contextNodeName.m_name);
		}
	}
	
	int returnValue = 0;

	// Try and work out if there's any real leaks here (or systems doing allocations in this tag that might get freed up later)
	{
		unsigned int startTag = 0;

		// Try and find the second transition from game to UI - our starting point (else revert back to first)
		for (unsigned int i=1; i < currentTag-1; i++)
		{
			if ((TrackNodes::Instance().TagType(i) == TrackNodes::TagGame) && (TrackNodes::Instance().TagType(i+1) == TrackNodes::TagFrontend))
			{
				bool secondOne = startTag != 0;

				startTag = i+1;

				if (secondOne)
					break;
			}
		}

		// Did we find the starting point?
		if (startTag != 0)
		{
			unsigned int gc(0), fc(0);

			CORE_PRINT("...Scan tags from %d to %d\n", startTag, currentTag-2);

			for (unsigned int i=startTag; i < currentTag-2; i++)
			{
				if (TrackNodes::Instance().TagType(i) == TrackNodes::TagFrontend)
				{
					if (tagUsage[i].numAllocs > 0)
					{
						fc++;
						info.frontendAllocationCount += tagUsage[i].numAllocs;
						info.frontendLeakSystem += tagUsage[i].systemSize;
						info.frontendLeakPhysical += tagUsage[i].physicalSize;
					}
					else
						info.frontendNoAllocs = true;
				}
				else if (TrackNodes::Instance().TagType(i) == TrackNodes::TagGame)
				{
					if (tagUsage[i].numAllocs > 0)
					{
						gc++;
						info.gameAllocationCount += tagUsage[i].numAllocs;
						info.gameLeakSystem += tagUsage[i].systemSize;
						info.gameLeakPhysical += tagUsage[i].physicalSize;
					}
					else
						info.gameNoAllocs = true;
				}

				if (tagUsage[i].numAllocs)
				{
					CORE_PRINT("Tag(%3d) - Showing %d allocations\n", i, tagUsage[i].numAllocs);
				}
			}
			
			if (fc > 0)
			{
				info.frontendAllocationCount /= fc;
				info.frontendLeakSystem /= fc;
				info.frontendLeakPhysical /= fc;
			}

			if (gc > 0)
			{
				info.gameAllocationCount /= gc;
				info.gameLeakSystem /= gc;
				info.gameLeakPhysical /= gc;
			}

			// Used to check our own internal counts
		}
		else
		{
			returnValue = -3;	// Couldn't find starting transition from Game to Frontend
		}
	}

	free(tagUsage);
	
	m_Lock.Exit();

	return returnValue;
#else
	return -1;
#endif
}

void Tracker::DumpTagStatsSub(U32 exclusionCount, const U32* exclusionList, TagUsageInfo* tagUsage, const AllocationNode *nodeIterator) const
{
	const AllocationNode *previousNode = nodeIterator->m_pPrev;

	int count = 0;

	for (; nodeIterator; previousNode=nodeIterator, (nodeIterator=nodeIterator->m_pNext), count++)
	{
		BIZ_ASSERT(nodeIterator->m_pPrev == previousNode);
		const AllocationNode *pNode = nodeIterator;

		// Save the tag here as pNode could be shifting around later
		const U32 iTag = pNode->m_iTag;

		if (exclusionCount > 0)
		{
			bool match(false);

			for (U32 iCount=0; iCount < exclusionCount; iCount++)
			{
				if (exclusionList[iCount] == pNode->m_contextId)
				{
					match = true;
					break;
				}
			}

			if (match)
				continue;
		}

		// We need to store these here as we'll be moving the pointer around in a bit to much hilarity trying to access earlier member variables
		const bool isPhysical = (pNode->m_type >= kMemType_Physical);
		unsigned int waste;

		// Need to handle the case where we store down extra space for wastage (to handle big amounts of waste)
		if ((nodeIterator->m_type != (U32)kMemType_System) && (_msize((void*)nodeIterator) == (U32)m_trackerStructSize+4))
			waste = *((S32*)(((U8*)nodeIterator)+m_trackerStructSize));
		else
			waste = nodeIterator->m_iWaste;

		if ((m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG) == 0)
			pNode = POINTER_OFFSET(pNode, -giENUMERATE_ALLOCATIONS_FLAG_SIZE);

		if ((m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG) == 0)
			pNode = POINTER_OFFSET(pNode, -giCALLSTACK_FLAG_SIZE);

		if (m_iFlags & MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG)
		{
			if (!isPhysical)
				tagUsage[iTag].systemSize += pNode->m_cSize + waste + m_trackerStructSize;
			else
				tagUsage[iTag].physicalSize += pNode->m_cSize + waste + m_trackerStructSize;
		}

		tagUsage[iTag].numAllocs++;
	}
}

/// dump information about all allocated blocks to the debug logging system
void Tracker::DumpTagStats(U32 exclusionCount, U32* exclusionList, bool includeChildren)
{
#ifndef CFG_GOLD
	if (!(m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG))
		return;

	m_Lock.Enter();

	const unsigned int currentTag = TrackNodes::Instance().Tag();

	TagUsageInfo* tagUsage = (TagUsageInfo*) malloc(sizeof(TagUsageInfo) * (currentTag+1));

	if (!tagUsage)
	{
		m_Lock.Exit();
		return;
	}

	memset(tagUsage, 0, sizeof(TagUsageInfo) * (currentTag+1));

	DumpTagStatsSub(exclusionCount, exclusionList, tagUsage, GetFirstAllocationNode());

	if (includeChildren)
	{
		for (int i=0; i < m_subTrackers.Size(); i++)
		{
			DumpTagStatsSub(exclusionCount, exclusionList, tagUsage, m_subTrackers[i]->GetFirstAllocationNode());
		}
	}

	Output( "\n");
	Output( "-- memory tracker tag stats --      system     phys       total     allocs\n");

	size_t totalSystem(0), totalPhysical(0), totalAllocs(0);
	for (unsigned int i=0; i <= currentTag; i++)
	{
		totalSystem += tagUsage[i].systemSize;
		totalPhysical += tagUsage[i].physicalSize;
		totalAllocs += tagUsage[i].numAllocs;

		if ((i > 0) && (i % 2 == 0))
			Output( "---------------------------------------------------------------------------\n");

		TrackNodes::eTagType tt = TrackNodes::Instance().TagType(i);

		Output("tag %3d :                         %6s %6s    %6s      %5d    %s\n", i,
			MemSizeToStringK(tagUsage[i].systemSize).Chars(),
			MemSizeToStringK(tagUsage[i].physicalSize).Chars(), 
			MemSizeToStringK(tagUsage[i].systemSize + tagUsage[i].physicalSize).Chars(), 
			tagUsage[i].numAllocs,
			(tt == 0) ? "Frontend" : (tt == 1) ? "Game" : "Unknown"
			);
	}
	
	Output( "---------------------------------------------------------------------------\n");
	Output("total   :                         %6s %6s    %6s      %5d\n",
			MemSizeToStringK(totalSystem).Chars(),
			MemSizeToStringK(totalPhysical).Chars(),
			MemSizeToStringK(totalSystem + totalPhysical).Chars(),
			totalAllocs);

	if (exclusionCount > 0)
	{
		for (U32 iCount=0; iCount < exclusionCount; iCount++)
		{
			const ContextNodeName& contextNodeName = TrackNodes::Instance().GetContextNode(exclusionList[iCount]);

			Output("excluded: %s\n", contextNodeName.m_name);
		}
	}

	free(tagUsage);
	
	m_Lock.Exit();
#endif
}

void Tracker::DebugTagAndNode(bool includeChildren)
{
#ifndef CFG_GOLD
	if (!(m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG))
		return;

	int nNodes = TrackNodes::Instance().GetNumContextNodes();

	struct DebugNodeInfo
	{
		size_t	psize;
		size_t	ssize;
		size_t	tsize;
		int		allocs;
	};

	DebugNodeInfo* debugNodeInfo = (DebugNodeInfo*)malloc(sizeof(DebugNodeInfo) * nNodes);

	if (!debugNodeInfo)
		return;

	memset(debugNodeInfo, 0, sizeof(DebugNodeInfo) * nNodes);

	size_t total(0);
	int totalAllocs(0);

	// Get stats from alloc node's
	AllocationNode *nodeIterator = Tracker::GetFirstAllocationNode();
	AllocationNode *previousNode = nodeIterator->m_pPrev;

	for (; nodeIterator; previousNode=nodeIterator, (nodeIterator=nodeIterator->m_pNext))
	{
		BIZ_ASSERT(nodeIterator->m_pPrev == previousNode);

		AllocationNode *pNode = nodeIterator;

		const ContextId id = pNode->m_contextId;
		size_t waste;

		// Need to handle the case where we store down extra space for wastage (to handle big amounts of waste)
		if ((nodeIterator->m_type != (U32)kMemType_System) && (_msize(nodeIterator) == (U32)m_trackerStructSize+4))
			waste = *((S32*)(((U8*)nodeIterator)+m_trackerStructSize));
		else
			waste = nodeIterator->m_iWaste;

		const bool isPhysical = pNode->m_type >= kMemType_Physical;

		if ((m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG) == 0)
			pNode = POINTER_OFFSET(pNode, -giENUMERATE_ALLOCATIONS_FLAG_SIZE);

		if ((m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG) == 0)
			pNode = POINTER_OFFSET(pNode, -giCALLSTACK_FLAG_SIZE);

		if (m_iFlags & MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG)
		{
			const size_t allocSize = pNode->m_cSize + waste;
			
			debugNodeInfo[id].tsize += allocSize;
			if (!isPhysical)
				debugNodeInfo[id].ssize += allocSize;
			else
				debugNodeInfo[id].psize += allocSize;

			total += allocSize;
		}

		debugNodeInfo[id].allocs++;
		totalAllocs++;
	}
	// collect pointers to nodes
	ContextNodeNamePtr *pList = (ContextNodeNamePtr*)malloc(sizeof(ContextNodeNamePtr) * nNodes);
	BIZ_ASSERT(pList);
	float fTotal = 0;
	for (int i=0; i<nNodes; i++)
	{
		pList[i].pNodeName	= &TrackNodes::Instance().GetContextNode(i);
		pList[i].pNode		= &GetContextNode(i);

		if (!pList[i].pNode->m_pParent)
			fTotal += pList[i].pNode->Allocated();
	}
	// no memory allocated - assume memory tracking is disabled
	if (fTotal == 0)
	{
		free(pList);
		return;
	}

	// sort nodes
	std::sort(&pList[0], &pList[nNodes]);

	// find node with longest name
	int iLongest = 30;
	for (int i=0; i<nNodes; i++)
	{
		int c = (int)strlen(pList[i].pNodeName->m_name);
		if (c > iLongest)
			iLongest = c;
	}
	iLongest++;

	// format string that pads out to the longest name
	char formatString[512];
	sprintf(formatString, "%%-%ds: %%s %%s  %%10d    %%6.2f%s     %%s  %%s  %%s  %%d\n", iLongest, "%%");

	char headerString[512];
	iLongest-=30;
	if (iLongest>511)
		iLongest = 511;

	{
		int i;
		for (i=0; i<iLongest; i++)
			headerString[i] = ' ';
		headerString[i] = 0;
	}

	// print nodes
	Output("\n");
	Output("-- memory tracker stats --    %s     size     wasted allocations percentage       p_size    s_size  tot_size alloc_count\n", headerString);
	for (int i=0; i<nNodes; i++)
	{
		const ContextNodeName *pNodeName = pList[i].pNodeName;
		const ContextNode *pNode = pList[i].pNode;

		if (pNode->m_nAllocations > 0)
		{
			int id = int(pList[i].pNode - m_Node);

			Output( formatString,
				pNodeName->m_name, 
				MemSizeToStringK(pNode->Allocated()).Chars(),
				MemSizeToStringK(pNode->m_iWasted).Chars(),
				pNode->m_nAllocations,
				(100.0f * pNode->Allocated()) / fTotal,
				MemSizeToStringK(debugNodeInfo[id].psize).Chars(),
				MemSizeToStringK(debugNodeInfo[id].ssize).Chars(),
				MemSizeToStringK(debugNodeInfo[id].tsize).Chars(),
				debugNodeInfo[id].allocs
				);
		}
	}

	Output( "---------------------------------------------------------------------------\n");

	// Used to check our own internal counts

	// Only display this if we have both system and physical memory allocations
	if (m_physicalMemoryNode.m_nAllocations && m_systemMemoryNode.m_nAllocations)
	{
		Output( formatString,
			"Physical", 
			MemSizeToStringK(m_physicalMemoryNode.Allocated()).Chars(),
			MemSizeToStringK(m_physicalMemoryNode.m_iWasted).Chars(),
			m_physicalMemoryNode.m_nAllocations,
			(100.0f * m_physicalMemoryNode.Allocated()) / fTotal,
			"-", "-", "-", 0
			);


		Output( formatString,
			"System", 
			MemSizeToStringK(m_systemMemoryNode.Allocated()).Chars(),
			MemSizeToStringK(m_systemMemoryNode.m_iWasted).Chars(),
			m_systemMemoryNode.m_nAllocations,
			(100.0f * m_systemMemoryNode.Allocated()) / fTotal,
			"-", "-", "-", 0
			);

		Output( "---------------------------------------------------------------------------\n");
	}

	const size_t totalAllocated = m_physicalMemoryNode.Allocated() + m_systemMemoryNode.Allocated();

	Output( formatString,
		"Total", 
		MemSizeToStringK(totalAllocated).Chars(),
		MemSizeToStringK(m_physicalMemoryNode.m_iWasted + m_systemMemoryNode.m_iWasted).Chars(),
		m_physicalMemoryNode.m_nAllocations + m_systemMemoryNode.m_nAllocations,
		(100.0f * totalAllocated) / fTotal,
		MemSizeToStringK(total).Chars(),
		MemSizeToStringK(0).Chars(),
		MemSizeToStringK(0).Chars(),
		totalAllocs
		);



	free(pList);
	free(debugNodeInfo);
#endif
}

// Dump memory stats to a file with as little memory overhead as possible - might have already run out
void Tracker::DumpMemoryRaw(const char* fname)
{
#ifndef CFG_GOLD
	m_Lock.Enter();

	#ifdef PLATFORM_PS3
	char filename[260];
	#else
	char filename[MAX_PATH];
	#endif
	
	{
#ifdef PLATFORM_PS3
		sprintf_s(filename, sizeof(filename),"%s.csv", fname);
#else
		sprintf_s(filename, "%s.csv", fname);
#endif
		FILE *fh = fopen(filename, "wt");
		if (fh)
		{
			fputs("Name,Size,Wasted,Allocations\n", fh);

			int nNodes = TrackNodes::Instance().GetNumContextNodes();

			for (int iNode=0; iNode < nNodes; iNode++)
			{
				const ContextNodeName& cnn = TrackNodes::Instance().GetContextNode(iNode);
				const ContextNode& cn = GetContextNode(iNode);

				if (cn.Allocated() || cn.Wasted() || cn.Allocations())
				{
					fprintf(fh, "%s,%d,%d,%d,%d\n", cnn.m_name, cn.Allocated(), cn.AllocatedMax(), cn.Wasted(), cn.Allocations());
				}
			}

			fprintf(fh, ",,,,Physical,%d,%d,%d,%d\n", m_physicalMemoryNode.Allocated(), m_physicalMemoryNode.AllocatedMax(), m_physicalMemoryNode.Wasted(), m_physicalMemoryNode.Allocations());
			fprintf(fh, ",,,,System,%d,%d,%d,%d\n", m_systemMemoryNode.Allocated(), m_systemMemoryNode.AllocatedMax(), m_systemMemoryNode.Wasted(), m_systemMemoryNode.Allocations());
			fprintf(fh, ",,,,Total,%d,%d,%d,%d\n", m_physicalMemoryNode.Allocated()+m_systemMemoryNode.Allocated(), m_physicalMemoryNode.AllocatedMax()+m_systemMemoryNode.AllocatedMax(), m_physicalMemoryNode.Wasted()+m_systemMemoryNode.Wasted(), m_physicalMemoryNode.Allocations()+m_systemMemoryNode.Allocations());

			fclose(fh);
		}
	}

	if (AllocationNode *firstNode = GetFirstAllocationNode())
	{
#ifdef PLATFORM_PS3
		sprintf_s(filename, sizeof(filename), "%s.mtrack", fname);
#else
		sprintf_s(filename, "%s.mtrack", fname);
#endif
		FILE *fh = fopen(filename, "wb");
		if (fh)
		{
#if defined(PLATFORM_PS3)
			fprintf(fh, "Platform PS3\n");
#else
			fprintf(fh, "Platform OTHER\n");
#endif
			// Try and find out where the PDB is located

			// Write out the heap information
			for (int iHeap=0; iHeap < m_subTrackers.Size(); iHeap++)
			{
				fprintf(fh, "Heap %d\n", iHeap+1);
				fprintf(fh, "%s\n", m_subTrackers[iHeap]->m_name.Chars());
			}

			{
				const int nNodes = TrackNodes::Instance().GetNumContextNodes();

				// collect pointers to nodes
				ContextNodeNamePtr *pList = (ContextNodeNamePtr*) _alloca(sizeof(ContextNodeNamePtr) * nNodes);

				if (!pList)
				{
					m_Lock.Exit();
					return;
				}

				for (int i=0; i < nNodes; i++)
				{
					pList[i].pNodeName = &TrackNodes::Instance().GetContextNode(i);
					pList[i].pNode = &GetContextNode(i);
				}

				// Sort nodes
				std::sort(&pList[0], &pList[nNodes]);

				fprintf(fh, "ContextNodes %d\n", nNodes);

				for (int i=0; i < nNodes; i++)
				{
					const ContextNodeName& nodeName = TrackNodes::Instance().GetContextNode(i);

					fprintf(fh, "%s\n", nodeName.m_name);

					// Parent index and sorted order of nodes
					fprintf(fh, "%d %d\n", (nodeName.m_parent) ? nodeName.m_parent : -1, pList[i].pNode - m_Node);
				}

				//_freea(pList);
			}

			int itemCount(0);

			for (int iHeap=0; iHeap <= m_subTrackers.Size(); iHeap++)
			{
				AllocationNode *nodeIterator = (iHeap > 0) ? m_subTrackers[iHeap-1]->GetFirstAllocationNode() : GetFirstAllocationNode();

				for (; nodeIterator; nodeIterator=nodeIterator->m_pNext)
					itemCount++;
			}

			fprintf(fh, "AllocStart 0 %d\n", itemCount);

			// Big loop - go through all available heaps
			for (int iHeap=0; iHeap <= m_subTrackers.Size(); iHeap++)
			{
				AllocationNode *nodeIterator = (iHeap > 0) ? m_subTrackers[iHeap-1]->GetFirstAllocationNode() : GetFirstAllocationNode();

				for (; nodeIterator; nodeIterator=nodeIterator->m_pNext)
				{
					AllocationNode *pNode = nodeIterator;

					if ((m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG) == 0)
						pNode = POINTER_OFFSET(pNode, -giENUMERATE_ALLOCATIONS_FLAG_SIZE);

					const size_t callstackSize = (m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG) ? BizNonTrackedSize(pNode->m_pCallStack) : 0;

					void *callstackPtr = pNode->m_pCallStack;

					if ((m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG) == 0)
						pNode = POINTER_OFFSET(pNode, -giCALLSTACK_FLAG_SIZE);

					unsigned int allocationSize = 0;

					if (m_iFlags & MEMORY_TRACKER_BLOCK_LOCATIONS)
					{
						allocationSize = pNode->m_cSize;
					}

					int waste;

					// Need to handle the case where we store down extra space for wastage (to handle big amounts of waste)
					if ((nodeIterator->m_type != (U32)kMemType_System) && (_msize(nodeIterator) == (U32)m_trackerStructSize+4))
						waste = *((S32*)(((U8*)nodeIterator)+m_trackerStructSize));
					else
						waste = nodeIterator->m_iWaste;

					// Write section name and a version number to send...
					fprintf(fh, "Alloc 2 %d %d %d %d %d %d %d\n",
						int(nodeIterator->m_contextId),
						allocationSize,
						waste,
						int(nodeIterator->m_type),
						int(nodeIterator->m_iTag),
						int(callstackSize / sizeof(void*)),
						iHeap);

					if (callstackSize > 0)
					{
						const DWORD* csPtr = (DWORD*)callstackPtr;

						fprintf(fh, "%x", (int)*csPtr++);

						for (size_t ics=sizeof(void*); ics < callstackSize; ics+=sizeof(void*))
							fprintf(fh, " %x", (int)*csPtr++);

						fprintf(fh, "\n");
					}
				}
			}

			fputs("AllocStop\n", fh);
			fclose(fh);
		}
	}
	
	m_Lock.Exit();
#endif
}

void Tracker::GetContextNode(ContextId id, ContextNode& node, bool includeChildTrackers) const
{
	memcpy(&node, &GetContextNode(id), sizeof(ContextNode));

	if (includeChildTrackers)
	{
		for (int iSubHeap=0; iSubHeap < m_subTrackers.Size(); iSubHeap++)
		{
			const Tracker* st = m_subTrackers[iSubHeap];

			node.Add(st->GetContextNode(id));
		}
	}
}

/// dump memory stats to the debug logging system
void Tracker::DumpNodeStats(bool bCSV, bool includeChildren)
{
#ifndef CFG_GOLD
	m_Lock.Enter();

	int nNodes = TrackNodes::Instance().GetNumContextNodes();

	// collect pointers to nodes
	ContextNodeNamePtr *pList = (ContextNodeNamePtr*)malloc(sizeof(ContextNodeNamePtr) * nNodes);

	// Check we didn't run out of memory
	if (!pList)
	{
		m_Lock.Exit();
		return;
	}
	
	ContextNode *pTotNodes = (ContextNode*)_alloca(sizeof(ContextNode) * nNodes);

	// We copy the existing context nodes holding memory usage so we can accumulate the other heaps into these
	ContextNode tempPhysNode	= m_physicalMemoryNode;
	ContextNode tempSysNode		= m_systemMemoryNode;
	memcpy(pTotNodes, &this->GetContextNode(0), sizeof(ContextNode) * nNodes);
	TrackNodes::Instance().UpdateParentPointers(pTotNodes, nNodes);

	if (!includeChildren)
	{
		// We display sub-heaps in the overall view for this heap
		for (int iSubHeap=0; iSubHeap < m_subTrackNodes.Size(); iSubHeap++)
		{
			const SubHeapTrack& sht = m_subTrackNodes[iSubHeap];

			pTotNodes[sht.m_contextId].UpdateMemoryOnNode(sht.m_size, 0, 1);

			switch (sht.m_type)
			{
			case kMemType_System:
				tempSysNode.UpdateMemoryOnNode(sht.m_size, 0, 1);
				break;

			case kMemType_Physical:
				tempPhysNode.UpdateMemoryOnNode(sht.m_size, 0, 1);
				break;

			default:
				BIZ_ASSERT_FAIL("Not supported");
			}
		}
	}
	else
	{
		// We merge in the individual allocations from all sub-heaps
		for (int iSubHeap=0; iSubHeap < m_subTrackers.Size(); iSubHeap++)
		{
			const Tracker* st = m_subTrackers[iSubHeap];

			for (int i=0; i < nNodes; i++)
			{
				pTotNodes[i].Add(st->GetContextNode(i));
			}

			tempPhysNode.Add(st->m_physicalMemoryNode);
			tempSysNode.Add(st->m_systemMemoryNode);
		}
	}

	float fTotal = 0;
	for (int i=0; i<nNodes; i++)
	{
		pList[i].pNodeName = &TrackNodes::Instance().GetContextNode(i);
		pList[i].pNode = &pTotNodes[i];
		if (!pList[i].pNode->m_pParent)
			fTotal += pList[i].pNode->Allocated();
	}

	// no memory allocated - assume memory tracking is disabled
	if (fTotal == 0)
	{
		free(pList);
		m_Lock.Exit();
		return;
	}

	// sort nodes
	std::sort(&pList[0], &pList[nNodes]);

	// find node with longest name
	int iLongest = 30;
	for (int i=0; i<nNodes; i++)
	{
		int c = (int)strlen(pList[i].pNodeName->m_name);
		if (c > iLongest)
			iLongest = c;
	}
	iLongest++;

	// format string that pads out to the longest name
	char formatString[512];
	sprintf_s(formatString, sizeof(formatString), "%%-%ds: %%s %%s %%s  %%10d    %%6.2f%s\n", iLongest, "%%");

	char headerString[512];
	iLongest-=30;
	if (iLongest>511)
		iLongest = 511;

	{
		int i;
		for (i=0; i<iLongest; i++)
			headerString[i] = ' ';
		headerString[i] = 0;
	}

	// print nodes
	if (!bCSV)
	{
		Output("\n");
		Output("-- %s --\n", (includeChildren) ? "Merged Heaps" : m_name.Chars());
		Output("-- memory tracker stats --    %s      size      max   wasted allocations percentage\n", headerString);
	}
	else
	{
		//Output( "---------------------------------------------------------------------------\n");
		Output("Name, Size, Wasted, Allocations, Max\n");
	}
	for (int i=0; i<nNodes; i++)
	{
		const ContextNodeName *pNodeName = pList[i].pNodeName;
		const ContextNode *pNode = pList[i].pNode;

		if (pNode->m_nAllocations > 0)
		{
			if (!bCSV)
			{
				Output( formatString,
					pNodeName->m_name, 
					MemSizeToStringK(pNode->Allocated()).Chars(),
					MemSizeToStringK(pNode->AllocatedMax()).Chars(),
					MemSizeToStringK(pNode->Wasted()).Chars(),
					pNode->Allocations(),
					(100.0f * pNode->Allocated()) / fTotal
					);
			}
			else
			{
				#if 1 // [LloydW] Dont format output for pie chart for the time being.
				Output( "%s, %d, %d, %d, %d\n", pNodeName->m_name, pNode->Allocated(), pNode->Wasted(), pNode->Allocations(), pNode->AllocatedMax());
				#else
				Output( "%s, %d, %d, %d, %d", pNodeName->m_name, pNode->AllocatedSingle(), pNode->WastedSingle(), pNode->AllocationsSingle(), pNode->AllocatedMaxSingle());
				#endif
			}
		}
	}

	if (!bCSV)
		Output( "---------------------------------------------------------------------------\n");

	// Used to check our own internal counts

	// Only display this if we have both system and physical memory allocations
	if (tempPhysNode.m_nAllocations && tempSysNode.m_nAllocations)
	{
		if (!bCSV)
		{
			Output( formatString,
				"Physical", 
				MemSizeToStringK(tempPhysNode.Allocated()).Chars(),
				MemSizeToStringK(tempPhysNode.AllocatedMax()).Chars(),
				MemSizeToStringK(tempPhysNode.Wasted()).Chars(),
				tempPhysNode.Allocations(),
				(100.0f * tempPhysNode.Allocated()) / fTotal
				);
		}
		else
		{
			Output( "Physical, %d, %d, %d, %d\n", tempPhysNode.AllocatedSingle(), tempPhysNode.WastedSingle(), tempPhysNode.AllocationsSingle(), tempPhysNode.AllocatedMaxSingle());
		}


		if (!bCSV)
		{
			Output( formatString,
				"System", 
				MemSizeToStringK(tempSysNode.Allocated()).Chars(),
				MemSizeToStringK(tempSysNode.AllocatedMax()).Chars(),
				MemSizeToStringK(tempSysNode.Wasted()).Chars(),
				tempSysNode.m_nAllocations,
				(100.0f * tempSysNode.Allocated()) / fTotal
				);
		}
		else
		{
			Output("System, %d, %d, %d, %d\n", tempSysNode.AllocatedSingle(), tempSysNode.WastedSingle(), tempSysNode.AllocationsSingle(), tempSysNode.AllocatedMaxSingle());
		}

		if (!bCSV)
			Output( "---------------------------------------------------------------------------\n");
	}

	const size_t totalAllocated = tempPhysNode.Allocated() + tempSysNode.Allocated();

	if (!bCSV)
	{
		Output( formatString,
			"Total", 
			MemSizeToStringK(totalAllocated).Chars(),
			MemSizeToStringK(0).Chars(),
			MemSizeToStringK(tempPhysNode.m_iWasted + tempSysNode.m_iWasted).Chars(),
			tempPhysNode.m_nAllocations + tempSysNode.m_nAllocations,
			(100.0f * totalAllocated) / fTotal
			);
	}
	else
	{
		Output("Total, %d, %d, %d, %d\n", totalAllocated, tempPhysNode.m_iWasted + tempSysNode.m_iWasted, tempPhysNode.m_nAllocations + tempSysNode.m_nAllocations, tempPhysNode.m_iAllocatedMax + tempSysNode.m_iAllocatedMax);
	}

	if (!bCSV)
	{

		if (m_mainTracker)
		{
			if (!includeChildren)
			{
			}
			else
			{
				// Show free amounts in sub-heaps
				for (int iSubHeap=0; iSubHeap < m_subTrackers.Size(); iSubHeap++)
				{
					const Tracker* st = m_subTrackers[iSubHeap];

					Util::C_String freeCaption;
					freeCaption.Printf("Free(%s)", st->m_name.Chars());

					size_t actualBufferFree = st->m_bufferSize - st->m_physicalMemoryNode.Allocated() - st->m_systemMemoryNode.Allocated();

					Output( formatString,
						freeCaption.Chars(), 
						MemSizeToStringK(actualBufferFree).Chars(),
						MemSizeToStringK(0).Chars(),
						MemSizeToStringK(0).Chars(),
						0,
						(100.0f * actualBufferFree) / st->m_bufferSize,
						"-", "-", "-", 0
						);
				}
			}

#ifdef PLATFORM_PS3
			U32 freeMemory = (512 * 1024 * 1024) - totalAllocated;//memInfo.total_user_memory-totalAllocated;
			Output(formatString,
				"Free(Out of 512)", 
				MemSizeToStringK(freeMemory).Chars(),
				MemSizeToStringK(0).Chars(),
				MemSizeToStringK(0).Chars(),
				0,
				0.0f,
				"-", "-", "-", 0
				);
#endif

		}
		else
		{
			size_t actualFree = m_bufferSize - m_physicalMemoryNode.Allocated() - m_systemMemoryNode.Allocated();

			Output( formatString,
				"Free(In Heap)",
				MemSizeToStringK(actualFree).Chars(),
				MemSizeToStringK(0).Chars(),
				MemSizeToStringK(0).Chars(),
				0,
				(100.0f * actualFree) / m_bufferSize,
				"-", "-", "-", 0
				);
		}
	}

	//_freea(pTotNodes);
	free(pList);

	m_Lock.Exit();
#endif
}

/*
/// save mtune memory profiling data file
/// assumes
void Tracker::SaveMTuneFile(const Util::C_String &filename)
{
#ifndef CFG_GOLD
#if !( (defined( PLATFORM_PC ) && !defined( _WIN64 )) || defined( PLATFORM_XENON ) )
	return;
#endif
	// all flags need to be enabled for this to work
	if (m_iFlags != 7)
		return;

	Sys::I_File *pFile = ::Debug::g_pFileSystem->Open(filename, Sys::E_FileAccess_Write);
	if (!pFile)
		return;

	Sys::BinarySerialiser<Ut::NativeToLittleEndian> ar(*pFile);

	// write header + modules
	SaveModuleInfo(pFile);

	// write actual allocation data
	for (AllocationNode *pNode = Tracker::GetFirstAllocationNode(); pNode; (pNode=pNode->m_pNext))
	{
		const size_t cCallStack = (pNode->m_pCallStack) ? (_msize(pNode->m_pCallStack) / sizeof(void*)) : 0; // pNode->m_iCallStackSize;
		int *pCallStack = (int*)pNode->m_pCallStack;
		ar << (int)(U64)pNode->m_pAddress << pNode->m_cSize;
		for (size_t i=0; i<cCallStack && pCallStack[i]!=0; i++)
			ar << pCallStack[i];
		ar << (int)0;
	}
	ar << (int)0;

	delete pFile;
#endif
}
*/
/// set whether the memory tracker will call DumpNodeStats when it is destroyed
void Tracker::DumpStatsOnExit(bool bOn)
{
	Tracker::m_bDumpStatsOnExit = bOn;
}

/// set whether the memory tracker will call DumpAllocatedBlocks when it is destroyed
void Tracker::DumpBlocksOnExit(bool bOn)
{
	Tracker::m_bDumpBlocksOnExit = bOn;
}

/// set output callback function
void Tracker::SetOutputCallBackFunction(OutputCallBackFunction *pFunc)
{
	Tracker::m_pOutputCallBackFunction = pFunc;
}

/// output a string
void Tracker::Output(char *psz, ...)
{
#ifndef CFG_GOLD
	char buffer[ 1024 ];
    
	va_list arglist;
	va_start( arglist, psz );
	vsprintf_s( buffer, 1024, psz, arglist );
	va_end( arglist );

	if (m_pOutputCallBackFunction)
		(*m_pOutputCallBackFunction)(buffer);
	else
	{
#if defined( COMPILER_VC )
		OutputDebugString(buffer);
#else
		printf(buffer);
#endif
	}
#endif
}

/// add the names of all tracking nodes to a vector of strings
void Tracker::GetContextNodeList(Ut::Vector<Util::C_String> &list)
{
#if !defined(CFG_GOLD)
	m_Lock.Enter();

	const int nNodes = TrackNodes::Instance().GetNumContextNodes();
	for (int i=0; i < nNodes; i++)
		list.PushBack(Util::C_String(TrackNodes::Instance().GetContextNode(i).m_name));

	m_Lock.Exit();
#endif
}

void Tracker::SendContextNodeList(ArteryMessageBuilder &b)
{
#if !defined(CFG_GOLD)
	m_Lock.Enter();

	const int nNodes = TrackNodes::Instance().GetNumContextNodes();

	// collect pointers to nodes
	ContextNodeNamePtr *pList = (ContextNodeNamePtr*) malloc(sizeof(ContextNodeNamePtr) * nNodes);

	if (!pList)
	{
		m_Lock.Exit();
		return;
	}
	
	for (int i=0; i < nNodes; i++)
	{
		pList[i].pNodeName = &TrackNodes::Instance().GetContextNode(i);
		pList[i].pNode = &this->GetContextNode(i);
	}

	// Sort nodes
	std::sort(&pList[0], &pList[nNodes]);

	b.Write(nNodes);

	for (int i=0; i < nNodes; i++)
	{
		const ContextNodeName& nodeName = TrackNodes::Instance().GetContextNode(i);

		b.Write(nodeName.m_name);
		
		b.Write(int((nodeName.m_parent) ? nodeName.m_parent : -1));
	}

	// Now we write out the sorted order of the nodes
	for (int i=0; i < nNodes; i++)
	{
		int contextIndex = int(pList[i].pNode - m_Node);
		b.Write(contextIndex);
	}

	free(pList);

	m_Lock.Exit();
#endif
}

void Tracker::SendAllocs(int version, int heapId, ArteryConnection con, bool justSuspectedLeaks, bool bSingleMessage)
{
	int nAllocationsPerMessage = bSingleMessage ? 10 : 1;
	int nAllocations = 0;

	unsigned int currentTag = TrackNodes::Instance().Tag();

	ArteryMessageBuilder b;

	// Big loop
	{
		AllocationNode *nodeIterator = GetFirstAllocationNode();

		const int bundleSize = 4;

		AllocationNode *firstNode = GetFirstAllocationNode();

		while (nodeIterator)
		{
			bool addedAlloc(false);

			for (int iCount=0; nodeIterator && (iCount < bundleSize); nodeIterator=nodeIterator->m_pNext)
			{
				if (justSuspectedLeaks && ((nodeIterator->m_iTag < 2) || (nodeIterator->m_iTag == currentTag)))
					continue;

				if (nAllocations == 0)
				{
					b.Clear();
					b.Write("Alloc");

					// Write a version number to send...
					b.Write(int(nAllocationsPerMessage));
				}

				if (version >= 2)
				{
					// Output version
					b.Write(2);
				}

				if (version >= 1)
				{
					b.Write(heapId);
				}

				b.Write(int(nodeIterator->m_contextId));

				// only send block locations if they are being tracked
				if (m_iFlags & MEMORY_TRACKER_BLOCK_LOCATIONS_FLAG)
				{
					// offset of block location in tracking structure depends on whether callstack tracking is enabled
					AllocationNode *p = nodeIterator;
					if (!(m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG))
						p = POINTER_OFFSET(nodeIterator, -giCALLSTACK_FLAG_SIZE);

					b.Write(p->m_cSize);

					if (bSingleMessage)
						b.Write((int)(U64)p->m_pAddress);

					if (version >= 2)
					{
						b.Write((int)(U64)p->m_pAddress);
					}
				}

				// Need to handle the case where we store down extra space for wastage (to handle big amounts of waste)
				if ((nodeIterator->m_type != (U32)kMemType_System) && (_msize(nodeIterator) == (U32)m_trackerStructSize+4))
					b.Write(int(*((S32*)(((U8*)nodeIterator)+m_trackerStructSize))));
				else
					b.Write(int(nodeIterator->m_iWaste));

				b.Write(int(nodeIterator->m_type));
				b.Write(int(nodeIterator->m_iTag));

				if (version >= 2)
				{
					if (g_smallBlockAllocatorPtr && g_smallBlockAllocatorPtr->MemoryOwnedByAllocator(nodeIterator))
						b.Write((ArteryU8)1);
					else
						b.Write((ArteryU8)0);
				}

				// only send callstack info if it is being tracked
				if (m_iFlags & MEMORY_TRACKER_CALLSTACK_FLAG)
				{
					const size_t callstackSize = BizNonTrackedSize(nodeIterator->m_pCallStack);
					b.Write(int(callstackSize / sizeof(void*)));
					b.Write(nodeIterator->m_pCallStack, int(callstackSize));
				}
				else
				{
					b.Write((int)0);
				}
				//*u.dwordPtr++	= DWORD(pBlock->address);

				iCount++;
				addedAlloc = true;

				nAllocations++;
				if (nAllocations >= nAllocationsPerMessage)
				{
					ArteryMessage msgAllocInfo;
					if (b.Build(msgAllocInfo))
						con.Send(msgAllocInfo);
					nAllocations = 0;
				}
			}

			if (!con.IsConnected())
				break;
		}
	}

	if (nAllocations)
	{
		ArteryMessage msg;
		b.Build(msg);
		con.Send(msg);
	}
}

void Tracker::SendAllocationList(int version, bool justSuspectedLeaks, ArteryConnection con, bool bSingleMessage)
{
#if !defined(CFG_GOLD)
	m_Lock.Enter();

	// this function doesn't work without enumerate_allocations on
	if (!(m_iFlags & MEMORY_TRACKER_ENUMERATE_ALLOCATIONS_FLAG))
	{
		ArteryMessageBuilder b;
		b.Write("The correct Memory Tracking flags are not enabled.");
		ArteryMessage newMsg;
		if (b.Build(newMsg))
			con.Send(newMsg);

		m_Lock.Exit();
		return;
	}

	AllocationNode *firstNode = GetFirstAllocationNode();

	if (!firstNode)
	{
		m_Lock.Exit();
		//LOGINFO(g_Log, "Tracker::SendAllocationList - No allocation tracking info held.");
		return;
	}

	int itemCount(0);

	unsigned int currentTag = TrackNodes::Instance().Tag();

	for (AllocationNode *nodeIterator=firstNode; nodeIterator; nodeIterator=nodeIterator->m_pNext)
	{
		const unsigned int tag = nodeIterator->m_iTag;

		// Filter out any entries before we've gone back to the F.E. for the first time, or entries against the current Tag marker
		if ((!justSuspectedLeaks) || (tag > 1 && tag < currentTag))
			itemCount++;
	}

	for (int iHeap=0; iHeap < m_subTrackers.Size(); iHeap++)
	{
		for (AllocationNode *nodeIterator=m_subTrackers[iHeap]->GetFirstAllocationNode(); nodeIterator; nodeIterator=nodeIterator->m_pNext)
		{
			const unsigned int tag = nodeIterator->m_iTag;

			// Filter out any entries before we've gone back to the F.E. for the first time, or entries against the current Tag marker
			if ((!justSuspectedLeaks) || (tag > 1 && tag < currentTag))
				itemCount++;
		}
	}

	ArteryMessageBuilder b;
	if (bSingleMessage)
	{
		b.Write("MemoryCapture");
		b.Write(0); // version
#if defined(PLATFORM_PS3)
		b.Write(1); // PS3
#else
		b.Write(2); // undefined
#endif
	}

	if (version >= 1)
	{
		b.Clear();
		b.Write("BeginMemDump");
	}

	// Try and find out where the PDB is located

	// Send over heap information
	if (version >= 1)
	{
		for (int iHeap=0; iHeap < m_subTrackers.Size(); iHeap++)
		{
			b.Clear();

			b.Write("Heap");
			b.Write(iHeap + 1);
			b.Write(m_subTrackers[iHeap]->m_name.Chars());

			ArteryMessage newMsg;
			if (b.Build(newMsg))
				con.Send(newMsg);
		}
	}

	if (!bSingleMessage)
	{
		b.Clear();
		b.Write("ContextNodes");
	}

	SendContextNodeList(b);

	if (!bSingleMessage)
	{
		ArteryMessage newMsg;
		if (b.Build(newMsg))
			con.Send(newMsg);
	}

	if (!bSingleMessage)
	{
		b.Clear();
		b.Write("AllocStart");
		b.Write(int(0));			// Flags = 0
	}
	b.Write(itemCount);

	{
		ArteryMessage newMsg;
		if (b.Build(newMsg))
			con.Send(newMsg);
	}

	SendAllocs(version, 0, con, justSuspectedLeaks, bSingleMessage);

	for (int iHeap=0; iHeap < m_subTrackers.Size(); iHeap++)
	{
		m_subTrackers[iHeap]->SendAllocs(version, iHeap+1, con, justSuspectedLeaks, bSingleMessage);
	}

	if (con.IsConnected())
	{
		b.Clear();
		b.Write("AllocStop");

		ArteryMessage newMsg2;
		if (b.Build(newMsg2))
			con.Send(newMsg2);
	}

	m_Lock.Exit();
#endif
}

struct MTPeer
{
	ArteryConnection con;
};

MTPeer* g_mtPeer(NULL);

void TrackNodes::SendTagToTrackingTool() const
{
	if (g_mtPeer && g_mtPeer->con.IsConnected())
	{
		ArteryMessageBuilder b;

		b.Write("Tag");
		b.Write((int)m_tag);

		ArteryMessage m;

		if (b.Build(m))
			g_mtPeer->con.Send(m);
	}
}

unsigned int TrackNodes::IncrementTag(eTagType type)
{
	unsigned int oldTag = m_tag;

	if (m_tag < Enum_Max_Tags-1)
	{
		m_tag++;
		
		// Store what type of mode we're going into
		m_tagType[m_tag] = type;
	}

	SendTagToTrackingTool();

	if (g_showTagStatsOnIncrement)
	{
		Tracker::Instance().DumpTagStats(0, NULL, true);
	}

	if (g_showTagIncrement)
	{
		LOGWARN(g_Log, "****************************************");
		LOGWARN(g_Log, "* MemTrack Tag now %d (%s)", m_tag, (type == TagFrontend) ? "Frontend" : "Game");
		LOGWARN(g_Log, "****************************************");
	}

	return oldTag;
}

void ProcessNetworkCommunications(ArteryConnection con, void*)
{
	MTPeer dp;
	dp.con = con;

	g_mtPeer = &dp;
	
	TrackNodes::Instance().SendTagToTrackingTool();

	while (con.IsConnected())
	{
		ArteryMessage msg;

		if (con.Receive(msg))
		{
			ArteryMessageReader msgReader(msg);

			const char* msgString(NULL);

			if (!msgReader.Read(msgString))
				continue;
			
			if (!strcmp(msgString, "Allocations"))
			{
				Tracker::Instance().SendAllocationList(0, false, con, false);
			}
			else if (!strcmp(msgString, "Allocations2"))
			{
				int version;
				msgReader.Read(version);

				Tracker::Instance().SendAllocationList(version, false, con, false);
			}
			else if (!strcmp(msgString, "AllocationLeaks"))
			{
				Tracker::Instance().SendAllocationList(0, true, con, false);
			}
			else if (!strcmp(msgString, "IncrementTag"))
			{
				TrackNodes::Instance().IncrementTag();
			}
		}
	}

	g_mtPeer = NULL;
}

int InitialiseNetworkConnection(void)
{
#ifndef CFG_GOLD
	ArteryRegister("MemoryTracker", ProcessNetworkCommunications, 0);
#endif
	return 0;
}

void WalkTheHeap()
{
	//{
	//	HANDLE hPrivate = GetProcessHeap();

	//	HeapLock(hPrivate);

	//	// Traversing a Heap blocks by using a HeapWalk API.
	//	PROCESS_HEAP_ENTRY pHeapEntry;

	//	// Initialize data members of PROCESS_HEAP_ENTRY to Zero.
	//	ZeroMemory(&pHeapEntry,sizeof(pHeapEntry));
	//	pHeapEntry.lpData = NULL;
	//	unsigned long totalBytes = 0;
	//	unsigned long committedBytes = 0;
	//	unsigned long uncommittedBytes = 0;

	//	// This loop will iterate all the blocks of the memory and
	//	// will be calculate the committed and uncommitted memory
	//	// for a heap.
	//	BOOL bFirstBlock = false;
	//	unsigned long prevPointerAdd = 0;

	//	while (BOOL bWalk = HeapWalk(hPrivate, &pHeapEntry))
	//	{
	//		PROCESS_HEAP_ENTRY tempHeapEntry = pHeapEntry; // (PROCESS_HEAP_ENTRY) (*iterHeapEntry);

	//		//  This represents the REGION block, which will contain all the details about committed blocks and uncommitted
	//		//  blocks present in a Heap.
	//		if (!bFirstBlock)
	//		{
	//			totalBytes = (unsigned long) tempHeapEntry.lpData - (unsigned long) hPrivate;

	//			totalBytes += (unsigned long) tempHeapEntry.Region.lpFirstBlock - (unsigned long)tempHeapEntry.lpData;
	//			committedBytes += (unsigned long)tempHeapEntry.Region.lpFirstBlock - (unsigned long)tempHeapEntry.lpData;
	//			committedBytes += (unsigned long)tempHeapEntry.lpData - (unsigned long) hPrivate;
	//			bFirstBlock = true;
	//		}
	//		//  This represents the allocated blocks in a heap. The number of bytes in the block is obtained by subtracting the starting address (virtual address)
	//		//  of the next block from the starting address (virtual address) of the present block.
	//		if (tempHeapEntry.wFlags == PROCESS_HEAP_ENTRY_BUSY)
	//		{
	//			PROCESS_HEAP_ENTRY nextHeadEntry = tempHeapEntry; // (PROCESS_HEAP_ENTRY) (*iterHeapEntry);
	//			HeapWalk(hPrivate, &nextHeadEntry);

	//			unsigned long bytesAllocated = (unsigned long) nextHeadEntry.lpData - (unsigned long) tempHeapEntry.lpData;
	//			totalBytes += bytesAllocated;
	//			committedBytes += bytesAllocated;
	//		}
	//		// This represents the committed block which is free, i.e. not being allocated or not being used as control structure.
	//		// Data member cbData represents the size in bytes for this range of free block.
	//		if (tempHeapEntry.wFlags == 0)
	//		{
	//			totalBytes += tempHeapEntry.cbData;

	//			committedBytes += tempHeapEntry.cbData;
	//		}
	//		// For Uncommitted block, cbData represents the size (in bytes) for range of uncommitted memory.
	//		if (tempHeapEntry.wFlags == PROCESS_HEAP_UNCOMMITTED_RANGE)
	//		{
	//			uncommittedBytes += tempHeapEntry.cbData;
	//			totalBytes += tempHeapEntry.cbData;
	//		}
	//	}

	//	HeapUnlock(hPrivate);

		//LOGWARN(g_Log, "Total Bytes in Heap - %d", totalBytes);
		//LOGWARN(g_Log, "Total Committed Bytes in Heap - %d", committedBytes);
		//LOGWARN(g_Log, "Total UnCommitted Bytes in Heap - %d", uncommittedBytes);
	//}}
}

void DumpHashVirPhysicalStats()
{
	BIZ_ASSERT_FAIL("Removed - Can put back in");
	//const unsigned int size = g_hashVirPhysical.Size();
	//const unsigned int capacity = g_hashVirPhysical.Capacity();
	//LOGINFO(g_Log, "HashVirPhysicalStats : Size '%i' Capacity '%i' Free '%i'\n", size, capacity, capacity-size);
}

Debug::Bool g_memDumpStats("System.Profile.Memory.DumpStats", false);
Debug::Bool g_memTagAndNode("System.Profile.Memory.DumpTagAndNode", false);

Debug::Bool g_memDumpToFile("System.Profile.Memory.DumpNodesToFile", false);

Debug::Bool g_memSmallBlockDebug("System.Profile.Memory.SmallBlockAllocatorDebug", false);

void Update(void)
{
#ifndef CFG_GOLD
	if (g_memSmallBlockDebug)
	{
		g_smallBlockAllocatorPtr->DumpStats();
		g_memSmallBlockDebug = false;
	}
#endif

	if (g_memDumpStats)
	{
		//Memory::Tracker::Instance().DumpAllocatedBlocks();
		Memory::Tracker::Instance().DumpNodeStats(false, true);
		Memory::Tracker::Instance().DumpTagStats(0, NULL, true);
		g_memDumpStats = false;
	}

	if (g_memTagAndNode)
	{
		Memory::Tracker::Instance().DebugTagAndNode(true);
		g_memTagAndNode = false;
	}

	if (g_memDumpToFile)
	{
#if defined(PLATFORM_PS3)
		Tracker::Instance().DumpMemoryRaw("/app_home/MemStats");
#else
		Tracker::Instance().DumpMemoryRaw("MemStats");
#endif
		g_memDumpToFile = false;
	}

	Memory::Tracker::Instance().Update();
}

/*
	Callstack Tracker
*/
class CallstackTrackerImpl
{
public:
	typedef Btl::Map<void*, void*, Btl::DynamicPool<Btl::NullType> > CSMemoryMap;

	CSMemoryMap m_allocList;
};

CallstackTracker::CallstackTracker()
{
	m_impl = new CallstackTrackerImpl();
}

CallstackTracker::~CallstackTracker()
{
	delete m_impl;
}

void CallstackTracker::Add(void* ptr)
{
	void *pCallStack[MEMORY_TRACKER_MAX_CALLSTACK_SIZE];

	int callstackSize = Memory::CaptureCallStack(pCallStack, MEMORY_TRACKER_MAX_CALLSTACK_SIZE);

	if (callstackSize > 0)
	{
		// How much to trim off the callstack to allow for the function call level getting here
		int callstackOverhead = 0;
		int m_callStackSkip = sizeof(void*) * 2;

		callstackSize -= m_callStackSkip + callstackOverhead;

		void* pcs = BizNonTrackedMalloc(callstackSize);
		size_t iActualSize = BizNonTrackedSize(pcs);

		memcpy(pcs, ((U8*)pCallStack) + m_callStackSkip + callstackOverhead, callstackSize);

		CallstackTrackerImpl::CSMemoryMap::InsertionReturnType result = m_impl->m_allocList.Insert(CallstackTrackerImpl::CSMemoryMap::DataType(ptr, pcs));
		BIZ_ASSERT_MSG(result.inserted, "Already tracking this pointer");
	}
}

void CallstackTracker::Remove(void* ptr)
{
	CallstackTrackerImpl::CSMemoryMap::Iterator mmi = m_impl->m_allocList.Find(ptr);
	BizNonTrackedFree(mmi->GetValue());
	BIZ_ASSERT(mmi != m_impl->m_allocList.End());
	m_impl->m_allocList.Erase(ptr);
}

void CallstackTracker::Dump() const
{
	if (m_impl->m_allocList.Size() > 0)
	{
		CallstackTrackerImpl::CSMemoryMap::ConstIterator it = m_impl->m_allocList.Begin();

		for (; it != m_impl->m_allocList.End(); it++)
		{
			LOGINFO(g_Log, "CallstackBuddy - 0x%x\n", it->GetKey());

			void *pcs = it->GetValue();
			
			size_t stackDepth = BizNonTrackedSize(pcs) / 4;

			for (unsigned int i=0; i < stackDepth; i++)
			{
				LOGINFO(g_Log, "- %2d) 0x%08x\n", i, ((DWORD*)pcs)[i]);
			}
		}
	}
}

};
