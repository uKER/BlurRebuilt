#ifndef ARTERYSYNC_H
#define ARTERYSYNC_H

#ifdef _MSC_VER
#pragma once
#endif

#if defined(PLATFORM_PC)
#define ARTERY_SYNC_WIN32
#elif defined(PLATFORM_PS3)
#define ARTERY_SYNC_PS3
#elif defined(PLATFORM_LINUX)
#define ARTERY_SYNC_PTHREADS
#else
#error Platform not supported.
#endif

//
// ArteryThread
//

typedef void (*ArteryThreadProc)(void*);

#if defined(ARTERY_SYNC_WIN32)
	typedef HANDLE ArteryThreadHandle;
	#define INVALID_ARTERY_THREAD_HANDLE (0)
#elif defined(ARTERY_SYNC_PS3)
	typedef sys_ppu_thread_t ArteryThreadHandle;
	#define INVALID_ARTERY_THREAD_HANDLE (SYS_PPU_THREAD_ID_INVALID)
#endif

class ArteryThread
{
public:
	ArteryThread();
	~ArteryThread();

public:
	bool Create(ArteryThreadProc proc, void* context, int stackSize = 0);
	void Close();

private:
	ArteryThreadHandle	_thread;
	ArteryThreadProc	_proc;
	void*				_context;

friend void CallArteryThreadProc(ArteryThread* t);
};

//
// ArteryLock
//

class ArteryLock
{
public:
	ArteryLock();
	~ArteryLock();

public:
	bool Try();
	void Wait();
	void Release();

private:
#if defined(ARTERY_SYNC_WIN32)
	CRITICAL_SECTION _cs;
#elif defined(ARTERY_SYNC_PS3)
	sys_lwmutex_t _mutex;
#endif
};

//
// ArteryMutex
//

class ArteryMutex
{
public:
	ArteryMutex();
	~ArteryMutex();

public:
	bool Create();
	void Destroy();

public:
	bool Try();
	void Wait();
	bool Wait(int timeMsec);
	void Release();

private:
#if defined(ARTERY_SYNC_WIN32)
	HANDLE _mutex;
#elif defined(ARTERY_SYNC_PS3)
	sys_mutex_t _mutex;
#endif

friend class ArteryCondVar;
};

//
// ArteryCondVar
//

class ArteryCondVar
{
public:
	ArteryCondVar();
	~ArteryCondVar();

public:
	bool Create(ArteryMutex* mutex);
	void Destroy();

public:
	void Signal();
	void Broadcast();

public:
	void Wait();
	bool Wait(int timeMsec);

private:
#if defined(ARTERY_SYNC_WIN32)
	bool WaitImpl(ArteryMutex* mutex, DWORD timeMsec);

	enum CvState
	{
		kDestroyed,
		kSignal,
		kBroadcast
	};

	CRITICAL_SECTION	_waiterLock;
	int					_waiterCount;
	CvState				_state;
	HANDLE				_allWaitersDone;
	HANDLE				_semaphore;
	ArteryMutex*		_mutex;
#elif defined(ARTERY_SYNC_PS3)
	sys_cond_t _cond;
#endif
};

//
// ArteryFlag
//

class ArteryFlag
{
public:
	ArteryFlag();
	~ArteryFlag();

public:
	bool Create();
	void Destroy();

public:
	void Lower();
	void Raise();

public:
	bool Try();
	void Wait();
	bool Wait(int timeMsec);

private:
#if defined(ARTERY_SYNC_WIN32)
	HANDLE _event;
#elif defined(ARTERY_SYNC_PS3)
	ArteryMutex		_mutex;
	ArteryCondVar	_cond;
	bool			_flag;
	bool			_init;
#endif
};

//
// ArteryEvent
//

class ArteryEvent
{
public:
	ArteryEvent();
	~ArteryEvent();

public:
	bool Create();
	void Destroy();

public:
	void Lower();
	void Raise();

public:
	bool Try();
	void Wait();
	bool Wait(int timeMsec);

private:
#if defined(ARTERY_SYNC_WIN32)
	HANDLE _event;
#elif defined(ARTERY_SYNC_PS3)
	sys_event_flag_t _flag;
#endif
};

#endif // ARTERYSYNC_H
