// DWBS ------------------------------------------------------------------ DWBS
//
// PLATFORM		: WIN32
// PRODUCT		: BIT_DEMON
// VISIBILITY	: PUBLIC
//
// DWBS ------------------------------------------------------------------ DWBS

#include <bdCore/bdMemory/bdDefaultMemory.h>

#if defined (BD_DEBUG_MEMORY)

#pragma comment(lib, "dbghelp")

#include <bdCore/bdMemory/bdDebugMemory.h>

#include <bdPlatform/bdPlatformMemory/bdPlatformMemory.h>
#include <bdPlatform/bdPlatformString/bdPlatformString.h>

#include <bdCore/bdMemory/bdAlignedOffsetMemory.h>
#include <bdCore/bdString/bdString.h>

bdDebugMemory::bdMemoryChainElement *bdDebugMemory::m_memoryChain = BD_NULL;
bdUWord bdDebugMemory::m_allocatedBytes = 0;
bdUInt bdDebugMemory::m_numAllocations = 0;

bdBool bdDebugMemory::m_initialized = false;
bdBool bdDebugMemory::m_recording = true;

void* BD_CALL bdDebugMemory::allocate(const bdUWord size)
{
	const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

	bdMemoryChainElement* link =  reinterpret_cast<bdMemoryChainElement*>(bdMalloc(totalSize));

	return recordMemory(link, size, false);
}

void BD_CALL bdDebugMemory::deallocate(void *p)
{
	if(p)
	{
		bdMemoryChainElement* link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;

		eraseMemory(link);

		if(m_recording)
		{
			m_recording = false;
			BD_ASSERT(link->m_aligned == false, "bdMallocMemory::free, "
												"memory block allocated as aligned "
												"but is beign deallocated with bdMallocMemory::free. "
												"Use bdMallocMemory::alignedFree instead.");
			m_recording = true;
		}

		bdFree(link);
	}
}

void* BD_CALL bdDebugMemory::reallocate(void *p,
										const bdUWord size)
{
	if(p)
	{
		bdMemoryChainElement* link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;
		eraseMemory(link);

		const bdUWord totalSize = size + sizeof(bdMemoryChainElement);
		link =  reinterpret_cast<bdMemoryChainElement*>(bdRealloc(link, totalSize));

		return recordMemory(link, size, false);
	}
	else
	{
		return allocate(size);
	}
}
 
void* BD_CALL bdDebugMemory::alignedAllocate(const bdUWord size, 
											 const bdUWord align)
{
	const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

	// use bdAlignedOffsetMalloc here to ensure the returned pointer is
	// correctly aligned rather than the address of the link.
	bdMemoryChainElement* link =  reinterpret_cast<bdMemoryChainElement*>(bdAlignedOffsetMalloc(totalSize, align, sizeof(bdMemoryChainElement)));
	
	return recordMemory(link, size, true);
}
 
void BD_CALL bdDebugMemory::alignedDeallocate(void *p)
{
	if(p)
	{
		bdMemoryChainElement* link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;
		eraseMemory(link);

		if(m_recording)
		{
			m_recording = false;
			BD_ASSERT(link->m_aligned == true, "bdMallocMemory::alignedFree, "
											   "memory block allocated unaligned "
											   "but is beign deallocated with bdMallocMemory::alignedFree. "
											   "Use bdMallocMemory::free instead.");
			m_recording = true;
		}

		bdAlignedOffsetFree(link);
	}
}
 
void* BD_CALL bdDebugMemory::alignedReallocate(void *p,
											   const bdUWord size,
											   const bdUWord align)
{
	if(p)
	{
		bdMemoryChainElement* link =  reinterpret_cast<bdMemoryChainElement*>(p) - 1;
		const bdUWord origSize = link->m_size + sizeof(bdMemoryChainElement);
		eraseMemory(link);

		const bdUWord totalSize = size + sizeof(bdMemoryChainElement);

		link =  reinterpret_cast<bdMemoryChainElement*>(bdAlignedOffsetRealloc(link, origSize, totalSize, align, sizeof(bdMemoryChainElement)));

		return recordMemory(link, size, true);
	}
	else
	{
		return alignedAllocate(size, align);
	}
}

void BD_CALL bdDebugMemory::cleanup()
{
	if(m_initialized)
	{		
		SymCleanup(GetCurrentProcess());
	}
}

