#pragma once

namespace CompressLib
{
	class C_CompressedDataUtil
	{
	public:
		static U32 CalculateSize(
			U32 width, 
			U32 height,
			E_CompressionFormat compressionFormat);

		static U32 GetBlockSizePixels(
			E_CompressionFormat compressionFormat);

		static U32 GetBlockSizeBytes(
			E_CompressionFormat compressionFormat);

		static U32 GetWordSize(
			E_CompressionFormat compressionFormat);

		static U32 GetDestDimension(
			U32 sourceDimension, 
			E_CompressionFormat compressionFormat);

		static void EndianSwap(
			U8* pData,
			U32 size,
			E_CompressionFormat compressionFormat);
	};
}
