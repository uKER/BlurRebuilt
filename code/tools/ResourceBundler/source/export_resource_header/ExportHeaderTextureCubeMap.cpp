#include "stdafx.h"

#include <display/bundler/ResourceHeader.h>
#include <display/bundler/ResourceTexture.h>
#include <display/bundler/ResourceTextureCubeMap.h>

#include "BundlerOutputOptions.h"
#include "Bundler.h"

#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>
#include "IExportResourceHeader.h"
#include "ExportHeaderTexture.h"
#include "ExportHeaderTextureCubeMap.h"

using namespace ResourceBundler;

C_ExportHeaderTextureCubeMap::C_ExportHeaderTextureCubeMap(
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

U32 C_ExportHeaderTextureCubeMap::CalculateHeaderSize() const
{
	return sizeof(Display::ResourceBundler::C_ResourceTextureCubeMap);
}

U32 C_ExportHeaderTextureCubeMap::GetType() const
{
	return Display::ResourceBundler::E_ResourceType_TextureCubeMap;
}

void C_ExportHeaderTextureCubeMap::Serialise(
	Sys::BinarySerialiserAutoEndian& serialiser,
	U16		offsetNext,				// Offset to next resource, or Zero for last one
	U16		offsetString,			// Offset to string entry from start of this header
	U32		offsetData) const	// Offset to platform specific resource data from start of platform specific data
{
	Display::ResourceBundler::C_ResourceTextureCubeMap resource;

	assert(GetType() < 65535);
	//Write out our resource type Id
	resource.type = (U16)GetType();

	resource.offsetNext = offsetNext;
	resource.offsetString = offsetString;
	resource.offsetData = offsetData;
	resource.uncompressedDataSize = m_uncompressedDataSize;
	resource.compressedDataSize = m_compressedDataSize;

	resource.size = m_outputHeader.textureCubeMap.size;
	resource.format = m_outputHeader.textureFormat;
	resource.mipLevels = m_outputHeader.numMipLevels;
	
	printf("Cube Map:\tWrote %-32s (%d bytes)\n\t%d x %d\n\t%d levels\n\t\n",
		m_exportOptions.name.Chars(),
		m_uncompressedDataSize,
		m_outputHeader.textureCubeMap.size,
		m_outputHeader.textureCubeMap.size,
		m_outputHeader.numMipLevels);

	//Serialise this resouce.
	serialiser << resource;
}
