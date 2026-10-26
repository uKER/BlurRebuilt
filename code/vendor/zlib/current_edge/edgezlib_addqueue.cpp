/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include <cell/spurs/lfqueue.h>

#include "edge_printf.h"
#include "edge_assert.h"
#include "edgezlib_inflate_queue_element.h"
#include "edgezlib_spu.h"


//======================================================================================================

/** 
 * @brief	Add a new piece of work to the Inflate Queue. The work added to this queue will be taken by
 *			one of the Inflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will block until there is space.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on. Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done.  At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function expects a pointer to the raw compressed data without any header.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 *										This should be a pointer to the raw data without any header.
 * @param	compressedSize				The size of the compressed input data.
 * @param	eaOutputUncompressedData	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	expectedUncompressedSize	The expected size of the uncompressed data.
 *										The SPU will assert if this value is incorrect.
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the raw data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 */

void edgeZlibAddInflateQueueElement(
		EdgeZlibInflateQHandle			handle,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompressedData,
		uint32_t						expectedUncompressedSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing,
		uint32_t						dmaTag )
{
	edgeZlibAddInflateQueueElementPartialCopyOut(	handle,
													eaInputCompressedData,
													compressedSize,
													eaOutputUncompressedData,
													0,
													expectedUncompressedSize,
													0,
													eaWorkToDoCounter,
													eaEventFlag,
													eventFlagBits,
													processing,
													dmaTag );
}

//======================================================================================================

/** 
 * @brief	Try to add a new piece of work to the Inflate Queue. The work added to this queue will be
 *			taken by one of the Inflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
 *
 * @note	If the item is added, then this function will return true. If the queue is full
 *			(ie. reached maxNumQueueEntries), and another item is pushed onto the queue, then this
 *			function will return false.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on. Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done.  At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function expects a pointer to the raw compressed data without any header.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 *										This should be a pointer to the raw data without any header.
 * @param	compressedSize				The size of the compressed input data.
 * @param	eaOutputUncompressedData	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	expectedUncompressedSize	The expected size of the uncompressed data.
 *										The SPU will assert if this value is incorrect.
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the raw data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 * @return								'true' if the item was added.  'false' if it failed to add.
 */

bool edgeZlibTryAddInflateQueueElement(
		EdgeZlibInflateQHandle			handle,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompressedData,
		uint32_t						expectedUncompressedSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing,
		uint32_t						dmaTag )
{
	return edgeZlibTryAddInflateQueueElementPartialCopyOut(	handle,
															eaInputCompressedData,
															compressedSize,
															eaOutputUncompressedData,
															0,
															expectedUncompressedSize,
															0,
															eaWorkToDoCounter,
															eaEventFlag,
															eventFlagBits,
															processing,
															dmaTag );
}

//======================================================================================================

/** 
 * @brief	Add a new piece of work to the Inflate Queue. The work added to this queue will be taken by
 *			one of the Inflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will block until there is space.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on.  Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done.  At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function expects a pointer to the raw compressed data without any header.
 *
 * @note	This function is very similar to edgeZlibAddInflateQueueElement. The additional parameters
 *			skipOutputBeginSize and skipOutputEndSize exist so that decompression of a segment can
 *			be done, but only a portion of the output may need to be written.
 *
 * @note	The sum of outputUncompSkipBeginSize, outputUncompPartialBuffSize and outputUncompSkipEndSize
 *			gives the expected uncompressed side of the compressed data.  The SPU will assert if this
 *			value is incorrect.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 *										This should be a pointer to the raw data without any header.
 * @param	compressedSize				The size of the compressed input data.
 * @param	eaOutputUncompPartialBuff	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	outputUncompSkipBeginSize	Don't output the first N bytes of the uncompressed output
 * @param	outputUncompPartialBuffSize	The size of the uncompressed data which will be DMAed out.
 * @param	outputUncompSkipEndSize		Don't output the last  N bytes of the uncompressed output
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the raw data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @param	dmaTag						The DMA tag to use when adding the element to the queue
 */

