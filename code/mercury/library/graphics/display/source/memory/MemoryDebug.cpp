#include "stdafx.h"
#include <core\containers\sortedbinarytree.h>
#include "Display/Memory/MemoryDebug.h"

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF 
#endif

Debug::Bool g_debugDisplayBreakOnMemoryStomping				("Graphics.Display.Debug.BreakOnMemoryStomping",					true);
Debug::Bool g_debugDisplayBreakOnMemoryStompingTestBefore	("Graphics.Display.Debug.BreakOnMemoryStompingTestBefore",		false);
Debug::Bool g_debugDisplayBreakOnMemoryStompingTestStart	("Graphics.Display.Debug.BreakOnMemoryStompingTestStart",		false);
Debug::Bool g_debugDisplayBreakOnMemoryStompingTestMiddle	("Graphics.Display.Debug.BreakOnMemoryStompingTestMiddle",		false);
Debug::Bool g_debugDisplayBreakOnMemoryStompingTestEnd		("Graphics.Display.Debug.BreakOnMemoryStompingTestEnd",			false);
Debug::Bool g_debugDisplayBreakOnMemoryStompingTestAfter	("Graphics.Display.Debug.BreakOnMemoryStompingTestAfter",		false);

namespace Display
{

Ut::UtSet<C_MemoryDebug*> C_MemoryDebug::m_List;

C_MemoryDebug::C_MemoryDebug(const U32 size, U8* pAddress)
{
	m_Size			= size;
	m_pOrigAddress	= pAddress;
	m_pCopyAddress	= new U8[size];
	memcpy(m_pCopyAddress, m_pOrigAddress, size);
}

C_MemoryDebug::~C_MemoryDebug()
{
	SAFE_DELETE_ARY(m_pCopyAddress);
}

C_MemoryDebug* C_MemoryDebug::Insert(const U32 size, U8* pAddress)
{
	assert(m_List.Size() == 0);

	C_MemoryDebug* pItem = new C_MemoryDebug(size, pAddress);
	m_List.Insert(pItem);
	return pItem;
}

void C_MemoryDebug::Remove(C_MemoryDebug* pItem)
{
	assert(m_List.Size() == 1);

	m_List.Erase(pItem);
}

void C_MemoryDebug::Check()
{
	Ut::UtSet<C_MemoryDebug*>::Iterator it = m_List.Begin();
	while(it != m_List.End())
	{
		C_MemoryDebug* pItem = *it;
		
		if(memcmp(pItem->m_pOrigAddress, pItem->m_pCopyAddress, pItem->m_Size) != 0)
		{
			if(g_debugDisplayBreakOnMemoryStomping)
			{
				LOGERROR(g_Log, "Memory has been changed : size '%i' orig '%i' copy '%i'\n", pItem->m_Size, (U32)(INTPTR)pItem->m_pOrigAddress, (U32)(INTPTR)pItem->m_pCopyAddress);	// [Blur2PC][x64] log only
			}
		}

		if(g_debugDisplayBreakOnMemoryStompingTestBefore)
		{
			U8* pPtr = pItem->m_pOrigAddress;			
			pPtr--;
			(*pPtr)++;
		}

		if(g_debugDisplayBreakOnMemoryStompingTestStart)
		{
			U8* pPtr = pItem->m_pOrigAddress;			
			(*pPtr)++;
		}

		if(g_debugDisplayBreakOnMemoryStompingTestMiddle)
		{
			U8* pPtr = pItem->m_pOrigAddress;			
			pPtr += (pItem->m_Size/2);
			(*pPtr)++;
		}

		if(g_debugDisplayBreakOnMemoryStompingTestEnd)
		{
			U8* pPtr = pItem->m_pOrigAddress;			
			pPtr += pItem->m_Size-1;
			(*pPtr)++;
		}

		if(g_debugDisplayBreakOnMemoryStompingTestAfter)
		{
			U8* pPtr = pItem->m_pOrigAddress;			
			pPtr += pItem->m_Size;
			(*pPtr)++;
		}

		it++;
	}
}

}; // namespace Display
