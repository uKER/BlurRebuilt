#include "stdafx.h"

#include "BundlerOutputOptions.h"
#include "Bundler.h"

#include <export_resource/IExportTexture.h>
#include <export_resource/OutputTextureData.h>

#include "Utility.h"

#include "../export_resource/ExportTextureOptions.h"
#include "../export_resource/IExportTextureFactory.h"
#include "../export_resource/360/ExportTextureFactory360.h"
#include "../export_resource/generic/ExportTextureFactoryGeneric.h"

#include "ProcessedMipmapChain.h"
#include "IProcessTexture.h"

using namespace ResourceBundler;

bool I_ProcessTexture::Read(
	const C_Bundler& bundler, 
	const Sys::C_XmlAttributeList &attributes)
{
	m_exportOptions.compressionLibrary = CompressLib::E_CompressionLibrary_Unknown;
	m_exportOptions.compressionTechnique = 0;
	m_exportOptions.numMipLevelsToGenerate = 0;

	for (int i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute *pAttr = attributes[i];
		Util::C_String n = pAttr->name.Lower();

		if (n == "name")
			m_exportOptions.name = pAttr->value;

		else if (n == "format")
		{
			if (!SetOutputFormatFromString(pAttr->value))
				return false;
		}

		else if (n == "compressor")
		{
			if (!SetCompressionSettingsFromString(pAttr->value))
				return false;
		}

		else if (n == "gamma")
		{
			if (!SetGammaCorrectionFromString(pAttr->value))
				return false;
		}

		else if (n == "discard_alpha")
		{
			if (!SetDiscardAlphaFromString(pAttr->value))
				return false;
		}
		else if (n == "levels")
		{
			if (!SetNumMipLevelsToGenerateFromString(pAttr->value))
				return false;
		}
	}

	return true;
}

bool I_ProcessTexture::LoadTexture(
	Attk::C_Image& image,
	const C_Bundler& bundler, 
	Util::C_String imgFile, 
	Util::C_String alphaFile)
{
	// Load image

	Attk::C_Status status;
	status = image.Read(imgFile);
	if (!status)
	{
		printf("Error: Failed to read %s\n", imgFile.Chars());//status.GetErrorString());
		return false;
	}
	if ((image.GetChannelsPerPixel() == 4) && (m_exportOptions.discardAlpha == false))
		m_useAlpha = true;
	else
		m_useAlpha = false;

	const Attk::E_PixelFormat intermediateFormat = Attk::E_PixelFormat_R32G32B32A32F;
	status = image.ChangePixelFormat(intermediateFormat);
	if (!status)
	{
		printf("Error: Failed to change format (%s)\n", imgFile.Chars());
		return false;
	}

	if ((alphaFile != "") && (m_exportOptions.discardAlpha == false))
	{
		Attk::C_Image alpha;
		status = alpha.Read(alphaFile);
		if (!status)
		{
			printf("Error: Failed to read %s\n", alphaFile.Chars());//status.GetErrorString());
			return false;
		}

		int w, h, d;

		w = Maths::Max(image.GetWidth(), alpha.GetWidth());
		h = Maths::Max(image.GetHeight(), alpha.GetHeight());
		d = Maths::Max(image.GetDepth(), alpha.GetDepth());

		AttkUt::C_ImageResize::Resize(image, w,h,d);
		AttkUt::C_ImageResize::Resize(alpha, w,h,d);

		alpha.ChangePixelFormat(intermediateFormat);

		int len = image.GetWidth() * image.GetHeight();

		// Copy alpha channel from alpha to img
		F32 *dst = (F32*) image.GetPixelDataPtr();
		F32 *src = (F32*) alpha.GetPixelDataPtr();

		if (intermediateFormat == Attk::E_PixelFormat_R32G32B32A32F)
		{
			dst+=3;	// Skip initial RGB
			src+=3;
		}

		for (int i=0; i<len; i++)
		{
			*dst = *src;

			dst += 4;
			src += 4;
		}

		m_useAlpha = true;
	}

	return true;
}

