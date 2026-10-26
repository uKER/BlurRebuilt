#include "StdAfx.h"
#include "Main.h"
#include "SectionPhysics.h"

#include "System/Memory/MemoryTrackerDefines.h"
#include "System/Memory/MemoryTracker.h"
#include "system/Memory/AllocatorFunctions.inl"

//#define ENABLE_MEMORY_TRACKER

#ifdef ENABLE_MEMORY_TRACKER

#include "System/Memory/AllocatorFunctions.h"

void* operator new[]( size_t size )
{
	return Memory::TrackedAlignedAlloc(size, 16);
}

void operator delete[]( void* ptr )
{
	Memory::TrackedAlignedFree(ptr);
}

void * operator new (size_t size)
{
	return Memory::TrackedAlignedAlloc(size,16);
}

void operator delete(void *mem)
{
	Memory::TrackedAlignedFree(mem);
}

#endif


namespace Samples
{

C_Sample::C_Sample()
{
}

C_Sample::~C_Sample()
{
}

Display::C_Device* C_Sample::CreateDevice()
{
	return new Display::C_Device();
}

bool C_Sample::MyCreate()
{
	AddSection(new C_SectionLauncher<C_SectionPhysics>("Physics"));
	
	return true;
}

}; // namespace Samples

//int PreStaticInit();

#ifdef PLATFORM_PS3
int main()
#else
void __cdecl main()
#endif
{
#if PLATFORM_PS3
	PreStaticInit();
#endif

	Samples::C_Sample sample;
	
	if (sample.Create())
	{
		sample.Run();
	}
	else
	{
		LOGFATAL(Samples::g_Log, "Failed to create the sample\n");
	}
}
