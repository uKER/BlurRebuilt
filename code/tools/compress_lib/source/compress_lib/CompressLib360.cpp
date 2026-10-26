#include "stdafx.h"

using namespace CompressLib;

#ifdef COMPRESS_LIB_ENABLE_X360

const S32 C_CompressLib360::m_libFormatEnumTable[] =
{
	D3DFMT_LIN_DXT1,
	D3DFMT_LIN_DXT2,
	D3DFMT_LIN_DXT3,
	D3DFMT_LIN_DXT4,
	D3DFMT_LIN_DXT5,
	D3DFMT_LIN_DXN
};

const E_CompressionFormat C_CompressLib360::m_genericFormatEnumTable[] = 
{ 
	E_CompressionFormat_DXT1,
	E_CompressionFormat_DXT2,
	E_CompressionFormat_DXT3,
	E_CompressionFormat_DXT4,
	E_CompressionFormat_DXT5,
	E_CompressionFormat_DXN
};

const S32 C_CompressLib360::m_defaultTechniqueMapping[] =
{
	0,	//E_CompressionFormat_DXT1
	0,	//E_CompressionFormat_DXT2
	0,	//E_CompressionFormat_DXT3
	0,	//E_CompressionFormat_DXT4
	0,	//E_CompressionFormat_DXT5
	0,	//E_CompressionFormat_DXN
};

const S32 C_CompressLib360::m_techniqueEnumTable[] =
{
	0
};

const char* C_CompressLib360::m_techniqueStringTable[] = 
{ 
	"Default",
	"\0"
};

C_CompressLib360 C_CompressLib360::m_instance;

C_CompressLib360::C_CompressLib360() 
{
	m_libFormatEnumTableRef = m_libFormatEnumTable;
	m_genericFormatEnumTableRef = m_genericFormatEnumTable;

	m_techniqueEnumTableRef = m_defaultTechniqueMapping;
	m_techniqueStringTableRef = m_techniqueStringTable;
	m_defaultTechniqueMappingRef = m_defaultTechniqueMapping;
}

bool C_CompressLib360::CompressSingleMipLevel(
	C_OutputTexture& output,
	E_CompressionFormat compressionFormat,
	bool useAlpha,
	S32 compressionTechnique,
	E_Endianness outputEndianness,
	U32 width,
	U32 height,
	Attk::E_PixelFormat format,
	const U8* pInput) const
{
	Attk::C_Image tmpImage;
	const U8* pNewInput;

	if (format != Attk::E_PixelFormat_B8G8R8A8U)
	{
		tmpImage.NewPixelData(width, height, 1, format, pInput);
		if (!tmpImage.ChangePixelFormat(Attk::E_PixelFormat_B8G8R8A8U))
			return false;
		pNewInput = tmpImage.GetPixelDataPtr();
	}
	else
		pNewInput = pInput;

	if (GetCompressionFormat(compressionFormat) == -1)
		return false;

	if (compressionTechnique == -1)
		compressionTechnique = GetDefaultCompressionTechnique(compressionFormat);

	if (compressionTechnique == -1)
		return false;

	U32 destWidth = C_CompressedDataUtil::GetDestDimension(width, compressionFormat);
	U32 destHeight = C_CompressedDataUtil::GetDestDimension(height, compressionFormat);

	U32 sourcePitch = width * 4;
	U32 originalSize = Attk::C_BaseImage::GetBytesPerPixel(Attk::E_PixelFormat_B8G8R8A8U) * width * height;

	U32 d3dSwappedSrcSize;
	D3DTexture headerSwappedSrc;
	XGSetTextureHeaderEx(
		destWidth, 
		destHeight,
		1, 
		0,
		D3DFMT_LIN_A8R8G8B8, 
		0,
		0,
		0,
		XGHEADER_CONTIGUOUS_MIP_OFFSET, 
		0,
		(D3DTexture*)&headerSwappedSrc, 
		&d3dSwappedSrcSize,
		0);

	D3DFORMAT d3dCompressionFormat = (D3DFORMAT)GetCompressionFormat(compressionFormat);
	U32 d3dDXTSize;
	D3DTexture headerDXT;
	XGSetTextureHeaderEx(
		destWidth, 
		destHeight,
		1, 
		0,
		d3dCompressionFormat, 
		0,
		0,
		0,
		XGHEADER_CONTIGUOUS_MIP_OFFSET, 
		0,
		(D3DTexture*)&headerDXT, 
		&d3dDXTSize,
		0);

	U8* pTemp = new U8[d3dDXTSize];

	U8* pSwappedInput = new U8[d3dSwappedSrcSize];
	//Xbox 360 XDK wants it's data in big endian format. The compression lib requires
	//that all data is provided as little-endian, so we're going to need to endian swap here.
	XGEndianSwapSurface(
		pSwappedInput, 
		sourcePitch, 
		pNewInput, 
		sourcePitch, 
		width, 
		height,  
		D3DFMT_LIN_A8R8G8B8); 

	if (C_CompressedDataUtil::CalculateSize(
		destWidth,
		destHeight,
		compressionFormat) != 
		output.dataSize)
	{
		assert(!"Data size mismatch!");
		return false;
	}

	HRESULT hr = XGCompressSurface(
		pTemp,										// Base address of the destination (compressed) image
		output.dataSize / destHeight * 4,			// Distance in bytes between one row of the destination image and the next
		width,										// Width of the destination image in texels
		height,										// Height of the destination image in texels
		d3dCompressionFormat,						// Compressed format of the destination image
		0,											// Offset in the destination image to place the compressed rectangle
		pSwappedInput,								// Base address of the source (uncompressed) image
		sourcePitch,								// Distance in bytes between one row of the source image and the next
		D3DFMT_LIN_A8R8G8B8,						// Format of the source image
		0,											// Rectangle within the source image to compress
		XGCOMPRESS_NO_DITHERING,					// Compression flags
		0.5f);										// Alpha threshold for DXT1 transparency

	delete[] pSwappedInput;

	if (hr != S_OK)
		return false;

	memcpy(output.pData, pTemp, output.dataSize);
	delete[] pTemp;

	if (outputEndianness == E_Endianness_LittleEndian)
	{
		//Our XDK output is big-endian, we're going to need to swap.
		C_CompressedDataUtil::EndianSwap(
			output.pData, 
			output.dataSize,
			compressionFormat);
	}

	return true;

}

bool C_CompressLib360::CompressSingleMipLevel(
	C_OutputTexture& output, 
	E_CompressionFormat compressionFormat,
	bool useAlpha,
	S32 compressionTechnique,
	E_Endianness outputEndianness,
	const Attk::C_Image& input) const
{
	if (input.GetPixelFormat() == Attk::E_PixelFormat_B8G8R8A8U)
	{
		return CompressSingleMipLevel(
			output,
			compressionFormat,
			useAlpha,
			compressionTechnique,
			outputEndianness,
			input.GetWidth(),
			input.GetHeight(),
			input.GetPixelFormat(),
			input.GetPixelDataPtr());
	}
	else
	{
		Attk::C_Image tmp(input);
		tmp.ChangePixelFormat(Attk::E_PixelFormat_B8G8R8A8U);
		return CompressSingleMipLevel(
			output,
			compressionFormat,
			useAlpha,
			compressionTechnique,
			outputEndianness,
			tmp.GetWidth(),
			tmp.GetHeight(),
			tmp.GetPixelFormat(),
			tmp.GetPixelDataPtr());
	}
}

const C_CompressLib360& C_CompressLib360::Instance()
{
	return m_instance;
}

#endif
