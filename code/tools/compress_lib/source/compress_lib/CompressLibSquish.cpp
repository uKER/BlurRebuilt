#include "stdafx.h"
#include <squish/squish.h>

using namespace CompressLib;
/*
	//! Use DXT1 compression.
	kDxt1 = ( 1 << 0 ), 
	
	//! Use DXT3 compression.
	kDxt3 = ( 1 << 1 ), 
	
	//! Use DXT5 compression.
	kDxt5 = ( 1 << 2 ), 
	
	//! Use a slow but high quality colour compressor (the default).
	kColourClusterFit = ( 1 << 3 ),	
	
	//! Use a fast but low quality colour compressor.
	kColourRangeFit	= ( 1 << 4 ),
	
	//! Use a perceptual metric for colour error (the default).
	kColourMetricPerceptual = ( 1 << 5 ),

	//! Use a uniform metric for colour error.
	kColourMetricUniform = ( 1 << 6 ),
	
	//! Weight the colour by alpha during cluster fit (disabled by default).
	kWeightColourByAlpha = ( 1 << 7 )
	*/

const S32 C_CompressLibSquish::m_libFormatEnumTable[] =
{
	squish::kDxt1,
	-1,
	squish::kDxt3,
	-1,
	squish::kDxt5,
	-1
};

const E_CompressionFormat C_CompressLibSquish::m_genericFormatEnumTable[] = 
{ 
	E_CompressionFormat_DXT1,
	E_CompressionFormat_DXT2,
	E_CompressionFormat_DXT3,
	E_CompressionFormat_DXT4,
	E_CompressionFormat_DXT5,
	E_CompressionFormat_DXN
};

const S32 C_CompressLibSquish::m_defaultTechniqueMapping[] =
{
	squish::kWeightColourByAlpha,	//E_CompressionFormat_DXT1
	0,								//E_CompressionFormat_DXT2
	squish::kWeightColourByAlpha,	//E_CompressionFormat_DXT3
	0,								//E_CompressionFormat_DXT4
	squish::kWeightColourByAlpha,	//E_CompressionFormat_DXT5
	0								//E_CompressionFormat_DXN
};

const S32 C_CompressLibSquish::m_techniqueEnumTable[] =
{
	squish::kColourClusterFit | squish::kColourMetricPerceptual,
	squish::kColourClusterFit | squish::kColourMetricUniform,
};

const char* C_CompressLibSquish::m_techniqueStringTable[] = 
{ 
	"ColourClusterFit_ColourMetricPerceptual",
	"ColourClusterFit_ColourMetricUniform",
	"\0"
};

C_CompressLibSquish C_CompressLibSquish::m_instance;


C_CompressLibSquish::C_CompressLibSquish() 
{
	m_libFormatEnumTableRef = m_libFormatEnumTable;
	m_genericFormatEnumTableRef = m_genericFormatEnumTable;

	m_techniqueEnumTableRef = m_defaultTechniqueMapping;
	m_techniqueStringTableRef = m_techniqueStringTable;
	m_defaultTechniqueMappingRef = m_defaultTechniqueMapping;
}

bool C_CompressLibSquish::CompressSingleMipLevel(
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
	S32 libCompressionFormat = GetCompressionFormat(compressionFormat);
	if (libCompressionFormat == -1)
		return false;

	if (compressionTechnique == -1)
		compressionTechnique = GetDefaultCompressionTechnique(compressionFormat);

	if (compressionTechnique == -1)
		return false;

	U32 destWidth = C_CompressedDataUtil::GetDestDimension(width, compressionFormat);
	U32 destHeight = C_CompressedDataUtil::GetDestDimension(height, compressionFormat);
	
	Attk::C_Image tmp;
	//If we need to resize or change our image format...
	if (width < destHeight || height < destHeight || format != Attk::E_PixelFormat_R8G8B8A8U)
	{
		if (!tmp.NewPixelData(
			width,
			height,
			1,
			format,
			pInput))
			return false;

		if (!AttkUt::C_ImageResize::Resize(
			tmp, 
			destWidth, 
			destHeight, 
			1, 
			AttkUt::C_ImageFilter::E_PixelFilter_Box))
			return false;

		if (!tmp.ChangePixelFormat(Attk::E_PixelFormat_R8G8B8A8U))
			return false;
	}

	if (C_CompressedDataUtil::CalculateSize(
		destWidth,
		destHeight,
		compressionFormat) != 
		output.dataSize)
	{
		assert(!"Data size mismatch!");
		return false;
	}

	S32 flags = libCompressionFormat | compressionTechnique;
	if (useAlpha)
		flags |= squish::kWeightColourByAlpha;

	squish::CompressImage(
		tmp.GetPixelDataPtr(),	// The pixels of the source.
		width,					// The width of the source image.
		height,					// The height of the source image.
		output.pData,			// Storage for the compressed output.
		flags);					// Compression flags.

	if (outputEndianness == E_Endianness_BigEndian)
	{
		C_CompressedDataUtil::EndianSwap(
			output.pData, 
			output.dataSize,
			compressionFormat);
	}

	return true;

}

bool C_CompressLibSquish::CompressSingleMipLevel(
	C_OutputTexture& output, 
	E_CompressionFormat compressionFormat,
	bool useAlpha,
	S32 compressionTechnique,
	E_Endianness outputEndianness,
	const Attk::C_Image& input) const
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

const C_CompressLibSquish& C_CompressLibSquish::Instance()
{
	return m_instance;
}
