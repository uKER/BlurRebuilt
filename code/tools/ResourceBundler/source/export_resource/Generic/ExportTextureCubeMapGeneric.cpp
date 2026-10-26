#include "stdafx.h"
#include <process_texture/ProcessedMipmapChain.h>

#include <export_resource/ExportTextureOptions.h>
#include <export_resource/IExportTexture.h>
#include <export_resource/OutputResourceHeader.h>

#include "ExportTextureGeneric.h"
#include "ExportArrayOfTextures2DGeneric.h"
#include "ExportTextureCubeMapGeneric.h"
#include "ExportTexture2DGeneric.h"

#include "../../BundlerOutputOptions.h"
#include "../OutputTextureData.h"

using namespace ResourceBundler;


bool C_ExportTextureCubeMapGeneric::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;

	header.textureCubeMap.size	= m_mipmapChains.Front()->GetWidth();

	return true;
}

