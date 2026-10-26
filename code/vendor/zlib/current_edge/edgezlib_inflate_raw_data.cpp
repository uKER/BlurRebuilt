/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#include "edge_printf.h"
#include "edge_assert.h"
#include "edgezlib_spu.h"
#include "edgezlib_deflate.h"
#include "edgezlib_inffast.h"

//////////////////////////////////////////////////////////////////////////

enum
{
	kLsMemEnd = 0x40000,
};

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	Decompresses a buffer of compressed data.
 *
 * @note	The input and output buffers are both in LS. Providing the
 *			input data and dealing with the output data is expected to be
 *			handled by the function which calls edgeZlibInflateRawData.
 *			This function doesn’t do any DMAs internally.
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
 * @param	pUncompr			Pointer to where the uncompressed data will be written to in Local Store.
 * @param	expectedUncompSize	The expected output size of the uncompressed data.
 *								The output buffer pointed to by pUncompr must be at least this large.
 * @param	pComprData			Pointer to where the compressed data will be read from in Local Store.
 *								This should be a pointer to the raw data without any header.
 * @param	comprDataSize		Size of the compressed data buffer to be read.
 */

extern "C" int edgeZlibInflateRawData( unsigned char* pUncompr, uint32_t expectedUncompSize, const unsigned char* pComprData, uint32_t comprDataSize )
{
	EDGE_ASSERT( ((uint32_t)pUncompr) < kLsMemEnd );		//Double check the user hasn't passed in effective addresses by mistake
	EDGE_ASSERT( ((uint32_t)pComprData) < kLsMemEnd );

	z_stream stream;
	stream.avail_in		= comprDataSize;
	stream.avail_out	= expectedUncompSize;

	stream.next_in		= pComprData;
	stream.next_out		= pUncompr;

	int err = inflateInit2( &stream, -MAX_WBITS );
	if ( err != Z_OK )
	{
		EDGE_ASSERT( false );
		return err;
	}

	err = inflate( &stream, Z_FINISH );
	if ( err != Z_STREAM_END )
	{
		//This may be caused by bad data.
		//Either assert here, or feedback the error so the PPU can re-load and re-attempt the decompression
		EDGE_ZLIB_DATA_ASSERT_MSG( false, ( "EDGE ZLIB ERROR: inflate error (%d)\n", err ) );
		inflateEnd(&stream);
		if ( (err == Z_NEED_DICT) || (err == Z_BUF_ERROR && stream.avail_in == 0) )
			return Z_DATA_ERROR;
		return err;
	}

	err = inflateEnd( &stream );
	if ( err != Z_OK )
	{
		//This may be caused by bad data.
		//Either assert here, or feedback the error so the PPU can re-load and re-attempt the decompression
		EDGE_ZLIB_DATA_ASSERT_MSG( false, ( "EDGE ZLIB ERROR: inflateEnd error (%d)\n", err ) );
		return err;
	}

	// check number of bytes uncompressed
	if ( stream.total_out != expectedUncompSize )
	{
		//This may be caused by bad data.
		//Either assert here, or feedback the error so the PPU can re-load and re-attempt the decompression
		EDGE_ZLIB_DATA_ASSERT_MSG( false,
						( "EDGE ZLIB ERROR: Stream decompressed to size different from expected (%d != %d)\n",
							stream.total_out, expectedUncompSize ) );
		return Z_DATA_ERROR;
	}

	return Z_OK;
}

//////////////////////////////////////////////////////////////////////////
