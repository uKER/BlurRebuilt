#include "AudioPch.h"

#ifdef PLATFORM_SPU
#include "SystemHeadersSPU.h"
#else
#include "SystemHeaders.h"
#endif

//using namespace Audio;


#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", off) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF


Audio::ComponentContainer::ComponentContainer()
{
	m_u32NumComponents = 0;
	m_pComponentIndex = 0;
	m_u32NextCellToPopulate = 0;
}


// temp code to track down a crash on amax PS3
#ifdef GET_COMPONENT_FROM_HASH_DEBUG_CODE
volatile U32 u32GetTypeOffset = 0;
volatile U32* pGetTypeComponentIndex = 0;
volatile U32 u32GetTypeIndex = 0;
#endif
U32 Audio::ComponentContainer::GetType(U32 u32Index)
{
	U32 u32Offset = ((U32*)m_pComponentIndex)[u32Index + 1];
// temp code to track down a crash on amax PS3
#ifdef GET_COMPONENT_FROM_HASH_DEBUG_CODE
u32GetTypeOffset = u32Offset;
pGetTypeComponentIndex = m_pComponentIndex;
u32GetTypeIndex = u32Index;
#endif
	return *((U32*)(((U8*)ComponentData()) + u32Offset - sizeof(U32)));
}


// temp code to track down a crash on amax PS3
#ifdef GET_COMPONENT_FROM_HASH_DEBUG_CODE
volatile U32 u32ArrayOperatorOffset = 0;
volatile U32* pArrayOperatorComponentIndex = 0;
volatile U32 u32ArrayOperatorIndex = 0;
#endif
Audio::Component* Audio::ComponentContainer::operator [] (U32 u32Index)
{
	// TODO: optimize

	U32 u32Offset = ((U32*)m_pComponentIndex)[u32Index + 1];
// temp code to track down a crash on amax PS3
#ifdef GET_COMPONENT_FROM_HASH_DEBUG_CODE
u32ArrayOperatorOffset = u32Offset;
pArrayOperatorComponentIndex = m_pComponentIndex;
u32ArrayOperatorIndex = u32Index;
#endif
	return (Component*)(((U8*)ComponentData()) + u32Offset);
}


void Audio::ComponentContainer::Clear()
{
	if (m_pComponentIndex)
		delete [] (U8*)m_pComponentIndex;

	m_u32NumComponents = 0;
	m_pComponentIndex = 0;
	m_u32NextCellToPopulate = 0;
}


void Audio::ComponentContainer::PushBack(Component* pCom)
{
	U32 u32Offset = (U8*)pCom - (U8*)ComponentData();
	((U32*)m_pComponentIndex)[m_u32NextCellToPopulate + 1] = u32Offset;

	m_u32NextCellToPopulate++;
}

#ifdef AUDIO_OPTIMIZE_OFF
#pragma optimize ("", on) // AUDIO_OPTIMIZE_OFF
#endif	//	AUDIO_OPTIMIZE_OFF
