#ifndef __UT_THREADS_H__
#define __UT_THREADS_H__

#pragma once

namespace Ut
{

/// @addtogroup Threading_Models
/// Used as template parameters to specify what type of thread synchronisation code
/// should be used by a class.
/// @{

/// No thread synchronisation.
class SingleThreaded
{
public:

	SingleThreaded() {}
	~SingleThreaded() {}

	class Lock
	{
	public:
		Lock(const SingleThreaded& object) {}
		~Lock() {}
	};
};

// system-dependent critical section
#ifdef _XENON

/// Multi-threaded, lockable per object.
class ObjectLockable
{
public:

	ObjectLockable()
	{
		InitializeCriticalSection(&m_criticalSection);
	}

	~ObjectLockable()
	{
		DeleteCriticalSection(&m_criticalSection);
	}

	class Lock
	{
	public:
		Lock(const ObjectLockable& object) : m_object(object) 
		{
			EnterCriticalSection(const_cast<CRITICAL_SECTION*>(&m_object.m_criticalSection));
		}

		~Lock()
		{
			LeaveCriticalSection(const_cast<CRITICAL_SECTION*>(&m_object.m_criticalSection));
		}

	private:
		const ObjectLockable& m_object;
	};

	friend Lock;

private:

	CRITICAL_SECTION m_criticalSection;
};

#endif

/// @}

}

#endif