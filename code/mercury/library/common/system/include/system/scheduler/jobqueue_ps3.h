#ifndef JOBQUEUE_PS3_H
#define JOBQUEUE_PS3_H

#include "system/scheduler/core/tl_defs.h"
#include "system/scheduler/core/tl_thread.h"
#include <core/Containers.h>
#include "system/Perf/Profiler.h"
#include "system/Perf/ProfilerSPU.h"

#include <cell/atomic.h>
#include <cell/spurs.h>

#define JQ_ALIGNED      TL_ALIGNED(16)
#define JQ_DMA_ALIGNED  TL_ALIGNED(128)

#if TL_PLAT_PS3_SPU
  #define JQ_SPU_ALIGNED    JQ_ALIGNED
#else
  #define JQ_SPU_ALIGNED
#endif

namespace Scheduler
{

enum jqProcessor
{
	JQ_CORE_PPU_PRIMARY	= 1 << 0,
	JQ_CORE_PPU_SECONDARY	= 1 << 1,
	JQ_CORE_SPU_0			= 1 << 2,
	JQ_CORE_SPU_1			= 1 << 3,
	JQ_CORE_SPU_2			= 1 << 4,
	JQ_CORE_SPU_3			= 1 << 5,
	JQ_CORE_SPU_4			= 1 << 6,
	JQ_CORE_SPU_5			= 1 << 7,
	JQ_CORE_ALL =			JQ_CORE_PPU_PRIMARY |
							JQ_CORE_PPU_SECONDARY |
							JQ_CORE_SPU_0 |
							JQ_CORE_SPU_1 |
							JQ_CORE_SPU_2 |
							JQ_CORE_SPU_3 |
							JQ_CORE_SPU_4 |
							JQ_CORE_SPU_5,
	JQ_CORE_ALL_SPU =		JQ_CORE_SPU_0 |
							JQ_CORE_SPU_1 |
							JQ_CORE_SPU_2 |
							JQ_CORE_SPU_3 |
							JQ_CORE_SPU_4 |
							JQ_CORE_SPU_5
};

//! Pointer to the function that is responsible for actually processing a batch of data.
typedef void (*jqFunctor)( struct Task* Batch );

struct jqModule
{
	// Constructors
	jqModule() { Init(NULL, NULL, NULL, NULL, 0, NULL); }
	jqModule(const char* _Name, jqFunctor _PPUCode, const char* _ELFName, void* _Code, unsigned _CodeSize, const char *pszLogger)
	{
		Init(_Name, _PPUCode, _ELFName, _Code, _CodeSize, pszLogger);
	}
	jqModule(const char* _Name, jqFunctor _PPUCode)
	{
		Init(_Name, _PPUCode, NULL, NULL, 0, NULL);
	}
	jqModule(const char* _Name, const char* _ELFName, void* _Code, unsigned _CodeSize, const char *pszLogger)
	{
		Init(_Name, NULL, _ELFName, _Code, _CodeSize, pszLogger);
	}

	void Init(const char* _Name, jqFunctor _PPUCode, const char* _ELFName, void* _Code, unsigned _CodeSize, const char *pszLogger);

	const char* Name;
	const char* ELFName;
	void* Code;
	unsigned CodeSize;
	jqFunctor PPUCode;

	bool CodeReplaced;

	static jqModule* Head;
	jqModule* Next;
	int pLogger;
};

//	this class provides a way for the SPU module code to call jobqueue functionality supplied by the policy module
class jqSpuInterface
{
	public:

		virtual void AddBatch(Task& Batch, int Priority );
		virtual ~jqSpuInterface() {}  //	not really needed but avoids a warning
};

// Implementation specific batch fields.
struct _Task
{
	void* Code;
	unsigned CodeSize;
	int pLogger;
	int iBreakIntoDebugger;
	U32 pszFlushString;
};

enum
{
	JQ_ASSERT_NONE = 0,
	JQ_ASSERT_DISPLAY = 1,
	JQ_ASSERT_HALT = 2,
};

struct _jqWorker
{
	TaskPool* PoolPtr;

	int Processor;		// for SPUs this is a number from 0 to 5

	void* BatchDataHeap;

	int MemBase;
	int MemLimit;
	
	uint64_t pLogger;	// contains current logger pointer and level
	void *PPUModuleAddress;
	uint64_t ppustringdata;

