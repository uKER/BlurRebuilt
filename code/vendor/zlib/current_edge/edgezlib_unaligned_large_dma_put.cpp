/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edgezlib_spu.h"
#include <cell/dma.h>

#include "edge_assert.h"

//////////////////////////////////////////////////////////////////////////

void _edgeZlibUnalignedLargeDmaPut( const void* ls, uint32_t ea, uint32_t size, uint32_t tag )
{
	//////////////////////////////////////////////////////////////////////////
	//
	//	Send out the output buffer.
	//	Note that the LS address and the EA address must both have the same
	//	offset within a qword.
	//	The transfer has to be split into various smaller sections.
	//		-	In the first qword, we need to do 1,2,4,8 byte transfers as
	//			necessary to get us aligned.
	//		-	Then transfer in multiples of qwords up to 64K max in a single
	//			transfer.
	//		-	Then in final qword, we need to do 8,4,2,1 byte transfers as
	//			necessary to send out the remaining data.
	//
	//////////////////////////////////////////////////////////////////////////

	uint32_t lsAddress	= (uint32_t) ls;

	//ls and ea must have same offset
	EDGE_ASSERT( (lsAddress & 0xF) == (ea & 0xF) );
	EDGE_ASSERT( tag < 32 );

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;

	while ( size > 0 )
	{
		//Calculate the size of the current DMA to transfer
		uint32_t currSize =	(lsAddress & 1) ? 1 :
							((lsAddress & 2) && (size >= 2)) ? 2 :
							((lsAddress & 4) && (size >= 4)) ? 4 :
							((lsAddress & 8) && (size >= 8)) ? 8 :
							(size >= MFC_MAX_DMA_SIZE) ? MFC_MAX_DMA_SIZE :
							(size >= MFC_MIN_DMA_SIZE) ? (size & ~(MFC_MIN_DMA_SIZE - 1)) :
							(size >= 8) ? 8 :
							(size >= 4) ? 4 :
							(size >= 2) ? 2 :
							1;

		spu_idisable();
		si_wrch( MFC_LSA, si_from_uint(lsAddress) );
		si_wrch( MFC_EAL, si_from_uint(ea) );
		si_wrch( MFC_Size, si_from_uint(currSize) );
		si_wrch( MFC_TagID, si_from_uint(tag) );
		si_wrch( MFC_Cmd, si_from_uint(MFC_CMD_WORD(0,0,MFC_PUT_CMD))) ;
		if ( interruptsEnabled )
			spu_ienable();

		lsAddress	+= currSize;
		ea			+= currSize;
		size		-= currSize;
	}
}

//////////////////////////////////////////////////////////////////////////
