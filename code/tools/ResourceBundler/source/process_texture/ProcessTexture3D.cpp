#include "stdafx.h"

#include "../BundlerOutputOptions.h"
#include "../Bundler.h"
#include <export_resource/ExportTextureOptions.h>

#include "ProcessedMipmapChain.h"
#include "IProcessTexture.h"
#include "ProcessArrayOfTextures2D.h"
#include "ProcessTexture3D.h"

#include <export_resource/IExportTexture.h>
#include <export_resource/IExportTextureFactory.h>
#include <export_resource/OutputTextureData.h>
#include <export_resource/OutputResourceHeader.h>

#include <export_resource_header/IExportResourceHeader.h>
#include <export_resource_header/ExportHeaderTexture.h>
#include <export_resource_header/ExportHeaderTextureArray2D.h>
#include <export_resource_header/ExportHeaderTexture3D.h>

using namespace ResourceBundler;

bool C_ProcessTexture3D::Read(
	const C_Bundler& bundler, 
	const Sys::C_XmlAttributeList &attributes)
{
	if (!__super::Read(bundler, attributes))
		return false;

	m_widthOverride = -1;
	m_heightOverride = -1;

	for (int i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute *pAttr = attributes[i];
		Util::C_String n = pAttr->name.Lower();

		if (n == "width")
			m_widthOverride = atoi(pAttr->value);
		else if (n == "height")
			m_heightOverride = atoi(pAttr->value);
	}

	return true;
}
bool C_ProcessTexture3D::Process(C_Bundler& bundler)
{
	printf("Process 3D Texture: %s\n", m_exportOptions.name.Chars());

	const I_ExportTextureFactory* pExportFactory = GetExportTextureFactory(
		bundler.GetOutputOptions().platform);

	C_OutputTextureData outputData;
	C_OutputResourceHeader outputHeader;
	I_ExportTexture* pExportTexture = pExportFactory->CreateTexture3D();
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
	C_ExportHeaderTexture3D* pResourceHeader = new C_ExportHeaderTexture3D(
		m_exportOptions,
		outputHeader,
		dataOffset,
		outputData.GetDataSize(),
		compressedSize);
	bundler.AddResource(pResourceHeader);

	delete pExportTexture;

	return true;
}

bool C_ProcessTexture3D::_Process(
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

	if (m_sources.Size() == 0)
		return false;

	const I_ExportTextureFactory* pExportFactory = GetExportTextureFactory(
		bundler.GetOutputOptions().platform);

	Attk::C_Image* pImageList = new Attk::C_Image[m_sources.Size()];
	for (U32 iImage = 0; iImage < (U32)m_sources.Size(); iImage++)
	{
		Btl::String alphaSource("");
		if (iImage < (U32)m_alphaSources.Size())
			alphaSource = m_alphaSources[iImage];
		if (!LoadTexture(pImageList[iImage], bundler, m_sources[iImage], alphaSource))
		{
			printf("Error: Unable to read %s/%s\n", m_sources[iImage].Chars(), alphaSource.Chars());
			return false;
		}
	}

	//Perform any necessary resizing operations.
	{
		if (!ResizeTexture(pImageList[0], m_widthOverride, m_heightOverride))
			return false;

		for (U32 iImage = 1; iImage < (U32)m_sources.Size(); iImage++)
			if (!ResizeTexture(pImageList[iImage], pImageList[0].GetWidth(), pImageList[0].GetHeight()))
				return false;
	}

	//Combine individual layers into single 3d texture
	Attk::C_Image texture3d;
	{
		if (!texture3d.NewPixelData(
				pImageList[0].GetWidth(),
				pImageList[0].GetHeight(),
				m_sources.Size(),
				pImageList[0].GetPixelFormat(),
				0))
		{
				printf("Error: Out of memory - Failed to combine layers into single volume texture\n");
				return false;
		}

		U8* imgDataPtr = texture3d.GetPixelDataPtr();
		for (U32 iImage=0; iImage<(U32)m_sources.Size(); iImage++)
		{
			U32 size = pImageList[iImage].GetPixelDataSize();
			memcpy(imgDataPtr, pImageList[iImage].GetPixelDataPtr(), size);
			imgDataPtr += size;
		}
	}

	C_ProcessedMipmapChain* pProcessedMipmapChain = new C_ProcessedMipmapChain(
		pImageList[0].GetWidth(),
		pImageList[0].GetHeight(),
		m_sources.Size());
	if (!pProcessedMipmapChain)
			return false;

	//Now generate our mip-chain in the correct format.
	if (!GenerateProcessedMipmapChain(
			bundler,
			*pProcessedMipmapChain,
			texture3d))
			return false;

	exportTexture.AddMipmapChain(pProcessedMipmapChain);

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

	delete[] pImageList;

	//We're done.
	return true;
}

