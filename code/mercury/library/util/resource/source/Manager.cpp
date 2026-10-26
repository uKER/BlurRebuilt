
#include "stdafx.h"
#include <system/time/scopedtimer.h>

#include <resource/Resource.h>

#include <btl/containers/map.h>
#include <btl/containers/vector.h>
#include <core/containers/genesis_containers/Sort.h>
#include <system/debug/Variables.h>
#include <system/misc/StringUtil.h>
#include <system/memory/MemoryTracker.h>

#ifndef CFG_GOLD
#define RESOURCE_MANAGER_CHECK_FOR_POST_LOAD_STALLS
#endif

#ifdef RESOURCE_MANAGER_CHECK_FOR_POST_LOAD_STALLS
#include <system/time/Time.h>
#endif

#ifndef PLATFORM_PC
// #define REMOVE_BLOCKING_RESOURCE_MUTEX
#endif

#ifdef PLATFORM_PS3
#include <sys/timer.h>
#endif

#ifndef CFG_GOLD
Debug::Function	g_DebugDumpResourceList("system.resource.DebugDump", Resource::C_Manager::DebugDump, NULL);
#endif


namespace Resource
{

C_Manager* g_pManager = NULL;

bool IsInLoadQueue( const E_State state )
{
	switch( state )
	{
		case E_State_InLoadQueue:
			return true;
		break;

		default:
		break;
	}
	
	return false;
}

bool IsInEvictList( const E_State state )
{
	switch( state )
	{
		case E_State_Evicting:
			return true;
		break;

		default:
		break;
	}

	return false;
}

bool IsInResourceList( const E_State state )
{
	switch( state )
	{
		case E_State_Unknown:
		case E_State_PreDeleting:
		case E_State_Deleted:
			return false;
		break;

		default:
		break;
	}

	return true;
}

bool IsInPostLoadingList( const E_State state )
{
	switch( state )
	{
		case E_State_PostLoading:
			return true;
		break;

		default:
		break;
	}

	return false;
}

bool IsInPreDeletingList( const E_State state )
{
	switch( state )
	{
		case E_State_PreDeleting:
			return true;
		break;

		default:
		break;
	}

	return false;
}

// C_ThreadPause

C_ThreadPause::C_ThreadPause()
	: m_Mutex( "MutexThreadPause" )
{
}

C_ThreadPause::~C_ThreadPause()
{
}

void C_ThreadPause::Pause()
{
	//if(m_Mutex.Wait(0))
	//{
	//	m_Mutex.Release();
	//}
	//else
	//{
	//	LOGWARN(g_Log, "Pausing resource thread when already paused");
	//}
	
	m_Mutex.Wait();
}

void C_ThreadPause::Resume()
{
	m_Mutex.Release();
}

void C_ThreadPause::WaitIfPaused()
{
	m_Mutex.Wait();
	m_Mutex.Release();
}

// C_LoadingThread

C_LoadingThread::C_LoadingThread()
	: m_EventSignalQuit("EventSignalQuit")
{
}

C_LoadingThread::~C_LoadingThread()
{
}

void C_LoadingThread::ThreadMain()
{

	C_Manager* pManager = C_Manager::GetInstance();
	
	pManager->SetStatusDirty();

	while(!ShouldQuit())
	{
		WaitIfPaused();

		const S32 evictCount = pManager->UpdateEvictList(1);
		const S32 loadCount = pManager->UpdateLoadQueue(1);

		bool nothingToProcess = ((evictCount == 0) && (loadCount == 0));

		if(nothingToProcess)
		{
			pManager->WaitForStatusDirty();
		}
	}
}

bool C_LoadingThread::ShouldQuit()
{
	return m_EventSignalQuit.Wait(0);
}

void C_LoadingThread::SendQuitMessage()
{
	m_EventSignalQuit.Signal();
}

// C_ResourceItem

C_ResourceItem::C_ResourceItem()
	: m_pResource( NULL )
{
}

C_ResourceItem::C_ResourceItem( C_ResourceInfo* pResource )
	: m_pResource( pResource )
{
}

C_ResourceItem::~C_ResourceItem()
{
	m_pResource = NULL;
}

const C_ResourceInfo* C_ResourceItem::GetResource() const
{
	return m_pResource;
}

C_ResourceInfo* C_ResourceItem::GetResource()
{
	return m_pResource;
}

bool C_ResourceItem::operator < ( const C_ResourceItem& other ) const
{
	if(GetResource()->GetResourceFilename() < other.GetResource()->GetResourceFilename())
		return true;
	if((S32)GetResource()->IsDataOld() < (S32)other.GetResource()->IsDataOld())
		return true;
	return false;
}

bool C_ResourceItem::operator == ( const C_ResourceItem& other ) const
{
	if(GetResource()->GetResourceFilename() != other.GetResource()->GetResourceFilename())
		return false;

	if(GetResource()->IsDataOld() != other.GetResource()->IsDataOld())
		return false;

	return true;
}


// C_Manager

C_Manager::C_Manager()
: m_StatusDirtyEvent("StatusDirtyEvent")
#ifndef REMOVE_BLOCKING_RESOURCE_MUTEX
, m_AllowBlockingMutex(true)
#endif
{
	m_pFileSystem = &Sys::C_FileSystem::Instance();
	m_pMemoryFs = NULL;

	LOGDEBUG( g_Log, "Resource::C_Manager::C_Manager()\n" );
	assert( !g_pManager && "Resource Manager Already Created" );
	g_pManager = this;

	StartLoadingThread();

	// main thread, get mutex
	BlockingMutexWait();

	m_EvictTimer = 0;

	// get main thread id
	m_hMainThreadId = MT::GetThreadId();

	m_backgroundLoading = E_BackgroundLoading_Disabled;
	m_backgroundLoadingBufferSize = 0;
	m_pBackgroundLoadingBuffer = NULL;
	m_startPauseTime = 0;
	m_startLoadQueueSize = 0;
}

C_Manager::~C_Manager()
{
	LOGDEBUG( g_Log, "Resource::C_Manager::~C_Manager()\n" );

	StopLoadingThread();

	assert( g_pManager == this );

	Dump();

	// delete any resources that are still allocated
	for( C_ResourceListType::Iterator i = m_ResourceList.Begin(); i !=m_ResourceList.End(); i = m_ResourceList.Begin() )
	{
		assert( 0 && "Resources still allocated" );
		//C_ResourceBase* pResource = &(*i);
		C_ResourceBase* pResource = (C_ResourceBase*)i->GetValue().GetResource();
		ChangeState( pResource, E_State_Deleted );
	}

	g_pManager = NULL;

	ReleaseBackgroundLoadingBuffer();
}

C_ResourceBase* C_Manager::FindResource( const Sys::C_FilePath& filename )
{
	C_ResourceInfo tempResourceInfo( filename );
	C_ResourceListType::Iterator iFound = m_ResourceList.Find( C_ResourceItem( &tempResourceInfo ) );
	if( iFound != m_ResourceList.End() )
	{
		return (C_ResourceBase*)iFound->GetValue().GetResource();
	}

	return NULL;
}

C_Manager* C_Manager::GetInstance()
{
	return g_pManager;
}

void C_Manager::SetFileSystems( Sys::I_FileSystem* pFileSystem, LogicalFs::C_MemoryFs* pMemoryFs)
{
	m_pFileSystem = pFileSystem;
	m_pMemoryFs = pMemoryFs;
}

void C_Manager::Dump()
{
	MutexWait();
	LOGDEBUG( g_Log, "Resource::C_Manager::Dump():\n" );

	DumpResourceList();
	DumpEvictList();
	DumpLoadQueue();

	LOGDEBUG( g_Log, "" );
	MutexRelease();
}

void C_Manager::DumpResourceList()
{
	MutexWait();

	LOGDEBUG( g_Log, " Resource List[%d]:\n", m_ResourceList.Size() );

	for( C_ResourceListType::Iterator i = m_ResourceList.Begin(); i!=m_ResourceList.End(); ++i)
	{
		//C_ResourceBase& pCurrResource = *i;
		C_ResourceBase& pCurrResource = *((C_ResourceBase*)i->GetValue().GetResource());
		pCurrResource.DumpResourceInfo();
	}

	MutexRelease();
}

void C_Manager::DumpEvictList()
{
	MutexWait();

	LOGDEBUG( g_Log, " Resource EvictList[%d]:\n", m_EvictList.Size() );

	for( C_EvictListType::Iterator i = m_EvictList.Begin(); i!=m_EvictList.End(); ++i)
	{
		C_EvictItem& evictItem = *i;
		C_ResourceBase* pCurrResource = evictItem.m_pResource;
		pCurrResource->DumpResourceInfo();
	}

	MutexRelease();
}

void C_Manager::DumpLoadQueue()
{
	MutexWait();

	LOGDEBUG( g_Log, " Resource Load Queue[%d]:\n", m_LoadQueue.Size() );

	for( C_LoadQueueType::Iterator i = m_LoadQueue.Begin(); i!=m_LoadQueue.End(); ++i)
	{
		C_ResourceBase* pCurrResource = (C_ResourceBase*)i->GetValue().GetResource();
		pCurrResource->DumpResourceInfo();
	}

	MutexRelease();
}

void C_Manager::AddToResourceList( C_ResourceBase* pResource )
{
	LOGDEBUG( g_Log, "AddToResourceList %s\n", pResource->GetResourceFilename().Chars() );
	m_ResourceList.Insert( C_ResourceItem(pResource) );
}

void C_Manager::RemoveFromResourceList( C_ResourceBase* pResource )
{
	LOGDEBUG( g_Log, "RemoveFromResourceList %s\n", pResource->GetResourceFilename().Chars() );
	const S32 erasedCount = m_ResourceList.Erase( C_ResourceItem(pResource) );
	assert( erasedCount == 1 );
}

void C_Manager::AddToEvictList( C_ResourceBase* pResource )
{
	SetStatusDirty();

	for( C_EvictListType::Iterator i = m_EvictList.Begin(); i!=m_EvictList.End(); ++i)
	{
		C_EvictItem& evictItem = *i;
		C_ResourceBase* pCurrResource = evictItem.m_pResource;
		if( pCurrResource == pResource )
		{
			// already in evict list
			LOGWARN( g_Log, "C_Manager::AddToEvictList( C_ResourceBase* pResource ) - Resource already in evict list\n");
			return;
		}
	}

	m_EvictList.PushBack( C_EvictItem(pResource, m_EvictTimer + pResource->GetEvictDelay()) );
}

void C_Manager::RemoveFromEvictList( C_ResourceBase* pResource )
{
	SetStatusDirty();

	for( C_EvictListType::Iterator i = m_EvictList.Begin(); i!=m_EvictList.End(); ++i)
	{
		C_EvictItem& evictItem = *i;
		C_ResourceBase* pCurrResource = evictItem.m_pResource;
		if( pCurrResource == pResource )
		{
			m_EvictList.Erase(i);
			return;
		}
	}

	assert(0);
}

void C_Manager::AddToLoadQueue( C_ResourceBase* pResource )
{
	SetStatusDirty();

	C_LoadQueueType& list = C_Manager::GetInstance()->m_LoadQueue;
	list.Insert( C_ResourceItem( pResource ) );
}

void C_Manager::RemoveFromLoadQueue( C_ResourceBase* pResource )
{
	SetStatusDirty();

	C_LoadQueueType& list = C_Manager::GetInstance()->m_LoadQueue;
	list.Erase( C_ResourceItem( pResource ) );
}


void C_Manager::AddToPostLoadingList( C_ResourceBase* pResource )
{
	SetStatusDirty();

	for( C_PostLoadingListType::Iterator i = m_PostLoadingList.Begin(); i!=m_PostLoadingList.End(); ++i)
	{
		C_PostLoadingItem& postLoadingItem = *i;
		C_ResourceBase* pCurrResource = postLoadingItem;
		if( pCurrResource == pResource )
		{
			// already in evict list
			LOGWARN( g_Log, "C_Manager::AddToPostLoadingList( C_ResourceBase* pResource ) - Resource already in post loading list\n");
			return;
		}
	}

	m_PostLoadingList.PushBack( pResource );
}

void C_Manager::RemoveFromPostLoadingList( C_ResourceBase* pResource )
{
	SetStatusDirty();

	for( C_PostLoadingListType::Iterator i = m_PostLoadingList.Begin(); i!=m_PostLoadingList.End(); ++i)
	{
		C_PostLoadingItem& postLoadingItem = *i;
		C_ResourceBase* pCurrResource = postLoadingItem;
		if( pCurrResource == pResource )
		{
			m_PostLoadingList.Erase(i);
			return;
		}
	}

	assert(0);
}


void C_Manager::AddToPreDeletingList( C_ResourceBase* pResource )
{
	SetStatusDirty();

	for( C_PreDeletingListType::Iterator i = m_PreDeletingList.Begin(); i!=m_PreDeletingList.End(); ++i)
	{
		C_PreDeletingItem& preDeletingItem = *i;
		C_ResourceBase* pCurrResource = preDeletingItem;
		if( pCurrResource == pResource )
		{
			// already in evict list
			LOGWARN( g_Log, "C_Manager::AddToPreDeletingList( C_ResourceBase* pResource ) - Resource already in pre deleting list\n");
			return;
		}
	}

	m_PreDeletingList.PushBack( pResource );
}

void C_Manager::RemoveFromPreDeletingList( C_ResourceBase* pResource )
{
	SetStatusDirty();

	for( C_PreDeletingListType::Iterator i = m_PreDeletingList.Begin(); i!=m_PreDeletingList.End(); ++i)
	{
		C_PreDeletingItem& preDeletingItem = *i;
		C_ResourceBase* pCurrResource = preDeletingItem;
		if( pCurrResource == pResource )
		{
			m_PreDeletingList.Erase(i);
			return;
		}
	}

	assert(0);
}

void C_Manager::ChangeStateUpdateLists( C_ResourceBase* pResource, const E_State oldState, const E_State newState )
{
	// Update Load Queue
	{
		const bool oldIsInLoadQueue = IsInLoadQueue( oldState );
		const bool newIsInLoadQueue = IsInLoadQueue( newState );

		if(oldIsInLoadQueue && !newIsInLoadQueue)
		{
			RemoveFromLoadQueue( pResource );
		}
		else
		if(!oldIsInLoadQueue && newIsInLoadQueue)
		{
			AddToLoadQueue( pResource );
		}
	}

	// Update Evict List
	{
		const bool oldIsInEvictList = IsInEvictList( oldState );
		const bool newIsInEvictList = IsInEvictList( newState );

		if(oldIsInEvictList && !newIsInEvictList)
		{
			RemoveFromEvictList( pResource );
		}
		else
		if(!oldIsInEvictList && newIsInEvictList)
		{
			AddToEvictList( pResource );
		}
	}

	// Update Resource List
	{
		const bool oldIsInResourceList = IsInResourceList( oldState );
		const bool newIsInResourceList = IsInResourceList( newState );

		if(oldIsInResourceList && !newIsInResourceList)
		{
			RemoveFromResourceList( pResource );
		}
		else
		if(!oldIsInResourceList && newIsInResourceList)
		{
			AddToResourceList( pResource );
		}
	}

	// Update Post Loading List
	{
		const bool oldIsInPostLoadingList = IsInPostLoadingList( oldState );
		const bool newIsInPostLoadingList = IsInPostLoadingList( newState );

		if(oldIsInPostLoadingList && !newIsInPostLoadingList)
		{
			RemoveFromPostLoadingList( pResource );
		}
		else
		if(!oldIsInPostLoadingList && newIsInPostLoadingList)
		{
			AddToPostLoadingList( pResource );
		}
	}

	// Update Pre Deleting List
	{
		const bool oldIsInPreDeletingList = IsInPreDeletingList( oldState );
		const bool newIsInPreDeletingList = IsInPreDeletingList( newState );

		if(oldIsInPreDeletingList && !newIsInPreDeletingList)
		{
			RemoveFromPreDeletingList( pResource );
		}
		else
		if(!oldIsInPreDeletingList && newIsInPreDeletingList)
		{
			AddToPreDeletingList( pResource );
		}
	}
}

E_State C_Manager::ChangeState( C_ResourceBase* pResource, const E_State newState )
{
	const E_State oldState = pResource->GetState();
	
	if(newState == oldState)
		return newState;

	// If evict delay is zero, change state to deleted instead of evicting
	if(newState == E_State_Evicting)
	{
		const S32 evictDelay = pResource->GetEvictDelay();
		if( evictDelay < 0 )
		{			
			return ChangeState(pResource, E_State_PreDeleting);
		}
	}

	//LOGDEBUG( g_Log, "C_ResourceBase '%s' ChangeState()-> %s", GetResourceFilename().Chars(), GetStateName(newState).Chars());

	pResource->SetState( newState );

	ChangeStateUpdateLists( pResource, oldState, newState );
	
	if(newState == E_State_Deleted)
	{
		BlockingMutexWait();		
		pResource->FreeResourceData();
		BlockingMutexRelease();

		C_TypeRegistry::GetInstance()->DestroyResource( pResource );
	}

	return newState;
}

void C_Manager::UpdateNotify( C_ResourceBase* pResource )
{
	E_Notify oldNotify = pResource->GetLastNotify();
	E_Notify newNotify = pResource->GetNotifyType();

	if( oldNotify != newNotify )
	{
		pResource->SendResourceNotify( pResource, newNotify );
		pResource->SetLastNotify( newNotify );
	}
}

bool C_Manager::Load( C_ResourceBase* pResource )
{
	switch( pResource->GetState() )
	{
		case E_State_Constructed:
			ChangeState( pResource, E_State_InLoadQueue );
		break;
		case E_State_InLoadQueue:
			ChangeState( pResource, E_State_InLoadQueue );
		break;
		case E_State_Loading:
			pResource->SetLoadCancelled( false );
		break;
		case E_State_PostLoading:
			pResource->SetLoadCancelled( false );
		break;
		case E_State_Ready:
			ChangeState( pResource, E_State_Ready );
		break;
		case E_State_Evicting:
			ChangeState( pResource, E_State_Ready );
		break;
	}

	return true;
}

void C_Manager::Evict( C_ResourceBase* pResource )
{
	switch( pResource->GetState() )
	{
		case E_State_Constructed:
			ChangeState( pResource, E_State_Deleted );
		break;

		case E_State_InLoadQueue:
			ChangeState( pResource, E_State_Deleted );
		break;

		case E_State_Loading:
			pResource->SetLoadCancelled( true );
		break;

		case E_State_PostLoading:
			pResource->SetLoadCancelled( true );
		break;

		case E_State_Ready:
			ChangeState( pResource, E_State_Evicting );
		break;

		case E_State_Evicting:
		break;
	}
}

bool C_Manager::Update()
{
	m_EvictTimer++;

	// main thread, release mutex
	BlockingMutexRelease();

	SetStatusDirty();

	UpdatePostLoadingList();
	UpdatePreDeletingList();

	// main thread, get mutex
	BlockingMutexWait();

	return true;
}

void C_Manager::PartialUpdate()
{
	MT::ThreadId hCurrentThread = MT::GetThreadId();
	if(m_hMainThreadId != hCurrentThread)
		return;

	// main thread, release mutex
	BlockingMutexRelease();

	UpdatePostLoadingList();

	UpdatePreDeletingList();
	// main thread, get mutex
	BlockingMutexWait();
}

S32 C_Manager::GetLoadQueueSize() const
{
	return m_LoadQueue.Size();
}

void C_Manager::SetStatusDirty()
{
	m_StatusDirtyEvent.Signal();
}

void C_Manager::WaitForStatusDirty()
{
	m_StatusDirtyEvent.Wait();
}

S32 C_Manager::UpdateEvictList( const S32 maxEvictCount )
{
	//LOGDEBUG( g_Log, "C_Manager::UpdateEvictList() - BEGIN" );

	MutexWait();

	S32 evictCount = 0;

	bool done = false;
	S32 i=0;

	while	( 
			(i<m_EvictList.Size()) && 
			((maxEvictCount<0) || (evictCount < maxEvictCount)) 
		)
	{
		C_EvictItem& evictItem = m_EvictList[i];
		C_ResourceBase* pCurrResource = evictItem.m_pResource;

		bool evicted = false;

		if( pCurrResource->GetState() == E_State_Evicting )
		{
			if(m_EvictTimer >= evictItem.m_EvictTime)
			{
				ChangeState( pCurrResource, E_State_PreDeleting );
				evicted = true;
				evictCount++;
			}			
		}

		if(!evicted)
		{
			i++;
		}
	}

	MutexRelease();

	//LOGDEBUG( g_Log, "C_Manager::UpdateEvictList() - END" );

	return evictCount;
}

S32 C_Manager::UpdateLoadQueue( const S32 maxResourceLoads )
{
	//LOGDEBUG( g_Log, "C_Manager::UpdateLoadQueue() - BEGIN" );

	MutexWait();
	
	S32 resourceLoadCount = 0;
	bool done = false;

	C_LoadQueueType& list = m_LoadQueue;

	C_LoadQueueType::Iterator i = list.Begin();
	while	( 
				(i!=list.End()) &&
				((maxResourceLoads<0) || (resourceLoadCount < maxResourceLoads))
			)
	{
		C_ResourceBase* pResource = (C_ResourceBase*)i->GetValue().GetResource();

		E_State newState = E_State_Unknown;
		switch( pResource->GetState() )
		{
			case E_State_InLoadQueue:
			{
				newState = ChangeState( pResource, E_State_Loading );

				MutexRelease();

				BlockingMutexWait();
				MEMORY_TRACKER_PUSH_BY_ID(pResource->GetMemoryTrackerContextID());
				bool success = pResource->LoadResourceData();
				MEMORY_TRACKER_POP_BY_ID(pResource->GetMemoryTrackerContextID());
				if( success )
				{
					pResource->SetDataValid( true );
				}
				else
				{
					pResource->SetDataValid( false );
				}

				BlockingMutexRelease();

				MutexWait();

				newState = ChangeState( pResource, E_State_PostLoading );

				resourceLoadCount++;
			}
			break;

			default:
			break;
		}

		if( IsInLoadQueue( newState ) )
		{
			i++;
		}
		else
		{
			// not sure what loading has done to the queue so start from the beginning
			i = list.Begin();
		}
	}

	MutexRelease();

	//LOGDEBUG( g_Log, "C_Manager::UpdateLoadQueue() - END" );

	return resourceLoadCount;
}

void C_Manager::MutexWait() 
{ 
	m_Mutex.Wait(); 
}

void C_Manager::MutexRelease()
{ 
	m_Mutex.Release();
}

S32 C_Manager::UpdatePostLoadingList( const S32 maxPostLoadingCount_ )
{
	//LOGDEBUG( g_Log, "C_Manager::UpdatePostLoadingList() - BEGIN" );
	MutexWait();

	S32 resourcePostLoadingCount = 0;

	S32 maxPostLoadingCount = maxPostLoadingCount_;
	if (maxPostLoadingCount == -1) {
		maxPostLoadingCount = m_PostLoadingList.Size();
	}

	while	( 
				(m_PostLoadingList.Size() > 0) && 
				(resourcePostLoadingCount < maxPostLoadingCount)
		)
	{
		C_ResourceBase* pResource = m_PostLoadingList[m_PostLoadingList.Size() - 1];		

		MutexRelease();

#ifdef RESOURCE_MANAGER_CHECK_FOR_POST_LOAD_STALLS
		Time::C_Time timeBegin = Time::GetPerformanceCounter();
#endif

		bool resourcePostLoadStatus = true;

		if( pResource->IsDataValid() )
		{
			MEMORY_TRACKER_PUSH_BY_ID(pResource->GetMemoryTrackerContextID());
			resourcePostLoadStatus = pResource->PostLoadResourceData();
			if (resourcePostLoadStatus)
			{
				pResource->SetDonePostLoad( true );
			}
			MEMORY_TRACKER_POP_BY_ID(pResource->GetMemoryTrackerContextID());
		}

#ifdef RESOURCE_MANAGER_CHECK_FOR_POST_LOAD_STALLS
		Time::C_Time timeEnd = Time::GetPerformanceCounter();

		Time::C_Time duration = timeEnd - timeBegin;
		if(duration > SYS_USEC_PER_SEC)
		{
			LOGWARN(g_Log, "Resource Manager Post-Load Stall [%s] \"%s\" took %s", pResource->GetResourceTypeName().Chars(), pResource->GetResourceFilename().Chars(), Time::FormatProfileDurationString(duration).Chars());
		}
#endif

		MutexWait();

		if( !pResource->IsDataValid() )
		{
			LOGWARN( g_Log, "Failed loading [%s] resource from file '%s'\n", pResource->GetResourceTypeName().Chars(), pResource->GetResourceFilename().Chars() );
		}

		//If resource post load doesn't require a retry, change state appropriately.
		if (resourcePostLoadStatus)
		{
			// if load cancelled while we were loading, put object onto evict list
			if( pResource->IsLoadCancelled() )
			{
				ChangeState( pResource, E_State_Evicting );
			}
			else 
			{
				ChangeState( pResource, E_State_Ready );
			}

			UpdateNotify( pResource );
		}
		resourcePostLoadingCount++;
	}


	MutexRelease();

//	if (resourcePostLoadingCount > 0)
//	{
//		LOGINFO( g_Log, "				C_Manager::UpdatePostLoadingList count %i", resourcePostLoadingCount);
//	}


	//LOGDEBUG( g_Log, "C_Manager::UpdatePostLoadingList() - END" );

	return resourcePostLoadingCount;
}

S32 C_Manager::UpdatePreDeletingList( const S32 maxDeleteCount )
{
	//LOGDEBUG( g_Log, "C_Manager::UpdatePreDeletingList() - BEGIN" );
	MutexWait();
	
	S32 resourceDeleteCount = 0;

	while	( 
				(m_PreDeletingList.Size() > 0) && 
				((maxDeleteCount<0) || (resourceDeleteCount < maxDeleteCount)) 
		)
	{
		C_ResourceBase* pResource = m_PreDeletingList[m_PreDeletingList.Size() - 1];		

		UpdateNotify( pResource );

		MutexRelease();

		if( pResource->HasDonePostLoad() )
		{
			pResource->PreFreeResourceData();	
		}
	
		pResource->SetDonePostLoad( false );

		MutexWait();

		ChangeState( pResource, E_State_Deleted );

		resourceDeleteCount++;
	}


	MutexRelease();

	//LOGDEBUG( g_Log, "C_Manager::UpdatePreDeletingList() - END" );

	return resourceDeleteCount;
}

bool C_Manager::StartLoadingThread()
{
	LOGDEBUG( g_Log, "Resource::C_Manager::StartLoadingThread()\n" );

	m_LoadingThread.Start(0,0,"ResourceLoadingThread");
	return true;
}

bool C_Manager::StopLoadingThread()
{
	LOGDEBUG( g_Log, "Resource::C_Manager::StopLoadingThread()\n" );
	// make sure thread is not waiting on status dirty
	m_LoadingThread.SendQuitMessage();
	SetStatusDirty();
	m_LoadingThread.Join();
	return false;
}

bool C_Manager::PauseLoadingThread()
{
	LOGINFO( g_Log, "Resource::C_Manager::PauseLoadingThread()\n" );
	SetStatusDirty();

	m_LoadingThread.Pause();
#ifdef RESOURCE_MANAGER_CHECK_FOR_POST_LOAD_STALLS
	m_startPauseTime = Time::GetPerformanceCounter();
	m_startLoadQueueSize = m_LoadQueue.Size();
#endif
	return true;
}

bool C_Manager::ResumeLoadingThread()
{
	LOGINFO( g_Log, "Resource::C_Manager::ResumeLoadingThread()\n" );
	SetStatusDirty();

	S32 loadQueueDelta = m_LoadQueue.Size()-m_startLoadQueueSize;

	m_LoadingThread.Resume();

#ifdef RESOURCE_MANAGER_CHECK_FOR_POST_LOAD_STALLS
	F32 deltaTime = ((F32)(Time::GetPerformanceCounter() - m_startPauseTime))/1000.f;
	{
		#ifdef COMPILER_VC
		LOGINFO(g_Log, "TID %I64u Resource Manager was paused for %.2fms. %i new items added meanwhile", MT::GetThreadId(), deltaTime, loadQueueDelta);
		#else
		LOGINFO(g_Log, "TID %llu Resource Manager was paused for %.2fms. %i new items added meanwhile.", MT::GetThreadId(), deltaTime, loadQueueDelta);
		#endif
	}
	m_startPauseTime = 0;
	m_startLoadQueueSize = 0;
#endif
	return true;
}

void C_Manager::SetBackgroundLoadingMode(E_BackgroundLoading mode)
{
	MutexWait();

	if (m_backgroundLoading != mode)
	{
		if (mode == E_BackgroundLoading_Enabled)
		{
			LOGINFO(g_Log, "C_Manager::SetBackgroundLoadingMode: ENABLED");
			const U32 kBufSize = 1024*1024*4;
			m_pBackgroundLoadingBuffer = new U8[kBufSize];
			if (m_pBackgroundLoadingBuffer)
			{
				m_backgroundLoadingBufferSize = kBufSize;
				m_backgroundLoading = mode;
			}
		}
		//Disabling background loading
		else
		{
			m_backgroundLoading = mode;
			LOGINFO(g_Log, "C_Manager::SetBackgroundLoadingMode: DISABLED");
			//Wait for anyone who might be using this buffer.
			WaitForAllResourceData();
			ReleaseBackgroundLoadingBuffer();
		}
	}
	MutexRelease();
}

E_BackgroundLoading C_Manager::GetBackgroundLoadingMode() const
{
	return m_backgroundLoading;
}

bool C_Manager::GetBackgroundLoadingBuffer(U8** ppData, U32* pSize) 
{
	MutexWait();
	bool success;
	if (m_backgroundLoading == E_BackgroundLoading_Enabled)
	{
		*ppData = m_pBackgroundLoadingBuffer;
		*pSize = m_backgroundLoadingBufferSize;
		success = true;
	}
	else 
		success = false;

	MutexRelease();
	return success;
}

void C_Manager::ReleaseBackgroundLoadingBuffer()
{
	m_backgroundLoadingBufferSize = 0;
	SAFE_DELETE_ARY(m_pBackgroundLoadingBuffer);
}

#ifdef CFG_DEBUG
bool C_Manager::Validate() 
{ 
	MutexWait();

	const S32 resourceCount = m_ResourceList.Size();
	const S32 evictCount = m_EvictList.Size();
	const S32 loadCount = m_LoadQueue.Size();

	assert( evictCount <= resourceCount );
	assert( loadCount <= resourceCount );

	for( C_ResourceListType::Iterator i = m_ResourceList.Begin(); i !=m_ResourceList.End(); ++i )
	{		
		C_ResourceBase* pCurrResource = ((C_ResourceBase*)i->GetValue().GetResource());

		C_SenderListenerItem* pCurrItem = pCurrResource->GetSenderFirstListener();
		while(pCurrItem)
		{
			assert( (C_ResourceBase*)pCurrItem->GetSender() == pCurrResource );
			pCurrItem = pCurrItem->GetSenderNextListener();
		}
	}

	for( C_EvictListType::Iterator i = m_EvictList.Begin(); i!=m_EvictList.End(); ++i)
	{
		C_EvictItem& evictItem = *i;
		C_ResourceBase* pCurrResource = evictItem.m_pResource;


		C_SenderListenerItem* pCurrItem = pCurrResource->GetSenderFirstListener();
		while(pCurrItem)
		{
			assert( (C_ResourceBase*)pCurrItem->GetSender() == pCurrResource );
			pCurrItem = pCurrItem->GetSenderNextListener();
		}
	}

	for( C_LoadQueueType::Iterator i = m_LoadQueue.Begin(); i!=m_LoadQueue.End(); ++i)
	{
		C_ResourceBase* pCurrResource = ((C_ResourceBase*)i->GetValue().GetResource());


		C_SenderListenerItem* pCurrItem = pCurrResource->GetSenderFirstListener();
		while(pCurrItem)
		{
			assert( (C_ResourceBase*)pCurrItem->GetSender() == pCurrResource );
			pCurrItem = pCurrItem->GetSenderNextListener();
		}
	}

	MutexRelease();

	return true; 
}
#endif


void C_Manager::GetHandleList( Btl::Vector<C_HandleBase>&resHandleList, const C_ResourceTypeId desiredTypeId, E_State desiredState )
{
	MutexWait();

	resHandleList.Clear();
	for( C_ResourceListType::Iterator i = m_ResourceList.Begin(); i !=m_ResourceList.End(); ++i )
	{		
		//C_ResourceBase* pResource = &(*i);
		C_ResourceBase* pResource = (C_ResourceBase*)i->GetValue().GetResource();

		if( (desiredTypeId == pResource->GetResourceTypeId()) || (desiredTypeId == -1) )
		{
			if( ( pResource->GetState() == desiredState ) || ( desiredState == E_State_Unknown ) )
			{
				C_HandleBase handle;
				handle = pResource;
				resHandleList.PushBack( handle );
			}
		}
	}

	MutexRelease();
}

C_ResourceBase* C_Manager::GetResource( const Sys::C_FilePath& filename, const C_ResourceTypeId typeId )
{
	C_ResourceBase* pResource = FindResource(filename);

	if(pResource)
	{
		if( pResource->GetResourceTypeId() != typeId )
		{
			LOGERROR( g_Log, "Cannot load two different resource types from the same filename. Trying to load type %s, existing type is %s from file '%s'", 
				C_TypeRegistry::GetInstance()->GetResourceTypeName( typeId ).Chars(), 
				C_TypeRegistry::GetInstance()->GetResourceTypeName( pResource->GetResourceTypeId() ).Chars(), 
				filename.Chars() );
		}
	}
	else
	{
		pResource = C_TypeRegistry::GetInstance()->CreateResource( typeId );
		pResource->m_Filename = filename;

		// this must happen after the filename is set as this is where the resource is added to the resource list which uses the filename as part of its key
		ChangeState( pResource, E_State_Constructed );
	}

	return pResource;
}

S32 C_Manager::GetPendingResourceCount()
{
	MutexWait();

	S32 pendingResourceCount = 0;

	for (C_ResourceListType::Iterator i = m_ResourceList.Begin(); i!=m_ResourceList.End(); ++i)
	{
		const C_ResourceBase* pCurrResource = (C_ResourceBase*)i->GetValue().GetResource();

		switch(pCurrResource->GetState())
		{
			case E_State_InLoadQueue:
			case E_State_Loading:
			case E_State_PostLoading:
				pendingResourceCount++;
			break;

			default:
			break;
		}
	}

	MutexRelease();

	return pendingResourceCount;
}


void C_Manager::WaitForAllResourceData()
{
	SCOPED_TIMER("C_Manager::WaitForAllResourceData");
	S32 pendingResources = GetPendingResourceCount();
	while( pendingResources > 0)
	{
		PartialUpdate();
		U32 uMillisecs = 100;
		#if defined(PLATFORM_PC)
		::Sleep(uMillisecs);
		#else
		sys_timer_usleep(uMillisecs * 1000);
		#endif

		pendingResources = GetPendingResourceCount();
	}
}

#ifndef CFG_GOLD

// Wrapper around C_ResourceInfo* that allows us to sort by filename
struct C_DebugDumpResourceItem
{
	const C_ResourceInfo*	m_pResource;

