
#ifndef __ZLIB_COMPRESSION_H__
#define __ZLIB_COMPRESSION_H__

#include "ZLib.h"

//bool data_Decompress_Init(U8 *pOutputBuffer, U32 dwOutputBufferSize);
//U32 data_Decompress_Term(void);
//
//bool data_Decompress(U8 *pCompressedData, U32 dwCompressedSize);
//
//bool data_Decompress(U8 *pCompressedData, U32 dwCompressedSize, U8 *pOutputBuffer, U32 dwOutputBufferSize);
//
//bool data_Compress(U8 *pOriginalData, U32 dwOriginalSize, U8 **ppCompressedBuffer, U32* pdwCompressedSize);
//


class CCompression
{
protected:
	z_stream	m_CompressionStream;
	bool		m_createdDecompressionContext;

public:
	CCompression(bool createDecompressionContext=true);
	~CCompression();

	bool		Decompress_Init(U8 *pOutputBuffer, U32 dwOutputBufferSize);
	//U32		Decompress_Term(void);
	
	bool		Decompress(U8 *pCompressedData, U32 dwCompressedSize);
	bool		Decompress(U8 *pCompressedData, U32 dwCompressedSize, U8 *pOutputBuffer, U32 dwOutputBufferSize);

	void		Decompress_SetOutputBuffer(U8 *pCompressedData, U32 dwCompressedSize);

	bool		Compress(U8 *pOriginalData, U32 dwOriginalSize, U8 **ppCompressedBuffer, U32* pdwCompressedSize);

	U32 TotalIn() const;
	U32 TotalOut() const;
	U32 AvailIn() const;
	U32 AvailOut() const;
};


#endif
