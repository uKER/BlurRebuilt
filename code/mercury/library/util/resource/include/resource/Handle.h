#ifndef UTIL_RESOURCE_HANDLE_H
#define UTIL_RESOURCE_HANDLE_H

#ifdef _MSC_VER
#pragma once
#endif

#include <core\Core.h>
#include <core\Containers.h>

#include <resource/TypeRegistry.h>

//! @addtogroup Resource
//! @{

namespace Resource
{

class C_Listener;
class C_ResourceBase;

//! C_HandleBase generic resource handle not specialised to a particular resource type.
//! The handle is a smart pointer. When all handles go out of scope the resource is evicted.
class C_HandleBase
{
public:
	//! Default Constructor
	inline C_HandleBase(); 

	//! Destructor
	inline ~C_HandleBase();

	//! Copy Constructor
    inline C_HandleBase( const C_HandleBase& handle );

	//! Assignment Operator: From object ptr
    inline const C_HandleBase& operator = ( C_ResourceBase* ptr );

    //! Assignment Operator: Copy
    inline const C_HandleBase& operator = ( const C_HandleBase& ptr );

	//! Equality Operator
    inline const bool operator == ( const C_HandleBase& other ) const;
    
    //! Inequality Operator
    inline const bool operator != ( const C_HandleBase& other ) const;

	//! Cast to Resource pointer. Use this to get to the resource. 
	//! This is only valid if the handle is pointing to a created resource.
	//! e.g. if Load has been called or the handle was initialised from another handle / resource.
	inline operator C_ResourceBase*() const;

	//! Sets handle to point to a resource of type 'typeId' with the filename 'filename'. 
	//! This will create a new resource if one with this filename does not exist.
	//! If the resource data is not loaded, the resource is added to the load queue.
	//! If a listener is provided, the listener will be notified once the resource data is loaded or evicted or immediately if the
	//! resource data is already loaded.
	bool Load( const Sys::C_FilePath& filename, const C_ResourceTypeId typeId, C_Listener* pListener = NULL, void* pListenerData = NULL );

protected:
    C_ResourceBase*		m_Ptr;
};

//! C_Handle resource handle specialised to a particular resource type. 
//! The handle is a smart pointer. When all handles go out of scope the resource is evicted.
//! Templated so that load and cast to resource type do not need to be specialised with the resource type.
template<typename T_Resource>
class C_Handle : public C_HandleBase
{
public:      
	//! Default Constructor
	C_Handle();

	//! Destructor
	~C_Handle();

	//! Copy Constructor
    C_Handle( const C_HandleBase& handle );

	//! Assignment Operator: From object ptr
    inline const C_Handle& operator = ( C_ResourceBase* ptr );

    //! Assignment Operator: Copy
    const C_Handle<T_Resource>& operator = ( const C_HandleBase& ptr );
    
	//! Equality Operator
    inline const bool operator == ( const C_HandleBase& other ) const;
    
    //! Inequality Operator
    inline const bool operator != ( const C_HandleBase& other ) const;

	//! Cast to Resource pointer. Use this to get to the resource. 
	//! This is only valid if the handle is pointing to a created resource.
	//! e.g. if Load has been called or the handle was initialised from another handle / resource.
    operator T_Resource*() const;

	//! Sets handle to point to a resource of type T_Resource with the filename 'filename'. 
	//! This will create a new resource if one with this filename does not exist.
	//! If the resource data is not loaded, the resource is added to the load queue.
	//! If a listener is provided, the listener will be notified once the resource data is loaded or immediately if the
	//! resource data is already loaded.
	bool Load( const Sys::C_FilePath& filename, C_Listener* pListener = NULL, void* pListenerData = NULL );

private:
};

} // namespace Resource

//! @}

#endif

