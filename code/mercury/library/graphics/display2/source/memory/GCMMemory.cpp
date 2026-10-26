#include "StdAfx.h"
#include "Display2/Memory/GCMMemory.h"

namespace Display2
{

// C_MemoryBuffer

C_MemoryBuffer::C_MemoryBuffer() :
	m_Size(0),
	m_Align(0),
	m_Flags(0),
	m_pAddress(NULL),
	m_Type(E_MemoryBuffer_Unknown)
{
}

bool C_MemoryBuffer::Create(const U32 size, const U32 align, const U32 flags, const E_MemoryBuffer type, U8* pBufferAddress)
{
	BIZ_ASSERT(!m_pAddress);

	m_Size		= size;
	m_Align		= align;
	m_Flags		= flags;
	m_Type		= type;

	if(m_Type == E_MemoryBuffer_Indirect)
	{
		m_pAddress = pBufferAddress;

		if(m_pAddress == NULL)
		{
			Destroy();
			LOGFATAL(g_Log, "Failed to create indirect memory buffer of size %i\n", m_Size);
			return false;
		}
	}
	else
	if(m_Type == E_MemoryBuffer_Main)
	{
		m_pAddress = (U8*)BizAlignedMalloc(m_Size, 0x100000); // Needs to be aligned to 1MB

		if(m_pAddress == NULL)
		{
			Destroy();
			LOGFATAL(g_Log, "Failed to alloc main memory of size %i\n", m_Size);
			return false;
		}

		U32 result = cellGcmMapMainMemory(m_pAddress, m_Size, &m_Offset);

		if(result != CELL_OK)
		{
			Destroy();
			LOGFATAL(g_Log, "Failed to map main memory of size %i\n", m_Size);
			return false;
		}
	}
	else
	if(m_Type == E_MemoryBuffer_Local)
	{
		m_Offset = 0; // TODO: Set this externally?

		CellGcmConfig gcmConfig;
		cellGcmGetConfiguration(&gcmConfig);

		if(m_Offset+m_Size > gcmConfig.localSize)
		{
			Destroy();
			LOGFATAL(g_Log, "Failed to alloc local memory of size %i offset %i out of total %i\n", m_Size, m_Offset, gcmConfig.localSize);
			return false;
		}

		m_pAddress = ((U8*)gcmConfig.localAddress)+m_Offset;
	}
	else
	{
		LOGERROR(g_Log, "Unsupported memory buffer type %i\n", m_Type);
		return false;
	}

	return true;
}

void C_MemoryBuffer::Destroy()
{
	if(m_pAddress)
	{
		if(m_Type == E_MemoryBuffer_Main)
		{
			BizAlignedFree(m_pAddress);
		}

		m_pAddress = NULL;
	}
}

bool C_MemoryBuffer::Inside(const U32 size, const U8* pAddress) const
{
	return (pAddress >= m_pAddress) && ((pAddress+size) <= (m_pAddress+m_Size)) ? true : false;
}

} // namespace Display2
