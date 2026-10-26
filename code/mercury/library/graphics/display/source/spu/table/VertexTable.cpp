#include <cell/dma.h>

#include <Core/Core.h>
#include <Core/Maths.h>

#include "Display/Spu/Table/VertexTable.h"



namespace Display
{
namespace Spu
{



bool C_VertexTable::Create(const U32 count, const U32 stride)
{
	m_Count = count;
	m_Stride = stride;
	m_pBuffer = memalign(0x80, (m_Stride * m_Count + 0xF) & ~0xF);

	return true;
}

void C_VertexTable::Destroy()
{
	if (m_pBuffer)
	{
		free((void *)((U32)(m_pBuffer) & ~0x7F));
		m_pBuffer = 0;
	}
}

void C_VertexTable::Load(const U32 address, const U32 memoryId)
{
	S32 size = (m_Count * m_Stride + 0xF) & ~0xF; // round to next 16 bytes
	U32 newAddress = address;
	U8 * pBuffer = (U8 *) m_pBuffer;
	while (size > 16 * 1024)
	{
		cellDmaGet(pBuffer, newAddress, 16 * 1024, memoryId, 0, 0); 
		size -= 16 * 1024;
		newAddress += 16 * 1024;
		pBuffer += 16 * 1024;
	}
	cellDmaGet(pBuffer, newAddress, size, memoryId, 0, 0);
}

void C_VertexTable::LoadSync(const U32 address, const U32 memoryId)
{
	Load(address, memoryId);
	cellDmaWaitTagStatusAll(1 << memoryId);
}



C_VertexBlock::C_VertexBlock()
{
	m_pBuffer = 0;
	m_Size = 0;
}

bool C_VertexBlock::Create(const U32 stride, const U32 count)
{
	m_Stride = stride;
	m_Count = count;
	return Create(stride * count);
}

bool C_VertexBlock::Create(const U32 size)
{
	m_pBuffer = memalign(0x80, size + 0x7F);
	m_Size = size;
	return true;
}


/*
C_VertexStream::C_VertexStream(const C_VertexBuffer &buffer, const U32 streamAmount)
{
	assert((streamAmount % 16) == 0);

	m_MemoryId = 0;
	m_Buffer = buffer;
	m_Offset = 0;
	m_Index = 0;

	m_Blocks[0].Create(buffer.m_Stride, streamAmount);
	m_Blocks[1].Create(buffer.m_Stride, streamAmount);
}

C_VertexStream::~C_VertexStream()
{
	m_Blocks[1].Destroy();
	m_Blocks[0].Destroy();
}

bool C_VertexStream::Prefetch()
{
	if (m_Offset >= m_Buffer.m_Count)
		return false;

	C_VertexBlock * pBlock = &m_Blocks[m_Index];

	const U32 vertexAddress = m_Buffer.m_Address + m_Offset * m_Buffer.m_Stride;
	void * pBlockBase = (void *) ((U32)(pBlock->m_pBuffer) & ~0x7F);

	pBlock->m_pBuffer = (void *) ((U32) pBlockBase + (vertexAddress & 0x7F));
	pBlock->m_Count = Maths::Min(pBlock->m_Size / m_Buffer.m_Stride, m_Buffer.m_Count - m_Offset);
	pBlock->m_Stride = m_Buffer.m_Stride;

	cellDmaGet(pBlock->m_pBuffer, vertexAddress, (pBlock->m_Count * m_Buffer.m_Stride + 0xF) & ~0xF, m_MemoryId + m_Index, 0, 0);

	m_Offset += pBlock->m_Count;
	m_Index = (~m_Index) & 0x01;

	return true;
}

C_VertexBlock * C_VertexStream::FetchLast()
{
	cellDmaWaitTagStatusAll(1 << (m_MemoryId + m_Index));
	return &m_Blocks[m_Index];
}

C_VertexBlock * C_VertexStream::FetchNext()
{
	U32 newIndex = (~m_Index) & 0x01;

	cellDmaWaitTagStatusAll(1 << (m_MemoryId + newIndex));
	return &m_Blocks[newIndex];
}
*/


}; // namespace Spu
}; // namespace Display