	inline bool operator > (const C_DebugDumpResourceItem& other) const
	{
		return stricmp(m_pResource->GetResourceFilename().Chars(), other.m_pResource->GetResourceFilename().Chars()) > 0;
	}
};

typedef Btl::Vector<C_DebugDumpResourceItem> C_DebugDumpResourceItems; // Sortable list of resources
typedef Btl::Map<Memory::ContextId, C_DebugDumpResourceItems> C_DebugDumpResourceGroups; // Groups lists of resource by memory context

// Wrapper around <list of resources loaded under the same memory context> that allows us to sort by memory context name
struct C_DebugDumpResourceRef
{
	Memory::ContextId				m_MemoryContextId;
	const C_DebugDumpResourceItems*	m_pResourceItems;

	inline bool operator > (const C_DebugDumpResourceRef& other) const
	{
		Memory::TrackNodes& trackNodes = Memory::TrackNodes::Instance();
		return stricmp(trackNodes.GetContextNode(m_MemoryContextId).m_name, trackNodes.GetContextNode(other.m_MemoryContextId).m_name) > 0;
	}
};

typedef Btl::Vector<C_DebugDumpResourceRef> C_DebugDumpResourceRefs; // Sortable list of lists of resources

typedef Btl::Map<Memory::ContextId, U32> C_DebugDumpMemoryUsages; // Gathers memory usage by context.

// Wrapper around <memory usage for context> that can be sorted by memory context name
struct C_DebugDumpMemoryUsageRef
{
	Memory::ContextId	m_MemoryContextId;
	U32					m_MemoryUsage;

