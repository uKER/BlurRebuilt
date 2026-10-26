#include "SystemPch.h"
#include "ArteryImpl.h"

//
// ArteryThread
//

void CallArteryThreadProc(ArteryThread* t)
{
	t->_proc(t->_context);
}

#if defined(ARTERY_SYNC_WIN32)
DWORD CALLBACK ArteryThreadProcImpl(void* param)
{
	CallArteryThreadProc((ArteryThread*)param);
	return 0;
}
#elif defined(ARTERY_SYNC_PS3) || defined(ARTERY_SYNC_PTHREADS)
void ArteryThreadProcImpl(uint64_t param)
{
	CallArteryThreadProc((ArteryThread*)(void*)param);
	sys_ppu_thread_exit(0);
}
#endif

ArteryThread::ArteryThread() :
	_thread(INVALID_ARTERY_THREAD_HANDLE),
	_proc(0),
	_context(0)
{
}

ArteryThread::~ArteryThread()
{
	Close();
}

bool ArteryThread::Create(ArteryThreadProc proc, void* context, int stackSize)
{
	if (_thread != INVALID_ARTERY_THREAD_HANDLE)
		return false;

	if (stackSize == 0)
		stackSize = 8 * 1024;

	_proc		= proc;
	_context	= context;

#if defined(ARTERY_SYNC_WIN32)
	return (_thread = CreateThread(0, stackSize, ArteryThreadProcImpl, (void*)this, 0, 0)) != INVALID_ARTERY_THREAD_HANDLE;
#elif defined(ARTERY_SYNC_PS3)
	if (sys_ppu_thread_create(&_thread, ArteryThreadProcImpl, (uint64_t)(void*)this, 1001, stackSize, SYS_PPU_THREAD_CREATE_JOINABLE, "Artery") != CELL_OK)
	{
		_thread = INVALID_ARTERY_THREAD_HANDLE;
		return false;
	}

	return true;
#endif
}

void ArteryThread::Close()
{
	if (_thread != INVALID_ARTERY_THREAD_HANDLE)
	{
#if defined(ARTERY_SYNC_WIN32)
		//if (wait) {
		WaitForSingleObject(_thread, INFINITE);
		//}
		CloseHandle(_thread);
#elif defined(ARTERY_SYNC_PS3)
		//if (wait) {
		uint64_t result;
		sys_ppu_thread_join(_thread, &result);
		//} else sys_ppu_thread_detach(_thread);
#endif
		_thread		= INVALID_ARTERY_THREAD_HANDLE;
		_proc		= 0;
		_context	= 0;
	}
}

//
// ArteryLock
//

ArteryLock::ArteryLock()
{
#if defined(ARTERY_SYNC_WIN32)
	InitializeCriticalSection(&_cs);
#elif defined(ARTERY_SYNC_PS3)
	sys_lwmutex_attribute_t attr;
	sys_lwmutex_attribute_initialize(attr);

	attr.attr_recursive = SYS_SYNC_RECURSIVE;

	sys_lwmutex_create(&_mutex, &attr);
#endif
}

ArteryLock::~ArteryLock()
{
#if defined(ARTERY_SYNC_WIN32)
	DeleteCriticalSection(&_cs);
#elif defined(ARTERY_SYNC_PS3)
	sys_lwmutex_destroy(&_mutex);
#endif
}

bool ArteryLock::Try()
{
#if defined(ARTERY_SYNC_WIN32)
	return TryEnterCriticalSection(&_cs) != FALSE;
#elif defined(ARTERY_SYNC_PS3)
	return sys_lwmutex_trylock(&_mutex) == CELL_OK;
#endif
}

void ArteryLock::Wait()
{
#if defined(ARTERY_SYNC_WIN32)
	EnterCriticalSection(&_cs);
#elif defined(ARTERY_SYNC_PS3)
	sys_lwmutex_lock(&_mutex, SYS_NO_TIMEOUT);
#endif
}

void ArteryLock::Release()
{
#if defined(ARTERY_SYNC_WIN32)
	LeaveCriticalSection(&_cs);
#elif defined(ARTERY_SYNC_PS3)
	sys_lwmutex_unlock(&_mutex);
#endif
}

//
// ArteryMutex
//

ArteryMutex::ArteryMutex()
#if defined(ARTERY_SYNC_WIN32)
	: _mutex(0)
#elif defined(ARTERY_SYNC_PS3)
	: _mutex(SYS_MUTEX_ID_INVALID)
#endif
{
}

ArteryMutex::~ArteryMutex()
{
	Destroy();
}