bool I_ProcessTexture::ResizeTexture(
	Attk::C_Image& image,
	S32 widthOverride,
	S32 heightOverride)
{
	U32 width = image.GetWidth();
	U32 height = image.GetHeight();
	U32 depth = image.GetDepth();
	if (widthOverride != -1)
		width = widthOverride;
	if (heightOverride != -1)
		height = heightOverride;

	//Downsize to next power of two for compressed images only.
	if (Utility::RequiresPow2Size(m_exportOptions.outputFormat) && 
		!Utility::IsPow2Size(width, height, depth))
	{
		printf("\tResize will be performed due to non power of two input size.\n");
		width	= Utility::MakePow2(image.GetWidth());
		height	= Utility::MakePow2(image.GetHeight());
		depth	= Utility::MakePow2(image.GetDepth());
	}

	if (width != image.GetWidth() || height != image.GetHeight())
	{
		printf("\tResizing from %d, %d to %d, %d\n", 
			image.GetWidth(), 
			image.GetHeight(),
			widthOverride, 
			heightOverride);

		Attk::C_Status status = AttkUt::C_ImageResize::Resize(
			image, 
			width, 
			height, 
			1);
		if (!status)
		{
			printf("Error: Failed to resize image\n");
			return false;
		}
	}

	return true;
}

bool I_ProcessTexture::GenerateProcessedMipmapChain(
	const C_Bundler& bundler, 
	C_ProcessedMipmapChain& processedMipmapChain,
	Attk::C_Image& img)
{
	//A little bit of bit-shifting to determine how many mip levels are required.
	{
		U32 levels = 1;

		if (img.GetDepth() == 1)
		{
			while (((1<<(levels-1)) < (S32)img.GetWidth()) && ((1<<(levels-1)) < (S32)img.GetHeight()))
				levels++;
		}
		else
		{
			while (((1<<(levels-1)) < (S32)img.GetWidth()) && ((1<<(levels-1)) < (S32)img.GetHeight()) && ((1<<(levels-1)) < (S32)img.GetDepth()))
				levels++;
		}

		if (m_exportOptions.numMipLevelsToGenerate < 1 || 
			m_exportOptions.numMipLevelsToGenerate > (S32)levels)
			m_exportOptions.numMipLevelsToGenerate = levels;
	}

	//Storage for our unprocessed mip-chain from the Attk.
	Attk::C_MipmapImageChain unprocessedMipmapChain;
	//Now generate our mip levels.
	{
		Attk::C_Status status = AttkUt::C_GenerateMipmaps::Generate(
			unprocessedMipmapChain, 
			img, 
			m_exportOptions.numMipLevelsToGenerate);
		if (!status)
		{
			printf("Failed to generate mipmap chain.\n");
			return false;
		}
	}

	return ProcessMipmapChain(bundler, unprocessedMipmapChain, processedMipmapChain);
}


bool I_ProcessTexture::GenerateProcessedMipmapChain(
	const C_Bundler& bundler, 
	C_ProcessedMipmapChain** processedMipmapChains,
	Attk::C_CubeMap& img,
	const AttkUt::C_GenerateMipmaps::CubeMapFilterParameters& filterParams)
{
	//A little bit of bit-shifting to determine how many mip levels are required.
	{
		U32 levels = 1;

		while (((1<<(levels-1)) < (S32)img[0].GetWidth()) && ((1<<(levels-1)) < (S32)img[0].GetHeight()))
			levels++;

		if (m_exportOptions.numMipLevelsToGenerate < 1 || 
			m_exportOptions.numMipLevelsToGenerate > (S32)levels)
			m_exportOptions.numMipLevelsToGenerate = levels;
	}

	//Storage for our unprocessed mip-chain from the Attk.
	Attk::C_MipmapCubeMapImageChain unprocessedMipmapChains;
	//Now generate our mip levels.
	{
		AttkUt::C_GenerateMipmaps::m_CubeMapFilterParams = filterParams;
		Attk::C_Status status = AttkUt::C_GenerateMipmaps::Generate(
			unprocessedMipmapChains,
			img, 
			m_exportOptions.numMipLevelsToGenerate);
		if (!status)
		{
			printf("Failed to generate mipmap chain.\n");
			return false;
		}
	}

	for (U32 iImage = 0; iImage < Attk::E_CubeMap_Face_Count; iImage++)
	{
		if (!ProcessMipmapChain(bundler, unprocessedMipmapChains[iImage], *processedMipmapChains[iImage]))
			return false;
	}
	return true;
}

