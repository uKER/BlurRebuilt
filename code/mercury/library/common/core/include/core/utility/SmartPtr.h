#ifndef SMARTPTR_H
#define SMARTPTR_H

#ifdef _MSC_VER
#pragma once
#endif

#ifdef PLATFORM_PS3
#include <libsn.h>
#endif

#include <core/core.h>
#include <core/utility/CoreDebug.h> 

namespace Sys
{

// forward declarations

template < class T_RefObj >
class RefPtr;

template < class T_RefObj >
class WeakPtr;

class RefObj; 

// RefPtr

template < class T_RefObj >
class RefPtr
{
#if !defined( COMPILER_GNU )
friend class RefPtr;
#endif
template < class T > friend class WeakPtr;
friend class RefObj;

public:
    
    /// Default Constructor (NULL)
    inline RefPtr()
        : m_Ptr( NULL )
    {
    }
    
    /// Constructor: From object ptr
    inline RefPtr( T_RefObj* ptr ) :
        m_Ptr( ptr )
    {
        if ( m_Ptr ) m_Ptr->IncRefCount();
    }
    
    /// Constructor: Copy
    FORCE_INLINE RefPtr( const RefPtr<T_RefObj>& ptr ) :
        m_Ptr( ptr.m_Ptr )
    {
        if ( m_Ptr ) m_Ptr->IncRefCount();
    }
    
    /// Explicit Constructor: From other Ptr<> type
    template < class S >
    inline explicit RefPtr( const RefPtr<S>& ptr ) : 
        m_Ptr( (T_RefObj*)ptr.m_Ptr )
    {
        if ( m_Ptr ) m_Ptr->IncRefCount();
    }
    
    /// Constructor: Copy from WeakPtr<>
    inline RefPtr( const WeakPtr<T_RefObj>& ptr ) :
        m_Ptr( ptr.m_Ptr )
    {
        if ( m_Ptr ) m_Ptr->IncRefCount();
    }
    
    /// Explicit Constructor: Copy from other WeakPtr<> type
    template < class S >
    inline explicit RefPtr( const WeakPtr<S>& ptr ) :
        m_Ptr( ptr.m_Ptr )
    {
        if ( m_Ptr ) m_Ptr->IncRefCount();
    }
    
    /// Destructor
    inline ~RefPtr()
    {
        if ( m_Ptr ) m_Ptr->DecRefCount();
    }
    
    /// Get pointer (avoid if possible)
    void* VoidPtr()
    {
        return (void*)m_Ptr;
    }

    const void* VoidPtr() const
    {
        return (const void*)m_Ptr;
    }

	T_RefObj* RawPtr()
    {
        return m_Ptr;
    }

	const T_RefObj* RawPtr() const
    {
        return m_Ptr;
    }
    
    /// Cast Operator: To bool for testing for NULL
    operator const bool () const
    {
        return m_Ptr != NULL;
    }
    
    /// Cast Operator: To object reference
    operator T_RefObj& () const
    {
        return *m_Ptr;
    }
    
    /// Dereference Operator
    T_RefObj& operator * () const
    {
        return *m_Ptr;
    }
    
    /// Member Access Operator
    inline T_RefObj* operator -> () const
    {
        return m_Ptr;
    }
    
    /// Assignment Operator: From object ptr
    inline const RefPtr& operator = ( T_RefObj* ptr )
    {
        if ( ptr )      ptr->IncRefCount();
        if ( m_Ptr )    m_Ptr->DecRefCount();
        m_Ptr = ptr;

        return *this;
    }
    
    /// Assignment Operator: Copy
    inline const RefPtr& operator = ( const RefPtr<T_RefObj>& ptr )
    {
        if ( ptr.m_Ptr )    ptr.m_Ptr->IncRefCount();
        if ( m_Ptr )        m_Ptr->DecRefCount();
        m_Ptr = ptr.m_Ptr;
        return *this;
    }

