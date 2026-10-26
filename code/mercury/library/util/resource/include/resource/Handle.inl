#ifndef UTIL_RESOURCE_HANDLE_INL
#define UTIL_RESOURCE_HANDLE_INL

#include <system/misc/StringUtil.h>

namespace Resource
{

	
C_HandleBase::C_HandleBase() 
	: m_Ptr(NULL)
{
}

C_HandleBase::C_HandleBase( const C_HandleBase& handle ) 
	: m_Ptr( handle.m_Ptr )
{
    if ( m_Ptr ) m_Ptr->IncRefCount();
}

C_HandleBase::~C_HandleBase()
{
    if ( m_Ptr ) m_Ptr->DecRefCount();
}

const C_HandleBase& C_HandleBase::operator = ( const C_HandleBase& ptr )
{
    if ( ptr.m_Ptr )    ptr.m_Ptr->IncRefCount();
    if ( m_Ptr )        m_Ptr->DecRefCount();
    m_Ptr = ptr.m_Ptr;
    return *this;
}

const C_HandleBase& C_HandleBase::operator = ( C_ResourceBase* ptr )
{
    if ( ptr )      ptr->IncRefCount();
    if ( m_Ptr )    m_Ptr->DecRefCount();
    m_Ptr = ptr;
    return *this;
}

const bool C_HandleBase::operator == ( const C_HandleBase& other ) const
{
    return other.m_Ptr == m_Ptr;
}

const bool C_HandleBase::operator != ( const C_HandleBase& other ) const
{
    return other.m_Ptr != m_Ptr;
}

C_HandleBase::operator C_ResourceBase*() const
{
	#ifndef CFG_SPEED
	if(!m_Ptr)
	{
		assert( 0 && "Getting pointer to resource which has not been created. Call Load before trying to get resource data pointer." );
	}
	else
	{
		switch(m_Ptr->GetState())
		{
			case E_State_Constructed:
			case E_State_InLoadQueue:
			case E_State_Loading:
			case E_State_Ready:
			break;

			case E_State_Evicting:
				assert( 0 && "Getting pointer to resource which is being evicted" );
			break;
		}
	}
	#endif

    return m_Ptr;
}
// C_Handle

template<typename T_Resource>
C_Handle<T_Resource>::C_Handle()
	: C_HandleBase()
{
}

template<typename T_Resource>
C_Handle<T_Resource>::C_Handle( const C_HandleBase& handle ) 
	: C_HandleBase( handle )
{
}

template<typename T_Resource>
C_Handle<T_Resource>::~C_Handle()
{
}

template<typename T_Resource>
const C_Handle<T_Resource>& C_Handle<T_Resource>::operator = ( const C_HandleBase& ptr )
{
    if ( ptr.m_Ptr )    ptr.m_Ptr->IncRefCount();
    if ( m_Ptr )        m_Ptr->DecRefCount();
    m_Ptr = ptr.m_Ptr;
    return *this;
}

template<typename T_Resource>
const C_Handle<T_Resource>& C_Handle<T_Resource>::operator = ( C_ResourceBase* ptr )
{
    if ( ptr )      ptr->IncRefCount();
    if ( m_Ptr )    m_Ptr->DecRefCount();
    m_Ptr = ptr;
    return *this;
}

template<typename T_Resource>
const bool C_Handle<T_Resource>::operator == ( const C_HandleBase& other ) const
{
    return other.m_Ptr == m_Ptr;
}

template<typename T_Resource>
const bool C_Handle<T_Resource>::operator != ( const C_HandleBase& other ) const
{
    return other.m_Ptr != m_Ptr;
}
    
template<typename T_Resource>
C_Handle<T_Resource>::operator T_Resource*() const
{
	#ifndef CFG_SPEED
	if(m_Ptr==NULL)
	{
		//assert( 0 && "Getting pointer to resource which has not been created. Call Load before trying to get resource data pointer." );
	}
	else
	{
		switch(m_Ptr->GetState())
		{
			case E_State_Constructed:
			case E_State_InLoadQueue:
			case E_State_Loading:
			case E_State_Ready:
			break;

			case E_State_Evicting:
				assert( 0 && "Getting pointer to resource which is being evicted" );
			break;
		}

		assert( (m_Ptr->GetResourceTypeId() == T_Resource::StaticGetResourceTypeId()) && "Wrong resource type" );
	}
	#endif

    return (T_Resource*)m_Ptr;
}

template<typename T_Resource>
bool C_Handle<T_Resource>::Load( const Sys::C_FilePath& filename, C_Listener* pListener, void* pListenerData )
{
	return C_HandleBase::Load( filename, T_Resource::StaticGetResourceTypeId(), pListener, pListenerData );
}

} // namespace Resource

#endif
