#include "SystemPch.h"
#include <core/Core.h>
#include <System/DebugLog.h>
#include "system/scheduler/jobqueue.h"

#if TL_PLAT_PS3_SPU
void jqCheckStack()
{
#ifndef CFG_GOLD
	int x;
//	if ((U32)&x < Scheduler::_jqThisWorker->MemLimit)
//		LOGFATAL(0,"SPU %d stack overflow!  Used=%d Size=%d", cellSpursGetCurrentSpuId(), JQ_LOCAL_MEMORY_SIZE - (U32)&x, JQ_LOCAL_MEMORY_SIZE - Scheduler::_jqThisWorker->MemLimit);
	if ((U32)&x < (U32)Scheduler::_jqThisWorker->MemBase)
	{
		sLOGERROR(0, "SPU %s stack overflow!", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
		//LOGFATAL(0,"SPU stack overflow! Used=%d Size=%d Addr=%d MemBase=%d MemLimit=%d", JQ_LOCAL_MEMORY_SIZE - (U32)&x, JQ_LOCAL_MEMORY_SIZE - Scheduler::_jqThisWorker->MemLimit, &x, Scheduler::_jqThisWorker->MemBase, Scheduler::_jqThisWorker->MemLimit);
	}

#endif
}
#endif


namespace Perf
{
extern int g_iDMAWaitTime;
}

namespace Spu
{

void* Fetch(void* dest, const void* src, unsigned size) 
{
	jqCheckStack();
	if (!size) return dest;

#if TL_PLAT_PS3_SPU
#ifndef CFG_SPEED
	if (__builtin_expect( (dest == NULL), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch dest (LS) is NULL.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( (src == NULL), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) is NULL.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)src < (256*1024)), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) is < 256k.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)dest&15), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch destination (LS) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)src&15), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)dest > (256*1024)), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch dest (LS) is > 256k.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
#endif
#if CFG_PROFILE
	U64 time = spu_read_decrementer();
#endif
	cellDmaLargeGet(dest, (uint64_t)(uint32_t)src, size, 0, 0, 0);
	cellDmaWaitTagStatusAll(1<<0);
#if CFG_PROFILE
	U64 newtime = spu_read_decrementer();
	Perf::g_iDMAWaitTime += time-newtime;
#endif
#else
	memcpy(dest, src, size);
#endif
	return dest;
}

void* FetchAligned(void* dest, const void* src, unsigned size)
{
	jqCheckStack();
	if (!size) return dest;

#if TL_PLAT_PS3_SPU
#ifndef CFG_SPEED
	if (__builtin_expect( (dest == NULL), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch dest (LS) is NULL.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( (src == NULL), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) is NULL.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)src < (256*1024)), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) is < 256k.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)dest&15), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch destination (LS) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( (size&15), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch size not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( (size>16*1024), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch size >16K.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)src&15), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)dest > (256*1024)), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch dest (LS) is > 256k.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
#endif
#if CFG_PROFILE
	U64 time = spu_read_decrementer();
#endif
	cellDmaGet(dest, (uint64_t)(uint32_t)src, size, 0, 0, 0);
	cellDmaWaitTagStatusAll(1<<0);
#if CFG_PROFILE
	U64 newtime = spu_read_decrementer();
	Perf::g_iDMAWaitTime += time-newtime;
#endif
#else
	memcpy(dest, src, size);
#endif
	return dest;
}

