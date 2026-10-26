#include "stdafx.h"

#include "../BundlerOutputOptions.h"
#include "../Bundler.h"
#include <export_resource/ExportTextureOptions.h>

#include "ProcessedMipmapChain.h"
#include "IProcessTexture.h"
#include "ProcessArrayOfTextures2D.h"
#include "ProcessTextureCubeMap.h"

#include <export_resource/IExportTexture.h>
#include <export_resource/IExportTextureFactory.h>
#include <export_resource/OutputTextureData.h>
#include <export_resource/OutputResourceHeader.h>

#include <export_resource_header/IExportResourceHeader.h>
#include <export_resource_header/ExportHeaderTexture.h>
#include <export_resource_header/ExportHeaderTextureCubeMap.h>

using namespace ResourceBundler;

bool C_ProcessTextureCubeMap::Read(
   const C_Bundler& bundler, 
   const Sys::C_XmlAttributeList &attributes)
{
	if (!__super::Read(bundler, attributes))
		return false;

	S32 size = -1;

	for (int i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute *pAttr = attributes[i];
		Util::C_String n = pAttr->name.Lower();
	
		if (n == "size")
		{
			size = atoi(pAttr->value);
		}
		else if (n == "basefilterangle")
		{
			m_CubeMapVariables.baseFilterAngle = (F32)atof(pAttr->value);
		}
		else if (n == "initialmipangle")
		{
			m_CubeMapVariables.initialMipAngle = (F32)atof(pAttr->value);
		}
		else if (n == "mipangleperlevelscale")
		{
			m_CubeMapVariables.mipAnglePerLevelScale = (F32)atof(pAttr->value);
		}
		else if (n == "filtertype")
		{
			Util::C_String filterType(pAttr->value);
			filterType.Lower();

			if (filterType == "disc")
				m_CubeMapVariables.filterType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_FilterType_Disc;
			else if (filterType == "cone")
				m_CubeMapVariables.filterType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_FilterType_Cone;
			else if (filterType == "cosine")
				m_CubeMapVariables.filterType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_FilterType_Cosine;
			else if (filterType == "angulargaussian")
				m_CubeMapVariables.filterType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_FilterType_AngularGaussian;
			else
			{
				printf("Unrecognised edge filter type.\n");
				return false;
			}
		}
		else if (n == "fixuptype")
		{
			Util::C_String fixupType(pAttr->value);
			fixupType.Lower();

			if (fixupType == "none")
				m_CubeMapVariables.fixupType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_EdgeFixupType_None;
			else if (fixupType == "pulllinear")
				m_CubeMapVariables.fixupType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_EdgeFixupType_PullLinear;
			else if (fixupType == "pullhermite")
				m_CubeMapVariables.fixupType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_EdgeFixupType_PullHermite;
			else if (fixupType == "avglinear")
				m_CubeMapVariables.fixupType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_EdgeFixupType_AvgLinear;
			else if (fixupType == "avghermite")
				m_CubeMapVariables.fixupType = AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_EdgeFixupType_AvgHermite;
			else
			{
				printf("Unrecognised edge fixup type.\n");
				return false;
			}
		}
		else if (n == "fixupwidth")
		{
			m_CubeMapVariables.fixupWidth = atoi(pAttr->value);
		}
		else if (n == "useSolidAngle")
		{
			Util::C_String useSolidAngle(pAttr->value);
			useSolidAngle.Lower();

			if (useSolidAngle == "false")
				m_CubeMapVariables.useSolidAngle = FALSE;
			else
				m_CubeMapVariables.useSolidAngle = TRUE;
		}
	}

	if (m_sources.Size() < 6)
	{
		printf("Insufficient images for cube map.\n");
		return false;
	}
	else if (m_sources.Size() > 6)
	{
		printf("Cube map has too many images.\n");
		return false;
	}

	if (m_alphaSources.Size() != 6)
	{
		m_alphaSources.Resize(6);
	}
	
	m_widthOverride = size;
	m_heightOverride = size;

	m_CubeMapVariables.compression = (m_exportOptions.outputFormat == Display::ResourceBundler::E_TextureFormat_DXT1) ?
										AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_Compression_DXT1
									:(m_exportOptions.outputFormat == Display::ResourceBundler::E_TextureFormat_DXT3) ?
										AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_Compression_DXT3
									:(m_exportOptions.outputFormat == Display::ResourceBundler::E_TextureFormat_DXT5) ?
										AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_Compression_DXT5
									: AttkUt::C_GenerateMipmaps::CubeMapFilterParameters::E_Compression_None;

	return true;
}