bool I_ProcessTexture::ProcessMipmapChain(
	const C_Bundler& bundler,
	Attk::C_MipmapImageChain& unprocessedMipmapChain,
	C_ProcessedMipmapChain& processedMipmapChain)
{
	//process our mip levels, converting them to our output format.
	bool compressed = false;
	for (U32 iLevel = 0; iLevel < unprocessedMipmapChain.GetNumLevels(); iLevel++)
	{
		Attk::C_Image& mipLevel = *unprocessedMipmapChain.GetLevelPtr(iLevel);

		Display::ResourceBundler::E_TextureFormat mipmapFormat;
		U32 dataSize;
		switch(m_exportOptions.outputFormat)
		{
		//Compressed formats.
		case Display::ResourceBundler::E_TextureFormat_DXT1:
		case Display::ResourceBundler::E_TextureFormat_DXT2:
		case Display::ResourceBundler::E_TextureFormat_DXT3:
		case Display::ResourceBundler::E_TextureFormat_DXT4:
		case Display::ResourceBundler::E_TextureFormat_DXT5:
		case Display::ResourceBundler::E_TextureFormat_DXN:
			compressed = true;
			dataSize = CompressLib::C_CompressedDataUtil::CalculateSize(
				mipLevel.GetWidth(),
				mipLevel.GetHeight(),
				GetCompressLibFormat());
			mipmapFormat = m_exportOptions.outputFormat;
			break;

		case Display::ResourceBundler::E_TextureFormat_A8R8G8B8:
			mipLevel.ChangePixelFormat(Attk::E_PixelFormat_B8G8R8A8U);
			mipmapFormat = Display::ResourceBundler::E_TextureFormat_A8R8G8B8;
			dataSize = mipLevel.GetPixelDataSize();
			break;

		case Display::ResourceBundler::E_TextureFormat_L8:
			mipLevel.ChangePixelFormat(Attk::E_PixelFormat_Luminance8U);
			mipmapFormat = Display::ResourceBundler::E_TextureFormat_L8;
			dataSize = mipLevel.GetPixelDataSize();
			break;

		case Display::ResourceBundler::E_TextureFormat_A32R32G32B32:
			mipLevel.ChangePixelFormat(Attk::E_PixelFormat_B32G32R32A32F);
			mipmapFormat = Display::ResourceBundler::E_TextureFormat_A32R32G32B32;
			dataSize = mipLevel.GetPixelDataSize();
			break;

		default:
			printf("Unknown texture format specified.\n");
			return false;
		}

		//Convert our mip-level to the desired output format.
		if (compressed)
		{
			//Attempt to aquire an instance of this compression library.
			const CompressLib::I_CompressLib* pCompressLib = 
				CompressLib::C_CompressLibFactory::GetCompressLibPtr(m_exportOptions.compressionLibrary);
			if (!pCompressLib)
			{
				printf("Unable to instantiate compression library.\n");
				return false;
			}
			
			//This memory will be deallocated during destruction of the processed mip-chain.
			U8* pCompressedData = new U8[dataSize*mipLevel.GetDepth()];

			U8* pLayerData = mipLevel.GetPixelDataPtr();

			//We need to compress each layer separately
			for (U32 jLayer=0; jLayer<mipLevel.GetDepth(); jLayer++)
			{
				CompressLib::C_OutputTexture compressedOutput;
				
				compressedOutput.pData = pCompressedData + jLayer*dataSize;
				compressedOutput.dataSize = dataSize;

				CompressLib::E_Endianness textureEndianness;
				if (bundler.GetOutputOptions().bigEndianTexture)
					textureEndianness = CompressLib::E_Endianness_BigEndian;
				else
					textureEndianness = CompressLib::E_Endianness_LittleEndian;

				if (mipLevel.GetPixelFormat() == Attk::E_PixelFormat_Compressed_DXT1 ||
					mipLevel.GetPixelFormat() == Attk::E_PixelFormat_Compressed_DXT2 ||
					mipLevel.GetPixelFormat() == Attk::E_PixelFormat_Compressed_DXT3 ||
					mipLevel.GetPixelFormat() == Attk::E_PixelFormat_Compressed_DXT4 ||
					mipLevel.GetPixelFormat() == Attk::E_PixelFormat_Compressed_DXT5 ||
					mipLevel.GetPixelFormat() == Attk::E_PixelFormat_Compressed_DXN)
				{
					assert(Utility::GetEquivalentTextureFormat(mipLevel.GetPixelFormat()) == m_exportOptions.outputFormat);
					assert(compressedOutput.dataSize == mipLevel.GetPixelDataSize());
					memcpy(compressedOutput.pData, mipLevel.GetPixelDataPtr(), compressedOutput.dataSize);

					if (textureEndianness == CompressLib::E_Endianness_BigEndian)
					{
						CompressLib::C_CompressedDataUtil::EndianSwap(
							compressedOutput.pData,
							compressedOutput.dataSize,
							GetCompressLibFormat());
					}
					pLayerData += 
						Attk::Utility::GetSlicePitch(mipLevel.GetWidth(), mipLevel.GetHeight(), mipLevel.GetPixelFormat());

				}
				else
				{
					if (!pCompressLib->CompressSingleMipLevel(
						compressedOutput,
						GetCompressLibFormat(),
						m_useAlpha,
						m_exportOptions.compressionTechnique,
						textureEndianness,
						mipLevel.GetWidth(),
						mipLevel.GetHeight(),
						mipLevel.GetPixelFormat(),
						pLayerData))
					{
						return false;
					}
					pLayerData += mipLevel.GetWidth() * mipLevel.GetHeight() *
						Attk::C_BaseImage::GetBytesPerPixel(mipLevel.GetPixelFormat());
				}
			}
			processedMipmapChain.AddLevelPtr(
				pCompressedData,
				mipmapFormat,
				bundler.GetOutputOptions().bigEndianTexture);
		}
		else
		{
			//This memory will be deallocated during destruction of the processed mip-chain.
			U8* pData = new U8[dataSize];
			memcpy(pData, mipLevel.GetPixelDataPtr(), dataSize);
			processedMipmapChain.AddLevelPtr(
				pData,
				mipmapFormat,
				false);
		}
	}

	if (compressed)
	{
		printf("\tTexture converted to %s format using %s\n", 
			Display::ResourceBundler::g_textureFormat[m_exportOptions.outputFormat],
			CompressLib::C_CompressLibFactory::GetCompressLibString(m_exportOptions.compressionLibrary));
	}
	else
	{
		printf("\tTexture converted to %s format\n", 
			Display::ResourceBundler::g_textureFormat[m_exportOptions.outputFormat]);
	}
	return true;
}
Display::ResourceBundler::E_Platform I_ProcessTexture::GetPlatform(const Util::C_String& platform) const
{
	return Display::ResourceBundler::E_Platform_Unknown;
}

