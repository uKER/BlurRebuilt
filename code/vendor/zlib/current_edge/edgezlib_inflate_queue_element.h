/* SCE CONFIDENTIAL
 * PLAYSTATION(R)Edge 0.4.0
 * Copyright (C) 2007 Sony Computer Entertainment Inc.
 * All Rights Reserved.
 */

#ifndef __EDGE_ZLIB_INFLATE_QUEUE_ELEMENT_H__
#define __EDGE_ZLIB_INFLATE_QUEUE_ELEMENT_H__

//////////////////////////////////////////////////////////////////////////

/** 
 * @brief	One entry in the queue which tells the Inflate Task to
 *			decompress a single segment of data.
 *
 * @note	The address of the compressed and uncompressed data can have
 *			any alignment.
 *
 * @note	The sum of m_outputUncompSkipBeginSize, m_outputUncompPartialBuffSize
 *			and m_outputUncompSkipEndSize gives the expected size that the
 *			compressed data will be decompress to.  If this value is incorrect
 *			then the SPU code will assert.
 */

typedef struct EdgeZlibInflateQueueElement
{   // align 16, 32 bytes
	uint32_t	m_eaCompressed;					///< effective address of compressed data.
	uint32_t	m_eaUncompressed;				///< effective address where to put uncompressed data.
	uint32_t	m_compressedSize;				///< size of compressed data.
	uint32_t	m_outputUncompPartialBuffSize;	///< size of uncompressed data to be DMAed out.
	uint32_t	m_eaWorkToDoCounter;			///< effective address of counter to atomically decrement when decompression done.
												///< If the SPU has an error with the compressed data it will set the high bit
												///< of the counter to alert the fact that an error occurred.  This EA may be NULL.
												///< LSB of this EA is a flag to say if segment was stored compressed (1) or uncompressed (0).
	uint32_t	m_eaEventFlag;					///< effective address of event flag.  May be NULL.
	uint16_t	m_eventFlagBits;				///< value to set to the event flag.
	uint16_t	m_outputUncompSkipBeginSize;	///< Don't output the first N bytes of the uncompressed output
	uint16_t	m_outputUncompSkipEndSize;		///< Don't output the last  N bytes of the uncompressed output.
	uint16_t	m_pad16;
} EdgeZlibInflateQueueElement __attribute__((aligned(16)));

//////////////////////////////////////////////////////////////////////////

enum
{
	kEdgeZlibInflateInputBufferMaxSize	= 0x10000,
	kEdgeZlibInflateOutputBufferMaxSize	= 0x10000,
};

//////////////////////////////////////////////////////////////////////////

#endif //__EDGE_ZLIB_INFLATE_QUEUE_ELEMENT_H__