bool ArteryMutex::Create()
{
#if defined(ARTERY_SYNC_WIN32)
	if (_mutex != 0)
		return false;

	return (_mutex = CreateMutex(0, FALSE, 0)) != 0;
#elif defined(ARTERY_SYNC_PS3)
	if (_mutex != SYS_MUTEX_ID_INVALID)
		return false;

	sys_mutex_attribute_t attr;
	sys_mutex_attribute_initialize(attr);

	attr.attr_recursive = SYS_SYNC_RECURSIVE;

	if (sys_mutex_create(&_mutex, &attr) != CELL_OK)
	{
		_mutex = SYS_MUTEX_ID_INVALID;
		return false;
	}

	return true;
#endif
}

void ArteryMutex::Destroy()
{
#if defined(ARTERY_SYNC_WIN32)
	if (_mutex != 0)
	{
		CloseHandle(_mutex);
		_mutex = 0;
	}
#elif defined(ARTERY_SYNC_PS3)
	if (_mutex != SYS_MUTEX_ID_INVALID)
	{
		sys_mutex_destroy(_mutex);
		_mutex = SYS_MUTEX_ID_INVALID;
	}
#endif
}

bool ArteryMutex::Try()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_mutex != 0);

	switch(WaitForSingleObject(_mutex, 0))
	{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED_0:
		return true;

		default:
		return false;
	}
#elif defined(ARTERY_SYNC_PS3)
	assert(_mutex != SYS_MUTEX_ID_INVALID);
	return sys_mutex_trylock(_mutex) == CELL_OK;
#endif
}

void ArteryMutex::Wait()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_mutex != 0);
	WaitForSingleObject(_mutex, INFINITE);
#elif defined(ARTERY_SYNC_PS3)
	assert(_mutex != SYS_MUTEX_ID_INVALID);
	sys_mutex_lock(_mutex, SYS_NO_TIMEOUT);
#endif
}

bool ArteryMutex::Wait(int timeMsec)
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_mutex != 0);

	switch(WaitForSingleObject(_mutex, timeMsec))
	{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED_0:
		return true;

		default:
		return false;
	}
#elif defined(ARTERY_SYNC_PS3)
	assert(_mutex != SYS_MUTEX_ID_INVALID);
	return sys_mutex_lock(_mutex, 1 + (timeMsec * 1000)) == CELL_OK;
#endif
}

void ArteryMutex::Release()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_mutex != 0);
	ReleaseMutex(_mutex);
#elif defined(ARTERY_SYNC_PS3)
	assert(_mutex != SYS_MUTEX_ID_INVALID);
	sys_mutex_unlock(_mutex);
#endif
}

//
// ArteryCondVar
//

ArteryCondVar::ArteryCondVar()
#if defined(ARTERY_SYNC_WIN32)
	: _waiterCount(0),
	_state(kDestroyed),
	_allWaitersDone(0),
	_semaphore(0)
#elif defined(ARTERY_SYNC_PS3)
	: _cond(SYS_COND_ID_INVALID)
#endif
{
}

ArteryCondVar::~ArteryCondVar()
{
	Destroy();
}

bool ArteryCondVar::Create(ArteryMutex* mutex)
{
	assert(mutex != 0);

#if defined(ARTERY_SYNC_WIN32)
	if (_state != kDestroyed)
		return false;

	if (
			((_allWaitersDone = CreateEvent(0, FALSE, FALSE, 0)) == 0) ||
			((_semaphore = CreateSemaphore(0, 0, 0x7FFFFFFF, 0)) == 0)
		)
	{
		Destroy();

		return false;
	}

	_waiterCount	= 0;
	_state			= kSignal;
	_mutex			= mutex;

	InitializeCriticalSection(&_waiterLock);

	return true;
#elif defined(ARTERY_SYNC_PS3)
	if (_cond != SYS_COND_ID_INVALID)
		return false;

	sys_cond_attribute_t attr;
	sys_cond_attribute_initialize(attr);

	if (sys_cond_create(&_cond, mutex->_mutex, &attr) != CELL_OK)
	{
		_cond = SYS_COND_ID_INVALID;
		return false;
	}

	return true;
#endif
}

void ArteryCondVar::Destroy()
{
#if defined(ARTERY_SYNC_WIN32)
	if (_state != kDestroyed)
	{
		DeleteCriticalSection(&_waiterLock);

		_waiterCount	= 0;
		_state			= kDestroyed;
	}
	
	if (_allWaitersDone != 0)
	{
		CloseHandle(_allWaitersDone);
		_allWaitersDone = 0;
	}

	if (_semaphore != 0)
	{
		CloseHandle(_semaphore);
		_semaphore = 0;
	}
#elif defined(ARTERY_SYNC_PS3)
	if (_cond != SYS_COND_ID_INVALID)
	{
		sys_cond_destroy(_cond);
		_cond = SYS_COND_ID_INVALID;
	}
#endif
}

