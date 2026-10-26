#include "stdafx.h"

#include <export_resource/IExportTexture.h>
#include <export_resource/OutputResourceHeader.h>

#include <process_texture/ProcessedMipmapChain.h>

#include "ExportTextureGeneric.h"

using namespace ResourceBundler;

C_ExportTextureGeneric::C_ExportTextureGeneric()
{
}

bool C_ExportTextureGeneric::_GetOutputHeader(C_OutputResourceHeader& header)
{
	return true;
}
