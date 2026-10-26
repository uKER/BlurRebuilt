
#pragma once

#include "ZLib.h"

//bool data_Decompress_Init(char *pOutputBuffer, DWORD dwOutputBufferSize);
//DWORD data_Decompress_Term(void);
//
//bool data_Decompress(char *pCompressedData, DWORD dwCompressedSize);
//
//bool data_Decompress(char *pCompressedData, DWORD dwCompressedSize, char *pOutputBuffer, DWORD dwOutputBufferSize);
//
//bool data_Compress(char *pOriginalData, DWORD dwOriginalSize, char **ppCompressedBuffer, DWORD* pdwCompressedSize);
//

class CCompression
{
protected:
	z_stream	m_CompressionStream;
public:
	bool		Decompress_Init(char *pOutputBuffer, int dwOutputBufferSize);
	int			Decompress_Term(void);
	bool		Decompress(char *pCompressedData, int dwCompressedSize);
	bool		Decompress(char *pCompressedData, int dwCompressedSize, char *pOutputBuffer, int dwOutputBufferSize);

	bool		Compress(char *pOriginalData, int dwOriginalSize, char **ppCompressedBuffer, int* pdwCompressedSize);
};