void ArteryCondVar::Signal()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_state != kDestroyed);

	EnterCriticalSection(&_waiterLock);

	bool canRelease = _waiterCount > 0;

	LeaveCriticalSection(&_waiterLock);

	if (canRelease)
		ReleaseSemaphore(_semaphore, 1, 0);
#elif defined(ARTERY_SYNC_PS3)
	assert(_cond != SYS_COND_ID_INVALID);
	sys_cond_signal(_cond);
#endif
}

void ArteryCondVar::Broadcast()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_state != kDestroyed);

	EnterCriticalSection(&_waiterLock);

	if (_waiterCount <= 0)
	{
		LeaveCriticalSection(&_waiterLock);
		return;
	}

	_state = kBroadcast;

	ReleaseSemaphore(_semaphore, _waiterCount, 0);

	LeaveCriticalSection(&_waiterLock);

	WaitForSingleObject(_allWaitersDone, INFINITE);

	_state = kSignal;
#elif defined(ARTERY_SYNC_PS3)
	assert(_cond != SYS_COND_ID_INVALID);
	sys_cond_signal_all(_cond);
#endif
}

void ArteryCondVar::Wait()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_state != kDestroyed);
	assert((_mutex != 0) && (_mutex->_mutex != 0));

	WaitImpl(_mutex, INFINITE);
#elif defined(ARTERY_SYNC_PS3)
	assert(_cond != SYS_COND_ID_INVALID);
	sys_cond_wait(_cond, SYS_NO_TIMEOUT);
#endif
}

bool ArteryCondVar::Wait(int timeMsec)
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_state != kDestroyed);
	assert((_mutex != 0) && (_mutex->_mutex != 0));

	return WaitImpl(_mutex, timeMsec);
#elif defined(ARTERY_SYNC_PS3)
	assert(_cond != SYS_COND_ID_INVALID);
	return sys_cond_wait(_cond, 1 + (timeMsec * 1000)) == CELL_OK;
#endif
}

#if defined(ARTERY_SYNC_WIN32)
bool ArteryCondVar::WaitImpl(ArteryMutex* mutex, DWORD timeMsec)
{
	EnterCriticalSection(&_waiterLock);
	_waiterCount++;
	LeaveCriticalSection(&_waiterLock);

	bool result = false;

	switch(SignalObjectAndWait(mutex->_mutex, _semaphore, timeMsec, FALSE))
	{
		case WAIT_OBJECT_0:
		case WAIT_ABANDONED_0:
			result = true;
		break;
	}

	EnterCriticalSection(&_waiterLock);

	_waiterCount--;

	bool wakeFromBroadcast = (_state == kBroadcast) && (_waiterCount <= 0);

	LeaveCriticalSection(&_waiterLock);

	if (wakeFromBroadcast)
		SignalObjectAndWait(_allWaitersDone, mutex->_mutex, INFINITE, FALSE);
	else
		WaitForSingleObject(mutex->_mutex, INFINITE);

	return result;
}
#endif

//
// ArteryFlag
//

ArteryFlag::ArteryFlag()
#if defined(ARTERY_SYNC_WIN32)
	: _event(0)
#elif defined(ARTERY_SYNC_PS3)
	: _flag(false),
	_init(false)
#endif
{
}

ArteryFlag::~ArteryFlag()
{
	Destroy();
}

bool ArteryFlag::Create()
{
#if defined(ARTERY_SYNC_WIN32)
	if (_event != 0)
		return false;

	return (_event = CreateEvent(0, TRUE, FALSE, 0)) != 0;
#elif defined(ARTERY_SYNC_PS3)
	if (_init)
		return false;

	if (!_mutex.Create() || !_cond.Create(&_mutex))
	{
		Destroy();
		return false;
	}

	_flag = false;
	_init = true;

	return true;
#endif
}

void ArteryFlag::Destroy()
{
#if defined(ARTERY_SYNC_WIN32)
	if (_event != 0)
	{
		CloseHandle(_event);
		_event = 0;
	}
#elif defined(ARTERY_SYNC_PS3)
	_mutex.Destroy();
	_cond.Destroy();

	_flag = false;
	_init = false;
#endif
}

void ArteryFlag::Lower()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	ResetEvent(_event);
#elif defined(ARTERY_SYNC_PS3)
	assert(_init != false);

	_mutex.Wait();

	_flag = false;

	_cond.Broadcast();
	_mutex.Release();
#endif
}

void ArteryFlag::Raise()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	SetEvent(_event);
#elif defined(ARTERY_SYNC_PS3)
	assert(_init != false);

	_mutex.Wait();

	_flag = true;

	_cond.Broadcast();
	_mutex.Release();
#endif
}

