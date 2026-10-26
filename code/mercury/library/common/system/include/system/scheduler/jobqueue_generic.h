#ifndef JOBQUEUE_GENERIC_H
#define JOBQUEUE_GENERIC_H

#include "system/scheduler/core/tl_defs.h"
#include "system/scheduler/core/tl_thread.h"

enum jqProcessor
{
    JQ_PROCESSOR_CPU,
	JQ_CORE_ALL = JQ_PROCESSOR_CPU
};

typedef int interlocked_size_t;

inline interlocked_size_t interlocked_increment(volatile interlocked_size_t* target) {
	return (*target)++;
}

inline interlocked_size_t interlocked_decrement(volatile interlocked_size_t* target) {
	return (*target)--;
}

inline interlocked_size_t interlocked_exchange_add(volatile interlocked_size_t* target, interlocked_size_t value) {
	*target += value;
	return *target - value;
}

inline interlocked_size_t interlocked_exchange_sub(volatile interlocked_size_t* target, interlocked_size_t value) {
	*target -= value;
	return *target + value;
}

inline interlocked_size_t interlocked_compare_exchange(volatile interlocked_size_t* target, interlocked_size_t expectedVal, interlocked_size_t newVal) {
	*target = newVal;
	return expectedVal;
}



#if defined(JQ_ENABLE_PS2_CRITICAL_SECTIONS)
	#include <eekernel.h>
	#define JQ_DECLARE_CRITICAL_SECTION(name)	extern int gLock_##name;extern bool gLockInit_##name;JQ_DECLARE_CS_TIMING(name);
	#define JQ_DEFINE_CRITICAL_SECTION(name)	int gLock_##name;bool gLockInit_##name=false;JQ_DEFINE_CS_TIMING(name);
	#define JQ_START_CRITICAL_SECTION(name)		JQ_START_CS_TIMING(name);if (!gLockInit_##name) {struct SemaParam param;param.maxCount = 1;param.option = 0;param.initCount = 1;gLock_##name = CreateSema( &param );gLockInit_##name=true;} WaitSema(&gLock_##name);JQ_STOP_CS_TIMING(name);
	#define JQ_END_CRITICAL_SECTION(name)		SignalSema(&gLock_##name);
	//someone put in the decl for thread local storage on the ps2 (if it even exists)
	#define JQ_TLS
#else
	#define JQ_DECLARE_CRITICAL_SECTION(name)
	#define JQ_DEFINE_CRITICAL_SECTION(name)
	#define JQ_START_CRITICAL_SECTION(name)
	#define JQ_END_CRITICAL_SECTION(name)
	#define JQ_TLS
#endif

typedef int jqMutex;

// The spin count is used to control how long a thread will wait on a mutex before going to sleep.
// Since the JobQueue generally owns the cores that it runs on, the default is to avoid sleeping as much as possible.
enum
{
	JQ_DEFAULT_SPIN_COUNT = 4096,
};

inline jqMutex jqCreateMutex(int spincount = JQ_DEFAULT_SPIN_COUNT)
{
	return 0;
}

inline void jqDestroyMutex(jqMutex mutex)
{
}

inline void jqLockMutex(jqMutex mutex)
{
}

inline void jqUnlockMutex(jqMutex mutex)
{
}

struct Task;
typedef int TaskHandle;
typedef int GroupHandle;

//! Pointer to the function that is responsible for actually processing a batch of data.
typedef void (*jqFunctor)( struct Task* Batch );

struct jqModule
{
	jqModule() : Code( NULL ), Name( NULL ) {}
	jqModule(jqFunctor Code, const char* Name)
		: Code(Code), Name(Name)
	{}
	jqFunctor Code;
	const char* Name;
};

typedef int TaskHandle;

struct _Task
{
};

struct _jqWorker
{
	int Processor;
	void* Thread;
};

#define jqPrintf tlPrintf

#define JQ_ASSERT(x, desc) TL_ASSERT(x, desc)

#define JQ_ALIGNED
#define JQ_DMA_ALIGNED

#define JQ_DECLARE_MODULE(name,fn,cppname) \
	jqModule name(fn,#name);

#define JQ_DECLARE_PPU_MODULE(name,fn) \
	jqModule name(fn,#name);


#endif // JOBQUEUE_GENERIC_H
