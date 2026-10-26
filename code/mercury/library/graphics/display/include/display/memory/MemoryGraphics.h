#ifndef DISPLAY_MEMORY_MEMORY_GRAPHICS_H
#define DISPLAY_MEMORY_MEMORY_GRAPHICS_H

#ifdef _MSC_VER
#pragma once
#endif

#include "Display/Memory/Memory.h"

namespace Sys
{
	class TextSerialiser;
}

namespace Display
{

//! Graphics memory manager

class C_MemoryManagerGraphics
{
public:

	C_MemoryManagerGraphics();
	~C_MemoryManagerGraphics();

	static void SetOutOfMemoryCallBackFunction(OutOfMemoryCallBackFunction* pFunc);

	static void OutOfMemory();

	static void Update();

	static void DumpUsage(const bool outputSystems, const bool outputTexture2Ds);
	static void DumpUsage(const Util::C_String &filePath, const bool outputSystems, const bool outputTexture2Ds);
	static void DumpUsageSystem(const bool bCSV);
	static void DumpUsageTexture2D(const bool bCSV);

private:

	static void Output(char* output, ...);

private:

	static OutOfMemoryCallBackFunction*		m_pOutOfMemoryCallBackFunction;
	static Sys::TextSerialiser*				m_pTextSerialiser;
};

}; // namespace Display

#endif // DISPLAY_MEMORY_MEMORY_GRAPHICS_H
