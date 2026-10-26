#include "stdafx.h"
#include "Display/Buffer/GCM/GCMUserFifo.h"
#include "Display/Device/Device.h"

#include "Display/Spu/Buffer/BufferManager.h"



namespace Display
{
namespace Spu
{



bool CreateMemoryBuffer(const Display::E_ResourceType type, const U32 flags, const U32 size, const U32 labelNumber, C_MemoryBuffer * pMemoryBuffer)
{
	pMemoryBuffer->m_ResourceType = type;
	pMemoryBuffer->m_ResourceFlags = flags;
	pMemoryBuffer->m_ResourceSystemId = Display::Device()->MemoryGetSystemID(); // for memory tracking

	Display::Device()->AllocResourceMemory((Display::E_ResourceType) pMemoryBuffer->m_ResourceType, pMemoryBuffer->m_ResourceFlags, size, (U8 **) &pMemoryBuffer->m_BeginAddress, pMemoryBuffer->m_ResourceSystemId);

	pMemoryBuffer->m_CurrentAddress = pMemoryBuffer->m_BeginAddress;
	pMemoryBuffer->m_EndAddress = pMemoryBuffer->m_BeginAddress + size;
	pMemoryBuffer->m_Location = (flags & Display::E_Usage_Video) ? CELL_GCM_LOCATION_LOCAL : CELL_GCM_LOCATION_MAIN;
	cellGcmAddressToOffset((void *) pMemoryBuffer->m_BeginAddress, &pMemoryBuffer->m_Offset);

	pMemoryBuffer->m_LabelNumber = labelNumber;
	pMemoryBuffer->m_LabelAddress = (U32) cellGcmGetLabelAddress(labelNumber);
	GCMCpuSetLabel(labelNumber, 1);

	return (pMemoryBuffer->m_BeginAddress != 0);
}

void DestroyMemoryBuffer(C_MemoryBuffer * pMemoryBuffer)
{
	if (pMemoryBuffer->m_BeginAddress)
	{
		U32 size = pMemoryBuffer->m_EndAddress - pMemoryBuffer->m_BeginAddress;
		Display::Device()->FreeResourceMemory((Display::E_ResourceType) pMemoryBuffer->m_ResourceType, pMemoryBuffer->m_ResourceFlags, size, (U8 *) pMemoryBuffer->m_BeginAddress, pMemoryBuffer->m_ResourceSystemId);
	
		pMemoryBuffer->m_BeginAddress = 0;
		pMemoryBuffer->m_CurrentAddress = 0;
		pMemoryBuffer->m_EndAddress = 0;
		pMemoryBuffer->m_Offset = 0;
		pMemoryBuffer->m_Location = CELL_GCM_LOCATION_MAIN;
	}
}

void BeginMemoryBuffer(C_MemoryBuffer * pMemoryBuffer)
{
	GCMCpuWaitOnLabel(pMemoryBuffer->m_LabelNumber, 1);
	GCMCpuSetLabel(pMemoryBuffer->m_LabelNumber, 0);
	pMemoryBuffer->m_CurrentAddress = pMemoryBuffer->m_BeginAddress;
}

void EndMemoryBuffer(C_MemoryBuffer * pMemoryBuffer)
{
	GCMGpuSetLabelOnTextureEnd(pMemoryBuffer->m_LabelNumber, 1);
	pMemoryBuffer->m_CurrentAddress = pMemoryBuffer->m_BeginAddress;
}

U8 * AllocateFromMemoryBuffer(const U32 sizeInBytes, const U32 alignment, C_MemoryBuffer * pMemoryBuffer)
{
	U8 * pCurrent = (U8 *) pMemoryBuffer->m_CurrentAddress;
	while ( ((U32)(pCurrent) & 0x7F) != alignment)
		++pCurrent;

	if ((U32)(pCurrent + sizeInBytes) < pMemoryBuffer->m_EndAddress)
	{
		pMemoryBuffer->m_CurrentAddress = (U32) (pCurrent + sizeInBytes);
		return pCurrent;
	}
	else
	{
		return 0;
	}
}



}; // namespace Spu
}; // namespace Display
