#include "stdafx.h"
#include "Display/Memory/GCM/GCMMemoryManager.h"
#include <System/Memory/MemoryTracker.h>

#ifdef CFG_ALLOW_OPTIMIZE_OFF
//#pragma optimize( "", off ) // CFG_ALLOW_OPTIMIZE_OFF
#endif

namespace Display
{
namespace GCM
{

C_GCMMemoryBuffer::C_GCMMemoryBuffer() :
	m_Created(false),
	m_Size(0),
	m_Align(0),
	m_Flags(0),
	m_Offset(0),
	m_pAddress(NULL),
	m_Type(E_GCMMemoryBufferType_Local)
{
}

C_GCMMemoryBuffer::~C_GCMMemoryBuffer()
{
	DestroyAddress();
}

void C_GCMMemoryBuffer::SetType(const E_GCMMemoryBufferType type)
{
	m_Type = type;
}

void C_GCMMemoryBuffer::SetOffset(const U32 offset)
{
	m_Offset = offset;
}

void C_GCMMemoryBuffer::SetAddress(U8* pAddress)
{
	m_pAddress = pAddress;
}

bool C_GCMMemoryBuffer::CreateAddress()
{
	assert(!m_Created);

	assert(m_Size > 0);

	if (m_Type == E_GCMMemoryBufferType_Main)
	{
		m_pAddress = (U8*)BizHeapAlignedMalloc(m_Size, 0x100000); // Needs to be aligned to 1MB.
		if(!m_pAddress)
			return false;

		cellGcmMapMainMemory(m_pAddress, m_Size, &m_Offset);
	}
	else
	{
		if (!m_pAddress)
		{
			CellGcmConfig config;
			cellGcmGetConfiguration(&config);

			assert(m_Offset+m_Size <= config.localSize);
			m_pAddress = ((U8*)config.localAddress)+m_Offset;

			Memory::Tracker::Instance().TrackHeapAllocation(m_pAddress, m_Size, Memory::kMemType_Physical);
		}
	}

	m_Created = true;

	return true;
}

void C_GCMMemoryBuffer::DestroyAddress()
{
	if (m_Type == E_GCMMemoryBufferType_Main)
	{
		if(m_pAddress)
		{
			BizHeapAlignedFree(m_pAddress);
			m_pAddress = NULL;
		}
	}
	else
	{
		Memory::Tracker::Instance().TrackHeapDeallocation(m_pAddress, m_Size, Memory::kMemType_Physical);

		m_pAddress = NULL;
	}

	m_Created = false;
}

}; // namespace GCM
}; // namespace Display