void edgeZlibAddInflateQueueElementPartialCopyOut(
		EdgeZlibInflateQHandle			handle,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompPartialBuff,
		uint16_t						outputUncompSkipBeginSize,
		uint32_t						outputUncompPartialBuffSize,
		uint16_t						outputUncompSkipEndSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing,
		uint32_t						dmaTag )
{
	EdgeZlibInflateQueueElement queueEntry;

	uint32_t expectedUncompressedSize = outputUncompSkipBeginSize + outputUncompPartialBuffSize + outputUncompSkipEndSize;

	EDGE_ASSERT_MSG( compressedSize <= kEdgeZlibInflateInputBufferMaxSize,
        ( "EDGE ZLIB ERROR: Size of compressed data is %d.  Maximum is %d.\n", compressedSize, kEdgeZlibInflateInputBufferMaxSize ) );
	
	EDGE_ASSERT_MSG( expectedUncompressedSize <= kEdgeZlibInflateOutputBufferMaxSize,
		( "EDGE ZLIB ERROR: Size of uncompressed data is %d.  Maximum is %d.\n", expectedUncompressedSize, kEdgeZlibInflateOutputBufferMaxSize ) );

	EDGE_ASSERT_MSG( outputUncompSkipBeginSize + outputUncompSkipEndSize <= expectedUncompressedSize,
		( "EDGE ZLIB ERROR: outputUncompSkipBeginSize(%d) + outputUncompSkipEndSize(%d) > expectedUncompressedSize(%d)\n",
				outputUncompSkipBeginSize, outputUncompSkipEndSize, expectedUncompressedSize ) );

	//EDGE_ASSERT( (eaWorkToDoCounter == NULL) || ((*eaWorkToDoCounter) > 0) );	//Requires a DMA to implement this assert

	if ( compressedSize > expectedUncompressedSize )
	{
		//If applying compression makes something bigger, then it'd be better to have just used
		//the uncompressed master data in the first place.
		//Just print a warning to draw people's attention to this and carry on.
		EDGE_PRINTF( "Warning: Compressed data at address 0x%08X is *bigger* than master data (%d > %d).  Pointless?\n",
				eaInputCompressedData,
				compressedSize,
				expectedUncompressedSize );
	}

	EDGE_ASSERT( (eaWorkToDoCounter & 0x3) == 0 );

	queueEntry.m_eaCompressed					= eaInputCompressedData;
	queueEntry.m_eaUncompressed					= eaOutputUncompPartialBuff;
	queueEntry.m_compressedSize					= compressedSize;
	queueEntry.m_outputUncompSkipBeginSize		= outputUncompSkipBeginSize;
	queueEntry.m_outputUncompPartialBuffSize	= outputUncompPartialBuffSize;
	queueEntry.m_outputUncompSkipEndSize		= outputUncompSkipEndSize;
	queueEntry.m_eaEventFlag					= eaEventFlag;
	queueEntry.m_eaWorkToDoCounter				= (eaWorkToDoCounter) | processing;	//Bottom bit is a flag to do decompression (1), or memcpy (0)
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_pad16							= 0;

	//If interrupts are enabled, disable them temporarily while adding to the queue
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	CellSpursLFQueuePushContainer pushContainer;
	cellSpursLFQueuePushContainerInitialize( &pushContainer, &queueEntry, dmaTag );

	//Note: if the queue is full this routine will wait (yield?)
	int ret = cellSpursLFQueuePushBegin( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );
	ret = cellSpursLFQueuePushEnd( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );

	if ( interruptsEnabled )
	{
		spu_ienable();
	}
}

//======================================================================================================

/** 
 * @brief	Try to add a new piece of work to the Inflate Queue. The work added to this queue will be
 *			taken by one of the Inflate Tasks at the next chance they get.
 *
 * @note	This is the SPU equivalent of the PPU function of the same name.
 *
 * @note	If the queue is full (ie. reached maxNumQueueEntries), and another item is pushed onto the
 *			queue, then this function will block until there is space.
 *
 * @note	The work-to-do-counter can be used to track completion of the processing on a collection
 *			of segments by initializing it to however many segments are being waited on. Then when
 *			each item completes it will decrement by one, and so when the value reaches zero all
 *			segments will have been done. At this point the specified event flag will be set.
 *			Note that if the SPU has an error with compressed data it will set the high bit of the
 *			counter.
 *
 * @note	If eaWorkToDoCounter is NULL, but eaEventFlag is valid, then the event flag will be set
 *			after this segment is done.
 *
 * @note	If the data was stored uncompressed and merely needs moving to its destination, then the
 *			Inflate Task can be used for this by passing the appropriate value for 'processing'.
 *
 * @note	This function expects a pointer to the raw compressed data without any header.
 *
 * @note	This function is very similar to edgeZlibAddInflateQueueElement. The additional parameters
 *			skipOutputBeginSize and skipOutputEndSize exist so that decompression of a segment can
 *			be done, but only a portion of the output may need to be written.
 *
 * @note	The sum of outputUncompSkipBeginSize, outputUncompPartialBuffSize and outputUncompSkipEndSize
 *			gives the expected uncompressed side of the compressed data. The SPU will assert if this
 *			value is incorrect.
 *
 * @param	handle						The handle of the Inflate Queue that the entry will be pushed
 *										onto.
 * @param	eaInputCompressedData		The Effective Address of the input data which is to be
 *										decompressed.
 *										This should be a pointer to the raw data without any header.
 * @param	compressedSize				The size of the compressed input data.
 * @param	eaOutputUncompPartialBuff	The Effective Address of the output buffer for the
 *										uncompressed data.
 * @param	outputUncompSkipBeginSize	Don't output the first N bytes of the uncompressed output
 * @param	outputUncompPartialBuffSize	The size of the uncompressed data which will be DMAed out.
 * @param	outputUncompSkipEndSize		Don't output the last  N bytes of the uncompressed output
 * @param	eaWorkToDoCounter			A counter in main memory which will be atomically decremented
 *										after this item has been decompressed.
 *										If the SPU has an error with the compressed data it will set the
 *										high bit of the counter to alert the fact that an error occurred.
 *										Can be NULL if you're not interested.
 * @param	eaEventFlag					The event flag to set.
 *										Can be NULL if you're not interested.
 * @param	eventFlagBits				The value to set to the event flag.
 * @param	processing					Choose what kind of processing the task has to do on the data.
 *										Can perform decompression, or merely move the raw data from
 *										pInputCompressedData to pOutputUncompressedData.
 * @param	dmaTagId					The DMA tag to use when adding the element to the queue
 * @return								'true' if the item was added. 'false' if it failed to add.
 */