bool C_ProcessTextureCubeMap::Process(
	C_Bundler& bundler)
{
	printf("Process Cube Map: %s\n", m_exportOptions.name.Chars());
	const I_ExportTextureFactory* pExportFactory = GetExportTextureFactory(
		bundler.GetOutputOptions().platform);

	//Load up to six images.
	Attk::C_Image imgList[6];
	for (U32 i = 0; i <6; i++)
	{
		if (!LoadTexture(imgList[i], bundler, m_sources[i], m_alphaSources[i]))
		{
			printf("Error: Unable to read %s/%s\n", m_sources[i].Chars(), m_alphaSources[i].Chars());
			return false;
		}
	}

	// Determine width/height of cubemap images
	if (m_widthOverride == -1)	// Size not specified, so determine size (Set to a power of 2)
		for (m_widthOverride = 1; m_widthOverride < (S32)imgList[0].GetWidth(); m_widthOverride<<=1);
	m_heightOverride = m_widthOverride;

	C_OutputTextureData outputData;
	C_OutputResourceHeader outputHeader;
	I_ExportTexture* pExportTexture = pExportFactory->CreateTextureCubeMap();
	if (!pExportTexture)
		return false;

	if (!_Process(
		bundler,
		*pExportTexture,
		outputData,
		outputHeader))
		return false;

	//Offset for next data chunk.
	const U32 dataOffset = bundler.GetDataOffset();
	//This is where the texture data is written into our data chunk.
	U32 compressedSize;
	if (!bundler.WriteData(
		compressedSize,
		outputData.GetDataPtr(), 
		outputData.GetDataSize()))
		return false;

	//Add a new resource header to our bundle.
	C_ExportHeaderTextureCubeMap* pResourceHeader = new C_ExportHeaderTextureCubeMap(
		m_exportOptions,
		outputHeader,
		dataOffset,
		outputData.GetDataSize(),
		compressedSize);
	bundler.AddResource(pResourceHeader);

	delete pExportTexture;

	//We're done.
	return true;
}

bool C_ProcessTextureCubeMap::_Process(
	C_Bundler& bundler,
	I_ExportTexture& exportTexture,
	C_OutputTextureData& outputData,
	C_OutputResourceHeader& outputHeader)
{
	if (m_widthOverride > 4096)
	{
		printf("Warning: Clamping width to 4096\n");
		m_widthOverride = 4096;
	}
	if (m_heightOverride > 4096)
	{
		printf("Warning: Clamping height to 4096\n");
		m_heightOverride = 4096;
	}

	if (m_sources.Size() != 6)
		return false;

	const I_ExportTextureFactory* pExportFactory = GetExportTextureFactory(
		bundler.GetOutputOptions().platform);

	Attk::C_CubeMap cubeMap;
	for (U32 iImage = 0; iImage < (U32)m_sources.Size(); iImage++)
	{
		Util::C_String alphaSource("");
		if (iImage < (U32)m_alphaSources.Size())
			alphaSource = m_alphaSources[iImage];
		if (!LoadTexture(cubeMap[iImage], bundler, m_sources[iImage], alphaSource))
		{
			printf("Error: Unable to read %s/%s\n", m_sources[iImage].Chars(), alphaSource.Chars());
			return false;
		}
	}

	//Perform any necessary resizing operations.
	{
		if (!ResizeTexture(cubeMap[0], m_widthOverride, m_heightOverride))
			return false;

		for (U32 iImage = 1; iImage < (U32)m_sources.Size(); iImage++)
			if (!ResizeTexture(cubeMap[iImage], cubeMap[0].GetWidth(), cubeMap[0].GetHeight()))
				return false;
	}

	C_ProcessedMipmapChain* pProcessedMipmapChains[6];
	for (U32 iImage = 0; iImage < (U32)m_sources.Size(); iImage++)
	{
		//Create a new mip-map chain on the heap which will be manager by I_ExportTexture.
		pProcessedMipmapChains[iImage] = new C_ProcessedMipmapChain(
			cubeMap[iImage].GetWidth(),
			cubeMap[iImage].GetHeight());
		if (!pProcessedMipmapChains[iImage])
			return false;
	}

	//Now generate our mip-chain in the correct format.
	if (!GenerateProcessedMipmapChain(
			bundler,
			pProcessedMipmapChains,
			cubeMap,
			m_CubeMapVariables))
			return false;

	for (U32 iImage = 0; iImage < (U32)m_sources.Size(); iImage++)
		exportTexture.AddMipmapChain(pProcessedMipmapChains[iImage]);

	//Create a new texture exporter and perform export.
	{
		//Generate output texture data.
		if (!exportTexture.Export(
			outputData,
			bundler.GetOutputOptions(),
			m_exportOptions))
			return false;
		assert(outputData.GetDataPtr());

		//Generate output header.
		if (!exportTexture.GetOutputHeader(outputHeader))
			return false;
	}

	//We're done.
	return true;
}