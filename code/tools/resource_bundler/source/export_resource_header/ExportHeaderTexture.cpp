#include "stdafx.h"

#include "BundlerOutputOptions.h"
#include "Bundler.h"

#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>
#include "IExportResourceHeader.h"
#include "ExportHeaderTexture.h"

using namespace ResourceBundler;


C_ExportHeaderTexture::C_ExportHeaderTexture(
	const C_ExportTextureOptions& exportOptions,
	const C_OutputResourceHeader& outputHeader,
	U32 dataOffset,
	U32 uncompressedDataSize,
	U32 compressedDataSize) : 
	I_ExportResourceHeader(
		exportOptions,
		outputHeader,
		dataOffset,
		uncompressedDataSize,
		compressedDataSize)
{
}