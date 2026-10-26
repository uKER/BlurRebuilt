#include "stdafx.h"
#include <nvimage/nvtt/nvtt.h>
//#include <nvcore/StrLib.h>
#include <nvcore/StdStream.h>

using namespace CompressLib;

const S32 C_CompressLibNVidia::m_libFormatEnumTable[] =
{
	nvtt::Format_DXT1,
	-1,
	nvtt::Format_DXT3,
	-1,
	nvtt::Format_DXT5,
	nvtt::Format_DXT5n,
};

const E_CompressionFormat C_CompressLibNVidia::m_genericFormatEnumTable[] = 
{ 
	E_CompressionFormat_DXT1,
	E_CompressionFormat_DXT2,
	E_CompressionFormat_DXT3,
	E_CompressionFormat_DXT4,
	E_CompressionFormat_DXT5,
	E_CompressionFormat_DXN
};

const S32 C_CompressLibNVidia::m_defaultTechniqueMapping[] =
{
	nvtt::Quality_Production,	//E_CompressionFormat_DXT1
	0,							//E_CompressionFormat_DXT2
	nvtt::Quality_Production,	//E_CompressionFormat_DXT3
	0,							//E_CompressionFormat_DXT4
	nvtt::Quality_Production,	//E_CompressionFormat_DXT5
	nvtt::Quality_Production	//E_CompressionFormat_DXN
};

const S32 C_CompressLibNVidia::m_techniqueEnumTable[] =
{
	nvtt::Quality_Production,
	nvtt::Quality_Highest
};

const char* C_CompressLibNVidia::m_techniqueStringTable[] = 
{ 
	"Quality_Production",
	"Quality_Highest",
	"\0"
};

C_CompressLibNVidia C_CompressLibNVidia::m_instance;


C_CompressLibNVidia::C_CompressLibNVidia() 
{
	m_libFormatEnumTableRef = m_libFormatEnumTable;
	m_genericFormatEnumTableRef = m_genericFormatEnumTable;

	m_techniqueEnumTableRef = m_defaultTechniqueMapping;
	m_techniqueStringTableRef = m_techniqueStringTable;
	m_defaultTechniqueMappingRef = m_defaultTechniqueMapping;
}

struct MyOutputHandler : public nvtt::OutputHandler
{
	MyOutputHandler(U8* _pOutputData) :
		total(0), progress(0), percentage(0), pOutputData(_pOutputData) {}
	virtual ~MyOutputHandler() { }
	
	virtual void setTotal(int t)
	{
		printf("\r    ");
		total = t;
	}

	virtual void mipmap(int size, int width, int height, int depth, int face, int miplevel)
	{
		// ignore.
	}
	
	// Output data.
	virtual void writeData(const void * data, int size)
	{
		//Ignore DDS header.
		memcpy(pOutputData+progress, data, size);

		progress += size;
		int p = (100 * progress) / total;
		if (p != percentage)
		{
			percentage = p;
			printf("\r%d%%", percentage);
			fflush(stdout);
		}
	}
	
	int total;
	int progress;
	int percentage;
	U8* pOutputData;
};

struct MyErrorHandler : public nvtt::ErrorHandler
{
	virtual void error(nvtt::Error e)
	{
		nvDebugBreak();
	}
};

bool C_CompressLibNVidia::CompressSingleMipLevel(
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
		printf("NVidia Texture Tools 2 library (Alpha Build 0.93.0703.0518) does not currently support DXT1 (DXT1a) with 1-bit alpha.\n");
		return false;
	}

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
	if (width < destHeight || height < destHeight || format != Attk::E_PixelFormat_B8G8R8A8U)
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

	MyErrorHandler errorHandler;
	MyOutputHandler outputHandler(output.pData);

	// Set input options.
	nvtt::InputOptions inputOptions;
	inputOptions.setTextureLayout(
		nvtt::TextureType_2D, 
		destWidth, 
		destHeight);

	inputOptions.setMipmapData(
		tmp.GetPixelDataPtr(),
		destWidth, 
		destHeight);
	if (compressionFormat == E_CompressionFormat_DXN)
	{
		inputOptions.setConvertToNormalMap(false);
		inputOptions.setGamma(1.0f, 1.0f);
		inputOptions.setNormalizeMipmaps(true);
	}
	else
	{
		inputOptions.setConvertToNormalMap(false);
		inputOptions.setGamma(2.2f, 2.2f);
		inputOptions.setNormalizeMipmaps(false);
	}

	if (compressionFormat == E_CompressionFormat_DXT1)
	{
		inputOptions.setQuantization(
			false,	// colour dithering
			false,	// alpha dithering
			true,	//binary alpha
			127);
	}
	else
	{
		inputOptions.setQuantization(
			false,	// colour dithering
			false,	// alpha dithering
			false);	// binary alpha
	}

	inputOptions.setFormat(nvtt::InputFormat_BGRA_8UB, useAlpha);
	inputOptions.setMipmapping(false);

	nvtt::CompressionOptions compressionOptions;
	compressionOptions.setQuality(nvtt::Quality_Normal);
	compressionOptions.setFormat((nvtt::Format)libCompressionFormat);

	compressionOptions.enableHardwareCompression(false);
	compressionOptions.setColorWeights(1, 1, 1);

	outputHandler.setTotal(nvtt::estimateSize(inputOptions, compressionOptions));
	nvtt::OutputOptions outputOptions(&outputHandler, &errorHandler);
	outputOptions.outputHeader = false;
	nvtt::compress(inputOptions, outputOptions, compressionOptions);

	if (outputEndianness == E_Endianness_BigEndian)
	{
		C_CompressedDataUtil::EndianSwap(
			output.pData, 
			output.dataSize,
			compressionFormat);
	}

	return true;
}

bool C_CompressLibNVidia::CompressSingleMipLevel(
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

const C_CompressLibNVidia& C_CompressLibNVidia::Instance()
{
	return m_instance;
}