const I_ExportTextureFactory* I_ProcessTexture::GetExportTextureFactory(const Display::ResourceBundler::E_Platform platform)
{
	switch(platform)
	{
	case Display::ResourceBundler::E_Platform_Xbox360:
		return &C_ExportTextureFactory360::Instance();

	case Display::ResourceBundler::E_Platform_PC:
	case Display::ResourceBundler::E_Platform_PS3_OpenGL:
	case Display::ResourceBundler::E_Platform_PS3_LibGCM:
	case Display::ResourceBundler::E_Platform_Wii:
		return &C_ExportTextureFactoryGeneric::Instance();

	case Display::ResourceBundler::E_Platform_Unknown:
	default:
		return 0;
	};
	return 0;
}

CompressLib::E_CompressionFormat I_ProcessTexture::GetCompressLibFormat() const
{
	switch (m_exportOptions.outputFormat)
	{
		case Display::ResourceBundler::E_TextureFormat_DXT1:
			return CompressLib::E_CompressionFormat_DXT1;
		case Display::ResourceBundler::E_TextureFormat_DXT2:
			return CompressLib::E_CompressionFormat_DXT2;
		case Display::ResourceBundler::E_TextureFormat_DXT3:
			return CompressLib::E_CompressionFormat_DXT3;
		case Display::ResourceBundler::E_TextureFormat_DXT4:
			return CompressLib::E_CompressionFormat_DXT4;
		case Display::ResourceBundler::E_TextureFormat_DXT5:
			return CompressLib::E_CompressionFormat_DXT5;
		case Display::ResourceBundler::E_TextureFormat_DXN:
			return CompressLib::E_CompressionFormat_DXN;
		default:
			break;
	};

	assert(!"Invalid compression format");
	return CompressLib::E_CompressionFormat_Unknown;
}

