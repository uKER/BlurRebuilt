#include "Display/Spu/Display.h"
#include "Display/Spu/Buffer/CommandBuffer.h"
#include "Display/Spu/Buffer/IndexBuffer.h"
#include "Display/Spu/Buffer/MemoryBuffer.h"
#include "Display/Spu/Buffer/VertexBuffer.h"
#include "Display/Spu/Device/DeviceTypes.h"
#include "Display/Spu/Device/Device.h"



namespace Display
{
namespace Spu
{



C_Device::C_Device()
{
}

C_Device::~C_Device()
{
}

bool C_Device::Create(const C_DeviceParams &deviceParams)
{
	m_CommandBufferAddress = deviceParams.m_CommandBufferAddress;
	m_CommandBufferSize = deviceParams.m_CommandBufferSize;
	m_MemoryBufferAddress = deviceParams.m_MemoryBufferAddress;
	m_pScratchBuffer = deviceParams.m_pScratchBuffer;

	m_gcmContext.begin = (U32 *) ((((U32) m_pScratchBuffer + 0x7F) & ~0x7F) + (m_CommandBufferAddress & 0x7F));
	m_gcmContext.current = m_gcmContext.begin;
	m_gcmContext.end = m_gcmContext.begin + m_CommandBufferSize / 4;
	m_gcmContext.callback = 0;

	m_pScratchBuffer = (U8 *) (((U32) m_gcmContext.end + 0x7F) & ~0x7F);

	return true;
}

bool C_Device::CreateVertexBuffer(const U32 size, const U32 alignment, C_VertexBuffer * pVertexBuffer)
{
	C_MemoryLocation location;
	if (!AllocateMemory(size, alignment, &location))
	{
		return false;
	}
	else
	{
		pVertexBuffer->m_Address = location.m_Address;
		pVertexBuffer->m_Offset = location.m_Offset;
		pVertexBuffer->m_Location = location.m_Location;
		pVertexBuffer->m_Size = size;
		return true;
	}
}

bool C_Device::CreateIndexBuffer(const U32 size, const U32 alignment, C_IndexBuffer * pIndexBuffer)
{
	C_MemoryLocation location;
	if (!AllocateMemory(size, alignment, &location))
	{
		return false;
	}
	else
	{
		pIndexBuffer->m_Address = location.m_Address;
		pIndexBuffer->m_Offset = location.m_Offset;
		pIndexBuffer->m_Location = location.m_Location;
		pIndexBuffer->m_Size = size;
		return true;
	}
}

void C_Device::Finish()
{
	uint32_t size = (m_gcmContext.current - m_gcmContext.begin) * 4;
	uint32_t newSize = PatchForLocalStallSync(m_gcmContext.begin, size);

	// Zero out the rest of the command buffer hole
	U32 * cmd = m_gcmContext.current + ((newSize - size) / 4);
	U32 * cmdEnd = m_gcmContext.end;
	const qword zero = (qword)spu_splats(0);
	const qword negativeOne = si_fsm(si_from_uint(0xF));
	const qword rotAmount = si_sfi(si_andi(si_from_ptr(cmd), 0xF),0);
	const qword mask = si_rotqmby(negativeOne, rotAmount);
	const qword tmp = si_lqx(si_from_ptr(cmd), zero);
	const qword out = si_selb(tmp, zero, mask);
	si_stqx(out, si_from_ptr(cmd), zero);
	qword aligned = si_andi(si_ai(si_from_ptr(cmd), 0xF), ~0xF);
	qword loop;
	do
	{
		si_stqx(zero, aligned, zero);
		aligned = si_ai(aligned, 0x10);
		loop = si_clgt(si_from_ptr(cmdEnd), aligned);
	} while(si_to_uint(loop));

	const U32 commandBufferSize = m_CommandBufferSize ? m_CommandBufferSize : 4;
	
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();
	
	spu_writech(MFC_LSA, (U32) m_gcmContext.begin);
	spu_writech(MFC_EAL, m_CommandBufferAddress);
	spu_writech(MFC_Size, commandBufferSize);
	spu_writech(MFC_TagID, 1 << 0);
	spu_writech(MFC_Cmd, MFC_PUTF_CMD);
	
	if (interruptsEnabled)
		spu_ienable();

	cellDmaWaitTagStatusAll(1 << 0);
}

// Read 4 bytes from video memory with a fence, to make sure that the proceeding
// output DMAs finish before the command buffer hole is filled.
void C_Device::WaitForVideoMemory(const U32 address)
{
	cellDmaSmallGetf(m_pScratchBuffer, address, 4, 0, 0, 0); // assume scratch buffer is aligned to 16 bytes
	m_pScratchBuffer += 0x10; // increment the scratch buffer by 16 bytes to keep alignment

	// no need to wait for this to arrive as we don't need the data, and Finish() waits on this dma tag so the data
	// is guaranteed to arrive before the end of the function (so it won't overwrite random memory)
}

bool C_Device::AllocateMemory(const U32 size, const U32 alignment, C_MemoryLocation * pLocation)
{
	U8 buffer[0x80] ALIGN128;
	volatile C_MemoryBuffer * pMemoryBuffer = (volatile C_MemoryBuffer *) buffer;

	U32 result = 0;
	do 
	{
		// get the buffer information
		spu_writech(MFC_LSA, (U32) pMemoryBuffer);
		spu_writech(MFC_EAL, m_MemoryBufferAddress);
		spu_writech(MFC_Cmd, MFC_GETLLAR_CMD);
		result = spu_readch(MFC_RdAtomicStat);
		
		// allocate from this buffer if we can
		const U32 currentAddress = pMemoryBuffer->m_CurrentAddress;
		if (currentAddress + size > pMemoryBuffer->m_EndAddress)
		{
			return false;
		}
		else
		{
			pLocation->m_Address = pMemoryBuffer->m_CurrentAddress;
			pLocation->m_Offset = pMemoryBuffer->m_Offset + (pMemoryBuffer->m_CurrentAddress - pMemoryBuffer->m_BeginAddress);
			pLocation->m_Location = pMemoryBuffer->m_Location;
			pMemoryBuffer->m_CurrentAddress += size;
		}

		// Write the shared buffer info back to main memory, making sure it hasn't been changed since it was read.
		spu_writech(MFC_LSA, (U32) pMemoryBuffer);
		spu_writech(MFC_EAL, m_MemoryBufferAddress);
		spu_writech(MFC_Cmd, MFC_PUTLLC_CMD);
		result = spu_readch(MFC_RdAtomicStat);
	} while (result & 1);
	
	return true;
}



C_Device g_Device;



}; // namespace Spu
}; // namespace Display
