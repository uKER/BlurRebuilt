#include "MiscSharedPch.h"
#include <System/Scheduler.h>
#include <cell/atomic.h>
#include <cell/dma.h>

#include "../../../../Vendor/Zlib/Current_Edge/EdgeZlib_Spu.h"

namespace Compression
{

void InflateTask(Scheduler::Task* pTask)
{
	const U32 nDmaTagId = 0;

	U8 pInflateInputBuffer[kEdgeZlibInflateInputBufferMaxSize + 128] ALIGN128;
	U8 pInflateOutputBuffer[kEdgeZlibInflateOutputBufferMaxSize + 128] ALIGN128;

	C_DeflateParameters* pParameters = Spu::FetchAlloc(pTask->Output, sizeof(C_DeflateParameters));
	
	const U32 compressedAddress		= pParameters->m_CompressionStream.next_in;
	const U32 uncompressedAddress	= pParameters->m_CompressionStream.next_out;
	const U32 compressedSize		= pParameters->m_CompressionStream.avail_in;
	const U32 uncompressedSize		= pParameters->m_CompressionStream.avail_out;

	if ((compressedSize > kEdgeZlibInflateInputBufferMaxSize) || (uncompressedSize > kEdgeZlibInflateOutputBufferMaxSize))
	{
		if (!edgeZlibFetchAndInflateRawData(uncompressedAddress, uncompressedSize, compressedAddress, compressedSize, nDmaTagId, pInflateInputBuffer, sizeof(pInflateInputBuffer), pInflateOutputBuffer, sizeof(pInflateOutputBuffer)))
		{
			return;
		}
	}
	else
	{
		U32	compressedAddressRoundDown	= compressedAddress & ~0x7F;
		U32	inputAlignOffset			= compressedAddress & 0x7F;
		U32	compressedSizeRoundUp		= compressedSize + inputAlignOffset;
			compressedSizeRoundUp		= (compressedSizeRoundUp + 0x7F) & ~0x7F;

		cellDmaLargeGet(pInflateInputBuffer, compressedAddressRoundDown, compressedSizeRoundUp, nDmaTagId, 0, 0);
		cellDmaWaitTagStatusAll(1 << nDmaTagId);

		U32 outputAlignOffset = uncompressedAddress & 0x7F;

		U8* pLsOutputBuffer = &pInflateOutputBuffer[outputAlignOffset];
		if (!edgeZlibInflateRawData(pLsOutputBuffer, uncompressedSize, &pInflateInputBuffer[inputAlignOffset], compressedSize))
		{
			return;
		}

		_edgeZlibUnalignedLargeDmaPut(pLsOutputBuffer, uncompressedAddress, uncompressedSize, nDmaTagId);
		cellDmaWaitTagStatusAll(1 << nDmaTagId);
	}

	return;
}

} // namespace Compression
