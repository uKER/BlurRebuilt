#ifndef DISPLAY_MEMORY_MEMORY_DEBUG_H
#define DISPLAY_MEMORY_MEMORY_DEBUG_H

#ifdef _MSC_VER
#pragma once
#endif

namespace Display
{

//! C_MemoryDebug

class C_MemoryDebug
{
public:

	C_MemoryDebug(const U32 size, U8* pAddress);
	~C_MemoryDebug();

	static C_MemoryDebug* Insert(const U32 size, U8* pAddress);
	static void Remove(C_MemoryDebug* pItem);
	static void Check();
	
private:

	U32 								m_Size;
	U8* 								m_pOrigAddress;
	U8* 								m_pCopyAddress;
	static Ut::UtSet<C_MemoryDebug*>	m_List;
};

}; // namespace Display

#endif // DISPLAY_MEMORY_MEMORY_DEBUG_H
