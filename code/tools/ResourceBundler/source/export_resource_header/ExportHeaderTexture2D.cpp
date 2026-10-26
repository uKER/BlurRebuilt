#include "stdafx.h"

#include <display/bundler/ResourceHeader.h>
#include <display/bundler/ResourceTexture.h>
#include <display/bundler/ResourceTexture2D.h>

#include "BundlerOutputOptions.h"
#include "Bundler.h"

#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>
#include "IExportResourceHeader.h"
#include "ExportHeaderTexture.h"
#include "ExportHeaderTexture2D.h"
#include "ExportHeaderTextureArray2D.h"

using namespace ResourceBundler;

C_ExportHeaderTexture2D::C_ExportHeaderTexture2D(
	const C_ExportTextureOptions& exportOptions,
	const C_OutputResourceHeader& outputHeader,
	U32 dataOffset,
	U32 uncompressedDataSize,
	U32 compressedDataSize)
 : 
	C_ExportHeaderTexture(
		exportOptions,
		outputHeader,
		dataOffset,
		uncompressedDataSize,
		compressedDataSize)
{
}

U32 C_ExportHeaderTexture2D::CalculateHeaderSize() const
{
	return sizeof(Display::ResourceBundler::C_ResourceTexture2D);
}

U32 C_ExportHeaderTexture2D::GetType() const
{
	return Display::ResourceBundler::E_ResourceType_Texture2D;
}

void C_ExportHeaderTexture2D::Serialise(
	Sys::BinarySerialiserAutoEndian& serialiser,
	U16	offsetNext,
	U16	offsetString,
	U32	offsetData) const
{
	Display::ResourceBundler::C_ResourceTexture2D resource;

	assert(GetType() < 65535);
	//Write out our resource type Id
	resource.type = (U16)GetType();

	resource.offsetNext = offsetNext;
	resource.offsetString = offsetString;
	resource.offsetData = offsetData;
	resource.uncompressedDataSize = m_uncompressedDataSize;
	resource.compressedDataSize = m_compressedDataSize;

	resource.width = m_outputHeader.texture2D.width;
	resource.height = m_outputHeader.texture2D.height;
	resource.format = m_outputHeader.textureFormat;
	resource.mipLevels = m_outputHeader.numMipLevels;
	
	printf("2D Texture:\tWrote %-32s (%d bytes)\n\t%d x %d\n\t%d levels\n\t\n",
		m_exportOptions.name.Chars(),
		m_uncompressedDataSize,
		m_outputHeader.texture2D.width, 
		m_outputHeader.texture2D.height, 
		m_outputHeader.numMipLevels);

	//Serialise this resouce.
	serialiser << resource;
}


