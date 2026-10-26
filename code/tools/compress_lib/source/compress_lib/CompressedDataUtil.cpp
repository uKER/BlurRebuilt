#include "stdafx.h"
#include <core/utility/endian.h>

using namespace CompressLib;

U32 C_CompressedDataUtil::CalculateSize(
	U32 width, 
	U32 height,
	E_CompressionFormat compressionFormat)
{
	if (compressionFormat == E_CompressionFormat_Unknown)
		return 0;

	width = GetDestDimension(width, compressionFormat);
	height = GetDestDimension(height, compressionFormat);

	//We square our block size as a compressed block is a group of pixels measuring
	//GetBlockSizePixels(...) in width and GetBlockSizePixels(...) in height.
	U32 blockSizePixelsSquared = GetBlockSizePixels(compressionFormat);
	blockSizePixelsSquared *= blockSizePixelsSquared;

	U32 size = (GetBlockSizeBytes(compressionFormat) * width * height) / blockSizePixelsSquared;
	return size;
}

U32 C_CompressedDataUtil::GetBlockSizePixels(
	E_CompressionFormat compressionFormat)
{
	switch (compressionFormat)
	{
		case E_CompressionFormat_DXT1:
		//case E_CompressionFormat_DXT2:
		case E_CompressionFormat_DXT3:
		case E_CompressionFormat_DXT4:
		case E_CompressionFormat_DXT5:
		case E_CompressionFormat_DXN:
			return 4;
		default:
			assert(!"Unknown texture format!");
			break;
	}

	return 0;
}

U32 C_CompressedDataUtil::GetBlockSizeBytes(
	E_CompressionFormat compressionFormat)
{
	switch (compressionFormat)
	{
		case E_CompressionFormat_DXT3:
		case E_CompressionFormat_DXT4:
		case E_CompressionFormat_DXT5:
		case E_CompressionFormat_DXN:
			// (8 byte block * 2)
			return GetWordSize(compressionFormat)*2;

		case E_CompressionFormat_DXT1:
			// 8 byte block
			return GetWordSize(compressionFormat);

		default:
		{
			assert(0);
		//	LOGERROR( g_Log, "Error: Unknown texture format.\n");
		}
	}
	return 0;
}

U32 C_CompressedDataUtil::GetWordSize(
	E_CompressionFormat compressionFormat)
{
	switch (compressionFormat)
	{
		case E_CompressionFormat_DXT3:
		case E_CompressionFormat_DXT4:
		case E_CompressionFormat_DXT5:
		case E_CompressionFormat_DXN:
		case E_CompressionFormat_DXT1:
			return 8;
		{
			assert(0);
		//	LOGERROR( g_Log, "Error: Unknown texture format.\n");
		}
	}
	return 0;
}

U32 C_CompressedDataUtil::GetDestDimension(
   U32 sourceDimension, 
   E_CompressionFormat compressionFormat)
{
	U32 blockSize = GetBlockSizePixels(compressionFormat);
	if (sourceDimension < blockSize) 
		return blockSize;

	return sourceDimension;
}

void C_CompressedDataUtil::EndianSwap(
	U8* pData,
	U32 size,
	E_CompressionFormat compressionFormat)
{
	Ut::DoEndianSwap::EndianSwap(
		pData, 
		"WWWW", 
		size / 8);
}
