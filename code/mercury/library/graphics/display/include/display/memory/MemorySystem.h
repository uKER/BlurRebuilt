#ifndef DISPLAY_MEMORY_MEMORY_SYSTEM_H
#define DISPLAY_MEMORY_MEMORY_SYSTEM_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Memory/Memory.h"
#include <System/Memory/MemoryTracker.h>

namespace Sys
{
	class TextSerialiser;
}

namespace Display
{

//! System memory manager

class C_MemoryManagerSystem
{
public:

	C_MemoryManagerSystem();
	~C_MemoryManagerSystem();

	static void SetOutOfMemoryCallBackFunction(OutOfMemoryCallBackFunction* pFunc);

	static void OutOfMemory();

	static void Update();
	
	static void DumpUsage();
	static void DumpUsage(const Util::C_String &filePath);

private:

	static void Output(char* output);

private:

	static OutOfMemoryCallBackFunction*		m_pOutOfMemoryCallBackFunction;
	static Sys::TextSerialiser*				m_pTextSerialiser;
};

}; // namespace Display

#endif // DISPLAY_MEMORY_MEMORY_SYSTEM_H