void BD_CALL bdDebugMemory::leakCheck()
{
	if(m_allocatedBytes)
	{
		bdNChar8 buf[100];
		bdSnprintf(buf, 100, "%u Bytes leaked in %u allocation(s)\n", m_allocatedBytes, m_numAllocations);

		OutputDebugString("********************************************\n");
		OutputDebugString("*      BITDEMON MEMORY LEAKS DETECTED      *\n");
		OutputDebugString("********************************************\n");
		OutputDebugString(buf);
		OutputDebugString("See memory.txt for details\n");
		OutputDebugString("********************************************\n");

		const bdBool recording = m_recording;
		m_recording = false;
		dumpMemory();
		m_recording = recording;
	}
}

void BD_CALL bdDebugMemory::releaseAllMemory()
{
	while(m_memoryChain)
	{
		bdMemoryChainElement *const link = m_memoryChain;

		eraseMemory(link);
		
		if(link->m_aligned)
		{
			bdAlignedOffsetFree(link);
		}
		else
		{
			bdFree(link);
		}
	}
}

void BD_CALL bdDebugMemory::dumpMemory()
{
	const bdBool recording = m_recording;
	m_recording = false;

	const bdUInt stringBufferSize = 1024;
	bdNChar8 stringBuffer[stringBufferSize];

	OutputDebugString("Memory Summary\n");
	OutputDebugString("--------------\n\n");
	
	bdSnprintf(stringBuffer, stringBufferSize, "Currently allocated: %u bytes (%u Kilobytes)\n", m_allocatedBytes, m_allocatedBytes/1024);
	OutputDebugString(stringBuffer);

	bdSnprintf(stringBuffer, stringBufferSize, "Number of allocations: %u\n\n", m_numAllocations);
	OutputDebugString(stringBuffer);

	OutputDebugString("Individual allocations\n");
	OutputDebugString("----------------------\n");

	bdMemoryChainElement *link = m_memoryChain;
	while(link)
	{
		const bdNChar8 *const passed = "Passed\n";
		const bdNChar8 *const failed = "Failed\n";
		const bdNChar8 *const magicTest =  link->m_magic==BD_MEMORY_MAGIC?passed:failed;

		OutputDebugString("Memory magic: ");
		OutputDebugString(magicTest);

		bdSnprintf(stringBuffer, stringBufferSize, "Allocation size: %u bytes (%u kilobytes)\n", link->m_size, link->m_size / 1024);
		OutputDebugString(stringBuffer);

		OutputDebugString("Stack trace:\n\n");
		dumpStackTrace(link->m_stack);
		OutputDebugString("\n");		
		link = link->m_next;
	}

	m_recording = recording;
}

DWORD xFilter(CONTEXT *const context, 
			  LPEXCEPTION_POINTERS xEP)
{
	bdMemcpy( context, xEP->ContextRecord, sizeof(CONTEXT));
	return EXCEPTION_EXECUTE_HANDLER;
}

void BD_CALL bdDebugMemory::initStack(bdMemoryChainElement *const link)
{
	new (&link->m_stack) bdFastArray<DWORD64>;
}

