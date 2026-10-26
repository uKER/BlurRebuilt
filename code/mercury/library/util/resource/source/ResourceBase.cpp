
#include "stdafx.h"
#include <system/time/scopedtimer.h>

#include <resource/Resource.h>
#include <system/memory/MemoryTracker.h>

namespace Resource
{

enum
{
	E_Flag_DataOld			= 0x0001,
	E_Flag_LoadCancelled	= 0x0002,
	E_Flag_DataValid		= 0x0004,
	E_Flag_DonePostLoad		= 0x0008
};

Util::C_String GetStateName( const E_State state ) 
{
	Util::C_String stateName;

	switch( state )
	{
		case E_State_Unknown:		stateName = "Unknown";			break;
		case E_State_Constructed:	stateName = "Constructed";		break;
		case E_State_InLoadQueue:	stateName = "InLoadQueue";		break;
		case E_State_Loading:		stateName = "Loading";			break;
		case E_State_PostLoading:	stateName = "PostLoading";		break;
		case E_State_Ready:			stateName = "Ready";			break;
		case E_State_Evicting:		stateName = "Evicting";			break;
		case E_State_PreDeleting:	stateName = "PreDeleting";		break;
		case E_State_Deleted:		stateName = "Deleted";			break;
		default:					stateName = "Unknown";			break;
	}

	return stateName;
}

// C_ResourceInfo
C_ResourceInfo::C_ResourceInfo()
	: m_RefCount(0)
	, m_State( E_State_Unknown )
	, m_LastNotify( E_Notify_NoData )
	, m_Filename( "" )
	, m_Flags( 0 )
	, m_MemoryTrackerContextID( 0 )
{
}

C_ResourceInfo::C_ResourceInfo( const Sys::C_FilePath& filename )
	: m_RefCount(0)
	, m_State( E_State_Unknown )
	, m_Filename( filename )
	, m_Flags( 0 )
	, m_MemoryTrackerContextID( 0 )
{
}

C_ResourceInfo::~C_ResourceInfo()
{
}
 
void C_ResourceInfo::SetMemoryTrackerContextID()
{
#ifndef CFG_SPEED
	const ::Memory::ContextId currentContext = ::Memory::TrackNodes::Instance().GetContext();
	const ::Memory::ContextNodeName&	cnnOld = ::Memory::TrackNodes::Instance().GetContextNode(m_MemoryTrackerContextID);
	const ::Memory::ContextNodeName&	cnnNew = ::Memory::TrackNodes::Instance().GetContextNode(currentContext);
	
	if (currentContext == 0)
	{
		LOGWARN( g_Log, "Resource '%s' being tracked as \"unknown\"", m_Filename.Chars());
	}

	//warn if this resource has already been added for load in a different memory tracking context.
	if ((m_MemoryTrackerContextID != 0) && (currentContext != m_MemoryTrackerContextID))
	{
		LOGDEBUG( g_Log, "Resource '%s' being tracked as '%s' but previously set as '%s'", m_Filename.Chars(), cnnNew.m_name, cnnOld.m_name);
	}
	m_MemoryTrackerContextID = currentContext;
#endif
}

Memory::ContextId C_ResourceInfo::GetMemoryTrackerContextID() const
{
	return m_MemoryTrackerContextID;
}

const Sys::C_FilePath& C_ResourceInfo::GetResourceFilename() const
{
	return m_Filename;
}

E_State C_ResourceInfo::GetState() const
{
	return (E_State)m_State;
}

void C_ResourceInfo::SetState( const E_State state )
{
	m_State = state;
}

bool C_ResourceInfo::IsDataReady() const
{
	return (GetNotifyType() == E_Notify_DataReady);
}

void C_ResourceInfo::SetDataOld( const bool dataOld )
{
	if(dataOld)
	{
		m_Flags |= E_Flag_DataOld;
	}
	else
	{
		m_Flags &= ~E_Flag_DataOld;
	}
}

bool C_ResourceInfo::IsDataOld() const
{
	return ((m_Flags & E_Flag_DataOld) != 0);
}

void C_ResourceInfo::SetLoadCancelled( const bool loadCancelled )
{
	if(loadCancelled)
	{
		m_Flags |= E_Flag_LoadCancelled;
	}
	else
	{
		m_Flags &= ~E_Flag_LoadCancelled;
	}
}

bool C_ResourceInfo::IsLoadCancelled() const
{
	return ((m_Flags & E_Flag_LoadCancelled) != 0);
}

void C_ResourceInfo::SetDataValid( const bool dataValid )
{
	if(dataValid)
	{
		m_Flags |= E_Flag_DataValid;
	}
	else
	{
		m_Flags &= ~E_Flag_DataValid;
	}
}

bool C_ResourceInfo::IsDataValid() const
{
	return ((m_Flags & E_Flag_DataValid) != 0);
}

void C_ResourceInfo::SetDonePostLoad( const bool donePostLoad )
{
	if(donePostLoad)
	{
		m_Flags |= E_Flag_DonePostLoad;
	}
	else
	{
		m_Flags &= ~E_Flag_DonePostLoad;
	}
}

bool C_ResourceInfo::HasDonePostLoad() const
{
	return ((m_Flags & E_Flag_DonePostLoad) != 0);
}

E_Notify C_ResourceInfo::GetLastNotify() const
{
	return (E_Notify)m_LastNotify;
}

void C_ResourceInfo::SetLastNotify( const E_Notify lastNotify )
{
	m_LastNotify = (U8)lastNotify;
}

E_Notify C_ResourceInfo::GetNotifyType() const
{
	E_Notify result = E_Notify_NoData;

	switch( GetState() )
	{
		case E_State_Ready:
		case E_State_Evicting:
			result = E_Notify_DataReady;
		break;
	}

	if( result == E_Notify_DataReady )
	{
		if( !IsDataValid() )
		{
			result = E_Notify_DataError;
		}
	}

	return result;
}


// C_ResourceBase
C_ResourceBase::C_ResourceBase()
{
	SetDataOld(false);
	SetLoadCancelled(false);
	SetDataValid(true);
}

C_ResourceBase::~C_ResourceBase()
{
	assert( m_RefCount == 0 );

	// This should not be needed as we should be in a deleted state or unknown if externally managed
	if(IsInLoadQueue( GetState() ))
	{
		C_Manager::GetInstance()->RemoveFromLoadQueue( this );
	}

	// This should not be needed as we should be in a deleted state or unknown if externally managed
	if(IsInEvictList( GetState() ))
	{
		C_Manager::GetInstance()->RemoveFromEvictList( this );
	}
}

void C_ResourceBase::IncRefCount()
{
	C_Manager::GetInstance()->MutexWait();
    m_RefCount++;
	C_Manager::GetInstance()->MutexRelease();
}

void C_ResourceBase::DecRefCount()
{
	C_Manager::GetInstance()->MutexWait();
    if ( --m_RefCount <= 0 )
    {
		C_Manager::GetInstance()->Evict( this );
    }
	C_Manager::GetInstance()->MutexRelease();
}

Sys::I_File* C_ResourceBase::OpenResourceFile() const
{
	return OpenResourceFile( GetResourceFilename() );
}

Sys::I_File* C_ResourceBase::OpenResourceFile( const Sys::C_FilePath& filename ) const
{
	Sys::I_File* pFile = C_Manager::GetInstance()->m_pFileSystem->Open( filename );
	//LOGINFO( g_Log, "*** %s", filename.Chars());

	if( !pFile )
	{
		LOGWARN( g_Log, "Failed to open resource file '%s'\n", filename.Chars() );
	}

	return pFile;
}

void C_ResourceBase::AddListener( C_Listener* pListener, void* pData )
{
	C_Manager::GetInstance()->MutexWait();
	SenderAddListener( pListener, pData );
	E_Notify notifyType = GetNotifyType();
	if( notifyType != E_Notify_NoData )
	{
		pListener->ResourceNotify( this, notifyType, pData );
	}
	C_Manager::GetInstance()->MutexRelease();
}

S32 C_ResourceBase::RemoveListener( C_Listener* pListener )
{
	return SenderRemoveListener( pListener );
}


void C_ResourceBase::DumpResourceInfo() const
{
	const S32 resourceTypeId = GetResourceTypeId();
	const Util::C_String& resourceTypeName = GetResourceTypeName();
	const Sys::C_FilePath& resourceFilename = GetResourceFilename();
	Util::C_String stateName = GetStateName( GetState() );

	LOGDEBUG( g_Log, "   pResource=0x%08x ResourceType:[%s] State:'%s' Filename:\"%s\" RefCount:%d", this, resourceTypeName.Chars(), stateName.Chars(), resourceFilename.Chars(), m_RefCount );
}

// Helper function WaitForResourceData

class C_BlockingListener : public Resource::C_Listener
{
public:
	// inherited from Resource::C_Listener
	void ResourceNotify( Resource::C_ResourceBase* pResource, const Resource::E_Notify result, void* pData )
	{
		switch( result )
		{
			case Resource::E_Notify_DataReady:
				m_Event.Signal();
			break;
			case Resource::E_Notify_DataError:
				m_Event.Signal();
			break;

			default:
			break;
		}
	}

	void WaitForLoad()
	{
		while(!m_Event.Wait(100))
		{
			Resource::C_Manager::GetInstance()->PartialUpdate();
		}
	}

private:
	MT::Event m_Event;
};

bool WaitForResourceData( C_ResourceBase* pResource )
{
	SCOPED_TIMER("WaitForResourceData");
	C_BlockingListener listener;
	pResource->AddListener( &listener, NULL );
	listener.WaitForLoad();

	return pResource->IsDataReady();
}


} // namespace Resource
