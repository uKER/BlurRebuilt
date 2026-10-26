#ifndef UTIL_RESOURCE_RESOURCE_BASE_H
#define UTIL_RESOURCE_RESOURCE_BASE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core/Core.h>
#include <System/DebugLog.h>
#include <system/memory/MemoryTracker.h>

#include <btl/containers/vector.h>
#include <btl/Containers/list.h>

#include <resource/Resource.h>

namespace Sys
{
	class I_BinaryStream;
}


//! @addtogroup Resource
//! @{

namespace Resource
{

//! C_ResourceInfo. This holds the resource state information.
class C_ResourceInfo : public C_Sender
{
template < class T_Resource > friend class C_Handle;
friend class C_Manager;
public:
	C_ResourceInfo();
	
	C_ResourceInfo( const Sys::C_FilePath& filename );

	virtual ~C_ResourceInfo();

	const Sys::C_FilePath	& GetResourceFilename() const;

	E_State GetState() const;
	
	bool IsDataReady() const;

	void SetDataOld( const bool dataOld );
				
	bool IsDataOld() const;

	void SetLoadCancelled( const bool loadCancelled );

	bool IsLoadCancelled() const;

	void SetDataValid( const bool dataValid );

	bool IsDataValid() const;

	void SetDonePostLoad( const bool donePostLoad );

	bool HasDonePostLoad() const;

	E_Notify GetNotifyType() const;

	Memory::ContextId GetMemoryTrackerContextID() const;

protected:
	
	void SetState( const E_State state );

	E_Notify GetLastNotify() const;

	void SetLastNotify( const E_Notify lastNotify );

	void SetMemoryTrackerContextID();

protected:
	S32							m_RefCount;
	U8							m_State;
	U8							m_LastNotify;
	U16							m_Flags;
	Sys::C_FilePath				m_Filename;	
	Memory::ContextId			m_MemoryTrackerContextID;
};

//! C_ResourceBase
class C_ResourceBase : public C_ResourceInfo
{
friend class C_HandleBase;
template < class T_Resource > friend class C_Handle;
friend class C_Manager;
public:
	C_ResourceBase();

	virtual ~C_ResourceBase();

	//! Helper function to be optionally used by derived class in implementation of LoadResourceData.
	//! Opens the resource file for reading. Generates a log warning and returns NULL if the file could not be opened.
	Sys::I_File* OpenResourceFile() const;

	//! Helper function to be optionally used by derived class in implementation of LoadResourceData.
	//! Opens the specified file for reading. Generates a log warning and returns NULL if the file could not be opened.
	Sys::I_File* OpenResourceFile( const Sys::C_FilePath& filename ) const;

	//! Implement this in the derived resource class. 
	//! Returns true if the load succeeded, false otherwise. If false is returned the data should cleaned up and reset to the unloaded state in this function.
	virtual bool LoadResourceData() = 0;

	//! Implment this in the derived resource class. 
	virtual bool FreeResourceData() = 0;

	//! Implment this in the derived resource class. 
	virtual bool PostLoadResourceData() { return true; }

	//! Implment this in the derived resource class. 
	virtual bool PreFreeResourceData() { return true; }

	//! Register a listener and data with this resource.
	void AddListener( C_Listener* pListener, void* pData = NULL );

	//! Unregister a listener with this resource.
	S32 RemoveListener( C_Listener* pListener );

	void DumpResourceInfo() const;

	//! Returns the resource type as an integer.
	//! no need to implement this function, it is implemented in C_Resource<>
	virtual const C_ResourceTypeId GetResourceTypeId() const = 0;

	//! Returns the resource type as a string.
	//! no need to implement this function, it is implemented in C_Resource<>
	virtual const Util::C_String GetResourceTypeName() const = 0;

private:

	//! Number of resource manager updates before a resource of this type is evicted.
	//! -1 means the resource can be evicted immediately on the thread that released the resource handle.
	//! Implement this in the derived resource class if required. 
	virtual S32 GetEvictDelay() { return -1; }

private:
    void IncRefCount();

    void DecRefCount();
};

//! C_Resource This templated class generates a unique static id for each derived resource type.
//! This is what resources should ultimately derive from.
//! i.e. C_Model : public C_Resource< C_Model >
template<typename T_Derived>
class C_Resource : public C_ResourceBase
{
public:
	C_Resource() {}

	virtual ~C_Resource() {}

	// inherited from C_ResourceBase
	const C_ResourceTypeId GetResourceTypeId() const
	{
		return StaticGetResourceTypeId();
	}

	// inherited from C_ResourceBase
	const Util::C_String GetResourceTypeName() const
	{
		return C_TypeRegistry::GetInstance()->GetResourceTypeName( StaticGetResourceTypeId() );
	}

	static const C_ResourceTypeId StaticGetResourceTypeId()
	{
		return m_StaticTypeId;
	}
	
	static C_ResourceBase* CreateResource() {return new T_Derived;}

private:
	static const C_ResourceTypeId m_StaticTypeId;
};

// Generate unique type ID per resource type
template < typename T_Derived >
const C_ResourceTypeId C_Resource<T_Derived>::m_StaticTypeId = C_TypeRegistry::GetInstance()->RegisterResourceType( T_Derived::StaticGetResourceTypeName(), &C_Resource<T_Derived>::CreateResource );

bool WaitForResourceData( C_ResourceBase* pResource );

} // namespace Resource

//! @}

#endif