void* BD_CALL bdDebugMemory::recordMemory(bdMemoryChainElement *link,
										  const bdUWord size,
										  const bdBool aligned)
{
	if(!link)
	{
		return BD_NULL;
	}

	if(!m_recording)
	{
		return reinterpret_cast<void*>(link+1);
	}

	if(!m_initialized)
	{
		initialize();
		if(!m_initialized)
		{
			return reinterpret_cast<void*>(link+1);
		}
	}

	// Would like to call placement new here but can't because of the
	// __try {} __except() {} construct, that we use to get the current
	// threads context. If we find a better way to be a valid thread CONTEXT
	// this can be removed.
	// new (&link->m_stack) bdFastArray<DWORD64>;
	initStack(link);

	HANDLE hProcess = GetCurrentProcess();
	HANDLE hThread = GetCurrentThread();

	CONTEXT context;
	bdMemset(&context, BD_NULL, sizeof(CONTEXT)); 
	
	__try
	{
		*((bdNChar8*)0)='x';
	}
	__except(xFilter(&context, GetExceptionInformation()))
	{
	}	

	STACKFRAME64 stackframe;
	bdMemset(&stackframe, BD_NULL, sizeof(STACKFRAME64));

	stackframe.AddrPC.Offset = context.Eip;
    stackframe.AddrPC.Mode = AddrModeFlat;
	stackframe.AddrFrame.Offset = context.Ebp;
	stackframe.AddrFrame.Mode = AddrModeFlat;

	while( StackWalk64(IMAGE_FILE_MACHINE_I386,
					   hProcess,
					   hThread,
					   &stackframe,					   
					   &context,
					   NULL,
					   SymFunctionTableAccess64,
					   SymGetModuleBase64,
					   NULL))
	{
		if(stackframe.AddrPC.Offset!=0)
		{
			const bdBool recording = m_recording;
			m_recording = false;
			link->m_stack.pushBack(stackframe.AddrPC.Offset);
			m_recording = recording;
		}
	}

	link->m_magic = BD_MEMORY_MAGIC;
	link->m_size = size;
	link->m_aligned = aligned;
	link->m_next = m_memoryChain;
	link->m_prev = BD_NULL;
	if(m_memoryChain)
	{
		m_memoryChain->m_prev = link;
	}
	m_memoryChain = link;

	m_allocatedBytes += size;
	m_numAllocations++;

	return reinterpret_cast<void*>(link+1);
}

void BD_CALL bdDebugMemory::eraseMemory(bdMemoryChainElement *link)
{
	if(!m_recording)
	{
		return;
	}

	if(!m_initialized)
	{
		initialize();
		if(!m_initialized)
		{
			return;
		}
	}

	if(link->m_magic != BD_MEMORY_MAGIC)
	{
		const bdBool recording = m_recording;
		m_recording = false;
		BD_ASSERT(link->m_magic == BD_MEMORY_MAGIC, "bdDebugMemory::eraseMemory, BD_MEMORY_MAGIC is incorrect.");
		m_recording = recording;
	}

	if(link->m_prev)
	{
		link->m_prev->m_next = link->m_next;
	}
	else
	{
		m_memoryChain = link->m_next;
	}

	if(link->m_next)
	{
		link->m_next->m_prev = link->m_prev;
	}

	const bdBool recording = m_recording;
	m_recording = false;
	link->m_stack.clear();
	m_recording = recording;

	m_allocatedBytes -= link->m_size;
	m_numAllocations--;
}

void BD_CALL bdDebugMemory::initialize()
{
	if(!m_initialized)
	{
		m_initialized = SymInitialize(GetCurrentProcess(), BD_NULL, true)?true:false;

		DWORD symbolOptions = SymGetOptions();	
		SymSetOptions( symbolOptions | SYMOPT_LOAD_LINES );
	}
}

void BD_CALL bdDebugMemory::dumpStackTrace(bdFastArray<DWORD64>& stack)
{
	const bdUInt stackDepth = stack.getSize();

	HANDLE hProcess = GetCurrentProcess();

	bdUByte8 symbolBuffer[sizeof(SYMBOL_INFO) + MAX_PATH];
	PSYMBOL_INFO symbol = reinterpret_cast<PSYMBOL_INFO>(symbolBuffer);

	for(bdUInt i=0; i<stackDepth; i++)
	{			
		// get the line number details
		DWORD displacement32 = 0;
		IMAGEHLP_LINE64 line; 
		bdMemset(&line, BD_NULL, sizeof(IMAGEHLP_LINE64));
		line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
		BOOL lineOk = SymGetLineFromAddr64(hProcess, stack[i], &displacement32, &line);
		// get the symbol details
		DWORD64 displacement64 = 0;
		bdMemset(symbol, BD_NULL, sizeof(symbolBuffer));
		symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
		symbol->MaxNameLen = MAX_PATH;
		BOOL symbolOk = SymFromAddr(hProcess, stack[i], &displacement64, symbol);
	
		if(lineOk && symbolOk)
		{
			const bdUInt stringBufferSize = 1024;
			bdNChar8 stringBuffer[stringBufferSize];

			bdSnprintf(stringBuffer, stringBufferSize, "%s:%u %s\n", line.FileName, static_cast<bdUInt>(line.LineNumber), symbol->Name);
			OutputDebugString(stringBuffer);
		}
	}
}

bdDebugMemory::bdDebugMemory()
{
}

#endif // defined (BD_DEBUG_MEMORY)



