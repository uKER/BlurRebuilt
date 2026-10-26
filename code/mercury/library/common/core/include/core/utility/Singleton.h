#ifndef SINGLETON_H
#define SINGLETON_H

#ifdef _MSC_VER
#pragma once
#endif

//////////////////////////////////////////////////////
// Singleton
//
// Example:
//
// class C_MyClass : public Ut::Singleton<C_MyClass>
// {
// public:
//     void MyCode();
// }
//
//////////////////////////////////////////////////////

#include <core/utility/threads.h>
#include <cstdlib> // for atexit

/////////////////////////////////////////////////////////

namespace Ut
{

/// @addtogroup Singleton
/// @{

template <class T>
struct CreateUsingNew;

template <class T>
struct DefaultLifetime;

template <class T, template <class> class CreationPolicy = CreateUsingNew, template <class> class LifetimePolicy = DefaultLifetime, class ThreadingModel = SingleThreaded>
class Singleton;

#ifdef _MSC_VER
#define C_CALLING_CONVENTION_QUALIFIER __cdecl 
#else
#define C_CALLING_CONVENTION_QUALIFIER 
#endif

typedef void (C_CALLING_CONVENTION_QUALIFIER *atexit_pfn_t)();

/// Class to cast a pointer from the base class to derived class taking into account multiple inheritance
template <typename Base, typename Derived, int size = sizeof(void*)>
class BaseToDerived
{
public:
	static Derived* GetPointer()
	{
        #if !defined( __GNUC__ )
	    BIZ_STATIC_ASSERT(0);
        #endif
	}
};

/// 32-bit implementation
template <typename Base, typename Derived>
class BaseToDerived<Base, Derived, 4>
{
public:
	static Derived* GetPointer(Base* pointer)
	{
		U64 offset = (U64)(Derived*)1 - (U64)(Derived*)(Base*)1 ;
		//return (Derived*)((U64)pointer - offset);
		return (Derived*)(U32)((U64)pointer - offset); // JASOND: Intel compiler: "warning #1195: conversion from integer to smaller pointer"
	}
};

// 64-bit implementation
template <typename Base, typename Derived>
class BaseToDerived<Base, Derived, 8>
{
public:
	static Derived* GetPointer(Base* pointer)
	{
		U64 offset = (U64)(Derived*)1 - (U64)(Derived*)(Base*)1 ;
		return (Derived*)((U64)pointer - offset);
	}
};

//////////////////////////////////////////////////////
// CreatePolicy: new, static, or external

// CreateUsingNew
/// Creates and destroys singleton using dynamic allocation
template <class T>
struct CreateUsingNew
{
	static T* Create()
	{
		return new T;
	}

	static void Destroy(T* p)
	{
		delete p;
	}

	static const bool m_bManagedDestroy = true;
};

// CreateExternal
/// Allows object to be created externally, to allow for non-trivial constructors
/// Objects of this type can be allocated and deleted many times by user, they will not be scheduled for destruction
template <class T>
struct CreateExternal : CreateUsingNew<T>
{
	static const bool m_bManagedDestroy = false;
};

// CreateStatic
/// Creates the singleton upon first access using static allocation
template <class T>
struct CreateStatic
{
    static T* Create()
    {
		static T staticMemory;
		return &staticMemory;
    }
    
    static void Destroy(T* p)
    {
		//p->~T();
    }
	
	static const bool m_bManagedDestroy = true;
};

//////////////////////////////////////////////////////
// ConstructorHelper:
// Helper class to allow CreateExternal types of singletons to obtain a correct instance pointer upon creation

// Default version doesn't so anything
template <class CreationPolicy, typename Singleton>
struct ConstructorHelper
{
	static typename Singleton::ObjectType* Create(Singleton* pThis)
	{
		BIZ_ASSERT_MSG(pThis->m_bCreated, "This type of singleton cannot be allocated directly!");	// assert to show incorrect behaviour
		return NULL;
	}
};

// CreateExternal version casts from singleton to derived class taking into account multiple inheritance offsets
template <typename Singleton>
struct ConstructorHelper<CreateExternal<typename Singleton::ObjectType>, Singleton>
{
	static typename Singleton::ObjectType* Create(Singleton* pThis)
	{
		return BaseToDerived<Singleton, typename Singleton::ObjectType>::GetPointer(pThis);
	}
};

//////////////////////////////////////////////////////
// DestructionHelper:
// Helper class to detect cases where delete is called on singletons created with

// Default version asserts, as its not ok to call destructor on CreateUsingNew or CreateStatic
template <class CreationPolicy, typename Singleton>
struct DestructionHelper
{
	static void Destroy()
	{
		BIZ_ASSERT_FAIL("Singleton: Tried to delete a managed singleton");
	}
};

// CreateExternal version casts from singleton to derived class taking into account multiple inheritance offsets
template <typename Singleton>
struct DestructionHelper<CreateExternal<typename Singleton::ObjectType>, Singleton>
{
	static void Destroy()
	{
		// this is ok
	}
};

//////////////////////////////////////////////////////
// LifetimePolicy: default only, possibility for more elaborte policies

// DefaultLifetime
/// Schedules object for destruction, detects access after destruction
template <class T>
struct DefaultLifetime
{
    static void ScheduleDestruction(T*, atexit_pfn_t pAtExit)
    { std::atexit(pAtExit); }
    
