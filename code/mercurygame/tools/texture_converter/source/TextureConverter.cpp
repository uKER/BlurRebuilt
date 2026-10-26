#include "stdafx.h"
#include "TextureConverter.h"
#include "attk/attk.h"
#include "tools_shared/PlatformHelpers.h"
#include "texture_export/texture.h"
#include "texture_export/targets/pctex.h"
#include "texture_export/targets/ps3tex.h"
#include "texture_export/targets/xetex.h"
#include "texture_export/converters/atidxt.h"
#include "texture_export/converters/generatemipmaps.h"
#include "texture_export/converters/normalisecolour.h"
#include "texture_export/converters/rescalesurface.h"
#include "texture_export/converters/sRGB.h"
#include "texture_export/utility.h"
#include <system/perf.h>
#include <converter_shared/helpers/ExportHelpers.h>



bool RemoveReadOnlyFlag(const Util::C_String& filePath)
{
	Sys::C_FileInfo info;
	if (Sys::C_FileSystem::Instance().FileInfo(filePath, &info))
	{
		//If the file exists and is not writable.
		if (info.m_FileType == Sys::E_FileType_File && !(info.m_FileAccess & Sys::E_FileAccess_Write))
		{
			if (!Sys::C_FileSystem::Instance().FileAccess(filePath, info.m_FileAccess | Sys::E_FileAccess_Write))
			{
				EXECUTER_FATAL("ERROR: Could not set file permissions for %s.\n", filePath.Chars());
				return false;
			}
		}
	}

	return true;
}

bool C_TextureConverter::Process(
	Tools::FileCopy::C_FileCopyList& uberCopyList,
	const C_Options& options)
{
	PERF_EVENT("Process");
	bool success = true;
	ActTools::Texture *pImage = NULL;

	//Read Texture
	if (Sys::C_FileSystem::Instance().FileExists(options.GetInputAssetFilePath()))
	{
		pImage = TextureExport::LoadTexture(options.GetInputAssetFilePath().Chars());

		if (!pImage)
		{
			EXECUTER_FATAL("Error reading texture %s", options.GetInputAssetFilePath());
			return false;
		}
	}
	else
	{
		EXECUTER_FATAL("Texture %s doesn't exist", options.GetInputAssetFilePath());
		return false;
	}

	//Process Texture

	U32 widthResize, heightResize;
	if (options.GetResize(widthResize, heightResize))
	{
		if( widthResize != pImage->GetWidth() || heightResize != pImage->GetHeight())
		{
			Texutils::ResizeTexture(pImage, widthResize, heightResize,
				DEFAULT_COLOR_FILTER, 1.f, 
				DEFAULT_ALPHA_FILTER, 1.f,
				CSTR_CLAMP, CSTR_CLAMP);

			//destroy mips
			for (U32 i=0; i<pImage->GetDepth(); ++i)
				for (U32 j=1; j<pImage->GetNumMipMaps(); ++j)
					pImage->DetachSurface(i, j);
		}
	}

	if (options.GetColourNormalize())
	{
		//Texutils::NormaliseColour(pImage);
		ColourNormalize(pImage);

	}

	//Check if we actually need to generate mipmaps (prefer artist-created ones in dds textures)
	if (options.GetGenerateMipmaps() && pImage->GetNumMipMaps() == 1)
	{
		if (options.GetGamma())
			Texutils::SRGBToLinear(pImage);

		Texutils::GenerateMipmaps(pImage, ActTools::FT_TRIANGLE, 1.f, ActTools::FT_TRIANGLE, 1.f, ActTools::EDGE_CLAMP, ActTools::EDGE_CLAMP, 0);

		if (options.GetGamma())
			Texutils::LinearToSRGB(pImage);
	}

//Uncomment to write out mip levels for debugging.
/*
		for (S32 i = 0; i < pImage->GetNumMipMaps(); ++i)
		{
			Attk::C_Image *pAttkImage = new Attk::C_Image();

			// make a copy of the input texture and convert it to A8B8G8R8
			ActTools::A8R8G8B8 texelFormat;
			ActTools::Texture *pTextureTmp = new ActTools::Texture;
			pTextureTmp->CopyFrom(*pImage);
			pTextureTmp->ConvertTo(&texelFormat);

			// use the resulting texture data to create an Attk image
			ActTools::Surface *pSurface = pTextureTmp->GetMipmap(0,i);
			pAttkImage->NewPixelData(
				pTextureTmp->GetMipmap(0 ,i)->GetWidth(),
				pTextureTmp->GetMipmap(0 ,i)->GetHeight(),
				1,
				Attk::E_PixelFormat_B8G8R8A8U,
				pSurface->GetTexels());
			delete pTextureTmp;

			char buf[MAX_PATH];
			sprintf(buf, "c:\\temp\\mip_%i.tga", i);
			pAttkImage->Write(buf);
			delete pAttkImage;
		}
*/

	//Write out data for our different skus.
	for(S32 platformNo=0; platformNo < Util::E_Platform_Count; platformNo++)
    {
		//Create output folder.
		Util::C_String textureName;
		if (options.GetOutputNameOverride().Length() == 0)
			//Using default texture name.
			textureName = Util::RemoveFileExtension(Util::ExtractFilename(options.GetInputAssetFilePath()));
		else
			//User has overriden texture name
			textureName = options.GetOutputNameOverride();
	
		Util::C_String relativeOutputFile = options.GetOutputPath() +  textureName;
		relativeOutputFile = relativeOutputFile.IReplace(options.GetRootTempPath(), "");

		Util::C_String outputFilePath = 
			options.GetCopyLocations()[platformNo].workingPath + relativeOutputFile;

		success = RemoveReadOnlyFlag(outputFilePath);
		if (!success) goto exit;

		outputFilePath += ".texture"; 
		
		//Export
		Sys::I_File* pFile = ExportHelpers::WriteExportFile( outputFilePath, (Util::E_Platform)platformNo, &uberCopyList);
	    if(pFile)
	    {	
			EXECUTER_INFO("Creating [%s]\n", outputFilePath.Chars());

			WriteTexture(pImage, options, Util::E_Platform(platformNo), *pFile);
    
			SAFE_DELETE_PTR(pFile);
	    }
	}

exit:
	SAFE_DELETE_PTR(pImage);
	return success;
}