bool edgeZlibTryAddInflateQueueElementPartialCopyOut(
		EdgeZlibInflateQHandle			handle,
		uint32_t						eaInputCompressedData,
		uint32_t						compressedSize,
		uint32_t						eaOutputUncompPartialBuff,
		uint16_t						outputUncompSkipBeginSize,
		uint32_t						outputUncompPartialBuffSize,
		uint16_t						outputUncompSkipEndSize,
		uint32_t						eaWorkToDoCounter,
		uint32_t						eaEventFlag,
		uint16_t						eventFlagBits,
		EdgeZlibInflateTaskProcessing	processing,
		uint32_t						dmaTagId )
{
	EdgeZlibInflateQueueElement queueEntry;

	uint32_t expectedUncompressedSize = outputUncompSkipBeginSize + outputUncompPartialBuffSize + outputUncompSkipEndSize;

	EDGE_ASSERT_MSG( compressedSize <= kEdgeZlibInflateInputBufferMaxSize,
        ( "EDGE ZLIB ERROR: Size of compressed data is %d.  Maximum is %d.\n", compressedSize, kEdgeZlibInflateInputBufferMaxSize ) );

	EDGE_ASSERT_MSG( expectedUncompressedSize <= kEdgeZlibInflateOutputBufferMaxSize,
		( "EDGE ZLIB ERROR: Size of uncompressed data is %d.  Maximum is %d.\n", expectedUncompressedSize, kEdgeZlibInflateOutputBufferMaxSize ) );

	EDGE_ASSERT_MSG( outputUncompSkipBeginSize + outputUncompSkipEndSize <= expectedUncompressedSize,
		( "EDGE ZLIB ERROR: outputUncompSkipBeginSize(%d) + outputUncompSkipEndSize(%d) > expectedUncompressedSize(%d)\n",
				outputUncompSkipBeginSize, outputUncompSkipEndSize, expectedUncompressedSize ) );

	//EDGE_ASSERT( (eaWorkToDoCounter == NULL) || ((*eaWorkToDoCounter) > 0) );	//Requires a DMA to implement this assert

	if ( compressedSize > expectedUncompressedSize )
	{
		//If applying compression makes something bigger, then it'd be better to have just used
		//the uncompressed master data in the first place.
		//Just print a warning to draw people's attention to this and carry on.
		EDGE_PRINTF( "Warning: Compressed data at address 0x%08X is *bigger* than master data (%d > %d).  Pointless?\n",
				eaInputCompressedData,
				compressedSize,
				expectedUncompressedSize );
	}

	EDGE_ASSERT( (eaWorkToDoCounter & 0x3) == 0 );

	queueEntry.m_eaCompressed					= eaInputCompressedData;
	queueEntry.m_eaUncompressed					= eaOutputUncompPartialBuff;
	queueEntry.m_compressedSize					= compressedSize;
	queueEntry.m_outputUncompSkipBeginSize		= outputUncompSkipBeginSize;
	queueEntry.m_outputUncompPartialBuffSize	= outputUncompPartialBuffSize;
	queueEntry.m_outputUncompSkipEndSize		= outputUncompSkipEndSize;
	queueEntry.m_eaEventFlag					= eaEventFlag;
	queueEntry.m_eaWorkToDoCounter				= (eaWorkToDoCounter) | processing;	//Bottom bit is a flag to do decompression (1), or memcpy (0)
	queueEntry.m_eventFlagBits					= eventFlagBits;
	queueEntry.m_pad16							= 0;

	//If interrupts are enabled, disable them temporarily while adding to the queue
	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;
	spu_idisable();

	CellSpursLFQueuePushContainer pushContainer;
	cellSpursLFQueuePushContainerInitialize( &pushContainer, &queueEntry, dmaTagId );

	//Test to see if we can push onto the queue
	int ret = cellSpursLFQueueTryPushBegin( handle, &pushContainer );
	if ( CELL_SPURS_TASK_ERROR_AGAIN == ret )
	{
		//No space
		if ( interruptsEnabled )
		{
			spu_ienable();
		}
		return false;
	}
	EDGE_ASSERT( CELL_OK == ret );

	ret = cellSpursLFQueuePushEnd( handle, &pushContainer );
	EDGE_ASSERT( CELL_OK == ret );

	if ( interruptsEnabled )
	{
		spu_ienable();
	}

	return true;
}

//======================================================================================================