	inline bool operator > (const C_DebugDumpMemoryUsageRef& other) const
	{
		Memory::TrackNodes& trackNodes = Memory::TrackNodes::Instance();
		return stricmp(trackNodes.GetContextNode(m_MemoryContextId).m_name, trackNodes.GetContextNode(other.m_MemoryContextId).m_name) > 0;
	}
};

typedef Btl::Vector<C_DebugDumpMemoryUsageRef> C_DebugDumpMemoryUsageRefs; // Sortable list of memory usage by context

// Recursively gathers memory usage for a tree of context nodes in a single memory tracker
void DebugDumpGatherMemoryUsageTracker(C_DebugDumpMemoryUsages& dest, Memory::TrackNodes& trackNodes, const Memory::Tracker& memoryTracker, const Memory::ContextId memoryContextId)
{
	const U32 memoryUsage = memoryTracker.GetContextNode(memoryContextId).m_iAllocated;

	if (memoryUsage > 0)
	{
		const C_DebugDumpMemoryUsages::InsertionReturnType result = dest.Insert(memoryContextId);

		if (result.inserted)
		{
			result.iterator->GetValue() = memoryUsage;
		}
		else
		{
			result.iterator->GetValue() += memoryUsage;
		}

		for (int i = 0; i < trackNodes.GetNumContextNodes(); ++i)
		{
			const Memory::ContextId childContextId = static_cast<Memory::ContextId>(i);

			if (trackNodes.GetContextNode(childContextId).m_parent == memoryContextId)
			{
				DebugDumpGatherMemoryUsageTracker(dest, trackNodes, memoryTracker, childContextId);
			}
		}
	}
}

// Recursively gathers memory usage for a tree of context nodes and a tree of memory trackers
void DebugDumpGatherMemoryUsage(C_DebugDumpMemoryUsages& dest, Memory::TrackNodes& trackNodes, const Memory::Tracker& memoryTracker, const Memory::ContextId memoryContextId)
{
	DebugDumpGatherMemoryUsageTracker(dest, trackNodes, memoryTracker, memoryContextId);

	for (U32 i = 0; i < memoryTracker.GetNumSubTrackers(); ++i)
	{
		const Memory::Tracker* pTracker = memoryTracker.GetSubTracker(i);

		if (pTracker != NULL)
		{
			DebugDumpGatherMemoryUsage(dest, trackNodes, *pTracker, memoryContextId);
		}
	}
}

void C_Manager::DebugDump(void*)
{
	C_Manager* pManager = GetInstance();

	if (pManager != NULL)
	{
		LOGINFO(g_Log, "Dumping resource list...");

		// collect resources by memory context
		C_DebugDumpResourceGroups resourceGroups;

		for (C_ResourceListType::ConstIterator it = pManager->m_ResourceList.Begin(); it != pManager->m_ResourceList.End(); ++it)
		{
			C_DebugDumpResourceItem resourceItem;
			resourceItem.m_pResource = it->GetValue().GetResource();
			resourceGroups[resourceItem.m_pResource->GetMemoryTrackerContextID()].PushBack(resourceItem);
		}

		// store refs
		C_DebugDumpResourceRefs resourceRefs;
		resourceRefs.Reserve(resourceGroups.Size());

		for (C_DebugDumpResourceGroups::Iterator it = resourceGroups.Begin(); it != resourceGroups.End(); ++it)
		{
			// sort resources by filename
			C_DebugDumpResourceItems& resourceItems = it->GetValue();
			Util::ShellSortArray<C_DebugDumpResourceItems, C_DebugDumpResourceItem>(resourceItems, resourceItems.Size());

			// add ref
			C_DebugDumpResourceRef resourceRef;
			resourceRef.m_MemoryContextId = it->GetKey();
			resourceRef.m_pResourceItems = &resourceItems;
			resourceRefs.PushBack(resourceRef);
		}

		// sort refs by memory context name
		Util::ShellSortArray<C_DebugDumpResourceRefs, C_DebugDumpResourceRef>(resourceRefs, resourceRefs.Size());

		// output sorted resources
		const Memory::Tracker& memoryTracker = Memory::Tracker::Instance();
		Memory::TrackNodes& trackNodes = Memory::TrackNodes::Instance();

		for (C_DebugDumpResourceRefs::ConstIterator it = resourceRefs.Begin(); it != resourceRefs.End(); ++it)
		{
			// gather memory usage
			C_DebugDumpMemoryUsages memoryUsages;
			DebugDumpGatherMemoryUsage(memoryUsages, trackNodes, memoryTracker, it->m_MemoryContextId);

			// copy refs and then sort by memory context name
			C_DebugDumpMemoryUsageRefs memoryRefs;
			memoryRefs.Reserve(memoryUsages.Size());

			for (C_DebugDumpMemoryUsages::ConstIterator j = memoryUsages.Begin(); j != memoryUsages.End(); ++j)
			{
				C_DebugDumpMemoryUsageRef memoryRef;
				memoryRef.m_MemoryContextId = j->GetKey();
				memoryRef.m_MemoryUsage = j->GetValue();
				memoryRefs.PushBack(memoryRef);
			}

			Util::ShellSortArray<C_DebugDumpMemoryUsageRefs, C_DebugDumpMemoryUsageRef>(memoryRefs, memoryRefs.Size());

			// dump memory usage
			for (C_DebugDumpMemoryUsageRefs::ConstIterator j = memoryRefs.Begin(); j != memoryRefs.End(); ++j)
			{
				LOGINFO(g_Log, "%10d  %s", j->m_MemoryUsage, Memory::TrackNodes::Instance().GetContextNode(j->m_MemoryContextId).m_name);
			}

			// dump resources
			for (C_DebugDumpResourceItems::ConstIterator j = it->m_pResourceItems->Begin(); j != it->m_pResourceItems->End(); ++j)
			{
				LOGINFO(g_Log, "                %s", j->m_pResource->GetResourceFilename().Chars());
			}
		}
	}
}

void C_Manager::DebugForEach(ForEachCallback callback)
{
	MutexWait();

	for (C_ResourceListType::Iterator i = m_ResourceList.Begin(); i!=m_ResourceList.End(); ++i)
	{
		const C_ResourceBase* pCurrResource = (C_ResourceBase*)i->GetValue().GetResource();

		// Call our callback with the resource information
		callback(pCurrResource);
	}

	MutexRelease();
}

#endif

} // namespace Resource
