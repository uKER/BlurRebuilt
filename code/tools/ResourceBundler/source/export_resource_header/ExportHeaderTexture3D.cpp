#include "stdafx.h"

#include <display/bundler/ResourceHeader.h>
#include <display/bundler/ResourceTexture.h>
#include <display/bundler/ResourceTexture3D.h>

#include "BundlerOutputOptions.h"
#include "Bundler.h"

#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>

#include "IExportResourceHeader.h"
#include "ExportHeaderTexture.h"
#include "ExportHeaderTexture3D.h"

using namespace ResourceBundler;

C_ExportHeaderTexture3D::C_ExportHeaderTexture3D(
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

U32 C_ExportHeaderTexture3D::CalculateHeaderSize() const
{
	return sizeof(Display::ResourceBundler::C_ResourceTexture3D);
}

U32 C_ExportHeaderTexture3D::GetType() const
{
	return Display::ResourceBundler::E_ResourceType_Texture3D;
}

void C_ExportHeaderTexture3D::Serialise(
	Sys::BinarySerialiserAutoEndian& serialiser,
	U16		offsetNext,				// Offset to next resource, or Zero for last one
	U16		offsetString,			// Offset to string entry from start of this header
	U32		offsetData) const		// Offset to platform specific resource data from start of platform specific data
{
	Display::ResourceBundler::C_ResourceTexture3D resource;

	assert(GetType() < 65535);
	//Write out our resource type Id
	resource.type = (U16)GetType();

	resource.offsetNext = offsetNext;
	resource.offsetString = offsetString;
	resource.offsetData = offsetData;
	resource.uncompressedDataSize = m_uncompressedDataSize;
	resource.compressedDataSize = m_compressedDataSize;

	resource.format = m_outputHeader.textureFormat;
	resource.mipLevels = m_outputHeader.numMipLevels;
	resource.width = m_outputHeader.arrayTexture2D.width;
	resource.height = m_outputHeader.arrayTexture2D.height;
	resource.depth = m_outputHeader.arrayTexture2D.numImages;
	
	printf("3D Texture:\tWrote %-32s (%d bytes)\n\t%d x %d x %d\n\t%d levels\n\t\n",
		m_exportOptions.name.Chars(),
		m_uncompressedDataSize,
		m_outputHeader.arrayTexture2D.width,
		m_outputHeader.arrayTexture2D.height,
		m_outputHeader.arrayTexture2D.numImages,
		m_outputHeader.numMipLevels);

	//Serialise this resouce.
	serialiser << resource;
}
