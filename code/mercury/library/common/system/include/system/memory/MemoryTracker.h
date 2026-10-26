#ifndef __SYSTEM_MEMORY_TRACKER_H__
#define __SYSTEM_MEMORY_TRACKER_H__

#ifdef _MSC_VER
#pragma once
#endif

#include <core\threads.h>
#include <core\containers\string.h>
#include <btl\containers\vector.h>
//#include <core\containers\openhashtable.h>
#include <btl\allocators\pool.h>
#include <btl\containers\chainedhashmap.h>

class ArteryConnection;
class ArteryMessageBuilder;

#define MEMORY_TRACKER_MAX_NODE_NAME_SIZE (128-4)

// maximum callstack size
#define MEMORY_TRACKER_MAX_CALLSTACK_SIZE 20

namespace Sys
{
	class I_BinaryStream;
}

namespace Debug
{
	class Bool;
}

namespace Memory
{
	typedef enum E_MEMORY_TYPES
	{
		kMemType_System				= 0,
		kMemType_Virtual,
		kMemType_Physical,					// Must keep all non-physical allocation's before this point
		kMemType_Local,						// PS3 only
		kMemType_MAX						// Stored in a 4 bits - So can't go above 15
	};

#ifndef CFG_SPEED

// obsolete
#define MEMORY_TRACKER_ENABLE_CALLSTACK_CAPTURE	 {((void)0);}

#define MEMORY_TRACKER_CONTEXT_ID(X)	::Memory::TrackNodes::Instance().GetId(X)

#define MEMORY_TRACKER_CURRENT_CONTEXT()	::Memory::TrackNodes::Instance().GetContext()

#define MEMORY_TRACKER_PUSH(X) \
	{ \
	static ::Memory::ContextId __tmp = ::Memory::TrackNodes::Instance().GetId(X); \
	::Memory::TrackNodes::Instance().Push(__tmp); \
	} 

#define MEMORY_TRACKER_CONTEXT(X) \
	::Memory::MemoryTrackerContext __mtap(X);

#define MEMORY_TRACKER_PUSH_SUB(X) \
	{ \
	::Memory::ContextId __tmp = ::Memory::TrackNodes::Instance().GetIdSub(X); \
	::Memory::TrackNodes::Instance().Push(__tmp); \
	} 

#define MEMORY_TRACKER_PUSH_DYNAMIC(...) \
	{ \
	::Memory::ContextId __tmp = ::Memory::TrackNodes::Instance().GetIdDynamic(__VA_ARGS__ ); \
	::Memory::TrackNodes::Instance().Push(__tmp); \
	} 

#define MEMORY_TRACKER_CONTEXT_DYNAMIC(...) \
	::Memory::MemoryTrackerContext __mtap(::Memory::TrackNodes::Instance().GetIdDynamic(__VA_ARGS__ );

#define MEMORY_TRACKER_PUSH_BY_ID(X) \
	::Memory::TrackNodes::Instance().Push(X);

#ifdef CFG_DEBUG

#define MEMORY_TRACKER_POP(X) \
	{ \
	static ::Memory::ContextId __tmp = ::Memory::TrackNodes::Instance().GetId(X); \
	::Memory::TrackNodes::Instance().Pop(__tmp); \
	} 

#define MEMORY_TRACKER_POP_SUB(X) \
	{ \
	::Memory::ContextId __tmp = ::Memory::TrackNodes::Instance().GetCurrentIdWithSub(X); \
	::Memory::TrackNodes::Instance().Pop(__tmp); \
	} 

#define MEMORY_TRACKER_POP_DYNAMIC(...) \
	{ \
	::Memory::ContextId __tmp = ::Memory::TrackNodes::Instance().GetIdDynamic(__VA_ARGS__ ); \
	::Memory::TrackNodes::Instance().Pop(__tmp); \
	} 

#define MEMORY_TRACKER_POP_BY_ID(X) \
	::Memory::TrackNodes::Instance().Pop(X);

#else

#define MEMORY_TRACKER_POP(X) \
	{ \
	::Memory::TrackNodes::Instance().Pop(); \
	} 

#define MEMORY_TRACKER_POP_SUB(X) \
	{ \
	::Memory::TrackNodes::Instance().Pop(); \
	} 

#define MEMORY_TRACKER_POP_DYNAMIC(...) \
	{ \
	::Memory::TrackNodes::Instance().Pop(); \
	} 

#define MEMORY_TRACKER_POP_BY_ID(X) \
	::Memory::TrackNodes::Instance().Pop();

#endif

#define MEMORY_TRACKER_SET_LIMIT(_X, _LIMIT) \
	{ \
	::Memory::ContextId __tmp = ::Memory::TrackNodes::Instance().GetId(_X);	\
	::Memory::TrackNodes::Instance().SetLimit(__tmp, _LIMIT);				\
	} 

#else

#define MEMORY_TRACKER_ENABLE_CALLSTACK_CAPTURE	 {((void)0);}

#define MEMORY_TRACKER_CONTEXT_ID(X)			(::Memory::ContextId(0))

#define MEMORY_TRACKER_CURRENT_CONTEXT()		(::Memory::ContextId(0))

#define MEMORY_TRACKER_PUSH(X)					{((void)0);}

#define MEMORY_TRACKER_CONTEXT(X)				{((void)0);}

#define MEMORY_TRACKER_PUSH_SUB(X)				{((void)0);}

#define MEMORY_TRACKER_PUSH_DYNAMIC(...)		{((void)0);}

#define MEMORY_TRACKER_CONTEXT_DYNAMIC(X)		{((void)0);}

#define MEMORY_TRACKER_PUSH_BY_ID(X)			{((void)0);}

#define MEMORY_TRACKER_POP(X)					{((void)0);}

#define MEMORY_TRACKER_POP_SUB(X)				{((void)0);}

#define MEMORY_TRACKER_POP_DYNAMIC(...)			{((void)0);}

#define MEMORY_TRACKER_POP_BY_ID(X)				{((void)0);}

#define MEMORY_TRACKER_SET_LIMIT(_X, _LIMIT)	{((void)0);}

#endif

#define INVALID_CONTEXT_ID 0

/// context ID is basically an int that defaults to 0
typedef U8 ContextId;

/// contains a single memory tracker context node
class ContextNodeName
{
public:
							/// constructor
							ContextNodeName();
	
	char					m_name[MEMORY_TRACKER_MAX_NODE_NAME_SIZE];
	U8						m_parent;										// 0=no parent
	U8						m_depth;										// Depth level of node in hierachy
	U8						m_sibling;
	U8						m_child;
};

/// contains a single memory tracker context node
class ContextNode
{
public:
							/// constructor
							ContextNode();

							
	ContextNode				*m_pParent;

	size_t					m_iAllocated, m_iAllocatedSingle;
	size_t					m_iAllocatedMax, m_iAllocatedMaxSingle;
	size_t					m_iWasted, m_iWastedSingle;
	size_t					m_maxAllowed;		// If non-zero, this is the size we should always be fitting inside
	int						m_nAllocations, m_nAllocationsSingle;
	int						m_nAllocationsMax, m_nAllocationsMaxSingle;

							/// Change the amount of allocated/wasted memory of this node and it's parents
	void					UpdateMemoryOnNode(int iAllocated, int iWasted, int nAllocations, bool bInherited = false);

							/// amount of memory allocated within this node
	size_t					AllocatedSingle() const;

							/// amount of memory allocated within this node and all it's children
	size_t					Allocated() const;

							/// maximum amount that was ever allocated on this node
	size_t					AllocatedMaxSingle() const;

							/// maximum amount that was ever allocated on this node and all it's children
	size_t					AllocatedMax() const;

							/// amount of wasted memory within this node
	size_t					WastedSingle() const;

							/// amount of wasted memory withing this node and all it's children
	size_t					Wasted() const;

							/// number of allocations within this node
	int						AllocationsSingle() const;

							/// number of allocations within this node and all it's children
	int						Allocations() const;

							/// maximum amount of allocations ever within this node
	int						AllocationsMaxSingle() const;

							/// maximum amount of allocations ever within this node and all it's children
	int						AllocationsMax() const;

	void					Add(const ContextNode& cn);
};

/// memory allocation info structure
class AllocationNode
{
public:
    ContextId				m_contextId;
	U8						m_iTag		: 7;		// We can track 127 tags
	U8						m_iAligned	: 1;		// Did we need to round up allocation size to keep the block aligned?
	U16						m_iWaste	: 12;		// TODO: For physical this isn't enough - could be 4096 or more depending on page size
	U16						m_type		: 4;

// MEMORY_TRACKER_ENUMERATE_ALLOCATIONS
	AllocationNode			*m_pNext;
	AllocationNode			*m_pPrev;
// MEMORY_TRACKER_ENUMERATE_ALLOCATIONS

// MEMORY_TRACKER_CALLSTACK
	void					*m_pCallStack;
// MEMORY_TRACKER_CALLSTACK

// MEMORY_TRACKER_BLOCK_LOCATIONS
	void					*m_pAddress;
	U32						m_cSize;
// MEMORY_TRACKER_BLOCK_LOCATIONS

	static U32 Size(int iFlags);
};

typedef void (OutputCallBackFunction)(char *psz);

class Tracker;

class TrackNodes
{
public:
	enum
	{
		Enum_Max_ContextNodes = 254,
	};

	enum
	{
		Enum_Max_Tags = 128,
	};

	enum eTagType
	{
		TagUnknown = -1,
		TagFrontend,
		TagGame,
	};

protected:
	ContextNodeName	m_Node[Enum_Max_ContextNodes];
	U16				m_nNodes;

	U8				m_sortedFirstNode;
	U8				m_tag;
	U8				m_tagType[Enum_Max_Tags];	// TODO: Only need one or two bits per tag, switch to something smaller

	MT::Lock		m_Lock;

	Tracker*		m_pTrackers;	// Maintain a list of all trackers so we can update their nodes
	
public:
	static TrackNodes& Instance()
	{
		static TrackNodes s_instance;
		return s_instance;
	}

	TrackNodes();

	void			Initialize();

					/// retrieve an Id for a given context hierarchy string
	ContextId		GetId(const Util::C_String &string);
	ContextId		GetIdSub(const Util::C_String &string);
	ContextId		GetIdDynamic(const char* string, ...);

#ifdef CFG_DEBUG
	ContextId		GetCurrentIdWithSub(const Util::C_String &string);
#endif

	ContextId		GetParentId(ContextId id);

					/// Push context item onto the stack
	void			Push(const ContextId item);

	void			UpdateParentPointers(ContextNode* pNodes, int numNodes);
					
#ifdef CFG_DEBUG
					/// Pop context item from the stack
	void			Pop(const ContextId &item);
#else
					/// Pop context item from the stack
	void			Pop();
#endif

					/// Set the size that this context should never grow beyond
	void			SetLimit(size_t maxLimit);

							/// retrieve the context structure for a given Id
	const ContextNodeName&	GetContextNode(ContextId id)
	{
		return m_Node[id];
	}
							/// Get current thread context item
	const ContextId&		GetContext();

							/// get number of nodes
	int GetNumContextNodes() const
	{
		return m_nNodes;
	}

	unsigned int Tag() const
	{
		return m_tag;
	}

	eTagType TagType(unsigned int tag) const
	{
		return (eTagType)m_tagType[tag];
	}

	unsigned int SortedFirstNode() const
	{
		return m_sortedFirstNode;
	}

	unsigned int IncrementTag(eTagType type=TagUnknown);
	void SendTagToTrackingTool() const;

	void					SetTag(unsigned int tag)
	{
		assert(tag < 256);	// Tag is stored in a U8 so must be 255 or less
		m_tag = tag;
	}

	void RegisterTracker(Tracker *tracker);

							/// find an existing node index, given a string
							/// retrurns INVALID_CONTEXT_ID if the node was not found
	ContextId				FindId(const Util::C_String &string);
							/// create a new node and it's children, returns the id of the new node
	ContextId				GetNode(const Util::C_String &string, bool bMutex = true);
							/// generate parent name from a given context name
	static Util::C_String	GenerateParentName(const Util::C_String &name);
};

struct MemoryLeakInfo
{
	size_t	frontendLeakSystem;
	size_t	frontendLeakPhysical;
	U32		frontendAllocationCount;
	size_t	gameLeakSystem;
	size_t	gameLeakPhysical;
	U32		gameAllocationCount;
	U32		physicalMemoryDifference;
	U32		totalMemoryDifference;
	bool	frontendNoAllocs;
	bool	gameNoAllocs;
};

/// associate a context hierarchy node (e.g. 'oldtech.physics.car') with a unique ID
class Tracker
{
public:
							/// constructor
							Tracker();
							/// destructor
							~Tracker();

	void					TrackAllocation(ContextId id, size_t iSize, size_t iWasted);
	void					TrackDeallocation(ContextId id, size_t iSize, size_t iWasted);

	bool					Initialised() const
	{
		return m_bInitialized;
	}

							/// register an allocation with the tracker
							/// pointer to an AllocationNode structure is to be supplied by the caller
	void					TrackAllocation(void *pAddress, size_t iOrigSize, size_t iAllocSize, E_MEMORY_TYPES memoryType, AllocationNode *pNode, int callstackOverhead=0);
							/// register a deallocation with the tracker
							/// after this call, the supplied AllocationNode structure can be freed
	void					TrackDeallocation(int iSize, E_MEMORY_TYPES memoryType, AllocationNode *pNode);

	void					TrackHeapAllocation(void *pAddress, size_t iSize, E_MEMORY_TYPES memoryType);
	void					TrackHeapDeallocation(void *pAddress, size_t iSize, E_MEMORY_TYPES memoryType);

	bool					RememberAllocNodeForVirPhys(AllocationNode *pNode, void *ptr);
	AllocationNode*			ForgetAllocNodeForVirPhys(void *ptr);
	void					CompressMemoryUsage();

							/// get current size of tracked memory
	size_t					TotalMemory() const
	{
		return m_systemMemoryNode.Allocated() + m_physicalMemoryNode.Allocated();
	}

							/// get current size of tracked memory
	size_t					SystemMemory() const
	{
		return m_systemMemoryNode.Allocated();
	}

							/// get current size of tracked memory
	size_t					PhysicalMemory() const
	{
		return m_physicalMemoryNode.Allocated();
	}

							/// get current size of wasted tracked memory
	size_t					SystemMemoryWasted() const
	{
		return m_systemMemoryNode.m_iWasted;
	}

							/// get current size of wasted tracked memory
	size_t					PhysicalMemoryWasted() const
	{
		return m_physicalMemoryNode.m_iWasted;
	}

							/// get current number of system memory allocations
	size_t					SystemMemoryAllocations() const
	{
		return m_systemMemoryNode.m_nAllocations;
	}

							/// get current number of physical memory allocations
	size_t					PhysicalMemoryAllocations() const
	{
		return m_physicalMemoryNode.m_nAllocations;
	}
	
							/// retrieve the context structure for a given Id
	const ContextNode&		GetContextNode(ContextId id) const
	{
		return m_Node[id];
	}

							/// retrieve the values for memory usage for a given id (can work across sub-heaps)
	void					GetContextNode(ContextId id, ContextNode& node, bool includeChildTrackers=true) const;

	unsigned int			TrackingStructureSize() const
	{
		return m_trackerStructSize;
	}

	unsigned int			TrackingAlignedStructureSize() const
	{
		return m_trackerAlignedStructSize;
	}

							/// get first allocation node (from a linked list)
	AllocationNode			*GetFirstAllocationNode() const;

	void					Initialize(const Util::C_String& name, int callStackSkip, unsigned int flags,  bool mainTracker);
	void					Initialize(const Util::C_String& name, Tracker& clone, size_t bufferSize);

	void					Update();

							/// dump information about all allocated blocks to the debug logging system
							/// bUseDebugLoggingSystem flag is provided in case debug logging system has already been freed
	void					DumpAllocatedBlocks(int iTag = -1);

							/// dump memory stats to the debug logging system
							/// bUseDebugLoggingSystem flag is provided in case debug logging system has already been freed
	void					DumpNodeStats(bool bCSV, bool includeChildren);

							// Dump memory stats to a file with as little memory overhead as possible
	void					DumpMemoryRaw(const char* filename);

							/// compares the information stored in nodes vs allocations to find discrepencies
	void					DebugTagAndNode(bool includeChildren);

							/// dump tag stats to the debug logging system
	void					DumpTagStats(U32 exclusionCount, U32* exclusionList, bool includeChildren);

	int						CheckForMemoryLeaks(U32 exclusionCount, U32* exclusionList, MemoryLeakInfo& info);

	void					SendAllocationList(int version, bool justSuspectedLeaks, ArteryConnection con, bool bSingleMessage);
	void					SendContextNodeList(ArteryMessageBuilder &b);

							/// add the names of all tracking nodes to a vector of strings
	void					GetContextNodeList(Ut::Vector<Util::C_String> &list);

							/// save mtune memory profiling data file
	void					SaveMTuneFile(const Util::C_String &filename);
	
							/// set whether the memory tracker will call DumpNodeStats when it is destroyed
	void					DumpStatsOnExit(bool bOn);

							/// set whether the memory tracker will call DumpAllocatedBlocks when it is destroyed
	void					DumpBlocksOnExit(bool bOn);

							/// set output callback function
							/// pass in NULL to remove the callback function
	void					SetOutputCallBackFunction(OutputCallBackFunction *pFunc);

							// Tell all the tracker's about node changes (like parental links)
	void					UpdateContext(unsigned int m_nNodes, int* parentInfo);

	static Tracker& Instance(void)
	{
		static Tracker s_instance;

		return s_instance;
	}

	void					AddSubSystem(Tracker* tracker);

	void					RemoveSubSystem(Tracker* tracker);

	U32						GetNumSubTrackers() const {return m_subTrackers.Size();}
	const Tracker*			GetSubTracker(const U32 index) const {return m_subTrackers[index];}

	const Util::C_String&	GetName() const {return m_name;}
	size_t					GetBufferSize() const {return m_bufferSize;}

	MT::Lock&				GetCriticalSection() { return m_Lock; }

protected:
	void					UpdateMemory(ContextId id, int iAllocated, int iWasted, int nAllocations, E_MEMORY_TYPES memoryType);

	void					RemoveAllocationFromStatsAndList(int iAllocSize, E_MEMORY_TYPES memoryType, AllocationNode *pNode);

public:
	Tracker*				m_pNext;	// We create a list of all tracker's in use so we can notify them all of hierachy changes

private:

	typedef struct
	{
		size_t	systemSize;
		size_t	physicalSize;
		size_t	numAllocs;
	} TagUsageInfo;

	struct SubHeapTrack
	{
		void*		pAddress;
		void*		m_pCallStack;

		U32			m_size;

		ContextId	m_contextId;
		U8			m_iTag;
		U8			m_type;
	};

							/// print a string
	void					Output(char *psz, ...);

	void					DumpTagStatsSub(U32 exclusionCount, const U32* exclusionList, TagUsageInfo* tagUsage, const AllocationNode *nodeIterator) const;

	void					UpdateTagUsageFromAllocs(U32 exclusionCount, U32* exclusionList, TagUsageInfo* tagUsage) const;

	void					SendAllocs(int version, int heapId, ArteryConnection con, bool justSuspectedLeaks, bool bSingleMessage);

	typedef Btl::ChainedHashMap<void*, Memory::AllocationNode*, Btl::HashKeyOpPtr, Btl::DynamicPool<> > HashVirPhysicalMap;

	HashVirPhysicalMap				m_hashVirPhysical;

	Btl::Vector<Tracker*>			m_subTrackers;
	Btl::Vector<SubHeapTrack>		m_subTrackNodes;

	ContextNode				m_systemMemoryNode;
	ContextNode				m_physicalMemoryNode;

	ContextNode				m_Node[TrackNodes::Enum_Max_ContextNodes];
	
	MT::Lock				m_Lock;

	AllocationNode			m_root;

	unsigned int			m_iFlags;
	unsigned int			m_trackerStructSize;
	unsigned int			m_trackerAlignedStructSize;
	int						m_callStackSkip;				// Number of entries off the top of the callstack to remove when capturing

	unsigned int			m_allocationsSinceLastUpdate;

	OutputCallBackFunction	*m_pOutputCallBackFunction;

	Util::C_String			m_name;

	Debug::Bool*			m_pDumpStatsBool;
	Debug::Bool*			m_pDumpTagNode;

	size_t					m_bufferSize;

	bool					m_bInitialized;
	bool					m_mainTracker;
	bool					m_bDumpStatsOnExit;
	bool					m_bDumpBlocksOnExit;
};

class MemoryTrackerContext
{
protected:
#ifdef CFG_DEBUG
	ContextId	m_id;
#endif

public:
	MemoryTrackerContext(const char* name)
	{
		ContextId id = ::Memory::TrackNodes::Instance().GetId(name);

#ifdef CFG_DEBUG
		m_id = id;
#endif
	
		::Memory::TrackNodes::Instance().Push(id);
	}

	MemoryTrackerContext(ContextId id)
	{
#ifdef CFG_DEBUG
		m_id = id;
#endif

		::Memory::TrackNodes::Instance().Push(id);
	}

	~MemoryTrackerContext()
	{
#ifdef CFG_DEBUG
		::Memory::TrackNodes::Instance().Pop(m_id);
#else
		::Memory::TrackNodes::Instance().Pop();
#endif
	}
};

class CallstackTrackerImpl;

class CallstackTracker
{
protected:
	CallstackTrackerImpl	*m_impl;

public:
	CallstackTracker();
	~CallstackTracker();

	void Add(void* ptr);
	void Remove(void* ptr);

	void Dump() const;
};

int InitialiseNetworkConnection(void);

void Update(void);

void DumpHashVirPhysicalStats();

void DumpSmallBlockStats();

};

#endif
