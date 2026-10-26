#ifndef UTIL_RESOURCE_MANAGER_H
#define UTIL_RESOURCE_MANAGER_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\Core.h>

#include <core/threads/Thread.h>
#include <btl/containers/vector.h>
#include <btl/Containers/set.h>
#include <system/serialise.h>

#include <core/threads/Event.h>
#include <core/threads/Mutex.h>
#include <core/threads/AtomicMutex.h>

#define REMOVE_BLOCKING_RESOURCE_MUTEX

//! @addtogroup Resource
//! @{

namespace LogicalFs
{
	class C_MemoryFs;
}

namespace Resource
{

enum E_BackgroundLoading
{
	E_BackgroundLoading_Disabled,
	E_BackgroundLoading_Enabled
};

bool IsInLoadQueue( const E_State state );
bool IsInEvictList( const E_State state );

class C_ResourceBase;
class C_ResourceInfo;

//! C_HandleList
typedef Btl::Vector<C_HandleBase> C_HandleList;

// C_ThreadPause
class C_ThreadPause
{
public:
	C_ThreadPause();

	~C_ThreadPause();

	void		Pause();

	void		Resume();

	void		WaitIfPaused();

private:
	MT::Mutex	m_Mutex;
};

// C_LoadingThread
class C_LoadingThread : public MT::Thread, public C_ThreadPause
{
public:
	C_LoadingThread();
	
	~C_LoadingThread();

    void        ThreadMain();

	void		SendQuitMessage();

	bool		ShouldQuit();

private:
	MT::Event		m_EventSignalQuit;	
};

// C_EvictItem
class C_EvictItem
{
public:
	C_EvictItem()
		: m_pResource( NULL )
		, m_EvictTime( 0 )
	{
	}

	C_EvictItem( C_ResourceBase* pResource, const S32 evictTime)
		: m_pResource( pResource )
		, m_EvictTime( evictTime )
	{
	}

	~C_EvictItem()
	{
	}

public:
	C_ResourceBase* m_pResource;
	U32				m_EvictTime;
};

// C_ResourceItem. This is what the resource list (set) is a list of.
class C_ResourceItem
{
public:
	C_ResourceItem();
	C_ResourceItem( C_ResourceInfo* pResource );
	~C_ResourceItem();

	const C_ResourceInfo* GetResource() const;
	C_ResourceInfo* GetResource();

	bool operator < ( const C_ResourceItem& other ) const;
	bool operator == ( const C_ResourceItem& other ) const;

private:
	C_ResourceInfo* m_pResource;
};

//! C_Manager. Holds the list of active resources, evict list and load queue.
//! Resource state management is performed by this class.
class C_Manager
{
	template < class T_Resource > 
	friend class C_Handle;
	friend class C_HandleBase;
	friend class C_ResourceBase;
	friend class C_LoadingThread;
	friend class C_Sender;
	friend class C_Listener;

	typedef C_ResourceBase* C_PostLoadingItem;
	typedef C_ResourceBase* C_PreDeletingItem;

	typedef Btl::Set<C_ResourceItem> C_ResourceListType;
	typedef Btl::Set<C_ResourceItem> C_LoadQueueType;
	typedef Btl::Vector<C_EvictItem> C_EvictListType;
	typedef Btl::Vector<C_PostLoadingItem> C_PostLoadingListType;
	typedef Btl::Vector<C_PreDeletingItem> C_PreDeletingListType;

public:
	//! Default Constructor
	C_Manager();

	//! Destructor
	~C_Manager();

	//! This class is not a singleton although only one instance should be created at once. 
	//! GetInstance can be used to return the currently created instance of the class. 
	//! If the class has not been created GetInstance will return NULL.
	static C_Manager* GetInstance();

	//! This is used to override the filesystem used by the resource manager.
	void SetFileSystems( Sys::I_FileSystem* pFileSystem, LogicalFs::C_MemoryFs* pMemoryFs );

	//! This is called each time round the game loop. The C_ResourceBase evict delays are in units of calls to this.
	bool Update();

	//! Pauses the loading thread.
	bool PauseLoadingThread();

	//! Resumes the loading thread.
	bool ResumeLoadingThread();

	void SetBackgroundLoadingMode(E_BackgroundLoading mode);

	E_BackgroundLoading GetBackgroundLoadingMode() const;

	bool GetBackgroundLoadingBuffer(U8** ppData, U32* pSize);

	//! Get vector of handles to all resources of type 'desiredTypeId' which are in state 'desiredState'.
	//! if desiredTypeId = -1 any type will match. If desiredState = E_State_Unknown, any state will match.
	void GetHandleList( C_HandleList& resHandleList, const C_ResourceTypeId desiredTypeId = -1, E_State desiredState = E_State_Ready );

