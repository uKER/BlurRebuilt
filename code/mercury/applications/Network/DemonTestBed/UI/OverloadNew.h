#ifndef GAME_MEMORY_OVERLOAD_NEW_H
#define GAME_MEMORY_OVERLOAD_NEW_H

#ifdef _MSC_VER
#pragma once
#endif

#include <System\Memory.h>
//#include "MemoryDefines.h"

#ifndef CGG_GOLD
#define USE_NEW_MEMORY_TRACKER
#endif

/*
extern const int g_BlockNumberOfSizes;
extern Memory::BlockAllocator g_Allocator;
extern int gSmallAllocCount;
extern int gOtherAllocCount;
*/

void* operator new ( size_t size );
void operator delete ( void* ptr );

void* operator new[]( size_t size );
void operator delete[]( void* ptr );

#endif // GAME_MEMORY_OVERLOAD_NEW_H