void Store(void* dest, const void* src, unsigned size) 
{
	jqCheckStack();
	if (!size) return;
#if TL_PLAT_PS3_SPU
#ifndef CFG_SPEED
	if (__builtin_expect( (src == NULL) , 0 ))
	{
		sLOGFATAL(0,"SPU %s store source (LS) is NULL.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( (dest == NULL), 0 ))
	{
		sLOGFATAL(0,"SPU %s store destination (Main) is NULL.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( ((U32)dest&15), 0 ))
	{
		sLOGFATAL(0,"SPU %s store destination (Main) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( ((U32)src&15), 0 ))
	{
		sLOGFATAL(0,"SPU %s store source (LS) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( (size&15), 0 ))
	{
		sLOGFATAL(0,"SPU %s store size not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( ((U32)src > (256*1024)), 0 ))
	{
		sLOGFATAL(0,"SPU %s store source (LS) is > 256k.", PPUDATA_STRING(gsz_jobmodulename));
	}
#endif
	cellDmaLargePut(src, (uint64_t)(uint32_t)dest, size, 0, 0, 0);
	cellDmaWaitTagStatusAll(1<<0);
#else
	memcpy(dest, src, size);
#endif
}

void* FetchAsync(void* dest, const void* src, unsigned size, unsigned tag) 
{
	jqCheckStack();
	if (!size) return dest;
#if TL_PLAT_PS3_SPU
#ifndef CFG_SPEED
	if (__builtin_expect( (dest == NULL), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch dest (LS) is NULL.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( (src == NULL), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) is NULL.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)src < (256*1024)), 0))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) is < 256k.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)dest > (256*1024)), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch dest (LS) is > 256k.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)dest&15), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch destination (LS) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( ((U32)src&15), 0 ))
	{
		sLOGERROR(0,"SPU %s fetch source (Main) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( (tag < 0 || tag >= 31), 0 ))
	{
		sLOGERROR(0,"SPU %s invalid async fetch tag.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
#endif
	cellDmaLargeGet(dest, (uint64_t)(uint32_t)src, size, tag, 0, 0);
#else
	memcpy(dest, src, size);
#endif
	return dest;
}

void StoreAsync(void* dest, const void* src, unsigned size, unsigned tag) 
{
	jqCheckStack();
	if (!size) return;
#if TL_PLAT_PS3_SPU
#ifndef CFG_SPEED
	if (__builtin_expect( ((U32)dest&15), 0 ))
	{
		sLOGFATAL(0,"SPU %s store destination (Main) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( ((U32)src&15), 0 ))
	{
		sLOGFATAL(0,"SPU %s store source (LS) not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( (size&15), 0 ))
	{
		sLOGFATAL(0,"SPU %s store size not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( (tag < 0 || tag >= 31), 0 ))
	{
		sLOGFATAL(0,"SPU %s invalid async store tag.", PPUDATA_STRING(gsz_jobmodulename));
	}
#endif
	cellDmaLargePut(src, (uint64_t)(uint32_t)dest, size, tag, 0, 0);
#else
	memcpy(dest, src, size);
#endif
}

#if TL_PLAT_PS3_SPU
void Wait(unsigned tag) 
{
	jqCheckStack();

#ifndef CFG_SPEED
	if (__builtin_expect( (tag < 0 || tag >= 31), 0 ))
	{
		sLOGERROR(0,"SPU %s invalid async wait tag.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
#endif
#if CFG_PROFILE
	U64 time = spu_read_decrementer();
#endif
	cellDmaWaitTagStatusAll(1<<tag);
#if CFG_PROFILE
	U64 newtime = spu_read_decrementer();
	Perf::g_iDMAWaitTime += time-newtime;
#endif

}
#endif

#if TL_PLAT_PS3_SPU
void WaitMultiple(unsigned mask) 
{
	jqCheckStack();
	cellDmaWaitTagStatusAll(mask);

}
#endif

#if TL_PLAT_PS3_SPU
void SetMemBase(const void* ptr) 
{
	jqCheckStack();
#ifndef CFG_SPEED
	if (__builtin_expect( ((U32)ptr&15), 0))
	{
		sLOGFATAL(0,"SPU %s allocation base not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( ((U32)ptr < JQ_MODULE_BASE || (U32)ptr >= (U32)Scheduler::_jqThisWorker->MemLimit), 0))
	{
		sLOGFATAL(0,"SPU %s memory allocator new base outside valid range.", PPUDATA_STRING(gsz_jobmodulename));
	}
#endif
	Scheduler::_jqThisWorker->MemBase = (unsigned)(size_t)(ptr);
}
#endif

#if TL_PLAT_PS3_SPU
void SetStackSize(int size) 
{
	jqCheckStack();
#ifndef CFG_SPEED
	if (__builtin_expect( (size&15), 0 ))
	{
		sLOGFATAL(0,"SPU %s stack size not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
	}
	if (__builtin_expect( (JQ_LOCAL_MEMORY_SIZE-size < Scheduler::_jqThisWorker->MemLimit), 0 ))
	{
		sLOGFATAL(0,"SPU %s stack size overlaps already allocated memory base.", PPUDATA_STRING(gsz_jobmodulename));
	}
#endif
	Scheduler::_jqThisWorker->MemLimit = JQ_LOCAL_MEMORY_SIZE - size;
}
#endif
#if TL_PLAT_PS3_SPU
int GetMemAvailable()
{
	jqCheckStack();
	return Scheduler::_jqThisWorker->MemLimit - Scheduler::_jqThisWorker->MemBase;
}
#endif

#if TL_PLAT_PS3_SPU
void* Alloc(unsigned size) 
{
	jqCheckStack();
#ifndef CFG_SPEED
	if (__builtin_expect( (Scheduler::_jqThisWorker->MemBase&15), 0 ))
	{
		sLOGERROR(0,"SPU %s allocation base not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( (size&15), 0 ))
	{
		sLOGERROR(0,"SPU %s allocation size not 16byte aligned.", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
	}
	if (__builtin_expect( (Scheduler::_jqThisWorker->MemBase+size > (U32)Scheduler::_jqThisWorker->MemLimit), 0 ))
	{
		sLOGERROR(0,"SPU %s out of memory", PPUDATA_STRING(gsz_jobmodulename));
		assert(0);
		//LOGFATAL(0,"SPU %d out of memory! Size=%d Used=%d Avail=%d", cellSpursGetCurrentSpuId(), size, (U32)Scheduler::_jqThisWorker->MemBase, (U32)Scheduler::_jqThisWorker->MemLimit-(U32)Scheduler::_jqThisWorker->MemBase);
	}

#endif
	void* dest = (void*)Scheduler::_jqThisWorker->MemBase;
	Scheduler::_jqThisWorker->MemBase += size;
	return dest;
}
#endif

#if TL_PLAT_PS3_SPU
void* GetMemBase()
{
	jqCheckStack();
	return (void*)Scheduler::_jqThisWorker->MemBase;
}
#endif

}; // namespace Spu