	Perf::SPUData profilerInfo;
	U32 bIsDebuggerAttached;
	U32 bBreakInDebugger;
	U32 *pAlert;
} JQ_ALIGNED;

// Kernel memory layout.
#define JQ_LOCAL_MEMORY_SIZE (256*1024)

#define JQ_KERNEL_BASE    (0xa00)
#define JQ_KERNEL_WORKER  (0x3600)
#define JQ_KERNEL_BATCHES (0x3800)
#define JQ_KERNEL_END     (0x4000)

#define JQ_MODULE_BASE    (0x4000)

// DMA tag used by batch data heap.  Not to be used by modules!
#define JQ_TAG_BATCH_DATA_HEAP 31

// SPU module implementation
#ifdef TL_PLAT_PS3_SPU

static jqWorker* const jqThisWorker = ((jqWorker*)JQ_KERNEL_WORKER);
static _jqWorker* const _jqThisWorker = ((_jqWorker*)JQ_KERNEL_WORKER);

extern int jqSPUID;

#if !CFG_SPEED
#define JQ_MODULE_BREAKINDEBUGGER	\
		if (Scheduler::_jqThisWorker->bBreakInDebugger)	\
			DebugBreak();
#else
#define JQ_MODULE_BREAKINDEBUGGER
#endif

#define JQ_SPU_MODULE_ENTRY(name,fn) \
	extern "C" void jq_main(); \
	char gsz_jobmodulename [] __attribute__((section("ppudata"))) = #name; \
	void jq_main() \
	{ \
		Perf::BeginNamedEvent(gsz_jobmodulename); \
		JQ_MODULE_BREAKINDEBUGGER \
		fn((Scheduler::Task*)JQ_KERNEL_BATCHES); \
		Perf::EndNamedEvent(); \
	}

#define JQ_NO_PPU NULL
// JQ_NO_SPU not an option on SPU platform.
//#define JQ_NO_SPU none

#define JQ_DECLARE_MODULE(name,fn,elfname) \
	JQ_SPU_MODULE_ENTRY(name,fn)

#define JQ_DECLARE_PPU_MODULE(name,fn)

#define JQ_DECLARE_SPU_MODULE(name,fn,elfname,logger) \
	JQ_SPU_MODULE_ENTRY(name,fn)

#define JQ_DECLARE_DUAL_MODULE(name,fn,elfname,logger) \
	JQ_SPU_MODULE_ENTRY(name,fn)

#undef snPause
#include <sn/libsn_spu.h>

#define tlFatal(format, ...) \
	do { \
		_jqThisWorker->AssertState = JQ_ASSERT_DISPLAY; \
		spu_printf(format, ## __VA_ARGS__); \
	} while(0);

#endif

// PPU module implementation
#if TL_PLAT_PS3

CellSpurs* jqGetSpurs();

#define JQ_MODULE_START_NAME(name) _binary_##name##_elf_start
#define JQ_MODULE_SIZE_NAME(name)  _binary_##name##_elf_size

#define JQ_MODULE_START(name) ((void*)(JQ_MODULE_START_NAME(name)))
#define JQ_MODULE_SIZE(name)  ((size_t)(JQ_MODULE_SIZE_NAME(name)))

#define JQ_NO_PPU NULL
#define JQ_NO_SPU none

#define JQ_EXTERN_SPU_MODULE(elfname) \
	extern "C" { extern char JQ_MODULE_START_NAME(elfname)[]; extern char JQ_MODULE_SIZE_NAME(elfname)[]; }

#define JQ_DECLARE_MODULE(name,fn,elfname) \
	JQ_EXTERN_SPU_MODULE(elfname) \
	Scheduler::jqModule name(#name, fn, #elfname, JQ_MODULE_START(elfname), JQ_MODULE_SIZE(elfname), 0);

#define JQ_DECLARE_PPU_MODULE(name,fn) \
	Scheduler::jqModule name(#name, fn, NULL, NULL, 0, 0);

#define JQ_DECLARE_SPU_MODULE(name,fn,elfname,logger) \
	JQ_EXTERN_SPU_MODULE(elfname) \
	Scheduler::jqModule name(#name, NULL, #elfname, JQ_MODULE_START(elfname), JQ_MODULE_SIZE(elfname), logger);

#define JQ_DECLARE_DUAL_MODULE(name,fn,elfname,logger) \
	JQ_EXTERN_SPU_MODULE(elfname) \
	Scheduler::jqModule name(#name, fn, #elfname, JQ_MODULE_START(elfname), JQ_MODULE_SIZE(elfname), logger);

#endif

#define jqPrintf tlPrintf

#define JQ_ASSERT(x, desc) TL_ASSERT(x, desc)

}

#ifdef PLATFORM_SPU
extern char gsz_jobmodulename [];	// string in PPU memory containing the name of the job
#endif

#endif