    static void OnDeadReference()
    { BIZ_ASSERT_FAIL("Singleton: Dead reference detected"); }
};

//////////////////////////////////////////////////////
/// Singleton
///                                                   
/// Choice of: @n
/// @li creation policy (CreateUsingNew, CreateStatic, CreateExternal) @n
/// @li lifetime policy (DefaultLifeTime only for now) @n
///
/// Classes can either derive off the Singleton using public inheritance
/// Alternatively create Singleton objects directly and use typedefs
///	
/// class MyClass : public Singleton<MyClass>
/// class MyClass : public Singleton<MyClass, CreateExternal, DefaultLifetime, ObjectLockable>
/// 
/// class MyClass
/// typedef Singleton<MyClass>	MyClassSingleton
/// @endcode
///
/// @param T The type of object to be a singleton
/// @param CreatePolicy How the object is created
/// @param LifetimePolicy How the object is destroyed
/// @param ThreadingModel To protect against multiple threads creating singletons
/// @remarks
/// CreateExternal can only be used with public inheritance, otherwise compiler error
template <class T, template <class> class CreationPolicy, template <class> class LifetimePolicy, class ThreadingModel>
class Singleton
{
public:
	typedef T		ObjectType;
	typedef T&		ObjectReference;
	typedef T*		ObjectPointer;

	static T&		Instance();
	static bool		Exists() { return m_pInstance != 0; }

protected:
	Singleton()
	{
		BIZ_ASSERT_MSG(m_pInstance == NULL, "Singleton: Instance already created!");
		// assign pointer if externally allocated, also ensure uniqueness
		m_pInstance = ConstructorHelper<CreationPolicy<T>, Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel> >::Create(this);
	}
	~Singleton()
	{
		if(!m_bDestroyed)
		{
			// deallocation was triggered externally, make sure object exists and external deallocation is valid
			assert(m_pInstance);
			DestructionHelper<CreationPolicy<T>, Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel> >::Destroy();
		}
		m_bCreated = false;
		m_pInstance = 0;
	}
	Singleton(const Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel>&);
	Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel>& operator=(const Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel>&);

	friend struct ConstructorHelper<CreationPolicy<T>, Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel> >;
	friend struct DestructionHelper<CreationPolicy<T>, Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel> >;
private:
	static void									CreateInstance();
	static void C_CALLING_CONVENTION_QUALIFIER	DestroyInstance();

	static T*				m_pInstance;
	static bool				m_bCreated;		// used to detect whether the singleton is active or not
    static bool				m_bDestroyed;	// slightly different, whether singleton has been destroyed to detect dead references
	static ThreadingModel	m_ThreadData;
};

template <class T, template <class> class CreationPolicy, template <class> class LifetimePolicy, class ThreadingModel>
INLINE T& Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel>::Instance()
{
	if (!m_pInstance)
	{
		CreateInstance();
	}
	return *m_pInstance;
}

// helpers
template <class T, template <class> class CreationPolicy, template <class> class LifetimePolicy, class ThreadingModel>
void Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel>::CreateInstance()
{
	typename ThreadingModel::Lock lock(m_ThreadData);

	if (!m_pInstance)
	{
		if (m_bDestroyed)
		{
			m_bDestroyed = false;
			LifetimePolicy<T>::OnDeadReference();
		}
		m_bCreated = true;
		m_pInstance = CreationPolicy<T>::Create();

		// CreateExternal leaves destruction to the app
		if(CreationPolicy<T>::m_bManagedDestroy)
			LifetimePolicy<T>::ScheduleDestruction(m_pInstance, &DestroyInstance);
	}
}

template <class T, template <class> class CreationPolicy, template <class> class LifetimePolicy, class ThreadingModel>
void C_CALLING_CONVENTION_QUALIFIER Singleton<T, CreationPolicy, LifetimePolicy, ThreadingModel>::DestroyInstance()
{
	// its not necessarily safe to call destroy twice (eg CreateStatic with non trivial destructor)
	assert(!m_bDestroyed && CreationPolicy<T>::m_bManagedDestroy);
	m_bDestroyed = true;

	CreationPolicy<T>::Destroy(m_pInstance);
	m_pInstance = 0;
	
}

////////////////////////////////////////////////////////////////////////////////
// Singleton's data
////////////////////////////////////////////////////////////////////////////////

template <class T, template <class> class CreationPolicy, template <class> class LifetimePolicy, class ThreadingModel> 
T* Singleton <T, CreationPolicy, LifetimePolicy, ThreadingModel>::m_pInstance = 0;

template <class T, template <class> class CreationPolicy, template <class> class LifetimePolicy, class ThreadingModel>
bool Singleton <T, CreationPolicy, LifetimePolicy, ThreadingModel>::m_bCreated = false;

template <class T, template <class> class CreationPolicy, template <class> class LifetimePolicy, class ThreadingModel>
bool Singleton <T, CreationPolicy, LifetimePolicy, ThreadingModel>::m_bDestroyed = false;

template <class T, template <class> class CreationPolicy, template <class> class LifetimePolicy, class ThreadingModel>
ThreadingModel Singleton <T, CreationPolicy, LifetimePolicy, ThreadingModel>::m_ThreadData;

/// @}

} // Ut

#endif // SINGLETON_H
