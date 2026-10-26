#include "stdafx.h"

#include "../BundlerOutputOptions.h"
#include "../Bundler.h"
#include <export_resource/ExportTextureOptions.h>

#include "ProcessedMipmapChain.h"
#include "IProcessTexture.h"
#include "ProcessArrayOfTextures2D.h"
#include "ProcessTextureArray2D.h"

#include <export_resource/IExportTexture.h>
#include <export_resource/IExportTextureFactory.h>
#include <export_resource/OutputTextureData.h>
#include <export_resource/OutputResourceHeader.h>

#include <export_resource_header/IExportResourceHeader.h>
#include <export_resource_header/ExportHeaderTexture.h>
#include <export_resource_header/ExportHeaderTexture2D.h>
#include <export_resource_header/ExportHeaderTextureArray2D.h>
#include <export_resource_header/ExportHeaderTextureCubeMap.h>

using namespace ResourceBundler;

bool C_ProcessTextureArray2D::Read(
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

bool C_ProcessTextureArray2D::Process(
	C_Bundler& bundler)
{
	printf("Process 2D Array Texture: %s\n", m_exportOptions.name.Chars());

	const I_ExportTextureFactory* pExportFactory = GetExportTextureFactory(
		bundler.GetOutputOptions().platform);
	if (!pExportFactory) return false;

	C_OutputTextureData outputData;
	C_OutputResourceHeader outputHeader;
	I_ExportTexture* pExportTexture = pExportFactory->CreateTextureArray2D();
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
	C_ExportHeaderTextureArray2D* pResourceHeader = new C_ExportHeaderTextureArray2D(
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