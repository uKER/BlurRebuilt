#ifndef DISPLAY_MEMORY_MEMORY_H
#define DISPLAY_MEMORY_MEMORY_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

typedef void (OutOfMemoryCallBackFunction)();

//! Memory

enum E_Memory
{
	E_Memory_System,
	E_Memory_Graphics,
};

//! Memory manager

class C_MemoryManager
{
public:

	C_MemoryManager();
	~C_MemoryManager();

	static void OutOfMemory(const E_Memory memory);
};

void PrintTexture2Ds();

void OutOfMemory();

}; // namespace Display

#endif // DISPLAY_MEMORY_MEMORY_H