bool ArteryFlag::Try()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	return WaitForSingleObject(_event, 0) == WAIT_OBJECT_0;
#elif defined(ARTERY_SYNC_PS3)
	assert(_init != false);

	_mutex.Wait();
	bool result = _flag;
	_mutex.Release();

	return result;
#endif
}

void ArteryFlag::Wait()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	WaitForSingleObject(_event, INFINITE);
#elif defined(ARTERY_SYNC_PS3)
	assert(_init != false);

	_mutex.Wait();

	while(!_flag)
		_cond.Wait();

	_mutex.Release();
#endif
}

bool ArteryFlag::Wait(int timeMsec)
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	return WaitForSingleObject(_event, timeMsec) == WAIT_OBJECT_0;
#elif defined(ARTERY_SYNC_PS3)
	assert(_init != false);

	_mutex.Wait();

	if (!_flag)
		_cond.Wait(timeMsec);

	bool result = _flag;

	_mutex.Release();

	return result;
#endif
}

//
// ArteryEvent
//

ArteryEvent::ArteryEvent()
#if defined(ARTERY_SYNC_WIN32)
	: _event(0)
#elif defined(ARTERY_SYNC_PS3)
	: _flag(SYS_EVENT_FLAG_ID_INVALID)
#endif
{
}

ArteryEvent::~ArteryEvent()
{
	Destroy();
}

bool ArteryEvent::Create()
{
#if defined(ARTERY_SYNC_WIN32)
	if (_event != 0)
		return false;

	return (_event = CreateEvent(0, FALSE, FALSE, 0)) != 0;
#elif defined(ARTERY_SYNC_PS3)
	if (_flag != SYS_EVENT_FLAG_ID_INVALID)
		return false;

	sys_event_flag_attribute_t attr;
	sys_event_flag_attribute_initialize(attr);

	attr.type = SYS_SYNC_WAITER_MULTIPLE;

	if (sys_event_flag_create(&_flag, &attr, 0) != CELL_OK)
	{
		_flag = SYS_EVENT_FLAG_ID_INVALID;
		return false;
	}

	return true;
#endif
}

void ArteryEvent::Destroy()
{
#if defined(ARTERY_SYNC_WIN32)
	if (_event != 0)
	{
		CloseHandle(_event);
		_event = 0;
	}
#elif defined(ARTERY_SYNC_PS3)
	if (_flag != SYS_EVENT_FLAG_ID_INVALID)
	{
		sys_event_flag_destroy(_flag);
		_flag = SYS_EVENT_FLAG_ID_INVALID;
	}	
#endif
}

void ArteryEvent::Lower()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	ResetEvent(_event);
#elif defined(ARTERY_SYNC_PS3)
	assert(_flag != SYS_EVENT_FLAG_ID_INVALID);

	sys_event_flag_clear(_flag, 1);
#endif
}

void ArteryEvent::Raise()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	SetEvent(_event);
#elif defined(ARTERY_SYNC_PS3)
	assert(_flag != SYS_EVENT_FLAG_ID_INVALID);

	sys_event_flag_set(_flag, 1);
#endif
}

bool ArteryEvent::Try()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	return WaitForSingleObject(_event, 0) == WAIT_OBJECT_0;
#elif defined(ARTERY_SYNC_PS3)
	assert(_flag != SYS_EVENT_FLAG_ID_INVALID);

	uint64_t result = 0;

	if (sys_event_flag_trywait(_flag, 1, SYS_EVENT_FLAG_WAIT_OR | SYS_EVENT_FLAG_WAIT_CLEAR, &result) == CELL_OK)
		return true;

	return false;
#endif
}

void ArteryEvent::Wait()
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	WaitForSingleObject(_event, INFINITE);
#elif defined(ARTERY_SYNC_PS3)
	assert(_flag != SYS_EVENT_FLAG_ID_INVALID);

	uint64_t result = 0;

	sys_event_flag_wait(_flag, 1, SYS_EVENT_FLAG_WAIT_OR | SYS_EVENT_FLAG_WAIT_CLEAR, &result, SYS_NO_TIMEOUT);
#endif
}

bool ArteryEvent::Wait(int timeMsec)
{
#if defined(ARTERY_SYNC_WIN32)
	assert(_event != 0);

	return WaitForSingleObject(_event, timeMsec) == WAIT_OBJECT_0;
#elif defined(ARTERY_SYNC_PS3)
	assert(_flag != SYS_EVENT_FLAG_ID_INVALID);

	uint64_t result = 0;

	if (sys_event_flag_wait(_flag, 1, SYS_EVENT_FLAG_WAIT_OR | SYS_EVENT_FLAG_WAIT_CLEAR, &result, 1 + (timeMsec * 1000)) == CELL_OK)
		return true;

	return false;
#endif
}
