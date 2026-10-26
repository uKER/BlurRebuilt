#ifndef BTL_THREADS_H
#define BTL_THREADS_H
#ifdef _MSC_VER
#pragma once
#endif

/////////////////////////////////////////////////////////////
// Threading models
/////////////////////////////////////////////////////////////

#pragma once

#include "core/threads/mutex.h"

namespace Btl
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

/// Multi-threaded, lockable per object.
class ObjectLockable
{
public:

	class Lock
	{
	public:
		Lock(const ObjectLockable& object) : m_object(object) 
		{
			const_cast<MT::Lock&>(m_object.m_lock).Enter();
		}

		~Lock()
		{
			const_cast<MT::Lock&>(m_object.m_lock).Exit();
		}

	private:
		const ObjectLockable& m_object;
	};

	FRIEND_CLASS Lock;

private:

	MT::Lock m_lock;
};

/// @}

}

#endif