enum E_TextureFileVersion
{
	E_TextureFileVersion_Initial = 0,
	E_TextureFileVersion_Current = E_TextureFileVersion_Initial
};


void C_TextureConverter::WriteTexture(ActTools::Texture *pImage, const C_Options& options, Util::E_Platform platform, Sys::I_File& file )
{
	PERF_EVENT("Write Texture");
	//Write blocks and convert
	Sys::C_FileBlock fileBlock( &file, "Texture" );

	{
		Sys::C_FileBlock headerBlock( &fileBlock, "Header" );
		Sys::BinarySerialiserAutoEndian ar( headerBlock );

		ar << U32(E_TextureFileVersion_Current);
		ar << U32((options.GetGamma())?1:0); //Gamma
	}

	{
		Sys::C_FileBlock dataBlock( &fileBlock, "Data" );

		ActTools::Texture *pTexture = new ActTools::Texture();
		*pTexture = *pImage;

		// convert to DXT5 using ATI library first
		if (options.GetTextureFormat() == "DXT1" || options.GetTextureFormat() == "DXT3" || options.GetTextureFormat() == "DXT5")
		{
			ActTools::TexelFormat *p = AtiDXT::GetTexelFormat(options.GetTextureFormat());
			pTexture->ConvertTo(p);
			delete p;
		}
		if (options.GetGamma())
		{
			pTexture->SetSRGBFlag(true);
		}
		else
		{
			pTexture->SetSRGBFlag(false);
		}
		switch (platform)
		{
		case Util::E_Platform_360:
			{
				NglXeTex texWriter;
				ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat(options.GetTextureFormat());
				pTexture->ConvertTo(pFormat);
				delete pFormat;
		
				texWriter.SerialOutTexture(&dataBlock, pTexture);
			}
			break;
		case Util::E_Platform_PC:
			{
				NglPcTex texWriter;
				ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat(options.GetTextureFormat());
				pTexture->ConvertTo(pFormat);
				delete pFormat;

				texWriter.SerialOutTexture(&dataBlock, pTexture);
			}
			break;
		case Util::E_Platform_PS3:
			{
				NglPs3Tex texWriter;
				ActTools::TexelFormat *pFormat = texWriter.GetTexelFormat(options.GetTextureFormat());
				pTexture->ConvertTo(pFormat);
				delete pFormat;

				texWriter.SerialOutTexture(&dataBlock, pTexture);
			}
			break;
		}
		delete pTexture;
	}
}

void C_TextureConverter::ColourNormalize(ActTools::Texture *pImage )
{
	//Each pixel, normalize
	PERF_EVENT("ColourNormalize");

	if (_stricmp(pImage->GetFormat()->GetId(), "A8R8G8B8"))
	{
		//should only get here if we've elected to keep the image dxt compressed (eg. cubemaps)
		return;
	}

	EXECUTER_INFO("Colour Normalize, Alpha channel deleted");
	for (U32 i=0; i<pImage->GetDepth() * pImage->GetNumMipMaps(); ++i)
	{
		U32 nTexels = pImage->GetSurface(i).GetWidth() * pImage->GetSurface(i).GetHeight();
		U32 *pData = (U32*)pImage->GetSurface(i).GetTexels();

		for (U32 j=0; j<nTexels; ++j)
		{
			Maths::C_RGBA_U8 pixel(pData[j]);

			//Normalize
			U8 maxColour = Maths::Max(pixel.R(), Maths::Max(pixel.G(), pixel.B()));

			pixel.R( (U8)(255.f*(F32)pixel.R()/(F32)maxColour) );
			pixel.G( (U8)(255.f*(F32)pixel.G()/(F32)maxColour) );
			pixel.B( (U8)(255.f*(F32)pixel.B()/(F32)maxColour) );

			//preserve exisiting behaviour for the alpha channel
			pixel.A( (U8)powf(maxColour, 1.1f) );

			pData[j] = pixel.Colour();
		}
	}
}

