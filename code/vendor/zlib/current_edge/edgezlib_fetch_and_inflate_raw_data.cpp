/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edgezlib_spu.h"
#include "edgezlib_deflate.h"
#include <algorithm>
#include <cell/dma.h>
#include "edgezlib_inffast.h"

#include "edge_assert.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kLsMemEnd = 0x40000,
};

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	Decompresses a buffer of compressed data in main memory to
 *			another main memory buffer.
 *
 * @note	This function fetches data into LS and sends data out of LS
 *			itself as necessary.
 *
 * @note	This function requires 2 temporary buffers in LS to work in.
 *			These can be specified as parameters ("pLsInputTempBuffer" and
 *			"pLsOutputTempBuffer").
 *
 * @note	This function will assert if the expectedUncompSize disagrees
 *			with the actual uncompressed size.
 *
 * @note	This function expects a pointer to the raw compressed data
 *			without any header.
 *
 * @note	If any data error is encountered, this function will not assert
 *			(unless a conditional define is changed) and instead returns an
 *			error value to its caller.
 *
 * @return	Zero on success. Non-zero on failure.
 *
 * @param	eaUncompOutput		Effective Address of where the uncompressed data will be written to.
 * @param	expectedUncompSize	The expected output size of the uncompressed data.
 *								The output buffer at eaUncompOutput must be at least this large.
 * @param	eaCompressed		Effective Address of where the compressed data will be read from.
 * @param	compressedSize		Size of the compressed data buffer to be read.
 * @param	dmaTag,				The DMA tag to be used for DMAs performed in this function.
 * @param	pLsInputTempBuffer	Pointer to the temporary buffer in Local Store to be used for
 *								reading input data into.
 *								The size of this buffer is specified by inputTempBuffSize.
 * @param	inputTempBuffSize	Size of the temporary buffer pointed to by pLsInputTempBuffer.
 * @param	pLsOutputTempBuffer	Pointer to the temporary buffer in Local Store to be used for
 *								computing the output data into and sending out from.
 *								The size of this buffer is specified by outputTempBuffSize.
 * @param	outputTempBuffSize	Size of the temporary buffer pointed to by pLsInputTempBuffer.
 */