    /// Equality Operator: For testing pointer equality
    const bool operator == ( const T_RefObj* other ) const
    {
        return other == m_Ptr;
    }
    
    /// Equality Operator: For testing smart pointer equality
    const bool operator == ( RefPtr<T_RefObj>& other ) const
    {
        return other.m_Ptr == m_Ptr;
    }
    
    /// Inequality Operator: For testing pointer inequality
    const bool operator != ( const T_RefObj* other ) const
    {
        return other != m_Ptr;
    }
    
    /// Inequality Operator: For testing smart pointer inequality
    const bool operator != ( RefPtr<T_RefObj>& other ) const
    {
        return other.m_Ptr != m_Ptr;
    }
        
    /// Accessor: Gets object reference count
    const S32 RefCount() const
    {
        if ( m_Ptr )
        {
            return m_Ptr->RefCount();
        }
        return 0;
    }      
    
private:
#if defined( PLATFORM_PS3 ) || defined( __INTEL_COMPILER ) // JASOND: Intel compiler doesn't like this being private as well as GCC
public:
#endif
    T_RefObj*   m_Ptr;

};  

// WeakPtr

template < class T_RefObj >
class WeakPtr
{
template < class T > friend class RefPtr;
#ifndef COMPILER_GNU
friend class WeakPtr;
#endif
friend class RefObj;

public:
    
    /// Default Constructor (NULL)
    WeakPtr() :
        m_Ptr( NULL )
    {
		m_Prev = NULL;
        m_Next = NULL;
    }
    
    /// Constructor: From object ptr
    WeakPtr( T_RefObj* ptr ) :
        m_Ptr( ptr )
    {
		m_Prev = NULL;
        m_Next = NULL;
        
        AddToWeakPtrList();
    }
    
    /// Constructor: Copy
    WeakPtr( const WeakPtr<T_RefObj>& ptr ) :
        m_Ptr( ptr.m_Ptr )
    {
		m_Prev = NULL;
        m_Next = NULL;
        
        AddToWeakPtrList();
    }
    
    /// Explicit Constructor: From other WeakPtr<> type
    template < class S >
    explicit WeakPtr( const WeakPtr<S>& ptr ) :
        m_Ptr( (T_RefObj*)ptr.m_Ptr )
    {
		m_Prev = NULL;
        m_Next = NULL;

		AddToWeakPtrList();
    }
    
    /// Constructor: Copy from RefPtr<>
    WeakPtr( const RefPtr<T_RefObj>& ptr ) :
        m_Ptr( ptr.m_Ptr )
    {
		m_Prev = NULL;
        m_Next = NULL;

		AddToWeakPtrList();
    }
    
    /// Destructor
    ~WeakPtr()
    {
        RemoveFromWeakPtrList();
    }
    
    /// Cast Operator: To bool for testing for NULL
    operator const bool () const
    {
        return m_Ptr != NULL;
    }
    
    /// Cast Operator: To object reference
    operator T_RefObj& () const
    {
        return *m_Ptr;
    }
    
    /// Dereference Operator
    T_RefObj& operator * () const
    {
        return *m_Ptr;
    }
    
    /// Member Access Operator
    T_RefObj* operator -> () const
    {
        return m_Ptr;
    }
    
	/// Assignment Operator: From object ptr
    const WeakPtr& operator = ( T_RefObj* ptr )
    {
        RemoveFromWeakPtrList();

		m_Ptr = ptr;

        AddToWeakPtrList();

        return *this;
    }
    
    /// Assignment Operator: Copy
    const WeakPtr& operator = ( const WeakPtr<T_RefObj>& ptr )
    {
        RemoveFromWeakPtrList();

        m_Ptr = ptr.m_Ptr;

        AddToWeakPtrList();

        return *this;
    }
    
    /// Assignment Operator: Copy from Ptr<>
    const WeakPtr& operator = ( const RefPtr<T_RefObj>& ptr )
    {
        RemoveFromWeakPtrList();

        m_Ptr = ptr.m_Ptr;

        AddToWeakPtrList();

        return *this;
    }
    
