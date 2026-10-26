#ifndef DISPLAY_SPU_MEMORY_BUFFER_H
#define DISPLAY_SPU_MEMORY_BUFFER_H

#if _MSC_VER > 1000
#pragma once
#endif



namespace Display
{
namespace Spu
{



struct C_MemoryLocation
{
	U32	m_Address;
	U32	m_Offset;
	U32	m_Location;
};



struct ALIGN128 C_MemoryBuffer
{
	U32	m_BeginAddress;
	U32	m_CurrentAddress;
	U32	m_EndAddress;
	U32	m_Offset;
	U32	m_Location;
	// labels for synchronisation
	U32	m_LabelNumber;
	U32	m_LabelAddress;
	// memory tracking
	U32	m_ResourceType;
	U32	m_ResourceFlags;
	U32	m_ResourceSystemId;
	// pad to 128 bytes to faciliate DMA transfers
	U32	m_Padding[22];
};



}; // namespace Spu
}; // namespace Display



#endif // DISPLAY_SPU_MEMORY_BUFFER_H
