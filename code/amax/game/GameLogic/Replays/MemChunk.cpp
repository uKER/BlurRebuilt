#include "stdafx.h"
#include <GameLogic\Replays\MemChunk.h>
#include <System\Memory.h>

Debug::Logger &g_MemChunkLog( Debug::GetLogger("amax.replay") );

#define DEFAULT_MAX_SIZE 5000

namespace Game
{

/*
===============
NAME: C_MemChunk::C_MemChunk

NOTE: Constructor, takes a specific size for the memory chunk
===============
*/
C_MemChunk::C_MemChunk(U32 p_Size = 1) : m_Size(p_Size), m_MaxSize(UINT_MAX)
{
	MEMORY_TRACKER_PUSH("game.memoryChunk");
	m_Start = new U8[m_Size];
	m_Current = m_Start;
	MEMORY_TRACKER_POP("game.memoryChunk");
}

/*
===============
NAME: C_MemChunk::ClearMemory

NOTE: Clears the entire chunk of memory
===============
*/
void C_MemChunk::ClearMemory()
{
	memset(m_Start, 0, m_Size);
	m_Current = m_Start;
}

/*
===============
NAME: C_MemChunk::ClearForward

NOTE: Clears the memory between m_Current and the end of the chunk. Does not resize the chunk.
===============
*/
void C_MemChunk::ClearForward()
{
	AssertChunk();
	U32 clearSize = (m_Start + m_Size) - m_Current;
	memset(m_Current, 0, clearSize);
}

/*
===============
NAME: C_MemChunk::ClearForward

NOTE: Clears from the given memory address to the end of the chunk. Does not resize the chunk.
===============
*/
void C_MemChunk::ClearForward(U8* p_ClearFrom)
{
	BIZ_ASSERT(p_ClearFrom <= (m_Start + m_Size) && (p_ClearFrom >= m_Start));
	U32 clearSize = (m_Start + m_Size) - p_ClearFrom;
	memset(p_ClearFrom, 0, clearSize);
}

/*
===============
NAME: C_MemChunk::ClearBack

NOTE: Clears the memory between the start of the chunk and m_Current. Does not resize the chunk.
===============
*/
void C_MemChunk::ClearBack()
{
	AssertChunk();
	U32 clearSize = m_Current - m_Start;
	memset(m_Start, 0, clearSize);
}

/*
===============
NAME: C_MemChunk::ClearBack

NOTE: Clears from the start of the chunk to the given memory address. Does not resize the chunk.
===============
*/
void C_MemChunk::ClearBack(U8 *p_ClearTo)
{
	BIZ_ASSERT(p_ClearTo <= (m_Start + m_Size) && (p_ClearTo >= m_Start));
	U32 clearSize = p_ClearTo - m_Start;
	memset(m_Start, 0, clearSize);
}

/*
===============
NAME: C_MemChunk::AssertChunk

NOTE: Self-checks the current position etc, making sure we don't overrun the allocated chunk of memory
===============
*/
void C_MemChunk::AssertChunk()
{
	BIZ_ASSERT(m_Current >= m_Start);
	BIZ_ASSERT(m_Current <= (m_Start + m_Size));
}


U32 C_MemChunk::GetUsedSize() const
{
	//	Make sure we don't return a size of 1 in the event this chunk hasn't been written to
	//	- might be used to create a new C_MemChunk and we wouldn't want to give it a size of zero!
	if(m_Current == m_Start)
		return 1;
	else
		return (m_Current - m_Start);
}


/*
===============
NAME: C_MemChunk::Resize

NOTE: Change the size of the memory pool (very costly as it just allocates an amount elsewhere and copies all the data!)
===============
*/
bool C_MemChunk::Resize(U32 p_NewSize)
{
	MEMORY_TRACKER_PUSH("game.memoryChunk");

	if(p_NewSize >= m_MaxSize)
	{
		//LOGINFO(g_MemChunkLog, "Memory Chunk attempted to resize(%d) beyond its maximum allocation(%d) - not resizing", p_NewSize, m_MaxSize);
		MEMORY_TRACKER_POP("game.memoryChunk");
		return false;
	}
	BIZ_ASSERT(p_NewSize < m_MaxSize);

	AssertChunk();

	U32 currentDepth = (m_Current - m_Start);

	BIZ_ASSERT(p_NewSize >= currentDepth);

	U8* temp = new U8[p_NewSize];
	memcpy(temp, m_Start, currentDepth);
	
	delete [] m_Start;
	
	m_Start		= temp;
	m_Size		= p_NewSize;
	m_Current	= m_Start + currentDepth;

	BIZ_ASSERT(m_Current <= (m_Start + m_Size));
	
	AssertChunk();

	MEMORY_TRACKER_POP("game.memoryChunk");

	return true;
}

/*
===============
NAME: C_MemChunk::Clone

NOTE: Copies the memory handled by a C_MemChunk object into this one
===============
*/
void C_MemChunk::Clone(C_MemChunk* toCopy)
{
	BIZ_ASSERT(toCopy);
	if(m_Size != toCopy->GetSize())
	{
		//	Clear the memory of this chunk to avoid any resizing complications
		ClearMemory();
		Resize(toCopy->GetSize());
	}
	memcpy(m_Start, toCopy->m_Start, m_Size);
	AssertChunk();
}

/*
===============
NAME: C_MemChunk::EfficientClone

NOTE: Copies the USED memory handled by a C_MemChunk object into this one
===============
*/
void C_MemChunk::EfficientClone(C_MemChunk* toCopy)
{
	BIZ_ASSERT(toCopy);
	if(m_Size != toCopy->GetUsedSize())
	{
		//	Clear the memory of this chunk to avoid any resizing complications
		ClearMemory();
		Resize(toCopy->GetUsedSize());
	}
	memcpy(m_Start, toCopy->m_Start, m_Size);
	AssertChunk();
}


/*
===============
NAME: C_MemChunk::GoToStart

NOTE: Returns m_Current to the start of the chunk
===============
*/
void C_MemChunk::GoToStart()
{
	m_Current = m_Start;
}

/*
===============
NAME: C_MemChunk::Push

NOTE: Adds new data to the chunk. Returns true if the chunk size had to be increased to accomodate the new data
===============
*/
bool C_MemChunk::Push(U8* p_PushedStart, U32 p_PushedSize)
{
	BIZ_ASSERT(p_PushedSize > 0);
	if((m_Current + p_PushedSize) > (m_Start + m_Size))
	{
			if(Resize(m_Size + p_PushedSize))
			{
				memcpy(m_Current, p_PushedStart, p_PushedSize);
				MoveForward(p_PushedSize);
			}
			AssertChunk();
			return true;
	}
	//assert((m_Current + p_PushedSize) < (m_Start + m_Size));
	else
	{
		memcpy(m_Current, p_PushedStart, p_PushedSize);
		MoveForward(p_PushedSize);
		AssertChunk();
		return false;
	}
}

/*
===============
NAME: C_MemChunk::MoveForward

NOTE: Increments current position in memory chunk in bytes
===============
*/
void C_MemChunk::MoveForward(U32 p_Distance)
{
	if((m_Current + p_Distance) > (m_Start + m_Size))
		m_Current = m_Start + m_Size;
	else
		m_Current += p_Distance;
	AssertChunk();
}

bool C_MemChunk::PushU8( U8 in )
{
	return Push((U8*)&in, sizeof(U8));
}

bool C_MemChunk::PushS8( S8 in )
{
	return Push((U8*)&in, sizeof(S8));
}

bool C_MemChunk::PushU16( U16 in )
{
	return Push((U8*)&in, sizeof(U16));
}

bool C_MemChunk::PushS16( S16 in )
{
	return Push((U8*)&in, sizeof(S16));
}

bool C_MemChunk::PushU32( U32 in )
{
	return Push((U8*)&in, sizeof(U32));
}

bool C_MemChunk::PushS32( S32 in )
{
	return Push((U8*)&in, sizeof(S32));
}

bool C_MemChunk::PushU64( U64 in )
{
	return Push((U8*)&in, sizeof(U64));
}

bool C_MemChunk::PushS64( S64 in )
{
	return Push((U8*)&in, sizeof(S64));
}

bool C_MemChunk::PushF32( F32 in )
{
	return Push((U8*)&in, sizeof(F32));
}

} // namspace Btl
