#include "stdafx.h"

#include "../BundlerOutputOptions.h"
#include "../Bundler.h"
#include "../export_resource/ExportTextureOptions.h"

#include "IProcessTexture.h"
#include "ProcessArrayOfTextures2D.h"

using namespace ResourceBundler;


#include "stdafx.h"

#include "../BundlerOutputOptions.h"
#include "../Bundler.h"
#include <export_resource/ExportTextureOptions.h>

#include "ProcessedMipmapChain.h"
#include "IProcessTexture.h"
#include "ProcessTextureCubeMap.h"

#include <export_resource/IExportTexture.h>
#include <export_resource/IExportTextureFactory.h>
#include <export_resource/OutputTextureData.h>
#include <export_resource/OutputResourceHeader.h>

#include <export_resource_header/IExportResourceHeader.h>
#include <export_resource_header/ExportHeaderTexture.h>
#include <export_resource_header/ExportHeaderTextureCubeMap.h>

using namespace ResourceBundler;

bool C_ProcessArrayOfTextures2D::Read(
	const C_Bundler& bundler, 
	const Sys::C_XmlAttributeList &attributes)
{
	if (!__super::Read(bundler, attributes))
		return false;

	for (int i=0; i<attributes.Elements(); i++)
	{
		const Sys::C_XmlAttribute *pAttr = attributes[i];
		Util::C_String n = pAttr->name.Lower();

		if (n == "source")
		{
			m_sources.PushBack(pAttr->value);
			if (!bundler.GetPath(m_sources.Back()))
			{
				printf("Error: %s does not exist\n", m_sources.Back().Chars());
				return false;
			}
		}
		else if (n == "alphasource" )
		{
			m_alphaSources.PushBack(pAttr->value);
			if (!bundler.GetPath(m_alphaSources.Back()))
			{
				printf("Error: %s doues not exist\n", m_alphaSources.Back().Chars());
				return false;
			}
		}
	}

	if (m_sources.Size() == 0)
	{
		printf("No images specified.\n");
		return false;
	}

	return true;
}

bool C_ProcessArrayOfTextures2D::_Process(
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
		Util::C_String alphaSource("");
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

	for (U32 iImage = 0; iImage < (U32)m_sources.Size(); iImage++)
	{
		//Create a new mip-map chain on the heap which will be manager by I_ExportTexture.
		C_ProcessedMipmapChain* pProcessedMipmapChain = new C_ProcessedMipmapChain(
			pImageList[iImage].GetWidth(),
			pImageList[iImage].GetHeight());
		if (!pProcessedMipmapChain)
			return false;

		//Now generate our mip-chain in the correct format.
		if (!GenerateProcessedMipmapChain(
			bundler,
			*pProcessedMipmapChain,
			pImageList[iImage]))
			return false;

		exportTexture.AddMipmapChain(pProcessedMipmapChain);
	}

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