#include "stdafx.h"

#include <process_texture/ProcessedMipmapChain.h>

#include "../IExportTexture.h"
#include "ExportTextureWii.h"
#include <export_resource/OutputResourceHeader.h>
#include <export_resource/ExportTextureOptions.h>

#include "../OutputTextureData.h"
#include "../../BundlerOutputOptions.h"
#include "../../Utility.h"

using namespace ResourceBundler;

bool C_ExportTextureWii::_GetOutputHeader(C_OutputResourceHeader& header)
{
	return true;
}
#if 0
U32 C_ExportTextureWii::_GetXboxFormat(Display::ResourceBundler::E_TextureFormat format)
{
	U32 output;
	//We need to find the D3D native format.
	switch (format)
	{
		case Display::ResourceBundler::E_TextureFormat_DXT1:
			output = D3DFMT_DXT1;
			break;
		case Display::ResourceBundler::E_TextureFormat_DXT2:
			output = D3DFMT_DXT2;
			break;
		case Display::ResourceBundler::E_TextureFormat_DXT3:
			output = D3DFMT_DXT3;
			break;
		case Display::ResourceBundler::E_TextureFormat_DXT4:
			output = D3DFMT_DXT4;
			break;
		case Display::ResourceBundler::E_TextureFormat_DXT5:
			output = D3DFMT_DXT5;
			break;
		case Display::ResourceBundler::E_TextureFormat_DXN:
			output = D3DFMT_DXN;
			break;
		case Display::ResourceBundler::E_TextureFormat_A8R8G8B8:
			output = D3DFMT_A8R8G8B8;
			break;
		case Display::ResourceBundler::E_TextureFormat_L8:
			output = D3DFMT_L8;
			break;
		case Display::ResourceBundler::E_TextureFormat_A32R32G32B32:
			output = D3DFMT_A32B32G32R32;
			break;
		/*
		case E_TextureFormat_A8:
			format = D3DFMT_A8;
			break;
		case E_TextureFormat_R5G6B5:
			format = D3DFMT_R6G5B5;
			break;
		case E_TextureFormat_A4R4G4B4:
			format = D3DFMT_A4R4G4B4;
			break;
			*/
		case Display::ResourceBundler::E_TextureFormat_Unknown:
		default:
			assert(!"Could not determine D3D format.");
			return 0;
	}

	return output;
}
#endif