	void Dump();
	void DumpResourceList();
	void DumpEvictList();
	void DumpLoadQueue();

	#ifdef CFG_DEBUG
	bool Validate();
	#else
	inline bool Validate() { return true; }
	#endif

	S32 GetPendingResourceCount();
	void WaitForAllResourceData();
	void PartialUpdate();
	S32 GetLoadQueueSize() const;

#ifndef CFG_GOLD
	typedef void (*ForEachCallback)(const C_ResourceBase*);

	static void DebugDump(void*);
	void DebugForEach(ForEachCallback);			// Allow us to have a callback function called for each object (with a pointer to that object)
#endif

private:
	// find a resource with the same filename or create a new resource with this name
	C_ResourceBase* GetResource( const Sys::C_FilePath	& filename, const C_ResourceTypeId typeId );

	template<typename T_Resource>
	T_Resource* GetResource( const Sys::C_FilePath	& filename );

	// returns a C_ResourceBase which has this filename, returns NULL if not found
	C_ResourceBase* FindResource( const Sys::C_FilePath& filename );

	E_State ChangeState( C_ResourceBase* pResource, const E_State newState );
	void ChangeStateUpdateLists( C_ResourceBase* pResource, const E_State oldState, const E_State newState );
	void UpdateNotify( C_ResourceBase* pResource );

	bool Load( C_ResourceBase* pResource );
	void Evict( C_ResourceBase* pResource );

	void AddToResourceList( C_ResourceBase* pResource );
	void RemoveFromResourceList( C_ResourceBase* pResource );

	void AddToEvictList( C_ResourceBase* pResource );
	void RemoveFromEvictList( C_ResourceBase* pResource );

	void AddToLoadQueue( C_ResourceBase* pResource );
	void RemoveFromLoadQueue( C_ResourceBase* pResource );

	void AddToPostLoadingList( C_ResourceBase* pResource );
	void RemoveFromPostLoadingList( C_ResourceBase* pResource );

	void AddToPreDeletingList( C_ResourceBase* pResource );
	void RemoveFromPreDeletingList( C_ResourceBase* pResource );

	void SetStatusDirty();
	void WaitForStatusDirty();

	// Called on the resource thread
	S32 UpdateEvictList( const S32 maxEvictCount = -1);
	// Called on the resource thread
	S32 UpdateLoadQueue( const S32 maxResourceLoads = -1);

	// Called on the main thread 
	S32 UpdatePostLoadingList( const S32 maxCreateCount = -1);
	// Called on the main thread 
	S32 UpdatePreDeletingList( const S32 maxDeleteCount = -1);

	bool StartLoadingThread();
	bool StopLoadingThread();

	void MutexWait();
	void MutexRelease();

	void ReleaseBackgroundLoadingBuffer();

public:
#ifdef REMOVE_BLOCKING_RESOURCE_MUTEX
	inline void BlockingMutexWait() {}
	inline void BlockingMutexRelease() {}
	inline void AllowBlockingMutex( bool b ) {}
#else
	inline void BlockingMutexWait() { if(m_AllowBlockingMutex) { m_BlockingMutexToFixAmaxCrash.Wait(); } }
	inline void BlockingMutexRelease() { if(m_AllowBlockingMutex) { m_BlockingMutexToFixAmaxCrash.Release(); } }
	inline void AllowBlockingMutex( bool b ) { m_AllowBlockingMutex = b; }
#endif

private:
	Sys::I_FileSystem* m_pFileSystem;
	LogicalFs::C_MemoryFs* m_pMemoryFs;

	MT::AtomicLock m_Mutex;
	MT::Event m_StatusDirtyEvent;

	C_ResourceListType m_ResourceList;
	C_EvictListType m_EvictList;
	C_LoadQueueType m_LoadQueue;
	C_PostLoadingListType m_PostLoadingList;
	C_PreDeletingListType m_PreDeletingList;

	U32 m_EvictTimer;

	C_LoadingThread m_LoadingThread;

	#ifndef REMOVE_BLOCKING_RESOURCE_MUTEX
	MT::Mutex m_BlockingMutexToFixAmaxCrash;
	bool m_AllowBlockingMutex;
	#endif

	MT::ThreadId m_hMainThreadId;	// main thread id stored

	E_BackgroundLoading m_backgroundLoading;
	U8* m_pBackgroundLoadingBuffer;
	U32 m_backgroundLoadingBufferSize;
	U64 m_startPauseTime;
	S32 m_startLoadQueueSize;
};

} // namespace Resource

//! @}

#endif