    /// Get pointer (avoid if possible)
    const void* VoidPtr() const
    {
        return (void*)m_Ptr;
    }

    void* VoidPtr()
    {
        return (void*)m_Ptr;
    }

	const T_RefObj* RawPtr() const
    {
        return m_Ptr;
    }

	T_RefObj* RawPtr()
    {
        return m_Ptr;
    }
    
    /// Equality Operator: For testing pointer equality
    const bool operator == ( T_RefObj* other ) const
    {
        return other == m_Ptr;
    }
    
    /// Accessor: Gets object reference count
    const S32 RefCount() const
    {
        if ( m_Ptr )
        {
            return m_Ptr->RefCount();
        }
        return 0;
    }
    
    /// Add this WeakPtr to the RefObj's weak ptr list
    void AddToWeakPtrList()
    {
        if ( m_Ptr )
        {
            WeakPtr<RefObj>* pFirst = m_Ptr->m_WeakPtrList;
            if ( pFirst )
            {
                pFirst->m_Prev = (WeakPtr<RefObj>*)this;
            }
            
            m_Next = pFirst;
            
            m_Ptr->m_WeakPtrList = (WeakPtr<RefObj>*)this;
        }
    }
    
    /// Remove this WeakPtr from the RefObj's weak ptr list
    void RemoveFromWeakPtrList()
    {
        if ( m_Ptr )
        {
            if ( m_Ptr->m_WeakPtrList == (WeakPtr<RefObj>*)this )
            {
                m_Ptr->m_WeakPtrList = m_Next;
            }
            
            if ( m_Prev )
            {
                m_Prev->m_Next = m_Next;
            }
            
            if ( m_Next )
            {
                m_Next->m_Prev = m_Prev;
            }
            
            m_Prev = NULL;
            m_Next = NULL;
        }
    }

	WeakPtr<RefObj>* GetNext()
	{
		return m_Next;
	}

	WeakPtr<RefObj>* GetPrev()
	{
		return m_Prev;
	}

    
#if defined( __INTEL_COMPILER ) || defined( COMPILER_GNU )
public:
#else
private:
#endif
    
    T_RefObj*           m_Ptr;

    WeakPtr<RefObj>*    m_Prev;
    WeakPtr<RefObj>*    m_Next;
};



// RefObj

class RefObj
{
template < class T > friend class RefPtr;
template < class T > friend class WeakPtr;

public:
    
    /// Constructor: Initialises ref count to zero
    RefObj() :
        m_RefCount( 0 )
    {
        m_WeakPtrList=NULL;
    }
    
    /// Virtual Destructor:
    virtual ~RefObj()
    {
        WeakPtr<RefObj>* currPtr = m_WeakPtrList;
        while ( currPtr )
        {
            currPtr->m_Ptr = NULL;
            currPtr = currPtr->m_Next;
        }
        m_WeakPtrList = NULL;
        
        if ( m_RefCount != 0 )
        {
            assert(0); // There are still valid pointers to this object!
        }
    }
    
    /// Accessor: Get reference count.
    const S32           RefCount() const    { return (S32)m_RefCount; }
    
    /// Address-of Operator: Returns Ptr<> to object!
    RefPtr<RefObj>      operator & ()       { return RefPtr<RefObj>( this ); }

	RefObj* RawPtr() { return this; }

	const RefObj* RawPtr() const { return this; } 
    
public:
    /// Increment ref count
    FORCE_INLINE void IncRefCount()
    {
        m_RefCount++;
    }
    
    /// Decrement ref count
    FORCE_INLINE void DecRefCount()
    {
        if ( --m_RefCount <= 0 )
        {
            delete this;
        }
    }
    
protected:
	int									m_RefCount;

    WeakPtr<RefObj>*                    m_WeakPtrList;
};

}; // namespace Sys

#endif
