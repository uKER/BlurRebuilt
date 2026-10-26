#include "stdafx.h"

#include <process_texture/ProcessedMipmapChain.h>

#include "../IExportTexture.h"
#include "ExportTextureGeneric.h"
#include "ExportArrayOfTextures2DGeneric.h"
#include "ExportTexture3DGeneric.h"
#include <export_resource/OutputResourceHeader.h>

using namespace ResourceBundler;


bool C_ExportTexture3DGeneric::_GetOutputHeader(C_OutputResourceHeader& header)
{
	if (!__super::_GetOutputHeader(header))
		return false;

	header.arrayTexture2D.numImages	= m_mipmapChains.Front()->GetDepth();
	header.arrayTexture2D.width		= m_mipmapChains.Front()->GetWidth();
	header.arrayTexture2D.height	= m_mipmapChains.Front()->GetHeight();

	return true;
}

