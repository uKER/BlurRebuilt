#ifndef MEM_CHUNK_H
#define MEM_CHUNK_H

#ifdef _MSC_VER
#pragma once
#endif

#include <string.h>

namespace Game
{

/* 
===============================================================
class C_MemChunk

NOTES: Class that allocates and manages....a chunk of memory!
===============================================================
*/
class C_MemChunk
{
public:
	C_MemChunk();
	C_MemChunk(U32 p_Size);
	~C_MemChunk() {Destroy();};


	//	Self-checks the current position etc, making sure we don't overrun the allocated chunk of memory
	void										AssertChunk();

	//	Returns the allocated memory chunk size in bytes
	U32											GetSize() const {return m_Size;};

	//	Returns size of the currently used memory in bytes
	U32											GetUsedSize() const;

	U32											GetMaxSize() const			{ return m_MaxSize; }

	U8*											GetStart() const {return m_Start;};

	U8*											GetCurrent() const {return m_Current;};

	U8*											GetEnd() const {return m_Start + m_Size;};

	//	Change the size of the memory pool (very costly as it just allocates an amount elsewhere and copies all the data!)
	bool										Resize(U32 p_NewSize);

	//	Copies the memory handled by a C_MemChunk object into this one
	void										Clone(C_MemChunk* p_toCopy);

	//	Copies the USED memory handled by a C_MemChunk object into this one -
	//	This chunk will likely now be smaller than the one we're copying from since it probably hasn't used all its allocated memory
	void										EfficientClone(C_MemChunk* p_toCopy);

	//	Increments current position in memory chunk in bytes
	void										MoveForward(U32 p_Distance);

	//	Adds new data to the chunk
	bool										Push(U8* p_PushedStart, U32 p_PushedSize);
	bool										PushU8(U8 in);
	bool										PushS8(S8 in);
	bool										PushU16(U16 in);
	bool										PushS16(S16 in);
	bool										PushU32(U32 in);
	bool										PushS32(S32 in);
	bool										PushU64(U64 in);
	bool										PushS64(S64 in);
	bool										PushF32(F32 in);

	//	Returns m_Current to the start of the chunk
	void										GoToStart();

	//	Clears this chunk of memory (sets to zero)
	void										ClearMemory();

	//	Clears the memory between m_Current and the end of the chunk
	void										ClearForward();

	//	Clears the memory between an address and the end of the chunk
	void										ClearForward(U8* p_ClearFrom);

	//	Clears the memory between the start of the chunk and m_Current
	void										ClearBack();

	//	Clears the memory between the start of the chunk and an address
	void										ClearBack(U8* p_ClearTo);

	//	Set the maximum size of the memory chunk in bytes
	void										SetMaxSize(U32 p_MaxSize)	{ m_MaxSize = p_MaxSize; }

private:

	U8*											m_Start;	//	Pointer to start of memory chunk
	U8*											m_Current;	//	Pointer to current location in chunk
	U32											m_Size;		//	Size of chunk in bytes.

	U32											m_MaxSize;	//	The maximum size this memory chunk is allowed to grow to

	//	Destroys the object
	void										Destroy() {SAFE_DELETE_ARY(m_Start)};
};

} // namspace Btl




#endif	//	MEM_CHUNK_H