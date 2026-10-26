#include <cell/dma.h>

#include <Core/Core.h>
#include <Core/Maths.h>

#include "Display/Spu/Table/IndexTable.h"



namespace Display
{
namespace Spu
{



C_IndexTable::C_IndexTable()
{
	m_pBuffer = 0;
}

bool C_IndexTable::Create(const U32 count)
{
	m_Count = count;
	m_pBuffer = (U16 *) memalign(0x80, (m_Count * 2 + 0xF) & ~0xF);

	return true;
}

void C_IndexTable::Destroy()
{
	if (m_pBuffer)
	{
		free((void *)((U32)(m_pBuffer) & ~0x7F));
		m_pBuffer = 0;
	}
}

void C_IndexTable::Load(const U32 address, const U32 memoryId)
{
	S32 size = (m_Count * 2 + 0xF) & ~0xF; // round to next 16 bytes
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

void C_IndexTable::LoadSync(const U32 address, const U32 memoryId)
{
	Load(address, memoryId);
	cellDmaWaitTagStatusAll(1 << memoryId);
}



}; // namespace Spu
}; // namespace Display
