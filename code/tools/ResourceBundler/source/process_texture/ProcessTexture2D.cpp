#include "stdafx.h"

#include <display/bundler/ResourceBundlerDefines.h>
#include <display/bundler/ResourceHeader.h>
#include <display/bundler/ResourceTexture.h>
#include <display/bundler/ResourceTexture2D.h>

#include "../BundlerOutputOptions.h"
#include "../Bundler.h"
#include "../export_resource/ExportTextureOptions.h"

#include "ProcessedMipmapChain.h"
#include "IProcessTexture.h"
#include "ProcessArrayOfTextures2D.h"
#include "ProcessTexture2D.h"

#include "../export_resource/IExportTextureFactory.h"
#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>
#include "../export_resource/IExportTexture.h"
#include "../export_resource/OutputTextureData.h"

#include "../export_resource_header/IExportResourceHeader.h"
#include "../export_resource_header/ExportHeaderTexture.h"
#include "../export_resource_header/ExportHeaderTexture2D.h"

#include "Utility.h"

#include "../BundlerOutputOptions.h"
#include "../Bundler.h"

using namespace ResourceBundler;


bool C_ProcessTexture2D::Read(
	const C_Bundler& bundler, 
	const Sys::C_XmlAttributeList &attributes)
{
	if (!__super::Read(
		bundler,
		attributes))
		return false;

	m_widthOverride = -1;
	m_heightOverride = -1;

	for (int i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute *pAttr = attributes[i];
		Util::C_String n = pAttr->name.Lower();

		if (n == "source")
		{
			m_source = pAttr->value.Chars();
			if (!bundler.GetPath(m_source))
			{
				printf("Error: %s does not exist\n", m_source.Chars());
				return false;
			}
		}
		else if (n == "alphasource")
		{
			m_alphaSource = pAttr->value.Chars();
			if (!bundler.GetPath(m_alphaSource))
			{
				printf("Error: %s does not exist\n", m_alphaSource.Chars());
				return false;
			}
		}
		else if (n == "width")
			m_widthOverride = atoi(pAttr->value);
		else if (n == "height")
			m_heightOverride = atoi(pAttr->value);
	}

	return true;
}

bool C_ProcessTexture2D::Process(C_Bundler& bundler)
{
	printf("Process 2D Texture: %s\n", m_exportOptions.name.Chars());

	const I_ExportTextureFactory* pExportFactory = GetExportTextureFactory(
		bundler.GetOutputOptions().platform);
	if (!pExportFactory) return false;
	
	// Load image
	Attk::C_Image img;
	if (!LoadTexture(img, bundler, m_source, m_alphaSource))
	{
		printf("Error: Unable to read %s/%s\n", m_source.Chars(), m_alphaSource.Chars());
		return false;
	}
	//Perform any necessary resizing operations.
	if (!ResizeTexture(img, m_widthOverride, m_heightOverride))
		return false;

	//Create a new mip-map chain on the heap which will be manager by I_ExportTexture.
	C_ProcessedMipmapChain* pProcessedMipmapChain = new C_ProcessedMipmapChain(
		img.GetWidth(),
		img.GetHeight());
	if (!pProcessedMipmapChain)
		return false;

	//Now generate our mip-chain in the correct format.
	if (!GenerateProcessedMipmapChain(
		bundler,
		*pProcessedMipmapChain,
		img))
		return false;

	//Create a new texture exporter and perform export.
	C_OutputTextureData outputData;
	C_OutputResourceHeader outputHeader;
	{
		I_ExportTexture* pExportTexture = pExportFactory->CreateTexture2D();
		if (!pExportTexture)
			return false;

		pExportTexture->AddMipmapChain(pProcessedMipmapChain);

		//Generate output texture data.
		if (!pExportTexture->Export(
			outputData,
			bundler.GetOutputOptions(),
			m_exportOptions))
			return false;
		assert(outputData.GetDataPtr());

		//Generate output header.
		if (!pExportTexture->GetOutputHeader(outputHeader))
			return false;

		//We don't need our texture exporter anymore.
		delete pExportTexture;
	}

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
	C_ExportHeaderTexture2D* pResourceHeader = new C_ExportHeaderTexture2D(
		m_exportOptions,
		outputHeader,
		dataOffset,
		outputData.GetDataSize(),
		compressedSize);
	bundler.AddResource(pResourceHeader);

	//We're done.
	return true;
}