int edgeZlibFetchAndInflateRawData(	uint32_t eaUncompOutput, uint32_t expectedUncompSize,
									uint32_t eaCompressed, uint32_t compressedSize,
									uint32_t dmaTag,
									unsigned char* pLsInputTempBuffer,
									uint32_t inputTempBuffSize,
									unsigned char* pLsOutputTempBuffer,
									uint32_t outputTempBuffSize	)
{
	EDGE_ASSERT( ((uint32_t)eaCompressed) >= kLsMemEnd );			//Double check the user hasn't passed in LS addresses by mistake
	EDGE_ASSERT( ((uint32_t)eaUncompOutput) >= kLsMemEnd );
	EDGE_ASSERT( ((uint32_t)pLsInputTempBuffer) < kLsMemEnd );		//Double check the user hasn't passed in effective addresses by mistake
	EDGE_ASSERT( ((uint32_t)pLsOutputTempBuffer) < kLsMemEnd );

	const uint32_t kEdgeZlibOverAllocateInputBufferSize = 128;		//Need to reserve more than maximum buffer size due to potential mis-alignment of incoming data
	const uint32_t kEdgeZlibOverAllocateOutputBufferSize = 128;		//Need to reserve more than maximum buffer size due to potential mis-alignment of outgoing data

	EDGE_ASSERT( (((uint32_t)pLsInputTempBuffer) & 0x7F) == 0 );
	EDGE_ASSERT( (inputTempBuffSize & 0x7F) == 0 );
	EDGE_ASSERT( (((uint32_t)pLsOutputTempBuffer) & 0x7F) == 0 );
	EDGE_ASSERT( (outputTempBuffSize & 0x7F) == 0 );
	uint32_t singleInputMaxSize				= inputTempBuffSize - kEdgeZlibOverAllocateInputBufferSize;
	EDGE_ASSERT( singleInputMaxSize > 0 );
	uint32_t singleOutputMaxSize			= outputTempBuffSize - kEdgeZlibOverAllocateOutputBufferSize;
	EDGE_ASSERT( singleOutputMaxSize > 0 );

	uint32_t currEaInput					= eaCompressed;
	uint32_t currEaOutput					= eaUncompOutput;
	uint32_t remainingInputData				= compressedSize;
	uint32_t remainingOutputSpace			= expectedUncompSize;

	z_stream stream;

	stream.avail_in							= 0;
	stream.next_in							= Z_NULL;

	bool interruptsEnabled = spu_readch(SPU_RdMachStat) & 1;

	int ret = inflateInit2( &stream, -MAX_WBITS );
	if ( ret != Z_OK )
		return ret;

	uint32_t totalOutputSize = 0;

	/* decompress until deflate stream ends or end of file */
	do
	{
		if ( remainingInputData == 0 )
			break;

		//Firstly, fetch up to "singleInputMaxSize" worth of input data
		uint32_t thisInputSize				= std::min( remainingInputData, singleInputMaxSize );

		{
			uint32_t eaInputRoundDown		= currEaInput & ~0x7F;
			uint32_t inputAlignOffset		= currEaInput & 0x7F;
			uint32_t inputSizeRoundUp		= thisInputSize + inputAlignOffset;
			inputSizeRoundUp = (inputSizeRoundUp + 0x7F) & ~0x7F;
			EDGE_ASSERT( inputAlignOffset <= kEdgeZlibOverAllocateInputBufferSize );
			EDGE_ASSERT( (eaInputRoundDown + inputAlignOffset) == currEaInput );
			EDGE_ASSERT( inputSizeRoundUp <= inputTempBuffSize );

			spu_idisable();
			cellDmaLargeGet( pLsInputTempBuffer, eaInputRoundDown, inputSizeRoundUp, dmaTag, 0, 0 );
			cellDmaWaitTagStatusAll( 1 << dmaTag );
			if ( interruptsEnabled )
				spu_ienable();

			stream.next_in					= &pLsInputTempBuffer[inputAlignOffset];
			stream.avail_in					= thisInputSize;
		}

		//We have consumed "thisInputSize" worth of the input data
		currEaInput							+= thisInputSize;
		remainingInputData					-= thisInputSize;

		//Set up our output buffer
		uint32_t thisOutputAlignOffset1		= currEaOutput & 0x7F;
		EDGE_ASSERT( thisOutputAlignOffset1 < kEdgeZlibOverAllocateOutputBufferSize );
		unsigned char* pThisOutputBase		= &pLsOutputTempBuffer[thisOutputAlignOffset1];
		stream.next_out						= pThisOutputBase;
		stream.avail_out					= singleOutputMaxSize;

		// Keep running inflate() on input and sending out data to main memory until we've consumed all input data
		while ( true )
		{
			ret = inflate( &stream, Z_NO_FLUSH );
			// Z_STREAM_ERROR is only for code error, not error in data itself
			EDGE_ASSERT( ret != Z_STREAM_ERROR );

			switch ( ret )
			{
			case Z_NEED_DICT:
				ret = Z_DATA_ERROR;
				// fall through

			case Z_DATA_ERROR:
			case Z_MEM_ERROR:
				//This may be caused by bad data.
				//Either assert here, or feedback the error so the PPU can re-load and re-attempt the decompression
				EDGE_ZLIB_DATA_ASSERT_MSG( false, ( "EDGE ZLIB ERROR: inflate error (%d)\n", ret ) );
				inflateEnd( &stream );
				return ret;
			}

			uint32_t thisOutputSize			= singleOutputMaxSize - stream.avail_out;
			EDGE_ASSERT( thisOutputSize <= singleOutputMaxSize );
			EDGE_ASSERT( thisOutputSize <= remainingOutputSpace );

			if ( thisOutputSize )
			{
				//Flush the LS buffer
				spu_idisable();
				_edgeZlibUnalignedLargeDmaPut( pThisOutputBase, currEaOutput, thisOutputSize, dmaTag );
				cellDmaWaitTagStatusAll( 1 << dmaTag );
				if ( interruptsEnabled )
					spu_ienable();

				remainingOutputSpace		-= thisOutputSize;
				currEaOutput				+= thisOutputSize;

				totalOutputSize				+= thisOutputSize;
			}

			if ( stream.avail_out )
			{
				//If there was still output buffer space available
				//then we must have run out of input data, so break out
				//of the inner while loop and fetch more input data
				break;
			}

			if ( thisOutputSize )
			{
				//Start filling the output buffer form the start again
				uint32_t thisOutputAlignOffset2	= currEaOutput & 0x7F;
				EDGE_ASSERT( thisOutputAlignOffset2 < kEdgeZlibOverAllocateOutputBufferSize );
				pThisOutputBase				= &pLsOutputTempBuffer[thisOutputAlignOffset2];

				stream.next_out				= pThisOutputBase;
				stream.avail_out			= singleOutputMaxSize;
			}
		}

		// Keep fetching more input data until inflate tells us we've reached the end of the stream
	} while ( ret != Z_STREAM_END );

	EDGE_ASSERT( totalOutputSize == expectedUncompSize );
	EDGE_ASSERT( remainingOutputSpace == 0 );

	// clean up and return
	inflateEnd( &stream );

	return ((ret == Z_STREAM_END) ? Z_OK : Z_DATA_ERROR);
}

//////////////////////////////////////////////////////////////////////////
