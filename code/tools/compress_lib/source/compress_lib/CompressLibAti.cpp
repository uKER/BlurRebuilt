#include "stdafx.h"

#ifdef COMPRESS_LIB_ENABLE_ATI

#include <ati_compress/ATI_Compress.h>

using namespace CompressLib;

const S32 C_CompressLibAti::m_libFormatEnumTable[] =
{
	ATI_TC_FORMAT_DXT1,		//E_CompressionFormat_DXT1
	-1,						//E_CompressionFormat_DXT2
	ATI_TC_FORMAT_DXT3,		//E_CompressionFormat_DXT3
	-1,						//E_CompressionFormat_DXT4
	ATI_TC_FORMAT_DXT5,		//E_CompressionFormat_DXT5
	ATI_TC_FORMAT_ATI2N_XY,	//E_CompressionFormat_DXN
};

const E_CompressionFormat C_CompressLibAti::m_genericFormatEnumTable[] = 
{ 
	E_CompressionFormat_DXT1,
	E_CompressionFormat_DXT2,
	E_CompressionFormat_DXT3,
	E_CompressionFormat_DXT4,
	E_CompressionFormat_DXT5,
	E_CompressionFormat_DXN
};

const S32 C_CompressLibAti::m_defaultTechniqueMapping[] =
{
	ATI_TC_FORMAT_DXT1,		//E_CompressionFormat_DXT1
	-1,						//E_CompressionFormat_DXT2
	ATI_TC_FORMAT_DXT3,		//E_CompressionFormat_DXT3
	-1,						//E_CompressionFormat_DXT4
	ATI_TC_FORMAT_DXT5,		//E_CompressionFormat_DXT5
	ATI_TC_FORMAT_ATI2N_XY	//E_CompressionFormat_DXN
};

const S32 C_CompressLibAti::m_techniqueEnumTable[] =
{
    ATI_TC_FORMAT_DXT1,
    ATI_TC_FORMAT_DXT3,
    ATI_TC_FORMAT_DXT5,
	ATI_TC_FORMAT_ATI1N,
    ATI_TC_FORMAT_ATI2N,
    ATI_TC_FORMAT_ATI2N_XY,
    ATI_TC_FORMAT_ATI2N_DXT5
};

const char* C_CompressLibAti::m_techniqueStringTable[] = 
{ 
	"DXT1",
	"DXT3",
	"DXT5",
	"ATI1N",
    "ATI2N",
    "ATI2N_XY",
    "ATI2N_DXT5",
	"\0"
};	

C_CompressLibAti::C_CompressLibAti() 
{
	m_techniqueEnumTableRef = m_defaultTechniqueMapping;
	m_techniqueStringTableRef = m_techniqueStringTable;
	m_defaultTechniqueMappingRef = m_defaultTechniqueMapping;
}

C_CompressLibAti C_CompressLibAti::m_instance;

bool C_CompressLibAti::CompressSingleMipLevel(
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
	if (compressionFormat == E_CompressionFormat_DXT1 && useAlpha)
	{
		printf("ATI Compressinator library is bugged and does not support DXT1 (DXT1a) with 1-bit alpha.\n");
		return false;
	}
	U32 destWidth = C_CompressedDataUtil::GetDestDimension(width, compressionFormat);
	U32 destHeight = C_CompressedDataUtil::GetDestDimension(height, compressionFormat);

	Attk::C_Image tmp;
	//If we need to resize or change our image format...
	if (width < destWidth || height < destHeight || format != Attk::E_PixelFormat_B8G8R8A8U)
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

		if (!tmp.ChangePixelFormat(Attk::E_PixelFormat_B8G8R8A8U))
			return false;

		pInput = tmp.GetPixelDataPtr();

		width = tmp.GetWidth();
		height = tmp.GetHeight();
	}
	U32 sourcePitch = width * 4;
	U32 destPitch = destWidth * 4;

	if (compressionTechnique == -1)
		compressionTechnique = GetDefaultCompressionTechnique(compressionFormat);

	//Are our format and technique compatible?
	if (!ValidateFormatAndTechnique(
		compressionFormat, 
		compressionTechnique))
		return false;

	//The Ati library doesn't define format and technique in seperation.
	S32 atiFormat = compressionTechnique;

	if (C_CompressedDataUtil::CalculateSize(
		destWidth,
		destHeight,
		compressionFormat) != 
		output.dataSize)
	{
		assert(!"Data size mismatch!");
		return false;
	}

	// Init source texture
	ATI_TC_Texture srcTexture;
	srcTexture.dwSize = sizeof(srcTexture);
	srcTexture.dwWidth = width;
	srcTexture.dwHeight = height;
	srcTexture.dwPitch = sourcePitch;
	srcTexture.format = ATI_TC_FORMAT_ARGB_8888;
	srcTexture.dwDataSize = ATI_TC_CalculateBufferSize(&srcTexture);
	//Doesn't actually modify anything.
	srcTexture.pData = const_cast<U8*>(pInput);


	// Init dest texture
	ATI_TC_Texture destTexture;
	destTexture.dwSize = sizeof(destTexture);
	destTexture.dwWidth = destWidth;
	destTexture.dwHeight = destHeight;
	destTexture.dwPitch = destPitch;
	destTexture.format = (ATI_TC_FORMAT)atiFormat;
	destTexture.dwDataSize = ATI_TC_CalculateBufferSize(&destTexture);
	destTexture.pData = output.pData;

	ATI_TC_CompressOptions options;
	options.bUseChannelWeighting = true;
	options.fWeightingRed = 0.33f;
	options.fWeightingGreen = 0.33f;
	options.fWeightingBlue = 0.33f;
	options.bUseAdaptiveWeighting = false;
	options.bDXT1UseAlpha = useAlpha;
	options.nAlphaThreshold = 127;

	// Compress
	ATI_TC_ERROR result = ATI_TC_ConvertTexture(
		&srcTexture, 
		&destTexture, 
		&options, 
		NULL, 
		NULL, 
		NULL);
	if (result != ATI_TC_OK)
		return false;

	if (outputEndianness == E_Endianness_BigEndian)
	{
		//The ATI library's output is big-endian, we're going to need to swap.
		C_CompressedDataUtil::EndianSwap(
			output.pData,
			output.dataSize,
			compressionFormat);
	}

	return true;
}

bool C_CompressLibAti::CompressSingleMipLevel(
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

bool C_CompressLibAti::ValidateFormatAndTechnique(
	E_CompressionFormat compressionFormat,
	S32 compressionTechnique) const
{
	switch (compressionFormat)
	{
	case E_CompressionFormat_DXT1:
		if (compressionTechnique == ATI_TC_FORMAT_DXT1)
			return true;
		break;

	case E_CompressionFormat_DXT2:
		return false;

	case E_CompressionFormat_DXT3:
		if (compressionTechnique == ATI_TC_FORMAT_DXT3)
			return true;
		break;

	case E_CompressionFormat_DXT4:
		return false;

	case E_CompressionFormat_DXT5:
		if (compressionTechnique == ATI_TC_FORMAT_DXT5)
			return true;
		break;

	case E_CompressionFormat_DXN:
		if (
			compressionTechnique == ATI_TC_FORMAT_ATI1N ||
			compressionTechnique == ATI_TC_FORMAT_ATI2N ||
			compressionTechnique == ATI_TC_FORMAT_ATI2N_XY ||
			compressionTechnique == ATI_TC_FORMAT_ATI2N_DXT5)
			return true;
		break;

	default:
		break;
	};

	return false;
}

const C_CompressLibAti& C_CompressLibAti::Instance()
{
	return m_instance;
}

#endif