bool I_ProcessTexture::SetOutputFormatFromString(const Util::C_String& format)
{
	m_exportOptions.outputFormat = Utility::GetTextureFormatType(format);
	if (m_exportOptions.outputFormat == Display::ResourceBundler::E_TextureFormat_Unknown)
	{
		printf("Error: Unknown output format (%s_\n", format.Chars());
		return false;
	}

	return true;
}

bool I_ProcessTexture::SetNumMipLevelsToGenerateFromString(const Util::C_String& numMipLevels)
{
	m_exportOptions.numMipLevelsToGenerate = atoi(numMipLevels.Chars());

	return true;
}

bool I_ProcessTexture::SetCompressionSettingsFromString(const Util::C_String& compressor)
{
	//There's a naming convention here. The "compressor" string specifies both the name
	//of the compression library to use as well as the compression technique to be used
	//from that library. The two are seperated by the first underscore in the string.

	S32 firstUnderscoreIndex = compressor.Search("_");

	Util::C_String compressionLibName;
	Util::C_String compressionTechniqueName;
	if (firstUnderscoreIndex == -1)
	{
		compressionLibName = compressor;
		compressionTechniqueName = "";
	}
	else
	{
		compressionLibName = compressor(0, firstUnderscoreIndex);
		compressionTechniqueName = compressor(firstUnderscoreIndex+1, compressor.Length());
	}

	//Find the enum of the compression library we're using.
	m_exportOptions.compressionLibrary = 
		CompressLib::C_CompressLibFactory::GetCompressLibType(compressionLibName);
	//Attempt to aquire an instance of this compression library.
	const CompressLib::I_CompressLib* pCompressLib = 
		CompressLib::C_CompressLibFactory::GetCompressLibPtr(m_exportOptions.compressionLibrary);
	if (!pCompressLib)
	{
		printf("Invalid compressor value [%s]. Check your RDF file.\n", compressor.Chars());
		return false;
	}

	if (compressionTechniqueName.Length() > 0)
	{
		m_exportOptions.compressionTechnique = pCompressLib->GetCompressionTechnique(compressionTechniqueName.Chars());
		if (m_exportOptions.compressionTechnique == -1)
			return false;
	}
	else
	{
		m_exportOptions.compressionTechnique = -1;
	}
	
	return true;
}

bool I_ProcessTexture::SetDiscardAlphaFromString(const Util::C_String& discardAlpha)
{
	Util::C_String tmp(discardAlpha);
	tmp.ConvertToLower();
	if (tmp == "true")
		m_exportOptions.discardAlpha = true;

	return true;
}

bool I_ProcessTexture::SetGammaCorrectionFromString(const Util::C_String& gamma)
{
	Util::C_String tmp(gamma);
	tmp.ConvertToLower();
	if (tmp == "true")
		m_exportOptions.gamma = true;

	return true;
}
