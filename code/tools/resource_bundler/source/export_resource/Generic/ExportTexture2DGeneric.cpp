#include "stdafx.h"
#include <process_texture/ProcessedMipmapChain.h>

#include <export_resource/ExportTextureOptions.h>
#include <export_resource/IExportTexture.h>
#include <export_resource/OutputResourceHeader.h>

#include "ExportTextureGeneric.h"
#include "ExportArrayOfTextures2DGeneric.h"
#include "ExportTexture2DGeneric.h"

#include "../../BundlerOutputOptions.h"
#include "../OutputTextureData.h"

using namespace ResourceBundler;


bool C_ExportTexture2DGeneric::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;

	header.texture2D.width	= m_mipmapChains.Front()->GetWidth();
	header.texture2D.height	= m_mipmapChains.Front()->GetHeight();

	return true;
}

