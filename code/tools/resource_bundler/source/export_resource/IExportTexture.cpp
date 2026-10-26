#include "stdafx.h"

#include <process_texture/ProcessedMipmapChain.h>

#include "ExportTextureOptions.h"
#include "IExportTexture.h"

#include <export_resource/OutputResourceHeader.h>

using namespace ResourceBundler;


I_ExportTexture::I_ExportTexture()
{
	m_name	= "";
	m_gamma = false;
}

I_ExportTexture::~I_ExportTexture()
{
	Ut::UtList<C_ProcessedMipmapChain*>::Iterator iter = m_mipmapChains.Begin();

	while (iter != m_mipmapChains.End())
	{
		delete *iter;

		++iter;
	}
}

bool I_ExportTexture::AddMipmapChain(C_ProcessedMipmapChain* pMipmapChain)
{
	if (m_mipmapChains.Size() > 0)
	{
		if (pMipmapChain->GetNumLevels() != m_mipmapChains.Front()->GetNumLevels())
		{
			printf("Mismatch in the number of mip levels between chains.\n");
			return false;
		}
	}

	m_mipmapChains.PushBack(pMipmapChain);
	return true;
}

U32 I_ExportTexture::GetNumMipmapChains()
{
	return m_mipmapChains.Size();
}


bool I_ExportTexture::GetOutputHeader(C_OutputResourceHeader& header)
{
	header.textureFormat	= m_textureFormat;
	header.numMipLevels		= m_mipmapChains.Front()->GetNumLevels();

	return _GetOutputHeader(header);
}

bool I_ExportTexture::Export(
	C_OutputTextureData& output, 
	const C_BundlerOutputOptions& bundlerOptions,
	const C_ExportTextureOptions& exportOptions)
{
	return _Export(output, bundlerOptions, exportOptions);
}

