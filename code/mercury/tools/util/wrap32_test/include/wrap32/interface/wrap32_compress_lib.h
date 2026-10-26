#pragma once

namespace Attk
{
	enum E_PixelFormat;
}

namespace CompressLib
{
	enum E_CompressionFormat;
	enum E_Endianness;
	enum E_CompressionLibrary;

	struct C_OutputTexture;
}

namespace Wrap32CompressLib
{
	U32 
	WINAPI 
	CalculateSize(
		U32 srcWidth, 
		U32 srcHeight,
		CompressLib::E_CompressionFormat compressionFormat);

	HANDLE 
	WINAPI 
	GetCompressLibHandle(
		CompressLib::E_CompressionLibrary compressionLibrary);

	U32 
	WINAPI 
	GetDefaultCompressionTechnique(
		HANDLE compressLibHandle,
		CompressLib::E_CompressionFormat compressionFormat);

	bool 
	WINAPI 
	CompressSingleMipLevel(
		HANDLE compressLibHandle,
		U8* pDestination,
		U32 destSize,
		CompressLib::E_CompressionFormat compressionFormat,
		bool useAlpha,
		U32 compressionTechnique,
		CompressLib::E_Endianness outputEndianness,
		U32 width,
		U32 height,
		Attk::E_PixelFormat format,
		const U8* pSource,
		U32 srcSize);
}
