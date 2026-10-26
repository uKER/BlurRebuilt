#pragma once

namespace Wrap32CompressLib
{
	struct CalculateSizeArguments
	{
		U32 srcWidth;
		U32 srcHeight;
		CompressLib::E_CompressionFormat compressionFormat;
	};

	struct GetCompressLibHandleArguments
	{
		CompressLib::E_CompressionLibrary compressionLibrary;
	};

	struct GetDefaultCompressionTechniqueArguments
	{
		WRAP32_FAR_POINTER compressLibHandle;
		CompressLib::E_CompressionFormat compressionFormat;
	};

	struct CompressSingleMipLevelArguments
	{
		WRAP32_FAR_POINTER compressLibHandle;
		//Pointer to our data.
		WRAP32_SHARED_MEMORY_OFFSET pDestination;
		//Data size in bytes.
		U32	destSize;
		CompressLib::E_CompressionFormat compressionFormat;
		bool useAlpha;
		U32 compressionTechnique;
		CompressLib::E_Endianness outputEndianness;
		U32 width;
		U32 height;
		Attk::E_PixelFormat format;
		WRAP32_SHARED_MEMORY_OFFSET pSource;
		U32	srcSize;
	};
}

